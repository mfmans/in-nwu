%top{

/*
	$ ExprCalc   (C) 2014 MF
	$ syntax/lexer.l   #1403
*/


#include "../header/base.h"
#include "../header/bridge.h"
#include "../header/lexer.h"



}



%option		bison-bridge
%option		case-insensitive
%option		8bit

%option		noyywrap
%option		nounistd
%option		noyyalloc
%option		noyyrealloc
%option		noyyfree



%%



<<EOF>>															CALL (eof);				COUNT ( );	RETURN ( );


<INITIAL>{
	"0"															CALL (integer,	 0);	COUNT ( );	RETURN ( );
	"0b"[01]+													CALL (integer,	 2);	COUNT ( );	RETURN ( );
	"0"[0-7]+													CALL (integer,	 8);	COUNT ( );	RETURN ( );
	[1-9][0-9]*													CALL (integer,	10);	COUNT ( );	RETURN ( );
	"0x"[0-9a-f]+												CALL (integer,	16);	COUNT ( );	RETURN ( );

	[0-9]+"e"[\+\-]?[0-9]+										CALL (float);			COUNT ( );	RETURN ( );
	[0-9]+"."[0-9]*("e"[\+\-]?[0-9]+)?							CALL (float);			COUNT ( );	RETURN ( );
	[0-9]*"."[0-9]+("e"[\+\-]?[0-9]+)?							CALL (float);			COUNT ( );	RETURN ( );

	"("|")"|"["|"]"|","											CALL (delimiter);		COUNT ( );	RETURN ( );
	"+"|"-"|"*"|"/"|"%"|"^"										CALL (operator);		COUNT ( );	RETURN ( );

	[a-z]+														CALL (identifier);		COUNT ( );	RETURN ( );
	"log10"|"a2r"|"r2a"											CALL (identifier);		COUNT ( );	RETURN ( );

	" "																					COUNT ( );

	.															CALL (undefined);					RETURN ( );
}



%%



static	void *	yyalloc		(yy_size_t size)			{	return	ec_mm_new		((unsigned int) size);		}
static	void *	yyrealloc	(void *p, yy_size_t size)	{	return	ec_mm_renew		(p, (unsigned int) size);	}
static	void	yyfree		(void *p)					{			ec_mm_delete	(p);						}


