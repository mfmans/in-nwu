/*
	$ Invoke   (C) 2005-2012 MF
	$ wMain.c, 0.1.1201
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_MAIN"
#define TITLE			"Invoke"
#define WIDTH			900
#define HEIGHT			380


/* 寄存器 */
static CONTEXT reg;

/* 参数对应的地址 */
static size_t	argCount	= 0;
static DWORD	*argAddr	= NULL;


static void registerRead() {
	ZeroMemory(&reg, sizeof reg);
	reg.ContextFlags = CONTEXT_FULL;

	GetThreadContext(RT.thread, &reg);
}

static void memoryGet(char *buffer, DWORD addr, size_t size) {
	size_t i;
	unsigned char data[16];

	if(processRead(addr, data, size) == 0) {
		*buffer = 0; return;
	}

	for(i = 0; i < size; i++) {
		sprintf(buffer + i * 3, "%02X ", data[i]);
	}
}


static void guiMainDraw(erp_window_s *win) {
	guiMainTitle(NULL, NULL, 0);
	guiMainStatus(FALSE, TRUE, TRUE);

	OBJECT(bMainHijack)->disable(FALSE);

	EDIT(eMainModule)->readonly(TRUE);
	EDIT(eMainOrdinal)->readonly(TRUE);
	EDIT(eMainFuncname)->readonly(TRUE);
	EDIT(eMainBp)->readonly(TRUE);

	if(RT.entry) {
		OBJECT(bMainSuspend)->caption("启动进程");
	}
}

static void * guiMainDestroy(erp_window_s *win, DWORD arg) {
	/* 恢复进程和调试线程 */
	processResume(RT.handle);
	ResumeThread(RT.debug);

	return NULL;
}

void guiMain() {
	char	*title[]	= {"地址", "内存数据", "名称", "类型", "类型值"};
	size_t	width[]		= {70, 130, 120, 160, 370};

	ZeroMemory(&wMain, sizeof wMain);

	wMain.classname	= CLASSNAME;
	wMain.title		= TITLE;
	wMain.width		= WIDTH;
	wMain.height	= HEIGHT;
	wMain.instance	= RT.instance;
	wMain.draw		= guiMainDraw;
	wMain.destroy	= guiMainDestroy;
	wMain.center	= TRUE;

	bMainSuspend	= DRAW_BUTTON	(wMain, MAIN_BUTTON_SUSPEND,	   5,   5,  90,  28, "恢复进程",		cbMainButtonSuspend);
	bMainDump		= DRAW_BUTTON	(wMain, MAIN_BUTTON_DUMP,		 100,   5,  90,  28, "导出镜像",		cbMainButtonDump);
	bMainHijack		= DRAW_BUTTON	(wMain, MAIN_BUTTON_HIJACK,		  -5,   5, 120,  28, "管理劫持函数",	cbMainButtonHijack);

	bMainRegister	= DRAW_BUTTON	(wMain, MAIN_BUTTON_REGISTER,	 280,   5,  80,  28, "寄存器",		cbMainButtonRegister);
	bMainDisasm		= DRAW_BUTTON	(wMain, MAIN_BUTTON_DISASM,		 365,   5,  80,  28, "反汇编",		cbMainButtonDisasm);
	bMainMemory		= DRAW_BUTTON	(wMain, MAIN_BUTTON_MEMORY,		 450,   5,  80,  28, "内存",			cbMainButtonMemory);
	bMainViewer		= DRAW_BUTTON	(wMain, MAIN_BUTTON_VIEWER,		 535,   5,  90,  28, "查看参数",		cbMainButtonViewer);

	lsMainStack		= DRAW_LIST		(wMain, MAIN_LIST_STACK,		   7,  40, 880, 200, title, width, sizeof title / sizeof(char *), cbMainListStack);

	lbMainModule	= DRAW_LABEL	(wMain, MAIN_LABEL_MODULE,		  10, 260,  80,  20, "模块：");
	lbMainModule	= DRAW_LABEL	(wMain, MAIN_LABEL_FUNCTION,	  10, 285,  80,  20, "函数：");
	lbMainBp		= DRAW_LABEL	(wMain, MAIN_LABEL_BP,			 470, 258, 100,  20, "断点地址：");
	lbMainEsp		= DRAW_LABEL	(wMain, MAIN_LABEL_ESP,			 570, 258, 100,  20, "ESP 栈顶：");
	lbMainRet		= DRAW_LABEL	(wMain, MAIN_LABEL_RET,			 670, 258, 100,  20, "返回地址：");
	lbMainEax		= DRAW_LABEL	(wMain, MAIN_LABEL_EAX,			 780, 258, 120,  20, "EAX 返回内容：");

	eMainModule		= DRAW_EDIT		(wMain, MAIN_EDIT_MODULE,		  60, 255, 350,  22, NULL);
	eMainOrdinal	= DRAW_EDIT		(wMain, MAIN_EDIT_ORDINAL,		  60, 280,  80,  22, NULL);
	eMainFuncname	= DRAW_EDIT		(wMain, MAIN_EDIT_FUNCNAME,		 145, 280, 265,  22, NULL);
	eMainBp			= DRAW_EDIT		(wMain, MAIN_EDIT_BP,			 470, 275,  90,  22, NULL);
	eMainEsp		= DRAW_EDIT		(wMain, MAIN_EDIT_ESP,			 570, 275,  90,  22, NULL);
	eMainRet		= DRAW_EDIT		(wMain, MAIN_EDIT_RET,			 670, 275,  90,  22, NULL);
	eMainEax		= DRAW_EDIT		(wMain, MAIN_EDIT_EAX,			 780, 275, 105,  22, NULL);

	bMainContinue	= DRAW_BUTTON	(wMain, MAIN_BUTTON_CONTINUE,	   5,  -5, 100,  28, "继续深入",		cbMainButtonContinue);
	bMainRuntoret	= DRAW_BUTTON	(wMain, MAIN_BUTTON_RUNTORET,	 110,  -5, 110,  28, "执行到返回",	cbMainButtonRuntoret);
	bMainRuntocon	= DRAW_BUTTON	(wMain, MAIN_BUTTON_RUNTOCON,	 225,  -5, 110,  28, "返回后继续",	cbMainButtonRuntocon);
	bMainReturn		= DRAW_BUTTON	(wMain, MAIN_BUTTON_RETURN,		 340,  -5, 100,  28, "直接返回",		cbMainButtonReturn);

	bMainCalc		= DRAW_BUTTON	(wMain, MAIN_BUTTON_CALC,		  -5,  -5,  90,  28, "刷新堆栈",		cbMainButtonCalc);
	bMainFunction	= DRAW_BUTTON	(wMain, MAIN_BUTTON_FUNCTION,	-100,  -5,  90,  28, "函数原型",		cbMainButtonFunction);

	bMainAbout		= DRAW_BUTTON	(wMain, MAIN_BUTTON_ABOUT,		-240,  -5,  80,  28, "关于",			cbMainButtonAbout);

	erpWindow(&wMain);
}

void guiMainTitle(char *module, char *function, size_t ordinal) {
	/* 固定标题 */
	static char *fixed = NULL;

	if(fixed == NULL) {
		if(uacAdminCheck()) {
			fixed = TITLE " [管理员]";
		} else {
			fixed = TITLE;
		}
	}

	if(module) {
		if(function) {
			wMain.caption("%s@%s - %s", function, module, fixed);
		} else {
			wMain.caption("#%ld@%s - %s", ordinal, module, fixed);
		}
	} else {
		wMain.caption(fixed);
	}
}

void guiMainStatus(BOOL suspend, BOOL proc, BOOL run) {
	OBJECT(bMainSuspend)->disable(suspend);

	OBJECT(bMainDump)->disable(proc);
	OBJECT(bMainHijack)->disable(proc);
	OBJECT(bMainRegister)->disable(proc);
	OBJECT(bMainDisasm)->disable(proc);
	OBJECT(bMainMemory)->disable(proc);

	OBJECT(bMainContinue)->disable(run);
	OBJECT(bMainRuntoret)->disable(run);
	OBJECT(bMainRuntocon)->disable(run);
	OBJECT(bMainReturn)->disable(run);
	OBJECT(bMainCalc)->disable(run);
	OBJECT(bMainFunction)->disable(run);

	OBJECT(eMainEsp)->disable(run);
	OBJECT(eMainRet)->disable(run);
	OBJECT(eMainEax)->disable(run);

	if((run == FALSE) && (LIST(lsMainStack)->selected >= 0)) {
		OBJECT(bMainViewer)->disable(FALSE);
	} else {
		OBJECT(bMainViewer)->disable(TRUE);
	}
}


ERP_CALLBACK(cbMainListStack) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	if((arg == -1) || OBJECT(bMainHijack)->disabled) {
		OBJECT(bMainViewer)->disable(TRUE);
	} else {
		OBJECT(bMainViewer)->disable(FALSE);
	}
}

ERP_CALLBACK(cbMainButtonSuspend) {
	guiMainStatus(TRUE, TRUE, TRUE);
	runSuspendResume();
}

ERP_CALLBACK(cbMainButtonDump) {
	moduleDump();
}

ERP_CALLBACK(cbMainButtonRegister) {
	registerRead();
	guiReg(win->hwnd, &reg);
}

ERP_CALLBACK(cbMainButtonDisasm) {
	registerRead();

	/* 显示反汇编窗口 */
	guiAsm(win->hwnd, "反汇编当前 EIP", reg.Eip, NULL, FALSE, 0, 0);
}

ERP_CALLBACK(cbMainButtonMemory) {
	DWORD	addr;
	int		index = LIST(lsMainStack)->selected;

	if((index < 0) || (index >= (int) argCount)) {
		registerRead();

		/* 堆栈数据 */
		addr = reg.Esp;
	} else {
		addr = argAddr[index];
	}

	guiMem(win->hwnd, addr);
}

ERP_CALLBACK(cbMainButtonHijack) {
	guiHij(win->hwnd);
}

ERP_CALLBACK(cbMainButtonCalc) {
	int		i = 0, enter = -1;
	DWORD	addr, esp, ret;

	char	buffer[2][128];
	char	*data[] = {buffer[0], buffer[1], NULL, NULL, NULL};
	char	*input;

	function_s	*func = RT.function;
	argument_s	*argv;
	type_s		type;

	/* 可变参类型 */
	typeSet(&type, US_INT_32, 0, 0);

	/* 读取设置的 ESP */
	input = EDIT(eMainEsp)->getText();

	sscanf(input, "%x", &enter);
	DEL(input);

	if(enter < 0) {
		warning(win->hwnd, "请输入正确的 ESP 栈顶地址。"); return;
	} else {
		esp = (DWORD) enter;
	}

	/* 计算返回地址 */
	if(func->hijack == NULL) {
		warning(win->hwnd, "函数原型已被删除。"); return;
	}

	if(func->hijack->esp) {
		addr = esp + 4;
	} else {
		addr = esp;
	}

	/* 读取并显示返回地址 */
	processRead(addr, &ret, sizeof ret);
	EDIT(eMainRet)->setText("%08X", ret);

	/* ESP 返回地址修正 */
	esp = addr + 4;

	LIST(lsMainStack)->clear();

	argCount = 0;
	DEL(argAddr);

	if(argv = func->hijack->argList) {
		/* 如果是可变参数 */
		if(func->hijack->vararg) {
			argCount = func->hijack->argCount + 4;
		} else {
			argCount = func->hijack->argCount;
		}

		NEW(argAddr, DWORD, argCount);

		while(argv) {
			sprintf(data[0], "%08X", esp);
			memoryGet(data[1], esp, argv->type.size);

			data[2] = argv->name;
			data[3] = type2string(&argv->type);
			data[4] = typeReadSimple(esp, &argv->type);

			LIST(lsMainStack)->append(data);

			DEL(data[3]);
			DEL(data[4]);

			/* 保存 ESP */
			argAddr[i] = esp;

			/* ESP 修正 */
			esp += argv->type.size;

			argv = argv->next;

			i++;
		}
	} else {
		argCount = 4;

		NEW(argAddr, DWORD, argCount);
	}

	/* 可变参表 */
	for(; i < (int) argCount; i++) {
		sprintf(data[0], "%08X", esp);
		memoryGet(data[1], esp, type.size);

		data[2] = "变参/堆栈";
		data[3] = type2string(&type);
		data[4] = typeReadSimple(esp, &type);

		LIST(lsMainStack)->append(data);

		DEL(data[3]);
		DEL(data[4]);

		/* 保存 ESP */
		argAddr[i] = esp;

		/* ESP 修正 */
		esp += type.size;
	}
}

ERP_CALLBACK(cbMainButtonFunction) {
	if(RT.function && RT.function->hijack) {
		guiPrt(RT.function->hijack, win->hwnd);

		BUTTON(bMainCalc)->click();
	}
}

ERP_CALLBACK(cbMainButtonViewer) {
	int		i = 0;
	int		index = LIST(lsMainStack)->selected;

	type_s		type, *stype = &type;
	argument_s	*argv;

	/* 可变参类型 */
	typeSet(&type, US_INT_32, 0, 0);

	if((index < 0) || (index >= (int) argCount)) {
		return;
	}

	/* 判断是不是参数 */
	if(RT.function && RT.function->hijack) {
		argv = RT.function->hijack->argList;

		while(argv) {
			if(i == index) {
				stype = &argv->type; break;
			}

			i++;
			argv = argv->next;
		}
	}

	guiView(win->hwnd, argAddr[index], stype);
}

ERP_CALLBACK(cbMainButtonContinue) {
	symbolRunAsyn(0);
}

ERP_CALLBACK(cbMainButtonRuntoret) {
	symbolRunAsyn(1);
}

ERP_CALLBACK(cbMainButtonRuntocon) {
	symbolRunAsyn(2);
}

ERP_CALLBACK(cbMainButtonReturn) {
	symbolRunAsyn(3);
}

ERP_CALLBACK(cbMainButtonAbout) {
	notice(win->hwnd, "Invoke   (C) 2005-2012 MF\n\n当前应用程序版本为 1.0.1204。\n此为西北大学第十二届电脑文化艺术节参赛版本。");
}
