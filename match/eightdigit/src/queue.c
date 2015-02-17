/*
	$ queue.c, 0.1.0115
*/

#include "base.h"


/* 初始化容量 */
#define SIZE_INIT			1048576			/* 1 MB */
/* 每次扩增的容量 */
#define SIZE_EXTEND			262144			/* 256 KB */


/* 队列操作 */
static int		action	= 0;
/* 队列容量 */
static size_t	size	= 0;

/* 队列空间 */
static size_t	*queue	= NULL;
static size_t	*edge	= NULL;

/* 头尾指针 */
static size_t	*head	= NULL;
static size_t	*tail	= NULL;


/*
	$ queueInit			初始化队列
*/
void queueInit() {
	if(queue) {
		queueClear();
	}

	/* 队列操作 */
	action	= 0;
	/* 队列容量 */
	size	= SIZE_INIT;

	/* 申请队列 */
	queue	= (size_t *) calloc(size, sizeof(size_t));
	/* 队列边界 */
	edge	= queue + size - 1;

	/* 申请失败 */
	if(queue == NULL) {
		exit(0);
	}

	/* 设置队头和队尾 */
	head	= queue;
	tail	= head;
}

/*
	$ queueClear		释放队列
*/
void queueClear() {
	if(queue == NULL) {
		return;
	}

	action	= 0;
	size	= 0;

	edge	= NULL;
	head	= NULL;
	tail	= NULL;

	free(queue);

	queue	= NULL;
}

/*
	$ queueAdd			向队列中添加排列
*/
void queueAdd(size_t data) {
	/* 新空间 */
	size_t *p;
	/* 新旧空间数据量 */
	int offset = 0, left = 0;

	/* 写入数据 */
	*tail = data;

	/* 到达队列边界 */
	if(tail == edge) {
		tail = queue;
	} else {
		tail++;
	}

	/* 队头和队尾重叠 */
	if(tail == head) {
		/* 如果上次操作是添加数据, 说明队满 */
		if(action == 1) {
			offset = 1;
		}
	}

	/* 如果队满, 扩充空间 */
	if(offset) {
		size += SIZE_EXTEND;

		if((p = (size_t *) realloc(queue, size * sizeof(size_t))) == NULL) {
			exit(0);
		}

		/* 计算偏移量 */
		offset = (int) p - (int) queue;

		/* 修正指针 */
		head = (size_t *) ((char *) head + offset);
		edge = (size_t *) ((char *) edge + offset) + 1;

		/* 计算空间 */
		left	= (int) tail - (int) queue;
		offset	= (int) (p + size) - (int) edge;

		/* 移动数据 */
		if(left) {
			if(left <= offset) {
				memmove(edge, p, left);

				/* 新的尾指针 */
				tail = (size_t *) ((char *) edge + left);
			} else {
				memmove(edge, p, offset);
				memmove(p, (char *) p + offset, left - offset);

				/* 新的尾指针向前移动 */
				tail = (size_t *) ((char *) p + (left - offset));
			}
		}

		/* 更新队列指针 */
		queue	= p;
		edge	= queue + size - 1;

		/* 更新尾指针 */
		if(tail > edge) {
			tail = queue;
		}
	}

	action = 1;
}

/*
	$ queueFetch		从队列中取出排列

	# size_t
*/
size_t queueFetch() {
	/* 取出数据 */
	size_t data = *head;

	/* 队列为空 */
	if(head == tail) {
		return 0;
	}

	/* 到达队列边界 */
	if(head == edge) {
		head = queue;
	} else {
		head++;
	}

	/* 设置操作 */
	action = 0;

	return data;
}
