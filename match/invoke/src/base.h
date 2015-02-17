/*
	$ Invoke   (C) 2005-2012 MF
	$ base.h, 0.1.1124
*/

#ifndef _INVOKE_BASE_H_
#define _INVOKE_BASE_H_

#pragma warning (disable: 4996)

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>
#include "dynmfunc.h"
#include "europa.h"


#if defined(__cplusplus)
	extern "C" {
#endif

#if defined(_INVOKE_MAIN_)
	#define _INVOKE_BASE_EXTERN_
#else
	#define _INVOKE_BASE_EXTERN_ extern
#endif


/* 结构体 */
struct hijack_s;
struct function_s;
struct module_s;


/* 调用规范 */
typedef enum {
	CL_CDECL, CL_STDCALL, CL_FASTCALL
} call_e;

/* 类型 */
typedef enum {
	VOID_0,
	SG_CHAR, SG_INT_16, SG_INT_32, SG_INT_64,
	US_CHAR, US_INT_16, US_INT_32, US_INT_64,
	WDCHAR, BYTE_8, REAL_32, REAL_64
} type_e;


/* 类型定义 */
typedef struct type_s {
	/* 类型 */
	type_e		type;
	/* 指针级别 */
	size_t		pointer;
	/* 数组索引 */
	size_t		arrindex;

	/* 大小 */
	size_t		size;
} type_s;


/* 调用参数 */
typedef struct argument_s {
	char	*name;
	type_s	type;

	struct argument_s *next;
} argument_s;

/* 劫持表 */
typedef struct hijack_s {
	/* 调用规范 */
	call_e	invoke;

	/* 返回类型 */
	type_s	ret;

	/* 参数 */
	size_t	argCount;
	argument_s *argList;

	/* 额外选项 */
	BOOL	vararg;
	BOOL	esp;

	struct function_s	*function;

	struct hijack_s		*prev;
	struct hijack_s		*next;
} hijack_s;


/* 函数表 */
typedef struct function_s {
	/* 名称或序号 */
	char	*name;
	DWORD	ordinal;

	/* 入口地址 */
	DWORD	entry;
	/* 断点地址 */
	DWORD	bp;

	/* @ hook.c  (v1) */
	/*
		// 原始指令地址
		DWORD	code;
		// 原始指令
		int		codeSize;
		char	codeData[32];
	*/

	/* 已经 hook */
	BOOL	hook;

	/* 原始指令代码 */
	unsigned char code;

	/* 劫持表 */
	struct hijack_s		*hijack;

	struct module_s		*module;
	struct function_s	*next;
} function_s;

/* 模块表 */
typedef struct module_s {
	/* 地址 */
	DWORD	base;
	/* 大小 */
	DWORD	size;

	/* 模块名 */
	char	*modname;
	/* 模块文件路径 */
	char	*filename;

	/* 导出函数表 */
	function_s *function;

	struct module_s *next;
} module_s;


/* 调试信息 */
typedef struct {
	/* 当前进程 */
	HINSTANCE	instance;
	HANDLE		process;

	/* 调试器线程 */
	HANDLE		debug;
	/* 是否附加别的进程 */
	BOOL		attach;
	/* 退出时解除断点 */
	BOOL		uninstall;

	/* 进程文件和主模块 */
	char		*file;
	char		*module;
	/* CUI */
	int			console;
	/* 主入口 */
	DWORD		entry;
	/* 进程信息 */
	DWORD		pid;
	HANDLE		handle;
	HANDLE		snapshot;
	/* 主模块 */
	DWORD		mainStart;
	DWORD		mainEnd;
	/* 当前运行信息 */
	HANDLE		thread;
	function_s	*function;
} runtime_s;


/* 运行链表 */
_INVOKE_BASE_EXTERN_ hijack_s		*HIJ;
_INVOKE_BASE_EXTERN_ module_s		*MOD;

/* 运行信息 */
_INVOKE_BASE_EXTERN_ runtime_s		RT;



/* 储存集合 */
#define packNew(name, ...)				void *name[] = {__VA_ARGS__};
#define packPtr(var, id, type)			((type *) ((void **) var)[id])
#define packVar(var, id, type)			(*((type *) ((void **) var)[id]))


/* 提示信息 */
#define error(win, message)					messageBoxFormat(win, MB_OK | MB_ICONERROR, "%s", message)
#define errorFormat(win, format, ...)		messageBoxFormat(win, MB_OK | MB_ICONERROR, format, __VA_ARGS__)
#define warning(win, message)				messageBoxFormat(win, MB_OK | MB_ICONWARNING, "%s", message)
#define warningFormat(win, format, ...)		messageBoxFormat(win, MB_OK | MB_ICONWARNING, format, __VA_ARGS__)
#define notice(win, message)				messageBoxFormat(win, MB_OK | MB_ICONINFORMATION, "%s", message)
#define noticeFormat(win, format, ...)		messageBoxFormat(win, MB_OK | MB_ICONINFORMATION, format, __VA_ARGS__)

/* 确认对话框 */
#define confirm(win, message)				(messageBoxFormat(win, MB_YESNO | MB_ICONQUESTION, "%s", message) == IDYES)
#define confirmNo(win, message)				(messageBoxFormat(win, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, "%s", message) == IDYES)
#define confirmFormat(win, format, ...)		(messageBoxFormat(win, MB_YESNO | MB_ICONQUESTION, format, __VA_ARGS__) == IDYES)
#define confirmNoFormat(win, format, ...)	(messageBoxFormat(win, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, format, __VA_ARGS__) == IDYES)


/* 申请内存 */
#define NEW(p, type, count) \
	if((p = (type *) calloc(count, sizeof(type))) == NULL) { \
		die("应用程序申请内存失败。"); \
	}

/* 重申请内存 */
#define RENEW(p, type, count) \
	if((p = (type *) realloc(p, sizeof(type) * (count))) == NULL) { \
		die("应用程序申请内存失败。"); \
	}

/* 释放内存 */
#define DEL(p) \
	if(p != NULL) { \
		free(p); p = NULL; \
	}


/* 创建线程 */
#define threadRun(name, arg)	_beginthread((void (*)(void *)) threadFunc##name, 0, (void *) arg)
/* 定义线程 */
#define threadNewVoid(name)		void threadFunc##name(void *arg)
#define threadNewChar(name)		void threadFunc##name(char *arg)



/* 错误退出 */
void die(char *message);

/* 信号相关 */
void symbolInit();
int symbolRun(int arg);
void symbolRunAsyn(int arg);
int symbolWait();
void symbolReturn(int data);

/* 检测管理员权限 */
int uacAdminCheck();

/* 格式化 MessageBox */
int messageBoxFormat(HWND win, unsigned int type, char *format, ...);

/* 激活窗口 */
void activeWindow(HWND win);

/* 选择文件打开或保存 */
char * selectFileOpen(HWND win, char *ext, char *title);
char * selectFileOpenByFilter(HWND win, char *filter, char *title);
char * selectFileSave(HWND win, char *filename, char *title);
char * selectFileSaveByFilter(HWND win, char *filename, char *ext, char *filter, char *title);

/* DOS 路径转换 */
void device2NtLetter(char *path);

/* 字符串操作 */
int match(char *str, char *sub, int vague, int sencase);
int strEqual(char *str1, char *str2);
char * wchar2char(wchar_t *str);


/* 线程相关 */
void threadCreate(DWORD threadid, HANDLE handle);
void threadExit(DWORD threadid);
HANDLE threadGet(DWORD threadid);
function_s * threadFunctionGet(DWORD threadid);
void threadFunctionSet(DWORD threadid, function_s *func);
DWORD threadRetGet(DWORD threadid, BOOL *retBk);
void threadRetSet(DWORD threadid, DWORD ret, BOOL retBk);
void threadSuspend(DWORD threadid);
void threadResume(DWORD threadid);

/* 进程相关 */
int processList(DWORD **pid, char ***name);
char * processSelect(int index);
char * processFile();
void processFree();
int processRead(DWORD addr, void *buffer, unsigned int size);
int processWrite(DWORD addr, void *buffer, unsigned int size);

/* 进程内存 */
void memoryRewind();
int memorySeek(int offset, int mode);
int memoryTell();
int memoryAvail(int offset);
int memoryRead(void *buffer, unsigned int size);
char * memoryReadString(unsigned int max, int display);
int memoryWrite(void *buffer, unsigned int size);

/* 模块 */
void moduleShow();
void moduleSet(module_s *module, DWORD offset, BOOL moveStack, BOOL moveCall, BOOL moveJump, BOOL moveRet);
function_s * moduleFind(HWND win, char *module, char *function, unsigned long int ordinal, int *mode);
void moduleDelete(DWORD base);
module_s * moduleLoad(HANDLE handle, DWORD base);
void moduleDump();

/* HOOK */
void hookSetCode(function_s *func);
void hookSetBp(function_s *func);
int hookInstall(function_s *func);
void hookInstallAll(HWND win);
int hookUninstall(function_s *func);
void hookUninstallAll();
function_s * hookFind(DWORD address, module_s **module);

/* 数据封装 */
void packageCreate(HWND win);
int packageLoad(HWND win, char *file, BOOL silent);
void packageAuto();

/* 类型相关 */
char * typeReadSimple(DWORD addr, type_s *type);
char * typeReadDetail(DWORD addr, type_s *type, char **pointer);
char * type2string(type_s *type);
void typeSet(type_s *p, type_e type, size_t pointer, size_t arrindex);
void typeComboInit(erp_object_combo_s *obj, type_s *type, BOOL allowVoid);
int typeComboChange(int index, erp_object_combo_s *obj, type_s *type, BOOL allowVoid);
char * typeArgument(argument_s *arg, BOOL vararg);


/* 暂停恢复 */
void runSuspendResume();

/* 调试主线程 */
threadNewVoid(runDebug);



/* ntdll.dll */
FUNCTION(processSuspend,	long, NTAPI, HANDLE process);
FUNCTION(processResume,		long, NTAPI, HANDLE process);
/* disasm.dll */
FUNCTION(disasm, int, __cdecl, unsigned int address, char *buffer, unsigned int bufSize, char *output, unsigned int outSize);


#if defined(__cplusplus)
	}
#endif

#endif   /* _INVOKE_BASE_H_ */