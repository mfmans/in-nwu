/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui_command.c, 0.1.1027
*/

#include <Windows.h>
#include <process.h>
#include "base.h"
#include "process.h"


extern HWND winHwnd;

extern HWND buttonExecuteHwnd, buttonDumpHwnd;
extern HWND buttonSuspendHwnd, buttonRunBpHwnd, buttonRunModuleHwnd, buttonRunSingleHwnd;
extern HWND buttonRegisterHwnd, buttonConfigHwnd, buttonAboutHwnd;

extern HWND textRvaHwnd, textEipHwnd, textSectionHwnd, textCodeHwnd;
extern HWND checkDisasmHwnd;

extern HWND textWorkingHwnd;

extern process_h PROCESS;


void cmdEnable(BOOL enable) {
	EnableWindow(buttonDumpHwnd,		enable);
	EnableWindow(buttonSuspendHwnd,		enable);
	EnableWindow(buttonRunBpHwnd,		enable);
	EnableWindow(buttonRunModuleHwnd,	enable);
	EnableWindow(buttonRunSingleHwnd,	enable);
	EnableWindow(buttonRegisterHwnd,	enable);
	EnableWindow(buttonConfigHwnd,		enable);

	EnableWindow(checkDisasmHwnd,		enable);
}

void cmdExecuteEnable(BOOL enable) {
	EnableWindow(buttonExecuteHwnd, enable);
}


void cmdInit() {
	bpInit();
	sectionInit();
	importInit();
	moduleInit();

	cmdEnable(FALSE);
}


void cmdExecute() {
	char *file;

	if(PROCESS.file == NULL) {
		if((file = SelectFileOpen("exe", "请选择需要载入的 PE 文件")) == NULL) {
			return;
		}

		/* 创建线程 */
		th_run(Process, file);
	} else {

	}
}

void cmdDump() {

}

void cmdSuspend() {

}


void cmdRunBp() {

}

void cmdRunModule() {

}

void cmdRunSingle() {

}

void cmdRunRegister() {

}


void cmdConfig() {

}

void cmdAbout() {
	MessageBox(winHwnd, "Ananke PE Loader \n\n这是一个易用的、图形化的 PE 文件加载调试工具，拥有启发式的代码\n分析引擎，可用于对 EXE 程序文件进行调试和脱壳等。\n\n程序的作者为 mfboy，当前正在使用的版本是 v0.1.1027。", "关于", MB_OK | MB_ICONINFORMATION);
}


void cmdAddress(DWORD rva, DWORD eip, char *section, char *code) {
	char buffer[10];

	sprintf(buffer, "%08X", eip);
	SetWindowText(textEipHwnd, buffer);

	if(rva == 0) {
		SetWindowText(textRvaHwnd, "-");
	} else {
		sprintf(buffer, "%08X", rva);
		SetWindowText(textRvaHwnd, buffer);
	}

	if(section == NULL) {
		SetWindowText(textSectionHwnd, "-");
	} else {
		SetWindowText(textSectionHwnd, section);
	}

	if(code == NULL) {
		SetWindowText(textCodeHwnd, "");
	} else {
		SetWindowText(textCodeHwnd, section);
	}
}


void cmdWorking(int append, char *format, ...) {
	int i, length;

	char *text;
	char buffer[1024];

	va_list arg;
	va_start(arg, format) ;

	wvsprintf(buffer, format, arg);

	if(append > 0) {
		if(length = GetWindowTextLength(textWorkingHwnd)) {
			mem_alloc(text, char, length + strlen(buffer) + append * 2 + 1);

			GetWindowText(textWorkingHwnd, text, length);

			for(i = 0; i < append; i++) {
				strcat(text, "\r\n");
			}

			strcat(text, buffer);

			SetWindowText(textWorkingHwnd, text);
			SendMessage(textWorkingHwnd, EM_SCROLLCARET, 0, 0);

			mem_delete(text);

			return;
		}
	}

	SetWindowText(textWorkingHwnd, buffer);
}
