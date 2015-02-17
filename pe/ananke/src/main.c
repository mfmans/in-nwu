/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ main.c, 0.1.1027
*/

#pragma comment (linker, "/subsystem:\"windows\"")
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include "base.h"

/* 主窗口 */
HWND		winHwnd		= NULL;
HINSTANCE	winInstance	= NULL;


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
			guiDraw(hwnd); return 0;

		case WM_DESTROY:
			PostQuitMessage(0); return 0;

		case WM_COMMAND:
			guiCommand(LOWORD(wParam)); return 0;

		case WM_NOTIFY:
			guiNotify(lParam); break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG			msg;
	WNDCLASS	wndclass;

	winInstance = hInstance;

	ZeroMemory(&wndclass, sizeof wndclass);

	wndclass.hInstance		= hInstance;
	wndclass.lpfnWndProc	= &WindowProc;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;
	wndclass.lpszClassName	= WIN_CLASS;

	if(RegisterClass(&wndclass) == 0) {
		die("创建应用程序失败。");
	}

	if((winHwnd = CreateWindow(
		WIN_CLASS, WIN_TITLE,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIN_WIDTH, WIN_HEIGHT,
		NULL, NULL, hInstance, NULL)) == NULL) {
			die("创建应用程序失败。");
	}

	ShowWindow(winHwnd, SW_SHOW);
	UpdateWindow(winHwnd);

	while(GetMessage(&msg, 0, 0, 0) > 0) {
		if(msg.message == WM_KEYDOWN) {
			guiKeyboard((DWORD) msg.wParam);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
