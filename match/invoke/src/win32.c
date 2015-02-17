/*
	$ Invoke   (C) 2005-2012 mfboy
	$ win32.c, 0.1.1124
*/

#pragma comment (lib, "Winmm.lib")

#include <Windows.h>
#include <Psapi.h>
#include "base.h"

/* 错误报告程序路径缓冲区 */
#define REPORTER_BUFFER		1024
/* messageBoxFormat 缓冲区大小 */
#define MSGBOX_BUFFER		2048
/* OPENFILENAME.nMaxFile 大小 */
#define FILE_MAXPATH		1024

/* 声音文件 */
#define WAVEFILE			"notice.wav"


/* 同步信号 */
static int		objectArg		= 0;
static HANDLE	objectSymbolRun	= NULL;
static HANDLE	objectSymbolRet	= NULL;


void die(char *message) {
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	size_t	length;
	char	*command;
	char	reporter[REPORTER_BUFFER];

	do {
		if(length = GetModuleFileNameEx(RT.process, NULL, reporter, REPORTER_BUFFER)) {
			if(command = (char *) calloc(length + strlen(message) + 16, sizeof(char))) {
				break;
			}
		}

		/* 无法使用错误报告程序 */
		MessageBox(NULL, message, "", MB_ICONERROR | MB_OK);
		ExitProcess(EXIT_FAILURE);
	} while(0);

	strcat(command, "\"");
	strcat(command, reporter);
	strcat(command, "\" --error ");
	strcat(command, message);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb = sizeof(STARTUPINFO);

	CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	ExitProcess(EXIT_FAILURE);
}


void symbolInit() {
	objectSymbolRun = CreateEvent(NULL, TRUE, FALSE, NULL);
	objectSymbolRet = CreateEvent(NULL, TRUE, FALSE, NULL);
}

int symbolRun(int arg) {
	symbolRunAsyn(arg);

	/* 等待返回 */
	WaitForSingleObject(objectSymbolRet, INFINITE);
	ResetEvent(objectSymbolRet);

	return objectArg;
}

void symbolRunAsyn(int arg) {
	objectArg = arg;

	/* 发送信号 */
	SetEvent(objectSymbolRun);
}

int symbolWait() {
	WaitForSingleObject(objectSymbolRun, INFINITE);

	return objectArg;
}

void symbolReturn(int data) {
	objectArg = data;

	ResetEvent(objectSymbolRun);
	SetEvent(objectSymbolRet);
}


int uacAdminCheck() {
	int		admin	= 0;

	DWORD	length	= 0;
	HANDLE	token	= NULL;
	UINT16	winver	= LOWORD(GetVersion());

	TOKEN_ELEVATION elevation;

	/* 低于 vista */
	if(MAKEWORD(HIBYTE(winver), LOBYTE(winver)) < 0x0600) {
		return FALSE;
	}

	/* 查询信息 */
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		if(GetTokenInformation(token, (TOKEN_INFORMATION_CLASS) 20, &elevation, sizeof elevation, &length)) {
			if(length == sizeof elevation) {
				if(elevation.TokenIsElevated) {
					admin = 1;
				}
			}

			CloseHandle(token);
		}
	}

	return admin;
}

int messageBoxFormat(HWND win, unsigned int type, char *format, ...) {
	va_list	arg;
	char	buffer[MSGBOX_BUFFER];

	va_start(arg, format) ;
	wvsprintf(buffer, format, arg);

	return MessageBox(win, buffer, "", type);
}

void activeWindow(HWND win) {
	SetActiveWindow(win);
	SetForegroundWindow(win);
	SetFocus(win);

	/* 播放声音 */
	PlaySound(WAVEFILE, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}


char * selectFileOpen(HWND win, char *ext, char *title) {
	int		i, j, k;
	char	filter[128] = {0};

	j = strlen(ext);
	k = strlen(" 文件 (*.");

	for(i = 0; i < j; i++) {
		if((ext[i] >= 'a') && (ext[i] <= 'z')) {
			filter[i] = ext[i] & 0xDF;
		} else {
			filter[i] = ext[i];
		}
		
		if((ext[i] >= 'A') && (ext[i] <= 'Z')) {
			filter[i + j     + k    ] = ext[i] | 0x2F;
			filter[i + j * 2 + k + 4] = ext[i] | 0x2F;
		} else {
			filter[i + j     + k    ] = ext[i];
			filter[i + j * 2 + k + 4] = ext[i];
		}
	}

	memcpy(filter + j,			" 文件 (*.",	k);
	memcpy(filter + j * 2 + k,	") *.",		4);

	strcat(filter,				" 所有文件 (*.*) *.*");

	filter[j * 3 + k + 19] = 0;
	filter[j * 3 + k +  4] = 0;
	filter[j * 2 + k +  1] = 0;

	return selectFileOpenByFilter(win, filter, title);
}

char * selectFileOpenByFilter(HWND win, char *filter, char *title) {
	char *output;
	char buffer[FILE_MAXPATH];

	OPENFILENAME file;

	ZeroMemory(&buffer, sizeof buffer);
	ZeroMemory(&file, sizeof file);

	file.lStructSize	= sizeof file;
	file.hwndOwner		= win;
	file.lpstrTitle		= title;
	file.lpstrFilter	= filter;
	file.nFilterIndex	= 1;
	file.lpstrFile		= buffer;
	file.nMaxFile		= FILE_MAXPATH;
	file.Flags			= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if(GetOpenFileName(&file)) {
		NEW(output, char, strlen(buffer) + 1);
		strcpy(output, buffer);

		return output;
	}

	return NULL;
}

char * selectFileSave(HWND win, char *filename, char *title) {
	int		i, j, k;
	char	*output;

	int		release	= 0;
	char	*ext	= NULL;
	char	*temp	= filename;
	char	*filter	= "所有文件 (*.*)\0*.*\0";

	if(filename) {
		while(temp = strstr(temp, ".")) {
			if(*(temp + 1) == 0) {
				break;
			}

			temp = ext = temp + 1;
		}

		if(ext) {
			release = 1;

			j = strlen(ext);
			k = strlen(" 文件 (*.");

			NEW(filter, char, strlen(ext) * 3 + 64);

			for(i = 0; i < j; i++) {
				if((ext[i] >= 'a') && (ext[i] <= 'z')) {
					filter[i] = ext[i] & 0xDF;
				} else {
					filter[i] = ext[i];
				}
		
				if((ext[i] >= 'A') && (ext[i] <= 'Z')) {
					filter[i + j     + k    ] = ext[i] | 0x2F;
					filter[i + j * 2 + k + 4] = ext[i] | 0x2F;
				} else {
					filter[i + j     + k    ] = ext[i];
					filter[i + j * 2 + k + 4] = ext[i];
				}
			}

			memcpy(filter + j,			" 文件 (*.",	k);
			memcpy(filter + j * 2 + k,	") *.",		4);

			strcat(filter,				" 所有文件 (*.*) *.*");

			filter[j * 3 + k + 19] = 0;
			filter[j * 3 + k +  4] = 0;
			filter[j * 2 + k +  1] = 0;
		}
	}

	output = selectFileSaveByFilter(win, filename, ext, filter, title);

	if(release) {
		DEL(filter);
	}

	return output;
}

char * selectFileSaveByFilter(HWND win, char *filename, char *ext, char *filter, char *title) {
	char *output;
	char buffer[FILE_MAXPATH];

	OPENFILENAME file;

	ZeroMemory(&file, sizeof file);

	file.lStructSize	= sizeof(OPENFILENAME);
	file.hwndOwner		= win;
	file.lpstrTitle		= title;
	file.lpstrFilter	= filter;
	file.nFilterIndex	= 1;
	file.lpstrFile		= buffer;
	file.lpstrDefExt	= ext;
	file.nMaxFile		= FILE_MAXPATH;
	file.Flags			= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(filename) {
		strcpy(buffer, filename);
	} else {
		ZeroMemory(&buffer, sizeof buffer);
	}

	if(GetSaveFileName(&file)) {
		NEW(output, char, strlen(buffer) + 1);
		strcpy(output, buffer);

		return output;
	}

	return NULL;
}


void device2NtLetter(char *path) {
	char	buffer[MAX_PATH + 1] = {0};
	char	device[MAX_PATH + 1] = {0};

	int		length	= 0;
	char	*p		= buffer;
	char	*temp	= NULL;
	char	*letter	= NULL;

	if((path == NULL) || (GetLogicalDriveStrings(MAX_PATH, buffer) == FALSE)) {
		return;
	}

	NEW(temp, char, strlen(path) + 1);
	strcpy(temp, path);

	while(*p) {
		letter = p;

		/* 找下一个盘符 */
		while(*p) {
			if(*p == '\\') {
				*p = 0;
			}

			p++;
		}

		if(QueryDosDevice(letter, device, MAX_PATH)) {
			length = strlen(device);

			if((strncmp(path, device, length) == 0) && (path[length] == '\\')) {
				sprintf(path, "%s%s", letter, temp + length); break;
			}
		}

		p++;
	}

	DEL(temp);
}


int match(char *str, char *sub, int vague, int sencase) {
	int		i, j;

	char	*pos, *newStr, *newSub;
	size_t	lenStr, lenSub;

	lenStr = strlen(str);
	lenSub = strlen(sub);

	if(lenStr < lenSub) {
		return 0;
	}

	if(sencase) {
		newStr = str;
		newSub = sub;
	} else {
		NEW(newStr, char, lenStr + 1);
		NEW(newSub, char, lenSub + 1);

		for(i = 0, j = lenStr; i < j; i++) {
			if((str[i] >= 'A') && (str[i] <= 'Z')) {
				newStr[i] = str[i] | 0x20;
			} else {
				newStr[i] = str[i];
			}
		}
		for(i = 0, j = lenSub; i < j; i++) {
			if((sub[i] >= 'A') && (sub[i] <= 'Z')) {
				newSub[i] = sub[i] | 0x20;
			} else {
				newSub[i] = sub[i];
			}
		}
	}

	/* 查找 */
	pos = strstr(newStr, newSub);

	if(vague) {
		if(pos) {
			i = 1;
		} else {
			i = 0;
		}
	} else {
		if(pos == newStr) {
			i = 1;
		} else {
			i = 0;
		}
	}

	if(sencase == 0) {
		DEL(newStr);
		DEL(newSub);
	}

	return i;
}

int strEqual(char *str1, char *str2) {
	int		i;
	char	chr1, chr2;

	int		len = strlen(str1);

	if(len != strlen(str2)) {
		return 0;
	}

	for(i = 0; i < len; i++) {
		chr1 = str1[i];
		chr2 = str2[i];

		if((chr1 >= 'A') && (chr1 <= 'Z')) {
			chr1 = chr1 | 0x20;
		}
		if((chr2 >= 'A') && (chr2 <= 'Z')) {
			chr2 = chr2 | 0x20;
		}

		if(chr1 != chr2) {
			return 0;
		}
	}

	return 1;
}

char * wchar2char(wchar_t *str) {
	char	*ret, *temp;
	size_t	length;
	
	if(length = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL)) {
		NEW(temp, char, length + 1);

		/* 宽字符转换 */
		WideCharToMultiByte(CP_ACP, 0, str, -1, temp, length, NULL, NULL);
	} else {
		temp = "";
	}

	DEL(str);

	NEW(ret, char, strlen(temp) + 1);
	strcpy(ret, temp);

	return ret;
}
