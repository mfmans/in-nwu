/**
	$ Calculator   (C) 2005-2014
	$ lexical.cpp
*/


#include "base.h"



/* 输入缓冲区 */
char *	lexical::buffer		= NULL;

/* 当前处理指针 */
char *	lexical::pointer	= NULL;
/* 结束边界指针 */
char *	lexical::boundary	= NULL;





/* 过滤空格 */
void lexical::trim (void) {
	char *	source	= lexical::buffer;
	char *	target	= lexical::buffer;

	// 遍历字符串
	while(* source != 0) {
		// 判断是否为空格
		if(isspace (* source) == 0) {
			// 移动字符
			if(source != target) {
				* target	= * source;
			}

			// 目标指向下一个字符
			target ++;
		}

		// 源指向下一个字符
		source ++;
	}

	* target	= 0;
}




/* 读取表达式 */
void lexical::read (FILE *stream) {
	// 分配缓冲区
	if(lexical::buffer == NULL) {
		lexical::buffer	= new char [EXPRESSION_MAX];
	}

	// 读取表达式
	fgets (lexical::buffer, EXPRESSION_MAX, stream);

	// 过滤所有空格
	lexical::trim ( );

	// 重置指针
	lexical::pointer	= lexical::buffer;
	lexical::boundary	= lexical::buffer + strnlen (lexical::buffer, EXPRESSION_MAX);
}



/* 解析下一个单词 */
bool lexical::next (char *symbol, double *value, int *error) {
	// 判断是否到达字符串结尾
	if(lexical::pointer == lexical::boundary) {
		return false;
	}

	// 初始化返回值
	* symbol	= 0;
	* value		= 0.;
	* error		= -1;

	// 解析运算符
	switch(* lexical::pointer) {
		// 普通运算符
		case '+':
		case '*':
		case '/':
		case '(':
		case ')':
			goto found_symbol;

		// 减号/负号
		case '-':
			// 判断下一个字符是否为 '-'
			if(* (lexical::pointer + 1) == '-') {
				goto found_symbol;
			} else {
				// 判断前一个字符是否为运算符
				if(lexical::pointer == lexical::buffer) {
					goto found_number;
				} else {
					switch(* (lexical::pointer - 1)) {
						case '+':
						case '-':
						case '*':
						case '/':
						case '(':
							goto found_number;
					}

					goto found_symbol;
				}
			}
	}


found_number:
	// 起止偏移量
	int	offset_from	= 0;
	int	offset_to	= 0;

	// 读取
	if(sscanf (lexical::pointer, "%n%lf%n", & offset_from, value, & offset_to) < 1) {
		goto error;
	}

	// 移动到字符后
	lexical::pointer += offset_to - offset_from;

	return true;


found_symbol:
	// 返回负号
	* symbol	= * lexical::pointer;

	// 符号长度为 1
	lexical::pointer ++;

	return true;


error:
	// 计算错误出现偏移量
	* error	= lexical::pointer - lexical::buffer;

	// 返回读取失败
	return false;
}

