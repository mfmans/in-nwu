/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ base.h, 0.1.1104
*/

#ifndef _SNP_BASE_H_
#define _SNP_BASE_H_

#pragma warning (disable: 4996)

#include <stdio.h>
#include <stdlib.h>


/* 字节开关的读写 */
#define bitGet(v, offset)				((v) & (0x01 << (offset)))
#define bitSet(v, offset, condition)	v = v | ((condition) ? (0x01 << (offset)) : 0)

/* 储存集合 */
#define packNew(name, ...)				void *name[] = {__VA_ARGS__};
#define packPtr(var, id, type)			(type *) ((void **) var)[id]
#define packVar(var, id, type)			*((type *) ((void **) var)[id])


/* 提示信息 */
#define error(message)					messageBoxFormat(MB_OK | MB_ICONERROR, "%s", message)
#define errorFormat(format, ...)		messageBoxFormat(MB_OK | MB_ICONERROR, format, __VA_ARGS__)
#define warning(message)				messageBoxFormat(MB_OK | MB_ICONWARNING, "%s", message)
#define warningFormat(format, ...)		messageBoxFormat(MB_OK | MB_ICONWARNING, format, __VA_ARGS__)
#define notice(message)					messageBoxFormat(MB_OK | MB_ICONINFORMATION, "%s", message)
#define noticeFormat(format, ...)		messageBoxFormat(MB_OK | MB_ICONINFORMATION, format, __VA_ARGS__)

/* 确认对话框 */
#define confirm(message)				(messageBoxFormat(MB_YESNO | MB_ICONQUESTION, "%s", message) == IDYES)
#define confirmNo(message)				(messageBoxFormat(MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, "%s", message) == IDYES)
#define confirmFormat(format, ...)		(messageBoxFormat(MB_YESNO | MB_ICONQUESTION, format, __VA_ARGS__) == IDYES)
#define confirmNoFormat(format, ...)	(messageBoxFormat(MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, format, __VA_ARGS__) == IDYES)

/* 异常退出 */
#define die(message)					error(message); exit(EXIT_FAILURE);


/* 申请内存 */
#define mem_alloc(p, type, count) \
	if((p = (type *) calloc(count, sizeof(type))) == NULL) { \
		die("应用程序申请内存失败。"); \
	}

/* 重申请内存 */
#define mem_realloc(p, type, count) \
	if((p = (type *) realloc(p, sizeof(type) * (count))) == NULL) { \
		die("应用程序申请内存失败。"); \
	}

/* 释放内存 */
#define mem_delete(p) \
	if(p != NULL) { \
		free(p); p = NULL; \
	}


/* 线程函数 */
#define thNew(name)			void __thread##name(void *arg)
#define thNewChar(name)		void __thread##name(char *arg)

/* 创建线程 */
#define thRun(name, arg)	_beginthread((void (*)(void *)) __thread##name, 0, (void *) arg)


/* 格式化 MessageBox */
int messageBoxFormat(size_t type, char *format, ...);

/* 检测管理员权限 */
BOOL uacIsAdmin();

/* 选择文件打开或保存 */
char * selectFileOpen(char *ext, char *title);
char * selectFileSave(char *file, char *title);

/* 读取字符串 */
char * readFromFile();
char * readFromBuffer(char *buffer, size_t bufsize);


/* 字符串大小写转换 */
void strtolower(char *source);
void strtoupper(char *source);


/* 检测内存是否为 0 */
BOOL isBlank(void *buffer, int size);


#endif   /* _SNP_BASE_H_ */