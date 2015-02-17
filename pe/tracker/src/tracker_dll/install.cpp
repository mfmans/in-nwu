/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/install.cpp   # 1312
*/


#include "base.h"



/* DLL 模块句柄 */
HMODULE		install::module_ntdll		= NULL;
HMODULE		install::module_controller	= NULL;


/* 基址 */
uintptr_t	install::base		= 0;

/* 常量/表/处理器基址 */
uintptr_t	install::base_library		= 0;
uintptr_t	install::base_controller	= 0;
uintptr_t	install::base_address		= 0;
uintptr_t	install::base_information	= 0;
uintptr_t	install::base_table			= 0;
uintptr_t	install::base_handler		= 0;

/* 函数基址 */
uintptr_t	install::base_function_jumper			= 0;
uintptr_t	install::base_function_injector			= 0;
uintptr_t	install::base_function_injector_local	= 0;
uintptr_t	install::base_function_injector_remote	= 0;
uintptr_t	install::base_function_loader			= 0;





/* 设置基址 */
void install::update (uintptr_t base) {
	// 保存基址
	install::base	= base;

	// 计算块基址
	install::base_library		= install::base				+ SIZE_THREAD_CODE + SIZE_THREAD_ARGUMENT;		// 动态链接库路径
	install::base_controller	= install::base_library		+ SIZE_LIBRARY;									// 控制器路径
	install::base_address		= install::base_controller	+ SIZE_CONTROLLER;								// Windows API 地址表
	install::base_information	= install::base_address		+ SIZE_ADDRESS;									// 信息块
	install::base_table			= install::base_information	+ SIZE_INFORMATION;								// 回调表
	install::base_handler		= install::base_table		+ SIZE_TABLE;									// 处理函数

	// 计算函数基址
	install::base_function_jumper			= install::base_handler						+ SIZE_HANDLER;						// WoW64 系统调用转发器
	install::base_function_injector			= install::base_function_jumper				+ SIZE_FUNCTION_JUMPER;				// 注入器
	install::base_function_injector_local	= install::base_function_injector			+ SIZE_FUNCTION_INJECTOR;			// 本地注入器
	install::base_function_injector_remote	= install::base_function_injector_local		+ SIZE_FUNCTION_INJECTOR_LOCAL;		// 本地注入器
	install::base_function_loader			= install::base_function_injector_remote	+ SIZE_FUNCTION_INJECTOR_REMOTE;	// 线程启动器
}


// 载入模块
bool install::controller (void) {
	// 载入控制器
	install::module_controller	= LoadLibraryA ((LPCSTR) install::base_controller);

	// 判断是否成功载入控制器
	if(install::module_controller == NULL) {
		MessageBoxA (NULL, "无法加载追踪控制器动态链接库模块。", "", MB_ICONERROR | MB_OK);

		return false;
	}

	return true;
}





/* 构造地址表 */
void install::build_address (void) {
	// 00 - 0F
	*((uint32_t *) (install::base_address + 0x00))	= (uint32_t) LoadLibraryA;
	*((uint32_t *) (install::base_address + 0x04))	= (uint32_t) FreeLibrary;
	*((uint32_t *) (install::base_address + 0x08))	= (uint32_t) GetProcAddress;

	// 10 - 1F
	*((uint32_t *) (install::base_address + 0x10))	= (uint32_t) SuspendThread;
	*((uint32_t *) (install::base_address + 0x14))	= (uint32_t) ResumeThread;
	*((uint32_t *) (install::base_address + 0x18))	= (uint32_t) GetThreadContext;
	*((uint32_t *) (install::base_address + 0x1C))	= (uint32_t) SetThreadContext;

	// 20 - 2F
	*((uint32_t *) (install::base_address + 0x20))	= (uint32_t) TerminateThread;
	*((uint32_t *) (install::base_address + 0x24))	= (uint32_t) GetThreadId;
	*((uint32_t *) (install::base_address + 0x28))	= (uint32_t) GetProcessIdOfThread;
	*((uint32_t *) (install::base_address + 0x2C))	= (uint32_t) GetThreadSelectorEntry;

	// 30 - 3F
	*((uint32_t *) (install::base_address + 0x30))	= (uint32_t) TerminateProcess;
}


/* 构造信息表 */
void install::build_information (void) {
	// 00 - 0F
	*((uint32_t *) (install::base_information + 0x00))	= (uint32_t) install::base_library;
	*((uint32_t *) (install::base_information + 0x04))	= (uint32_t) install::base_controller;

	// 10 - 1F
	*((uint32_t *) (install::base_information + 0x10))	= (uint32_t) install::base_function_jumper;

	// 20 - 2F
	*((uint32_t *) (install::base_information + 0x20))	= (uint32_t) install::base_function_injector;
	*((uint32_t *) (install::base_information + 0x24))	= (uint32_t) install::base_function_injector_local;
	*((uint32_t *) (install::base_information + 0x28))	= (uint32_t) install::base_function_injector_remote;

	// 30 - 3F
	*((uint32_t *) (install::base_information + 0x30))	= (uint32_t) install::base_function_loader;
}




/* 安装回调表 */
bool install::install_table (void) {
	// 获取 ntdll.dll 句柄
	install::module_ntdll	= LoadLibraryA ("ntdll.dll");

	// 检测是否成功获得 ntdll.dll 模块句柄
	if(module_ntdll == NULL) {
		return false;
	}


	// 系统函数名
	const char *	function_system		[ ] = {	"ZwResumeThread",	"ZwCreateThreadEx",		"ZwCreateUserProcess",	"ZwOpenFile",	"ZwCreateFile"	};
	// 回调函数名
	const char *	function_callback	[ ] = {	NULL,				NULL,					"tCreateProcess",		"tCreateFile",	"tCreateFile"	};


	// 自主加载函数地址
	uintptr_t	load	= (uintptr_t) GetProcAddress (install::module_controller, "tLoad");

	// 当前项指针
	uintptr_t	pointer	= install::base_table;


	// 依次安装函数
	for(int i = 0; i < (sizeof function_system / sizeof(const char *)); i ++) {
		if(install::install_table_function (pointer, load, function_system [i], function_callback [i]) == false) {
			return false;
		}

		// 移动指针指向下一项
		pointer	= pointer + 0x10;
	}


	// 写入空白
	memset ((void *) pointer, 0x00, 0x10);


	// 如果使用自主加载, 卸载模块
	if(load != 0) {
		FreeLibrary (install::module_controller);
	}


	return true;
}


/* 安装回调表函数 */
bool install::install_table_function (uintptr_t address, uintptr_t load, const char *system, const char *callback) {
	// 系统函数地址
	uintptr_t	address_system		= (uintptr_t) GetProcAddress (install::module_ntdll, system);
	// 回调函数地址
	uintptr_t	address_callback	= 0;

	// 判断是否成功从 ntdll.dll 导出表获取函数地址
	if(address_system == 0) {
		return false;
	}

	// 判断是否需要回调函数
	if(callback != NULL) {
		// 获取回调函数地址
		if(load == 0) {
			// 使用导出表
			address_callback	= (uintptr_t) GetProcAddress (install::module_controller, callback);
		} else {
			// 使用自主加载
			address_callback	= ((uintptr_t (__cdecl *)(const char *)) load) (callback);
		}

		// 判断是否成功获取回调函数地址
		if(address_callback == 0) {
			return false;
		}
	}

	// 读取系统函数所需权限
	uint32_t	flag	= PAGE_EXECUTE_READWRITE;

	// 设置内存权限
	if(VirtualProtect ((LPVOID) address_system, 0x0F, flag, (PDWORD) & flag) == FALSE) {
		return false;
	}

	// 检测函数是否正确
	if(*((uint8_t  *) (address_system + 0x00)) != 0xB8) {
		return false;
	}
	if(*((uint32_t *) (address_system + 0x05)) != 0xC015FF64) {
		return false;
	}
	if(*((uint32_t *) (address_system + 0x09)) != 0xC2000000) {
		return false;
	}

	// 写入回调表
	*((uint32_t *) (address + 0x00))	= (uint32_t) address_system + 0x0C;			// 触发的返回地址
	*((uint32_t *) (address + 0x04))	= 0;										// 处理器地址
	*((uint32_t *) (address + 0x08))	= (uint32_t) address_callback;				// 回调函数地址

	// 恢复内存权限
	VirtualProtect ((LPVOID) address_system, 0x0F, flag, (PDWORD) & flag);

	return true;
}




/* 映射函数 */
void install::map_function (void) {
	install::map_function_jumper			(install::base_function_jumper,				install::base_information,		install::base_table			);

	install::map_function_injector			(install::base_function_injector,			install::base_address,			install::base_information	);
	install::map_function_injector_local	(install::base_function_injector_local,		install::base_address,			install::base_information	);
	install::map_function_injector_remote	(install::base_function_injector_remote,	install::base_address,			install::base_information	);
	install::map_function_loader			(install::base_function_loader,				install::base_function_jumper								);
}

