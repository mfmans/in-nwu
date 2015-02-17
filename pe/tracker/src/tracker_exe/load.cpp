/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.exe/main.cpp   # 1312
*/


#include "base.h"



/* DLL 路径 */
char *	load::library		= NULL;

/* 启动信息 */
char *	load::command		= NULL;
/* 控制器 */
char *	load::controller	= NULL;





/* 生成 DLL 路径 */
void load::build_library (void) {
	// 分配缓冲区
	load::library	= new char [MAX_PATH + 16];

	// 初始化缓冲区
	memset (load::library, 0, MAX_PATH + 16);

	// 获取 EXE 路径
	GetModuleFileNameA (NULL, load::library, MAX_PATH);

	// 查找最后的 "\" 以找到目录部分
	char *	end	= strrchr (load::library, '\\');

	// 如果能拆分出目录部分, 在目录部分结尾结束字符串
	if(end != NULL) {
		* (end + 1) = '\0';
	}

	// 连接文件名
	strcat (load::library, TRACKER_DLL_FILENAME);
}



/* 生成默认启动信息 */
void load::build_argument (const char *argument) {
	// 设置默认参数
	load::command		= NULL;
	load::controller	= NULL;

	// 启动参数长度
	unsigned int	length	= (unsigned int) strlen(argument);

	// 判断启动参数是否为空
	if(length == 0) {
		return;
	}

	// 分配启动参数副本内存
	char *	copy	= new char [length + 1];

	// 创建启动参数副本
	memcpy (copy, argument, length + 1);

	// 检测启动参数是否以 @"*" 开头
	if((*copy == '@') && (*(copy + 1) == '"')) {
		// 控制器为 @"*" 中的内容
		load::controller	= copy + 2;
		// 启动信息指向控制器结束
		load::command		= strchr (load::controller, '"');

		// 如果没有找到控制器结束引号
		if(load::command == NULL) {
			// 控制器信息不完整
			load::controller	= NULL;
		} else {
			// 将原有的引号修改为 \0 作为控制器字符串结束标志
			* load::command		= '\0';

			// 将启动信息移向分割后的第二个字符串
			load::command ++;
		}
	} else {
		// 所有启动参数作为启动信息
		load::command	= copy;
	}

	// 判断空启动信息
	if(load::command != NULL) {
		// 过滤启动信息里的空格
		while(isspace (* load::command) != 0) {
			load::command ++;
		}

		// 判断启动信息是否为空字符串
		if(* load::command == '\0') {
			load::command = NULL;
		}
	}

	// 判断空控制器
	if((load::controller != NULL) && (* load::controller == '\0')) {
		load::controller = NULL;
	}

	// 如果启动信息和控制器均为空, 释放启动参数副本
	if((load::command == NULL) && (load::controller == NULL)) {
		delete [] copy;
	}
}





/* 执行线程 */
int __stdcall load::thread (void *dummy) {
	// 载入动态链接库
	HMODULE		module		= LoadLibraryA (load::library);
	// 获取远程注入函数地址
	uintptr_t	function	= (uintptr_t) GetProcAddress (module, "tracker_remote");

	// 判断是否获取成功
	if(function == 0) {
		gui::response_error ("加载动态链接库 tracker.dll 失败。");

		goto end;
	}

	// 启动信息
	STARTUPINFOA		startup;
	// 进程信息
	PROCESS_INFORMATION	process;
	
	// 清空启动进程信息
	ZeroMemory(& startup,	sizeof(startup));
	ZeroMemory(& process,	sizeof(process));

	// 初始化启动信息
	startup.cb		= sizeof(startup);

	// 启动进程
	if(CreateProcessA (
		/* lpApplicationName */		NULL,
		/* lpCommandLine */			(LPSTR) load::command,
		/* lpProcessAttributes */	NULL,
		/* lpThreadAttributes */	NULL,
		/* bInheritHandles */		FALSE,
		/* dwCreationFlags */		CREATE_SUSPENDED,
		/* lpEnvironment */			NULL,
		/* lpCurrentDirectory */	NULL,
		/* lpStartupInfo */			& startup,
		/* lpProcessInformation */	& process
	) == FALSE) {
		gui::response_error (	"无法启动指定的应用程序。\n\n"
								"请检查输入的路径是否正确（如带有空格的路径被意外分割），以及启动权限是否足够（如需要管理员权限）等。"	);

		goto end;
	}

	// 调用追踪函数注入新启动进程 (in tracker)
	int result = ((int (__cdecl *) (HANDLE, const char *, const char *)) function) (process.hThread, load::library, load::controller);

	// 检测是否注入成功
	if(result == 0) {
		// 注入失败终止进程
		TerminateProcess	(process.hProcess, 0);

		// 等待进程退出
		WaitForSingleObject	(process.hProcess, INFINITE);

		// 失败响应, 错误信息由 tracker_install 输出
		gui::response_error	( );
	} else {
		// 注入成功恢复主线程
		ResumeThread	(process.hThread);

		// 成功响应
		gui::response_succeed	( );
	}

	// 关闭句柄
	CloseHandle (process.hProcess);
	CloseHandle (process.hThread);


end:
	// 卸载动态链接库
	FreeLibrary (module);

	return 0;
}





/* 初始化信息 */
void load::start (const char *argument) {
	// 生成 DLL 路径
	load::build_library		( );
	// 生成默认启动信息
	load::build_argument	(argument);
}



/* 启动进程 */
void load::run (void) {
	// 启动线程
	if(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) load::thread, NULL, 0, NULL) == NULL) {
		gui::response_error ("无法创建新的线程。");
	}
}

