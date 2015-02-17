/*
	$ Monitor Module   (C) 2005-2014 MF
	$ handler.cpp
*/


#include "header/base.h"
#include "header/error.h"
#include "header/handler.h"



/* 处理器实例 */
HandlerClass *	Handler::object	= NULL;

/* DLL 模块实例 */
void *			Handler::dll	= NULL;




/* 载入处理器 */
int Handler::load (const char *path, const char *extension) {
	// 错误
	int	error	= 0;

	// DLL
	if(stricmp(extension, "dll") == 0) {
		Handler::object = new handler_dll();

		error	= MM_ERROR_HANDLER_DLL;

		goto start;
	}

	// PHP
	if(stricmp(extension, "php") == 0) {
		Handler::object = new handler_php();

		error	= MM_ERROR_HANDLER_PHP;

		goto start;
	}

	// 无法识别的控制器
	return MM_ERROR_HANDLER_UNKNOWN;


start:
	// 启动控制器
	if(Handler::object->start(path) == false) {
		// 弹出错误提示
		MessageBoxA(NULL, "启动控制器失败。\n如果控制台窗口正在显示，请根据输出信息获取错误原因。点击确定后窗口将自动关闭。", "", MB_ICONERROR | MB_OK);

		// 关闭控制器
		Handler::object->close();

		// 返回错误
		return error;
	}

	// 返回成功
	return MM_ERROR_SUCCEED;
}




/* 打开 */
int Handler::start (const char *path) {
	// 没有启动参数
	if(path == NULL) {
		return MM_ERROR_SUCCEED;
	}

	// 启动参数为空字符串
	if(strlen(path) == 0) {
		return MM_ERROR_SUCCEED;
	}

	// 查找扩展名
	const char *	extension	= strrchr(path, '.');

	// 如果没有找到扩展名, 无法使用对应的处理器
	if(extension == NULL) {
		return MM_ERROR_HANDLER_UNKNOWN;
	}

	// 载入处理器
	return Handler::load (path, extension + 1);
}


/* 关闭 */
void Handler::close (void) {
	// 没有处理器
	if(Handler::object == NULL) {
		return;
	}

	// 调用处理器关闭方法
	Handler::object->close();
}



/* 发送请求 */
bool Handler::send (unsigned int index, const char *function, int argc, ...) {
	// 没有处理器
	if(Handler::object == NULL) {
		// 直接允许继续
		return true;
	}

	// 参数地址
	void *	argv	= (void *) ((uintptr_t) & argc + sizeof(uintptr_t));

	// 调用处理器请求处理方法
	return Handler::object->send(index, function, argc, argv);
}

