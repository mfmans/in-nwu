/*
	$ dispatch.c, 0.2.1221
*/

#include "base.h"


/* 启动和停止进程动态线程 */
#define ANIMATE_START(item, c)								\
	ResetEvent(ANI_STOP);									\
	ResetEvent(ANI_EXIT);									\
	THRD_RUN(threadAnimate, index | (c << 31));				\
	num->start##item = clock()
#define ANIMATE_STOP(item)									\
	num->end##item = clock();								\
	SetEvent(ANI_STOP);										\
	WaitForSingleObject(ANI_EXIT, INFINITE)


/* 是否正在运行中 */
static int		RUN		= 0;
/* 使用 SSE */
static int		SSE		= 0;

/* 运行线程 */
static HANDLE	THRD	= NULL;

/* 动态进度信号 */
static HANDLE	ANI_STOP	= NULL;
static HANDLE	ANI_EXIT	= NULL;



/* 动态进度线程 */
static THRD_NEW(threadAnimate) {
	/* 等待消息 */
	DWORD	signal;

	/* 序号 */
	size_t	i = 0;
	/* 进度信息 */
	char	*process[] = {".", "..", "..."};

	while(1) {
		listSet (
			/* 行 */		arg & 0x7FFFFFFF,
			/* 列 */		(arg >> 31) + 5,
			"%s",
			process[i]
		);

		/* 选择下一个动态 */
		i = (i + 1) % (sizeof process / sizeof(char *));

		/* 等待退出 */
		signal = WaitForSingleObject(ANI_STOP, 1000);

		if(signal == WAIT_OBJECT_0) {
			break;
		} else if(signal != WAIT_TIMEOUT) {
			DIE("错误的同步信号信息");
		}
	}

	SetEvent(ANI_EXIT);
}


/* 计算调度线程 */
static THRD_NEW(threadDispatch) {
	number_s *num = NUM->next;

	/* 计算函数 */
	size_t (*func)(bigint_t, int_t, int_t) = NULL;

	double	notation;
	size_t	temp, index = 0, status = 0;

	/* 系数、底数及结果最高位 */
	int_t	ratio, exp, high;

	/* 选择计算函数 */
	if(SSE) {
		func = powWithSSE;
	} else {
		func = powWithALU;
	}

	/* 判断状态 */
	while(num) {
		if(num->doneCalc) {
			status |= 1;
		} else {
			status |= 2;
		}

		num = num->next;
	}

	/* 根据状态提示信息 */
	switch(status) {
		/* 没有数据 */
		case 0: goto done;

		/* 所有数据已完成计算 */
		case 1:
			if(!CONFIRM("是否重新开始计算？")) {
				goto done;
			}

			status = 1;
			break;

		/* 所有数据尚未完成计算 */
		case 2:
			status = 1; break;

		/* 部分数据已完成 */
		case 3:
			if(CONFIRM("有部分数据已经计算完成，是否只计算剩余的数据？如果选择否，则重新计算所有数据。")) {
				status = 0;
			} else {
				status = 1;
			}
			break;
	}

	num = NUM->next;

	/* 初始化数据 */
	while(num) {
		if((num->doneCalc == 0) || (status == 1)) {
			DELA(num->data);

			num->doneCalc	= 0;
			num->doneOutput	= 0;

			/* 清空显示 */
			listSet(index, 3, "?");
			listSet(index, 4, "?");
			listSet(index, 5, "-");
			listSet(index, 6, "-");
			listSet(index, 7, "-");
		}

		index++;
		num = num->next;
	}

	index	= 0;
	num		= NUM->next;

	while(num) {
		if(num->doneCalc) {
			index++;
			num = num->next;

			continue;
		}

		/* 分配内存 */
		BIGINT(num->data, num->exponent) {
			listSet(index, 2, "");
			listSet(index, 3, "");
			listSet(index, 4, "内存不足");
			listSet(index, 5, "");
			listSet(index, 6, "");
			listSet(index, 7, "");

			num->doneCalc	= 1;
			num->doneOutput	= 1;

			index++;
			num = num->next;

			continue;
		}

		/* 系数 */
		if(num->remainder) {
			ratio = num->remainder;
		} else {
			ratio = 1;
		}

		/* 指数 */
		exp = num->exponent;

		/* 转换为 3 ^ 16 为底的乘方 */
		if(temp = exp % 16) {
			while(temp--) {
				ratio *= 3;
			}
		}

		/* 3 ^ 16n = (3 ^ 16) ^ n */
		exp >>= 4;

		/* 计算乘积 */
		ANIMATE_START(Mul, 0);
		num->countHex = func(num->data, ratio, exp);
		ANIMATE_STOP(Mul);

		/* 输出时间 */
		listSet(index, 5, "%.3lfs", (double) (num->endMul - num->startMul) / 1000.);

		/* 转换为 10 ^ 10 进制 */
		ANIMATE_START(Conv, 1);
		num->countDec = convert(num->data, num->countHex);
		ANIMATE_STOP(Conv);

		/* 长度 */
		num->lenAll		= 9 * num->countDec;
		num->lenHigh	= 9;

		/* 最高位 */
		high = *(num->data + (num->countDec - 1) * OFFSET + 1);
		/* 初始化数量级 */
		temp = NOTATION / 10;

		/* 计算十进制位数 */
		while(high < temp) {
			temp = temp / 10;

			num->lenAll--;
			num->lenHigh--;
		}

		/* 输出乘积 */
		if(num->countHex == 1) {
			listSet(index, 2, "%lu\n\n", high);
		} else {
			/* 最高位转换 */
			temp		= high;
			notation	= (double) NOTATION;

			/* 计算除法系数 */
			while(notation < high) {
				notation	/= 10.;
			}

			/* 将最高位凑够 10^10 */
			while(temp < NOTATION) {
				notation	/= 10.;
				temp		*= 10;
			}

			temp += (int_t) ((double) *(num->data + (num->countDec - 2) * OFFSET + 1) / notation);

			listSet(index, 2, "%1.4lf * 10 ^ %lu\n\n", (double) temp / NOTATION, num->lenAll - 1);
		}

		/* 输出其它信息 */
		listSet(index, 3, "%lu", num->lenAll);
		listSet(index, 4, "%lu Byte", sizeof(int_t) * 2 * num->countDec);
		listSet(index, 6, "%.3lfs", (double) (num->endConv - num->startConv) / 1000.);
		listSet(index, 7, "%.3lfs", (double) ((num->endMul - num->startMul) + (num->endConv - num->startConv)) / 1000.);

		num->doneCalc = 1;

		index++;
		num = num->next;
	}


done:		/* 完成计算 */
	num		= NUM->next;

	RUN		= 0;
	THRD	= NULL;

	/* 按钮状态 */
	openEnable(1);
	saveEnable(0);
	sseEnable(1);
	calcText("开始计算");

	/* 检测是否有计算完成 */
	while(num) {
		if(num->doneCalc) {
			saveEnable(1); break;
		}

		num = num->next;
	}
}


/* 开始运行 */
static void start() {
	if(NUM->next == NULL) {
		return;
	}

	/* 判断 SSE 开启 */
	if((SSE = sseChecked()) == 0) {
		if(cpuSSE()) {
			if(!CONFIRM("使用 SSE2 指令可对计算进行优化，极大加快计算的速度。关闭 SSE2 优化后，计算可能会耗费较长时间，是否继续？")) {
				return;
			}
		}
	}

	openEnable(0);
	saveEnable(0);
	sseEnable(0);
	calcText("停止计算");

	RUN = 1;

	/* 创建信号量 */
	if(ANI_STOP == NULL) {
		ANI_STOP = CreateEvent(NULL, TRUE, FALSE, NULL);
		ANI_EXIT = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	/* 创建运行线程 */
	THRD = (HANDLE) THRD_RUN(threadDispatch, 0);
}

/* 停止运行 */
static void stop() {
	int			done = 0, index = 0;
	number_s	*num = NUM->next;

	if(!CONFIRM("真的要停止计算？")) {
		return;
	}
	if(RUN == 0) {
		return;
	}

	calcEnable(0);

	/* 终止运行线程 */
	TerminateThread(THRD, 0);

	/* 发送终止信号 */
	SetEvent(ANI_STOP);
	WaitForSingleObject(ANI_EXIT, 1200);

	/* 检测是否有已经完成的输出 */
	while(num) {
		if(num->doneCalc) {
			done = 1;
		} else {
			/* 清空显示 */
			listSet(index, 2, "?");
			listSet(index, 3, "?");
			listSet(index, 4, "?");
			listSet(index, 5, "-");
			listSet(index, 6, "-");
			listSet(index, 7, "-");
		}

		index++;
		num = num->next;
	}

	openEnable(1);
	calcEnable(1);
	sseEnable(1);
	saveEnable(done);
	calcText("开始计算");

	RUN		= 0;
	THRD	= NULL;
}


/* 数据处理, 分发到子函数 */
void dispatch() {
	if(RUN) {
		stop();
	} else {
		start();
	}
}

/* 关闭窗口时检测保存状态 */
int close() {
	char		*msg = NULL;
	number_s	*num = NUM->next;

	if(RUN) {
		msg = "程序正在进行数据计算，是否确定要退出？";
	} else {
		while(num) {
			if((num->doneCalc == 0) || (num->doneOutput == 0)) {
				msg = "还有数据没有计算或结果没有保存，是否确定要退出？"; break;
			}

			num = num->next;
		}
	}

	if(msg) {
		return CONFIRM(msg);
	} else {
		return 1;
	}
}
