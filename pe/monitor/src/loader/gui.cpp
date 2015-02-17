/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ gui.cpp
*/


#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include "base.h"



/* 窗口信息 */
#define CLASS			TEXT("monitor_loader_class")
#define TITLE			TEXT("Monitor 启动器")
#define WIDTH			420
#define HEIGHT			230


/* 窗口控件 */
#define ID_LABEL_RUN			0x1000
#define ID_LABEL_HANDLER		0x1001
#define ID_TEXT_RUN				0x2000
#define ID_TEXT_HANDLER			0x2001
#define ID_CHECK_CONSOLE		0x3000
#define ID_BUTTON_OK			0x4000
#define ID_BUTTON_BROWSE		0x4001




/* 启动信息 */
static	LPCSTR		start_run;
/* 控制器路径 */
static	LPCSTR		start_handler;

/* 显示控制台窗口 */
static	bool		start_console;


/* 正在启动信号 */
static	HANDLE		status;


/* 窗口实例 */
static	HINSTANCE	application;


/* 窗口句柄 */
static	HWND		window;
/* 控件句柄 */
static	HWND		handle_label_run, handle_label_handler;
static	HWND		handle_text_run, handle_text_handler;
static	HWND		handle_check_console;
static	HWND		handle_button_ok, handle_button_browse;




/* 禁用输入 */
static void disable (bool set) {
	if(set == true) {
		EnableWindow(handle_text_run,		FALSE);
		EnableWindow(handle_text_handler,	FALSE);
		EnableWindow(handle_check_console,	FALSE);
		EnableWindow(handle_button_ok,		FALSE);
		EnableWindow(handle_button_browse,	FALSE);
	} else {
		EnableWindow(handle_text_run,		TRUE);
		EnableWindow(handle_text_handler,	TRUE);
		EnableWindow(handle_check_console,	TRUE);
		EnableWindow(handle_button_ok,		TRUE);
		EnableWindow(handle_button_browse,	TRUE);
	}
}


/* 选择文件 */
static void browse (void) {
	// 输入缓冲区
	char	buffer	[MAX_PATH + 1];

	// 文件信息
	OPENFILENAME	file;

	ZeroMemory(& buffer,	sizeof buffer);
	ZeroMemory(& file,		sizeof file);

	// 设置文件信息
	file.lStructSize	= sizeof file;
	file.hwndOwner		= window;
	file.lpstrTitle		= "选择应用程序";
	file.lpstrFilter	= "可执行文件\0*.exe\0";
	file.nFilterIndex	= 1;
	file.lpstrFile		= buffer;
	file.nMaxFile		= MAX_PATH;
	file.Flags			= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	// 打开文件选择对话框
	if(GetOpenFileNameA(& file) == FALSE) {
		return;
	}

	// 将所选文件路径设置到文本框中
	SetWindowTextA(handle_text_run, buffer);
}




/* 窗口绘制 */
static int draw (HWND hwnd) {
	// 可绘制区域
	RECT	rect;

	// 获取可绘制区域大小
	GetClientRect(hwnd, & rect);

	// 文本
	handle_label_run		= CreateWindowExA (0, "STATIC", "应用程序路径及启动参数：",		WS_CHILD | WS_VISIBLE, 15, 15, rect.right - 30, 30, hwnd, (HMENU) ID_LABEL_RUN,		application, 0);
	handle_label_handler	= CreateWindowExA (0, "STATIC", "应用程序接口调用控制器路径：",	WS_CHILD | WS_VISIBLE, 15, 75, rect.right - 30, 30, hwnd, (HMENU) ID_LABEL_HANDLER,	application, 0);

	// 输入文本框
	handle_text_run		= CreateWindowExA (WS_EX_CLIENTEDGE, "EDIT", start_run,		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,	15, 37, rect.right - 30, 22, hwnd, (HMENU) ID_TEXT_RUN,		application, 0);
	handle_text_handler	= CreateWindowExA (WS_EX_CLIENTEDGE, "EDIT", start_handler,	WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,	15, 97, rect.right - 30, 22, hwnd, (HMENU) ID_TEXT_HANDLER,	application, 0);

	// 复选框
	handle_check_console	= CreateWindowExA (0, "BUTTON", "显示控制台窗口",		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 15, rect.bottom - 43, 150, 25, hwnd, (HMENU) ID_CHECK_CONSOLE,	application, 0);

	// 按钮
	handle_button_ok		= CreateWindowExA (0, "BUTTON", "确定",				WS_CHILD | WS_VISIBLE, rect.right - 230, rect.bottom - 43,  90, 28, hwnd, (HMENU) ID_BUTTON_OK,		application, 0);
	handle_button_browse	= CreateWindowExA (0, "BUTTON", "选择应用程序 ...",	WS_CHILD | WS_VISIBLE, rect.right - 135, rect.bottom - 43, 120, 28, hwnd, (HMENU) ID_BUTTON_BROWSE,	application, 0);

	// 获取绘图设备
	HDC		hdc		= GetDC(hwnd);
	// 获取系统默认字体
	HFONT	hfont	= (HFONT) GetStockObject(DEFAULT_GUI_FONT);

	// 选择字体
	SelectObject	(hdc, hfont);
	// 释放设备
	ReleaseDC		(hwnd, hdc);

	// 设置字体
	SendDlgItemMessage(hwnd, ID_LABEL_RUN,		WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_LABEL_HANDLER,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_TEXT_RUN,		WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_TEXT_HANDLER,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_CHECK_CONSOLE,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_BUTTON_OK,		WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage(hwnd, ID_BUTTON_BROWSE,	WM_SETFONT, (WPARAM) hfont, 0);

	// 判断是否显示控制台窗口
	if(start_console == true) {
		SendMessage(handle_check_console, BM_SETCHECK, BST_CHECKED, 0);
	}

	// 判断是否自动开始
	if(start_run != NULL) {
		// 发送按钮单击事件
		SendMessage(handle_button_ok, BM_CLICK, 0, 0);
	}

	return 0;
}



/* 消息处理 */
static void command (WPARAM wparam, LPARAM lparam) {
	// 判断是否为启动状态
	if(WaitForSingleObject(status, 0) == WAIT_OBJECT_0) {
		return;
	}

	// 控件 ID
	WORD id = LOWORD(wparam);

	switch(id) {
		case ID_BUTTON_OK:
			{
				// 输入的文字
				char *	input_run		= NULL;
				char *	input_handler	= NULL;

				// 显示控制台窗口
				bool	input_console	= false;

				// 输入的文字长度
				int		length_run		= GetWindowTextLengthA(handle_text_run);
				int		length_handler	= GetWindowTextLengthA(handle_text_handler);

				// 读取启动信息
				if(length_run > 0) {
					input_run		= new char [length_run + 1];

					GetWindowTextA(handle_text_run, (LPSTR) input_run, length_run + 1);
				}

				// 读取控制器路径
				if(length_handler > 0) {
					input_handler	= new char [length_handler + 1];

					GetWindowTextA(handle_text_handler, (LPSTR) input_handler, length_handler + 1);
				}

				// 读取显示控制台窗口复选框
				if(SendMessage(handle_check_console, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					input_console	= true;
				}

				// 禁用输入
				disable(true);

				// 设置启动状态
				SetEvent(status);

				// 调用处理
				call(input_run, input_handler, input_console);
			}

			break;

		case ID_BUTTON_BROWSE:
			// 选择文件
			browse();

			break;
	}
}


/* 消息循环 */
static LRESULT CALLBACK message (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		// 绘制窗口
		case WM_CREATE:
			// 执行绘制
			return (LRESULT) draw(hwnd);

		// 关闭窗口
		case WM_CLOSE:
			// 判断是否为启动状态
			if(WaitForSingleObject(status, 0) == WAIT_OBJECT_0) {
				MessageBox(window, TEXT("启动器正在启动指定的应用程序，请稍等。"), TEXT(""), MB_OK | MB_ICONWARNING);

				return 0;
			}

			break;

		// 销毁窗口
		case WM_DESTROY:
			// 终止消息循环
			PostQuitMessage(0);

			return 0;

		// 命令分发
		case WM_COMMAND:
			// 交由消息处理
			command(wparam, lparam);
			
			break;
	}

	// 使用默认方法
	return DefWindowProc (hwnd, msg, wparam, lparam);
}




/* GUI 入口 */
int gui_start (const char *run, const char *handler, bool console, void *instance, int show) {
	// 启动参数
	start_run		= (LPCSTR)	run;
	start_handler	= (LPCSTR)	handler;
	start_console	=			console;

	// 应用程序实例
	application	= (HINSTANCE)	instance;

	// 创建信号
	if((status = CreateEvent (NULL, TRUE, FALSE, NULL)) == NULL) {
		return 0;
	}

	// 窗口类
	WNDCLASS	wndclass;
	// 消息
	MSG			msg;
	// 桌面大小
	RECT		rect;

	// 重置内存
	ZeroMemory(& wndclass,	sizeof wndclass);
	ZeroMemory(& msg,		sizeof msg);
	ZeroMemory(& rect,		sizeof rect);

	// 窗口类
	wndclass.hInstance		=   application;
	wndclass.lpfnWndProc	= & message;
	wndclass.lpszClassName	= CLASS;
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;
	wndclass.hIcon			= LoadIcon		(application,	MAKEINTRESOURCEA(102));				// IDI_ICON2
	wndclass.hCursor		= LoadCursor	(NULL,			IDC_ARROW);

	// 注册窗口类
	if(RegisterClass(& wndclass) == 0) {
		return 0;
	}

	// 获取桌面大小
	GetWindowRect(GetDesktopWindow(), &rect);

	// 创建窗口
	if((window = CreateWindowEx (
		/* dwExStyle */		0,
		/* lpClassName */	CLASS,
		/* lpWindowName */	TITLE,
		/* dwStyle */		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		/* x */				(rect.right - WIDTH) / 2,
		/* y */				(rect.bottom - HEIGHT) / 2,
		/* nWidth */		WIDTH,
		/* nHeight */		HEIGHT,
		/* hWndParent */	NULL,
		/* hMenu */			NULL,
		/* hInstance */		application,
		/* lpParam */		0
	)) == NULL) {
		return 0;
	}

	// 显示窗口
	ShowWindow(window, show);

	// 刷新窗口
	UpdateWindow(window);

	// 消息循环
	while(GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage	(&msg);
		DispatchMessage		(&msg);
	}

	return 0;
}


/* GUI 反馈 */
void gui_response (const char *error) {
	// 取消启动状态
	ResetEvent(status);

	// 取消输入禁用
	disable(false);

	// 判断是否出现错误
	if(error != NULL) {
		// 弹出错误提示
		MessageBoxA(window, (LPCSTR) error, "", MB_OK | MB_ICONERROR);

		return;
	}

	// 结束消息循环
	SendMessage(window, WM_DESTROY, 0, 0);
}

