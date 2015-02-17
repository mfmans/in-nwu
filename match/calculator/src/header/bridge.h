/*
	$ ExprCalc   (C) 2014 MF
	$ header/bridge.h   #1403
*/


#ifndef _EC_BRIDGE_H_
#define _EC_BRIDGE_H_

#ifdef __cplusplus
	extern "C" {
#endif



/* 内部传递类型 */
#define YYSTYPE			ec_operand_t



/* 词法分析器缓冲区 */
struct yy_buffer_state;




/* 词法分析器 */
int		yylex			(YYSTYPE *);
int		yylex_destroy	(void);

struct yy_buffer_state *	yy_scan_string	(const char *);



/* 语法分析器 */
int		yyparse	(void);




/* 由 bison 生成 token 标号的头文件 */
#include "../syntax/parser.yy.h"




#ifdef __cplusplus
	}
#endif

#endif   /* BRIDGE */