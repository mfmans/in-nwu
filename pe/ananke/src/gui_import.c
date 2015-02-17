/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui_import.c, 0.1.1027
*/

#include <Windows.h>
#include "base.h"


extern HWND listImportHwnd;
extern HWND buttonImportEditHwnd, buttonImportFixHwnd, buttonImportBpHwnd, buttonImportRebuildHwnd;

static BOOL listModuleEnable;


void importInit() {
	int width[]		= {85, 120, 65, 65, 90};
	char *column[]	= {"DLL", "函数", "当前入口", "实际入口", "IAT"};

	guiListDraw(listImportHwnd, sizeof width / sizeof(int), column, width);

	importEnable(FALSE);
}

void importEnable(BOOL enable) {
	EnableWindow(buttonImportEditHwnd, enable);
	EnableWindow(buttonImportFixHwnd, enable);
	EnableWindow(buttonImportBpHwnd, enable);
	EnableWindow(buttonImportRebuildHwnd, enable);
}

void importSelect(int index) {
	if(listModuleEnable == FALSE) {
		return;
	}


}

void importPush(import_s *import) {
	char buffer[4][32] = {0};
	char *data[5] = {NULL, buffer[0], buffer[1], buffer[2], buffer[3]};

	data[0] = import->dll;

	if(import->function == NULL) {
		sprintf(data[1], "# %lu", import->ordinal);
	} else {
		data[1] = import->function;
	}

	if(import->entryReal == 0) {
		data[2][0] = data[3][0] = '-';
	} else {
		sprintf(data[2], "%08X", import->entryTable);
		sprintf(data[3], "%08X", import->entryReal);
	}

	sprintf(data[4], "%08X / %08X", import->iatAddr, import->iatOffset);

	import->index = guiListPush(listImportHwnd, 5, data);
}

void importUpdate(import_s *import) {
	char buffer[2][16] = {0};
	char *data[4] = {NULL, NULL, buffer[0], buffer[1]};

	sprintf(data[2], "%08X", import->entryTable);
	sprintf(data[3], "%08X", import->entryReal);

	guiListModify(listImportHwnd, import->index, 4, data);
}
