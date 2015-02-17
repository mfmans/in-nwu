/*
	$ MF Client   (C) 2005-2013 MF
	$ icon.c
*/

#include "base.h"
#include "../resource.h"


/* icon object */
static NOTIFYICONDATA icon;


/*
	iconCreate										创建任务栏图标
*/
void iconCreate() {
	HINSTANCE instance;

	instance = (HINSTANCE) GetModuleHandle(NULL);

	ZeroMemory(&icon, sizeof icon);

	icon.cbSize				= sizeof icon;
	icon.hWnd				= handle_window;
	icon.uID				= 0;
	icon.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	icon.uCallbackMessage	= WM_CLIENT;
	icon.hIcon				= LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON2));

	_tcscpy_s(icon.szTip, _countof(icon.szTip), TEXT("网络连接"));

	Shell_NotifyIcon(NIM_ADD, &icon);
}

/*
	iconDestroy										销毁任务栏图标
*/
void iconDestroy() {
	Shell_NotifyIcon(NIM_DELETE, &icon);
}

/*
	iconTip											显示气泡提示

	@ PTCHAR	title
	@ PTCHAR	message
	@ int		type								图标样式
*/
void iconTip(PTCHAR title, PTCHAR message, int type) {
	icon.uFlags			= NIF_MESSAGE | NIF_ICON | NIF_INFO;
	icon.uTimeout		= 5000;

	switch(type) {
		case 1:		icon.dwInfoFlags = NIIF_WARNING;	break;
		case 2:		icon.dwInfoFlags = NIIF_ERROR;		break;
		default:	icon.dwInfoFlags = NIIF_INFO;		break;
	}

	_tcscpy_s(icon.szInfoTitle,	_countof(icon.szInfoTitle),	title);
	_tcscpy_s(icon.szInfo,		_countof(icon.szInfo),		message);

	Shell_NotifyIcon(NIM_MODIFY, &icon);
}

/*
	iconDraw										绘制菜单
*/
void iconDraw() {
	POINT	point;
	HMENU	menu;

	if((menu = CreatePopupMenu()) == NULL) {
		return;
	}

	AppendMenu(menu, MF_STRING,			0x2000,		TEXT("连接网络 (&C)"));
	AppendMenu(menu, MF_STRING,			0x3000,		TEXT("断开连接 (&D)"));
	AppendMenu(menu, MF_SEPARATOR,		0,			NULL);
	AppendMenu(menu, MF_STRING,			0x4000,		TEXT("打开连接窗口 (&W)"));
	AppendMenu(menu, MF_SEPARATOR,		0,			NULL);
	AppendMenu(menu, MF_STRING,			0x1000,		TEXT("关闭 (&X)"));

	/* 禁用菜单 */
	switch(callStatus()) {
		/* 未连接上 */
		case 0:
			EnableMenuItem(menu, 0x3000, MF_DISABLED);
			break;

		/* 已连接上 */
		case 1:
			EnableMenuItem(menu, 0x2000, MF_DISABLED);
			EnableMenuItem(menu, 0x4000, MF_DISABLED);
			break;

		/* 正在连接 */
		case 2:
			EnableMenuItem(menu, 0x2000, MF_DISABLED);
			EnableMenuItem(menu, 0x3000, MF_DISABLED);
			EnableMenuItem(menu, 0x4000, MF_DISABLED);
			break;
	}

	GetCursorPos		(&point);

	SetForegroundWindow	(handle_window);
	TrackPopupMenu		(menu, TPM_LEFTALIGN, point.x, point.y, 0, handle_window, NULL);
	DestroyMenu			(menu);

	PostMessage			(handle_window, WM_NULL, 0, 0);
}

/*
	iconInvoke										菜单功能回调
*/
void iconInvoke(WPARAM id) {
	switch(id) {
		/* 关闭 */
		case 1:
			callClose();
			break;

		/* 连接网络 */
		case 2:
			callStart();
			break;

		/* 断开连接 */
		case 3:
			callDisconnect();
			break;

		/* 显示窗口 */
		case 4:
			iconDestroy();
			guiShow(NULL);
			break;
	}
}
