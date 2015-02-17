/*
	$ Invoke   (C) 2005-2012 MF
	$ wFunc.c, 0.1.1129
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_FUNCTION"
#define TITLE			"添加函数"
#define WIDTH			580
#define HEIGHT			450


/* 查找结果数据 */
typedef struct func_result_s {
	function_s	*function;
	struct func_result_s *next;
} func_result_s;


/* 查找结果、待添加结果 */
static int				SEL = -1;

static func_result_s	*RS = NULL;
static hijack_s			*LS = NULL;


static void RSClear() {
	func_result_s *curr;

	/* 清理 RS */
	while(RS->next) {
		curr = RS->next->next;

		DEL(RS->next);

		RS->next = curr;
	}
}

static void RSFlush() {
	char buffer[16];
	char *data[] = {NULL, NULL, buffer, NULL};

	func_result_s	*curr = RS->next;

	module_s		*module;
	function_s		*function;

	LIST(lsFuncResult)->clear();

	SEL = -1;

	while(curr) {
		function	= curr->function;
		module		= function->module;

		data[0]		= module->modname;

		if((data[1] = function->name) == NULL) {
			data[1] = "N/A";
		}

		sprintf(buffer, "%lu", function->ordinal);

		if(function->hijack) {
			data[3] = "是";
		} else {
			data[3] = "";
		}

		LIST(lsFuncResult)->append(data);

		curr = curr->next;
	}
}


static void guiFuncDraw(erp_window_s *win) {
	module_s *module = MOD->next;

	SEL	= -1;

	if(RS == NULL) {
		NEW(RS, func_result_s, 1);
	}
	if(LS == NULL) {
		NEW(LS, hijack_s, 1);
	}

	RS->next = NULL;
	LS->next = NULL;

	COMBO(cmbFuncModule)->append("在所有模块中查找");

	while(module) {
		COMBO(cmbFuncModule)->append(module->filename);

		module = module->next;
	}

	OBJECT(bFuncAdd)->disable(TRUE);
	OBJECT(bFuncRemove)->disable(TRUE);
}

static void * guiFuncDestroy(erp_window_s *win, DWORD arg) {
	int cancel = 0;
	int result = 0;

	hijack_s *hijack;
	
	if(LS->next) {
		hijack = HIJ;

		while(hijack->next) {
			hijack = hijack->next;
		}

		hijack->next	= LS->next;
		LS->next->prev	= hijack;

		LS->next = NULL;

		result = 1;
	}
	
	RSClear();

	return (void *) result;
}

int guiFunc(HWND win) {
	char	*titleResult[]	= {"所在模块", "函数名称", "导入序号", "已添加"};
	size_t	widthResult[]	= {130, 240, 80, 60};
	char	*titleConfirm[]	= {"所在模块", "函数名称", "导入序号"};
	size_t	widthConfirm[]	= {130, 280, 100};

	ZeroMemory(&wFunc, sizeof wFunc);

	wFunc.classname	= CLASSNAME;
	wFunc.title		= TITLE;
	wFunc.width		= WIDTH;
	wFunc.height	= HEIGHT;
	wFunc.instance	= RT.instance;
	wFunc.draw		= guiFuncDraw;
	wFunc.destroy	= guiFuncDestroy;
	wFunc.center	= TRUE;
	wFunc.tool		= TRUE;
	wFunc.parent	= win;

	eFuncName		= DRAW_EDIT		(wFunc, FUNC_EDIT_NAME,		  15,  15, 185,  22, NULL);
	cmbFuncModule	= DRAW_COMBO	(wFunc, FUNC_COMBO_MODULE,	  15,  42, 350,  20, NULL);

	cFuncVague		= DRAW_CHECK	(wFunc, FUNC_CHECK_VAGUE,	 210,  16,  70,  20, "模糊查找",		NULL);
	cFuncSencase	= DRAW_CHECK	(wFunc, FUNC_CHECK_SENCASE,	 285,  16,  80,  20, "区分大小写",	NULL);

	bFuncSearch		= DRAW_BUTTON	(wFunc, FUNC_BUTTON_SEARCH,	 380,  15,  85,  48, "查找", cbFuncButtonSearch);
	bFuncAdd		= DRAW_BUTTON	(wFunc, FUNC_BUTTON_ADD,	 -15,  15,  85,  48, "添加", cbFuncButtonAdd);

	lsFuncResult	= DRAW_LIST		(wFunc, FUNC_LIST_RESULT,	  15,  75, 542, 100, titleResult, widthResult, sizeof titleResult / sizeof(char *), cbFuncListResult);
	lsFuncConfirm	= DRAW_LIST		(wFunc, FUNC_LIST_CONFIRM,	  15, 185, 542, 180, titleConfirm, widthConfirm, sizeof titleConfirm / sizeof(char *), cbFuncListConfirm);

	bFuncRemove		= DRAW_BUTTON	(wFunc, FUNC_BUTTON_REMOVE,	  15, -15, 180,  28, "从添加列表中删除",	cbFuncButtonRemove);

	return (int) erpWindow(&wFunc);
}


ERP_CALLBACK(cbFuncButtonSearch) {
	int i, j;
	int ordinal = -1;

	char	*keyword	= EDIT(eFuncName)->getText();

	int		index		= COMBO(cmbFuncModule)->selected - 1;
	int		vague		= CHECK(cFuncVague)->checked;
	int		sencase		= CHECK(cFuncSencase)->checked;

	module_s *mod		= MOD->next;
	function_s *func	= NULL;

	func_result_s *res, *curr;

	if((keyword == NULL) || (*keyword == 0)) {
		warning(win->hwnd, "请输入需要搜索的函数关键词！");
		
		DEL(keyword); return;
	}

	sscanf(keyword, "%ld", &ordinal);

	NEW(res, func_result_s, 1);

	i = 0;
	curr = res;

	while(mod) {
		if((index < 0) || (index == i)) {
			func = mod->function;

			while(func) {
				do {
					j = 0;

					if(func->name) {
						if(match(func->name, keyword, vague, sencase)) {
							j = 1; break;
						}
					}
					if(ordinal >= 0) {
						if(func->ordinal == ordinal) {
							j = 1; break;
						}
					}
				} while(0);

				if(j) {
					NEW(curr->next, func_result_s, 1);

					curr->next->function = func;
					curr = curr->next;
				}

				func = func->next;
			}
		}

		i++;
		mod = mod->next;
	}

	/* 找到结果 */
	if(res->next) {
		RSClear();

		RS->next = res->next;

		RSFlush();
	} else {
		warning(win->hwnd, "找不到相关的函数。");
	}

	DEL(keyword);
	DEL(res);
}

ERP_CALLBACK(cbFuncListResult) {
	unsigned int	i		= 0;
	func_result_s	*next	= RS->next;

	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	SEL = -1;

	OBJECT(bFuncAdd)->disable(TRUE);

	if(arg == -1) {
		return;
	}

	while(next) {
		if(i == arg) {
			break;
		}

		i++;
		next = next->next;
	}

	if(next->function->hijack == NULL) {
		SEL = arg;

		OBJECT(bFuncAdd)->disable(FALSE);
	}
}

ERP_CALLBACK(cbFuncListConfirm) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	if(arg == -1) {
		OBJECT(bFuncRemove)->disable(TRUE);
	} else {
		OBJECT(bFuncRemove)->disable(FALSE);
	}
}

ERP_CALLBACK(cbFuncButtonAdd) {
	int		i = 0;

	char	buffer[16];
	char	*data[] = {NULL, NULL, NULL, "是"};

	func_result_s	*res = RS->next;
	hijack_s		*hij = LS;

	if(SEL == -1) {
		return;
	}

	while(hij->next) {
		hij = hij->next;
	}

	NEW(hij->next, hijack_s, 1);

	hij->next->prev = hij;
	hij = hij->next;

	while(res) {
		if(i == SEL) {
			break;
		}

		i++;
		res = res->next;
	}

	res->function->hijack = hij;

	hij->function	= res->function;
	hij->invoke		= CL_CDECL;

	/* 默认返回类型 */
	typeSet(&hij->ret, US_INT_32, 0, 0);

	/* 更新表中元素 */
	LIST(lsFuncResult)->modify(SEL, data);

	data[0] = res->function->module->modname;
	data[2] = buffer;
	
	if((data[1] = res->function->name) == NULL) {
		data[1] = "N/A";
	}

	sprintf(buffer, "%lu", res->function->ordinal);

	/* 更新下方列表 */
	LIST(lsFuncConfirm)->append(data);

	OBJECT(bFuncAdd)->disable(TRUE);
}

ERP_CALLBACK(cbFuncButtonRemove) {
	int		i = 0;
	char	*data[] = {NULL, NULL, NULL, ""};

	func_result_s	*res = RS->next;
	hijack_s		*hij = LS->next, *curr;

	int index = LIST(lsFuncConfirm)->selected;

	if(index == -1) {
		return;
	}

	/* 寻找节点 */
	while(hij) {
		if(i == index) {
			break;
		}

		i++;
		hij = hij->next;
	}

	/* 连接节点 */
	curr = hij;
	curr->prev->next = curr->next;

	if(curr->next) {
		curr->next->prev = curr->prev;
	}

	i = 0;

	while(res) {
		if(res->function->hijack == curr) {
			LIST(lsFuncResult)->modify(i, data);

			if(i == LIST(lsFuncResult)->selected) {
				OBJECT(bFuncAdd)->disable(FALSE);
				SEL = i;
			}

			break;
		}

		i++;
		res = res->next;
	}

	curr->function->hijack = NULL;

	LIST(lsFuncConfirm)->remove(index);

	DEL(curr);
}
