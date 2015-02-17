/*
	$ Invoke   (C) 2005-2012 MF
	$ run.c, 0.1.1202
*/

#include <Windows.h>
#include <Psapi.h>
#include "base.h"
#include "gui.h"


/* 运行状态 */
static int			RUN = -1;

/* 调试信息 */
static DEBUG_EVENT	debug;
/* 寄存器信息 */
static CONTEXT		reg;


static void registerRead() {
	ZeroMemory(&reg, sizeof reg);

	reg.ContextFlags = CONTEXT_ALL;

	if(GetThreadContext(threadGet(debug.dwThreadId), &reg) == FALSE) {
		die("Invoke 出现错误，无法获取活动线程的寄存器信息，应用程序已被迫终止。");
	}
}

static void registerSave() {
	if(SetThreadContext(threadGet(debug.dwThreadId), &reg) == FALSE) {
		die("Invoke 出现错误，无法设置活动线程的寄存器信息，应用程序已被迫终止。");
	}
}


static void runShow(function_s *func, BOOL justret) {
	int		i, mode, input;
	char	*buffer;

	DWORD		enter;
	argument_s	*arg;

	/* 设置运行信息 */
	RT.thread	= threadGet(debug.dwThreadId);
	RT.function	= func;

	/* 激活窗口 */
	activeWindow(wMain.hwnd);

	guiMainStatus(TRUE, FALSE, FALSE);

	if(justret) {
		OBJECT(eMainEsp)->disable(TRUE);
		OBJECT(eMainRet)->disable(TRUE);

		OBJECT(bMainCalc)->disable(TRUE);

		OBJECT(bMainContinue)->disable(TRUE);
		OBJECT(bMainRuntoret)->disable(TRUE);
		OBJECT(bMainRuntocon)->disable(TRUE);
	} else {
		guiMainTitle(func->module->modname, func->name, func->ordinal);

		EDIT(eMainModule)->setText("%s", func->module->modname);
		EDIT(eMainOrdinal)->setText("%ld", func->ordinal);

		if(func->name) {
			EDIT(eMainFuncname)->setText("%s", func->name);
		} else {
			EDIT(eMainFuncname)->setText("");
		}

		EDIT(eMainEsp)->setText("%08X", reg.Esp);

		OBJECT(eMainEsp)->disable(FALSE);
		OBJECT(eMainRet)->disable(FALSE);
		OBJECT(bMainCalc)->disable(FALSE);

		BUTTON(bMainCalc)->click();
	}

	EDIT(eMainEax)->setText("%08X", reg.Eax);

	/* 设置信号 */
	while(1) {
		symbolReturn(-1);

		/* 执行模式 */
		if((mode = symbolWait()) == 0) {
			break;
		}

		if(justret) {
			buffer	= EDIT(eMainEax)->getText();
			input	= sscanf(buffer, "%x", &enter);

			DEL(buffer);

			if(input <= 0) {
				warning(wMain.hwnd, "输入的返回内容无效。"); continue;
			}

			registerRead();
			reg.Eax = enter;
			registerSave();

			guiMainTitle(NULL, NULL, 0);
		} else {
			buffer	= EDIT(eMainRet)->getText();
			input	= sscanf(buffer, "%x", &enter);

			DEL(buffer);

			if(input <= 0) {
				warning(wMain.hwnd, "输入的返回地址无效。"); continue;
			}

			if(mode == 3) {
				if((func == NULL) || (func->hijack == NULL)) {
					warning(wMain.hwnd, "函数原型不存在，无法直接返回。"); continue;
				}

				registerRead();

				buffer	= EDIT(eMainEax)->getText();
				input	= sscanf(buffer, "%x", &reg.Eax);

				DEL(buffer);

				if(input <= 0) {
					warning(wMain.hwnd, "输入的返回内容无效。"); continue;
				}

				if(RT.mainStart && RT.mainEnd) {
					if((enter < RT.mainStart) || (enter >= RT.mainEnd)) {
						if(!confirmFormat(wMain.hwnd, "直接返回意味着不会进入函数内部执行，此时 EAX 的设置值相当于函数的返回值，堆栈 ESP 和 EBP 的值将会根据函数原型设置进行平衡。\n\n返回地址：%08X \n主模块地址：%08X - %08X\n\n输入的返回地址不在主模块地址范围内，是否继续？", enter, RT.mainStart, RT.mainEnd)) {
							continue;
						}
					}
				} else {
					if(!confirmFormat(wMain.hwnd, "直接返回意味着不会进入函数内部执行，此时 EAX 的设置值相当于函数的返回值，堆栈 ESP 和 EBP 的值将会根据函数原型设置进行平衡。\n\n无法确认输入的返回地址 %08X 是否在主模块地址范围内，是否继续？", enter)) {
						continue;
					}
				}

				reg.Eip = enter;

				/* 执行 pop ebx */
				if(func->hijack->esp) {
					processRead(reg.Esp, &enter, sizeof enter);

					reg.Ebp  = enter;
					reg.Esp += 4;
				}

				/* ESP 减去返回地址 */
				reg.Esp += 4;

				/* 调用模式 */
				switch(func->hijack->invoke) {
					/* cdecl, 调用者平衡 */
					case CL_CDECL:
						break;

					/* stdcall, 被调用者平衡 */
					case CL_STDCALL:
						if(arg = func->hijack->argList) {
							while(arg) {
								reg.Esp += arg->type.size;
								arg = arg->next;
							}
						}
						break;

					/* fastcall, 前两个参数来自 ECX/EDX, 被调用者平衡 */
					case CL_FASTCALL:
						i = 0;

						if(arg = func->hijack->argList) {
							while(arg) {
								if(i > 1) {
									reg.Esp += arg->type.size;
								}

								i++;
								arg = arg->next;
							}
						}

						break;
				}
				
				registerSave();
			} else {
				/* 使用硬件断点 */
				registerRead();
				reg.Dr0 = enter;
				reg.Dr7 = 0x101;
				registerSave();

				if(mode == 1) {
					threadRetSet(debug.dwThreadId, enter, TRUE);
				} else if(mode == 2) {
					threadRetSet(debug.dwThreadId, enter, FALSE);
				}
			}
		}

		break;
	}

	/* 恢复窗口状态 */
	guiMainStatus(FALSE, TRUE, TRUE);
}


static void runDebugProcessExit() {
	notice(wMain.hwnd, "正在调试的进程已经中止运行，点击确定 Invoke 将会自动退出。");

	ExitProcess(EXIT_SUCCESS);
}

static void runDebugThreadCreate() {
	threadCreate(debug.dwThreadId, debug.u.CreateThread.hThread);
}

static void runDebugThreadExit() {
	threadExit(debug.dwThreadId);
}

static void runDebugDllLoad() {
	int check = 0;
	module_s *module;
	
	/* 创建模块 */
	module = moduleLoad(debug.u.LoadDll.hFile, (DWORD) debug.u.LoadDll.lpBaseOfDll);
	/* 关闭句柄 */
	CloseHandle(debug.u.LoadDll.hFile);

	if(module == NULL) {
		return;
	}

	/* 推荐设置模块 */
	if(strEqual(module->modname, "user32.dll")) {
		check = 1;
	}

	/* 断点窗口 */
	if(guiBp(wMain.hwnd, module->modname, &module, 1, check)) {
		/* 劫持窗口 */
		guiHij(wMain.hwnd);
	}
}

static void runDebugDllUnload() {
	moduleDelete((DWORD) debug.u.UnloadDll.lpBaseOfDll);
}

static void runDebugExSglStep() {
	BOOL retBk;
	function_s	*func;

	/* 断点地址 */
	DWORD		address = (DWORD) debug.u.Exception.ExceptionRecord.ExceptionAddress;

	EDIT(eMainBp)->setText("%08X", address);

	if(func = threadFunctionGet(debug.dwThreadId)) {
		/* 恢复上次进入的函数断点 */
		hookSetBp(func);
		threadFunctionSet(debug.dwThreadId, NULL);

		/* 恢复其它线程 */
		threadResume(debug.dwThreadId);
	}

	/* 返回点 */
	if(address == threadRetGet(debug.dwThreadId, &retBk)) {
		registerRead();
		reg.Dr0 = 0;
		reg.Dr7 = 0;
		reg.Eip = address;
		registerSave();

		threadRetSet(debug.dwThreadId, 0, FALSE);

		if(retBk) {
			runShow(NULL, TRUE);
		} else {
			EDIT(eMainEax)->setText("%08X", reg.Eax);
		}
	}
}

static int runDebugExBkPoint() {
	module_s	*module;
	function_s	*function, *oldfunc;

	/* 断点地址 */
	DWORD		address = (DWORD) debug.u.Exception.ExceptionRecord.ExceptionAddress;

	EDIT(eMainBp)->setText("%08X", address);

	/* 恢复上次进入函数的断点 */
	if(oldfunc = threadFunctionGet(debug.dwThreadId)) {
		hookSetBp(oldfunc);
		threadFunctionSet(debug.dwThreadId, NULL);
	}

	if(((function = hookFind(address, &module)) == NULL) || (function->hook == FALSE)) {
		if(debug.u.Exception.dwFirstChance) {
			//warningFormat(wMain.hwnd, "触发了一个断点，位置在 %08X。", address);
			
			return 0;
		} else {
			return 1;
		}
	}

	/* 设置本次进入函数 */
	threadFunctionSet(debug.dwThreadId, function);
	/* 恢复代码 */
	hookSetCode(function);

	/* 修改 EIP 执行原始指令 */
	registerRead();
	reg.Eip		= address;
	reg.EFlags	|= 0x100;
	registerSave();

	/* 如果没有设置返回 */
	if(threadRetGet(debug.dwThreadId, NULL) == 0) {
		runShow(function, FALSE);
	}

	/* 暂停其它所有线程 */
	threadSuspend(debug.dwThreadId);

	return 1;
}

static int runDebugExMisc() {
	char *message;

	switch(debug.u.Exception.ExceptionRecord.ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:			message = "EXCEPTION_ACCESS_VIOLATION";			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:		message = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";	break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:		message = "EXCEPTION_DATATYPE_MISALIGNMENT";	break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:		message = "EXCEPTION_FLT_DENORMAL_OPERAND";		break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:			message = "EXCEPTION_FLT_DIVIDE_BY_ZERO";		break;
		case EXCEPTION_FLT_INEXACT_RESULT:			message = "EXCEPTION_FLT_INEXACT_RESULT";		break;
		case EXCEPTION_FLT_INVALID_OPERATION:		message = "EXCEPTION_FLT_INVALID_OPERATION";	break;
		case EXCEPTION_FLT_OVERFLOW:				message = "EXCEPTION_FLT_OVERFLOW";				break;
		case EXCEPTION_FLT_STACK_CHECK:				message = "EXCEPTION_FLT_STACK_CHECK";			break;
		case EXCEPTION_FLT_UNDERFLOW:				message = "EXCEPTION_FLT_UNDERFLOW";			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:			message = "EXCEPTION_ILLEGAL_INSTRUCTION";		break;
		case EXCEPTION_IN_PAGE_ERROR:				message = "EXCEPTION_IN_PAGE_ERROR";			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:			message = "EXCEPTION_INT_DIVIDE_BY_ZERO";		break;
		case EXCEPTION_INT_OVERFLOW:				message = "EXCEPTION_INT_OVERFLOW";				break;
		case EXCEPTION_INVALID_DISPOSITION:			message = "EXCEPTION_INVALID_DISPOSITION";		break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:	message = "EXCEPTION_INT_OVERFLOW";				break;
		case EXCEPTION_PRIV_INSTRUCTION:			message = "EXCEPTION_PRIV_INSTRUCTION";			break;
		case EXCEPTION_STACK_OVERFLOW:				message = "EXCEPTION_STACK_OVERFLOW";			break;

		default:
			return 0;
	}

	/* 交给系统处理 */
	if(debug.u.Exception.dwFirstChance) {
		return 0;
	} else {
		return 1;
	}

	/*
		errorFormat (
			wMain.hwnd, "程序运行时发生意外中断：\n[0x%08X] %s @ 0x%08X",
			debug.u.Exception.ExceptionRecord.ExceptionCode,
			message,
			debug.u.Exception.ExceptionRecord.ExceptionAddress
		);
	*/
}


static int runDebug() {
	void (*handle)() = NULL;

	switch(debug.dwDebugEventCode) {
		/* 进程退出 */
		case EXIT_PROCESS_DEBUG_EVENT:	handle = runDebugProcessExit;	break;

		/* 线程创建和销毁 */
		case CREATE_THREAD_DEBUG_EVENT:	handle = runDebugThreadCreate;	break;
		case EXIT_THREAD_DEBUG_EVENT:	handle = runDebugThreadExit;	break;

		/* 载入或销毁 DLL 模块 */
		case LOAD_DLL_DEBUG_EVENT:		handle = runDebugDllLoad;		break;
		case UNLOAD_DLL_DEBUG_EVENT:	handle = runDebugDllUnload;		break;

		/* 异常处理器 */
		case EXCEPTION_DEBUG_EVENT:
			switch(debug.u.Exception.ExceptionRecord.ExceptionCode) {
				/* 单步 */
				case EXCEPTION_SINGLE_STEP:
					handle = runDebugExSglStep; break;

				/* 断点 */
				case EXCEPTION_BREAKPOINT:
					return runDebugExBkPoint();
				/* 其它应用程序异常 */
				default:
					return runDebugExMisc();
			}

			break;
	}

	if(handle) {
		handle();
	}

	return 1;
}


void runSuspendResume() {
	/* 开始运行 */
	if(RUN == -1) {
		OBJECT(bMainSuspend)->caption("暂停进程");
		OBJECT(bMainSuspend)->disable(FALSE);

		RUN = 1;

		/* 发送信号 */
		symbolRunAsyn(0);

		return;
	}

	/* 由运行到暂停 */
	if(RUN) {
		/* 暂停线程 */
		SuspendThread(RT.debug);
		/* 暂停调试进程 */
		processSuspend(RT.handle);

		guiMainStatus(FALSE, TRUE, TRUE);

		OBJECT(bMainSuspend)->caption("恢复进程");

		OBJECT(bMainDump)->disable(FALSE);
		OBJECT(bMainDisasm)->disable(FALSE);
		OBJECT(bMainMemory)->disable(FALSE);
		OBJECT(bMainHijack)->disable(FALSE);

		if(LIST(lsMainStack)->selected >= 0) {
			OBJECT(bMainViewer)->disable(FALSE);
		}

		RUN = 0;
	} else {
		OBJECT(bMainSuspend)->caption("暂停进程");

		guiMainStatus(FALSE, TRUE, TRUE);

		/* 恢复进程 */
		processResume(RT.handle);
		ResumeThread(RT.debug);

		RUN = 1;
	}
}

threadNewVoid(runDebug) {
	DWORD	status;

	int		mode;
	int		wait	= 1;
	int		inited	= 0;

	char	*error;

	ZeroMemory(&debug, sizeof debug);

	/* 加载进程 */
	do {
		mode	= symbolWait();
		error	= NULL;

		/* 运行新进程 */
		if(mode == -1) {
			error = processFile();
		} else {
			if((error = processSelect(mode)) == NULL) {
				inited = 1;
			}
		}

		/* 返回错误信息 */
		symbolReturn((int) error);

		if(error == NULL) {
			wait = 0;
		}
	} while(wait);

	/* 等待调试循环 */
	symbolWait();

	while(1) {
		status = DBG_EXCEPTION_NOT_HANDLED;

		/* 等待调试信息 */
		if(WaitForDebugEvent(&debug, INFINITE) == FALSE) {
			errorFormat(wMain.hwnd, "Invoke 出现错误，无法获取调试信息，应用程序已被迫终止。错误代码：%d", GetLastError());
		}

		/* 首次进入调试, 清除 Dr0 断点 */
		if(inited == 0) {
			registerRead();

			reg.Dr0 = 0;
			reg.Dr7 = 0;

			registerSave();

			inited = 1;
		}

		/* 分发调试信息 */
		if(debug.dwDebugEventCode) {
			if(runDebug()) {
				status = DBG_CONTINUE;
			}
		}

		/* 继续执行 */
		ContinueDebugEvent(debug.dwProcessId, debug.dwThreadId, status);
	}
}
