/*
	$ main.c, 0.2.1221
*/

/* @ 链接器: 子系统
     对于 vc11, 请使用 v110_xp 平台工具集并手动设置子系统 */
#if _MSC_VER < 1700
	#pragma comment (linker, "/subsystem:windows")
#endif

/* @ 链接器: 清单文件 */
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
/* @ ComCtl32.lib */
#pragma comment (lib, "ComCtl32.lib")


#include "base.h"
#include <Windows.h>


/* 数据链表 */
number_s	*NUM		= NULL;
/* 主窗口信息 */
HWND		 win		= NULL;
HINSTANCE	 instance	= NULL;


/* 主入口 */
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	instance = hInstance;

	/* 初始化链表 */
	NEW(NUM, number_s, 1);

	/* 启动图形用户界面 */
	gui();

	return 0;
}
