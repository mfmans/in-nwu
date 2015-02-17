/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui.c, 0.1.1027
*/

#include <Windows.h>
#include <Commctrl.h>
#include "base.h"


#define LABEL(name, text, left, top, width, height)		CreateWindow("static", text, WS_CHILD | WS_VISIBLE, left, top, width, height, hwnd, (HMENU) LABEL_##name, NULL, NULL)
#define BUTTON(name, text, left, top, width, height)	CreateWindow("button", text, WS_CHILD | WS_VISIBLE, left, top, width, height, hwnd, (HMENU) BUTTON_##name, NULL, NULL)
#define CHECKBOX(name, text, left, top, width, height)	CreateWindow("button", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, left, top, width, height, hwnd, (HMENU) CHECK_##name, NULL, NULL)
#define RADIOBOX(name, text, left, top, width, height)	CreateWindow("button", text, WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, left, top, width, height, hwnd, (HMENU) RADIO_##name, NULL, NULL)
#define TEXTBOX(name, left, top, width, height)			CreateWindowEx(WS_EX_CLIENTEDGE, "edit", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, left, top, width, height, hwnd, (HMENU) TEXT_##name, NULL, NULL)
#define TEXTAREA(name, text, left, top, width, height)	CreateWindowEx(WS_EX_CLIENTEDGE, "edit", text, WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, left, top, width, height, hwnd, (HMENU) TEXT_##name, NULL, NULL)
#define LIST(name, left, top, width, height)			CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, left, top, width, height, hwnd, (HMENU) LIST_##name, NULL, NULL);


#define BUTTON_EXECUTE			1001
#define BUTTON_DUMP				1002
#define BUTTON_SUSPEND			1003
#define BUTTON_RUN_BP			1004
#define BUTTON_RUN_MODULE		1005
#define BUTTON_RUN_SINGLE		1006

#define BUTTON_REGISTER			1007
#define BUTTON_CONFIG			1008
#define BUTTON_ABOUT			1009

#define LABEL_RVA				1010
#define LABEL_EIP				1011
#define LABEL_SECTION			1012
#define LABEL_CODE				1013
#define TEXT_RVA				1014
#define TEXT_EIP				1015
#define TEXT_SECTION			1016
#define TEXT_CODE				1017
#define CHECK_DISASM			1018

#define LIST_BP					1019
#define LIST_SECTION			1020
#define LIST_IMPORT				1021
#define LIST_MODULE				1022

#define BUTTON_BP_ADDADDR		1023
#define BUTTON_BP_ADDRANGE		1024
#define BUTTON_BP_EDIT			1025
#define BUTTON_BP_DELETE		1026
#define CHECK_BP_DYNCODE		1027
#define CHECK_BP_DYNMEMORY		1028
#define CHECK_BP_DIFFSEC		1029
#define RADIO_BP_ALWAYS			1030
#define RADIO_BP_DIFFSEC		1031
#define RADIO_BP_RETURN			1032

#define BUTTON_SECTION_EXPORT	1033
#define BUTTON_SECTION_IMPORT	1034
#define BUTTON_SECTION_ALLOC	1035
#define BUTTON_SECTION_BP		1036

#define BUTTON_IMPORT_EDIT		1037
#define BUTTON_IMPORT_FIX		1038
#define BUTTON_IMPORT_BP		1039
#define BUTTON_IMPORT_REBUILD	1040

#define BUTTON_MODULE_EXPORT	1041
#define BUTTON_MODULE_IMPORT	1042
#define BUTTON_MODULE_BP		1043

#define TEXT_WORKING			1044


extern HWND winHwnd;

HWND buttonExecuteHwnd, buttonDumpHwnd;
HWND buttonSuspendHwnd, buttonRunBpHwnd, buttonRunModuleHwnd, buttonRunSingleHwnd;
HWND buttonRegisterHwnd, buttonConfigHwnd, buttonAboutHwnd;

HWND labelRvaHwnd, labelEipHwnd, labelSectionHwnd, labelCodeHwnd;
HWND textRvaHwnd, textEipHwnd, textSectionHwnd, textCodeHwnd;
HWND checkDisasmHwnd;

HWND listBpHwnd, listSectionHwnd, listImportHwnd, listModuleHwnd;

HWND buttonBpAddaddrHwnd, buttonBpAddrangeHwnd, buttonBpEditHwnd, buttonBpDeleteHwnd;
HWND checkBpDyncodeHwnd, checkBpDynmemoryHwnd, checkBpDiffsecHwnd;
HWND radioBpAlwaysHwnd, radioBpDiffsecHwnd, radioBpReturnHwnd;

HWND buttonSectionExportHwnd, buttonSectionImportHwnd, buttonSectionBpHwnd, buttonSectionAllocHwnd;

HWND buttonImportEditHwnd, buttonImportFixHwnd, buttonImportBpHwnd, buttonImportRebuildHwnd;

HWND buttonModuleExportHwnd, buttonModuleImportHwnd, buttonModuleBpHwnd;

HWND textWorkingHwnd;


void guiDraw(HWND hwnd) {
	int i;

	HDC		hdc;
	HFONT	font;

	buttonExecuteHwnd		= BUTTON	(EXECUTE,		"选择文件",				5,   5, 100, 28);
	buttonDumpHwnd			= BUTTON	(DUMP,			"导出内存镜像",			110, 5, 100, 28);

	buttonSuspendHwnd		= BUTTON	(SUSPEND,		"暂停进程 (F5)",			240, 5, 100, 28);
	buttonRunBpHwnd			= BUTTON	(RUN_BP,		"执行到下一个断点 (F8)",	345, 5, 140, 28);
	buttonRunModuleHwnd		= BUTTON	(RUN_MODULE,	"执行到返回程序领空 (F9)",	490, 5, 150, 28);
	buttonRunSingleHwnd		= BUTTON	(RUN_SINGLE,	"单步执行 (F7)",			645, 5, 100, 28);

	buttonRegisterHwnd		= BUTTON	(REGISTER,		"寄存器",					810, 5, 70, 28);
	buttonConfigHwnd		= BUTTON	(CONFIG,		"设置",					885, 5, 70, 28);
	buttonAboutHwnd			= BUTTON	(ABOUT,			"关于",					960, 5, 70, 28);

	labelRvaHwnd			= LABEL		(RVA,			"RVA：",				13,  42, 50, 14);
	labelEipHwnd			= LABEL		(EIP,			"EIP：",				143, 42, 50, 14);
	labelSectionHwnd		= LABEL		(SECTION,		"区段：",				293, 42, 50, 14);
	labelCodeHwnd			= LABEL		(CODE,			"数据指令：",			503, 42, 100, 14);

	textRvaHwnd				= TEXTBOX	(RVA,									10,  57, 120, 20);
	textEipHwnd				= TEXTBOX	(EIP,									140, 57, 120, 20);
	textSectionHwnd			= TEXTBOX	(SECTION,								290, 57, 180, 20);
	textCodeHwnd			= TEXTBOX	(CODE,									500, 57, 350, 20);

	checkDisasmHwnd			= CHECKBOX	(DISASM,		"反汇编 EIP 指向的数据",	870, 56, 150, 22);

	listBpHwnd				= LIST		(BP,									10,  90,  550, 280);
	listSectionHwnd			= LIST		(SECTION,								570, 90,  455, 150);
	listImportHwnd			= LIST		(IMPORT,								570, 285, 455, 180);
	listModuleHwnd			= LIST		(MODULE,								570, 510, 455, 110);

	buttonBpAddaddrHwnd		= BUTTON	(BP_ADDADDR,	"添加断点地址",			420, 375, 140, 26);
	buttonBpAddrangeHwnd	= BUTTON	(BP_ADDRANGE,	"添加断点范围",			420, 401, 140, 26);
	buttonBpEditHwnd		= BUTTON	(BP_EDIT,		"编辑",					420, 427, 70,  26);
	buttonBpDeleteHwnd		= BUTTON	(BP_DELETE,		"删除",					490, 427, 70,  26);

	checkBpDyncodeHwnd		= CHECKBOX	(BP_DYNCODE,	"执行被动态修改的代码时中断",	13, 381, 180, 22);
	checkBpDynmemoryHwnd	= CHECKBOX	(BP_DYNMEMORY,	"跳转到动态申请内存中时中断",	13, 403, 180, 22);
	checkBpDiffsecHwnd		= CHECKBOX	(BP_DIFFSEC,	"跨区段跳转时中断",			13, 425, 180, 22);

	radioBpAlwaysHwnd		= RADIOBOX	(BP_ALWAYS,		"总是检查代码的改动",			205, 381, 190, 22);
	radioBpDiffsecHwnd		= RADIOBOX	(BP_DIFFSEC,	"跨区段跳转时检查代码的改动",	205, 403, 190, 22);
	radioBpReturnHwnd		= RADIOBOX	(BP_RETURN,		"返回程序领空时检查代码的改动",	205, 425, 190, 22);

	buttonSectionExportHwnd	= BUTTON	(SECTION_EXPORT,	"导出",				955, 245, 70,  26);
	buttonSectionImportHwnd	= BUTTON	(SECTION_IMPORT,	"导入",				880, 245, 70,  26);
	buttonSectionBpHwnd		= BUTTON	(SECTION_BP,		"断点",				805, 245, 70,  26);
	buttonSectionAllocHwnd	= BUTTON	(SECTION_ALLOC,		"申请动态内存",		570, 245, 130, 26);

	buttonImportEditHwnd	= BUTTON	(IMPORT_EDIT,		"编辑入口",			925, 470, 100, 26);
	buttonImportFixHwnd		= BUTTON	(IMPORT_FIX,		"恢复入口",			820, 470, 100, 26);
	buttonImportBpHwnd		= BUTTON	(IMPORT_BP,			"断点",				745, 470, 70,  26);
	buttonImportRebuildHwnd	= BUTTON	(IMPORT_REBUILD,	"重建输入表",			570, 470, 120, 26);

	buttonModuleExportHwnd	= BUTTON	(MODULE_EXPORT,		"导出",				955, 625, 70, 26);
	buttonModuleImportHwnd	= BUTTON	(MODULE_IMPORT,		"导入",				880, 625, 70, 26);
	buttonModuleBpHwnd		= BUTTON	(MODULE_BP,			"断点",				805, 625, 70, 26);

	textWorkingHwnd			= TEXTAREA	(WORKING,		"准备就绪",				10, 465, 550, 185);

	hdc		= GetDC(hwnd);
	font	= (HFONT) GetStockObject(DEFAULT_GUI_FONT);

	SelectObject(hdc, font);
	ReleaseDC(hwnd, hdc);

	for(i = BUTTON_EXECUTE; i <= TEXT_WORKING; i++) {
		SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM) font, 0);
	}

	cmdInit();
}

void guiTitle(char *file, int status) {
	static char *format, *buffer;
	char *run, *pos;

	if(file == NULL) {
		if(format == NULL) {
			SetWindowText(winHwnd, WIN_TITLE); return;
		}
	} else {
		if(format != NULL) {
			mem_delete(format);
			mem_delete(buffer);
		}

		while((pos = strstr(file, "\\")) != NULL) {
			if(*(pos + 1) == 0) {
				break;
			}

			file = pos + 1;
		}

		mem_alloc(format, char, strlen(file) + strlen(WIN_TITLE) + 16);
		mem_alloc(buffer, char, strlen(file) + strlen(WIN_TITLE) + 64);

		strcat(format, file);
		strcat(format, "  [%s]  -  " WIN_TITLE);
	}

	switch(status) {
		case 0: run = "已终止"; break;
		case 1: run = "运行中"; break;
		case 2: run = "断点触发"; break;
		case 3: run = "已暂停"; break;

		default: run = "未知状态";
	}

	sprintf(buffer, format, run);

	SetWindowText(winHwnd, buffer);
}

void guiCommand(DWORD id) {
	int		i;
	BOOL	checked;

	HWND *hwnd			= NULL;
	HWND group[2]		= {NULL};

	void (*button)()	= NULL;
	void (*check)(BOOL)	= NULL;
	void (*radio)(HWND)	= NULL;

	switch(id) {
		case BUTTON_EXECUTE:		hwnd = &buttonExecuteHwnd;		button = cmdExecute;	break;
		case BUTTON_DUMP:			hwnd = &buttonDumpHwnd;			button = cmdDump;		break;
		case BUTTON_SUSPEND:		hwnd = &buttonSuspendHwnd;		button = cmdSuspend;	break;
		case BUTTON_RUN_BP:			hwnd = &buttonRunBpHwnd;		button = cmdRunBp;		break;
		case BUTTON_RUN_MODULE:		hwnd = &buttonRunModuleHwnd;	button = cmdRunModule;	break;
		case BUTTON_RUN_SINGLE:		hwnd = &buttonRunSingleHwnd;	button = cmdRunSingle;	break;
		case BUTTON_CONFIG:			hwnd = &buttonConfigHwnd;		button = cmdConfig;		break;
		case BUTTON_ABOUT:			hwnd = &buttonAboutHwnd;		button = cmdAbout;		break;

		case BUTTON_BP_ADDADDR:		break;
		case BUTTON_BP_ADDRANGE:	break;
		case BUTTON_BP_EDIT:		break;
		case BUTTON_BP_DELETE:		break;

		case BUTTON_SECTION_ALLOC:	break;
		case BUTTON_SECTION_BP:		break;
		case BUTTON_SECTION_IMPORT:	break;
		case BUTTON_SECTION_EXPORT:	break;

		case BUTTON_IMPORT_REBUILD:	break;
		case BUTTON_IMPORT_BP:		break;
		case BUTTON_IMPORT_FIX:		break;
		case BUTTON_IMPORT_EDIT:	break;

		case BUTTON_MODULE_BP:		break;
		case BUTTON_MODULE_IMPORT:	break;
		case BUTTON_MODULE_EXPORT:	break;

		case CHECK_BP_DYNCODE:		hwnd = &checkBpDyncodeHwnd;		check = bpCheckDyncode;		break;
		case CHECK_BP_DYNMEMORY:	hwnd = &checkBpDynmemoryHwnd;	check = bpCheckDynmemory;	break;
		case CHECK_BP_DIFFSEC:		hwnd = &checkBpDiffsecHwnd;		check = bpCheckDiffsec;		break;

		case RADIO_BP_ALWAYS:		hwnd = &radioBpAlwaysHwnd;		radio = bpRadio;	group[0] = radioBpDiffsecHwnd;	group[1] = radioBpReturnHwnd;	break;
		case RADIO_BP_DIFFSEC:		hwnd = &radioBpDiffsecHwnd;		radio = bpRadio;	group[0] = radioBpAlwaysHwnd;	group[1] = radioBpReturnHwnd;	break;
		case RADIO_BP_RETURN:		hwnd = &radioBpReturnHwnd;		radio = bpRadio;	group[0] = radioBpAlwaysHwnd;	group[1] = radioBpDiffsecHwnd;	break;
	}

	if(hwnd == NULL) {
		return;
	}
	if(IsWindowEnabled(*hwnd) == FALSE) {
		return;
	}

	if(button != NULL) {
		button();
	} else if(check != NULL) {
		checked = (BOOL) SendMessage(*hwnd, BM_GETCHECK, 0, 0);

		SendMessage(*hwnd, BM_SETCHECK, checked, 0);
		check(checked);
	} else if(radio != NULL) {
		if(SendMessage(*hwnd, BM_GETCHECK, 0, 0)) {
			return;
		}

		i = 0;

		while(group[i] != NULL) {
			SendMessage(group[i], BM_SETCHECK, 0, 0);

			i++;
		}

		SendMessage(*hwnd, BM_SETCHECK, 1, 0);
		radio(*hwnd);
	}
}

void guiNotify(LPARAM lParam) {
	void (*handle)(int) = NULL;

	NMHDR *nm = (NMHDR *) lParam;
	LPNMLISTVIEW list = NULL;

	switch(nm->idFrom) {
		case LIST_BP:		handle = bpSelect;		break;
		case LIST_SECTION:	handle = sectionSelect;	break;
		case LIST_IMPORT:	handle = importSelect;	break;
		case LIST_MODULE:	handle = moduleSelect;	break;

		default: return;
	}

	switch(nm->code) {
		case LVN_ITEMCHANGED:
			list = (LPNMLISTVIEW) lParam;

			if((list->uChanged & LVIF_STATE) && ((list->uNewState & LVIS_SELECTED) != (list->uOldState & LVIS_SELECTED))) {
				if(list->uNewState & LVIS_SELECTED) {
					handle(list->iItem);
				} else {
					handle(-1);
				}
			}

			break;

		case NM_KILLFOCUS:
			handle(-1); break;
	}
}

void guiKeyboard(DWORD key) {
	switch(key) {
		case VK_F5: guiCommand(BUTTON_SUSPEND); break;
		case VK_F7: guiCommand(BUTTON_RUN_SINGLE); break;
		case VK_F8: guiCommand(BUTTON_RUN_BP); break;
		case VK_F9: guiCommand(BUTTON_RUN_MODULE); break;
	}
}


void guiListDraw(HWND hwnd, size_t count, char **title, int *width) {
	size_t i;
	LVCOLUMN column;

	ZeroMemory(&column, sizeof column);

	column.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.fmt	= LVCFMT_LEFT;

	for(i = 0; i < count; i++) {
		column.cx		= width[i];
		column.pszText	= title[i];

		ListView_InsertColumn(hwnd, i, &column);
	}

	ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

void guiListClear(HWND hwnd) {
	ListView_DeleteAllItems(hwnd);
}

int guiListPush(HWND hwnd, size_t count, char **data) {
	int index;

	size_t i;
	LVITEM item;

	ZeroMemory(&item, sizeof item);

	item.mask		= LVIF_TEXT;
	item.iItem		= ListView_GetItemCount(hwnd);

	if((index = ListView_InsertItem(hwnd, &item)) == -1) {
		die("ListView 插入错误。");
	}

	for(i = 0; i < count; i++) {
		ListView_SetItemText(hwnd, index, i, data[i]);
	}

	return index;
}

void guiListModify(HWND hwnd, int index, size_t count, char **data) {
	size_t i;

	for(i = 0; i < count; i++) {
		if(data[i] != NULL) {
			ListView_SetItemText(hwnd, index, i, data[i]);
		}
	}
}

void guiListRemove(HWND hwnd, int index) {
	ListView_DeleteItem(hwnd, index);
}

void guiListSelect(HWND hwnd, int index) {
	SetFocus(hwnd);

	ListView_SetItemState(hwnd, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

int guiListSelected(HWND hwnd) {
	int i, j;

	j = ListView_GetItemCount(hwnd);

	for(i = 0; i < j; i++) {
		if(ListView_GetItemState(hwnd, i, LVIS_SELECTED) == LVIS_SELECTED) {
			return i;
		}
	}

	return -1;
}

