/*
	$ Invoke   (C) 2005-2012 MF
	$ wArg.c, 0.1.1201
*/


#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_ARGUMENT"
#define TITLE			"参数"
#define WIDTH			350
#define HEIGHT			170


/* 临时类型 */
static type_s temp;


static void guiArgDraw(erp_window_s *win) {
	argument_s *arg = (argument_s *) win->param;

	if(arg) {
		EDIT(eArgName)->setText("%s", arg->name);
	}

	typeComboInit(cmbArgType, &temp, TRUE);
}

static void * guiArgDestroy(erp_window_s *win, DWORD arg) {
	char *name;
	argument_s *argv = (argument_s *) win->param;

	if(arg == 0) {
		return NULL;
	}

	if((name = EDIT(eArgName)->getText()) == NULL) {
		warning(win->hwnd, "输入的参数名无效。");

		win->cancel = 1; return NULL;
	}
	if((name[0] == 0) || (strlen(name) >= 64)) {
		warning(win->hwnd, "输入的参数名长度不合法。");

		win->cancel = 1; return NULL;
	}

	if(argv) {
		DEL(argv->name);
	} else {
		NEW(argv, argument_s, 1);
	}

	argv->name = name;

	memcpy(&argv->type, &temp, sizeof(type_s));

	return argv;
}

argument_s * guiArg(argument_s *argv, HWND win) {
	/* 复制临时类型 */
	if(argv) {
		memcpy(&temp, &argv->type, sizeof(type_s));
	} else {
		typeSet(&temp, US_INT_32, 0, 0);
	}

	ZeroMemory(&wArg, sizeof wArg);

	wArg.classname	= CLASSNAME;
	wArg.title		= TITLE;
	wArg.width		= WIDTH;
	wArg.height		= HEIGHT;
	wArg.instance	= RT.instance;
	wArg.draw		= guiArgDraw;
	wArg.destroy	= guiArgDestroy;
	wArg.center		= TRUE;
	wArg.tool		= TRUE;
	wArg.parent		= win;
	wArg.param		= (DWORD) argv;

	lbArgName		= DRAW_LABEL	(wArg, ARG_LABEL_NAME,		 15,  15, 100, 20, "参数名称：");
	lbArgType		= DRAW_LABEL	(wArg, ARG_LABEL_TYPE,		 15,  43, 100, 20, "参数类型：");

	eArgName		= DRAW_EDIT		(wArg, ARG_EDIT_NAME,		 90,  13, 240, 20, NULL);
	cmbArgType		= DRAW_COMBO	(wArg, ARG_COMBO_TYPE,		 90,  40, 240, 20, cbArgComboType);

	bArgOk			= DRAW_BUTTON	(wArg, ARG_BUTTON_OK,		-90, -15,  70, 26, "确定", cbArgButtonOk);
	bArgCancel		= DRAW_BUTTON	(wArg, ARG_BUTTON_CANCEL,	-15, -15,  70, 26, "取消", cbArgButtonCancel);

	return (argument_s *) erpWindow(&wArg);
}


ERP_CALLBACK(cbArgComboType) {
	if(typeComboChange((int) arg, P_COMBO(obj), &temp, TRUE) == 0) {
		warning(win->hwnd, "选择了无效的类型。");

		COMBO(obj)->select(0);
	}
}

ERP_CALLBACK(cbArgButtonOk) {
	win->quit(1);
}

ERP_CALLBACK(cbArgButtonCancel) {
	win->quit(0);
}
