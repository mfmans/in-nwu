/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui_section.c, 0.1.1027
*/

#include <stdio.h>
#include <Windows.h>
#include "base.h"


extern HWND listSectionHwnd;
extern HWND buttonSectionExportHwnd, buttonSectionImportHwnd, buttonSectionBpHwnd, buttonSectionAllocHwnd;

static BOOL listSectionEnable;


void sectionInit() {
	int width[]		= {100, 70, 70, 70, 65, 50};
	char *column[]	= {"区段", "内存地址", "RVA", "磁盘位置", "大小", "所有者"};

	guiListDraw(listSectionHwnd, sizeof width / sizeof(int), column, width);

	sectionEnable(FALSE);
}

void sectionEnable(BOOL enable) {
	EnableWindow(buttonSectionExportHwnd, enable);
	EnableWindow(buttonSectionImportHwnd, enable);
	EnableWindow(buttonSectionBpHwnd, enable);
	EnableWindow(buttonSectionAllocHwnd, enable);
}

void sectionSelect(int index) {
	if(listSectionEnable == FALSE) {
		return;
	}


}

void sectionPush(section_s *section) {
	char buffer[4][16] = {0};
	char *data[6] = {NULL, buffer[0], buffer[1], buffer[2], buffer[3], NULL};

	if(section->name == NULL) {
		data[0] = "(匿名)";
	} else {
		data[0] = section->name;
	}

	if(section->address == 0) {
		sprintf(data[1], "N/A");
	} else {
		sprintf(data[1], "%08X", section->address);
	}

	sprintf(data[2], "%08X", section->virtualaddr);
	sprintf(data[3], "%08X", section->offset);
	sprintf(data[4], "%lu", section->size);

	switch(section->owner) {
		case SYSTEM:	data[5] = "调试器";	break;
		case PE:		data[5] = "程序";	break;
		case DYNAMIC:	data[5] = "动态申请";	break;
		case USER:		data[5] = "用户";	break;
	}

	section->index = guiListPush(listSectionHwnd, 6, data);
}

void sectionUpdate(section_s *section) {
	char buffer[16] = {0};
	char *data[2] = {NULL, buffer};

	sprintf(data[1], "%08X", section->address);

	guiListModify(listSectionHwnd, section->index, 2, data);
}
