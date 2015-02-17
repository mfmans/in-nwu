/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.exe/main.cpp   # 1312
*/


#include "base.h"



/* 内存分配失败回调函数 */
static void __cdecl callback_new_exception (void) {
	// 输出提示
	MessageBoxA (NULL, "内存空间申请失败，应用程序无法继续运行。\n点击“确定”后应用程序将自动终止。", "", MB_ICONERROR | MB_OK);

	// 强制结束程序
	exit(0);
}




/* 主入口 */
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 设置 new 失败回调
	std::set_new_handler (callback_new_exception);

	// 初始化加载器
	load::start	((const char *) lpCmdLine);

	// 进入 GUI
	gui::start	(hInstance, nCmdShow);

	return 0;
}

