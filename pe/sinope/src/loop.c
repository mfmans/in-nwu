/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ loop.c, 0.1.1104
*/

#include <Windows.h>
#include "base.h"
#include "image.h"
#include "gui.h"
#include "action.h"
#include "libdisasm/libdis.h"


/* 正在执行的指令 */
static struct {
	DWORD		eip;
	BOOL		main;
	BOOL		dynamic;

	section_s	*section;
	module_s	*module;

	/* 线程 */
	thread_s	*thread;
	/* 寄存器 */
	CONTEXT		reg;
} command = {0, FALSE, FALSE, NULL, NULL, {0}};


/* 模式切换 */
static void loopMode(BOOL e) {
	BOOL d = e ? FALSE : TRUE;

	enable(TRUE, d, e, d);
}

/* 当前执行指令 */
static void loopCommand() {
	char	output[64] = {0};
	DWORD	rva;

	if((command.main == FALSE) && (CONFIG.dllEip == FALSE)) {
		return;
	}

	EDIT(eEip)->setText("%08X", command.eip);

	if(rva = addr2rva(command.eip)) {
		EDIT(eRva)->setText("%08X", rva);
	} else {
		EDIT(eRva)->setText("-");
	}

	if(command.section) {
		if(command.section->name) {
			EDIT(eSection)->setText(command.section->name);
		} else {
			EDIT(eSection)->setText("@ %08X", command.section->addr);
		}
	} else {
		EDIT(eSection)->setText("-");
	}

	loopDisasm(FALSE);
}


/* 断点表检测 */
static BOOL loopBreakpoint() {
	bp_s *bp = BP->next;

	while(bp) {
		/* 中断成功 */
		if((bp->addrFrom <= command.eip) && (bp->addrTo >= command.eip)) {
			bpActive(bp->index); return TRUE;
		}

		bp = bp->next;
	}

	return FALSE;
}

/* 动态代码检测 */
static BOOL loopDynmCode() {
	char		buffer[10]	= {0};
	size_t		size		= 0;

	switch(CONFIG.bp.dynamicCodeMode) {
		/* 总是检查 */
		case 0:
			break;

		/* 跨区段检查 */
		case 1:
			if(RUNTIME.proc.tBreak == NULL) {
				return FALSE;
			}
			if(command.section == RUNTIME.proc.tBreak->run.section) {
				return FALSE;
			}
			break;

		/* 返回领空 */
		case 2:
			if(RUNTIME.proc.tBreak == NULL) {
				return FALSE;
			}
			if((command.main == FALSE) || (RUNTIME.proc.tBreak->run.main == TRUE)) {
				return FALSE;
			}
			break;
	}

	if((command.section == NULL) || (command.section->copy == NULL)) {
		return FALSE;
	}

	/* 计算比较大小 */
	size = (command.section->addr + command.section->sizeImage) - command.eip;
	size = (size > sizeof buffer) ? sizeof buffer : size;

	if(size > 0) {
		if(memoryRead(command.eip, buffer, size)) {
			/* 内存比较 */
			if(memcmp(buffer, (char *) command.section->copy + (command.eip - command.section->addr), size)) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 检测动态内存 */
static BOOL loopDynmMemory() {
	if((RUNTIME.proc.tBreak == NULL) || (command.section == NULL)) {
		return FALSE;
	}
	if((command.section->dynamic == FALSE) || (command.section->file == FALSE)) {
		return FALSE;
	}
	if((RUNTIME.proc.tBreak->run.section->dynamic == TRUE) && (RUNTIME.proc.tBreak->run.section->file == TRUE)) {
		return FALSE;
	}

	return TRUE;
}

/* 跨区段跳转 */
static BOOL loopStride() {
	if(RUNTIME.proc.tBreak == NULL) {
		return FALSE;
	}
	if(command.section == RUNTIME.proc.tBreak->run.section) {
		return FALSE;
	}

	/* 从领空外跳回来不算 */
	if(RUNTIME.proc.tBreak->run.main == FALSE) {
		return FALSE;
	}
	if(command.main == FALSE) {
		return FALSE;
	}

	return TRUE;
}


static BOOL loopPause() {
	/* 进入暂停状态 */
	loopMode(TRUE);
	procTitle("暂停中");

	/* 反汇编 */
	loopDisasm(TRUE);

	/* 挂起 */
	while(RUNTIME.running == FALSE) {
		/* 退出进程 */
		if(RUNTIME.alive == FALSE) {
			loopTerminate(); return FALSE;
		}

		Sleep(1);
	}

	/* 恢复运行 */
	loopMode(FALSE);
	procTitle("运行中");

	WORKING_SET("进程恢复运行。");

	return TRUE;
}

static int loopWait() {
	DEBUG_EVENT	debug;
	char		*msg = NULL;

	while(1) {
		/* 检测停止 */
		if(RUNTIME.alive == FALSE) {
			loopTerminate(); return -1;
		}

		/* 暂停 */
		if(RUNTIME.running == FALSE) {
			if(loopPause() == FALSE) {
				return -1;
			}
		}

		/* 等待 */
		if(WaitForDebugEvent(&debug, 1)) {
			break;
		}
	}

	/* 当前线程 */
	if(command.thread = threadGet(debug.dwThreadId)) {
		/* 获取寄存器 */
		GetThreadContext(command.thread->handle, &command.reg);

		/* 获取信息 */
		command.eip		= command.reg.Eip;
		command.main	= isPeModule(command.eip);
		command.section	= getSectionByAddr(command.eip);
		command.module	= getModuleByAddr(command.eip);
	} else {
		if(debug.dwDebugEventCode != CREATE_THREAD_DEBUG_EVENT) {
			loopTerminate(); return -1;
		}
	}

	/* 更新指令区 */
	loopCommand();

	switch(debug.dwDebugEventCode) {
		/* 线程创建 */
		case CREATE_THREAD_DEBUG_EVENT:
			threadCreate(
				/* handle */	debug.u.CreateThread.hThread,
				/* threadid */	GetThreadId(debug.u.CreateThread.hThread),
				/* address */	(DWORD) debug.u.CreateThread.lpStartAddress
			);

			threadFlush();

			/* 新线程 */
			command.thread = threadGet(debug.dwThreadId);

			/* 初始化寄存器 */
			command.reg.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
			GetThreadContext(command.thread->handle, &command.reg);

			return 0;

		/* 异常 */
		case EXCEPTION_DEBUG_EVENT:
			break;

		/* DLL 加载 */
		case LOAD_DLL_DEBUG_EVENT:
			CloseHandle(debug.u.LoadDll.hFile); return 1;

		/* 线程退出 */
		case EXIT_THREAD_DEBUG_EVENT:
			threadExit(threadGet(debug.dwThreadId)->handle); return 0;

		/* 进程退出 */
		case EXIT_PROCESS_DEBUG_EVENT:
			return -1;

		default:
			return 0;
	}
	
	switch(debug.u.Exception.ExceptionRecord.ExceptionCode) {
		case EXCEPTION_SINGLE_STEP:
			break;

		default:
			return 0;
	}

	/* 判断是不是需要调试的线程 */
	if(RUNTIME.proc.tDebug != NULL) {
		if(RUNTIME.proc.tBreak->id != debug.dwThreadId) {
			return 1;
		}
	}

	/* 断点判断 */
	do {
		if(RUNTIME.singleStep) {
			RUNTIME.singleStep = FALSE;

			msg = "单步执行中断。"; break;
		}

		if(loopBreakpoint()) {
			msg = "触发了断点。"; break;
		}

		if(CONFIG.bp.dynamicCode) {
			if(loopDynmCode()) {
				msg = "执行被动态修改的代码。"; break;
			}
		}
		if(CONFIG.bp.dynamicMemory) {
			if(loopDynmMemory()) {
				msg = "跳转到动态申请内存中。"; break;
			}
		}
		if(CONFIG.bp.stride) {
			if(loopStride()) {
				msg = "跨区段跳转。"; break;
			}
		}
	} while(0);

	/* 保存状态 */
	RUNTIME.proc.tBreak					= command.thread;
	RUNTIME.proc.tBreak->run.eip		= command.eip;
	RUNTIME.proc.tBreak->run.main		= command.main;
	RUNTIME.proc.tBreak->run.dynamic	= command.dynamic;
	RUNTIME.proc.tBreak->run.section	= command.section;
	RUNTIME.proc.tBreak->run.module		= command.module;

	if(msg) {
		/* 更新状态 */
		RUNTIME.running = FALSE;

		/* 更新提示 */
		WORKING_SET(msg);

		/* 提示音 */
		if(CONFIG.beep) {
			MessageBeep(MB_ICONASTERISK);
		}

		/* 进入暂停状态 */
		if(loopPause() == FALSE) {
			return -1;
		}

		bpActive(-1);
	}

	return 1;
}

static void loopContinue(BOOL handled) {
	DWORD	status;

	/* 设置寄存器 */
	command.reg.Dr0 = 0;
	command.reg.Dr7 = 0;
	command.reg.EFlags |= 0x100;

	if(handled) {
		status = DBG_CONTINUE;
	} else {
		status = DBG_EXCEPTION_NOT_HANDLED;
	}

	/* 设置寄存器 */
	SetThreadContext(command.thread->handle, &command.reg);

	/* 继续调试 */
	ContinueDebugEvent(RUNTIME.proc.pid, command.thread->id, status);
}


void loop() {
	/* 初始化 command */
	command.reg.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	while(1) {
		switch(loopWait()) {
			/* 进程退出 */
			case -1: return;

			/* 未处理 */
			case 0: loopContinue(FALSE); break;

			/* 已处理 */
			case 1: loopContinue(TRUE); break;
		}
	}
}

void loopTerminate() {
	DebugActiveProcessStop(RUNTIME.proc.pid);
	TerminateProcess(RUNTIME.proc.handle, 0);
}

/* 反汇编 */
void loopDisasm(BOOL disasm) {
	x86_insn_t		insn;

	int				i;
	char			output[128] = {0};
	unsigned char	buffer[10];

	if(memoryRead(command.eip, buffer, sizeof buffer) == FALSE) {
		output[0] = '-';
	} else {
		if((disasm == TRUE) && (CONFIG.disasm == TRUE)) {
			if(x86_disasm(buffer, sizeof buffer, addr2rva(command.eip), 0, &insn)) {
				x86_format_insn(&insn, output, sizeof output, intel_syntax);

				x86_oplist_free(&insn);
			}
		}

		if(output[0] == 0) {
			for(i = 0; i < sizeof buffer; i++) {
				sprintf(output + i * 3, "%02X ", buffer[i]);
			}
		}
	}

	EDIT(eCode)->setText(output);
}
