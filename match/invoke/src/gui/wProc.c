/*
	$ Invoke   (C) 2005-2012 MF
	$ wProc.c, 0.1.1124
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME	"INVOKE_PROC"
#define TITLE		"Invoke - 选择需要调试的进程"
#define WIDTH		600
#define HEIGHT		350


static void guiProcDraw(erp_window_s *win) {
	BUTTON(bProcRefresh)->click();
}

static void * guiProcDestroy(erp_window_s *win, DWORD arg) {
	if(arg == 0) {
		exit(EXIT_SUCCESS);
	}

	processFree();

	return 0;
}

void guiProc() {
	/* 进程列表 */
	char	*title[]	= {"PID", "文件"};
	size_t	width[]		= {70, 460};

	ZeroMemory(&wProc, sizeof wProc);

	wProc.classname	= CLASSNAME;
	wProc.title		= TITLE;
	wProc.width		= WIDTH;
	wProc.height	= HEIGHT;
	wProc.instance	= RT.instance;
	wProc.draw		= guiProcDraw;
	wProc.destroy	= guiProcDestroy;
	wProc.center	= TRUE;

	lbProcTip		= DRAW_LABEL	(wProc, PROC_LABEL_TIP,			 20,  18, 300,  20, "请在下方选择一个用于调试的进程，然后点击确定按钮。");
	lsProcProcess	= DRAW_LIST		(wProc, PROC_LIST_PROCESS,		 20,  50, 558, 210, title, width, sizeof title / sizeof(char *), cbProcListProcess);

	bProcRefresh	= DRAW_BUTTON	(wProc, PROC_BUTTON_REFRESH,	-15,  11, 100,  28, "刷新进程",	cbProcButtonRefresh);
	bProcFile		= DRAW_BUTTON	(wProc, PROC_BUTTON_FILE,		 20, -15, 120,  28, "运行新进程",	cbProcButtonFile);
	bProcStart		= DRAW_BUTTON	(wProc, PROC_BUTTON_START,		-15, -15, 100,  28, "确定",		cbProcButtonStart);

	erpWindow(&wProc);
}

void guiProcDisable(BOOL disable) {
	OBJECT(lsProcProcess)->disable(disable);

	OBJECT(bProcRefresh)->disable(disable);
	OBJECT(bProcFile)->disable(disable);

	if((disable == FALSE) && (LIST(lsProcProcess)->selected >= 0)) {
		OBJECT(bProcStart)->disable(FALSE);
	} else {
		OBJECT(bProcStart)->disable(TRUE);
	}
}


ERP_CALLBACK(cbProcButtonRefresh) {
	int		i, count;
	/* 进程信息 */
	DWORD	*pid;
	char	**name;
	/* 显示信息 */
	char	buffer[16];
	char	*data[] = {buffer, NULL};

	guiProcDisable(TRUE);

	/* 获取进程列表 */
	if((count = processList(&pid, &name)) == 0) {
		win->quit(0);
	}

	/* 刷新列表 */
	LIST(lsProcProcess)->clear();

	for(i = 0; i < count; i++) {
		sprintf(buffer, "%lu", pid[i]);

		if(name[i]) {
			data[1] = name[i];
		} else {
			data[1] = "-";
		}

		LIST(lsProcProcess)->append(data);
	}

	guiProcDisable(FALSE);
}

ERP_CALLBACK(cbProcButtonFile) {
	char *error;

	guiProcDisable(TRUE);

	if(error = (char *) symbolRun(-1)) {
		if(error[0]) {
			warning(win->hwnd, error);
		}
	} else {
		win->quit(1); return;
	}

	guiProcDisable(FALSE);
}

ERP_CALLBACK(cbProcButtonStart) {
	int		index;
	char	*error;

	if((index = LIST(lsProcProcess)->selected) < 0) {
		return;
	}

	guiProcDisable(TRUE);

	if(error = (char *) symbolRun(index)) {
		warning(win->hwnd, error);

		guiProcDisable(FALSE);
	} else {
		win->quit(1);
	}
}

ERP_CALLBACK(cbProcListProcess) {
	switch(evt) {
		case SELECT:	break;
		default:		return;
	}

	if(arg == -1) {
		OBJECT(bProcStart)->disable(TRUE);
	} else {
		OBJECT(bProcStart)->disable(FALSE);
	}
}
