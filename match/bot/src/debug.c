/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ debug.c
*/

#include "base.h"


/* 线程类型 */
typedef struct thread_s {
	HANDLE	handle;
	DWORD	threadid;

	struct thread_s *next;
} thread_s;


/* 线程链表 */
static thread_s *thread = NULL;


/* 线程创建 */
static DWORD threadCreate(HANDLE handle, DWORD threadid) {
	thread_s *list = thread;

	while(list->next) {
		list = list->next;
	}

	NEW(list->next, thread_s, 1);

	list->next->handle		= handle;
	list->next->threadid	= threadid;

	return DBG_CONTINUE;
}

/* 线程结束 */
static DWORD threadExit(DWORD threadid) {
	thread_s *next;
	thread_s *list = thread;

	while(list->next) {
		if(list->next->threadid == threadid) {
			break;
		}

		list = list->next;
	}

	if(list->next == NULL) {
		return DBG_CONTINUE;
	}
	if(list->next->threadid != threadid) {
		return DBG_CONTINUE;
	}

	next = list->next;
	list->next = next->next;

	DEL(next);

	return DBG_CONTINUE;
}

/* 线程获取 */
static HANDLE threadGet(DWORD threadid) {
	thread_s *list = thread;

	while(list->next) {
		if(list->next->threadid == threadid) {
			break;
		}

		list = list->next;
	}

	if(list->next && (list->next->threadid == threadid)) {
		return list->next->handle;
	}

	NEW(list->next, thread_s, 1);

	list = list->next;
	list->threadid = threadid;

	/* 打开句柄 */
	if((list->handle = OpenThread (
			THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_TERMINATE,
			FALSE, threadid
		)) == NULL) {
			DIE("程序无法获取活动线程的句柄，被迫终止。");
	}

	return list->handle;
}


/* 读取寄存器 */
static void registerRead(HANDLE thread, CONTEXT *reg) {
	ZeroMemory(reg, sizeof(CONTEXT));

	reg->ContextFlags = CONTEXT_ALL;

	if(GetThreadContext(thread, reg) == FALSE) {
		DIE("程序无法读取活动线程的寄存器数据，被迫终止。");
	}
}

/* 保存寄存器 */
static void registerSave(HANDLE thread, CONTEXT *reg) {
	if(SetThreadContext(thread, reg) == FALSE) {
		DIE("程序无法保存活动线程的寄存器数据，被迫终止。");
	}
}


THREAD(debug) {
	/* 同步信息 */
	DWORD		symbol;
	/* 调试状态 */
	DWORD		status;
	/* 线程句柄 */
	HANDLE		handle;

	/* 寄存器 */
	CONTEXT		reg;
	/* 调试信息 */
	DEBUG_EVENT	debug;

	/* 创建线程链表 */
	NEW(thread, thread_s, 1);

	/* 等待挂载 */
	while(1) {
		WAIT(Run);

		if(procMount()) {
			break;
		}
	}

	/* 调试主循环 */
	while(1) {
		status = DBG_EXCEPTION_NOT_HANDLED;

		/* 程序终止 */
		TIMEOUT(symbol, Destroy, 0) {
			break;
		}

		/* 等待消息 */
		if(WaitForDebugEvent(&debug, 1) == FALSE) {
			if(GetLastError() == ERROR_SEM_TIMEOUT) {
				continue;
			}

			DIE("程序在读取调试状态时出现错误，被迫终止。");
		}

		/* 处理调试信息 */
		switch(debug.dwDebugEventCode) {
			/* 进程退出 */
			case EXIT_PROCESS_DEBUG_EVENT:
				NOTICE("游戏进程已经退出，点击确定后程序将自动关闭。");
				ExitProcess(EXIT_SUCCESS);

			/* 线程创建 */
			case CREATE_THREAD_DEBUG_EVENT:
				status = threadCreate(debug.u.CreateThread.hThread, debug.dwThreadId); break;
			/* 线程退出 */
			case EXIT_THREAD_DEBUG_EVENT:
				status = threadExit(debug.dwThreadId); break;

			/* 加载 DLL */
			case LOAD_DLL_DEBUG_EVENT:
				CloseHandle(debug.u.LoadDll.hFile); status = DBG_CONTINUE; break;

			/* 异常 */
			case EXCEPTION_DEBUG_EVENT:
				/* 单步或断点 */
				if((debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) || (debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)) {
					/* 获取线程句柄 */
					handle = threadGet(debug.dwThreadId);

					registerRead(handle, &reg);
					procAddress((size_t) debug.u.Exception.ExceptionRecord.ExceptionAddress, &reg);
					registerSave(handle, &reg);

					status = DBG_CONTINUE;
				} else {
					/* 交给系统处理 */
					if(debug.u.Exception.dwFirstChance == 0) {
						status = DBG_CONTINUE;
					}
				}
				break;
		}
		
		/* 继续执行 */
		ContinueDebugEvent(debug.dwProcessId, debug.dwThreadId, status);
	}

	/* 恢复进程内存 */
	procRestore();
}
