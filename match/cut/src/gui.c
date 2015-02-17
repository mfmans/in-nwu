/*
	$ gui.c, 0.2.1221
*/

#include <Windows.h>
#include <Commctrl.h>
#include <windowsx.h>
#include "base.h"


/* 帮助内容 */
#define HELP	"这是 2012 年西北大学第十二届电脑文化艺术节程序设计大赛入门组第二题的参赛作品。\n\n"	\
				"要使用这个程序，请首先准备文件 file.1，文件中应存放有若干个需要进行处理的正整数，每个整数一行。然后点击程序左上角的“导入数据”按钮，选择这个文件。\n\n"	\
				"导入数据后，请单击程序右上角的“开始计算”按钮，程序即对导入的正整数逐个进行运算，算出它们的乘积。\n\n"	\
				"在计算完成后，点击“保存结果”按钮可以将计算出来的乘积保存到 file.2 文件中。保存乘积的位数可以在程序左下角的文本框中进行设置。如果输入无效，比如输入了 0，程序会保存完整的计算结果。"


/* 窗口信息 */
#define TITLE			""
#define CLASSNAME		"cut_window_class"
#define WIDTH			800
#define HEIGHT			350

/* 控件 menuid */
#define ID_LABEL_PRE	1000
#define ID_LABEL_POST	1001
#define ID_OPEN			2000
#define ID_SAVE			2001
#define ID_CALC			2002
#define ID_HELP			2003
#define ID_TEXT			3000
#define ID_SSE			4000
#define ID_LIST			5000


/* 控件句柄 */
static HWND hwndOpen, hwndSave, hwndCalc, hwndHelp;
static HWND hwndText, hwndSSE, hwndList;



/* 窗口界面绘制 */
static void draw(HWND hwnd) {
	size_t	i;
	size_t	ids[] = {ID_LABEL_PRE, ID_LABEL_POST, ID_OPEN, ID_SAVE, ID_CALC, ID_HELP, ID_TEXT, ID_SSE, ID_LIST};

	/* 列表分栏标题和宽度 */
	char	*title[] = {"数字", "表达式", "乘积", "乘积位数", "使用内存", "计算耗时", "转换耗时", "总耗时"};
	size_t	 width[] = {85, 120, 145, 70, 95, 75, 75, 75};

	RECT	rect;
	HDC		hdc;
	HFONT	hfont;

	LVCOLUMN column;
	ZeroMemory(&column, sizeof column);

	/* 获取可绘制区 */
	GetClientRect(hwnd, &rect);

	/* 提示文本 */
	CreateWindowEx(0, TEXT("STATIC"), TEXT("保存"),		WS_CHILD | WS_VISIBLE,  15, rect.bottom - 30, 30, 25, hwnd, (HMENU) ID_LABEL_PRE,	instance, 0);
	CreateWindowEx(0, TEXT("STATIC"), TEXT("位数字"),		WS_CHILD | WS_VISIBLE, 100, rect.bottom - 30, 50, 25, hwnd, (HMENU) ID_LABEL_POST,	instance, 0);
	/* 绘制按钮 */
	hwndOpen	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("导入数据"),	 WS_CHILD | WS_VISIBLE, 10,					10,					90, 28, hwnd, (HMENU) ID_OPEN, instance, 0);
	hwndSave	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("保存结果"),	 WS_CHILD | WS_VISIBLE, 105,				10,					90, 28, hwnd, (HMENU) ID_SAVE, instance, 0);
	hwndCalc	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("开始计算"),	 WS_CHILD | WS_VISIBLE, rect.right - 100,	10,					90, 28, hwnd, (HMENU) ID_CALC, instance, 0);
	hwndHelp	= CreateWindowEx(0, TEXT("BUTTON"), TEXT("使用帮助"),	 WS_CHILD | WS_VISIBLE, rect.right - 100,	rect.bottom - 38,	90, 28, hwnd, (HMENU) ID_HELP, instance, 0);
	/* 绘制输入框 */
	hwndText	= CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 45, rect.bottom - 33, 50, 22, hwnd, (HMENU) ID_TEXT, instance, 0);
	/* 绘制复选框 */
	hwndSSE		= CreateWindowEx(0, TEXT("BUTTON"), TEXT("使用 SSE2 指令对计算进行优化"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 210, rect.bottom - 35, 190, 25, hwnd, (HMENU) ID_SSE, instance, 0);
	/* 绘制结果列表 */
	hwndList	= CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, 10, 45, rect.right - 20, rect.bottom - 90, hwnd, (HMENU) ID_LIST, instance, 0);

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

	/* 列表列样式 */
	column.fmt	= LVCFMT_LEFT;
	column.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	/* 为结果列表添加列 */
	for(i = 0; i < (sizeof title / sizeof(char *)); i++) {
		column.pszText	= title[i];
		column.cx		= width[i];

		ListView_InsertColumn(hwndList, i, &column);
	}

	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	/* 禁用按钮 */
	saveEnable(0);
	calcEnable(0);

	/* SSE 选项 */
	if(cpuSSE()) {
		SendMessage(hwndSSE, BM_SETCHECK, 1, 0);
	} else {
		sseEnable(0);
	}
}

/* 按钮事件 */
static void command(WPARAM wparam, LPARAM lparam) {
	/* 控件 ID */
	WORD id = LOWORD(wparam);

	switch(id) {
		case ID_OPEN:	input();		break;
		case ID_SAVE:	output();		break;
		case ID_CALC:	dispatch();		break;
		case ID_HELP:	NOTICE(HELP);	break;
	}
}

/* 窗口消息循环 */
static LRESULT CALLBACK message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		/* 绘制窗口 */
		case WM_CREATE:
			draw(hwnd); return 0;

		/* 关闭窗口 */
		case WM_CLOSE:
			if(close() == 0) {
				return 0;
			}
			break;

		/* 销毁窗口 */
		case WM_DESTROY:
			PostQuitMessage(0); return 0;

		/* 命令分发 */
		case WM_COMMAND:
			command(wparam, lparam); break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}



void gui() {
	MSG						msg;
	RECT					rect;
	WNDCLASS				wndclass;
	INITCOMMONCONTROLSEX	iccx;

	/* 获取桌面大小 */
	GetWindowRect(GetDesktopWindow(), &rect);

	ZeroMemory(&iccx, sizeof iccx);
	ZeroMemory(&wndclass, sizeof wndclass);

	iccx.dwSize	= sizeof iccx;
	iccx.dwICC	= ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;

	/* 初始化 ListView */
	InitCommonControlsEx(&iccx);

	wndclass.hInstance		= instance;
	wndclass.lpszClassName	= TEXT(CLASSNAME);
	wndclass.lpfnWndProc	= &message;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;

	/* 注册窗口类 */
	if(RegisterClass(&wndclass) == 0) {
		DIE("注册窗口类失败");
	}

	/* 创建主窗口 */
	if((win = CreateWindowEx (
			/* ex style */	0,
			/* classname */	CLASSNAME,
			/* title */		TITLE,
			/* style */		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			/* left */		(rect.right - WIDTH) / 2,
			/* top */		(rect.bottom - HEIGHT) / 2,
			/* width */		WIDTH,
			/* height */	HEIGHT,
			/* parent */	NULL,
			/* menu */		NULL,
			/* instance */	instance,
			/* lpparam */	0
		)) == NULL) {
			DIE("创建图形窗口失败");
	}

	ShowWindow(win, SW_SHOW);
	UpdateWindow(win);

	while(GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


/* 启用或禁用 导入数据 按钮 */
void openEnable(int enable) {
	EnableWindow(hwndOpen, (BOOL) enable);
}

/* 启用或禁用 保存结果 按钮 */
void saveEnable(int enable) {
	EnableWindow(hwndSave, (BOOL) enable);
}

/* 启用或禁用 开始计算/终止计算 按钮 */
void calcEnable(int enable) {
	EnableWindow(hwndCalc, (BOOL) enable);
}

/* 设置 开始计算/终止计算 按钮的文字 */
void calcText(char *text) {
	SetWindowText(hwndCalc, TEXT(text));
}

/* 获取 输出位数 */
size_t textRead() {
	int		input;
	char	buffer[16];

	ZeroMemory(buffer, sizeof buffer);
	GetWindowText(hwndText, buffer, sizeof buffer);

	if(sscanf(buffer, "%d", &input) < 1) {
		return 0;
	}
	if(input < 0) {
		return 0;
	}

	return (size_t) input;
}

/* 启用或禁用 使用SSE 选项 */
void sseEnable(int enable) {
	if(cpuSSE() == 0) {
		enable = 0;
	}

	EnableWindow(hwndSSE, (BOOL) enable);
}

/* 获取是否选择了 使用SSE */
int sseChecked() {
	if(cpuSSE()) {
		if(Button_GetCheck(hwndSSE) == BST_CHECKED) {
			return 1;
		}
	}

	return 0;
}

/* 向列表中添加项 */
void listAppend(number_s *number) {
	int		index;
	LVITEM	item;

	ZeroMemory(&item, sizeof item);

	item.mask	= LVIF_TEXT;
	item.iItem	= ListView_GetItemCount(hwndList);

	if((index = ListView_InsertItem(hwndList, &item)) == -1) {
		DIE("添加数据项失败");
	}

	/* 数字 */
	listSet(index, 0, "%lu", number->number);

	/* 表达式 */
	if(number->remainder == 0) {
		listSet(index, 1, "3 * %lu", number->exponent);
	} else {
		listSet(index, 1, "%lu + 3 * %lu", number->remainder, number->exponent);
	}

	/* 乘积 */
	listSet(index, 2, "?");
	/* 积位数 */
	listSet(index, 3, "?");
	/* 内存占用 */
	listSet(index, 4, "?");
	/* 时间 */
	listSet(index, 5, "-");
	listSet(index, 6, "-");
	listSet(index, 7, "-");
}

/* 设置某项某列的文字 */
void listSet(int_t index, int_t column, char *format, ...) {
	char *text;

	va_list arg;
	va_start(arg, format);

	/* 文字缓冲区 */
	NEW(text, char, _vscprintf(format, arg) + 1);
	/* 生成文字 */
	vsprintf(text, format, arg);

	ListView_SetItemText(hwndList, index, column, text);

	va_end(arg);
	DEL(text);
}

/* 清除列表 */
void listClear() {
	ListView_DeleteAllItems(hwndList);
}
