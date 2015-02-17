/*
	$ ExprCalc   (C) 2014 MF
	$ header/lexer.h   #1403
*/


#ifndef _EC_LEXER_H_
#define _EC_LEXER_H_

#ifdef __cplusplus
	extern "C" {
#endif



/* 词法分析函数名称 */
#define F(name)				ec_lexer_##name


/* 词法分析函数定义 */
#define DECL(name, ...)		int F (name) (YYSTYPE *out, const char *text, int length, __VA_ARGS__)

/* 调用词法分析函数 */
#define CALL(name, ...)		ec_lexer_return	= F (name) (yylval, yytext, yyleng, __VA_ARGS__);


/* 统计词数 */
#define COUNT()				read_offset		= read_offset + (unsigned int) yyleng;

/* 返回匹配结果 */
#define RETURN()			return ec_lexer_return;




/* 词法分析函数 */
DECL (eof);
DECL (integer,	int base);			DECL (float);
DECL (delimiter);					DECL (operator);				DECL (identifier);
DECL (undefined);




/* 词法分析函数返回结果 */
int		ec_lexer_return;					/* @ lexer.c */




#ifdef __cplusplus
	}
#endif

#endif   /* LEXER */