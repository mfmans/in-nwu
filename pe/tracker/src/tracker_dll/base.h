/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/base.h   # 1312
*/


#ifndef _TRACKER_HEADER_
#define _TRACKER_HEADER_


#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE


#ifdef UNICODE
	#undef UNICODE
#endif

#ifdef _UNICODE
	#undef _UNICODE
#endif




/* PEB 标志位置 */
#define OFFSET_PEB						0x0ACC
/* TEB 备份位置 */
#define OFFSET_TEB						0x0ACC


/* 分配内存块大小 */
#define ALLOCATE						0x3000				// 分配内存块大小 (4 pages)


/* 注入线程代码大小 */
#define SIZE_THREAD_CODE				0x0100
/* 注入线程启动参数数组大小 */
#define SIZE_THREAD_ARGUMENT			0x0100

/* 动态链接库路径大小 */
#define SIZE_LIBRARY					0x0200				// tracker.dll
/* 追踪控制器路径大小 */
#define SIZE_CONTROLLER					0x0200

/* 地址表大小 */
#define SIZE_ADDRESS					0x0100
/* 信息块大小 */
#define SIZE_INFORMATION				0x0100
/* 回调表大小 */
#define SIZE_TABLE						0x0200

/* 处理函数大小 */
#define SIZE_HANDLER					0x1000				// 处理器

/* 函数大小 */
#define SIZE_FUNCTION_JUMPER			0x0200				// WoW64 系统调用转发器
#define SIZE_FUNCTION_INJECTOR			0x0200				// 注入器
#define SIZE_FUNCTION_INJECTOR_LOCAL	0x0200				// 本地注入器
#define SIZE_FUNCTION_INJECTOR_REMOTE	0x0200				// 远程注入器
#define SIZE_FUNCTION_LOADER			0x0200				// 线程启动器





/* for C++ */
#include <new>

/* for C */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* for Windows API */
#include <Windows.h>





/* 远程进程 */
class remote {
	public:
		// 在目标进程中分配内存空间
		static	uintptr_t	allocate	(HANDLE process, unsigned int size);

		// 从目标进程内存读取数据
		static	bool	read	(HANDLE process, uintptr_t address, void *buffer, unsigned int size);
		// 向目标进程内存写入数据
		static	bool	write	(HANDLE process, uintptr_t address, void *buffer, unsigned int size);

		// 在目标进程中创建线程
		static	HANDLE	thread	(HANDLE process, uintptr_t address, uint32_t argument);
};




/* 安装 */
class install {
	private:
		// DLL 模块句柄
		static	HMODULE		module_ntdll;
		static	HMODULE		module_controller;


		// 安装回调表函数
		static	bool	install_table_function (uintptr_t address, uintptr_t load, const char *system, const char *callback);

		// 映射函数
		static	void	map_function_jumper				(uintptr_t base, uintptr_t information, uintptr_t table);
		static	void	map_function_injector			(uintptr_t base, uintptr_t address, uintptr_t information);
		static	void	map_function_injector_local		(uintptr_t base, uintptr_t address, uintptr_t information);
		static	void	map_function_injector_remote	(uintptr_t base, uintptr_t address, uintptr_t information);
		static	void	map_function_loader				(uintptr_t base, uintptr_t jumper);



	public:
		// 基址
		static	uintptr_t	base;

		// 常量/表/处理器基址
		static	uintptr_t	base_library;
		static	uintptr_t	base_controller;
		static	uintptr_t	base_address;
		static	uintptr_t	base_information;
		static	uintptr_t	base_table;
		static	uintptr_t	base_handler;

		// 函数基址
		static	uintptr_t	base_function_jumper;
		static	uintptr_t	base_function_injector;
		static	uintptr_t	base_function_injector_local;
		static	uintptr_t	base_function_injector_remote;
		static	uintptr_t	base_function_loader;


		// 设置基址
		static	void	update		(uintptr_t base);
		// 载入模块
		static	bool	controller	(void);

		// 构造表格
		static	void	build_address		(void);
		static	void	build_information	(void);

		// 安装回调表
		static	bool	install_table		(void);
		// 安装处理器
		static	void	install_handler		(void);

		// 映射函数
		static	void	map_function		(void);
};




#endif   /* HEADER */