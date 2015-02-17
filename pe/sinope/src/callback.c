/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ callback.c, 0.1.1104
*/

#include <process.h>
#include <Windows.h>
#include "base.h"
#include "image.h"
#include "gui.h"
#include "action.h"


ERP_CALLBACK(cbButtonExecute) {
	if(RUNTIME.file == NULL) {
		if((RUNTIME.file = selectFileOpen("exe", "打开 PE 文件")) == NULL) {
			return;
		}
	}

	/* 禁用按钮 */
	OBJECT(obj)->disable(TRUE);

	/* 如果正在运行中 */
	if(RUNTIME.alive == TRUE) {
		RUNTIME.alive = FALSE; return;
	}

	/* 分析文件并启动进程 */
	if(pe() == TRUE) {
		thRun(Process, NULL); return;
	}

	/* 首次打开文件并分析失败 */
	if(RUNTIME.filename == NULL) {
		mem_delete(RUNTIME.file);
	}

	/* 恢复按钮 */
	OBJECT(obj)->disable(FALSE);
}

ERP_CALLBACK(cbButtonDump) {
	if(guiDump() == TRUE) {
		dump();
	}
}

ERP_CALLBACK(cbButtonSuspend) {
	OBJECT(obj)->disable(TRUE);

	WORKING_SET("暂停进程。");

	RUNTIME.running = FALSE;
}

ERP_CALLBACK(cbButtonContinue) {
	OBJECT(obj)->disable(TRUE);
	OBJECT(bSingleStep)->disable(TRUE);

	RUNTIME.singleStep	= FALSE;
	RUNTIME.running		= TRUE;
}

ERP_CALLBACK(cbButtonSingleStep) {
	OBJECT(obj)->disable(TRUE);
	OBJECT(bContinue)->disable(TRUE);

	RUNTIME.singleStep	= TRUE;
	RUNTIME.running		= TRUE;
}

ERP_CALLBACK(cbButtonRefresh) {
	if(confirm("是否根据内存中的镜像信息强制刷新区段表、导入表和模块表？强制刷新后，区段表中原备份镜像将会被丢弃并创建新备份，而导入表也可能得不到正确的结果。是否继续？") == FALSE) {
		return;
	}

	procClear();

	/* 操作指针 */
	pfRead		= dataReadMemory;
	pfSeek		= dataSeekMemory;
	pfTell		= dataTellMemory;
	pfOffset	= dataOffsetMemory;
	pfEnd		= dataEndMemory;

	peHeader(TRUE);
	peSection();

	procMapModule();
	procMapSection();

	peImport();
	procImport();

	sectionFlush();
	importFlush();
	moduleFlush();
}

ERP_CALLBACK(cbButtonRegister) {
	CONTEXT reg;

	reg.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(RUNTIME.proc.tBreak->handle, &reg);

	guiRegister(&reg);
}

ERP_CALLBACK(cbButtonConfig) {
	guiConfig();

	/* 刷新列表, 重新计算 RVA */
	bpFlush();
	sectionFlush();
	importFlush();
	moduleFlush();

	/* 反汇编 */
	loopDisasm(TRUE);
}

ERP_CALLBACK(cbButtonAbout) {
	guiAbout();
}


ERP_CALLBACK(cbComboThread) {
	thread_s *thread;

	if(evt != SELECT) {
		return;
	}

	if(arg == 0) {
		/* 所有线程 */
		RUNTIME.proc.tDebug = NULL;
	} else {
		thread = THREAD->next;

		while(thread) {
			if(thread->index == arg) {
				RUNTIME.proc.tDebug = thread;

				return;
			}

			thread = thread->next;
		}

		/* 选择无效 */
		COMBO(obj)->select(0);
	}
}


ERP_CALLBACK(cbListBp) {
	BOOL disable;

	if((RUNTIME.alive == TRUE) && (RUNTIME.running == FALSE)) {
		switch(evt) {
			case FOCUS:
				if(LIST(obj)->selected >= 0) {
					disable = FALSE;
				} else {
					disable = TRUE;
				}
				break;

			case SELECT:
				if(((int) arg) < 0) {
					disable = TRUE;
				} else {
					disable = FALSE;
				}
				break;

			default:
				return;
		}
	} else {
		disable = TRUE;
	}

	OBJECT(bBpEdit)->disable(disable);
	OBJECT(bBpDelete)->disable(disable);
}

ERP_CALLBACK(cbButtonBpAdd) {
	DWORD	from	= 0;
	DWORD	to		= 0;
	BOOL	rva		= TRUE;

	if(guiBp(&from, &to, &rva, &bpCallback) == TRUE) {
		bpAdd(from, to, rva);
	}
}

ERP_CALLBACK(cbButtonBpEdit) {
	DWORD	from, to;
	BOOL	rva;

	int		index = LIST(lsBp)->selected;
	bp_s	*bp = BP->next;

	if(index >= 0) {
		while(bp) {
			if(bp->index == index) {
				break;
			}

			bp = bp->next;
		}
	} else {
		return;
	}

	if((bp == NULL) || (bp->index != index)) {
		return;
	}

	if(bp->rva) {
		from	= va2rva(bp->vaFrom);
		to		= va2rva(bp->vaTo);

		rva		= TRUE;
	} else {
		from	= bp->addrFrom;
		to		= bp->addrTo;

		rva		= FALSE;
	}

	if(guiBp(&from, &to, &rva, &bpCallback) == TRUE) {
		bpEdit(bp, from, to, rva);
	}
}

ERP_CALLBACK(cbButtonBpDelete) {
	int		index = LIST(lsBp)->selected;

	bp_s	*next = BP;
	bp_s	*curr = NULL;

	if(index < 0) {
		return;
	}

	while(next->next) {
		if(next->next->index == index) {
			curr = next->next;
			next->next = curr->next;

			break;
		}

		next = next->next;
	}

	if(curr) {
		mem_delete(curr);

		bpFlush();
	}
}

ERP_CALLBACK(cbCheckBpDynmCode)		{ CONFIG.bp.dynamicCode		= TRUE;	}
ERP_CALLBACK(cbCheckBpDynmMemory)	{ CONFIG.bp.dynamicMemory	= TRUE;	}
ERP_CALLBACK(cbCheckBpStride)		{ CONFIG.bp.stride			= TRUE;	}
ERP_CALLBACK(cbRadioBpAlways) { CONFIG.bp.dynamicCodeMode	= 0; }
ERP_CALLBACK(cbRadioBpStride) { CONFIG.bp.dynamicCodeMode	= 1; }
ERP_CALLBACK(cbRadioBpReturn) { CONFIG.bp.dynamicCodeMode	= 2; }


ERP_CALLBACK(cbListSection) {}
ERP_CALLBACK(cbButtonSectionExport) {}
ERP_CALLBACK(cbButtonSectionImport) {}
ERP_CALLBACK(cbButtonSectionBp) {}

ERP_CALLBACK(cbButtonSectionAlloc) {
	guiSection(&sectionCallback);
}


ERP_CALLBACK(cbListImport) {}

ERP_CALLBACK(cbButtonImportEdit) {
	DWORD a = 1;

	guiImport(a, &importCallback);
}

ERP_CALLBACK(cbButtonImportFix) {}
ERP_CALLBACK(cbButtonImportBp) {}
ERP_CALLBACK(cbButtonImportRebuild) {}

ERP_CALLBACK(cbListModule) {}
ERP_CALLBACK(cbButtonModuleExport) {}
ERP_CALLBACK(cbButtonModuleImport) {}
ERP_CALLBACK(cbButtonModuleBp) {}


ERP_CALLBACK(cbDumpButonOk) {
	win->quit(1);
}

ERP_CALLBACK(cbConfigButtonOk) {
	win->quit(1);
}


ERP_CALLBACK(cbRegisterButtonOk) {
	win->quit(0);
}


ERP_CALLBACK(cbConfigButtonCancel) {
	win->quit(0);
}


ERP_CALLBACK(cbAboutButtonOk) {
	win->quit(0);
}


ERP_CALLBACK(cbBpButtonOk) {
	win->quit(1);
}

ERP_CALLBACK(cbBpButtonCancel) {
	win->quit(0);
}


ERP_CALLBACK(cbSectionButtonOk) {
	win->quit(1);
}

ERP_CALLBACK(cbSectionButtonCancel) {
	win->quit(0);
}


ERP_CALLBACK(cbImportButtonOk) {
	win->quit(1);
}

ERP_CALLBACK(cbImportButtonCancel) {
	win->quit(0);
}
