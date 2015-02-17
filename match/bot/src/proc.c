/*
	$ Robot for LinLink   (C) 2005-2013 MF
	$ proc.c
*/

#include <Windows.h>
#include <Psapi.h>

#include "base.h"
#include "game.h"


/* 最大进程数量 */
#define COUNT		1024
/* 最大进程路径 */
#define BUFFER		(MAX_PATH * 2)


/* 游戏主窗口 */
HWND	form	= NULL;
/* 游戏进程句柄 */
HANDLE	process	= NULL;


/* 枚举进程 */
static DWORD processFind() {
	DWORD	pid[COUNT];
	char	name[BUFFER];

	HANDLE	handle;
	DWORD	i, size, length;

	/* 枚举进程 */
	if(EnumProcesses(pid, COUNT, &size) == FALSE) {
		return 0;
	}

	/* 计算进程数目 */
	if(size) {
		size = size / sizeof(DWORD);
	} else {
		return 0;
	}

	/* 读取进程路径 */
	for(i = 0; i < size; i++) {
		if((handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid[i])) == NULL) {
			continue;
		}
		
		ZeroMemory(&name, sizeof name);
		GetProcessImageFileName(handle, name, BUFFER);

		CloseHandle(handle);

		if((length = strlen(name)) == 0) {
			continue;
		}
		if(length < strlen(GAME_PROCESS)) {
			continue;
		}

		/* 比较文件名 */
		if(lstrcmpi(name + length - strlen(GAME_PROCESS), GAME_PROCESS) == 0) {
			return pid[i];
		}
	}

	return 0;
}

/* 附加进程 */
static int processAttach(DWORD pid) {
	/* 打开目标进程 */
	if((process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)) == NULL) {
		return 0;
	}

	/* 附加调试 */
	if(DebugActiveProcess(pid) == FALSE) {
		return 0;
	}

	return 1;
}

/* 进程验证 */
static int processValidate() {
	if(memoryEqualV(GAME_CLICK_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_1_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_2_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_3_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_4_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_F_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_HINT_S_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_LIFE_1_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_LIFE_2_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_LIFE_3_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_LIFE_4_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_PASS_1_DATA) == 0) {
		return 0;
	}
	if(memoryEqualV(GAME_PASS_2_DATA) == 0) {
		return 0;
	}

	return 1;
}

/* 获取主窗口 */
static int processWindow(DWORD pid) {
	DWORD processid;

	if((form = FindWindow(TEXT("HGE__WNDCLASS"), NULL)) == NULL) {
		return 0;
	}

	/* 获取窗口 pid */
	GetWindowThreadProcessId(form, &processid);

	if(pid == processid) {
		return 1;
	} else {
		return 0;
	}
}

/* 安装断点 */
static int processInstall() {
	/* NOP */
	if(memorySet(GAME_HINT_1_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_HINT_2_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_HINT_3_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_HINT_4_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_LIFE_1_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_LIFE_2_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_LIFE_3_SIZE, 0x90) == 0) {
		return 0;
	}
	if(memorySet(GAME_LIFE_4_SIZE, 0x90) == 0) {
		return 0;
	}

	/* INT 3 */
	if(memorySet(GAME_CLICK_SIZE, 0xCC) == 0) {
		return 0;
	}
	if(memorySet(GAME_HINT_F_SIZE, 0xCC) == 0) {
		return 0;
	}
	if(memorySet(GAME_HINT_S_SIZE, 0xCC) == 0) {
		return 0;
	}
	if(memorySet(GAME_PASS_1_SIZE, 0xCC) == 0) {
		return 0;
	}
	if(memorySet(GAME_PASS_2_SIZE, 0xCC) == 0) {
		return 0;
	}

	return 1;
}


int procMount() {
	DWORD	pid;
	int		success = 0;

	do {
		/* 找 pid */
		if((pid = processFind()) == 0) {
			WARNING("没有找到游戏进程，请检查游戏是否正在运行。");			break;
		}

		/* 附加调试器 */
		if(processAttach(pid) == 0) {
			WARNING("无法挂载到游戏进程上，请检查本程序的运行权限。");		break;
		}

		/* 验证程序 */
		if(processValidate() == 0) {
			WARNING("找到的游戏进程未通过内容验证，可能是版本不正确。");		break;
		}

		/* 获取主窗口 */
		if(processWindow(pid) == 0) {
			WARNING("无法找到游戏进程的主窗口，可能是版本不正确。");			break;
		}

		/* 安装断点 */
		if(processInstall() == 0) {
			WARNING("修改游戏进程内容失败，游戏可能无法正常运行。");			break;
		}

		success = 1;
	} while(0);
	
	/* 成功 */
	if(success) {
		buttonRun(1);
		return 1;
	}
	
	/* 解除绑定 */
	if(pid) {
		DebugSetProcessKillOnExit(FALSE);
		DebugActiveProcessStop(pid);

		DebugSetProcessKillOnExit(TRUE);
	}

	if(process) {
		CloseHandle(process);
	}

	form	= NULL;
	process	= NULL;

	buttonLoad(1);

	return 0;
}


void procAddress(size_t address, CONTEXT *reg) {
	switch(address) {
		/* 单击鼠标 */
		case GAME_CLICK_ADDR:
			reg->Esi = reg->Ecx;
			reg->Eip = GAME_CLICK_ADDR + 2;

			if(bpclick == -1) {
				/* 发送消息 */
				SEND(Run);

				/* 设置 Esi */
				bpclick = (int) reg->Ecx;

				/* 等待处理回调 */
				WAIT(Stop);
			}

			break;

		/* 成功获取提示 */
		case GAME_HINT_S_ADDR:
			reg->Eax = reg->Ebx;
			reg->Eip = GAME_HINT_S_ADDR + 2;

			/* 设置坐标 */
			x1 = (int) reg->Esi;
			y1 = (int) reg->Ebp;
			x2 = (int) reg->Edi;
			y2 = (int) reg->Ebx;

			SEND(Run);
			break;

		/* 获取失败 */
		case GAME_HINT_F_ADDR:
			reg->Eax = 0;
			reg->Eip = GAME_HINT_F_ADDR + 2;

			SEND(Stop);
			break;

		/* 本关结束 */
		case GAME_PASS_1_ADDR:
		case GAME_PASS_2_ADDR:
			reg->Ecx = reg->Esp;
			reg->Eip = address + 2;

			SEND(Stop);
			break;
	}
}


void procRestore() {
	/* 解绑 */
	DebugSetProcessKillOnExit(FALSE);

	/* 恢复内存数据 */
	memoryWriteV(GAME_CLICK_DATA);
	memoryWriteV(GAME_HINT_1_DATA);
	memoryWriteV(GAME_HINT_2_DATA);
	memoryWriteV(GAME_HINT_3_DATA);
	memoryWriteV(GAME_HINT_4_DATA);
	memoryWriteV(GAME_HINT_F_DATA);
	memoryWriteV(GAME_HINT_S_DATA);
	memoryWriteV(GAME_LIFE_1_DATA);
	memoryWriteV(GAME_LIFE_2_DATA);
	memoryWriteV(GAME_LIFE_3_DATA);
	memoryWriteV(GAME_LIFE_4_DATA);
	memoryWriteV(GAME_PASS_1_DATA);
	memoryWriteV(GAME_PASS_2_DATA);
}
