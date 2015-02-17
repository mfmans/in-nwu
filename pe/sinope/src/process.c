/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ process.c, 0.1.1104
*/

#define _SNP_PROCESS_

#include <Windows.h>
#include <Psapi.h>
#include "base.h"
#include "image.h"
#include "gui.h"
#include "action.h"


/* 指针 */
size_t pfPosition = 0;


/* 模块加载 */
BOOL procMapModule() {
	int			i, j;
	char		proc[MAX_PATH], name[MAX_PATH];

	module_s	*next, *curr;

	DWORD		count;
	HMODULE		module[4096];
	MODULEINFO	info;

	/* 获取进程名 */
	if(GetModuleFileNameEx(RUNTIME.proc.handle, NULL, proc, MAX_PATH) == 0) {
		return FALSE;
	}
	/* 获取所有模块 */
	if(EnumProcessModules(RUNTIME.proc.handle, module, sizeof module, &count) == FALSE) {
		return FALSE;
	}

	j		= count / sizeof(HMODULE);
	next	= MODULE;

	for(i = 0; i < j; i++) {
		if(GetModuleFileNameEx(RUNTIME.proc.handle, module[i], name, MAX_PATH) == 0) {
			continue;
		}
		/* 查询模块长度 */
		if(GetModuleInformation(RUNTIME.proc.handle, module[i], &info, sizeof info) == FALSE) {
			continue;
		}

		/* 加入模块表 */
		mem_alloc(curr, module_s, 1);
		mem_alloc(curr->path, char, strlen(name) + 1);

		curr->address	= (DWORD) module[i];
		curr->index		= -1;
		curr->size		= info.SizeOfImage;

		strcpy(curr->path, name);

		/* 由主模块获取真实的基地址 */
		if(strcmp(proc, name) == 0) {
			RUNTIME.proc.base	= (DWORD) module[i];
			RUNTIME.proc.size	= curr->size;
			RUNTIME.proc.module	= curr;
		}

		next->next = curr;
		next = curr;
	}

	/* 没有模块 */
	if(MODULE->next == NULL) {
		return FALSE;
	}

	/* 未获取基地址 */
	if(RUNTIME.proc.base == 0) {
		return FALSE;
	}

	RUNTIME.proc.entry	= RUNTIME.pe.entry + RUNTIME.proc.base;
	THREAD->next->start	= RUNTIME.proc.entry;

	return TRUE;
}

/* 区段映射 */
void procMapSection() {
	BOOL		image = FALSE;
	section_s	*section = SECTION->next;

	while(section) {
		/* 实际地址 */
		section->addr = section->va + RUNTIME.proc.base;

		/* 创建备份 */
		if((section->file == TRUE) && (section->dynamic == FALSE)) {
			mem_alloc(section->copy, char, section->sizeImage);

			/* 复制内存 */
			if(memoryRead(section->addr, section->copy, section->sizeImage)) {
				image = TRUE;
			} else {
				mem_delete(section->copy);
			}
		}

		section = section->next;
	}

	if(image == FALSE) {
		warning("无法为 PE 文件任何一个区段创建原始备份，这可能会造成动态代码检测的功能无法使用。");
	}
}


/* 初始化 */
static BOOL procInit() {
	/* 运行状态 */
	BOOL		run		= 1;
	/* 主线程 */
	thread_s	*thread	= THREAD->next;

	/* 调试信息 */
	DEBUG_EVENT	debug;
	/* 寄存器 */
	CONTEXT		reg;
	/* 状态 */
	DWORD		status;

	/* 设置寄存器 */
	reg.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	WORKING_SET("正在运行进程 ...");

	while(run) {
		status = DBG_EXCEPTION_NOT_HANDLED;

		WaitForDebugEvent(&debug, INFINITE);
		GetThreadContext(thread->handle, &reg);

		if(debug.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
			if(debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) {
				switch(run) {
					/* 首次中断 */
					case 1:
						reg.Dr0 = reg.Eax;
						reg.Dr7 = 0x101;

						run = 2; break;

					/* 创建运行备份和环境 */
					case 2:
						/* 获取模块 */
						if(procMapModule() == FALSE) {
							loopTerminate(); return FALSE;
						}

						/* 映射 */
						procMapSection();
						procImport();

						/* 添加入口断点 */
						bpFlush();
						bpAdd(RUNTIME.proc.entry, RUNTIME.proc.entry, FALSE);

						/* 设置程序入口 */
						reg.Dr0 = RUNTIME.proc.entry;
						reg.Dr7 = 0x101;

						/* 设置标题 */
						procTitle("运行中");

						run = 0; break;
				}

				SetThreadContext(thread->handle, &reg);

				status = DBG_CONTINUE;
			}
		} else if(debug.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
			return FALSE;
		}

		ContinueDebugEvent(RUNTIME.proc.pid, thread->id, status);
	}

	return TRUE;
}

/* 回收 */
static void procFree() {
	void *next;

	RUNTIME.alive		= FALSE;
	RUNTIME.running		= FALSE;
	RUNTIME.singleStep	= FALSE;

	ZeroMemory(&RUNTIME.proc, sizeof RUNTIME.proc);
	ZeroMemory(&RUNTIME.pe, sizeof RUNTIME.pe);

	/* 清理线程 */
	while(THREAD->next) {
		next = THREAD->next->next;

		mem_delete(THREAD->next);

		THREAD->next = (thread_s *) next;
	}

	/* 清理断点 */
	while(BP->next) {
		next = BP->next->next;

		mem_delete(BP->next);

		BP->next = (bp_s *) next;
	}

	/* 清理区段 */
	while(SECTION->next) {
		next = SECTION->next->next;

		//mem_delete(SECTION->next->name);
		mem_delete(SECTION->next->copy);
		mem_delete(SECTION->next);

		SECTION->next = (section_s *) next;
	}

	/* 清理导入表 */
	while(IMPORT->next) {
		next = IMPORT->next->next;

		//mem_delete(IMPORT->next->dll);
		//mem_delete(IMPORT->next->function);
		mem_delete(IMPORT->next);

		IMPORT->next = (import_s *) next;
	}

	/* 清理模块 */
	while(MODULE->next) {
		next = MODULE->next->next;

		mem_delete(MODULE->next->path);
		mem_delete(MODULE->next);

		MODULE->next = (module_s *) next;
	}
}

/* 进程启动 */
static char * process() {
	int		status = 0;

	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	CONTEXT				reg;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb		= sizeof(STARTUPINFO);
	si.dwFlags	= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	/* CUI 创建控制台 */
	if(RUNTIME.pe.console) {
		if(CONFIG.console == FALSE) {
			if(AllocConsole() == FALSE) {
				return "创建控制台窗口失败。";
			}

			CONFIG.console = TRUE;
		}

		si.hStdInput	= GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput	= GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError	= GetStdHandle(STD_ERROR_HANDLE);
	}

	/* 运行进程 */
	if(CreateProcess(RUNTIME.file, NULL, NULL, NULL, FALSE, PROCESS_QUERY_INFORMATION | DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi) == FALSE) {
		return "启动进程失败。";
	}

	RUNTIME.alive		= TRUE;
	RUNTIME.running		= TRUE;
	RUNTIME.singleStep	= FALSE;

	/* 进程信息 */
	RUNTIME.proc.pid	= pi.dwProcessId;
	RUNTIME.proc.handle	= pi.hProcess;

	/* 创建主线程 */
	threadCreate(pi.hThread, pi.dwThreadId, 0);

	/* 设置标记以获得寄存器信息 */
	reg.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	/* 设置 EF 标记, 实现单步执行 */
	GetThreadContext(pi.hThread, &reg);
	reg.EFlags |= 0x100;
	SetThreadContext(pi.hThread, &reg);

	/* 启动进程 */
	ResumeThread(pi.hThread);

	/* 初始化调试信息 */
	if(procInit() == FALSE) {
		return "进程异常退出。";
	}

	/* 设置各种状态 */
	enable(TRUE, FALSE, FALSE, FALSE);
	procTitle("运行中");

	/* 显示各种信息 */
	threadFlush();
	sectionFlush();
	importFlush();
	moduleFlush();

	/* 选择主线程 */
	COMBO(cmbThread)->select(0);

	/* 主循环 */
	loop();

	return NULL;
}


thNew(Process) {
	char *error;

	/* 修改按钮 */
	OBJECT(bExecute)->caption("终止进程");
	OBJECT(bExecute)->disable(TRUE);

	/* 进程执行 */
	if((error = process()) == NULL) {
		WORKING_SET("进程已退出。");
	} else {
		WORKING_SET_ALERT(error);
	}

	procFree();
	procTitle(NULL);

	enable(TRUE, FALSE, FALSE, FALSE);

	OBJECT(bExecute)->caption("启动进程");
}


/* 导入表 */
void procImport() {
	DWORD		address;
	import_s	*import = IMPORT->next;

	while(import) {
		if(memoryRead(va2addr(import->iatVa), (char *) &address, sizeof address)) {
			if(import->entry == 0) {
				import->entry = address;
			}

			import->point = address;
		}

		import = import->next;
	}
}

/* 标题 */
void procTitle(char *status) {
	int admin = -1;

	if(admin < 0) {
		admin = (int) uacIsAdmin();
	}

	if(status == NULL) {
		wMain.caption(WIN_TITLE);
	} else if(admin == 0) {
		wMain.caption("%s (%s) - %s", RUNTIME.filename, status, WIN_TITLE);
	} else {
		wMain.caption("%s (%s) - %s [管理员]", RUNTIME.filename, status, WIN_TITLE);
	}
}

/* 清理 PE 文件信息 */
void procClear() {
	section_s	*section = SECTION, *sectionNext;
	import_s	*import;
	module_s	*module;

	/* 清理区段 */
	while(section && section->next) {
		/* 清理 PE 文件里的区段 */
		if((section->next->dynamic == FALSE) && (section->next->file == TRUE)) {
			sectionNext = section->next->next;

			//mem_delete(section->next->name);
			mem_delete(section->next->copy);
			mem_delete(section->next);

			section = sectionNext;
		} else {
			section = section->next;
		}
	}

	/* 清理导入表 */
	while(IMPORT->next) {
		import = IMPORT->next->next;

		//mem_delete(IMPORT->next->dll);
		//mem_delete(IMPORT->next->function);
		mem_delete(IMPORT->next);

		IMPORT->next = import;
	}

	/* 清理模块 */
	while(MODULE->next) {
		module = MODULE->next->next;

		mem_delete(MODULE->next->path);
		mem_delete(MODULE->next);

		MODULE->next = (module_s *) module;
	}
}


thread_s * threadGet(DWORD id) {
	thread_s *thread = THREAD->next;

	while(thread) {
		if(thread->id == id) {
			return thread;
		}

		thread = thread->next;
	}

	return NULL;
}

thread_s * threadCreate(HANDLE handle, DWORD id, DWORD addr) {
	thread_s *next = THREAD;

	/* 找链尾 */
	while(next->next != NULL) {
		next = next->next;
	}

	/* 获取 threadid */
	if(id == 0) {
		id = GetThreadId(handle);
	}

	mem_alloc(next->next, thread_s, 1);

	next->next->id		= id;
	next->next->handle	= handle;
	next->next->start	= addr;

	return next->next;
}

void threadExit(HANDLE handle) {
	thread_s *curr = NULL;
	thread_s *next = THREAD;

	while(next->next) {
		if(next->next->handle == handle) {
			curr = next->next;
			next->next = curr->next;

			break;
		}

		next = next->next;
	}
	
	if(curr) {
		COMBO(cmbThread)->remove(curr->index);
		mem_delete(curr);
	}

	threadFlush();
}


BOOL memoryRead(DWORD address, void *buffer, DWORD size) {
	size_t count;

	if(ReadProcessMemory(RUNTIME.proc.handle, (LPCVOID) address, buffer, size, (SIZE_T *) &count)) {
		return TRUE;
	}

	return FALSE;
}

BOOL memoryWrite(DWORD address, void *buffer, DWORD size) {
	size_t count;

	if(WriteProcessMemory(RUNTIME.proc.handle, (LPVOID) address, buffer, size, (SIZE_T *) &count)) {
		return TRUE;
	}

	return FALSE;
}

BOOL isPeModule(DWORD addr) {
	if(addr < RUNTIME.proc.base) {
		return FALSE;
	}
	if(addr >= (RUNTIME.proc.base + RUNTIME.proc.module->size)) {
		return FALSE;
	}

	return TRUE;
}


BOOL dataReadMemory(void *buffer, size_t size) {
	if(memoryRead(RUNTIME.proc.base + pfPosition, buffer, size)) {
		pfPosition += size;

		return TRUE;
	}

	return FALSE;
}

void dataSeekMemory(long offset, int mode) {
	switch(mode) {
		case SEEK_SET: pfPosition = (size_t) offset; break;
		case SEEK_CUR: pfPosition = (size_t) ((long) pfPosition + offset); break;
	}
}

long dataTellMemory() {
	return (long) pfPosition;
}

DWORD dataOffsetMemory(DWORD va) {
	return va;
}

BOOL dataEndMemory() {
	if(pfPosition < (RUNTIME.proc.base + RUNTIME.proc.size)) {
		return FALSE;
	} else {
		return TRUE;
	}
}
