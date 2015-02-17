/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ mouse.c
*/

#include "base.h"
#include "game.h"


/* 窗口位置 */
static RECT	rect;

/* 第一个方格位置 */
static int	x0, y0;
/* 每个方格的大小 */
static int	w0, h0, w1, h1;

/* 提示的两个图标位置 */
int x1, y1;
int x2, y2;
/* 点击鼠标后发送同步信号 */
int bpclick;


/* 单击 */
static void click(int x, int y, int right) {
	DWORD dw, up;

	if(right) {
		dw = MOUSEEVENTF_RIGHTDOWN;
		up = MOUSEEVENTF_RIGHTUP;
	} else {
		dw = MOUSEEVENTF_LEFTDOWN;
		up = MOUSEEVENTF_LEFTUP;
	}

	/* 激活窗口 */
	SetForegroundWindow(form);

	/* 获取窗口位置 */
	GetWindowRect(form, &rect);
	Sleep(50);

	/* 计算负数 x, y */
	if(x < 0) {
		x = rect.right - rect.left + x;
	}
	if(y < 0) {
		y = rect.bottom - rect.top + y;
	}

	/* 移动到指定位置 */
	SetCursorPos(rect.left + x, rect.top + y);
	Sleep(50);

	/* 按下鼠标 */
	mouse_event(dw, 0, 0, 0, 0);
	Sleep(50);

	/* 释放鼠标 */
	mouse_event(up, 0, 0, 0, 0);
	Sleep(20);
}

/* 单击某个方块 */
static void map(int x, int y) {
	click(x0 + x * w0 + w1, y0 + y * h0 + h1, 0);
}

/* 点击空白处 */
static void blank() {
	click(-50, 50, 1);
}


/* 要求用户点击两个图标以确定位置 */
static int init() {
	int		success;
	int		datint;
	float	datfloat;

	POINT	mouse;
	DWORD	symbol;

	/* 信号重置 */
	RESET(Run);
	/* 设置状态 */
	bpclick = -1;
	
	/* 获取当前鼠标位置 */
	GetCursorPos(&mouse);

	/* 点击右键释放焦点 */
	blank();
	/* 点击左键触发断点 */
	click(-50, 50, 0);

	/* 移动回原来的位置 */
	SetCursorPos(mouse.x, mouse.y);

	/* 等待点击响应 */
	TIMEOUT(symbol, Run, 300) {
		;
	} else {
		return 0;
	}

	do {
		success = 0;

		/* 图标宽度 */
		if(memoryRead(GAME_ICON_W, sizeof datint, &datint)) {
			w0 = datint;
			w1 = w0 / 2;
		} else {
			break;
		}
		/* 图标宽度 */
		if(memoryRead(GAME_ICON_H, sizeof datint, &datint)) {
			h0 = datint;
			h1 = h0 / 2;
		} else {
			break;
		}

		/* 左边距 */
		if(memoryRead(bpclick + 0x30, sizeof datint, &datfloat)) {
			x0 = (int) datfloat;
		} else {
			break;
		}
		/* 上边距 */
		if(memoryRead(bpclick + 0x34, sizeof datint, &datfloat)) {
			y0 = (int) datfloat;
		} else {
			break;
		}

		success = 1;
	} while(0);

	/* 恢复状态 */
	bpclick = 0;

	RESET(Run);
	SEND(Stop);

	return success;
}


THREAD(mouse) {
	DWORD	symbol;
	POINT	mouse;

	/* 上次记录 */
	int		oldx1, oldy1, oldx2, oldy2;
	/* 重试次数 */
	int		retry = 0, duplicate = 0;

	/* 重置信号 */
	RESET(Run);

	/* 重置坐标 */
	oldx1 = oldy1 = 0;
	oldx2 = oldy2 = 0;

	/* 获取第一格位置 */
	if(init() == 0) {
		buttonRun(1);
		return;
	}

	/* 保存指针 */
	GetCursorPos(&mouse);
	/* 在空白处单击 */
	blank();

	while(1) {
		/* 点击提示按钮 */
		click(GAME_HINT_X, GAME_HINT_Y, 0);

		/* 成功信号 */
		TIMEOUT(symbol, Run, 200) {
			retry = 0;

			/* 总是重复点击同一个地方 */
			if((x1 == oldx1) && (y1 == oldy1) && (x2 == oldx2) && (y2 == oldy2)) {
				if((++duplicate) >= 3) {
					break;
				}
			} else {
				duplicate = 0;

				oldx1 = x1;
				oldy1 = y1;
				oldx2 = x2;
				oldy2 = y2;
			}

			/* 点击 x1 */
			map(x1, y1);
			Sleep(100);

			/* 点击 x2 */
			map(x2, y2);
			Sleep(100);
		}

		/* 失败信号 */
		TIMEOUT(symbol, Stop, 0) {
			break;
		}

		/* 错误次数 */
		if((++retry) < 3) {
			continue;
		}

		break;
	}

	/* 恢复鼠标 */
	SetCursorPos(mouse.x, mouse.y);

	/* 恢复按钮 */
	buttonRun(1);
}
