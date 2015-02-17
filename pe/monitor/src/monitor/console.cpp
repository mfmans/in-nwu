/*
	$ Monitor Module   (C) 2005-2014 MF
	$ console.cpp
*/


#include "header/base.h"



/* 控制台句柄 */
void *	Console::handle		= NULL;




/* 创建控制台窗口 */
bool Console::open (void) {
	// 分配控制台
	if(AllocConsole() == FALSE) {
		return false;
	}

	// 获取控制台输出句柄
	Console::handle	= (void *) GetStdHandle(STD_OUTPUT_HANDLE);

	return true;
}



/* 设置颜色 */
void Console::color (bool red, bool green, bool blue, bool intensity) {
	// 如果没有句柄, 不设置直接返回
	if(Console::handle == NULL) {
		return;
	}

	// 属性值
	unsigned int attribute	= 0;

	// 合并颜色
	attribute	|= (red			== true) ? FOREGROUND_RED		: 0;
	attribute	|= (green		== true) ? FOREGROUND_GREEN		: 0;
	attribute	|= (blue		== true) ? FOREGROUND_BLUE		: 0;
	attribute	|= (intensity	== true) ? FOREGROUND_INTENSITY	: 0;

	// 设置属性
	SetConsoleTextAttribute((HANDLE) Console::handle, (WORD) attribute);
}




/* 格式化输出 */
void Console::printf (const char *format, ...) {
	// 计算参数位置
	uintptr_t argument = ((uintptr_t) & format) + sizeof(uintptr_t);

	// 执行输出
	Console::vprintf(format, (void *) argument);
}


/* 动态参数格式化输出 */
void Console::vprintf (const char *format, void *argument) {
	// 如果没有句柄, 直接返回
	if(Console::handle == NULL) {
		return;
	}

	// 输出大小
	int		length	= _vsnprintf(NULL, 0, format, (va_list) argument);
	// 输出缓冲区
	char *	buffer	= NULL;

	// 判断长度是否为负数
	if(length <= 0) {
		// 分配固定大小缓冲区
		buffer	= new char [64];

		// 输出错误
		sprintf(buffer, "{FAILED TO EXECUTE vsnprintf()}\n");

		// 计算长度
		length	= strlen(buffer);
	} else {
		// 分配缓冲区
		buffer = new char [length + 4];

		// 生成字符串
		vsnprintf(buffer, length, format, (va_list) argument);
	}

	// 输出文字
	WriteConsoleA((HANDLE) Console::handle, buffer, (DWORD) length, (LPDWORD) & length, NULL);

	// 释放字符串
	delete [] buffer;
}




/* 执行输出 */
void Console::output (void *p) {
	// 格式化文本
	const char *	format		= *((const char **) p);
	// 参数表
	va_list			argument	= (va_list) ((uintptr_t) p + sizeof(uintptr_t));

	// 如果格式第一个字符为 #, 按格式输出
	if(*format == '#') {
		// 查找结束的字符
		const char * end	= strchr(format + 1, '#');

		// 获取时间
		time_t	timestamp	= time(NULL);
		// 转换为本地时间
		tm *	timeinfo	= localtime(& timestamp);

		// 时间缓冲区
		char	timebuffer	[128];

		// 生成时间字符串
		strftime(timebuffer, sizeof timebuffer, "[%b %d %H:%M:%S]", timeinfo);

		if(end == NULL) {
			// 如果没有找到结束符号, 只输出时间
			Console::printf("%s ", timebuffer);

			// 跳过第一个字符 #
			format ++;
		} else {
			// 前缀长度
			int		length	= end - format - 1;
			// 前缀字符串
			char *	prefix	= new char [length + 1];

			// 复制前缀
			memcpy(prefix, format + 1, (size_t) length);
			// 结束字符串
			memset(prefix + length, 0, sizeof(char));

			// 输出信息
			Console::printf("%s - %s ", prefix, timebuffer);

			// 释放前缀字符串
			delete [] prefix;

			// 跳过前缀
			format = format + length + 2;
		}
	}

	Console::vprintf(format, argument);
}




/* 输出白色提示 */
void Console::notice (const char *format, ...) {
	// 白色
	Console::color	(true,	true,	true,	false);
	// 输出文字
	Console::output	(& format);
}


/* 输出绿色警告 */
void Console::warning (const char *format, ...) {
	// 绿色
	Console::color	(false,	true,	false,	false);
	// 输出文字
	Console::output	(& format);
}


/* 输出红色错误 */
void Console::error (const char *format, ...) {
	// 红色
	Console::color	(true,	false,	false,	true);
	// 输出文字
	Console::output	(& format);
}

