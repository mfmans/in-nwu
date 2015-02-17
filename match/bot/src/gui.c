/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ gui.c
*/

#include "base.h"


/* 标题 */
#define TITLE					"MF's Robot For LinLink5"
#define TITLE_ON				"[运行中] " TITLE
#define TITLE_OFF				"[已停止] " TITLE

/* 窗口 */
#define CLASSNAME				"mf_robot_class"
#define WIDTH					350
#define HEIGHT					80

/* 按钮 */
#define ID_LOAD					1000
#define ID_RUN					1001


/* 按钮句柄 */
static HWND hLoad, hRun;
/* 主窗口句柄 */
HWND hWin = NULL;


/* 窗口绘制 */
static void draw(HWND hwnd) {
	int		i, w, h;
	int		ids[] = {ID_LOAD, ID_RUN};

	RECT	rect;
	HDC		hdc;
	HFONT	hfont;

	/* 获取可绘制区 */
	GetClientRect(hwnd, &rect);

	/* 计算按钮宽高 */
	w = (rect.right - 15) / 2;
	h = rect.bottom - 10;

	/* 绘制按钮 */
	hLoad	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("挂载游戏进程"), WS_CHILD | WS_VISIBLE, 5, 5,						w, h, hwnd, (HMENU) ID_LOAD,	instance, 0);
	hRun	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("开启自动连线"), WS_CHILD | WS_VISIBLE, rect.right - w - 5, 5,	w, h, hwnd, (HMENU) ID_RUN,		instance, 0);

	/* 设备 */
	hdc		= GetDC(hwnd);
	/* 默认字体 */
	hfont	= (HFONT) GetStockObject(DEFAULT_GUI_FONT);

	SelectObject(hdc, hfont);
	ReleaseDC(hwnd, hdc);

	/* 设置字体 */
	for(i = 0; i < (sizeof ids / sizeof(size_t)); i++) {
		SendDlgItemMessage(hwnd, ids[i], WM_SETFONT, (WPARAM) hfont, 0);
	}

	/* 禁用按钮 */
	EnableWindow(hRun, FALSE);
}

/* 命令处理 */
static void command(int id) {
	switch(id) {
		/* 挂载按钮 */
		case ID_LOAD:
			EnableWindow(hLoad,	FALSE);
			SEND(Run);
			break;

		/* 执行功能 */
		case ID_RUN:
			buttonRun(0);
			THRDRUN(mouse);
			break;
	}
}

/* 消息循环 */
static LRESULT CALLBACK message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		/* 绘制窗口 */
		case WM_CREATE:		draw(hwnd);					return 0;
		/* 销毁窗口 */
		case WM_DESTROY:	PostQuitMessage(0);			return 0;
		/* 命令分发 */
		case WM_COMMAND:	command(LOWORD(wparam));	break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


/* GUI 线程 */
THREAD(gui) {
	MSG						msg;
	RECT					rect;
	WNDCLASS				wndclass;

	/* 获取桌面大小 */
	GetWindowRect(GetDesktopWindow(), &rect);

	ZeroMemory(&wndclass, sizeof wndclass);

	wndclass.hInstance		= instance;
	wndclass.lpszClassName	= TEXT(CLASSNAME);
	wndclass.lpfnWndProc	= &message;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;

	/* 注册窗口类 */
	if(RegisterClass(&wndclass) == 0) {
		DIE("程序注册窗口类时发生错误，被迫终止。");
	}

	/* 创建主窗口 */
	if((hWin = CreateWindowEx (
			/* ex style */	0,
			/* classname */	CLASSNAME,
			/* title */		TITLE,
			/* style */		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			/* left */		(rect.right - WIDTH) / 2,
			/* top */		(rect.bottom - HEIGHT) / 2,
			/* width */		WIDTH,
			/* height */	HEIGHT,
			/* parent */	NULL,
			/* menu */		NULL,
			/* instance */	instance,
			/* lpparam */	0
		)) == NULL) {
			DIE("程序创建窗口失败，被迫终止。");
	}

	ShowWindow(hWin, SW_SHOW);
	UpdateWindow(hWin);

	while(GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SEND(Exit);
}


/* 挂载按钮 */
void buttonLoad(int enable) {
	EnableWindow(hLoad, (BOOL) enable);
}

/* 开启/关闭按钮 */
void buttonRun(int enable) {
	EnableWindow(hRun, (BOOL) enable);
	
	if(enable) {
		SetWindowText(hWin, TEXT(TITLE_OFF));
	} else {
		SetWindowText(hWin, TEXT(TITLE_ON));
	}
}
