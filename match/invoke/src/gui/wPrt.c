/*
	$ Invoke   (C) 2005-2012 MF
	$ wPrt.c, 0.1.1201
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_PROTO"
#define TITLE			"函数原型"
#define WIDTH			400
#define HEIGHT			510


static argument_s *argvFind(int index, hijack_s *hij, argument_s **prev) {
	int i = 0;
	argument_s *argv = hij->argList;

	*prev = NULL;

	if(index == -1) {
		return NULL;
	}
	if((hij->argCount == 0) || (hij->argCount <= (size_t) index)) {
		return NULL;
	}

	while(argv) {
		if(index == i) {
			return argv;
		}

		i++;

		*prev = argv;
		argv = argv->next;
	}

	return NULL;
}

static void argvShow(argument_s *arg, int index) {
	char *data[2] = {
		arg->name,
		type2string(&arg->type)
	};

	if(index < 0) {
		LIST(lsPrtArgs)->append(data);
	} else {
		LIST(lsPrtArgs)->modify(index, data);
	}

	DEL(data[1]);
}


static void guiPrtDraw(erp_window_s *win) {
	char *data[2];

	hijack_s	*hij	= (hijack_s *) win->param;
	argument_s	*argv	= hij->argList;

	switch(hij->invoke) {
		case CL_CDECL:		RADIO(rPrtInvokeC)->select();		break;
		case CL_STDCALL:	RADIO(rPrtInvokeStd)->select();		break;
		case CL_FASTCALL:	RADIO(rPrtInvokeFast)->select();	break;
	}

	OBJECT(rPrtInvokeFast)->disable(TRUE);

	if(hij->function->hook) {
		OBJECT(bPrtBk)->caption("查看");
	}

	if(hij->vararg) {
		CHECK(cPrtVararg)->check(TRUE);
	}
	if(hij->esp) {
		CHECK(cPrtEsp)->check(TRUE);
	}

	while(argv) {
		data[0] = argv->name;
		data[1] = type2string(&argv->type);

		LIST(lsPrtArgs)->append(data);

		argv = argv->next;
	}

	typeComboInit(cmbPrtReturn, &hij->ret, TRUE);

	EDIT(ePrtBk)->setText("%08X", hij->function->bp);
	EDIT(ePrtBk)->readonly(TRUE);

	OBJECT(bPrtArgsEdit)->disable(TRUE);
	OBJECT(bPrtArgsDel)->disable(TRUE);
	OBJECT(bPrtArgsMvUp)->disable(TRUE);
	OBJECT(bPrtArgsMvDown)->disable(TRUE);
}

void guiPrt(hijack_s *hij, HWND win) {
	char	*title[]	= {"名称", "类型"};
	size_t	width[]		= {110, 225};

	ZeroMemory(&wPrt, sizeof wPrt);

	wPrt.classname	= CLASSNAME;
	wPrt.title		= TITLE;
	wPrt.width		= WIDTH;
	wPrt.height		= HEIGHT;
	wPrt.instance	= RT.instance;
	wPrt.draw		= guiPrtDraw;
	wPrt.center		= TRUE;
	wPrt.tool		= TRUE;
	wPrt.parent		= win;
	wPrt.param		= (DWORD) hij;

	lbPrtInvoke		= DRAW_LABEL	(wPrt, PRT_LABEL_INVOKE,		 15,  15,  100,  20, "调用规范：");
	rPrtInvokeC		= DRAW_RADIO	(wPrt, PRT_RADIO_INVOKE_C,		 95,  11,   50,  20, "cdecl",		1, cbPrtRadioInvokeC);
	rPrtInvokeStd	= DRAW_RADIO	(wPrt, PRT_RADIO_INVOKE_STD,	170,  11,   60,  20, "stdcall",		1, cbPrtRadioInvokeStd);
	rPrtInvokeFast	= DRAW_RADIO	(wPrt, PRT_RADIO_INVOKE_FAST,	255,  11,   70,  20, "fastcall",	1, cbPrtRadioInvokeFast);

	lbPrtReturn		= DRAW_LABEL	(wPrt, PRT_LABEL_RETURN,		 15,  42,  100,  20, "返回类型：");
	cmbPrtReturn	= DRAW_COMBO	(wPrt, PRT_COMBO_RETURN,		 95,  38,  250,  20, cbPrtComboReturn);

	lbPrtBk			= DRAW_LABEL	(wPrt, PRT_LABEL_BK,			 15,  69,  100,  20, "断点地址：");
	ePrtBk			= DRAW_EDIT		(wPrt, PRT_EDIT_BK,				 95,  65,  170,  20, NULL);
	bPrtBk			= DRAW_BUTTON	(wPrt, PRT_BUTTON_BK,			275,  63,   70,  24, "设置",	cbPrtButtonBk);

	lbPrtArgs		= DRAW_LABEL	(wPrt, PRT_LABEL_ARGS,			 15, 120,  100,  20, "调用参数：");
	lsPrtArgs		= DRAW_LIST		(wPrt, PRT_LIST_ARGS,			 15, 140,  365, 150, title, width, sizeof title / sizeof(char *), cbPrtListArgs);
	bPrtArgsAdd		= DRAW_BUTTON	(wPrt, PRT_BUTTON_ARGS_ADD,		 15, 295,   60,  24, "添加", cbPrtButtonArgsAdd);
	bPrtArgsEdit	= DRAW_BUTTON	(wPrt, PRT_BUTTON_ARGS_EDIT,	 80, 295,   60,  24, "编辑", cbPrtButtonArgsEdit);
	bPrtArgsDel		= DRAW_BUTTON	(wPrt, PRT_BUTTON_ARGS_DEL,		145, 295,   60,  24, "删除", cbPrtButtonArgsDel);
	bPrtArgsMvUp	= DRAW_BUTTON	(wPrt, PRT_BUTTON_ARGS_MVUP,	-80, 295,   60,  24, "上移", cbPrtButtonArgsMvUp);
	bPrtArgsMvDown	= DRAW_BUTTON	(wPrt, PRT_BUTTON_ARGS_MVDOWN,	-15, 295,   60,  24, "下移", cbPrtButtonArgsMvDown);

	lbPrtMisc		= DRAW_LABEL	(wPrt, PRT_LABEL_MISC,			 15, 355,  100,  20, "其他设置：");
	cPrtVararg		= DRAW_CHECK	(wPrt, PRT_CHECK_VARARG,		 30, 375,  300,  20, "允许可变参数调用 （上方设置固定参数）",			cbPrtCheckVararg);
	cPrtEsp			= DRAW_CHECK	(wPrt, PRT_CHECK_ESP,			 30, 392,  300,  20, "在执行断点位置指令前，原栈底指针（EBP）被压入栈",	cbPrtCheckEsp);

	bPrtOk			= DRAW_BUTTON	(wPrt, PRT_BUTTON_OK,			-15, -15,   80,  28, "关闭", cbPrtButtonOk);

	erpWindow(&wPrt);
}


ERP_CALLBACK(cbPrtRadioInvokeC) {
	if(arg) {
		((hijack_s *) win->param)->invoke = CL_CDECL;
	}
}

ERP_CALLBACK(cbPrtRadioInvokeStd) {
	if(arg) {
		((hijack_s *) win->param)->invoke = CL_STDCALL;
	}
}

ERP_CALLBACK(cbPrtRadioInvokeFast) {
	if(arg) {
		((hijack_s *) win->param)->invoke = CL_FASTCALL;
	}
}

ERP_CALLBACK(cbPrtComboReturn) {
	if(typeComboChange((int) arg, P_COMBO(obj), &((hijack_s *) win->param)->ret, TRUE) == 0) {
		warning(win->hwnd, "选择了无效的类型。");

		COMBO(obj)->select(0);
	}
}

ERP_CALLBACK(cbPrtButtonBk) {
	DWORD		address;
	char		*title;

	hijack_s	*hij	= (hijack_s *) win->param;
	function_s	*func	= hij->function;

	if(hij->function->name) {
		NEW(title, char, strlen(func->name) + strlen(func->module->modname) + 16);

		sprintf(title, "[%08X] %s@%s", func->bp, func->name, func->module->modname);
	} else {
		NEW(title, char, strlen(func->module->modname) + 32);

		sprintf(title, "[%08X] #%ld@%s", func->bp, func->ordinal, func->module->modname);
	}

	if(func->hook) {
		guiAsm(win->hwnd, title, func->entry, NULL, FALSE, 0, 0);
	} else {
		address = func->bp;

		guiAsm(win->hwnd, title, func->entry, &address, TRUE, func->entry, func->entry + 10);

		if(func->bp != address) {
			EDIT(ePrtBk)->setText("%08X", address);
			func->bp = address;
		}
	}

	DEL(title);
}

ERP_CALLBACK(cbPrtListArgs) {
	int index = (int) arg;

	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	OBJECT(bPrtArgsMvUp)->disable(TRUE);
	OBJECT(bPrtArgsMvDown)->disable(TRUE);

	if(index == -1) {
		OBJECT(bPrtArgsEdit)->disable(TRUE);
		OBJECT(bPrtArgsDel)->disable(TRUE);
	} else {
		OBJECT(bPrtArgsEdit)->disable(FALSE);
		OBJECT(bPrtArgsDel)->disable(FALSE);

		if(index > 0) {
			OBJECT(bPrtArgsMvUp)->disable(FALSE);
		}
		if((index >= 0) && (index <= (((int) ((hijack_s *) win->param)->argCount) - 2))) {
			OBJECT(bPrtArgsMvDown)->disable(FALSE);
		}
	}
}

ERP_CALLBACK(cbPrtButtonArgsAdd) {
	hijack_s	*hij = (hijack_s *) win->param;
	argument_s	*argv, *next;

	if(argv = guiArg(NULL, win->hwnd)) {
		argvShow(argv, -1);

		if(next = hij->argList) {
			while(next->next) {
				next = next->next;
			}

			next->next = argv;
		} else {
			hij->argList = argv;
		}

		hij->argCount++;
	}
}

ERP_CALLBACK(cbPrtButtonArgsEdit) {
	int index;

	hijack_s	*hij = (hijack_s *) win->param;
	argument_s	*argv, *prev;

	if((index = LIST(lsPrtArgs)->selected) < 0) {
		return;
	}
	if((argv = argvFind(index, hij, &prev)) == NULL) {
		return;
	}

	guiArg(argv, win->hwnd);
	argvShow(argv, index);
}

ERP_CALLBACK(cbPrtButtonArgsDel) {
	int index;

	hijack_s	*hij = (hijack_s *) win->param;
	argument_s	*argv, *prev;

	if((index = LIST(lsPrtArgs)->selected) < 0) {
		return;
	}
	if((argv = argvFind(index, hij, &prev)) == NULL) {
		return;
	}

	hij->argCount--;

	LIST(lsPrtArgs)->remove(index);

	if(prev) {
		prev->next = argv->next;
	} else {
		hij->argList = argv->next;
	}

	DEL(argv);
}

ERP_CALLBACK(cbPrtButtonArgsMvUp) {
	int index;

	hijack_s	*hij = (hijack_s *) win->param;
	argument_s	*argv, *last, *prev, *next;

	if((index = LIST(lsPrtArgs)->selected) <= 0) {
		return;
	}

	if((argv = argvFind(index, hij, &last)) == NULL) {
		return;
	} else if(last == NULL) {
		return;
	}

	argvFind(index - 1, hij, &prev);

	/* 交换 next 域 */
	next		= argv->next;
	argv->next	= last;
	last->next	= next;

	if(prev) {
		prev->next = argv;
	} else {
		hij->argList = argv;
	}

	argvShow(last, index);
	argvShow(argv, index - 1);

	LIST(lsPrtArgs)->select(index - 1);
}

ERP_CALLBACK(cbPrtButtonArgsMvDown) {
	int index;

	hijack_s	*hij = (hijack_s *) win->param;
	argument_s	*argv, *prev, *next;

	if((index = LIST(lsPrtArgs)->selected) < 0) {
		return;
	}

	if((argv = argvFind(index, hij, &prev)) == NULL) {
		return;
	} else if(argv->next == NULL) {
		return;
	}

	next		= argv->next;
	argv->next	= next->next;
	next->next	= argv;

	if(prev) {
		prev->next = next;
	} else {
		hij->argList = next;
	}

	argvShow(next, index);
	argvShow(argv, index + 1);

	LIST(lsPrtArgs)->select(index + 1);
}

ERP_CALLBACK(cbPrtCheckVararg) {
	((hijack_s *) win->param)->vararg = (BOOL) arg;
}

ERP_CALLBACK(cbPrtCheckEsp) {
	((hijack_s *) win->param)->esp = (BOOL) arg;
}

ERP_CALLBACK(cbPrtButtonOk) {
	win->quit(0);
}
