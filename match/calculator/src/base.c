/*
	$ ExprCalc   (C) 2014 MF
	$ base.c   #1403
*/


#include "header/base.h"
#include "header/bridge.h"



/* 输入缓冲 */
const char *	buffer_input	= NULL;				/* @ base.h */
/* 输出缓冲 */
char *			buffer_output	= NULL;				/* @ base.h */
/* 错误缓冲 */
char *			buffer_error	= NULL;				/* @ base.h */

/* 处理空格表 */
uint8_t *		read_space		= NULL;				/* @ base.h */
/* 处理偏移量 */
unsigned int	read_offset		= 0;				/* @ base.h */



/* 内存托管表使用情况 */
static	unsigned int	memory_count	= 0;
static	unsigned int	memory_unused	= 0;

/* 内存托管表 */
static	void **			memory_table	= NULL;




/* 分配内存 */
void * ec_mm_new (unsigned int size) {
	void *	p	= calloc (1, (size_t) size);

	if(p == NULL) {
		ec_ex_runtime ("failed to allocate memory");
	}

	return p;
}


/* 重分配内存 */
void * ec_mm_renew (void *p, unsigned int size) {
	p	= realloc (p, (size_t) size);

	if(p == NULL) {
		ec_ex_runtime ("failed to reallocate memory");
	}

	return p;
}


/*  释放内存 */
void * ec_mm_delete (void *p) {
	if(p != NULL) {
		free (p);
	}

	return NULL;
}




/* 打开内存托管 */
void ec_mg_open (void) {
	memory_count	= 0;
	memory_unused	= EC_MG_BLOCK;

	memory_table	= (void **) ec_mm_new (sizeof (void *) * EC_MG_BLOCK);
}


/* 关闭内存托管 */
void ec_mg_close (void) {
	unsigned int i;

	for(i = 0; i < memory_count; i ++) {
		ec_mm_delete (memory_table [i]);
	}

	ec_mm_delete (memory_table);
}


/* 分配托管内存 */
void * ec_mg_new (unsigned int size) {
	void *	p	= ec_mm_new (size);

	if(memory_unused == 0) {
		memory_table	= (void **) ec_mm_renew (memory_table, sizeof (void *) * (memory_count + EC_MG_BLOCK));

		memory_unused	= EC_MG_BLOCK;
	}

	memory_table [memory_count]	= p;

	memory_count  ++;
	memory_unused --;

	return p;
}


/* 重分配内存托管 */
void * ec_mg_renew (void *p, unsigned int size) {
	unsigned int i;

	for(i = 0; i < memory_count; i ++) {
		if(memory_table [i] == p) {
			break;
		}
	}

	if(memory_count == i) {
		ec_ex_runtime ("managed memory function called with unmanaged memory address");
	}

	p	= ec_mm_renew (p, size);

	memory_table [i]	= p;

	return p;
}




/* 运行时异常 */
void ec_ex_runtime (const char *message) {
	/* 向标准错误流输出错误信息 */
	fprintf (stderr, "%s", message);

	/* 终止进程 */
	exit (-1);
}


/* 语法错误 */
int ec_ex_syntax (const char *message) {
	unsigned int	i		= 0;
	unsigned int	offset	= 0;

	if(* buffer_error != '\0') {
		return 0;
	}

	/* 计算实际偏移量 */
	while(i < read_offset) {
		/* 当前字节 */
		uint8_t	byte	= read_space [offset / 8];

		/* 判断当前位表示的字节是否不为空格 */
		if(((byte >> (offset % 8)) & 0x01) == 0) {
			i ++;
		}

		offset ++;
	}

	sprintf (buffer_error, "offset %u : %s", offset, message);

	return 0;
}




/* 打开 */
void calc_open (const char *input, char *output, char *error) {
	unsigned int	i;
	unsigned int	j;

	/* 输入字符串长度 */
	unsigned int	length	= strlen (input);

	/* 打开内存托管 */
	ec_mg_open ( );

	/* 创建输入副本 */
	buffer_input	= (const char *) ec_mg_new (length + 1);

	/* 创建空格表 */
	read_space		= (uint8_t *) ec_mg_new ((length / 8) + 1);
	/* 重置处理偏移量 */
	read_offset		= 0;

	/* 遍历每个输入字节 */
	for(i = j = 0; i < length; i ++) {
		/* 判断是否为空格 */
		if(isspace (input [i]) != 0) {
			/* 空格表字节 */
			uint8_t	byte	= 0x01 << (i % 8);

			/* 保存到空格表 */
			read_space [i / 8]	|=	byte;
		} else {
			/* 保存到输入备份 */
			((char *) buffer_input) [j ++]	= input [i];
		}
	}

	/* 设置缓冲区 */
	buffer_output	= output;
	buffer_error	= error;

	/* 清空输出缓冲区和错误缓冲区 */
	* buffer_output	= '\0';
	* buffer_error	= '\0';

	/* 设置词法分析器从字符串输入 */
	yy_scan_string (buffer_input);
}


/* 执行 */
int calc_execute (void) {
	/* 执行解析 */
	yyparse ( );

	/* 判断计算是否成功 */
	if(* buffer_error == '\0') {
		return 1;
	} else {
		return 0;
	}
}


/* 关闭 */
void calc_close (void) {
	/* 关闭词法分析器 */
	yylex_destroy ( );

	/* 关闭内存托管 */
	ec_mg_close ( );
}

