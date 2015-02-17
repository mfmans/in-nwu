/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ call.cpp
*/


#include "base.h"



/* 读取模块路径 */
static bool call_module (char *buffer, const char *dll) {
	// 清空内存
	memset(buffer, 0, MAX_PATH + 1);

	// 读取 EXE 文件完整路径
	if(GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0) {
		return false;
	}

	// 查找最后一个 '\'
	char * end	= strrchr(buffer, '\\');

	// 如果没有 '\', 表明读取失败
	if(end == NULL) {
		return false;
	}

	// 计算 '\' 后是否足够的空间容纳 dll 文件名
	if((buffer + MAX_PATH - end - 1) <= (int) strlen(dll)) {
		return false;
	}

	// 连接 dll 文件名
	strcpy(end + 1, dll);

	return true;
}


/* 获取操作系统版本 */
static bool call_system (void) {
	// 获取版本信息
	unsigned long version = (unsigned long) GetVersion();

	// 5.x 为 2000/XP 系统
	if(LOBYTE(LOWORD(version)) <= 5) {
		return true;
	}

	return false;
}




/* 执行注入 */
static const char * call_inject (void *process, const char *handler, bool console, const char *dll, const char *function) {
	// 远程线程
	void *		thread;

	// 注入模块工作函数入口
	uintptr_t	entry;
	// 注入模块工作函数返回结果
	uintptr_t	result;

	// 分配注入代码内存
	uintptr_t	address	= process_memory (process);

	// 判断是否分配成功
	if(address == 0) {
		return "无法在目标进程中分配内存。";
	}

	// 注入代码
	if(remote_inject(process, address, dll, function) == 0) {
		return "向目标进程注入代码失败。";
	}

	// 复制控制台窗口信息
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + REMOTE_MEMORY_CODE, & console, 4) == false) {
		return "向目标进程复制控制台窗口状态失败";
	}

	// 复制控制器路径
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + REMOTE_MEMORY_CODE + 4, (void *) handler, REMOTE_MEMORY_HANDLER) == false) {
		return "向目标进程复制控制器路径失败。";
	}

	// 启动初始化线程
	if((thread = remote_thread (process, address + REMOTE_MEMORY_CONSTANT, 0)) == NULL) {
		return "在目标进程中启动初始化线程失败。";
	}

	// 等待线程结束
	WaitForSingleObject(thread, INFINITE);

	// 获取注入模块工作函数入口
	if(GetExitCodeThread(thread, (LPDWORD) & entry) == FALSE) {
		return "获取 Monitor 注入模块工作函数入口失败。";
	}

	// 判断函数入口是否合法
	if(entry == 0) {
		return "获取 Monitor 注入模块工作函数入口失败。";
	}

	// 释放初始化线程句柄
	CloseHandle(thread);

	// 启动工作函数线程
	if((thread = remote_thread (process, entry, address + REMOTE_MEMORY_CONSTANT + REMOTE_MEMORY_CODE)) == NULL) {
		return "在目标进程中启动注入模块工作函数线程失败。";
	}

	// 等待线程结束
	WaitForSingleObject(thread, INFINITE);

	// 获取工作函数返回结果
	if(GetExitCodeThread(thread, (LPDWORD) & result) == FALSE) {
		return "获取 Monitor 注入模块工作函数返回结果失败。";
	}

	// 释放工作函数线程句柄
	CloseHandle(thread);

	// 判断处理结果
	switch(result) {
		// 成功
		case ML_ERROR_SUCCEED:		break;

		// 失败
		case ML_ERROR_CONSOLE:		return "分配新的控制台窗口失败。\n\n每一个进程只能拥有一个控制台窗口，如果使用 Monitor 加载控制台应用程序，请设置不显示控制台窗口。";
		case ML_ERROR_THREAD:		return "创建 Monitor 监视线程失败。";

		// 控制器错误
		case ML_ERROR_HANDLER_UNKNOWN:				return "不支持当前类型的控制器。";
		case ML_ERROR_HANDLER_DLL:					return "加载 DLL 控制器失败。";
		case ML_ERROR_HANDLER_PHP:					return "启动 PHP 控制器失败。";

		// 劫持函数失败
		case ML_ERROR_FUNCTION_CREATEFILEW:			return "无法劫持 CreateFileW() 函数。";
		case ML_ERROR_FUNCTION_LOADLIBRARYEXW:		return "无法劫持 LoadLibraryExW() 函数。";
		case ML_ERROR_FUNCTION_CREATEPROCESSW:		return "无法劫持 CreateProcessW() 函数。";

		// 未知错误
		default:		return "Monitor 注入模块工作函数返回错误。";
	}

	return NULL;
}


/* 执行线程 */
static int WINAPI call_thread (void *command) {
	// 启动参数
	const char **	argument	= (const char **) command;

	// 启动信息
	const char *	run		= argument [0];
	const char *	handler	= argument [1];
	// 显示控制台窗口
	bool	console	= (bool)  argument [2];

	// 判断控制器长度
	if(strlen(handler) >= REMOTE_MEMORY_HANDLER) {
		gui_response ("控制器路径超过允许的长度。");

		goto done;
	}

	// DLL 文件名
	char		dll			[MAX_PATH + 1];
	// 注入模块工作函数名
	const char	function	[ ]	= "monitor_main";

	// 获取完整的 DLL 文件路径
	if(call_module(dll, "monitor.dll") == false) {
		gui_response ("无法获取需要注入的模块的路径。");

		goto done;
	}

	// 进程信息
	void *	process	= NULL;
	void *	thread	= NULL;

	// 启动进程
	process = process_start(run, & thread);

	// 判断是否启动成功
	if(process == NULL) {
		gui_response ("无法启动指定的应用程序。\n\n请检查输入的路径是否正确（如带有空格的路径被意外分割），以及启动权限是否足够（如需要管理员权限）等。");

		goto done;
	}

	// 如果是老版本 Windows, 必须先让主线程运行一段时间以完成初始化工作
	if(call_system() == true) {
		// 提示
		MessageBoxA(NULL, "您正在使用低版本的 Windows 操作系统，由于注入线程不能先于主线程运行，将无法监视应用程序初始化阶段的函数调用过程。", "", MB_ICONWARNING | MB_OK);

		// 恢复主线程
		ResumeThread((HANDLE) thread);

		// 等待 500ms
		Sleep(500);

		// 再次挂起主线程
		SuspendThread((HANDLE) thread);
	}

	// 执行注入工作
	const char * error = call_inject (process, handler, console, dll, function);

	if(error == NULL) {
		// 如果没有出现错误, 启动主线程
		ResumeThread(thread);

		// 释放句柄
		CloseHandle(thread);
		CloseHandle(process);
	} else {
		// 如果出现错误, 终止进程
		process_close(process, thread);
	}

	// 调用反馈
	gui_response (error);


done:
	// 释放启动信息
	delete [] run;

	// 释放控制器路径
	delete [] handler;

	return 0;
}



/* 执行启动 */
void call (const char *run, const char *handler, bool console) {
	// 线程参数
	static const char * argument [ ] = {
		/* run */		NULL,
		/* handler */	NULL,
		/* console */	NULL,
	};

	// 判断是否提供了启动参数
	if(run == NULL) {
		gui_response ("没有输入应用程序路径及启动参数。");

		return;
	}

	// 判断是否提供了控制器路径
	if(handler == NULL) {
		// 创建 4 个字节缓冲区
		handler = new char [4];

		// 设置为 32 位 0
		memset((void *) handler, 0, 4);
	}

	// 保存线程参数
	argument [0] = run;
	argument [1] = handler;
	argument [2] = (const char *) console;

	// 创建启动线程
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) call_thread, (LPVOID) argument, 0, NULL);
}

