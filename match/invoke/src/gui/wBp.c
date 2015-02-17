/*
	$ Invoke   (C) 2005-2012 MF
	$ wBp.c, 0.1.1124
*/

#include "../base.h"
#include "../gui.h"

#define TITLE_INIT	"Invoke - 设定中断位置"
#define TITLE_ADD	"Invoke - 加载模块 \"%s\""

#define CLASSNAME	"INVOKE_BP"
#define WIDTH		750
#define HEIGHT		500


/* 跳过 */
static int SKIP = -1;


static void guiBpDraw(erp_window_s *win) {
	int		i;

	char	buffer[2][16];
	char	*data[3] = {buffer[0], buffer[1], NULL};

	int			count		= packVar(win->param, 1, int);
	module_s	**module	= packPtr(win->param, 0, module_s *);

	if(packVar(win->param, 2, int) == 1) {
		CHECK(cBpHij)->check(TRUE);
	}
	
	/* 输出模块 */
	for(i = 0; i < count; i++) {
		sprintf(data[0], "%08X", module[i]->base);
		sprintf(data[1], "%lu", module[i]->size);

		data[2] = module[i]->filename;

		LIST(lsBpModule)->append(data);
	}

	OBJECT(bBpDisasm)->disable(TRUE);
	OBJECT(bBpBatch)->disable(TRUE);
	
	if(count) {
		LIST(lsBpModule)->select(0);
	}
}

static void * guiBpDestroy(erp_window_s *win, DWORD arg) {
	if(packVar(win->param, 2, int) >= 0) {
		SKIP = CHECK(cBpSkip)->checked;

		return (void *) CHECK(cBpHij)->checked;
	}

	return NULL;
}

int guiBp(HWND win, char *title, module_s **module, unsigned int count, int check) {
	int		result;
	char	*text;

	char	*titleModule[]		= {"基地址", "大小", "模块路径"};
	char	*titleFunction[]	= {"函数名称", "导入序号", "入口地址", "断点地址"};
	size_t	widthModule[]		= {100, 100, 485};
	size_t	widthFunction[]		= {385, 100, 100, 100};

	packNew(arg, module, &count, &check);
	
	ZeroMemory(&wBp, sizeof wBp);

	/* 跳过 */
	if(SKIP == 1) {
		return 0;
	}

	/* 窗口标题 */
	if(title) {
		NEW(text, char, strlen(title) + strlen(TITLE_ADD) + 16);

		sprintf(text, TITLE_ADD, title);
	} else {
		text = TITLE_INIT;
	}

	wBp.classname	= CLASSNAME;
	wBp.title		= text;
	wBp.width		= WIDTH;
	wBp.height		= HEIGHT;
	wBp.instance	= RT.instance;
	wBp.draw		= guiBpDraw;
	wBp.destroy		= guiBpDestroy;
	wBp.center		= TRUE;
	wBp.param		= (DWORD) arg;
	wBp.parent		= win;

	lsBpModule		= DRAW_LIST		(wBp, BP_LIST_MODULE,	  15,  15, 715, 130, titleModule, widthModule,		sizeof titleModule / sizeof(char *),	cbBpListModule);
	lsBpFunction	= DRAW_LIST		(wBp, BP_LIST_FUNCTION,	  15, 160, 715, 250, titleFunction, widthFunction,	sizeof titleFunction / sizeof(char *),	cbBpListFunction);
	
	bBpDisasm		= DRAW_BUTTON	(wBp, BP_BUTTON_DISASM,	  15, -15, 100,  28, "查看反汇编",	cbBpButtonDisasm);
	bBpBatch		= DRAW_BUTTON	(wBp, BP_BUTTON_BATCH,	 125, -15, 100,  28, "批量设置",		cbBpButtonBatch);
	bBpOk			= DRAW_BUTTON	(wBp, BP_BUTTON_OK,		 -15, -15,  90,  28, "完成设置",		cbBpButtonOk);

	if(check >= 0) {
		cBpHij		= DRAW_CHECK	(wBp, BP_CHECK_HIJ,		-150, -18, 170,  20, "稍后打开劫持函数设置窗口", NULL);
		cBpSkip		= DRAW_CHECK	(wBp, BP_CHECK_SKIP,	-340, -18,  70,  20, "不再显示", NULL);
	}

	result = (int) erpWindow(&wBp);

	if(title) {
		DEL(text);
	}

	return result;
}


ERP_CALLBACK(cbBpListModule) {
	char		buffer[3][16];
	char		*data[4] = {NULL, buffer[0], buffer[1], buffer[2]};

	module_s	*module;
	function_s	*function;

	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	OBJECT(bBpDisasm)->disable(TRUE);
	OBJECT(bBpBatch)->disable(TRUE);

	LIST(lsBpFunction)->clear();

	if(arg == -1) {
		return;
	} else {
		module		= packPtr(win->param, 0, module_s *)[arg];
		function	= module->function;
	}

	while(function) {
		if(function->hook == FALSE) {
			if(function->name) {
				data[0] = function->name;
			} else {
				data[0] = "N/A";
			}

			sprintf(data[1], "%ld", function->ordinal);
			sprintf(data[2], "%08X", function->entry);
			sprintf(data[3], "%08X", function->bp);

			LIST(lsBpFunction)->append(data);
		}

		function = function->next;
	}

	OBJECT(bBpBatch)->disable(FALSE);
}

ERP_CALLBACK(cbBpListFunction) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	if(arg == -1) {
		OBJECT(bBpDisasm)->disable(TRUE);
	} else {
		OBJECT(bBpDisasm)->disable(FALSE);
	}
}

ERP_CALLBACK(cbBpButtonDisasm) {
	char		buffer[16];

	char		*name;
	char		*data[] = {NULL, NULL, NULL, buffer};

	DWORD		address;

	module_s	*module;
	function_s	*function;

	int i;
	int indexMod	= LIST(lsBpModule)->selected;
	int indexFunc	= LIST(lsBpFunction)->selected;

	if((indexMod < 0) || (indexFunc < 0)) {
		return;
	}
	if(indexMod >= (int) packVar(win->param, 1, unsigned int)) {
		return;
	}

	module		= packPtr(win->param, 0, module_s *)[indexMod];
	function	= module->function;

	for(i = 0; i < indexFunc; i++) {
		if(function->next == NULL) {
			return;
		}
		if(function->hook) {
			i--;
		}

		function = function->next;
	}

	if(function->name) {
		NEW(name, char, strlen(module->modname) + strlen(function->name) + 8);

		strcat(name, function->name);
		strcat(name, "@");
		strcat(name, module->modname);
	} else {
		NEW(name, char, strlen(module->modname) + 16);

		sprintf(name, "#%lu", function->ordinal);

		strcat(name, "@");
		strcat(name, module->modname);
	}

	address = function->bp;

	guiAsm(win->hwnd, name, function->entry, &address, TRUE, function->entry, function->entry + 10);

	DEL(name);

	/* 如果修改了断点地址 */
	if(address != function->bp) {
		function->bp = address;

		sprintf(buffer, "%08X", address);

		LIST(lsBpFunction)->modify(i, data);
	}
}

ERP_CALLBACK(cbBpButtonBatch) {
	/* 设置 */
	DWORD	offset;
	BOOL	moveStack, moveCall, moveJump, moveRet;

	module_s	*module;

	int index = LIST(lsBpModule)->selected;

	if(index < 0) {
		return;
	}
	if(index >= (int) packVar(win->param, 1, unsigned int)) {
		return;
	}
	
	/* 取消 */
	if(guiBtch(win->hwnd, &offset, &moveStack, &moveCall, &moveJump, &moveRet) == 0) {
		return;
	}

	module = packPtr(win->param, 0, module_s *)[index];

	moduleSet(module, offset, moveStack, moveCall, moveJump, moveRet);

	/* 更新列表 */
	cbBpListModule(win, (erp_object_s *) lsBpModule, SELECT, index);
}

ERP_CALLBACK(cbBpButtonOk) {
	win->quit(0);
}
