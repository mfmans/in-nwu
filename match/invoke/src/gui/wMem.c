/*
	$ Invoke   (C) 2005-2012 MF
	$ wMem.c, 0.1.1201
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_MEMORY"
#define TITLE			"查看内存"
#define WIDTH			400
#define HEIGHT			400


/* 显示的字节数 */
#define SIZE			(8 * 15)


static void guiMemDraw(erp_window_s *win) {
	EDIT(eMemAddress)->setText("%08X", win->param);
	BUTTON(bMemRead)->click();
}

void guiMem(HWND win, DWORD address) {
	ZeroMemory(&wMem, sizeof wMem);

	wMem.classname	= CLASSNAME;
	wMem.title		= TITLE;
	wMem.width		= WIDTH;
	wMem.height		= HEIGHT;
	wMem.instance	= RT.instance;
	wMem.draw		= guiMemDraw;
	wMem.center		= TRUE;
	wMem.param		= address;
	wMem.parent		= win;

	eMemAddress		= DRAW_EDIT		(wMem, MEM_EDIT_ADDRESS,	 15,  15, 120,  22, NULL);
	eaMemData		= DRAW_EDITAREA	(wMem, MEM_EDITAREA_DATA,	 15,  50, 363, 250, NULL);

	bMemRead		= DRAW_BUTTON	(wMem, MEM_BUTTON_READ,		-15,  12,  70,  26, "读取",	cbMemButtonRead);
	bMemWrite		= DRAW_BUTTON	(wMem, MEM_BUTTON_WRITE,	-95,  12,  70,  26, "写入",	cbMemButtonWrite);
	bMemOk			= DRAW_BUTTON	(wMem, MEM_BUTTON_OK,		-15, -15, 100,  28, "确定",	cbMemButtonOk);

	erpWindow(&wMem);
}


ERP_CALLBACK(cbMemButtonRead) {
	int i, j;
	DWORD address;

	/* 缓冲区 */
	char *input;
	char data[SIZE];
	char buffer[SIZE * 10] = {0};

	/* 读取地址 */
	input	= EDIT(eMemAddress)->getText();
	i		= sscanf(input, "%x", &address);

	DEL(input);

	if(i <= 0) {
		warning(win->hwnd, "输入的地址无效。"); return;
	}

	/* 读取内容 */
	processRead(address, data, SIZE);

	for(i = j = 1; i <= SIZE; i++) {
		sprintf(buffer + strlen(buffer), "%02X", (unsigned char) data[i - 1]);

		/* 换行 */
		if(i % 8 == 0) {
			strcat(buffer, "\r\n");

			if(j % 5 == 0) {
				strcat(buffer, "\r\n");
			}

			j++;
		} else if(i % 4 == 0) {
			strcat(buffer, "        ");
		} else {
			strcat(buffer, "    ");
		}
	}

	EDITAREA(eaMemData)->setText("%s", buffer);
}

ERP_CALLBACK(cbMemButtonWrite) {
	int		i, j, k;
	DWORD	address, read;
	size_t	length;

	char	*buffer;
	char	data[SIZE], source[3];

	/* 读取地址 */
	buffer	= EDIT(eMemAddress)->getText();
	i		= sscanf(buffer, "%x", &address);

	DEL(buffer);

	if(i <= 0) {
		warning(win->hwnd, "输入的地址无效。"); return;
	}

	i = j = 0;
	buffer = EDITAREA(eaMemData)->getText();
	length = strlen(buffer);

	if(length) {
		while((i < SIZE) && (j < (int) length)) {
			/* 空格判断 */
			if((buffer[j] == 0x20) || ((buffer[j] >= 0x09) && (buffer[j] <= 0x0D))) {
				j++; continue;
			}

			/* 剩余字数 */
			k = length - j;

			/* 复制最多 2 个字节 */
			if(k < 2) {
				strncpy(source, buffer + j, k);
			} else {
				strncpy(source, buffer + j, 2);
			}

			/* 读取 2 个字节 */
			if(sscanf(source, "%x", &read) == 0) {
				j++; continue;
			}

			/* 保存 */
			data[i] = (char) (read & 0x000000FF);

			i++;
			j += 2;
		}
	}

	DEL(buffer);

	if(i == 0) {
		warning(win->hwnd, "从输入的内容中没有读到任何有效的数据。"); return;
	} else if(i != SIZE) {
		if(!confirmFormat(win->hwnd, "仅读到 %d 字节的数据，是否保存？", i)) {
			return;
		}
	}

	if(i = processWrite(address, data, i)) {
		noticeFormat(win->hwnd, "向内存地址 %08X 成功写入 %d 字节数据。", address, i);

		BUTTON(bMemRead)->click();
	} else {
		warningFormat(win->hwnd, "向内存地址 %08X 写入数据失败。", address);
	}
}

ERP_CALLBACK(cbMemButtonOk) {
	win->quit(0);
}
