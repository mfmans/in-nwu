/*
	$ Monitor Module   (C) 2005-2014 MF
	$ base.h
*/


#include "header/base.h"
#include "header/error.h"



/* 导出函数 */
#pragma comment (linker, "/EXPORT:monitor_main=_monitor_main@4,PRIVATE")




/* 分配内存失败回调函数 */
static void __cdecl callback_new_exception (void) {
	// 错误提示
	MessageBoxA(NULL, "分配内存失败，应用程序被迫终止。", "", MB_ICONERROR | MB_OK);

	// 结束程序
	exit(0);
}



/* 准备线程 */
static DWORD WINAPI monitor_thread (LPVOID lpParameter) {
	// 输出挂起
	Console::notice("#SYSTEM#Monitor thread executed\n");

	return 0;
}




/* 工作函数 */
extern "C" int __stdcall monitor_main (uintptr_t argument) {
	// 设置 new 失败回调函数
	std::set_new_handler (callback_new_exception);

	// 设置参数值
	bool			console	= *((bool *) argument);
	const char *	script	= (const char *) (argument + 4);

	// 启动错误
	int		error	= 0;

	// 分配控制台窗口
	if(console == true) {
		if(Console::open() == false) {
			return MM_ERROR_CONSOLE;
		}
	}

	// 启动控制器
	if((error = Handler::start(script)) != MM_ERROR_SUCCEED) {
		return error;
	}

	// 安装劫持
	if((error = Router::install()) != MM_ERROR_SUCCEED) {
		return error;
	}

	// 创建准备线程
	if(CreateThread(NULL, 0, monitor_thread, NULL, 0, NULL) == NULL) {
		return MM_ERROR_THREAD;
	}

	// 成功完成工作
	return MM_ERROR_SUCCEED;
}




/* DLL 入口 */
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch(fdwReason) {
		// 进程开始: 设置 DLL 句柄
		case DLL_PROCESS_ATTACH:	Handler::dll = (void *) hinstDLL;	break;
		// 进程结束: 结束控制器
		case DLL_PROCESS_DETACH:	Handler::close();					break;

		// 线程开始
		case DLL_THREAD_ATTACH:		break;
		// 线程结束
		case DLL_THREAD_DETACH:		break;
	}

	return TRUE;
}

