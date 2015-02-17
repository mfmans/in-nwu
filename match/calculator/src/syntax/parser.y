%{

/*
	$ ExprCalc   (C) 2014 MF
	$ syntax/parser.y   #1403
*/


#include "../header/base.h"
#include "../header/bridge.h"
#include "../header/parser.h"


%}



%token		T_END				0

%left		','
%left		'+' '-'
%left		'*' '/'
%left		'^' '%' T_FUNCTION_MOD

%token		T_NUMBER

%token		T_FUNCTION_SIN
%token		T_FUNCTION_COS
%token		T_FUNCTION_TAN
%token		T_FUNCTION_ARCSIN
%token		T_FUNCTION_ARCCOS
%token		T_FUNCTION_ARCTAN
%token		T_FUNCTION_SINH
%token		T_FUNCTION_COSH
%token		T_FUNCTION_TANH
%token		T_FUNCTION_LOG
%token		T_FUNCTION_LOG10
%token		T_FUNCTION_LN
%token		T_FUNCTION_POW
%token		T_FUNCTION_EXP
%token		T_FUNCTION_FACT
%token		T_FUNCTION_MOD
%token		T_FUNCTION_SQRT
%token		T_FUNCTION_CUBEROOT
%token		T_FUNCTION_YROOT
%token		T_FUNCTION_AVG
%token		T_FUNCTION_SUM
%token		T_FUNCTION_VAR
%token		T_FUNCTION_VARP
%token		T_FUNCTION_STDEV
%token		T_FUNCTION_STDEVP

%token		T_FUNCTION_MAX
%token		T_FUNCTION_MIN
%token		T_FUNCTION_FLOOR
%token		T_FUNCTION_CEIL
%token		T_FUNCTION_ROUND
%token		T_FUNCTION_RAND
%token		T_FUNCTION_A2R
%token		T_FUNCTION_R2A



%%



start:
		expression														{	CALL (end,	&$1, NULL, NULL);	}
;


expression:
		'(' expression ')'												{	$$ = $2;	}

	|	T_NUMBER														{	$$ = $1;	}

	|	'+' expression													{	EXEC (pos,	&$$, &$2);			}
	|	'-' expression													{	EXEC (neg,	&$$, &$2);			}

	|	expression '+' expression										{	EXEC (add,	&$$, &$1, &$3);		}
	|	expression '-' expression										{	EXEC (sub,	&$$, &$1, &$3);		}
	|	expression '*' expression										{	EXEC (mul,	&$$, &$1, &$3);		}
	|	expression '/' expression										{	EXEC (div,	&$$, &$1, &$3);		}
	|	expression '%' expression										{	EXEC (mod,	&$$, &$1, &$3);		}
	|	expression '^' expression										{	EXEC (pow,	&$$, &$1, &$3);		}

	|	expression T_FUNCTION_MOD expression	 						{	EXEC (mod,	&$$, &$1, &$3);		}

	|	function
;



function:
		T_FUNCTION_SIN		'(' expression ')'							{	EXEC (sin,		&$$, &$3);		}
	|	T_FUNCTION_COS		'(' expression ')'							{	EXEC (cos,		&$$, &$3);		}
	|	T_FUNCTION_TAN		'(' expression ')'							{	EXEC (tan,		&$$, &$3);		}
	|	T_FUNCTION_ARCSIN	'(' expression ')'							{	EXEC (arcsin,	&$$, &$3);		}
	|	T_FUNCTION_ARCCOS	'(' expression ')'							{	EXEC (arccos,	&$$, &$3);		}
	|	T_FUNCTION_ARCTAN	'(' expression ')'							{	EXEC (arctan,	&$$, &$3);		}
	|	T_FUNCTION_SINH		'(' expression ')'							{	EXEC (sinh,		&$$, &$3);		}
	|	T_FUNCTION_COSH		'(' expression ')'							{	EXEC (cosh,		&$$, &$3);		}
	|	T_FUNCTION_TANH		'(' expression ')'							{	EXEC (tanh,		&$$, &$3);		}
	|	T_FUNCTION_LOG		'(' expression ',' expression ')'			{	EXEC (log,		&$$, &$3, &$5);	}
	|	T_FUNCTION_LOG10	'(' expression ')'							{	EXEC (log10,	&$$, &$3);		}
	|	T_FUNCTION_LN		'(' expression ')'							{	EXEC (ln,		&$$, &$3);		}
	|	T_FUNCTION_POW		'(' expression ',' expression ')'			{	EXEC (pow,		&$$, &$3, &$5);	}
	|	T_FUNCTION_EXP		'(' expression ')'							{	EXEC (exp,		&$$, &$3);		}
	|	T_FUNCTION_FACT		'(' expression ')'							{	EXEC (fact,		&$$, &$3);		}
	|	T_FUNCTION_MOD		'(' expression ',' expression ')'			{	EXEC (mod,		&$$, &$3, &$5);	}
	|	T_FUNCTION_SQRT		'(' expression ')'							{	EXEC (sqrt,		&$$, &$3);		}
	|	T_FUNCTION_CUBEROOT	'(' expression ')'							{	EXEC (cuberoot,	&$$, &$3);		}
	|	T_FUNCTION_YROOT	'(' expression ',' expression ')'			{	EXEC (yroot,	&$$, &$3, &$5);	}
	|	T_FUNCTION_AVG		'(' set_list ')'							{	EXEC (avg,		&$$, &$3);		}
	|	T_FUNCTION_SUM		'(' set_list ')'							{	EXEC (sum,		&$$, &$3);		}
	|	T_FUNCTION_VAR		'(' set_list ')'							{	EXEC (var,		&$$, &$3);		}
	|	T_FUNCTION_VARP		'(' set_list ')'							{	EXEC (varp,		&$$, &$3);		}
	|	T_FUNCTION_STDEV	'(' set_list ')'							{	EXEC (stdev,	&$$, &$3);		}
	|	T_FUNCTION_STDEVP	'(' set_list ')'							{	EXEC (stdevp,	&$$, &$3);		}
	|	T_FUNCTION_MAX		'(' set_list ')'							{	EXEC (max,		&$$, &$3);		}
	|	T_FUNCTION_MIN		'(' set_list ')'							{	EXEC (min,		&$$, &$3);		}
	|	T_FUNCTION_FLOOR	'(' expression ')'							{	EXEC (floor,	&$$, &$3);		}
	|	T_FUNCTION_CEIL		'(' expression ')'							{	EXEC (ceil,		&$$, &$3);		}
	|	T_FUNCTION_ROUND	'(' expression ')'							{	EXEC (round,	&$$, &$3);		}
	|	T_FUNCTION_RAND		'(' ')'										{	EXEC (rand,		&$$, NULL);		}
	|	T_FUNCTION_A2R		'(' expression ')'							{	EXEC (a2r,		&$$, &$3);		}
	|	T_FUNCTION_R2A		'(' expression ')'							{	EXEC (r2a,		&$$, &$3);		}
;



set_list:
		'(' set_list ')'												{	$$ = $2;	}
	|	set_list_define													{	$$ = $1;	}
;


set_list_define:
	'[' set_list_define_entry ']'										{	$$ = $2;	}
;


set_list_define_entry:
		expression														{	CALL (setbuild,	&$$, &$1, NULL);	}
	|	set_list_define_entry ',' expression							{	CALL (setwrite,	&$$, &$1, &$3);		}
	|	set_list														{	CALL (setmerge,	&$$, &$1, NULL);	}
	|	set_list_define_entry ',' set_list								{	CALL (setmerge,	&$$, &$1, &$3);		}
;



%%

