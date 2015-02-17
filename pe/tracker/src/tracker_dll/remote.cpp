/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/remote.cpp   # 1312
*/


#include "base.h"



/* 在目标进程中分配内存空间 */
uintptr_t remote::allocate (HANDLE process, unsigned int size) {
	return (uintptr_t) VirtualAllocEx (process, NULL, (SIZE_T) size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}




/* 从目标进程内存读取数据 */
bool remote::read (HANDLE process, uintptr_t address, void *buffer, unsigned int size) {
	// 读取的内存大小
	unsigned int read;

	// 读取内存
	if(ReadProcessMemory (process, (LPCVOID) address, (LPVOID) buffer, (SIZE_T) size, (SIZE_T *) & read) == FALSE) {
		return false;
	}

	// 判断是否完全读取
	if(size != read) {
		return false;
	}

	return true;
}


/* 向目标进程内存写入数据 */
bool remote::write (HANDLE process, uintptr_t address, void *buffer, unsigned int size) {
	// 写入的内存大小
	unsigned int written;

	// 写入到进程内存
	if(WriteProcessMemory (process, (LPVOID) address, (LPCVOID) buffer, size, (SIZE_T *) & written) == FALSE) {
		return false;
	}

	// 判断是否完全写入
	if(size != written) {
		return false;
	}

	return true;
}



/* 在目标进程中创建线程 */
HANDLE remote::thread (HANDLE process, uintptr_t address, uint32_t argument) {
	return CreateRemoteThread (
		/* hProcess */				(HANDLE) process,
		/* lpThreadAttributes */	NULL,
		/* dwStackSize */			0,
		/* lpStartAddress */		(LPTHREAD_START_ROUTINE) address,
		/* lpParameter */			(LPVOID) argument,
		/* dwCreationFlags */		0,
		/* lpThreadId */			NULL
	);
}

