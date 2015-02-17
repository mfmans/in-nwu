/*
	$ ExprCalc   (C) 2014 MF
	$ parser.c   #1403
*/


#include "header/base.h"
#include "header/bridge.h"
#include "header/parser.h"



/* 角度 => 弧度 */
#define A2R(v)			((v) * M_PI / 180)
/* 弧度 => 角度 */
#define R2A(v)			((v) * 180 / M_PI)




/* 浮点异常处理 */
static int __cdecl ec_parser_exception_floating (_FPIEEE_RECORD *record) {
	ec_ex_syntax ("floating error");

	return EXCEPTION_EXECUTE_HANDLER;
}




/* 统计函数公共运算 */
static ec_value_t ec_parser_statistic (YYSTYPE *in, ec_value_t *count) {
	unsigned int	i;

	ec_value_t		avg	= 0;
	ec_value_t		sum	= 0;

	/* 各项求和 */
	SL(in, i) {
		avg	+= SV(in, i);
	}

	/* 求平均值 */
	avg	= avg / (ec_value_t) i;

	/* 平方求和 */
	SL(in, i) {
		ec_value_t	diff	= SV(in, i) - avg;

		sum	+= diff * diff;
	}

	* count	= (ec_value_t) i;

	return sum;
}




/* 正反 */
DD1 (pos)	{	NW(out)	=   N(in_1);						} END_OF (out)
DD1 (neg)	{	NW(out)	= - N(in_1);						} END_OF (out)

/* 加减乘除 */
DD2 (add)	{	NW(out)	= N(in_1) + N(in_2);				} END_OF (out)
DD2 (sub)	{	NW(out)	= N(in_1) - N(in_2);				} END_OF (out)
DD2 (mul)	{	NW(out)	= N(in_1) * N(in_2);				} END_OF (out)
DD2 (div)	{	NW(out)	= N(in_1) / N(in_2);				} END_EX (out, "division by zero")

/* 求模乘方 */
DD2 (mod)	{	NW(out)	= fmod	(N(in_1), N(in_2));			} END_EX (out, "modulo by zero")
DD2 (pow)	{	NW(out)	= pow	(N(in_1), N(in_2));			} END_OF (out)

/* 三角函数 */
DD1 (sin)	{	NW(out)	= sin (A2R(N(in_1)));				} END_OF (out)
DD1 (cos)	{	NW(out)	= cos (A2R(N(in_1)));				} END_OF (out)
DD1 (tan)	{	NW(out)	= tan (A2R(N(in_1)));				} END_OF (out)

/* 反三角函数 */
DD1 (arcsin)	{	NW(out)	= R2A(asin (N(in_1)));			} END_OF (out)
DD1 (arccos)	{	NW(out)	= R2A(acos (N(in_1)));			} END_OF (out)
DD1 (arctan)	{	NW(out)	= R2A(atan (N(in_1)));			} END_OF (out)

/* 双曲函数 */
DD1 (sinh)	{	NW(out)	= sinh (N(in_1));					} END_OF (out)
DD1 (cosh)	{	NW(out)	= cosh (N(in_1));					} END_OF (out)
DD1 (tanh)	{	NW(out)	= tanh (N(in_1));					} END_OF (out)

/* 对数 */
DD2 (log)	{	NW(out)	= log (N(in_1)) / log (N(in_2));	} END_OF (out)
DD1 (log10)	{	NW(out)	= log10 (N(in_1));					} END_OF (out)
DD1 (ln)	{	NW(out)	= log   (N(in_1));					} END_OF (out)

/* 指数 */
DD1 (exp)	{	NW(out)	= exp (N(in_1));					} END_OF (out)

/* 阶乘 */
DD1 (fact)	{
	int	i;
	int	from	= (int) N(in_1);

	if(from < 0) {
		ec_ex_syntax ("factorials for negative integers");

		RETURN_FAILURE;
	}

	NW(out)	= 1;

	for(i = 1; i <= from; i ++) {
		NW(out)	*= (ec_value_t) i;
	}
} END_OF (out)

/* 开方 */
DD1 (sqrt) {
	NW(out)	= sqrt (N(in_1));
} END_OF (out)

/* 开三次方 */
DD1 (cuberoot) {
	if(N(in_1) < 0) {
		NW(out)	= - pow (- N(in_1), 1. / 3.);
	} else {
		NW(out)	= pow (N(in_1), 1. / 3.);
	}
} END_OF (out)

/* 开任意次方 */
DD2 (yroot) {
	if(N(in_1) < 0) {
		ec_ex_syntax ("unable to root a negative number");

		RETURN_FAILURE;
	}

	if(N(in_2) == 0) {
		ec_ex_syntax ("extract a root by zero");

		RETURN_FAILURE;
	}

	NW(out)	= pow (N(in_1), 1. / N(in_2));
} END_OF (out)


/* 集合的算术平均值 */
DD1 (avg) {
	unsigned int	i;
	ec_value_t		sum	= 0;

	SC(in_1);
	SL(in_1, i) {	sum += SV(in_1, i);		}

	NW(out)	= sum / (double) i;
} END_OF (out)


/* 集合的统计 */
DD1 (sum) {
	unsigned int	i;

	NW(out)	= 0;

	SC(in_1);
	SL(in_1, i) {	N(out) += SV(in_1, i);	}
} END_OF (out)


/* 集合的样本方差 */
DD1 (var) {
	ec_value_t	sum;
	ec_value_t	count;

	SC(in_1);

	sum	= ec_parser_statistic (in_1, & count);

	if(count < 2) {
		ec_ex_syntax ("sample variance division by zero");

		RETURN_FAILURE;
	}

	NW(out)	= sum / (count - 1);
} END_OF (out)


/* 集合的样本总体方差 */
DD1 (varp) {
	ec_value_t	sum;
	ec_value_t count;

	SC(in_1);

	sum	= ec_parser_statistic (in_1, & count);

	NW(out)	= sum / (ec_value_t) count;
} END_OF (out)


DD1 (stdev) {
	ec_value_t	sum;
	ec_value_t	count;

	SC(in_1);

	sum	= ec_parser_statistic (in_1, & count);

	if(count < 2) {
		ec_ex_syntax ("standard deviation division by zero");

		RETURN_FAILURE;
	}

	NW(out)	= sqrt (sum / (count - 1));
} END_OF (out)


DD1 (stdevp) {
	ec_value_t	sum;
	ec_value_t count;

	SC(in_1);

	sum	= ec_parser_statistic (in_1, & count);

	NW(out)	= sqrt (sum / (ec_value_t) count);
} END_OF (out)


/* 集合的最大值 */
DD1 (max) {
	unsigned int	i;

	NW(out)	= SV(in_1, 0);

	SC(in_1);

	SL(in_1, i) {
		if(N(out) < SV(in_1, i)) {
			N(out)	= SV(in_1, i);
		}
	}
} END_OF (out)


/* 集合的最小值 */
DD1 (min) {
	unsigned int	i;

	NW(out)	= SV(in_1, 0);

	SC(in_1);

	SL(in_1, i) {
		if(N(out) > SV(in_1, i)) {
			N(out)	= SV(in_1, i);
		}
	}
} END_OF (out)


/* 向下取整 */
DD1 (floor)	{	NW(out)	= floor (N(in_1));		} END_OF (out)
/* 向上取整 */
DD1 (ceil)	{	NW(out)	= ceil  (N(in_1));		} END_OF (out)

/* 四舍五入 */
DD1 (round)	{
	if(N(in_1) >= 0) {
		NW(out)	= (ec_value_t) ((int) (N(in_1) + 0.5));
	} else {
		NW(out)	= (ec_value_t) ((int) (N(in_1) - 0.5));
	}
} END_OF (out)

/* 生成随机数 */
DD1 (rand)	{
	NW(out)	= ((ec_value_t) rand ( )) / ((ec_value_t) RAND_MAX);
} END

/* 角度转弧度 */
DD1 (a2r) {		NW(out)	= A2R(N(in_1));			} END_OF (out)
/* 弧度转角度 */
DD1 (r2a) {		NW(out)	= R2A(N(in_1));			} END_OF (out)




/* 计算完成 */
DI (end) {
	sprintf (buffer_output, "%lf", N(op_1));
	
	RETURN_SUCCEED;
}



/* 创建集合 */
DI (setbuild) {
	/* 分配集合空间 */
	ec_value_t *	set	= (ec_value_t *) ec_mg_new (sizeof (ec_value_t) * EC_SET_BLOCK);

	/* 保存第一个元素 */
	set [0]	= op_2->v.number;

	op_1->is_set		= 1;
	op_1->v.set.count	= 1;
	op_1->v.set.pointer	= set;
	op_1->v.set.unused	= EC_SET_BLOCK - 1;

	RETURN_SUCCEED;
}


/* 写入集合 */
DI (setwrite) {
	/* 判断容量 */
	if(op_2->v.set.unused == 0) {
		/* 重分配集合 */
		op_2->v.set.pointer	= (ec_value_t *) ec_mg_renew (op_2->v.set.pointer, sizeof (ec_value_t) * (op_2->v.set.count + EC_SET_BLOCK));

		/* 更新可用空间 */
		op_2->v.set.unused	= EC_SET_BLOCK;
	}

	/* 保存元素 */
	op_2->v.set.pointer [op_2->v.set.count]	= op_3->v.number;

	op_1->is_set		= 1;
	op_1->v.set.count	= op_2->v.set.count  + 1;
	op_1->v.set.unused	= op_2->v.set.unused - 1;
	op_1->v.set.pointer	= op_2->v.set.pointer;

	RETURN_SUCCEED;
}


/* 集合合并 */
DI (setmerge) {
	SC(op_2);

	if(op_3 != NULL) {
		unsigned int	i;

		SC(op_3);

		/* 判断容量 */
		if(op_2->v.set.unused < op_3->v.set.count) {
			/* 扩充元素个数 */
			unsigned int	required	= op_2->v.set.unused + op_3->v.set.count;

			/* 重分配集合 */
			op_2->v.set.pointer	= (ec_value_t *) ec_mg_renew (op_2->v.set.pointer, sizeof (ec_value_t) * (op_2->v.set.count + required));

			/* 更新可用空间 */
			op_2->v.set.unused	= required;
		}

		/* 合并集合 */
		for(i = 0; i < op_3->v.set.count; i ++) {
			op_2->v.set.pointer [op_2->v.set.count + i]	= op_3->v.set.pointer [i];
		}
		
		op_2->v.set.count	+= i;
		op_2->v.set.unused	-= i;
	}

	op_1->is_set		= 1;
	op_1->v.set.count	= op_2->v.set.count;
	op_1->v.set.unused	= op_2->v.set.unused;
	op_1->v.set.pointer	= op_2->v.set.pointer;

	RETURN_SUCCEED;
}

