/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.exe/gui.cpp   # 1312
*/


#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include "base.h"




/* 窗口信息 */
#define CLASS				"tracker_loader_class"
#define TITLE				"Tracker w64"
#define WIDTH				420
#define HEIGHT				230



/* 窗口控件编号高位 */
#define ID							0x1000

/* 窗口控件编号低位 */
#define ID_LABEL_COMMAND			0x01
#define ID_LABEL_CONTROLLER			0x02
#define ID_TEXT_COMMAND				0x03
#define ID_TEXT_CONTROLLER			0x04
#define ID_BUTTON_OK				0x05
#define ID_BUTTON_BROWSE			0x06





// 当前状态
HANDLE		gui::status		= NULL;

// 应用程序实例
HINSTANCE	gui::instance	= NULL;

// 窗口句柄
HWND		gui::window		= NULL;
// 控件句柄
HWND		gui::component	[8];





/* 选择文件 */
void gui::browse (void) {
	// 输入缓冲区
	char	buffer	[MAX_PATH + 1];

	// 文件信息
	OPENFILENAME	file;

	ZeroMemory (& buffer,	sizeof buffer);
	ZeroMemory (& file,		sizeof file);

	// 设置文件信息
	file.lStructSize	= sizeof file;
	file.lpstrFile		= buffer;
	file.hwndOwner		= window;
	file.lpstrTitle		= "选择应用程序";
	file.lpstrFilter	= "可执行文件\0*.exe\0";
	file.nFilterIndex	= 1;
	file.nMaxFile		= MAX_PATH;
	file.Flags			= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	// 打开文件选择对话框
	if(GetOpenFileNameA (& file) == FALSE) {
		return;
	}

	// 将所选文件路径设置到文本框中
	SetWindowTextA (gui::component [ID_TEXT_COMMAND], buffer);
}



/* 禁止操作 */
void gui::disable (bool status) {
	// 系统菜单
	HMENU	menu	= GetSystemMenu (gui::window, FALSE);

	if(status == true) {
		// 禁用输入
		EnableWindow	(gui::component [ID_TEXT_COMMAND	], FALSE);
		EnableWindow	(gui::component [ID_TEXT_CONTROLLER	], FALSE);
		EnableWindow	(gui::component [ID_BUTTON_OK		], FALSE);
		EnableWindow	(gui::component [ID_BUTTON_BROWSE	], FALSE);

		// 禁用关闭按钮
		EnableMenuItem	(menu, SC_CLOSE, MF_DISABLED | MF_GRAYED);
	} else {
		// 启用输入
		EnableWindow	(gui::component [ID_TEXT_COMMAND	], TRUE);
		EnableWindow	(gui::component [ID_TEXT_CONTROLLER	], TRUE);
		EnableWindow	(gui::component [ID_BUTTON_OK		], TRUE);
		EnableWindow	(gui::component [ID_BUTTON_BROWSE	], TRUE);

		// 启用关闭按钮
		EnableMenuItem	(menu, SC_CLOSE, MF_ENABLED);
	}
}




/* 绘制窗口 */
int gui::draw (HWND hwnd) {
	// 可绘制区域
	RECT	rect;

	// 获取可绘制区域大小
	GetClientRect (hwnd, & rect);

	// 标签
	gui::component [ID_LABEL_COMMAND	]	= CreateWindowExA (0, "STATIC", "应用程序路径及启动参数：",	WS_CHILD | WS_VISIBLE, 15, 15, rect.right - 30, 30, hwnd, (HMENU) (ID | ID_LABEL_COMMAND),		gui::instance, 0);
	gui::component [ID_LABEL_CONTROLLER	]	= CreateWindowExA (0, "STATIC", "追踪控制器模块路径：",		WS_CHILD | WS_VISIBLE, 15, 75, rect.right - 30, 30, hwnd, (HMENU) (ID | ID_LABEL_CONTROLLER),	gui::instance, 0);

	// 文本框
	gui::component [ID_TEXT_COMMAND		]	= CreateWindowExA (WS_EX_CLIENTEDGE, "EDIT", load::command,		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,	15, 37, rect.right - 30, 22, hwnd, (HMENU) (ID | ID_TEXT_COMMAND),		gui::instance, 0);
	gui::component [ID_TEXT_CONTROLLER	]	= CreateWindowExA (WS_EX_CLIENTEDGE, "EDIT", load::controller,	WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,	15, 97, rect.right - 30, 22, hwnd, (HMENU) (ID | ID_TEXT_CONTROLLER),	gui::instance, 0);

	// 按钮
	gui::component [ID_BUTTON_OK		]	= CreateWindowExA (0, "BUTTON", "确定",				WS_CHILD | WS_VISIBLE, rect.right - 230, rect.bottom - 43,  90, 28, hwnd, (HMENU) (ID | ID_BUTTON_OK),		gui::instance, 0);
	gui::component [ID_BUTTON_BROWSE	]	= CreateWindowExA (0, "BUTTON", "选择应用程序 ...",	WS_CHILD | WS_VISIBLE, rect.right - 135, rect.bottom - 43, 120, 28, hwnd, (HMENU) (ID | ID_BUTTON_BROWSE),	gui::instance, 0);

	// 保存窗口句柄
	gui::window		= hwnd;

	// 获取绘图设备
	HDC		hdc		= GetDC (hwnd);
	// 获取系统默认字体
	HFONT	hfont	= (HFONT) GetStockObject (DEFAULT_GUI_FONT);

	// 选择字体
	SelectObject	(hdc, hfont);
	// 释放设备
	ReleaseDC		(hwnd, hdc);

	// 设置字体
	SendDlgItemMessage (hwnd, ID | ID_LABEL_COMMAND,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage (hwnd, ID | ID_LABEL_CONTROLLER,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage (hwnd, ID | ID_TEXT_COMMAND,		WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage (hwnd, ID | ID_TEXT_CONTROLLER,	WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage (hwnd, ID | ID_BUTTON_OK,		WM_SETFONT, (WPARAM) hfont, 0);
	SendDlgItemMessage (hwnd, ID | ID_BUTTON_BROWSE,	WM_SETFONT, (WPARAM) hfont, 0);

	// 判断是否自动开始
	if((load::command != NULL) && (load::controller != NULL)) {
		// 发送按钮单击事件
		SendMessage (gui::component [ID_BUTTON_OK], BM_CLICK, 0, 0);
	}

	return 0;
}


/* 销毁窗口 */
bool gui::close (void) {
	// 判断是否为启动状态
	if(WaitForSingleObject (gui::status, 0) == WAIT_OBJECT_0) {
		MessageBoxA (gui::window, "应用程序正在启动中，在应用程序完成启动后，窗口将自动关闭。", "", MB_ICONINFORMATION | MB_OK);

		return true;
	}

	return false;
}



/* 事件处理 */
void gui::command (WPARAM wparam) {
	// 控件 ID
	WORD id = LOWORD(wparam);

	// 过滤高位
	id	= id & (~ ID);

	// 处理事件
	switch(id) {
		case ID_BUTTON_OK:
			{
				// 输入的文字长度
				int		length_command		= GetWindowTextLengthA (gui::component [ID_TEXT_COMMAND		]) + 1;
				int		length_controller	= GetWindowTextLengthA (gui::component [ID_TEXT_CONTROLLER	]) + 1;

				// 判断是否输入了启动信息
				if(length_command <= 1) {
					MessageBoxA (gui::window, "请输入应用程序路径及启动参数。", "", MB_ICONWARNING | MB_OK);

					break;
				}

				// 判断是否输入了控制器
				if(length_controller <= 1) {
					MessageBoxA (gui::window, "请输入追踪控制器模块路径。", "", MB_ICONWARNING | MB_OK);

					break;
				}

				// 分配缓冲区
				load::command		= new char [length_command		];
				load::controller	= new char [length_controller	];

				// 读取输入信息
				GetWindowTextA (gui::component [ID_TEXT_COMMAND		], load::command,		length_command);
				GetWindowTextA (gui::component [ID_TEXT_CONTROLLER	], load::controller,	length_controller);

				// 判断控制器是否输入了相对路径
				if((strchr (load::controller, '\\') == NULL) && (strchr (load::controller, '/') == NULL)) {
					if(MessageBoxA (gui::window, "输入的追踪控制器模块路径未包含所处目录，当环境变量发生变化时，可能会导致加载控制器失败。是否继续？", "", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
						// 释放缓冲区
						delete [] load::command;
						delete [] load::controller;

						return;
					}
				}

				// 进入禁止操作模式
				gui::disable (true);

				// 设置状态
				SetEvent (gui::status);

				// 启动进程
				load::run();
			}

			break;

		case ID_BUTTON_BROWSE:
			// 选择文件
			gui::browse();

			break;
	}
}




/* 消息循环 */
LRESULT __stdcall gui::loop (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		// 绘制窗口
		case WM_CREATE:
			// 执行窗口绘制
			return (LRESULT) gui::draw (hwnd);

		// 关闭窗口
		case WM_CLOSE:
			// 终止消息循环
			if(gui::close() == true) {
				return 0;
			}

			break;

		// 销毁窗口
		case WM_DESTROY:
			PostQuitMessage(0);

			break;

		// 命令分发
		case WM_COMMAND:
			// 交由消息处理
			gui::command(wparam);
			
			break;
	}

	// 使用默认消息处理方法
	return DefWindowProc (hwnd, msg, wparam, lparam);
}





/* 进入 GUI */
void gui::start (HINSTANCE instance, int show) {
	// 应用程序实例
	gui::instance	= instance;

	// 创建状态信号
	if((gui::status = CreateEvent (NULL, TRUE, FALSE, NULL)) == NULL) {
		return;
	}

	// 窗口类
	WNDCLASSA	wndclass;
	// 消息
	MSG			message;
	// 桌面大小
	RECT		rectangle;

	// 重置内存
	ZeroMemory (& wndclass,		sizeof wndclass);
	ZeroMemory (& message,		sizeof message);
	ZeroMemory (& rectangle,	sizeof rectangle);

	// 窗口类
	wndclass.hInstance		= gui::instance;
	wndclass.lpfnWndProc	= gui::loop;
	wndclass.lpszClassName	= CLASS;
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;
	wndclass.hCursor		= LoadCursor	(NULL,			IDC_ARROW);
	wndclass.hIcon			= LoadIcon		(gui::instance,	MAKEINTRESOURCEA(101));				// IDI_ICON1

	// 注册窗口类
	if(RegisterClassA (& wndclass) == 0) {
		return;
	}

	// 获取桌面大小
	if(GetWindowRect (GetDesktopWindow(), & rectangle) == FALSE) {
		// 设置默认分辨率
		rectangle.right		= WIDTH  * 2;
		rectangle.bottom	= HEIGHT * 2;
	}

	// 创建窗口
	if((gui::window = CreateWindowExA (
		/* dwExStyle */		0,
		/* lpClassName */	CLASS,
		/* lpWindowName */	TITLE,
		/* dwStyle */		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		/* x */				(rectangle.right  - WIDTH ) / 2,
		/* y */				(rectangle.bottom - HEIGHT) / 2,
		/* nWidth */		WIDTH,
		/* nHeight */		HEIGHT,
		/* hWndParent */	NULL,
		/* hMenu */			NULL,
		/* hInstance */		gui::instance,
		/* lpParam */		0
	)) == NULL) {
		return;
	}

	// 显示窗口
	ShowWindow		(gui::window, show);

	// 刷新窗口
	UpdateWindow	(gui::window);

	// 消息循环
	while(GetMessage (& message, 0, 0, 0) > 0) {
		TranslateMessage	(& message);
		DispatchMessage		(& message);
	}
}





/* 响应成功 */
void gui::response_succeed (void) {
	// 终止消息循环
	SendMessage (gui::window, WM_DESTROY, 0, 0);
}



/* 响应失败 (不带消息) */
void gui::response_error (void) {
	// 设置状态
	ResetEvent (gui::status);

	// 释放缓冲区
	delete [] load::command;
	delete [] load::controller;

	// 退出禁止操作模式
	gui::disable (false);
}


/* 响应失败 (带消息) */
void gui::response_error (const char *error) {
	// 弹出错误消息
	MessageBoxA (gui::window, error, "", MB_ICONERROR | MB_OK);

	// 调用通用处理
	gui::response_error ( );
}

