/*
	$ Invoke   (C) 2005-2012 MF
	$ wAsm.c, 0.1.1125
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME	"INVOKE_ASM"
#define TITLE		"反汇编"
#define WIDTH		500
#define HEIGHT		500

#define COUNT		200


/* 显示数量 */
static char	*countTitle[]	= {"20 条", "50 条", "100 条", "200 条"};
static int	countData[]		= {20, 50, 100, 200};
/* 当前选择 */
static int	countId			= 0;
static int	countMax		= sizeof countData / sizeof(int);

/* 指令地址 */
static DWORD *codeIndex		= NULL;


static void guiAsmDraw(erp_window_s *win) {
	int i;

	for(i = 0; i < countMax; i++) {
		COMBO(cmbAsmCount)->append(countTitle[i]);
	}

	EDIT(eAsmAddress)->setText("%08X", packVar(win->param, 0, DWORD));
	COMBO(cmbAsmCount)->select(countId);

	//BUTTON(bAsmGoto)->click();
}

static void * guiAsmDestroy(erp_window_s *win, DWORD arg) {
	DEL(codeIndex);

	return NULL;
}

static void guiAsmDisable(BOOL disable) {
	OBJECT(eAsmAddress)->disable(disable);
	OBJECT(cmbAsmCount)->disable(disable);

	OBJECT(lsAsmCode)->disable(disable);

	OBJECT(bAsmGoto)->disable(disable);
	OBJECT(bAsmOk)->disable(disable);
}


int guiAsm(HWND parent, char *title, DWORD address, DWORD *selected, BOOL selAllow, DWORD selMin, DWORD selMax) {
	char *text		= TITLE;
	char *button	= "确定";

	char	*titleCode[]	= {"地址", "数据", "指令"};
	size_t	widthCode[]		= {65, 105, 265};

	packNew(arg, &address, selected, &selAllow, &selMin, &selMax);

	ZeroMemory(&wAsm, sizeof wAsm);

	if(title) {
		text = title;
	}
	if(selAllow) {
		button = "选定";
	}

	/* 为指令索引分配内存 */
	NEW(codeIndex, DWORD, COUNT);

	wAsm.classname	= CLASSNAME;
	wAsm.title		= text;
	wAsm.width		= WIDTH;
	wAsm.height		= HEIGHT;
	wAsm.instance	= RT.instance;
	wAsm.draw		= guiAsmDraw;
	wAsm.destroy	= guiAsmDestroy;
	wAsm.tool		= TRUE;
	wAsm.center		= TRUE;
	wAsm.parent		= parent;
	wAsm.param		= (DWORD) arg;

	eAsmAddress		= DRAW_EDIT		(wAsm, ASM_EDIT_ADDRESS,	 15,  15,  90,  22, NULL);
	cmbAsmCount		= DRAW_COMBO	(wAsm, ASM_COMBO_COUNT,		-15,  15,  80,  22, cbAsmComboCount);

	lsAsmCode		= DRAW_LIST		(wAsm, ASM_LIST_CODE,		 15,  50, 465, 360, titleCode, widthCode, sizeof titleCode / sizeof(char *), cbAsmListCode);

	bAsmGoto		= DRAW_BUTTON	(wAsm, ASM_BUTTON_GOTO,		110,  15,  50,  22, "查看",	cbAsmButtonGoto);
	bAsmOk			= DRAW_BUTTON	(wAsm, ASM_BUTTON_OK,		-15, -15, 100,  28, button,	cbAsmButtonOk);

	erpWindow(&wAsm);

	return 0;
}


ERP_CALLBACK(cbAsmComboCount) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	if((int) arg > countMax) {
		return;
	}

	countId = arg;

	BUTTON(bAsmGoto)->click();
}

ERP_CALLBACK(cbAsmListCode) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	OBJECT(bAsmOk)->disable(FALSE);

	if(arg == -1) {
		if(packVar(win->param, 2, BOOL)) {
			OBJECT(bAsmOk)->disable(TRUE);
		}

		return;
	}
}

ERP_CALLBACK(cbAsmButtonGoto) {
	int		i, j, size;

	char	buffer[2][64], output[128];
	char	*data[]	= {buffer[0], buffer[1], output};

	int		count	= countData[countId];
	char	*input	= EDIT(eAsmAddress)->getText();

	DWORD	address;

	i = sscanf(input, "%X", &address);

	DEL(input);

	/* 输入了无效的地址 */
	if(i == 0) {
		warning(win->hwnd, "输入了无效的地址。");
		return;
	}

	ZeroMemory(codeIndex, sizeof(DWORD) * COUNT);

	LIST(lsAsmCode)->clear();

	guiAsmDisable(TRUE);

	for(i = 0; i < count; i++) {
		/* 读取失败 */
		if(processRead(address, buffer[0], 64) == 0) {
			break;
		}

		/* 保存索引 */
		codeIndex[i] = address;

		/* 反汇编的字节数 */
		if((size = disasm(address, buffer[0], 64, output, 128)) == 0) {
			break;
		}

		for(j = 0; j < 128; j++) {
			if(output[j] == 0) {
				break;
			}
			if(output[j] == '\t') {
				output[j] = ' ';
			}
		}

		/* 输出 */
		for(j = 0; j < size; j++) {
			sprintf(buffer[1] + j * 3, "%02X ", (unsigned char) buffer[0][j]);
		}

		sprintf(buffer[0], "%08X", address);

		LIST(lsAsmCode)->append(data);

		/* 地址顺延 */
		address += size;
	}

	guiAsmDisable(FALSE);

	if(packVar(win->param, 2, BOOL)) {
		OBJECT(bAsmOk)->disable(TRUE);
	}
}

ERP_CALLBACK(cbAsmButtonOk) {
	int		index;

	DWORD	address;
	DWORD	*selected	= packPtr(win->param, 1, DWORD);
	DWORD	selMin		= packVar(win->param, 3, DWORD);
	DWORD	selMax		= packVar(win->param, 4, DWORD);

	if(packVar(win->param, 2, BOOL) == FALSE) {
		win->quit(0); return;
	}

	index = LIST(lsAsmCode)->selected;

	/* 无效的索引 */
	if((index < 0) || (codeIndex[index] == 0)) {
		return;
	} else {
		address = codeIndex[index];
	}

	/* 范围 */
	if((selMin && (address < selMin)) || (selMax && (address > selMax))) {
		if(!confirmNoFormat(win->hwnd, "不推荐中断于选择的地址 %08X，是否继续？", address)) {
			win->cancel = 1; return;
		}
	}

	*selected = address;

	win->quit(0);
}
