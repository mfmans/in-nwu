/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ base.h
*/


#ifndef _LOADER_BASE_H_
#define _LOADER_BASE_H_


#pragma warning (disable: 4102)
#pragma warning (disable: 4800)
#pragma warning (disable: 4996)


#ifdef UNICODE
	#undef UNICODE
#endif

#ifdef _UNICODE
	#undef _UNICODE
#endif



#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <Windows.h>
#include <Commctrl.h>

#include "error.h"




/* 需要在目标进程中分配的内存空间 */
#define REMOTE_MEMORY_SIZE			0x1000

/* 注入代码大小 */
#define REMOTE_MEMORY_CODE			0x100

/* 常量空间大小 */
#define REMOTE_MEMORY_CONSTANT		0x400

/* 控制器路径大小 */
#define REMOTE_MEMORY_HANDLER		0x200




#ifdef __cplusplus
	extern "C" {
#endif




/* process.c */
void *		process_start	(const char *command, void **thread);
void		process_close	(void *process, void *thread);
uintptr_t	process_memory	(void *process);


/* remote.c */
bool	remote_write	(void *process, uintptr_t address, void *buffer, unsigned int size);
bool	remote_inject	(void *process, uintptr_t address, const char *dll, const char *function);
void *	remote_thread	(void *process, uintptr_t address, uintptr_t argument);


/* call.c */
void	call	(const char *run, const char *handler, bool console);


/* gui.c */
int		gui_start		(const char *run, const char *handler, bool console, void *instance, int show);
void	gui_response	(const char *error);




#ifdef __cplusplus
	}
#endif

#endif   /* BASE */