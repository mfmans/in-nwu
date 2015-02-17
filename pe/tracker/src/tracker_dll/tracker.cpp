/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/tracker.cpp   # 1312
*/


#include "base.h"



/* 注入线程 */
static __declspec(naked) void __cdecl tracker_remote_call (void) {
	__asm {
			call	entry

			// 'tracker_install', \0
			__emit	0x74
			__emit	0x72
			__emit	0x61
			__emit	0x63
			__emit	0x6B
			__emit	0x65
			__emit	0x72
			__emit	0x5F
			__emit	0x69
			__emit	0x6E
			__emit	0x73
			__emit	0x74
			__emit	0x61
			__emit	0x6C
			__emit	0x6C
			__emit	0x00


		entry:
			push	ebp
			mov		ebp,	esp

			mov		esi,	[ebp + 0x0C]				; ESI	= {ARGUMENT}

			push	esi									; [ESP]	= {ARGUMENT}
			push	dword ptr 0							; [ESP]	= HMODULE


			mov		eax,	fs:[0x30]					; EAX	= linear address of	PEB
			mov		eax,	[eax + OFFSET_PEB]			; EAX	= tracker loader address in PEB

			test	eax,	eax
			jz		start


			mov		[ebp + 0x04],	eax

			jmp		done


		start:
			push	dword ptr [esi + 0x04]				; LoadLibraryA.lpFileName	= {ARGUMENT.library}
			call	dword ptr [esi + 0x10]				; LoadLibraryA

			test	eax,	eax
			jz		error


			mov		[esp],	eax							; [ESP]	= HMODULE	= return of LoadLibraryA
			mov		esi,	[esp + 0x04]				; ESI	= {ARGUMENT}

			push	dword ptr [ebp + 0x04]				; GetProcAddress.lpProcName	= "tracker_install"
			push	eax									; GetProcAddress.hModule	= HMODULE
			call	dword ptr [esi + 0x18]				; GetProcAddress

			test	eax,	eax
			jz		error


			mov		esi,	[esp + 0x04]				; ESI	= {ARGUMENT}

			push	dword ptr [esi]						; tracker_install.base		= {ARGUMENT.base}
			call	eax									; tracker_install
			add		esp,	0x04
			
			test	eax,	eax
			jz		error


			mov		dword ptr [ebp + 0x04],	eax			; set return code	= return value of tracker_install


			mov		esi,	fs:[0x30]					; ESI	= linear address of	PEB

			mov		[esi + OFFSET_PEB],	eax				; save tracker loader to PEB


			jmp		done


		error:
			mov		dword ptr [ebp + 0x04],	0x00		; set return code	= 0


		done:
			mov		esi,	[esp + 0x04]				; ESI	= {ARGUMENT}

			call	dword ptr [esi + 0x14]				; FreeLibrary


			add		esp,	0x04

			pop		ebp
			pop		eax									; EAX	= return code


			retn	4
	}
}





/* 注入函数, 运行于父进程 */
extern "C" __declspec(dllexport) int __cdecl tracker_remote (HANDLE thread, const char *library, const char *controller) {
	// 判断动态链接库路径长度
	if(strlen (library) > SIZE_LIBRARY) {
		MessageBoxA (NULL, "动态链接库路径长度超过最大限制。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 判断控制器路径长度
	if(strlen (controller) > SIZE_CONTROLLER) {
		MessageBoxA (NULL, "追踪控制器模块路径长度超过最大限制。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 获取线程 ID
	DWORD	threadid	= GetThreadId (thread);
	// 获取拥有所需权限的线程句柄
	thread	= OpenThread (THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, TRUE, threadid);

	// 判断是否成功获取进程句柄
	if(thread == NULL) {
		MessageBoxA (NULL, "无法获得拥有所期望权限的线程句柄。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 获取进程 ID
	DWORD	processid	= GetProcessIdOfThread	(thread);
	// 获取进程句柄
	HANDLE	process		= OpenProcess	(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processid);

	// 判断是否成功获取进程句柄
	if(process == NULL) {
		MessageBoxA (NULL, "无法通过线程句柄获取期望的进程句柄。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 分配内存块
	uintptr_t	base	= remote::allocate (process, ALLOCATE);

	// 判断是否分配成功
	if(base == 0) {
		MessageBoxA (NULL, "在目标进程空间内分配内存失败。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 块基址
	uintptr_t	base_thread_code		= base;
	uintptr_t	base_thread_argument	= base_thread_code		+ SIZE_THREAD_CODE;
	uintptr_t	base_library			= base_thread_argument	+ SIZE_THREAD_ARGUMENT;
	uintptr_t	base_controller			= base_library			+ SIZE_LIBRARY;


	// 线程启动参数
	uintptr_t	argument [ ]	= {
		/* 0x00 */	base,									// 内存块基址
		/* 0x04 */	base_library,							// 动态链接库路径地址
		/* 0x08 */	0,
		/* 0x0C */	0,
		/* 0x10 */	(uintptr_t) LoadLibraryA,				// @ LoadLibraryA
		/* 0x14 */	(uintptr_t) FreeLibrary,				// @ FreeLibrary
		/* 0x18 */	(uintptr_t) GetProcAddress,				// @ GetProcAddress
		/* 0x1C */	0,
	};


	// 块实际大小
	uint32_t	size_thread_code		= 0x86;
	uint32_t	size_thread_argument	= sizeof argument;
	uint32_t	size_library			= strlen (library)		+ 1;
	uint32_t	size_controller			= strlen (controller)	+ 1;


	// 写入代码
	if(remote::write (process, base_thread_code, tracker_remote_call, size_thread_code) == false) {
		MessageBoxA (NULL, "向目标进程内存写入注入线程代码失败。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 写入参数
	if(remote::write (process, base_thread_argument, argument, size_thread_argument) == false) {
		MessageBoxA (NULL, "向目标进程内存写入注入线程启动参数失败。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 写入动态链接库路径
	if(remote::write (process, base_library, (void *) library, size_library) == false) {
		MessageBoxA (NULL, "向目标进程内存写入动态链接库路径失败。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 写入追踪控制器路径
	if(remote::write (process, base_controller, (void *) controller, size_controller) == false) {
		MessageBoxA (NULL, "向目标进程内存写入追踪控制器路径失败。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 启动远程线程
	HANDLE	handle	= remote::thread (process, base_thread_code, (uint32_t) base_thread_argument);

	// 判断是否启动成功
	if(handle == NULL) {
		MessageBoxA (NULL, "无法在目标进程中启动远程线程。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 等待线程执行完毕
	WaitForSingleObject (handle, INFINITE);


	// 获取进程退出代码作为线程启动器地址
	uintptr_t	base_loader	= 0;

	// 获取进程退出代码
	if(GetExitCodeThread (handle, (LPDWORD) & base_loader) == FALSE) {
		MessageBoxA (NULL, "无法获取远程线程退出代码。", "", MB_ICONERROR | MB_OK);

		return 0;
	}

	// 判断启动器地址是否有效
	if((base_loader <= 0xFFFF) || (base_loader >= 0x7FFF0000)) {
		// 此时由 tracker_install 负责错误输出
		return 0;
	}

	// 判断启动器地址是否不存在于当前内存块
	if((base_loader < base) || (base_loader >= (base + ALLOCATE))) {
		// 释放当前内存块
		VirtualFree ((LPVOID) base, 0, MEM_RELEASE);
	}


	// 线程上下文环境
	CONTEXT	context;

	// 设置获取所有寄存器信息
	context.ContextFlags	= CONTEXT_ALL;

	// 获取线程寄存器信息
	if(GetThreadContext (thread, & context) == FALSE) {
		MessageBoxA (NULL, "无法获取线程的寄存器信息。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// TEB 线性地址
	LDT_ENTRY	teb_base;

	// 获取 TEB 线性地址
	if(GetThreadSelectorEntry (thread, context.SegFs, & teb_base) == FALSE) {
		MessageBoxA (NULL, "无法获取线程信息块线性地址。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 计算 TEB 地址
	uintptr_t	teb		= (teb_base.HighWord.Bytes.BaseHi << 24) | (teb_base.HighWord.Bytes.BaseMid << 16) | teb_base.BaseLow;
	// 线程当前执行地址
	uintptr_t	pointer	= (uintptr_t) context.Eip;

	// 将当前执行地址保存到 TEB 中
	if(remote::write (process, teb + OFFSET_TEB, & pointer, 0x04) == false) {
		MessageBoxA (NULL, "无法将线程执行入口保存到线程信息块中。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 设置新的执行地址为线程启动器
	context.Eip	= (DWORD) base_loader;

	// 保存寄存器信息
	if(SetThreadContext (thread, & context) == FALSE) {
		MessageBoxA (NULL, "无法保存线程的寄存器信息。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 关闭远程线程句柄
	CloseHandle (handle);
	// 关闭进程句柄
	CloseHandle (process);
	// 关闭线程句柄
	CloseHandle (thread);

	return 1;
}




/* 环境创建函数, 运行于子进程 */
extern "C" __declspec(dllexport) int __cdecl tracker_install (uintptr_t base) {
	// 计算基址
	install::update (base);

	// 加载控制器模块
	if(install::controller ( ) == false) {
		return 0;
	}

	// 构建表格
	install::build_address		( );
	install::build_information	( );

	// 创建回调表
	if(install::install_table ( ) == false) {
		MessageBoxA (NULL, "无法初始化函数回调表。请检查追踪控制器动态链接库文件是否符合所需的编写规范，如回调函数是否存在并被正确导出。", "", MB_ICONERROR | MB_OK);

		return 0;
	}


	// 安装处理器
	install::install_handler	( );

	// 映射函数
	install::map_function	( );


	// 保存到 PEB
	__asm {
		mov		eax,	install::base_function_loader		; address of		loader
		mov		ebx,	fs:[0x30]							; linear address of	PEB

		mov		[ebx + OFFSET_PEB],	eax
	}


	// 返回线程启动器地址
	return (int) install::base_function_loader;
}

