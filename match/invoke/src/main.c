/*
	$ Invoke   (C) 2005-2012 MF
	$ main.c, 0.1.1124
*/

#pragma comment (linker, "/subsystem:\"windows\"")

#define _INVOKE_MAIN_

#include "base.h"
#include "gui.h"


/* 错误报告程序 */
void mainError(char *command) {
	if(strlen(command) < 9) {
		return;
	}
	if(strncmp(command, "--error ", 8)) {
		return;
	}

	error(NULL, command + 8);

	ExitProcess(EXIT_FAILURE);
}

/* Win32 主入口 */
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	/* 导入函数 */
	Function(&processSuspend,	"ntdll.dll",	"NtSuspendProcess", FUNCTION_PASSIVE);
	Function(&processResume,	"ntdll.dll",	"NtResumeProcess",	FUNCTION_PASSIVE);
	Function(&disasm,			"disasm.dll",	"disasm",			FUNCTION_PASSIVE);

	/* 错误报告 */
	mainError(lpCmdLine);

	/* @ RUNTIME */
	ZeroMemory(&RT, sizeof RT);
	/* @ HIJACKED-FUNCTIONS/MODULES for debugging process */
	NEW(HIJ, hijack_s, 1);
	NEW(MOD, module_s, 1);

	/* 初始化信号 */
	symbolInit();

	/* 当前进程信息 */
	RT.instance	= hInstance;
	RT.process	= GetCurrentProcess();
	/* 创建调试器线程 */
	RT.debug	= (HANDLE) threadRun(runDebug, NULL);

	/* 初始化 Europa GUI */
	erpInit();

	/* 选择进程 */
	guiProc();

	/* 模块函数断点设置 */
	moduleShow();

	/* 载入 ivh 文件 */
	packageAuto();
	/* 手动设置劫持函数 */
	guiHij(NULL);

	/* 主界面 */
	guiMain();

	/* 调试结束，解除断点 */
	if(RT.uninstall) {
		hookUninstallAll();
	}
}
