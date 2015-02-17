/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ win32.c, 0.1.1104
*/

#include <Windows.h>
#include "base.h"
#include "gui.h"
#include "image.h"


int messageBoxFormat(size_t type, char *format, ...) {
	char buffer[1024] = {0};

	va_list arg;
	va_start(arg, format) ;

	wvsprintf(buffer, format, arg);

	return MessageBox(wMain.hwnd, buffer, "", type);
}

BOOL uacIsAdmin() {
	BOOL isAdmin = FALSE;

	HANDLE token = NULL;
	TOKEN_ELEVATION elevation;
	DWORD length = 0;

	UINT16 winver = LOWORD(GetVersion());
	winver = MAKEWORD(HIBYTE(winver), LOBYTE(winver));

	/* 低于 vista */
	if(winver < 0x0600) {
		return FALSE;
	}

	/* 查询信息 */
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		if(GetTokenInformation(token, (TOKEN_INFORMATION_CLASS) 20, &elevation, sizeof elevation, &length)) {
			if(length == sizeof elevation) {
				isAdmin = elevation.TokenIsElevated ? TRUE : FALSE;
			}

			CloseHandle(token);
		}
	}

	return isAdmin;
}


char * selectFileOpen(char *ext, char *title) {
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
	file.hwndOwner = wMain.hwnd;
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

char * selectFileSave(char *filename, char *title) {
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
	file.hwndOwner = wMain.hwnd;
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


char * readFromFile() {
	int i;

	char *result;
	char buffer[MAX_PATH] = {0};

	for(i = 0; i < MAX_PATH; i++) {
		if(pfRead(buffer + i, sizeof(char)) == FALSE) {
			buffer[i] = 0; break;
		}

		if(buffer[i] == 0) {
			break;
		}
	}

	if(buffer[0] == 0) {
		return NULL;
	} else if(buffer[MAX_PATH - 1] != 0) {
		return NULL;
	} else {
		mem_alloc(result, char, i + 1);

		strcpy(result, buffer);

		return result;
	}
}

char * readFromBuffer(char *buffer, size_t bufsize) {
	char *result;
	size_t i;

	if(buffer[0] == 0) {
		return NULL;
	} else {
		i = 0;
	}

	mem_alloc(result, char, bufsize + 1);

	while((i < bufsize) && (buffer[i] != 0)) {
		result[i] = buffer[i];

		i++;
	}

	if(i < bufsize) {
		mem_realloc(result, char, i);

		result[i] = 0;
	}

	return result;
}


void strtolower(char *source) {
	while(*source != 0) {
		if((*source >= 'A') && (*source <= 'Z')) {
			*source |= 0x20;
		}

		source++;
	}
}

void strtoupper(char *source) {
	while(*source != 0) {
		if((*source >= 'a') && (*source <= 'z')) {
			*source &= 0xDF;
		}

		source++;
	}
}

BOOL isBlank(void *buffer, int size) {
	while((size--) > 0) {
		if(*((char *) buffer) != 0) {
			return FALSE;
		}

		buffer = ((char *) buffer) + 1;
	}

	return TRUE;
}
