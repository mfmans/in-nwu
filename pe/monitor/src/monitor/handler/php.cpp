/*
	$ Monitor Module   (C) 2005-2014 MF
	$ handler/php.cpp
*/


#include "../header/base.h"
#include "../header/handler.h"



/* 读取管道 */
char * handler_php::read (void *stream, unsigned int *size, unsigned int timeout) {
	// 开始时间
	uint32_t	start	= (uint32_t) GetTickCount();
	// 缓冲区
	char *		buffer	= NULL;

	// 已读取大小
	*size	= 0;

	while(true) {
		// 退出代码
		unsigned int code;

		// 判断进程状态
		if(GetExitCodeProcess((HANDLE) this->process, (LPDWORD) & code) == FALSE) {
			break;
		}

		// 如果进程已退出, 结束循环
		if(code != STILL_ACTIVE) {
			break;
		}

		// 管道中数据大小
		unsigned int length;

		// 检查是否有数据可读
		if(PeekNamedPipe((HANDLE) stream, NULL, 0, NULL, (LPDWORD) & length, NULL) == FALSE) {
			break;
		}

		// 如果存在数据可读
		if(length > 0) {
			// 扩充缓冲区大小
			if((buffer = (char *) realloc(buffer, *size + length)) == NULL) {
				break;
			}

			// 读取数据
			ReadFile((HANDLE) stream, buffer + *size, length, (LPDWORD) & length, NULL);

			// 保存已读取大小
			*size = *size + length;

			// 判断最后一个字节是否为结束标志
			if(*(buffer + *size - 1) == '\xFF') {
				break;
			}
		} else {
			// 当前时间
			uint32_t	current		= (uint32_t) GetTickCount();

			// 如果时间溢出, 重置开始时间
			if(current < start) {
				start = current;
			}

			// 计算是否超时
			if((current - start) > timeout) {
				break;
			}
		}
	}

	return buffer;
}


/* 写入管道 */
bool handler_php::write (const char *format, ...) {
	// 如果进程已关闭, 无法写入
	if(this->process == NULL) {
		return false;
	}

	// 参数列表地址
	va_list	argument	= (va_list) (((uintptr_t) & format) + sizeof(uintptr_t));

	// 计算生成长度 (含 \0)
	int		length		= _vscprintf(format, argument) + 1;

	// 判断长度异常
	if(length <= 1) {
		return false;
	}

	// 缓冲区
	char *	buffer		= new char	[length];

	// 转换为 ansi
	vsprintf(buffer, format, argument);

	// 生成结果
	bool			result	= false;
	// 写入的长度
	unsigned int	written	= 0;

	// 写入到标准流
	if(WriteFile(this->stream_in, (LPCVOID) buffer, (DWORD) length, (LPDWORD) & written, NULL) != FALSE) {
		// 判断写入数量
		if(written == (unsigned int) length) {
			result = true;
		}
	}

	delete [] buffer;

	return result;
}



/* 刷新进程状态 */
void handler_php::refresh (void) {
	// 判断当前状态
	if(this->process == NULL) {
		return;
	}

	// 退出代码
	unsigned int code;

	// 获取子进程退出代码
	if(GetExitCodeProcess((HANDLE) this->process, (LPDWORD) & code)) {
		// 进程依旧运行
		if(code == STILL_ACTIVE) {
			return;
		}
	}

	// 设置进程退出状态
	this->process	= NULL;

	// 输出提示信息
	Console::error("#HANDLER#Child process has been terminated (File: php.exe, Code: 0x%02X)\n", code);
}



/* 获取启动参数 */
char * handler_php::command (void) {
	// 当前模块所在路径
	char	directory	[MAX_PATH + 1];

	// 分配 3 倍路径长度缓冲区
	char *	buffer		= new char [MAX_PATH * 3];

	// 清空缓冲区
	memset(directory,	0, MAX_PATH + 1);
	memset(buffer,		0, MAX_PATH * 3);

	// 读取路径
	if(GetModuleFileNameA((HMODULE) Handler::dll, directory, MAX_PATH) == 0) {
		goto failed;
	}

	// 查找路径最后结尾
	char * end	= strrchr(directory, '\\');

	// 路径查找失败
	if(end == NULL) {
		goto failed;
	}

	// 更新最后的 \\ 为 \0, 结束字符串
	*end = '\0';

	// 生成启动参数
	sprintf(buffer, "\"%s\\php\\php.exe\" -f \"%s\\php\\monitor.php\"", directory, directory);

	return buffer;

failed:
	// 释放缓冲区
	delete [] buffer;

	return false;
}




/* 启动 */
bool handler_php::start (const char *path) {
	// 启动参数
	char *	command		= this->command();

	// 获取启动参数失败
	if(command == NULL) {
		return false;
	}

	// 管道通道
	HANDLE	pipe_in_read,  pipe_in_write;					// stdin
	HANDLE	pipe_out_read, pipe_out_write;					// stdout
	HANDLE	pipe_err_read, pipe_err_write;					// stderr

	// 安全
	SECURITY_ATTRIBUTES		security;

	ZeroMemory(& security, sizeof(security));

	security.nLength				= sizeof security;
	security.lpSecurityDescriptor	= NULL;
	security.bInheritHandle			= TRUE;					// 子进程可以继承

	// stdin
	if(CreatePipe(& pipe_in_read, & pipe_in_write, & security, 0) == FALSE) {
		return false;
	}

	// stdout
	if(CreatePipe(& pipe_out_read, & pipe_out_write, & security, 0) == FALSE) {
		return false;
	}

	// stderr
	if(CreatePipe(& pipe_err_read, & pipe_err_write, & security, 0) == FALSE) {
		return false;
	}

	// 启动信息
	STARTUPINFO			startup;
	PROCESS_INFORMATION	process;
	
	ZeroMemory(& startup,	sizeof startup);
	ZeroMemory(& process,	sizeof process);

	// 启动对象
	startup.cb			= sizeof startup;
	startup.hStdInput	= pipe_in_read;
	startup.hStdOutput	= pipe_out_write;
	startup.hStdError	= pipe_err_write;
	startup.wShowWindow	= SW_HIDE;
	startup.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// 启动进程
	BOOL result = CreateProcessA (
		/* lpApplicationName */		NULL,
		/* lpCommandLine */			(LPSTR) command,
		/* lpProcessAttributes */	NULL,
		/* lpThreadAttributes */	NULL,
		/* bInheritHandles */		TRUE,					// 子进程必须可以继承句柄
		/* dwCreationFlags */		0,
		/* lpEnvironment */			NULL,
		/* lpCurrentDirectory */	NULL,
		/* lpStartupInfo */			& startup,
		/* lpProcessInformation */	& process
	);

	// 释放参数
	delete [] command;

	// 启动失败
	if(result == FALSE) {
		Console::error("#HANDLER#Failed to run child process (File: php.exe)\n");

		return false;
	}

	// 关闭管道句柄
	CloseHandle(pipe_in_read);
	CloseHandle(pipe_out_write);
	CloseHandle(pipe_err_write);

	// 关闭线程句柄
	CloseHandle(process.hThread);

	// 保存句柄
	this->process		= (void *) process.hProcess;
	this->stream_in		= (void *) pipe_in_write;
	this->stream_out	= (void *) pipe_out_read;
	this->stream_error	= (void *) pipe_err_read;

	// 发送参数
	uintptr_t argv [ ] = {
		(uintptr_t)		"%hs",
		(uintptr_t)		path,
	};

	// 载入脚本
	if(this->send(0, "~!", 1, (void *) argv) == true) {
		Console::error("#HANDLER#Failed to load script: %s\n", path);

		return false;
	}

	return true;
}


/* 关闭 */
void handler_php::close (void) {
	// 没有进程直接返回
	if(this->process == NULL) {
		return;
	}

	// 终止子进程
	TerminateProcess(this->process, 0);

	// 等待子进程结束
	WaitForSingleObject(this->process, INFINITE);

	// 释放所有管道句柄
	CloseHandle(this->stream_in);
	CloseHandle(this->stream_out);
	CloseHandle(this->stream_error);

	// 释放进程句柄
	CloseHandle(this->process);

	// 重置进程句柄
	this->process	= NULL;
}


/* 发送请求 */
bool handler_php::send (unsigned int index, const char *function, int argc, void *argv) {
	// 刷新子进程状态
	this->refresh();

	// 如果没有进程句柄, 直接允许继续
	if(this->process == NULL) {
		return true;
	}

	// 发送函数请求
	if(this->write("%d@%s\n", argc, function) == false) {
		goto failed;
	}

	// 格式地址
	uintptr_t	format	= (uintptr_t) argv;
	// 参数值地址
	uintptr_t	value	= format + sizeof(uintptr_t);

	// 发送参数请求
	for(int i = 0; i < argc; i ++) {
		// 发送参数信息
		if(this->write(*((const char **) format), *((uint32_t *) value)) == false) {
			goto failed;
		}

		// 发送参数结束标志
		if(this->write("%hs", "\n") == false) {
			goto failed;
		}

		// 指向下一个参数
		format	= value  + sizeof(uintptr_t);
		value	= format + sizeof(uintptr_t);
	}

	// 返回信息长度
	unsigned int	size_error	= 0;
	unsigned int	size_out	= 0;

	// 读取返回信息
	char *	data_error	= this->read(this->stream_error,	& size_error,	500);
	char *	data_out	= this->read(this->stream_out,		& size_out,		100);

	// 输出标准输出流返回
	if(data_out != NULL) {
		Console::notice("#HANDLER#STDOUT: %hs\n", data_out);

		// 释放缓冲区
		free(data_out);
	}

	// 判断标准错误流是否返回
	if(data_error == NULL) {
		Console::warning("#HANDLER#STDERR return 0 byte  [ACCEPT]\n");

		// 直接允许继续
		return true;
	}

	// 是否继续执行
	bool	next	= true;

	if(size_error > 0) {
		// 判断控制字符
		switch(*data_error) {
			// 拒绝
			case 0x00:
				next	= false;

				// 输出提示
				if(*function != '~') {
					Console::warning("#HANDLER#%s handled   [REJECT]\n", function);
				}

				break;

			// 允许
			case 0x01:
				break;

			default:
				{
					char * buffer	= new char [size_error + 1];

					// 复制错误信息
					memcpy(buffer, data_error, size_error);
					// 结束字符串
					memset(buffer + size_error, 0, 1);

					// 输出警告
					Console::warning("#HANDLER#STDERR return unexpected string: %s\n", buffer);
					Console::warning("#HANDLER#STDERR return no control character   [ACCEPT]\n");

					delete [] buffer;
				}
		}
	}

	free(data_error);

	return next;


failed:
	// 输出错误
	Console::error("#HANDLER#Failed to write to STDIN   [ACCEPT]\n");

	// 允许继续
	return true;
}

