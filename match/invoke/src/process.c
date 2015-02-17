/*
	$ Invoke   (C) 2005-2012 MF
	$ process.c, 0.1.1124
*/

#pragma comment (lib, "Psapi.lib")

#include <Windows.h>
#include <Tlhelp32.h>
#include <Psapi.h>
#include "base.h"
#include "gui.h"


/* 最大进程数量 */
#define COUNT		1024
/* 最大进程路径 */
#define BUFFER		(MAX_PATH * 2)


/* 进程信息 */
static DWORD	*processPid		= NULL;
static char		**processName	= NULL;

/* 内存指针 */
static DWORD	memoryPtr		= 0;


static char * processPe(FILE *fp) {
	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS pe;

	/* 解析 PE 头 */
	if((fread(&dos, sizeof dos, 1, fp) == 0) || (dos.e_magic != IMAGE_DOS_SIGNATURE)) {
		return "解析文件 DOS 头失败，所选的文件可能不是一个正确的 EXE 文件。";
	} else {
		fseek(fp, (long) dos.e_lfanew, SEEK_SET);
	}
	if((fread(&pe, sizeof pe, 1, fp) == 0) || (pe.Signature != IMAGE_NT_SIGNATURE)) {
		return "解析文件 PE 头失败，所选的文件可能不是一个正确的 EXE 文件。";
	}

	/* 判断是不是 DLL */
	if((pe.FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) {
		return "所选的文件是 DLL 类型的 PE 文件，程序不能运行此类文件。";
	} else if((pe.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) {
		return "无法识别所选的文件的运行标记。";
	}

	/* 判断是不是 PE+ */
	switch(pe.OptionalHeader.Magic) {
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:	break;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:	return "所选的文件是 64 位编译的文件，程序不能运行此类文件。";
		default:							return "无法识别所选的文件的类型标记。";
	}

	/* 处理器类型 */
	switch(pe.FileHeader.Machine) {
		case IMAGE_FILE_MACHINE_I386:	break;
		default:						return "无法识别运行所选的文件所需要的处理器类型。";
	}

	/* 子系统 */
	switch(pe.OptionalHeader.Subsystem) {
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
			RT.console = 0; break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			RT.console = 1; break;

		default: return "不支持运行所选的文件所需要的子系统。";
	}

	RT.attach		= FALSE;
	RT.uninstall	= FALSE;
	RT.entry		= pe.OptionalHeader.AddressOfEntryPoint;

	return NULL;
}

static void processInit(DWORD pid, HANDLE handle) {
	char	buffer[BUFFER];
	char	*temp, *next;

	RT.file		= "Unknown";
	RT.pid		= pid;
	RT.handle	= handle;

	/* 获取镜像 */
	if((RT.snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)) == NULL) {
		die("无法获取调试进程的镜像信息。");
	}
	if(RT.snapshot == INVALID_HANDLE_VALUE) {
		die("无法获取调试进程的镜像信息。");
	}

	/* 获取主模块名 */
	if(GetModuleFileNameEx(handle, NULL, buffer, BUFFER) == FALSE) {
		die("无法获取调试进程的主模块路径。");
	} else {
		NEW(RT.module, char, strlen(buffer) + 1);

		strcpy(RT.module, buffer);

		next = RT.module;
	}

	while(temp = strstr(next, "\\")) {
		next = temp + 1;
	}

	if(*next) {
		RT.file = next;
	}
}

static DWORD processBase() {
	MODULEENTRY32 module;
	module.dwSize = sizeof module;

	do {
		if(Module32First(RT.snapshot, &module) == FALSE) {
			break;
		}

		do {
			if(strcmp(RT.module, module.szExePath) == 0) {
				RT.mainStart	= (DWORD) module.modBaseAddr;
				RT.mainEnd		= RT.mainStart + module.modBaseSize;

				return (DWORD) RT.mainStart;
			}
		} while(Module32Next(RT.snapshot, &module));
	} while(0);

	die("无法获取调试进程主模块的信息。");

	return 0;
}

static char * processOpen(char *file) {
	static int console = 0;

	int		run		= 1;
	DWORD	mode	= 0;

	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	CONTEXT				reg;
	DEBUG_EVENT			debug;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb		= sizeof(STARTUPINFO);
	si.dwFlags	= STARTF_USESTDHANDLES;

	reg.ContextFlags = CONTEXT_ALL;

	/* CUI */
	if(RT.console) {
		if(console == 0) {
			if(AllocConsole() == FALSE) {
				return "创建控制台窗口失败。";
			}

			console = 1;
		}

		si.hStdInput	= GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput	= GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError	= GetStdHandle(STD_ERROR_HANDLE);
	}

	if(CreateProcess(NULL, file, NULL, NULL, FALSE, PROCESS_QUERY_INFORMATION | DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi) == FALSE) {
		return "启动进程失败。";
	}

	/* 保存线程 */
	threadCreate(pi.dwThreadId, pi.hThread);

	/* 设置 EF 标记 */
	GetThreadContext(pi.hThread, &reg);
	reg.EFlags |= 0x100;
	/* 实现单步执行 */
	SetThreadContext(pi.hThread, &reg);

	ResumeThread(pi.hThread);

	while(run) {
		mode = DBG_EXCEPTION_NOT_HANDLED;

		WaitForDebugEvent(&debug, INFINITE);
		GetThreadContext(pi.hThread, &reg);

		if(debug.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
			if(debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) {
				/* 断点于 NtContinue@ntdll.dll, for XP */
				if(run == 1) {
					reg.Dr0 = (DWORD) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtContinue");
					reg.Dr7 = 0x101;

					run = 2;
				}

				mode = DBG_CONTINUE;
			} else if(debug.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) {
				switch(run) {
					/* 初始化中断 */
					case 2:
						reg.Dr0 = reg.Eax;
						reg.Dr7 = 0x101;

						run = 3; break;

					/* 初始化完成 */
					case 3:
						processInit(pi.dwProcessId, pi.hProcess);

						/* 设置程序入口 */
						reg.Dr0 = processBase() + RT.entry;
						reg.Dr7 = 0x101;

						run = 0; break;
				}

				mode = DBG_CONTINUE;
			}
		} else if(debug.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
			return FALSE;
		}

		SetThreadContext(pi.hThread, &reg);
		ContinueDebugEvent(debug.dwProcessId, debug.dwThreadId, mode);
	}

	return NULL;
}

static BOOL processPrivilege(HANDLE token, char *privilege, BOOL enable) {
	DWORD				size;

	LUID				luid;
	TOKEN_PRIVILEGES	tp;
	TOKEN_PRIVILEGES	tpPrevious;

	if(LookupPrivilegeValue(NULL, privilege, &luid) == FALSE) {
		return FALSE;
	}

	size = sizeof(TOKEN_PRIVILEGES);

	tp.PrivilegeCount			= 1;
	tp.Privileges[0].Luid		= luid;
	tp.Privileges[0].Attributes	= 0;

	if(AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &size) == FALSE) {
		return FALSE;
	}

	tpPrevious.PrivilegeCount		= 1;
	tpPrevious.Privileges[0].Luid	= luid;

	if(enable) {
		tpPrevious.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
	} else {
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);
	}

	return AdjustTokenPrivileges(token, FALSE, &tpPrevious, size, NULL, NULL);
}


int processList(DWORD **pid, char ***name) {
	HANDLE	handle;
	DWORD	size;
	int		i, j, count;

	if(processPid == NULL) {
		NEW(processPid,		DWORD,	COUNT);
		NEW(processName,	char *,	COUNT);
	} else {
		ZeroMemory(processPid,	sizeof(DWORD) * COUNT);
		
		for(i = 0; i < COUNT; i++) {
			DEL(processName[i]);
		}
	}

	if(EnumProcesses(processPid, COUNT, &size) == FALSE) {
		return 0;
	}

	if(size == 0) {
		return 0;
	} else {
		count = (int) size / sizeof(DWORD);
	}

	for(i = j = 0; i < count; j++) {
		size = BUFFER;

		if(i != j) {
			processPid[i] = processPid[j];
		}

		/* 打开进程查询 */
		if((handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processPid[i])) == NULL) {
			count--; continue;
		}

		NEW(processName[i], char, BUFFER);

		if(GetProcessImageFileName(handle, processName[i], size) == FALSE) {
			DEL(processName[i]);
		}

		/* 将物理路径转换为盘符 */
		device2NtLetter(processName[i]);

		CloseHandle(handle);

		i++;
	}

	*pid	= processPid;
	*name	= processName;

	return count;
}

char * processSelect(int index) {
	DWORD	pid;
	HANDLE	token;

	/* 指定 pid */
	pid = processPid[index];

	/* 提权 */
	if(OpenProcessToken(RT.process, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token) == FALSE) {
		return "程序申请提权失败。";
	}
	if(processPrivilege(token, SE_DEBUG_NAME, TRUE) == FALSE) {
		CloseHandle(token);
		
		return "程序申请提权失败。";
	}

	/* 打开目标进程 */
	if((RT.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)) == NULL) {
		CloseHandle(token);

		return "无法访问所选的进程，这可能是由于权限不足导致的。";
	}

	processPrivilege(token, SE_DEBUG_NAME, FALSE);
	CloseHandle(token);

	if(DebugActiveProcess(pid) == FALSE) {
		return "绑定所选的进程失败。造成的这个问题的原因可能是其为系统进程，或者是 64 位进程，或者已终止。";
	}

	if(!confirm(NULL, "在 Invoke 退出时是否终止被调试的进程？")) {
		RT.uninstall = TRUE;

		DebugSetProcessKillOnExit(FALSE);
	}

	processInit(pid, RT.handle);

	RT.attach = TRUE;

	return NULL;
}

char * processFile() {
	FILE	*fp;
	char	*file, *error;

	if((file = selectFileOpen(NULL, "exe", "请选择需要运行的文件")) == NULL) {
		return "";
	}
	if((fp = fopen(file, "rb")) == NULL) {
		return "无法打开文件，请确认文件是否存在或正在被其它应用程序使用。";
	}

	error = processPe(fp);
	fclose(fp);

	if(error == NULL) {
		error = processOpen(file);
	}

	return error;
}

void processFree() {
	int i;

	for(i = 0; i < COUNT; i++) {
		DEL(processName[i]);
	}

	DEL(processPid);
	DEL(processName);
}


int processRead(DWORD addr, void *buffer, unsigned int size) {
	int read;

	if(ReadProcessMemory(RT.handle, (LPCVOID) addr, buffer, size, (SIZE_T *) &read)) {
		if(read < (int) size) {
			return 0;
		} else {
			return read;
		}
	} else {
		return 0;
	}
}

int processWrite(DWORD addr, void *buffer, unsigned int size) {
	int		written;
	DWORD	permOld, permTemp;

	if(WriteProcessMemory(RT.handle, (LPVOID) addr, buffer, size, (SIZE_T *) &written)) {
		return written;
	}

	/* 提权失败 */
	if(VirtualProtectEx(RT.handle, (LPVOID) addr, size, PAGE_EXECUTE_READWRITE, &permOld) == FALSE) {
		return 0;
	}

	/* 提权后写入 */
	WriteProcessMemory(RT.handle, (LPVOID) addr, buffer, size, (SIZE_T *) &written);
	/* 恢复权限 */
	VirtualProtectEx(RT.handle, (LPVOID) addr, size, permOld, &permTemp);

	return written;
}


void memoryRewind() {
	memoryPtr = 0;
}

int memorySeek(int offset, int mode){
	switch(mode) {
		case SEEK_SET: memoryPtr = (DWORD) offset; break;
		case SEEK_CUR: memoryPtr = (DWORD) ((int) memoryPtr + offset); break;
	}

	return memoryPtr;
}

int memoryTell() {
	return memoryPtr;
}

int memoryAvail(int offset) {
	if(memoryPtr < (DWORD) offset) {
		return 1;
	} else {
		return 0;
	}
}

int memoryRead(void *buffer, unsigned int size) {
	int result = processRead(memoryPtr, buffer, size);

	memorySeek((int) size, SEEK_CUR);

	return result;
}

char * memoryReadString(unsigned int max, int display) {
	char *data;
	unsigned int i;

	NEW(data, char, max);

	for(i = 0; i < max; i++) {
		if(memoryRead(data + i, sizeof(char))) {
			if(data[i] == 0) {
				return data;
			}

			if(display) {
				if((data[i] < 0x20) || (data[i] > 0x7E)) {
					data[i] = '?';
				}
			}
		}
	}

	DEL(data);

	return NULL;
}

int memoryWrite(void *buffer, unsigned int size) {
	int result = processWrite(memoryPtr, buffer, size);

	memorySeek((int) size, SEEK_CUR);

	return result;
}
