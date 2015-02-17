/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ main.c
*/

/* @ for: Psapi.h */
#pragma comment (lib, "Psapi.lib")
/* @ for: Commctrl.h */
#pragma comment (lib, "ComCtl32.lib")

/* @ 链接器: 子系统 */
#pragma comment (linker, "/subsystem:\"windows\"")
/* @ 链接器: 清单文件 */
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <Windows.h>
#include <Commctrl.h>
#include "base.h"


/* 运行实例 */
HINSTANCE	instance;
/* 同步信号 */
HANDLE		symbolRun, symbolStop, symbolExit, symbolDestroy;


/* 初始化 ComCtl32.dll */
static void initGui() {
	INITCOMMONCONTROLSEX iccx;

	iccx.dwSize	= sizeof iccx;
	iccx.dwICC	= ICC_STANDARD_CLASSES;

	InitCommonControlsEx(&iccx);
}

/* 初始化访问令牌, 启用调试权限 */
static void initToken() {
	HANDLE				token;
	TOKEN_PRIVILEGES	privilege;

	do {
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == FALSE) {
			break;
		}
		if(LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &privilege.Privileges[0].Luid) == FALSE) {
			break;
		}

		privilege.PrivilegeCount = 1;
		privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if(AdjustTokenPrivileges(token, FALSE, &privilege, sizeof privilege, NULL, NULL) == FALSE) {
			break;
		}
		if(GetLastError() != ERROR_SUCCESS) {
			break;
		}

		CloseHandle(token);

		return;
	} while(0);

	DIE("获取调试权限失败，请尝试使用管理员权限重新启动程序。");
}

/* 初始化所有线程同步信号事件 */
static void initEvent() {
	int i;

	/* 所有的信号 */
	void * symbols[] = {&symbolRun, &symbolStop, &symbolExit, &symbolDestroy};

	for(i = 0; i < (sizeof(symbols) / sizeof(void *)); i++) {
		if((*((HANDLE *) symbols[i]) = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
			DIE("程序创建线程同步信号时发生错误，被迫终止。");
		}
	}
}


/* 退出管理器 */
static void exitWait() {
	/* 等待程序执行完毕 */
	WAIT(Exit);
	/* 发送销毁信号 */
	SEND(Destroy);

	/* 等待 1s */
	Sleep(1000);
}


/* 主入口 */
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	instance = hInstance;

	/* 初始化程序 */
	initGui();
	initToken();
	initEvent();

	/* 创建 GUI 线程 */
	THRDRUN(gui);
	/* 创建调试线程 */
	THRDRUN(debug);

	/* 等待程序执行完毕 */
	exitWait();

	return 0;
}
