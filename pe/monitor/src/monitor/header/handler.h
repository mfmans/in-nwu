/*
	$ Monitor Module   (C) 2005-2014 MF
	$ handler.h
*/


#ifndef _MODULE_HANDLER_H_
#define _MODULE_HANDLER_H_



/* 控制函数数量 */
#define MM_HANDLER_MAX				0x04




/* for DLL */
class handler_dll : public HandlerClass {
	private:
		/* DLL 句柄 */
		void *		handle;

		/* 函数指针 */
		uintptr_t 	pointer		[MM_HANDLER_MAX];

		/* 加载函数 */
		bool	load	(unsigned int index, const char *name);


	public:
		virtual	bool	start	(const char *path);
		virtual	void	close	(void);

		virtual	bool	send	(unsigned int index, const char *function, int argc, void *argv);
};




/* for PHP */
class handler_php : public HandlerClass {
	private:
		/* 进程句柄 */
		void *	process;
		
		/* 流指针 */
		void *	stream_in;
		void *	stream_out;
		void *	stream_error;

		/* 读取管道 */
		char *	read	(void *stream, unsigned int *size, unsigned int timeout);
		/* 写入管道 */
		bool	write	(const char *format, ...);

		/* 刷新进程状态 */
		void	refresh	(void);

		/* 获取启动参数 */
		char *	command	(void);


	public:
		virtual	bool	start	(const char *path);
		virtual	void	close	(void);

		virtual	bool	send	(unsigned int index, const char *function, int argc, void *argv);
};




#endif   /* HANDLER */