/*
	$ ExprCalc   (C) 2014 MF
	$ header/base.h   #1403
*/


#ifndef _MSC_VER
	#error This project DOES NOT support your compiler
#endif


#ifndef _EC_BASE_H_
#define _EC_BASE_H_


#pragma warning (disable: 4003)
#pragma warning (disable: 4996)


#define _USE_MATH_DEFINES


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <fpieee.h>
#include <Windows.h>


#ifdef __cplusplus
	extern "C" {
#endif



/* 数值类型 */
typedef		double		ec_value_t;


/* 集合类型 */
typedef struct {
	ec_value_t *		pointer;				/* 集合数组指针 */

	unsigned int		count;					/* 集合元素个数 */
	unsigned int		unused;					/* 数组剩余个数 */
} ec_set_t;


/* 运算数类型 */
typedef struct {
	int		is_set;								/* 集合标记 */

	union {
		ec_value_t	number;						/* 数值 */
		ec_set_t	set;						/* 集合 */
	} v;
} ec_operand_t;




/* 内存管理 */
void *	ec_mm_new		(unsigned int size);
void *	ec_mm_renew		(void *p, unsigned int size);
void *	ec_mm_delete	(void *p);

/* 带托管的内存管理 */
void	ec_mg_open		(void);
void	ec_mg_close		(void);
void *	ec_mg_new		(unsigned int size);
void *	ec_mg_renew		(void *p, unsigned int size);

/* 错误管理 */
void	ec_ex_runtime	(const char *message);
int		ec_ex_syntax	(const char *message);

/* 计算器功能 */
void	calc_open		(const char *input, char *output, char *error);
int		calc_execute	(void);
void	calc_close		(void);




/* 处理缓冲 */
extern	const char *	buffer_input;			/* @ base.c */
extern	char *			buffer_output;			/* @ base.c */
extern	char *			buffer_error;			/* @ base.c */

/* 处理空格表 */
extern	uint8_t *		read_space;				/* @ base.c */
/* 处理偏移量 */
extern	unsigned int	read_offset;			/* @ base.c */




/* 托管内存表扩充幅度 */
#define EC_MG_BLOCK				16

/* 集合扩充幅度 */
#define EC_SET_BLOCK			32




#ifdef __cplusplus
	}
#endif

#endif   /* BASE */