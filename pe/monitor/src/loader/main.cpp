/*
	$ Monitor Loader   (C) 2005-2014 MF
	$ main.cpp
*/


#include "base.h"



/* 分配内存失败回调函数 */
static void __cdecl callback_new_exception (void) {
	// 错误提示
	MessageBoxA(NULL, "分配内存失败，应用程序被迫终止。", "", MB_ICONERROR | MB_OK);

	// 结束程序
	exit(0);
}



/* 启动参数分割 */
static void command_split (const char *argument, char **run, char **handler, bool *console) {
	// 启动参数长度
	size_t	length	= strlen(argument);

	// 如果没有启动参数, 不进行分割处理
	if(length == 0) {
		return;
	}

	// 分配启动参数副本内存
	char *	command	= new char [length + 1];

	// 生成启动参数副本
	strcpy(command, argument);

	// 判断是否以 @"..." 开头
	if((*command == '@') && (*(command + 1) == '"')) {
		// 指向控制器
		*handler	= command + 2;
		// 指向启动信息
		*run		= strchr(*handler, '"');

		// 判断控制器路径是否完整
		if(*run == NULL) {
			// 控制器路径不完整
			*handler	= NULL;
		} else {
			// 将原来指向的 '"' 修改为字符串结束
			**run	= '\0';

			// 移向下一个字符
			(*run) ++;

			// 查找第一个非空格
			while((*run != '\0') && isspace((int) **run)) {
				(*run) ++;
			}
		}
	} else {
		// 指向启动信息
		*run	= command;
	}

	// 判断是否隐藏控制台窗口
	if(*run != NULL) {
		if(**run == '!') {
			// 设置控制台窗口为隐藏状态
			*console = false;

			// 移向下一个字符
			(*run) ++;
		}
	}

	// 判断启动信息
	if((*run != NULL) && (**run == '\0')) {
		*run = NULL;
	}

	// 判断控制器
	if((*handler != NULL) && (**handler == '\0')) {
		*handler = NULL;
	}

	// 如果既没有启动信息又没有控制器, 释放启动参数备份
	if((*run == NULL) && (*handler == NULL)) {
		delete [] command;
	}
}




/* 主入口 */
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 启动信息
	char *	run		= NULL;
	// 控制器路径
	char *	handler	= NULL;
	// 显示控制台窗口
	bool	console	= true;

	// 设置 new 失败回调函数
	std::set_new_handler (callback_new_exception);

	// 分割启动参数
	command_split (lpCmdLine, & run, & handler, & console);

	// 进入 GUI
	return gui_start (run, handler, console, (void *) hInstance, nCmdShow);
}

