/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui_module.c, 0.1.1027
*/

#include <Windows.h>
#include "base.h"


extern HWND listModuleHwnd;
extern HWND buttonModuleExportHwnd, buttonModuleImportHwnd, buttonModuleBpHwnd;

static BOOL listModuleEnable;


void moduleInit() {
	int width[]		= {305, 60, 60};
	char *column[]	= {"模块名称", "起始地址", "大小"};

	guiListDraw(listModuleHwnd, sizeof width / sizeof(int), column, width);

	moduleEnable(FALSE);
}

void moduleEnable(BOOL enable) {
	EnableWindow(buttonModuleExportHwnd, enable);
	EnableWindow(buttonModuleImportHwnd, enable);
	EnableWindow(buttonModuleBpHwnd, enable);
}

void moduleSelect(int index) {
	if(listModuleEnable == FALSE) {
		return;
	}


}

void modulePush(module_s *module) {
	char buffer[2][16] = {0};
	char *data[3] = {module->path, buffer[0], buffer[1]};

	sprintf(data[1], "%08X", module->address);

	if(module->size) {
		sprintf(data[2], "%lu", module->size);
	} else {
		data[2][0] = '-';
	}

	guiListPush(listModuleHwnd, 3, data);
}
