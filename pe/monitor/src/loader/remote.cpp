/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ remote.cpp
*/


#include "base.h"



static __declspec(naked) void __cdecl remote_thread_code (void) {
	__asm {
			call	start


		function:
			; LoadLibraryA
				_emit	0x00
				_emit	0x00
				_emit	0x00
				_emit	0x00

			; GetProcAddress
				_emit	0x00
				_emit	0x00
				_emit	0x00
				_emit	0x00


		constant:
			; "monitor.dll"
				_emit	0x00
				_emit	0x00
				_emit	0x00
				_emit	0x00

			; "monitor_main"
				_emit	0x00
				_emit	0x00
				_emit	0x00
				_emit	0x00


		start:
			mov		eax, [esp]
			add		eax, 0x08

			push	eax										; [esp + 0x00] = :: constant
															; [esp + 0x04] = :: function


			push	dword ptr [eax + 0x04]					; GetProcAddress	[LPCSTR lpProcName]		= "monitor_main"

			push	dword ptr [eax]							; LoadLibraryA		[LPCTSTR lpFileName]	= "monitor.dll"


			mov		eax, [esp + 0x0C]
			call	dword ptr [eax]							; LoadLibraryA

			test	eax, eax
			jz		failed


			push	eax										; GetProcAddress	[HMODULE hModule]		= eax


			mov		eax, [esp + 0x0C]
			call	dword ptr [eax + 0x04]					; GetProcAddress


			add		esp, 0x08
			ret		4

		failed:
			add		esp, 0x0C
			ret		4
	}
}




/* 目标进程内存写入 */
bool remote_write (void *process, uintptr_t address, void *buffer, unsigned int size) {
	// 写入的内存大小
	unsigned int written;

	// 写入到进程内存
	if(WriteProcessMemory((HANDLE) process, (LPVOID) address, (LPCVOID) buffer, size, (SIZE_T *) & written) == FALSE) {
		return false;
	}

	// 判断是否完全写入
	if(size != written) {
		return false;
	}

	return true;
}



/* 将代码注入目标进程 */
bool remote_inject (void *process, uintptr_t address, const char *dll, const char *function) {
	// 写入函数代码
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT, remote_thread_code, REMOTE_MEMORY_CODE) == false) {
		return false;
	}

	// 常量长度
	unsigned int	length_dll		= strlen(dll) + 1;
	unsigned int	length_function	= strlen(function) + 1;

	// 写入 DLL 文件名
	if(remote_write(process, address, (void *) dll, length_dll) == false) {
		return false;
	}

	// 写入注入模块工作函数名
	if(remote_write(process, address + length_dll, (void *) function, length_function) == false) {
		return false;
	}

	// 常量地址
	uintptr_t	pointer_dll			= address;
	uintptr_t	pointer_function	= address + (uintptr_t) length_dll;

	// 写入 DLL 文件名地址
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + 0x0D, & pointer_dll, sizeof(uintptr_t)) == false) {
		return false;
	}

	// 写入注入模块工作函数名地址
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + 0x11, & pointer_function, sizeof(uintptr_t)) == false) {
		return false;
	}

	// 函数地址
	uintptr_t	function_loadlibrarya	= (uintptr_t) & LoadLibraryA;
	uintptr_t	function_getprocaddress	= (uintptr_t) & GetProcAddress;

	// 写入 LoadLibraryA 地址
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + 0x05, & function_loadlibrarya, sizeof(uintptr_t)) == false) {
		return false;
	}

	// 写入 GetProcAddress 地址
	if(remote_write(process, address + REMOTE_MEMORY_CONSTANT + 0x09, & function_getprocaddress, sizeof(uintptr_t)) == false) {
		return false;
	}

	return true;
}



/* 启动注入线程 */
void * remote_thread (void *process, uintptr_t address, uintptr_t argument) {
	// 启动远程线程
	return (void *) CreateRemoteThread (
		/* hProcess */				(HANDLE) process,
		/* lpThreadAttributes */	NULL,
		/* dwStackSize */			0,
		/* lpStartAddress */		(LPTHREAD_START_ROUTINE) address,
		/* lpParameter */			(LPVOID) argument,
		/* dwCreationFlags */		0,
		/* lpThreadId */			NULL
	);
}

