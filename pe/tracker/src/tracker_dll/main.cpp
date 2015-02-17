/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/main.cpp   # 1312
*/


#include "base.h"



/* Ö÷Èë¿Ú */
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

