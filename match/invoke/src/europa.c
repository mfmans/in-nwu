/*
	$ Europa Win32 GUI Interface  (C) 2005-2012 mfboy
	$ europa.c, 0.1.1104
*/

#pragma warning (disable: 4996)
#pragma comment (lib, "ComCtl32.lib")
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define _EUROPA_

#include <Windows.h>
#include <Commctrl.h>
#include "europa.h"


/* caption() 方法缓冲区 */
#define WINDOW_BUFFER		1024

/* setText() 方法缓冲区大小 */
#define EDIT_BUFFER			1024
#define EDITAREA_BUFFER		4096


/* this 指针 */
erp_object_s *erpThis = NULL;
/* 当前窗口 */
erp_window_s *window = NULL;
/* 退出时的数据 */
static void *quit = NULL;


/* 窗口生成 */
static void erpCreate(HWND hwnd) {
	size_t i;

	HDC		hdc;
	HFONT	font;
	RECT	rect;

	char	*element;
	DWORD	style;
	BOOL	extension;

	LVCOLUMN column;

	erp_size_t left, top;
	erp_object_s *obj;

	if(window->object == NULL) {
		return;
	}
	if((obj = window->object->next) == NULL) {
		return;
	}

	GetClientRect(hwnd, &rect);

	while(obj) {
		style		= WS_CHILD | WS_VISIBLE;
		extension	= FALSE;

		if(obj->x < 0) {
			left = (size_t) (obj->x + rect.right - obj->width);
		} else {
			left = obj->x;
		}
		if(obj->y < 0) {
			top = (size_t) (obj->y + rect.bottom - obj->height);
		} else {
			top = obj->y;
		}

		switch(obj->type) {
			case LABEL:
				element = "STATIC"; break;

			case BUTTON:
				element = "BUTTON"; break;

			case EDIT:
				element		= "EDIT";
				extension	= TRUE;
				style		= style | ES_AUTOHSCROLL;

				break;

			case EDITAREA:
				element		= "EDIT";
				extension	= TRUE;
				style		= style | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
				
				break;

			case RADIO:
				element	= "BUTTON";
				style	= style | BS_RADIOBUTTON;

				break;

			case CHECK:
				element	= "BUTTON";
				style	= style | BS_AUTOCHECKBOX;

				break;

			case COMBO:
				element		= "COMBOBOX";
				extension	= TRUE;
				style		= style | WS_VSCROLL | CBS_DROPDOWNLIST;

				break;

			case LIST:
				element		= WC_LISTVIEW;
				extension	= TRUE;
				style		= style | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;

				break;
		}

		if(extension) {
			obj->hwnd = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				element, obj->text, style,
				left, top, obj->width, obj->height,
				hwnd, (HMENU) obj->id, window->instance, NULL
			);
		} else {
			obj->hwnd = CreateWindow (
				element, obj->text, style,
				left, top, obj->width, obj->height,
				hwnd, (HMENU) obj->id, window->instance, NULL
			);
		}

		/* 附加属性 */
		switch(obj->type) {
			case LIST:
				ZeroMemory(&column, sizeof column);

				column.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
				column.fmt	= LVCFMT_LEFT;

				for(i = 0; i < P_LIST(obj)->colCount; i++) {
					column.pszText	= P_LIST(obj)->colTitle[i];
					column.cx		= P_LIST(obj)->colWidth[i];

					ListView_InsertColumn(obj->hwnd, i, &column);
				}

				ListView_SetExtendedListViewStyle(obj->hwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

				break;
		}

		obj = obj->next;
	}

	hdc		= GetDC(hwnd);
	font	= (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	obj		= window->object->next;

	SelectObject(hdc, font);
	ReleaseDC(hwnd, hdc);

	while(obj) {
		SendDlgItemMessage(hwnd, obj->id, WM_SETFONT, (WPARAM) font, 0);

		obj = obj->next;
	}
}

/* 命令分发 */
static void erpCommand(WPARAM wparam, LPARAM lparam) {
	int i;
	WORD id = LOWORD(wparam);

	erp_object_s *obj	= NULL;
	erp_object_s *next	= window->object->next;

	while(next) {
		if(next->id == id) {
			obj = next; break;
		}

		next = next->next;
	}

	if(obj == NULL) {
		return;
	}
	if(IsWindowEnabled(obj->hwnd) == FALSE) {
		return;
	}

	switch(obj->type) {
		case BUTTON:
			BUTTON(obj)->click(); break;
		case RADIO:
			RADIO(obj)->select(); break;
		case CHECK:
			CHECK(obj)->check(-1); break;

		case COMBO:
			if(HIWORD(wparam) == CBN_SELCHANGE) {
				i = SendMessage(obj->hwnd, CB_GETCURSEL, 0, 0);

				if(i != COMBO(obj)->selected) {
					COMBO(obj)->selected = i;

					if(erpThis->callback) {
						erpThis->callback(window, obj, SELECT, i);
					}
				}
			}

			break;
	}
}

/* 通知处理 */
static void erpNotify(LPARAM lparam) {
	NMHDR			*nm		= (NMHDR *) lparam;
	LPNMLISTVIEW	list	= NULL;

	erp_object_s	*obj	= NULL;
	erp_object_s	*next	= window->object->next;

	while(next) {
		if((next->type == LIST) && (next->id == nm->idFrom)) {
			obj = next; break;
		}

		next = next->next;
	}

	if(obj == NULL) {
		return;
	}

	switch(nm->code) {
		case LVN_ITEMCHANGED:
			list = (LPNMLISTVIEW) lparam;

			if((list->uChanged & LVIF_STATE) && ((list->uNewState & LVIS_SELECTED) != (list->uOldState & LVIS_SELECTED))) {
				if(list->uNewState & LVIS_SELECTED) {
					P_LIST(obj)->selected = list->iItem;

					if(obj->callback) {
						obj->callback(window, obj, SELECT, list->iItem);
					}
				} else {
					P_LIST(obj)->selected = -1;

					if(obj->callback) {
						obj->callback(window, obj, SELECT, -1);
					}
				}
			}

			break;

		case NM_SETFOCUS:
			if(obj->callback) {
				obj->callback(window, obj, FOCUS, 1);
			}

			break;

		case NM_KILLFOCUS:
			if(obj->callback) {
				obj->callback(window, obj, FOCUS, 0);
			}

			break;
	}
}


/* 消息循环回调函数 */
static LRESULT CALLBACK erpCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if(window->loop) {
		window->loop(window, hwnd, msg, wparam, lparam);
	}

	switch(msg) {
		case WM_CREATE:
			erpCreate(hwnd);

			if(window->draw) {
				window->hwnd = hwnd;

				window->draw(window);
			}

			window->s.initiated = TRUE;

			return 0;

		case WM_CLOSE:
			if(window->destroy) {
				window->cancel = FALSE;
				quit = window->destroy(window, window->s.quit);

				/* 取消关闭 */
				if(window->cancel) {
					window->s.quit = 0;

					return 0;
				}
			}

			break;

		case WM_DESTROY:
			PostQuitMessage(0); return 0;

		case WM_COMMAND:
			erpCommand(wparam, lparam); break;

		case WM_NOTIFY:
			erpNotify(lparam); break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void erpInit() {
	INITCOMMONCONTROLSEX iccx;

	iccx.dwSize	= sizeof iccx;
	iccx.dwICC	= ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;

	/* 初始化 ListView */
	InitCommonControlsEx(&iccx);
}

void * erpWindow(erp_window_s *win) {
	MSG			msg;
	RECT		rect;
	WNDCLASS	wndclass;

	DWORD		left, top;
	DWORD		style;

	erp_object_s	*obj, *next;
	erp_window_s	*oldwin = window;

	/* 修改全局变量 */
	window	= win;
	quit	= NULL;

	/* 窗口位置 */
	if(win->center) {
		GetWindowRect(GetDesktopWindow(), &rect);

		left	= (rect.right - win->width) / 2;
		top		= (rect.bottom - win->height) / 2;
	} else {
		left	= CW_USEDEFAULT;
		top		= CW_USEDEFAULT;
	}

	/* 样式 */
	if(win->tool) {
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	} else {
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	/* 清理标记 */
	ZeroMemory(&window->s, sizeof window->s);

	/* 设置功能函数 */
	win->quit		= erpWindowQuit;
	win->hidden		= erpWindowHidden;
	win->disable	= erpWindowDisable;
	win->caption	= erpWindowCaption;

	ZeroMemory(&wndclass, sizeof wndclass);

	wndclass.hInstance		= win->instance;
	wndclass.lpszClassName	= win->classname;
	wndclass.lpfnWndProc	= &erpCallback;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;

	RegisterClass(&wndclass);

	if((win->hwnd = CreateWindow (
		win->classname,
		win->title,
		style,
		left, top,
		win->width,
		win->height,
		win->parent,
		NULL,
		win->instance,
		NULL)) == NULL) {
			return NULL;
	}

	/* 设置父窗口挂起 */
	if(win->parent) {
		EnableWindow(win->parent, FALSE);
	}

	ShowWindow(win->hwnd, SW_SHOW);
	UpdateWindow(win->hwnd);

	while(GetMessage(&msg, 0, 0, 0) > 0) {
		if(msg.message == WM_KEYDOWN) {
			if(win->keyboard) {
				win->keyboard(win, (DWORD) msg.wParam);
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* 清理对象 */
	if(obj = win->object) {
		while(obj) {
			next = obj->next;

			free(obj);

			obj = next;
		}
	}

	/* 恢复调用层次 */
	window = oldwin;

	/* 清理信息 */
	win->hwnd			= NULL;
	win->object			= NULL;
	win->s.initiated	= FALSE;

	/* 恢复父窗口 */
	if(win->parent) {
		EnableWindow(win->parent, TRUE);

		SetFocus(win->parent);
	}

	return quit;
}

erp_object_s * erpDraw(erp_window_s *win, erp_object_e type, WORD id, void *callback, erp_loc_t x, erp_loc_t y, erp_size_t width, erp_size_t height, ...) {
	va_list arg;
	erp_object_s *obj, *next;

	if(win == NULL) {
		return NULL;
	}
	if(win->object == NULL) {
		if((win->object = (erp_object_s *) calloc(1, sizeof(erp_object_s))) == NULL) {
			return NULL;
		}
	}

	switch(type) {
		case LABEL:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_label_s)); break;
		case BUTTON:	obj = (erp_object_s *) calloc(1, sizeof(erp_object_button_s)); break;
		case EDIT:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_edit_s)); break;
		case EDITAREA:	obj = (erp_object_s *) calloc(1, sizeof(erp_object_editarea_s)); break;
		case RADIO:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_radio_s)); break;
		case CHECK:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_check_s)); break;
		case COMBO:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_combo_s)); break;
		case LIST:		obj = (erp_object_s *) calloc(1, sizeof(erp_object_list_s)); break;
	}

	/* 申请内存失败 */
	if(obj == NULL) {
		return NULL;
	}

	/* 不定参数表 */
	va_start(arg, height);

	obj->type		= type;
	obj->id			= id;
	obj->x			= x;
	obj->y			= y;
	obj->width		= width;
	obj->height		= height;
	obj->hidden		= FALSE;
	obj->disabled	= FALSE;
	obj->callback	= (void (*)(erp_window_s *, erp_object_s *, erp_event_e, DWORD)) callback;

	obj->hide		= erpObjectHide;
	obj->disable	= erpObjectDisable;
	obj->caption	= erpObjectCaption;

	/* 处理额外属性 */
	switch(type) {
		case LABEL:
		case BUTTON:
		case EDIT:
		case EDITAREA:
		case RADIO:
		case CHECK:
			obj->text = va_arg(arg, char *);
	}

	/* 实例化 */
	switch(type) {
		case BUTTON:
			P_BUTTON(obj)->click = erpButtonClick;

			break;

		case EDIT:
			P_EDIT(obj)->readonly	= erpEditReadonly;
			P_EDIT(obj)->setText	= erpEditSetText;
			P_EDIT(obj)->getText	= erpEditGetText;

			break;

		case EDITAREA:
			P_EDITAREA(obj)->readonly	= erpEditReadonly;
			P_EDITAREA(obj)->setText	= erpEditSetText;
			P_EDITAREA(obj)->appendText	= erpEditareaAppendText;
			P_EDITAREA(obj)->getText	= erpEditGetText;

			break;

		case RADIO:
			P_RADIO(obj)->group		= va_arg(arg, int);
			P_RADIO(obj)->select	= erpRadioSelect;

			break;

		case CHECK:
			P_CHECK(obj)->check = erpCheckCheck;

			break;

		case COMBO:
			P_COMBO(obj)->append		= erpComboAppend;
			P_COMBO(obj)->remove		= erpComboRemove;
			P_COMBO(obj)->clear			= erpComboClear;
			P_COMBO(obj)->select		= erpComboSelect;

			break;

		case LIST:
			P_LIST(obj)->colTitle	= va_arg(arg, char **);
			P_LIST(obj)->colWidth	= va_arg(arg, size_t *);
			P_LIST(obj)->colCount	= va_arg(arg, size_t);
			P_LIST(obj)->selected	= -1;
			P_LIST(obj)->append		= erpListAppend;
			P_LIST(obj)->modify		= erpListModify;
			P_LIST(obj)->remove		= erpListRemove;
			P_LIST(obj)->clear		= erpListClear;
			P_LIST(obj)->select		= erpListSelect;

			break;
	}

	/* 添加到链表中 */
	next = win->object;

	while(next->next) {
		next = next->next;
	}

	next->next = obj;

	va_end(arg);

	return obj;
}


static void erpWindowQuit(DWORD arg) {
	window->s.quit = arg;

	SendMessage(window->hwnd, WM_CLOSE, 0, 0);
}

static void erpWindowHidden(BOOL hidden) {
	if(hidden) {
		ShowWindow(window->hwnd, SW_HIDE);
	} else {
		ShowWindow(window->hwnd, SW_SHOW);
	}
}

static void erpWindowDisable(BOOL disable) {
	if(disable) {
		EnableWindow(window->hwnd, FALSE);
	} else {
		EnableWindow(window->hwnd, TRUE);
	}
}

static void erpWindowCaption(char *format, ...) {
	va_list	arg;
	char	buffer[WINDOW_BUFFER] = {0};

	va_start(arg, format);

	wvsprintf(buffer, format, arg);

	SetWindowText(window->hwnd, buffer);
}


static void erpObjectHide(BOOL hidden) {
	if(hidden == FALSE) {
		ShowWindow(erpThis->hwnd, SW_SHOW);
	} else {
		ShowWindow(erpThis->hwnd, SW_HIDE);
	}

	erpThis->hidden = hidden;
}

static void erpObjectDisable(BOOL disabled) {
	EnableWindow(erpThis->hwnd, (disabled == TRUE) ? FALSE : TRUE);

	erpThis->disabled = disabled;
}

static void erpObjectCaption(char *text) {
	SetWindowText(erpThis->hwnd, text);

	erpThis->text = text;
}

static void erpButtonClick() {
	if(erpThis->disabled) {
		return;
	}

	if(erpThis->callback) {
		erpThis->callback(window, erpThis, CLICK, 0);
	}
}

static void erpEditReadonly(BOOL readonly) {
	SendMessage(erpThis->hwnd, EM_SETREADONLY, (readonly == FALSE) ? 0 : 1, 0);
}

static void erpEditSetText(char *format, ...) {
	va_list	arg;

	char	*buffer, *output;
	size_t	i, j, length;

	if(strcmp(format, "%s")) {
		if((buffer = (char *) calloc(1, EDIT_BUFFER)) == NULL) {
			return;
		}

		wvsprintf(buffer, format, va_start(arg, format));
	} else {
		va_start(arg, format);

		/* 第二个参数 */
		output = va_arg(arg, char *);
		length = strlen(output);

		va_end(arg);

		/* 换行计数 */
		for(i = j = 0; i < length; i++, j++) {
			if(output[i] == '\r') {
				j++;

				if(output[i + 1] == '\n') {
					i++;
				}
			} else if(output[i] == '\n') {
				j++;
			}
		}

		/* 临时空间 */
		if((buffer = (char *) calloc(1, j + 1)) == NULL) {
			return;
		}

		/* 转换换行标记 */
		for(i = j = 0; i < length; i++, j++) {
			if(output[i] == '\r') {
				buffer[j] = '\r';
				buffer[j + 1] = '\n';

				j++;

				if(output[i + 1] == '\n') {
					i++;
				}
			} else if(output[i] == '\n') {
				buffer[j] = '\r';
				buffer[j + 1] = '\n';

				j++;
			} else {
				buffer[j] = output[i];
			}
		}
	}

	erpObjectCaption(buffer);

	free(buffer);
}

static char * erpEditGetText() {
	size_t	length	= GetWindowTextLength(erpThis->hwnd);
	char	*buffer	= (char *) calloc(length + 1, sizeof(char));

	GetWindowText(erpThis->hwnd, buffer, length + 1);

	return buffer;
}

static void erpEditareaAppendText(char *format, ...) {
	va_list	arg;

	char	output[EDITAREA_BUFFER] = {0};
	char	*buffer = erpEditGetText();

	wvsprintf(output, format, va_start(arg, format));

	if(buffer = (char *) realloc(buffer, strlen(buffer) + strlen(output) + 1)) {
		strcat(buffer, output);

		erpObjectCaption(buffer);
	}

	free(buffer);
}

static void erpRadioSelect() {
	erp_object_s *next = window->object->next;

	if(erpThis->disabled) {
		return;
	}

	if(P_RADIO(erpThis)->selected) {
		return;
	} else {
		SendMessage(erpThis->hwnd, BM_SETCHECK, 1, 0);

		P_RADIO(erpThis)->selected = TRUE;

		if(erpThis->callback) {
			erpThis->callback(window, erpThis, SELECT, 1);
		}
	}

	if(window->s.initiated) {
		while(next) {
			if((next->type == RADIO) && (P_RADIO(next)->group == P_RADIO(erpThis)->group)) {
				if((next->id != erpThis->id) && P_RADIO(next)->selected) {
					SendMessage(next->hwnd, BM_SETCHECK, 0, 0);

					P_RADIO(next)->selected = FALSE;

					if(next->callback) {
						next->callback(window, next, SELECT, 0);
					}
				}
			}

			next = next->next;
		}
	}
}

static void erpCheckCheck(BOOL checked) {
	if(erpThis->disabled) {
		return;
	}

	if(checked < 0) {
		if(P_CHECK(erpThis)->checked == FALSE) {
			checked = TRUE;
		} else {
			checked = FALSE;
		}
	}

	if(checked == P_CHECK(erpThis)->checked) {
		return;
	}

	SendMessage(erpThis->hwnd, BM_SETCHECK, checked, 0);

	P_CHECK(erpThis)->checked = checked;
		
	if(erpThis->callback) {
		erpThis->callback(window, erpThis, SELECT, checked);
	}
}

static int erpComboAppend(char *text) {
	int index;

	P_COMBO(erpThis)->count++;

	index = SendMessage(erpThis->hwnd, CB_ADDSTRING, 0, (LPARAM) text);

	if(P_COMBO(erpThis)->selected != -1) {
		SendMessage(erpThis->hwnd, CB_SETCURSEL, P_COMBO(erpThis)->selected, 0);
	}

	return index;
}

static void erpComboRemove(int index) {
	P_COMBO(erpThis)->count--;

	SendMessage(erpThis->hwnd, CB_DELETESTRING, (WPARAM) index, 0);
}

static void erpComboClear() {
	P_COMBO(erpThis)->count = 0;

	SendMessage(erpThis->hwnd, CB_RESETCONTENT, 0, 0);
}

static void erpComboSelect(int index) {
	P_COMBO(erpThis)->selected = index;

	SendMessage(erpThis->hwnd, CB_SETCURSEL, index, 0);

	if(erpThis->callback) {
		erpThis->callback(window, erpThis, SELECT, index);
	}
}

static int erpListAppend(char **data) {
	int		index, i, j;
	LVITEM	item;

	ZeroMemory(&item, sizeof item);

	item.mask		= LVIF_TEXT;
	item.iItem		= ListView_GetItemCount(erpThis->hwnd);

	if((index = ListView_InsertItem(erpThis->hwnd, &item)) == -1) {
		return -1;
	}

	i = 0;
	j = P_LIST(erpThis)->colCount;

	for(; i < j; i++) {
		ListView_SetItemText(erpThis->hwnd, index, i, data[i]);
	}

	P_LIST(erpThis)->count++;

	return index;
}

static void erpListModify(int index, char **data) {
	int i;
	int j = P_LIST(erpThis)->colCount;

	for(i = 0; i < j; i++) {
		if(data[i]) {
			ListView_SetItemText(erpThis->hwnd, index, i, data[i]);
		}
	}
}

static void erpListRemove(int index) {
	ListView_DeleteItem(erpThis->hwnd, index);

	P_LIST(erpThis)->count--;

	if(P_LIST(erpThis)->selected == index) {
		P_LIST(erpThis)->selected = -1;
	}
}

static void erpListClear() {
	ListView_DeleteAllItems(erpThis->hwnd);

	P_LIST(erpThis)->count		= 0;
	P_LIST(erpThis)->selected	= -1;
}

static void erpListSelect(int index) {
	P_LIST(erpThis)->selected = index;

	SetFocus(erpThis->hwnd);

	if(index < 0) {
		ListView_SetItemState(erpThis->hwnd, 0, 0, LVIS_SELECTED);
	} else {
		ListView_SetItemState(erpThis->hwnd, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}
