/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ base.h, 0.1.1005
*/

#pragma once
#pragma warning (disable: 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <Windows.h>


#ifndef _BASE_H_
#define _BASE_H_

/* 引入全局变量 */
#ifdef _GUI_
	HWND hwnd;
	int running;
#else
	extern HWND hwnd;
	extern int running;
#endif


/* 值类型 */
typedef unsigned char var_t;
/* 寻址位置及长度类型 */
typedef unsigned long seek_t;
/* 数目类型 */
typedef unsigned int count_t;
/* CRC32 结果 */
typedef unsigned int crc32_t;


/* 提示 */
#define success(m) MessageBox(hwnd, m, "", MB_OK | MB_ICONINFORMATION);
/* 警告 */
#define alert(m) MessageBox(hwnd, m, "", MB_OK | MB_ICONEXCLAMATION);
/* 确认 */
#define confirm(m) (MessageBox(hwnd, m, "", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
/* 异常退出 */
#define die(m) MessageBox(hwnd, m, "", MB_OK | MB_ICONERROR); exit(EXIT_FAILURE);

/* 申请内存 */
#define m_new(p, type, count) \
	if((p = (type *) calloc(count, sizeof(type))) == NULL) { \
		die("应用程序尝试申请内存失败，已被迫终止运行。"); \
	}
/* 重新申请内存 */
#define m_renew(p, type, count) \
	if((p = (type *) realloc(p, sizeof(type) * (count))) == NULL) { \
		die("应用程序尝试申请内存失败，已被迫终止运行。"); \
	}
/* 释放内存 */
#define m_del(p) \
	if(p != NULL) { \
		free(p); \
		p = NULL; \
	}
/* 内存清零 */
#define m_zero(p, size) memset(p, 0, size)
/* 复制内存 */
#define m_copy(dst, src, size) memcpy(dst, src, size)
/* 比较内存 */
#define m_compare(dst, src, size) memcmp(dst, src, size)
/* 为字符串申请内存 */
#define ms_new(p, length) m_new(p, char, (length) + 1)
/* 为字符串重新申请内存 */
#define ms_renew(p, length) m_renew(p, char, (length) + 1)
/* 内存块与字符串比较 */
#define ms_compare(buf, str) m_compare(buf, str, strlen(str))

/* 运行线程 */
#define t_start(func, arg) _beginthread(thread_##func, 0, arg)
/* 注册线程 */
#define t_register(func) void thread_##func(void *arg)


#endif
