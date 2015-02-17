/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.exe/base.h   # 1312
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



/* DLL 文件名 */
#define TRACKER_DLL_FILENAME			"tracker.dll"




/* for C++ */
#include <new>

/* for C */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* for Windows API */
#include <Windows.h>





/* 加载器 */
class load {
	private:
		// DLL 路径
		static	char *	library;


		// 生成 DLL 路径
		static	void	build_library	(void);
		// 生成默认启动信息
		static	void	build_argument	(const char *argument);


		// 执行线程
		static	int		__stdcall	thread	(void *dummy);



	public:
		// 启动信息
		static	char *	command;
		// 控制器
		static	char *	controller;


		// 初始化信息
		static	void	start	(const char *argument);

		// 启动进程
		static	void	run		(void);
};




/* GUI */
class gui {
	private:
		// 当前状态
		static	HANDLE	status;

		// 应用程序实例
		static	HINSTANCE	instance;

		// 窗口句柄
		static	HWND	window;
		// 控件句柄
		static	HWND	component	[8];


		// 选择文件
		static	void	browse	(void);

		// 禁止操作
		static	void	disable	(bool status);


		// 绘制窗口
		static	int		draw	(HWND hwnd);
		// 销毁窗口
		static	bool	close	(void);
		// 事件处理
		static	void	command	(WPARAM wparam);

		// 消息循环
		static	LRESULT __stdcall	loop	(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);



	public:
		// 进入 GUI
		static	void	start	(HINSTANCE instance, int show);

		// 响应成功
		static	void	response_succeed	(void);
		// 响应失败
		static	void	response_error		(void);
		static	void	response_error		(const char *error);
};



#endif   /* HEADER */