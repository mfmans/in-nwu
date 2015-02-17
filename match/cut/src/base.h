/*
	$ base.h, 0.2.1221
*/

#pragma warning (disable: 4996)


#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <process.h>
#include <Windows.h>


#if defined(__cplusplus)
	extern "C" {
#endif


/* 整数类型 (32 bit) */
typedef unsigned int int_t;
/* 大整数类型 */
typedef int_t * bigint_t;


/* 数据链表 */
typedef struct number_s {
	/* 数字 */
	int_t		number;

	/* 分解后的余数和指数 */
	int_t		remainder;
	int_t		exponent;

	/* 大整数数据 */
	bigint_t	data;
	/* 整数数量 (DW) */
	size_t		countHex;
	size_t		countDec;

	/* 结果长度 */
	size_t		lenAll;
	/* 最高位 (10^10 进制) 长度 */
	size_t		lenHigh;

	/* 处理时间 */
	clock_t		startMul, endMul;
	clock_t		startConv, endConv;

	/* 已完成运算 */
	int			doneCalc;
	/* 已进行输出 */
	int			doneOutput;

	struct number_s *next;
} number_s;



/* 转换时使用的中转进制 (10 ^ 10) */
#define NOTATION	0x3B9ACA00

/* 10 ^ 10 进制数据储存偏移 */
#define OFFSET		2



/* 异常退出 */
#define DIE(message)													\
	MessageBox (														\
		win,															\
		TEXT(message "，应用程序已被迫终止运行"),							\
		"",																\
		MB_OK | MB_ICONERROR											\
	);																	\
	ExitProcess(EXIT_FAILURE);


/* 内存申请和释放 */
#define NEW(p, type, count)												\
	if((p = (type *) calloc(count, sizeof(type))) == NULL) {			\
		DIE("申请内存失败");												\
	}
#define DEL(p)															\
	if(p) {																\
		free(p); p = NULL;												\
	}

/* 按 16 位对齐的内存申请和释放 */
#define NEWA(p, type, count)											\
	if(p = (type *) _aligned_malloc(sizeof(type) * (count), 16)) {		\
		memset(p, 0, sizeof(type) * (count));							\
	} else 
#define DELA(p)															\
	if(p) {																\
		_aligned_free(p); p = NULL;										\
	}


/* 为 x * 3 ^ n 申请足够大小的大数空间 */
#define BIGINT(p, n)													\
	NEWA(p, int_t, (((n + 1) << 2) + 4) << 2)


/* 定义线程 */
#define THRD_NEW(name)													\
	void __cdecl name (int_t arg)

/* 运行线程 */
#define THRD_RUN(name, arg)												\
	_beginthread((void (__cdecl *)(void *)) name, 0, (void *) (arg))


/* 消息提醒对话框 */
#define ALERT(message)		MessageBox(win, TEXT(message), TEXT(""), MB_OK | MB_ICONWARNING)
#define NOTICE(message)		MessageBox(win, TEXT(message), TEXT(""), MB_OK | MB_ICONINFORMATION)

/* 确认对话框 */
#define CONFIRM(message)	(MessageBox(win, TEXT(message), TEXT(""), MB_YESNO | MB_ICONQUESTION) == IDYES)



/* 数据链表 */
extern number_s		*NUM;

/* 窗口句柄和实例 */
extern HWND			 win;
extern HINSTANCE	 instance;



/* 图形用户界面 */
void gui();

/* 按钮属性 */
void openEnable(int enable);
void saveEnable(int enable);
void calcEnable(int enable);
void calcText(char *text);

/* 获取输出位数 */
size_t textRead();

/* 使用 SSE 选项 */
int sseChecked();
void sseEnable(int enable);

/* 列表操作 */
void listAppend(number_s *number);
void listSet(int_t index, int_t column, char *format, ...);
void listClear();


/* 读取数据 */
void input();
/* 输出结果 */
void output();

/* 处理数据 */
void dispatch();
/* 关闭确认 */
int close();


/* 检测 CPU 是否支持 SSE2 */
int cpuSSE();


/* 乘方运算 */
size_t powWithALU(bigint_t data, int_t ratio, int_t exp);
size_t powWithSSE(bigint_t data, int_t ratio, int_t exp);

/* 进制转换 */
size_t convert(bigint_t data, size_t count);


#if defined(__cplusplus)
	}
#endif

#endif   /* _BASE_H_ */