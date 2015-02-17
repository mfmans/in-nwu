/*
	$ Monitor Module   (C) 2005-2014 MF
	$ handler/dll.cpp
*/


#include "../header/base.h"
#include "../header/handler.h"



/* 加载函数 */
bool handler_dll::load (unsigned int index, const char *name) {
	// 获取函数地址
	uintptr_t	address	= (uintptr_t) GetProcAddress((HMODULE) this->handle, (LPCSTR) name);

	if(address == 0) {
		// 输出警告
		Console::warning("#HANDLER#Callback %hs => NOT FOUND\n", name);

		// 加载失败设置无效地址
		this->pointer [index]	= 0xFFFFFFFF;

		return false;
	} else {
		// 输出提醒
		Console::notice("#HANDLER#Callback %hs => 0x%08X\n", name, address);

		// 保存地址
		this->pointer [index]	= address;

		return true;
	}
}




/* 启动 */
bool handler_dll::start (const char *path) {
	// 初始化函数地址
	for(int i = 0; i < MM_HANDLER_MAX; i ++) {
		this->pointer [i] = 0;
	}

	// 加载句柄
	this->handle	= (void *) LoadLibraryA(path);

	// 判断是否加载成功
	if(this->handle == NULL) {
		Console::error("#HANDLER#Failed to load dynamic library: %s\n", path);

		return false;
	}

	return true;
}


/* 关闭 */
void handler_dll::close (void) {
	// 卸载模块
	FreeLibrary((HMODULE) this->handle);
}


/* 发送请求 */
bool handler_dll::send (unsigned int index, const char *function, int argc, void *argv) {
	// 判断地址是否已被加载
	if(this->pointer[index] == 0) {
		// 尝试加载
		if(this->load(index, function) == false) {
			// 函数不存在直接返回允许
			return true;
		}
	}

	// 判断地址是否为未定义地址
	if(this->pointer[index] == 0xFFFFFFFF) {
		return true;
	}

	// 函数地址
	uintptr_t	address	= this->pointer [index];

	// 处理器返回结果
	int			result	= 0;

	__try {
		__asm {
				mov		ecx,	argc

				mov		ebx,	ecx
				dec		ebx
				shl		ebx,	0x03

				add		ebx,	argv
				add		ebx,	0x04

			next:
				push	dword ptr [ebx]

				sub		ebx,	0x08

				loop	next

			
				mov		eax,	address
				call	eax

				mov		result,	eax

				mov		ecx,	argc
				shl		ecx,	0x02

				add		esp,	ecx
		}
	} __except (1) {
		// 输出未捕获异常信息
		Console::warning("#HANDLER#Unhandled exception at callback %s   [ACCEPT]\n", function);

		// 出错默认返回允许
		result	= 1;
	}

	// 输出拒绝
	if(result == 0) {
		Console::warning("#HANDLER#%s handled   [REJECT]\n", function);
	}

	return (bool) result;
}

