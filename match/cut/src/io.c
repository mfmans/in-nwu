/*
	$ io.c, 0.2.1221
*/

#include "base.h"


/* 默认打开和保存的文件名 */
#define FILE_OPEN		"file.1"
#define FILE_SAVE		"file.2"

/* 允许处理的最大数和最小数 (不含) */
#define MAX			1e10 + 1
#define MIN			0



/* 打开文件 */
static char * open() {
	char *buffer;
	OPENFILENAME file;

	NEW(buffer, char, MAX_PATH + 1);
	ZeroMemory(&file, sizeof file);

	file.lStructSize	= sizeof file;
	file.hwndOwner		= win;
	file.lpstrTitle		= FILE_OPEN;
	file.lpstrFilter	= FILE_OPEN "\0" FILE_OPEN "\0所有文件\0*.*\0";
	file.nFilterIndex	= 1;
	file.lpstrFile		= buffer;
	file.nMaxFile		= MAX_PATH;
	file.Flags			= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if(GetOpenFileName(&file) == FALSE) {
		DEL(buffer);
	}

	return buffer;
}

/* 保存文件 */
static char * save() {
	char *buffer;
	OPENFILENAME file;

	NEW(buffer, char, MAX_PATH + 1);
	ZeroMemory(&file, sizeof file);

	strcpy(buffer, FILE_SAVE);

	file.lStructSize	= sizeof(OPENFILENAME);
	file.hwndOwner		= win;
	file.lpstrTitle		= FILE_SAVE;
	file.lpstrFilter	= FILE_SAVE "\0" FILE_SAVE "\0所有文件\0*.*\0";
	file.nFilterIndex	= 1;
	file.lpstrFile		= buffer;
	file.lpstrDefExt	= "2";
	file.nMaxFile		= MAX_PATH;
	file.Flags			= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&file) == FALSE) {
		DEL(buffer);
	}

	return buffer;
}


void input() {
	/* 输入缓冲 */
	int			number;
	char		buffer[80];
	/* 余数和指数 */
	int_t		remainder, exponent;

	/* 文件 */
	char		*file;
	FILE		*fp;
	/* 链表 */
	number_s	*num = NUM;

	/* 检测现有数据 */
	if(NUM->next) {
		num		= NUM->next;
		number	= 0;

		/* 未完成计算或还没保存的结果 */
		while(num) {
			if((num->doneCalc == 0) || (num->doneOutput == 0)) {
				number = 1; break;
			}

			num = num->next;
		}

		if(number) {
			if(!CONFIRM("还有数据没有计算，或计算的结果没有保存，这时候选择一个文件导入数据意味着放弃这些信息，是否继续？")) {
				return;
			}
		}

		num = NUM;
	}

	if((file = open()) == NULL) {
		return;
	}

	if(fp = fopen(file, "r")) {
		DEL(file);
	} else {
		ALERT("打开选定的文件失败，可能文件不存在，没有访问权限或者正在被其它应用程序使用。");
		DEL(file);

		return;
	}

	/* 删除现有的所有数据 */
	if(NUM->next) {
		while(NUM->next) {
			num = NUM->next->next;

			DELA(NUM->next->data);
			DEL(NUM->next);

			NUM->next = num;
		}

		num = NUM;

		listClear();
	}

	while(!feof(fp)) {
		number = -1;
		
		/* 读取一行 */
		if(fgets(buffer, sizeof buffer, fp) == NULL) {
			break;
		}

		/* 检测输入是否有效 */
		if(sscanf(buffer, "%d", &number) < 1) {
			continue;
		}
		if((number <= MIN) || (number >= MAX)) {
			continue;
		}

		/* 复位 */
		remainder	= (int_t) number;
		exponent	= 0;

		/* 切割数字为 X + 3 * N */
		if(remainder % 3) {
			if(remainder > 4) {
				exponent	= remainder / 3;
				remainder	= remainder % 3;

				if(remainder == 1) {
					exponent--;
					remainder = 4;
				}
			}
		} else {
			exponent	= remainder / 3;
			remainder	= 0;
		}

		NEW(num->next, number_s, 1);

		num				= num->next;
		num->number		= (int_t) number;
		num->remainder	= remainder;
		num->exponent	= exponent;

		listAppend(num);
	}

	if(NUM->next == NULL) {
		ALERT("在所选的文件中没有读取到任何正确的或在处理范围内的数字。");

		calcEnable(0);
	} else {
		calcEnable(1);
	}

	saveEnable(0);

	fclose(fp);
}

void output() {
	/* 输出限制长度 */
	size_t		limit;
	/* 输出长度 */
	size_t		count, length;
	/* 输出缓冲区 */
	char		buffer[16];

	/* 文件 */
	char		*file;
	FILE		*fp;

	/* 当前位指针 */
	int_t		*number;
	/* 链表 */
	number_s	*num = NUM->next;

	/* 检测是否有已处理完成的数据 */
	do {
		while(num) {
			if(num->doneCalc) {
				break;
			}

			num = num->next;
		}
		
		if((num == NULL) || (num->doneCalc == 0)) {
			return;
		}

		num = NUM->next;
	} while(0);

	/* 读取输出长度 */
	if((limit = textRead()) == 0) {
		if(!CONFIRM("输入的保存长度无效，是否保存全长度的计算结果？")) {
			return;
		}
	}

	if((file = save()) == NULL) {
		return;
	}

	if(fp = fopen(file, "wb")) {
		DEL(file);
	} else {
		ALERT("打开选定的文件路径失败，可能没有访问权限。");
		DEL(file);

		return;
	}

	while(num) {
		if(num->doneCalc) {
			count	= num->countDec;
			length	= num->lenHigh;

			/* 最高位 */
			number	= num->data + ((count - 1) * OFFSET) + 1;
			sprintf(buffer, "%lu", *number);

			/* 输出数字 */
			fprintf(fp, "%lu\r\n", num->number);

			/* 若最高位长度大于输出长度 */
			if(limit && (length > limit)) {
				/* 设置 NULL */
				buffer[limit] = 0;

				/* 直接输出 */
				fprintf(fp, "%s", buffer);
			} else {
				/* 先输出最高位 */
				fprintf(fp, "%s", buffer);

				/* 输出剩余位 */
				if(--count) {
					while(count--) {
						/* 输出长度检测 */
						if(limit) {
							/* 超长 */
							if(length >= limit) {
								break;
							}

							/* 不足 */
							if((limit - length) < 9) {
								/* 输出到缓冲区 */
								sprintf(buffer, "%09lu", *(num->data + (count * OFFSET) + 1));
								/* 调整长度 */
								buffer[limit - length] = 0;

								fprintf(fp, "%s", buffer);

								break;
							}
						}

						fprintf(fp, "%09lu", *(num->data + (count * 2) + 1));
						length += 9;
					}
				}
			}

			fprintf(fp, "\r\n%lu\r\n\r\n", num->lenAll);

			num->doneOutput = 1;
		}

		num = num->next;
	}

	fclose(fp);
}
