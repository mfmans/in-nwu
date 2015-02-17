/*
	$ Invoke   (C) 2005-2012 MF
	$ thread.c, 0.1.1202
*/

#include "base.h"


/* 线程结构体 */
typedef struct thread_s {
	DWORD	threadid;
	HANDLE	handle;

	/* 返回位置 */
	DWORD	ret;
	/* 返回断点 */
	BOOL	retBk;

	/* 恢复代码的函数 */
	function_s *function;

	struct thread_s *next;
} thread_s;


/* 线程链表 */
static thread_s *THDS = NULL;
/* 最后使用 */
static thread_s *LAST = NULL;


static thread_s * threadFind(DWORD threadid) {
	thread_s *list = THDS;

	/* 查找最后使用 */
	if(LAST && (LAST->threadid == threadid)) {
		return LAST;
	}

	/* 查找 */
	while(list->next) {
		if(list->next->threadid == threadid) {
			LAST = list->next;

			return LAST;
		}

		list = list->next;
	}

	return NULL;
}


void threadCreate(DWORD threadid, HANDLE handle) {
	thread_s *list;

	if(THDS == NULL) {
		NEW(THDS, thread_s, 1);
	}

	list = THDS;

	while(list->next) {
		if(list->next->threadid == threadid) {
			list = list->next; break;
		}

		list = list->next;
	}

	/* 链表同步 */
	if(list->threadid == threadid) {
		list->handle = handle;
	} else {
		NEW(list->next, thread_s, 1);

		list->next->threadid	= threadid;
		list->next->handle		= handle;
	}
}

void threadExit(DWORD threadid) {
	thread_s *next;
	thread_s *list = THDS;

	while(list->next) {
		if(list->next->threadid == threadid) {
			break;
		}

		list = list->next;
	}

	/* 找到线程 */
	if(list->next->threadid == threadid) {
		if(LAST == list->next) {
			LAST = NULL;
		}

		/* 恢复函数断点 */
		hookSetCode(list->next->function);

		next = list->next->next;

		DEL(list->next);

		list->next = next;
	}
}

HANDLE threadGet(DWORD threadid) {
	thread_s *list = THDS;

	/* 查找最后使用 */
	if(LAST && (LAST->threadid == threadid)) {
		return LAST->handle;
	}

	/* 查找 */
	while(list->next) {
		if(list->next->threadid == threadid) {
			LAST = list->next;


			return LAST->handle;
		}

		list = list->next;
	}

	/* 新运行的进程 */
	if(RT.entry) {
		die("Invoke 出现错误，非法的线程句柄查询，应用程序已被迫终止。");
	}

	NEW(list->next, thread_s, 1);

	list			= list->next;
	list->threadid	= threadid;

	/* 打开句柄 */
	if((list->handle = OpenThread (
		THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_TERMINATE,
		FALSE, list->threadid)) == NULL) {
			die("Invoke 出现错误，无法获取活动的线程句柄，应用程序已被迫终止。");
	}

	LAST = list;

	return list->handle;
}

function_s * threadFunctionGet(DWORD threadid) {
	thread_s *thread;

	if(thread = threadFind(threadid)) {
		return thread->function;
	} else {
		return NULL;
	}
}

void threadFunctionSet(DWORD threadid, function_s *func) {
	thread_s *thread;

	if(thread = threadFind(threadid)) {
		thread->function = func;
	}
}

DWORD threadRetGet(DWORD threadid, BOOL *retBk) {
	thread_s *thread;

	if(thread = threadFind(threadid)) {
		if(retBk) {
			*retBk = thread->retBk;
		}

		return thread->ret;
	} else {
		return 0;
	}
}

void threadRetSet(DWORD threadid, DWORD ret, BOOL retBk) {
	thread_s *thread;

	if(thread = threadFind(threadid)) {
		thread->ret		= ret;
		thread->retBk	= retBk;
	}
}

void threadSuspend(DWORD threadid) {
	thread_s *thread = THDS->next;

	while(thread) {
		if(thread->threadid != threadid) {
			SuspendThread(thread->handle);
		}

		thread = thread->next;
	}
}

void threadResume(DWORD threadid) {
	thread_s *thread = THDS->next;

	while(thread) {
		if(thread->threadid != threadid) {
			ResumeThread(thread->handle);
		}

		thread = thread->next;
	}
}
