/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ win32.c, 0.1.1027
*/

#include <Windows.h>
#include "base.h"


extern HWND winHwnd;


int MessageBoxFormat(UINT type, char *format, ...) {
	char buffer[1024];

	va_list arg;
	va_start(arg, format) ;

	wvsprintf(buffer, format, arg);

	return MessageBox(winHwnd, buffer, "", type);
}


char * SelectFileOpen(char *ext, char *title) {
	int i, j, k;
	char *filter, *buffer;

	OPENFILENAME file;

	j = strlen(ext);
	k = strlen(" 文件 (*.");

	mem_alloc(filter, char, j * 3 + 64);
	mem_alloc(buffer, char, MAX_PATH + 1);

	for(i = 0; i < j; i++) {
		if((ext[i] >= 'a') && (ext[i] <= 'z')) {
			filter[i] = ext[i] & 0xDF;
		} else {
			filter[i] = ext[i];
		}
		
		if((ext[i] >= 'A') && (ext[i] <= 'Z')) {
			filter[i + j + k]			= ext[i] | 0x2F;
			filter[i + j * 2 + k + 4]	= ext[i] | 0x2F;
		} else {
			filter[i + j + k]			= ext[i];
			filter[i + j * 2 + k + 4]	= ext[i];
		}
	}

	memcpy(filter + j, " 文件 (*.", k);
	memcpy(filter + j * 2 + k, ") *.", 4);
	strcat(filter, " 所有文件 (*.*) *.*");

	filter[j * 3 + k + 19] = 0;
	filter[j * 3 + k + 4] = 0;
	filter[j * 2 + k + 1] = 0;

	ZeroMemory(&file, sizeof file);

	file.lStructSize = sizeof file;
	file.hwndOwner = winHwnd;
	file.lpstrTitle = title;
	file.lpstrFilter = filter;
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if(GetOpenFileName(&file) == FALSE) {
		mem_delete(buffer);
	}

	mem_delete(filter);

	return buffer;
}

char * SelectFileSave(char *filename, char *title) {
	int i, j, k;
	char *ext, *temp, *filter, *buffer;

	OPENFILENAME file;

	ext		= NULL;
	temp	= filename;

	filter	= "所有文件 (*.*)\0*.*\0";

	mem_alloc(buffer, char, MAX_PATH + 1);

	if(filename != NULL) {
		if(strlen(filename) < MAX_PATH) {
			strcpy(buffer, filename);
		}

		while((temp = strstr(temp, ".")) != NULL) {
			if(*(temp + 1) == 0) {
				break;
			}

			temp = ext = temp + 1;
		}

		if(ext != NULL) {
			j = strlen(ext);
			k = strlen(" 文件 (*.");

			mem_alloc(filter, char, strlen(ext) * 3 + 64);

			for(i = 0; i < j; i++) {
				if((ext[i] >= 'a') && (ext[i] <= 'z')) {
					filter[i] = ext[i] & 0xDF;
				} else {
					filter[i] = ext[i];
				}
		
				if((ext[i] >= 'A') && (ext[i] <= 'Z')) {
					filter[i + j + k]			= ext[i] | 0x2F;
					filter[i + j * 2 + k + 4]	= ext[i] | 0x2F;
				} else {
					filter[i + j + k]			= ext[i];
					filter[i + j * 2 + k + 4]	= ext[i];
				}
			}

			memcpy(filter + j, " 文件 (*.", k);
			memcpy(filter + j * 2 + k, ") *.", 4);
			strcat(filter, " 所有文件 (*.*) *.*");

			filter[j * 3 + k + 19] = 0;
			filter[j * 3 + k + 4] = 0;
			filter[j * 2 + k + 1] = 0;
		}
	}

	ZeroMemory(&file, sizeof file);

	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = winHwnd;
	file.lpstrTitle = title;
	file.lpstrFilter = filter;
	file.nFilterIndex = 1;
	file.lpstrFile = buffer;
	file.lpstrDefExt = ext;
	file.nMaxFile = MAX_PATH;
	file.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&file) != TRUE) {
		mem_delete(buffer);
	}
	if(ext != NULL) {
		mem_delete(filter);
	}

	return buffer;
}
