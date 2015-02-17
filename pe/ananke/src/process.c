/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ process.c, 0.1.1027
*/

#include <Windows.h>
#include <Psapi.h>
#include "base.h"
#include "pe.h"


/* 进程信息 */
process_h PROCESS;

/* 断点、节点、导入表和模块链表 */
bp_s		*BP			= NULL;
section_s	*SECTION	= NULL;
import_s	*IMPORT		= NULL;
module_s	*MODULE		= NULL;


static char * procRead(char *file) {
	FILE *fp;

	section_s	*section, *next;
	import_s	*import;

	IMAGE_DOS_HEADER		fileDos;
	IMAGE_NT_HEADERS		filePe;
	IMAGE_SECTION_HEADER	fileSection;
	IMAGE_IMPORT_DESCRIPTOR	fileImport;

	if((fp = fopen(file, "rb")) == NULL) {
		return "无法打开所选的文件，请检查文件是否存在或程序拥有访问权限。";
	}

	/* 找 DOS 头 */
	if((fread(&fileDos, sizeof fileDos, 1, fp) == 1) && (fileDos.e_magic == IMAGE_DOS_SIGNATURE)) {
		/* 移动到 PE 头 */
		fseek(fp, (long) fileDos.e_lfanew, SEEK_SET);
	} else {
		return "没有在所选的文件中找到 DOS 头，该文件可能不是一个合法的 PE 文件。";
	}
	/* 找 PE 头 */
	if((fread(&filePe, sizeof filePe, 1, fp) != 1) || (filePe.Signature != IMAGE_NT_SIGNATURE)) {
		return "没有在所选的文件中找到 PE 头，该文件可能不是一个合法的 PE 文件。";
	}

	/* 判断是不是 PE+ */
	switch(filePe.OptionalHeader.Magic) {
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			return "程序暂时不支持加载 64 位编译的 PE 文件。";

		default:
			return "无法识别文件的类型。";
	}

	/* 处理器类型 */
	switch(filePe.FileHeader.Machine) {
		case IMAGE_FILE_MACHINE_I386:
			break;

		default:
			return "无法识别运行此 PE 文件所需要的处理器类型。";
	}

	/* 子系统 */
	switch(filePe.OptionalHeader.Subsystem) {
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			break;

		default:
			return "无法识别运行此 PE 文件所需要的子系统。";
	}

	/* 判断是不是 DLL */
	if((filePe.FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) {
		return "不能运行 DLL 文件。";
	} else if((filePe.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) {
		return "无法识别文件的标记。";
	}

	/* 初始化链表 */
	if(BP == NULL) {
		mem_alloc(BP,		bp_s,		1);
		mem_alloc(SECTION,	section_s,	1);
		mem_alloc(IMPORT,	import_s,	1);
		mem_alloc(MODULE,	module_s,	1);
	}

	cmdWorking(2, "%s", "正在读取区段表 ...");

	/* 区段表 */
	next = SECTION;

	/* 读取区段 */
	while(!feof(fp)) {
		if(fread(&fileSection, sizeof fileSection, 1, fp) == 0) {
			break;
		}
		if(fileSection.VirtualAddress == 0) {
			break;
		}

		mem_alloc(section, section_s, 1);

		section->virtualaddr	= fileSection.VirtualAddress;
		section->offset			= fileSection.PointerToRawData;
		section->size			= fileSection.Misc.VirtualSize;
		section->name			= readFromBuffer((char *) fileSection.Name, 8);
		section->owner			= PE;

		next->next = section;
		next = section;

		sectionPush(section);
	}

	if(SECTION->next == NULL) {
		return "没有在 PE 文件中找到任何区段。";
	}

	/* 导入表 */
	if(filePe.OptionalHeader.DataDirectory[1].VirtualAddress != 0) {
		cmdWorking(1, "%s", "正在读取导入表 ...");

		/* 移动到导入表 */
		fseek(fp, (long) rva2offset(filePe.OptionalHeader.DataDirectory[1].VirtualAddress), SEEK_SET);

		while(!feof(fp)) {
			if(fread(&fileImport, sizeof fileImport, 1, fp) == 0) {
				break;
			}
			if(*((DWORD *) &fileImport) == 0) {
				break;
			}

			peFunction(fp, &fileImport);
		}

		import = IMPORT->next;

		while(import != NULL) {
			importPush(import);

			import = import->next;
		}
	}

	PROCESS.file		= file;
	PROCESS.status		= LOAD;
	PROCESS.entry		= filePe.OptionalHeader.AddressOfEntryPoint;
	PROCESS.baseRVAddr	= filePe.OptionalHeader.ImageBase;

	return NULL;
}

static char * procLoad(char *file) {
	int loaded = 0;

	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	CONTEXT				regs;
	DEBUG_EVENT			debug;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb			= sizeof(STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	if(CreateProcess(file, NULL, NULL, NULL, FALSE, PROCESS_QUERY_INFORMATION | DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi) == FALSE) {
		return "启动进程失败。";
	}

	PROCESS.status		= RUN;
	PROCESS.procHandle	= pi.hProcess;
	PROCESS.procId		= pi.dwProcessId;

	regs.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	/* 设置 EF 标记以实现单步执行 */
	GetThreadContext(pi.hThread, &regs);
	regs.EFlags |= 0x100;
	SetThreadContext(pi.hThread, &regs);

	/* 恢复进程 */
	ResumeThread(pi.hThread);

	cmdWorking(2, "正在准备调试信息 ...");

	while(1) {
		WaitForDebugEvent(&debug, INFINITE);

		if(debug.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
			if(debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) {
				GetThreadContext(pi.hThread, &regs);

				switch(loaded) {
					/* 首次中断 */
					case 0:
						regs.Dr0 = regs.Eax;
						regs.Dr7 = 0x101;

						loaded = 1; break;

					/* 中断于系统调试信息加载前 */
					case 1:
						peEntry();

						/* 找不到基地址 */
						if(PROCESS.baseAddr == 0) {
							TerminateProcess(pi.hProcess, 0);

							return "无法找到程序入口地址。";
						} else {
							PROCESS.baseOffset = ((long) PROCESS.baseAddr) - ((long) PROCESS.baseRVAddr);
						}
						/* 没有模块 */
						if(MODULE->next == NULL) {
							TerminateProcess(pi.hProcess, 0);

							return "未找到任何加载的模块。";
						}

						regs.Dr0 = ((long) PROCESS.entry) + PROCESS.baseAddr;
						regs.Dr7 = 0x101;

						loaded = 2;

						peSection();
						peImport(FALSE);
						peBreakAddress(regs.Dr0);

						cmdWorking(0, "程序运行中 ...");

						break;

					/* 中断 */
					default:
						cmdAddress (
							addr2rva(regs.Eip),
							regs.Eip,
							addr2section(regs.Eip),
							NULL
						);

						loop(regs.Eip);

						regs.Dr0 = 0;
						regs.Dr7 = 0;
						regs.EFlags |= 0x100;
				}

				SetThreadContext(pi.hThread, &regs);
			}
		} else if(debug.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
			break;
		}

		ContinueDebugEvent(pi.dwProcessId, pi.dwThreadId, DBG_CONTINUE);
	}

	return NULL;
}


th_new_char(Process) {
	char *message = NULL;

	cmdExecuteEnable(FALSE);
	cmdWorking(0, "正在打开文件 \"%s\" ...", arg);

	if((message = procRead(arg)) == NULL) {
		if((message = procLoad(arg)) == NULL) {
			cmdWorking(0, "进程已退出。");
		} else {
			cmdWorking(0, "%s", message);
		}
	} else {
		cmdWorking(2, "%s", message);

		mem_delete(arg);
	}

	if(message != NULL) {
		warning(message);
	}

	cmdExecuteEnable(TRUE);
}
