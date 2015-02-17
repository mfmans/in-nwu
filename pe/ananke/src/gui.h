/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ gui.h, 0.1.1027
*/

#ifndef _ANK_GUI_H_
#define _ANK_GUI_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 主窗口注册类名 */
#define WIN_CLASS		"ananke_window_class"
/* 标题 */
#define WIN_TITLE		"Ananke PE Loader"

/* 窗口宽高 */
#define WIN_WIDTH		1050
#define WIN_HEIGHT		700



void guiDraw(HWND hwnd);
void guiTitle(char *file, int status);
void guiCommand(DWORD id);
void guiNotify(LPARAM lParam);
void guiKeyboard(DWORD key);

void guiListDraw(HWND hwnd, size_t count, char **title, int *width);
void guiListClear(HWND hwnd);
void guiListModify(HWND hwnd, int index, size_t count, char **data);
void guiListRemove(HWND hwnd, int index);
void guiListSelect(HWND hwnd, int index);
int guiListSelected(HWND hwnd);
int guiListPush(HWND hwnd, size_t count, char **data);


void cmdInit();
void cmdEnable(BOOL enable);
void cmdExecuteEnable(BOOL enable);
void cmdExecute();
void cmdDump();
void cmdSuspend();
void cmdRunBp();
void cmdRunModule();
void cmdRunSingle();
void cmdRunRegister();
void cmdConfig();
void cmdAbout();
void cmdAddress(DWORD rva, DWORD eip, char *section, char *code);
void cmdWorking(int append, char *format, ...);

void bpInit();
void bpEnable(BOOL enable);
void bpSelect(int index);
void bpPush(bp_s *bp);
void bpUpdateTrigger(bp_s *bp, BOOL trigger);
void bpCheckDyncode(BOOL checked);
void bpCheckDynmemory(BOOL checked);
void bpCheckDiffsec(BOOL checked);
void bpRadio(HWND hwnd);

void sectionInit();
void sectionEnable(BOOL enable);
void sectionSelect(int index);
void sectionPush(section_s *section);
void sectionUpdate(section_s *section);

void importInit();
void importEnable(BOOL enable);
void importSelect(int index);
void importPush(import_s *import);
void importUpdate(import_s *import);

void moduleInit();
void moduleEnable(BOOL enable);
void moduleSelect(int index);
void modulePush(module_s *module);



#if defined(__cplusplus)
	}
#endif

#endif   /* _ANK_GUI_H_ */