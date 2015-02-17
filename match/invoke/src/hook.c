/*
	$ Invoke   (C) 2005-2012 MF
	$ hook.c, v2 @ 0.1.1202
*/

#include "base.h"


void hookSetCode(function_s *func) {
	if(func == NULL) {
		return;
	}

	if(processWrite(func->bp, &func->code, 1) == 0) {
		die("Invoke 出现错误，无法写入被调试进程的内存，应用程序已被迫终止。");
	}
}

void hookSetBp(function_s *func) {
	char bkpoint = 0xCC;

	if(func == NULL) {
		return;
	}

	if(processWrite(func->bp, &bkpoint, sizeof bkpoint) == 0) {
		die("Invoke 出现错误，无法写入被调试进程的内存，应用程序已被迫终止。");
	}
}


int hookInstall(function_s *func) {
	char bkpoint = 0xCC;

	if(func->hook) {
		return 1;
	}

	/* 保存旧指令 */
	if(processRead(func->bp, &func->code, 1) == 0) {
		return 0;
	}

	/* 写入 INT3 */
	if(processWrite(func->bp, &bkpoint, sizeof bkpoint) == 0) {
		return 0;
	}

	return 1;
}

void hookInstallAll(HWND win) {
	module_s	*module		= MOD->next;
	function_s	*function	= NULL;

	while(module) {
		function = module->function;
		
		while(function) {
			if((function->hijack) && (function->hook == FALSE)) {
				if(hookInstall(function)) {
					function->hook = TRUE;
				} else {
					if(function->name) {
						warningFormat(win, "向模块 %s 中的函数 %s 安装断点失败。", function->module, function->name);
					} else {
						warningFormat(win, "向模块 %s 中的导入函数 %ld 安装断点失败。", function->module, function->ordinal);
					}
				}
			}

			function = function->next;
		}

		module = module->next;
	}
}


int hookUninstall(function_s *func) {
	if(func->hook == FALSE) {
		return 1;
	}

	if(processWrite(func->bp, &func->code, 1) == 0) {
		return 0;
	}

	func->hook = FALSE;

	return 1;
}

void hookUninstallAll() {
	int failed = 0;

	module_s	*module = MOD->next;
	function_s	*function;

	while(module) {
		function = module->function;

		while(function) {
			if(function->hook) {
				if(processWrite(function->bp, &function->code, 1) == 0) {
					failed = 1;
				}
			}

			function = function->next;
		}

		module = module->next;
	}

	if(failed) {
		warning(NULL, "部分断点卸载失败，被调试的进程可能工作不正常。");
	}
}


function_s * hookFind(DWORD address, module_s **module) {
	module_s	*mod		= MOD->next;
	function_s	*function	= NULL;

	while(mod) {
		if((address >= mod->base) && ((mod->size == 0) || (address < (mod->base + mod->size)))) {
			function = mod->function;

			while(function) {
				if(address == function->bp) {
					*module = mod;

					return function;
				}

				function = function->next;
			}
		}

		mod = mod->next;
	}

	return NULL;
}
