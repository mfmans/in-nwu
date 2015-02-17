/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui_breakpoint.c, 0.1.1027
*/

#include <Windows.h>
#include "base.h"


extern HWND listBpHwnd;
extern HWND buttonBpAddaddrHwnd, buttonBpAddrangeHwnd, buttonBpEditHwnd, buttonBpDeleteHwnd;
extern HWND checkBpDyncodeHwnd, checkBpDynmemoryHwnd, checkBpDiffsecHwnd;
extern HWND radioBpAlwaysHwnd, radioBpDiffsecHwnd, radioBpReturnHwnd;

static BOOL listBpEnable;


void bpInit() {
	int width[]		= {30, 135, 135, 100, 120};
	char *column[]	= {"", "断点地址", "RVA", "区段", "数据"};

	guiListDraw(listBpHwnd, sizeof width / sizeof(int), column, width);

	SendMessage(checkBpDyncodeHwnd, BM_SETCHECK, 1, 0);
	SendMessage(checkBpDynmemoryHwnd, BM_SETCHECK, 1, 0);
	SendMessage(radioBpDiffsecHwnd, BM_SETCHECK, 1, 0);

	bpEnable(FALSE);
}

void bpEnable(BOOL enable) {
	listBpEnable = enable;

	EnableWindow(buttonBpAddaddrHwnd, enable);
	EnableWindow(buttonBpAddrangeHwnd, enable);
	EnableWindow(buttonBpEditHwnd, enable);
	EnableWindow(buttonBpDeleteHwnd, enable);

	EnableWindow(checkBpDyncodeHwnd, enable);
	EnableWindow(checkBpDynmemoryHwnd, enable);
	EnableWindow(checkBpDiffsecHwnd, enable);

	EnableWindow(radioBpAlwaysHwnd, enable);
	EnableWindow(radioBpDiffsecHwnd, enable);
	EnableWindow(radioBpReturnHwnd, enable);
}

void bpSelect(int index) {
	if(listBpEnable == FALSE) {
		return;
	}


}

void bpPush(bp_s *bp) {
	char buffer[2][24] = {0};
	char *data[] = {"", buffer[0], buffer[1], bp->section, ""};

	if(bp->addrStart == bp->addrEnd) {
		sprintf(buffer[0], "%08X", bp->addrStart);
	} else {
		sprintf(buffer[0], "%08X - %08X", bp->addrStart, bp->addrEnd);
	}

	if(bp->rvaStart == 0) {
		buffer[1][0] = '-';
	} else if(bp->rvaStart == bp->rvaEnd) {
		sprintf(buffer[1], "%08X", bp->rvaStart);
	} else {
		sprintf(buffer[1], "%08X - %08X", bp->rvaStart, bp->rvaEnd);
	}

	if(data[3] == NULL) {
		data[3] = "-";
	}

	bp->index = guiListPush(listBpHwnd, 5, data);
}

void bpUpdateTrigger(bp_s *bp, BOOL trigger) {
	char *data[1] = {NULL};

	if(trigger == TRUE) {
		data[0] = "=>";
	} else {
		data[0] = "";
	}

	guiListModify(listBpHwnd, bp->index, 1, data);
}


void bpCheckDyncode(BOOL checked) {

}

void bpCheckDynmemory(BOOL checked) {

}

void bpCheckDiffsec(BOOL checked) {

}

void bpRadio(HWND hwnd) {
	notify("Change!");
}
