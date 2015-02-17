/*
	$ Tracker w64   (C) 2005-2014 MF
	$ demo/main.cpp   # 1312
*/


#pragma warning (disable: 4996)


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>



// MessageBoxTimeoutA
static	int (__stdcall * MessageBoxTimeoutW) (HWND hwnd, wchar_t *message, wchar_t *title, unsigned int type, void *language, unsigned int timeout);


// 日志文件
static	FILE *	fp	= NULL;

// 临界区
static	CRITICAL_SECTION	section;



/* 主入口 */
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}





/* @ ZwCreateUserProcess */
extern "C" __declspec(dllexport) int __cdecl tCreateProcess (wchar_t *application, wchar_t *command, uintptr_t argument) {
	return 1;
}




/* @ ZwCreateFile */
extern "C" __declspec(dllexport) int __cdecl tCreateFile (wchar_t *buffer, unsigned int length, unsigned int access) {
	return 1;
}

