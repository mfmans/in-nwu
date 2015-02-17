/*
	$ Monitor Module   (C) 2005-2014 MF
	$ router.cpp
*/


#include "header/base.h"
#include "header/error.h"
#include "header/handler.h"



/* 转发器开始 */
#define ROUTE_BEGIN()													\
	__asm {		push	offset critical							}		\
	__asm {		call	EnterCriticalSection					}


/* 转发器结束 */
#define ROUTE_END(handle, argc, function, size)							\
	__asm {		call	handle									}		\
	__asm {		add		esp, argc								}		\
	__asm {		test	eax, eax								}		\
	__asm {		jnz		run										}		\
	__asm {		push	offset critical							}		\
	__asm {		call	LeaveCriticalSection					}		\
	__asm {		ret		size									}		\
	run:																\
	__asm {		push	offset critical							}		\
	__asm {		call	LeaveCriticalSection					}		\
	__asm {		mov		eax, function							}		\
	__asm {		add		eax, 2									}		\
	__asm {		jmp		eax										}





/* 临界区 */
static	CRITICAL_SECTION	critical;



/* 函数地址 */
void *	Router::address_createfilew		= NULL;
void *	Router::address_loadlibraryexw	= NULL;
void *	Router::address_createprocessw	= NULL;





/* 跳转 CreateFileW */
__declspec(naked) void Router::jump_createfilew (void) {
	ROUTE_BEGIN();

	__asm {
			push	dword ptr [esp + 0x14]					; dwCreationDisposition
			push	dword ptr [esp + 0x10]					; dwShareMode
			push	dword ptr [esp + 0x10]					; dwDesiredAccess
			push	dword ptr [esp + 0x10]					; lpFileName
	}

	ROUTE_END(Router::run_createfilew, 0x10, Router::address_createfilew, 0x1C);
}


/* 跳转 LoadLibraryExW */
__declspec(naked) void Router::jump_loadlibraryexw (void) {
	ROUTE_BEGIN();

	__asm {
			push	dword ptr [esp + 0x04]					; lpFileName
	}

	ROUTE_END(Router::run_loadlibraryexw, 0x04, Router::address_loadlibraryexw, 0x0C);
}


/* 跳转 CreateProcessW */
__declspec(naked) void Router::jump_createprocessw (void) {
	ROUTE_BEGIN();

	__asm {
			push	dword ptr [esp + 0x08]					; lpCommandLine
			push	dword ptr [esp + 0x08]					; lpApplicationName
	}

	ROUTE_END(Router::run_createprocessw, 0x08, Router::address_createprocessw, 0x28);
}





/* 处理 CreateFileW */
bool Router::run_createfilew (wchar_t *name, int access, int share, int creation) {
	// 没有文件名直接返回失败
	if(name == NULL) {
		Console::warning("#ROUTER#CreateFileW (lpFileName: NULL)  [REJECT]\n");

		return false;
	}

	__try {
		Console::notice("#ROUTER#CreateFileW (lpFileName: \"%ls\", dwDesiredAccess: 0x%08X, dwShareMode: 0x%08X, dwCreationDisposition: 0x%08X)\n", name, access, share, creation);

		// 处理器
		if(Handler::send(0, "CreateFile", 4, "%ls", name, "%d", access, "%d", share, "%d", creation) == false) {
			return false;
		}
	} __except (1) {
		Console::notice("CreateFileW (lpFileName: 0x%08X)  [ACCESS VIOLENT]\n", name);
	}

	return true;
}


/* 处理 LoadLibraryExW */
bool Router::run_loadlibraryexw (wchar_t *name) {
	// 没有文件名直接返回失败
	if(name == NULL) {
		Console::warning("#ROUTER#LoadLibraryExW (lpFileName: NULL)  [REJECT]\n");

		return false;
	}

	__try {
		Console::notice("#ROUTER#LoadLibraryExW (lpFileName: \"%ls\")\n", name);

		// 处理器
		if(Handler::send(1, "LoadLibrary", 1, "%ls", name) == false) {
			return false;
		}
	} __except (1) {
		Console::notice("LoadLibraryExW (lpFileName: 0x%08X)  [ACCESS VIOLENT]\n", name);
	}

	return true;
}


/* 处理 CreateProcessW */
bool Router::run_createprocessw (wchar_t *application, wchar_t *command) {
	__try {
		if(application == NULL) {
			if(command == NULL) {
				// 没有启动参数直接返回失败
				Console::warning("#ROUTER#CreateProcessW (lpApplicationName: NULL, lpCommandLine: NULL)  [REJECT]\n");

				return false;
			} else {
				Console::notice("#ROUTER#CreateProcessW (lpCommandLine: \"%ls\")\n", command);
			}
		} else {
			if(command == NULL) {
				Console::notice("#ROUTER#CreateProcessW (lpApplicationName: \"%ls\")\n", application);
			} else {
				Console::notice("#ROUTER#CreateProcessW (lpApplicationName: \"%ls\", lpCommandLine: \"%ls\")\n", application, command);
			}
		}
	} __except (1) {
		Console::notice("CreateProcessW (lpApplicationName: 0x%08X, lpCommandLine: 0x%08X)  [ACCESS VIOLENT]\n", application, command);
	}

	// 保证调用参数不为 NULL
	if(application == NULL) {
		application = L"";
	}
	if(command == NULL) {
		command = L"";
	}

	// 调用控制脚本
	if(Handler::send(2, "CreateProcess", 2, "%ls", application, "%ls", command) == false) {
		return false;
	}

	return true;
}





/* 检测代码是否可供劫持 */
bool Router::validate (void *p) {
	// 函数入口
	uintptr_t	entry	= (uintptr_t) p;
	// 中介跳板
	uintptr_t	jump	= entry - 0x05;

	// 检测函数入口 (mov edi, edi)
	if(*((uint16_t *) entry) != 0xFF8B) {
		return false;
	}

	// 检测中介跳板
	while(jump < entry) {
		uint8_t byte = *((uint8_t *) (jump ++));

		// 必须为 int3 或 nop
		if((byte != 0xCC) && (byte != 0x90)) {
			return false;
		}
	}

	return true;
}


/* 写入内存 */
bool Router::write (uintptr_t address, unsigned int size, uint8_t *buffer) {
	// 当前权限
	unsigned long current;

	// 更新权限
	if(VirtualProtect((LPVOID) address, (SIZE_T) size, PAGE_EXECUTE_READWRITE, (PDWORD) & current) == FALSE) {
		return false;
	}

	// 依次写入每一个字节
	for(unsigned int i = 0; i < size; i ++) {
		*((uint8_t *) (address + i)) = buffer [i];
	}

	// 恢复权限
	if(VirtualProtect((LPVOID) address, (SIZE_T) size, current, (PDWORD) & current) == FALSE) {
		return false;
	}

	return true;
}


/* 劫持函数 */
bool Router::inject (void *target, void *source) {
	// 输出信息
	Console::notice("#SYSTEM#Hook: 0x%08X => 0x%08X\n", source, target);

	// 需要写入的内容
	uint8_t code [7] = {
		0xE9,							// jmp long ptr
		0x00, 0x00, 0x00, 0x00,			// jmp relative address
		0xEB, 0xF9,						// jmp short ptr -7
	};

	// 计算函数跳转差值
	*((int32_t *) ((uintptr_t) code + 1)) = (int32_t) target - (int32_t) source;

	// 验证是否可以劫持
	if(Router::validate(source) == false) {
		return false;
	}

	// 修改函数
	if(Router::write((uintptr_t) source - 0x05, sizeof(code), code) == false) {
		return false;
	}

	// 判断是否劫持成功
	if(Router::validate(source) == true) {
		// 如果劫持失败, 指令没有变化
		return false;
	} else {
		// 如果劫持成功, 指令必定发生改变
		return true;
	}
}




/* 执行劫持 */
int Router::install (void) {
	// 初始化临界区
	InitializeCriticalSection(& critical);

	// 输出提示
	Console::notice("#SYSTEM#Ready\n");

	// 加载 kernelbase.dll
	HMODULE	handle	= LoadLibraryA("kernelbase.dll");

	if(handle == NULL) {
		// 输出警告
		Console::warning("#SYSTEM#Failed to load dynamic library <kernelbase.dll>\n");
	} else {
		// 如果加载成功, 从 kernelbase.dll 中获取函数地址
		Router::address_createfilew		= (void *) GetProcAddress(handle, (LPCSTR) "CreateFileW");
		Router::address_loadlibraryexw	= (void *) GetProcAddress(handle, (LPCSTR) "LoadLibraryExW");
		Router::address_createprocessw	= (void *) GetProcAddress(handle, (LPCSTR) "CreateProcessW");
	}

	// 加载失败的从 kernel32.dll 中获取函数地址
	if(Router::address_createfilew == NULL) {
		Router::address_createfilew = CreateFileW;
	}
	if(Router::address_loadlibraryexw == NULL) {
		Router::address_loadlibraryexw = LoadLibraryExW;
	}
	if(Router::address_createprocessw == NULL) {
		Router::address_createprocessw = CreateProcessW;
	}

	// 输出地址
	Console::notice("#SYSTEM#Peek: CreateFileW (...)    = 0x%08X\n", Router::address_createfilew);
	Console::notice("#SYSTEM#Peek: LoadLibraryExW (...) = 0x%08X\n", Router::address_loadlibraryexw);
	Console::notice("#SYSTEM#Peek: CreateProcessW (...) = 0x%08X\n", Router::address_createprocessw);

	// CreateFileW
	if(Router::inject(Router::jump_createfilew, Router::address_createfilew) == false) {
		return MM_ERROR_FUNCTION_CREATEFILEW;
	}

	// LoadLibraryExW
	if(Router::inject(Router::jump_loadlibraryexw, Router::address_loadlibraryexw) == false) {
		// 输出控制台错误
		Console::error("#SYSTEM#Failed to hook LoadLibraryExW() on old version of Windows\n");
	}

	// CreateProcessW
	if(Router::inject(Router::jump_createprocessw, Router::address_createprocessw) == false) {
		return MM_ERROR_FUNCTION_CREATEPROCESSW;
	}

	// 劫持成功
	return MM_ERROR_SUCCEED;
}

