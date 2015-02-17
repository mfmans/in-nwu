/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ base.h
*/

#ifndef _BASE_H_
#define _BASE_H_

/* for _beginthread */
#include <process.h>
/* for all */
#include <Windows.h>

#if defined(__cplusplus)
	extern "C" {
#endif


/* 空标题 */
#define T_NULL	TEXT("")


/* 运行错误 */
#define DIE(message)													\
	MessageBox(hWin, TEXT(message),	T_NULL, MB_OK | MB_ICONERROR);		\
	ExitProcess(EXIT_FAILURE);

/* 内存申请和释放 */
#define NEW(p, type, count)												\
	if((p = (type *) calloc(count, sizeof(type))) == NULL) {			\
		DIE("程序申请内存时发生错误，被迫终止。");								\
	}
#define DEL(p)															\
	if(p) {																\
		free(p); p = NULL;												\
	}

/* 信号发送、等待及复位 */
#define SEND(name)														\
	if(SetEvent(symbol##name) == FALSE) {								\
		DIE("程序发送线程同步信号时发生错误，被迫终止。");						\
	}
#define WAIT(name)														\
	if(WaitForSingleObject(symbol##name, INFINITE) != WAIT_OBJECT_0) {	\
		DIE("程序等待线程同步信号时发生错误，被迫终止。");						\
	}																	\
	RESET(name)
#define TIMEOUT(val, name, millisec)									\
	val = WaitForSingleObject(symbol##name, millisec);					\
	RESET(name);														\
	if((val != WAIT_TIMEOUT) && (val != WAIT_OBJECT_0)) {				\
		DIE("程序等待延迟性线程同步信号时发生错误，被迫终止。");					\
	} else if(val == WAIT_OBJECT_0) 
#define RESET(name)														\
	ResetEvent(symbol##name)

/* 线程定义 */
#define THREAD(name)			void name (void *dummy)
/* 线程启动 */
#define THRDRUN(name)			((HANDLE) _beginthread((void (__cdecl *)(void *)) name, 0, NULL))

/* 信息提示 */
#define NOTICE(message)			 MessageBox(hWin, TEXT(message), T_NULL, MB_OK | MB_ICONINFORMATION)
#define WARNING(message)		 MessageBox(hWin, TEXT(message), T_NULL, MB_OK | MB_ICONWARNING)


/* 运行实例 */
extern HINSTANCE	instance;
/* 主窗口句柄 */
extern HWND			hWin;
/* 对象窗口和进程 */
extern HWND			form;
extern HANDLE		process;

/* 启停信号 */
extern HANDLE		symbolRun, symbolStop;
/* 终止信号 */
extern HANDLE		symbolExit, symbolDestroy;


/* 线程 */
THREAD(gui);
THREAD(debug);
THREAD(mouse);

/* gui 相关 */
void buttonLoad(int enable);
void buttonRun(int enable);

/* 进程相关 */
int procMount();
void procAddress(size_t address, CONTEXT *reg);
void procRestore();
int procClose();
void procRun();

/* 内存读写 */
int memoryRead(DWORD address, size_t size, void *buffer);
int memoryWrite(DWORD address, size_t size, void *buffer);
int memoryWriteV(DWORD address, size_t size, ...);
int memorySet(DWORD address, size_t size, int data);
int memoryEqual(DWORD address, size_t size, void *buffer);
int memoryEqualV(DWORD address, size_t size, ...);


#if defined(__cplusplus)
	}
#endif

#endif   /* _BASE_H_ */