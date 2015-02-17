/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ process.cpp
*/


#include "base.h"



/* 启动新进程 */
void * process_start (const char *command, void **thread) {
	// 获取启动参数长度
	size_t	length	= strlen(command);
	// 创建启动参数拷贝
	char *	line	= new char [length + 1];

	// 生成启动参数拷贝
	strcpy(line, command);

	// 启动信息
	STARTUPINFOA		startup;
	// 进程信息
	PROCESS_INFORMATION	process;
	
	// 清空启动进程信息
	ZeroMemory(& startup,	sizeof(startup));
	ZeroMemory(& process,	sizeof(process));

	// 初始化启动信息
	startup.cb		= sizeof(startup);

	// 启动进程
	BOOL result = CreateProcessA (
		/* lpApplicationName */		NULL,
		/* lpCommandLine */			(LPSTR) line,
		/* lpProcessAttributes */	NULL,
		/* lpThreadAttributes */	NULL,
		/* bInheritHandles */		FALSE,
		/* dwCreationFlags */		CREATE_SUSPENDED,
		/* lpEnvironment */			NULL,
		/* lpCurrentDirectory */	NULL,
		/* lpStartupInfo */			& startup,
		/* lpProcessInformation */	& process
	);

	// 释放启动参数拷贝
	delete [] line;

	// 判断是否启动成功
	if(result == FALSE) {
		return NULL;
	}

	if(thread == NULL) {
		// 关闭线程句柄
		CloseHandle(process.hThread);
	} else {
		// 返回线程句柄
		*thread	= (void *) process.hThread;
	}

	// 返回进程句柄
	return (void *) process.hProcess;
}



/* 终止进程 */
void process_close (void *process, void *thread) {
	// 进程启动失败
	if(process == NULL) {
		return;
	}

	// 释放线程句柄
	CloseHandle(thread);

	// 终止进程
	TerminateProcess(process, 0);

	// 等待进程结束
	WaitForSingleObject(process, INFINITE);

	// 释放进程句柄
	CloseHandle(process);
}




/* 在目标进程中分配内存空间 */
uintptr_t process_memory (void *process) {
	// 申请内存
	return (uintptr_t) VirtualAllocEx(process, NULL, (SIZE_T) REMOTE_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

