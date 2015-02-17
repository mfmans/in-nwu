/*
	$ main.c, 0.1.0115
*/

/* @ for: Commctrl.h */
#pragma comment (lib, "ComCtl32.lib")

/* @ 链接器: 子系统=Windows */
#pragma comment (linker, "/subsystem:\"windows\"")
/* @ 链接器: 清单文件 */
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <Windows.h>
#include "base.h"


/* 提示 */
#define ALERT(message)		\
	MessageBox(hWin, TEXT(message), TEXT(""), MB_OK | MB_ICONWARNING)
#define NOTICE(message)		\
	MessageBox(hWin, TEXT(message), TEXT(""), MB_OK | MB_ICONINFORMATION)


/* 主窗口句柄 */
extern HWND hWin;
/* 运行实例 */
extern HINSTANCE instance;

/* 初始序列 */
static unsigned char number[9];
/* 移动步骤和数目 */
static size_t  count	= 0;
static size_t *result	= NULL;


/*
	$ threadCalc		计算线程

	@ void *dummy
*/
void threadCalc(void *dummy) {
	char	*buffer;
	char	message[128];

	size_t	i, position;
	time_t	start, end;

	unsigned char data[9];

	/* 释放上次的计算结果 */
	if(result) {
		free(result);
	}

	start	= clock();
	result	= calculate(number, &count);
	end		= clock();

	/* 不可解 */
	if(result == NULL) {
		if(count == 1) {
			NOTICE("输入的序列为目标序列，无需计算。");
		} else {
			NOTICE("输入的数字序列不可解。");
		}

		/* 演示按钮不可用 */
		enable(1, 0);

		return;
	}

	sprintf(message, "计算完毕，共 %ld 步，耗时 %.3lf 秒。", count, (double) (end - start) / 1000);
	NOTICE(message);

	/* 生成结果 */
	if((buffer = (char *) calloc(count + 1, sizeof(char))) == NULL) {
		exit(0);
	}

	/* 初始化位置 */
	position = number[0];

	/* 计算移动步骤 */
	for(i = 0; i < count; i++) {
		unpack(data, result[i]);

		/* 判断移动的相对位移 */
		switch((int) data[0] - (int) position) {
			case -3: buffer[i] = 'U'; break;
			case -1: buffer[i] = 'L'; break;
			case  1: buffer[i] = 'R'; break;
			case  3: buffer[i] = 'D'; break;
		}

		position = data[0];
	}

	step(buffer);
	free(buffer);

	/* 演示按钮可用 */
	enable(1, 1);
}

/*
	$ threadPlay		演示线程

	@ void *dummy
*/
void threadPlay(void *dummy) {
	size_t i;
	unsigned char data[9];

	/* 恢复初始序列 */
	show(-1, number);

	/* 设置焦点 */
	focus(1);

	/* 动态显示 */
	for(i = 0; i < count; i++) {
		Sleep(1000);

		unpack(data, result[i]);
		show((int) i, data);
	}

	/* 释放焦点 */
	Sleep(1000);
	focus(0);

	/* 恢复按钮和焦点 */
	enable(1, 1);
}


/*
	$ actCalc			计算操作

	@ char *buffer		输入的数字序列字符串缓冲区
	@ size_t length		字符串长度
*/
void actCalc(char *buffer, size_t length) {
	size_t i, j;
	unsigned char input[9] = {0};

	if(buffer == NULL) {
		ALERT("请输入矩阵的初始数字序列！"); return;
	}
	if(length < 9) {
		ALERT("输入的数字序列应包括 8 个数字和空格，因而不能少于 9 个字符。"); return;
	}

	for(i = 0, j = 1; i < 9; i++) {
		switch(buffer[i]) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				if(input[j]) {
					ALERT("输入的数字序列无效，发现重复的数字！"); return;
				} else {
					input[j++] = buffer[i] - '1' + 1;
				}
				break;

			default:
				if(input[0]) {
					ALERT("输入的数字序列无效，发现重复的空格！"); return;
				} else {
					input[0] = i + 1;
				}
				break;
		}
	}

	/* 判断是否输入空格 */
	if(input[0] == 0) {
		ALERT("输入的数字序列无效，没有包含空格！"); return;
	}

	/* 判断数字是否完整或重复 */
	for(i = 1; i < 9; i++) {
		if(input[i] == 0) {
			ALERT("输入的数字序列无效，数字不完整！"); return;
		}

		for(j = i + 1; j < 9; j++) {
			if(input[i] == input[j]) {
				ALERT("输入的数字序列无效，数字有重复！"); return;
			}
		}
	}

	memcpy(number, input, sizeof input);

	/* 禁用按钮 */
	enable(0, 0);
	/* 显示输入 */
	show(-1, number);
	/* 清空结果 */
	step("");

	/* 创建计算线程 */
	_beginthread(threadCalc, 0, NULL);
}

/*
	$ actPlay			演示操作
*/
void actPlay() {
	if((count == 0) || (result == NULL)) {
		return;
	}

	/* 禁用按钮 */
	enable(0, 0);

	/* 创建演示线程 */
	_beginthread(threadPlay, 0, NULL);
}


/*
	$ WinMain

	@ HINSTANCE hInstance
	@ HINSTANCE hPrevInstance
	@ LPSTR lpCmdLine
	@ int nCmdShow

	# int
*/
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	instance = hInstance;

	gui();

	return 0;
}
