/*
	$ Invoke   (C) 2005-2012 MF
	$ wHij.c, 0.1.1128
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_HIJACK"
#define TITLE			"Invoke - 选择需要劫持的函数"
#define WIDTH			700
#define HEIGHT			450


static int		 SEI = -1;
static hijack_s *SEL = NULL;


static void hijackShow(hijack_s *hij, int index) {
	char		buffer[16];
	char		*data[] = {NULL, NULL, buffer, NULL, NULL, NULL};

	function_s	*function;
	module_s	*module;

	function	= hij->function;
	module		= function->module;

	data[0]		= module->modname;

	if((data[1] = function->name) == NULL) {
		data[1] = "N/A";
	}

	sprintf(data[2], "%ld", function->ordinal);

	switch(hij->invoke) {
		case CL_CDECL:		data[3] = "cdecl";		break;
		case CL_STDCALL:	data[3] = "stdcall";	break;
		case CL_FASTCALL:	data[3] = "fastcall";	break;
	}

	data[4] = type2string(&hij->ret);
	data[5] = typeArgument(hij->argList, hij->vararg);

	if(index == -1) {
		LIST(lsHijFunction)->append(data);
	} else {
		LIST(lsHijFunction)->modify(index, data);
	}

	DEL(data[4]);
	DEL(data[5]);
}


static void guiHijDraw(erp_window_s *win) {
	module_s *mod = MOD->next;

	COMBO(cmbHijModule)->append("所有模块");

	while(mod) {
		COMBO(cmbHijModule)->append(mod->filename);

		mod = mod->next;
	}

	OBJECT(bHijProto)->disable(TRUE);
	OBJECT(bHijRemove)->disable(TRUE);

	COMBO(cmbHijModule)->select(0);
}

static void * guiHijDestroy(erp_window_s *win, DWORD arg) {
	if(HIJ->next == NULL) {
		if(!confirmNo(win->hwnd, "没有设置劫持任何函数，这将无法发挥 Invoke 的功能，是否继续？")) {
			win->cancel = 1; return NULL;
		}
	}

	hookInstallAll(win->hwnd);

	return NULL;
}

void guiHij(HWND win) {
	char	*title[]	= {"模块", "函数名称", "导入序号", "调用规范", "返回类型", "参数表"};
	size_t	width[]		= {90, 150, 65, 65, 100, 160};

	ZeroMemory(&wHij, sizeof wHij);

	wHij.classname	= CLASSNAME;
	wHij.title		= TITLE;
	wHij.width		= WIDTH;
	wHij.height		= HEIGHT;
	wHij.instance	= RT.instance;
	wHij.draw		= guiHijDraw;
	wHij.destroy	= guiHijDestroy;
	wHij.center		= TRUE;
	wHij.tool		= TRUE;
	wHij.parent		= win;

	cmbHijModule	= DRAW_COMBO	(wHij, HIJ_COMBO_MODULE,	  15,  15, 470,  20, cbHijComboModule);
	lsHijFunction	= DRAW_LIST		(wHij, HIJ_LIST_FUNCTION,	  15,  50, 665, 310, title, width, sizeof title / sizeof(char *), cbHijListFunction);

	bHijAdd			= DRAW_BUTTON	(wHij, HIJ_BUTTON_ADD,		 -15,  11,  80,  28, "添加",		cbHijButtonAdd);
	bHijProto		= DRAW_BUTTON	(wHij, HIJ_BUTTON_PROTO,	  15, -15, 100,  28, "函数原型",	cbHijButtonProto);
	bHijRemove		= DRAW_BUTTON	(wHij, HIJ_BUTTON_REMOVE,	 120, -15, 100,  28, "取消劫持",	cbHijButtonRemove);
	bHijImport		= DRAW_BUTTON	(wHij, HIJ_BUTTON_IMPORT,	-230, -15,  80,  28, "导入",		cbHijButtonImport);
	bHijExport		= DRAW_BUTTON	(wHij, HIJ_BUTTON_EXPORT,	-145, -15,  80,  28, "导出",		cbHijButtonExport);
	bHijOk			= DRAW_BUTTON	(wHij, HIJ_BUTTON_OK,		 -15, -15, 100,  28, "确定",		cbHijButtonOk);

	erpWindow(&wHij);
}


ERP_CALLBACK(cbHijComboModule) {
	int i;

	module_s	*module;
	function_s	*function;
	hijack_s	*hijack;

	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	LIST(lsHijFunction)->clear();

	OBJECT(bHijProto)->disable(TRUE);
	OBJECT(bHijRemove)->disable(TRUE);

	if(arg == 0) {
		hijack = HIJ->next;

		while(hijack) {
			hijackShow(hijack, -1);
			hijack = hijack->next;
		}
	} else {
		i = 1;
		module = MOD->next;

		while(module) {
			if(i == arg) {
				break;
			}

			i++;
			module = module->next;
		}

		if((function = module->function) == NULL) {
			return;
		}

		while(function) {
			if(hijack = function->hijack) {
				hijackShow(hijack, -1);
			}

			function = function->next;
		}
	}

	SEI = -1;
	SEL = NULL;
}

ERP_CALLBACK(cbHijListFunction) {
	module_s	*module;
	function_s	*function;
	hijack_s	*hijack;

	int		i, j;
	BOOL	disable = FALSE;

	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	SEI = (int) arg;

	if(arg == -1) {
		SEL		= NULL;
		disable	= TRUE;
	} else {
		SEL = NULL;

		if(j = COMBO(cmbHijModule)->selected) {
			i = 1;
			module = MOD->next;

			while(module) {
				if(i == j) {
					break;
				}

				i++;
				module = module->next;
			}

			i = 0;
			function = module->function;

			while(function) {
				if(function->hijack) {
					if(i == arg) {
						SEL = function->hijack; break;
					}

					i++;
				}

				function = function->next;
			}
		} else {
			i = 0;
			hijack = HIJ->next;

			while(hijack) {
				if(i == arg) {
					SEL = hijack; break;
				}

				i++;
				hijack = hijack->next;
			}
		}
	}

	OBJECT(bHijProto)->disable(disable);
	OBJECT(bHijRemove)->disable(disable);
}

ERP_CALLBACK(cbHijButtonAdd) {
	if(guiFunc(win->hwnd)) {
		cbHijComboModule(win, (erp_object_s *) cmbHijModule, SELECT, COMBO(cmbHijModule)->selected);

		OBJECT(bHijProto)->disable(TRUE);
		OBJECT(bHijRemove)->disable(TRUE);
	}
}

ERP_CALLBACK(cbHijButtonProto) {
	if(SEL == NULL) {
		return;
	}

	guiPrt(SEL, win->hwnd);
	hijackShow(SEL, SEI);
}

ERP_CALLBACK(cbHijButtonRemove) {
	if(SEL == NULL) {
		return;
	}

	SEL->function->hijack = NULL;
	SEL->prev->next = SEL->next;

	if(SEL->next) {
		SEL->next->prev = SEL->prev;
	}

	hookUninstall(SEL->function);

	DEL(SEL);

	LIST(lsHijFunction)->remove(SEI);

	SEI = -1;
}

ERP_CALLBACK(cbHijButtonImport) {
	if(packageLoad(win->hwnd, NULL, FALSE)) {
		COMBO(cmbHijModule)->select(0);
	}
}

ERP_CALLBACK(cbHijButtonExport) {
	packageCreate(win->hwnd);
}

ERP_CALLBACK(cbHijButtonOk) {
	win->quit(0);
}
