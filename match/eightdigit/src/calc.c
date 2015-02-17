/*
	$ calc.c, 0.1.0115
*/

#include "base.h"


/* 目标排列	[(9), 1, 2, 3, 4, 5, 6, 7, 8] */
#define RESULT		0x09053977


/*
	$ soluble				判断问题可解

	# unsigned char *number

	@ size_t				问题可解, 返回封装好的数据, 否则返回 0
*/
static size_t soluble(unsigned char *number) {
	int i, j;
	int inverse = 0;

	/* 计算逆序数 */
	for(i = 1; i < 9; i++) {
		for(j = i + 1; j < 9; j++) {
			if(number[i] > number[j]) {
				inverse++;
			}
		}
	}

	/* 奇排列不可解 */
	if(inverse % 2) {
		return 0;
	}

	/* 封装数据 */
	return pack(number);
}

/*
	$ extend				排列展开

	@ size_t data
*/
static void extend(size_t data) {
	size_t next;

	/* 向上 */
	if(next = moveUp(data)) {
		if(flagSet(next, 1) == 0) {
			queueAdd(next);
		}
	}

	/* 向右 */
	if(next = moveRight(data)) {
		if(flagSet(next, 2) == 0) {
			queueAdd(next);
		}
	}

	/* 向下 */
	if(next = moveDown(data)) {
		if(flagSet(next, 3) == 0) {
			queueAdd(next);
		}
	}

	/* 向左 */
	if(next = moveLeft(data)) {
		if(flagSet(next, 4) == 0) {
			queueAdd(next);
		}
	}
}

/*
	$ steps					生成步骤

	@ size_t input
	@ size_t *count

	# size_t *
*/
static size_t * steps(size_t input, size_t *count) {
	size_t i, j;
	size_t data, direction;

	/* 结果 */
	size_t  size	= 0;
	size_t *result	= NULL;

	/* 尾节点 */
	data = RESULT;

	/* 步骤数 */
	*count = 0;

	/* 循环回溯 */
	while(direction = flagGet(data)) {
		if(direction == 7) {
			break;
		}

		if(*count == size) {
			size += 32;

			if((result = (size_t *) realloc(result, size * sizeof(size_t))) == NULL) {
				exit(0);
			}
		}

		/* 保存结果 */
		result[*count] = data;

		/* 增加步骤数 */
		(*count)++;

		/* 计算父节点 */
		switch(direction) {
			case 1:		data = moveDown(data);	break;
			case 2:		data = moveLeft(data);	break;
			case 3:		data = moveUp(data);	break;
			case 4:		data = moveRight(data);	break;
			default:	data = 0; break;
		}
	}

	/* 判断回溯结果对不对 */
	if(data != input) {
		free(result);

		return NULL;
	}

	/* 转置数组 */
	for(i = 0, size = *count / 2; i < size; i++) {
		j = *count - i - 1;

		data = result[j];

		result[j] = result[i];
		result[i] = data;
	}

	return result;
}


/*
	$ calcuate				排列计算

	@ unsigned char *number
	@ size_t *step			移动步骤数

	# size_t *				可解返回移动过程, 否则返回 NULL
*/
size_t * calculate(unsigned char *number, size_t *step) {
	/* 封装的数字 */
	size_t input, data;

	/* 步骤数 */
	*step = 0;

	/* 问题不可解 */
	if((input = soluble(number)) == 0) {
		return NULL;
	}

	/* 无需计算 */
	if(input == RESULT) {
		*step = 1;
		return NULL;
	}

	/* 初始化所用空间 */
	flagInit();
	queueInit();

	/* 将初始状态压入队列 */
	flagSet(input, 7);
	queueAdd(input);

	/* 队列循环 */
	while(data = queueFetch()) {
		/* 判断是否为结果 */
		if(data == RESULT) {
			break;
		}

		/* 对排列进行展开 */
		extend(data);
	}

	/* 清理队列 */
	queueClear();

	/* 循环错误 */
	if(data == 0) {
		return NULL;
	}

	return steps(input, step);
}
