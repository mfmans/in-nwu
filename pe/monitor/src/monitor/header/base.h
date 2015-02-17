/*
	$ Monitor Module   (C) 2005-2014 MF
	$ base.h
*/


#ifndef _MODULE_BASE_H_
#define _MODULE_BASE_H_


#pragma warning (disable: 4800)
#pragma warning (disable: 4996)


#ifdef UNICODE
	#undef UNICODE
#endif

#ifdef _UNICODE
	#undef _UNICODE
#endif



#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <Windows.h>




/* 控制台 */
class Console {
	private:
		static	void *	handle;


	public:
		/* 创建控制台窗口 */
		static	bool	open	(void);

		/* 设置颜色 */
		static	void	color	(bool red, bool green, bool blue, bool intensity);

		/* 格式化输出 */
		static	void	printf	(const char *format, ...);
		/* 动态参数格式化输出 */
		static	void	vprintf	(const char *format, void *argument);

		/* 执行输出 */
		static	void	output	(void *p);

		/* 输出白色提示 */
		static	void	notice	(const char *format, ...);
		/* 输出绿色警告 */
		static	void	warning	(const char *format, ...);
		/* 输出红色错误 */
		static	void	error	(const char *format, ...);
};




/* 转发器 */
class Router {
	private:
		/* 强制返回值 */
		static	uintptr_t	value;


		/* 函数入口 */
		static	void *	address_createfilew;
		static	void *	address_loadlibraryexw;
		static	void *	address_getprocaddress;
		static	void *	address_createprocessw;
		static	void *	address_shellexecuteexw;
		static	void *	address_winexec;


		/* 跳转器 */
		static	void	jump_createfilew		(void);
		static	void	jump_loadlibraryexw		(void);
		static	void	jump_getprocaddress		(void);
		static	void	jump_createprocessw		(void);
		static	void	jump_shellexecuteexw	(void);
		static	void	jump_winexec			(void);


		/* 处理器 */
		static	bool	run_createfilew		(wchar_t *, uint32_t, uint32_t, void *, uint32_t, uint32_t, void *);
		static	bool	run_loadlibraryexw	(wchar_t *, void *, uint32_t);
		static	bool	run_getprocaddress	(void *, char *);
		static	bool	run_createprocessw	(wchar_t *, wchar_t *, void *, void *, int32_t, uint32_t, void *, wchar_t *, void *, void *);
		static	bool	run_shellexecuteexw	(void *);
		static	bool	run_winexec			(char *, uint32_t);


		/* 验证函数 */
		static	bool	validate	(void *p);
		/* 内存写入 */
		static	bool	write		(uintptr_t address, unsigned int size, uint8_t *buffer);
		/* 注入函数 */
		static	bool	inject		(void *target, void *source);


	public:
		/* 安装劫持代码 */
		static	int		install		(void);
};




/* 脚本处理器公共类 */
class HandlerClass {
	public:
		/* 启动 */
		virtual	bool	start	(const char *path)	= 0;
		/* 关闭 */
		virtual	void	close	(void)				= 0;

		/* 发送请求 */
		virtual	bool	send	(unsigned int index, const char *function, int argc, void *argv)	= 0;
};




/* 脚本处理器 */
class Handler {
	private:
		/* 处理器实例 */
		static	HandlerClass *	object;

		/* 载入处理器 */
		static	int		load	(const char *path, const char *extension);


	public:
		/* DLL 模块实例 */
		static	void *	dll;

		/* 打开 */
		static	int		start	(const char *path);
		/* 关闭 */
		static	void	close	(void);

		/* 发送请求 */
		static	bool	send	(unsigned int index, const char *function, int argc, ...);
};



#endif   /* BASE */