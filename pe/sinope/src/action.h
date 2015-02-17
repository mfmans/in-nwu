/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ action.h, 0.1.1104
*/

#ifndef _SNP_ACTION_H_
#define _SNP_ACTION_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 线程 */
void threadFlush();

/* 断点 */
void bpDraw(bp_s *bp);
void bpFlush();
void bpActive(int index);
void bpAdd(DWORD from, DWORD to, BOOL rva);
void bpEdit(bp_s *bp, DWORD from, DWORD to, BOOL rva);
char * bpCallback(int *from, int *to, BOOL *rva);

/* 区段 */
void sectionDraw(section_s *section);
void sectionFlush();
void sectionAdd(char *name, DWORD address, DWORD size);
char * sectionCallback(char *name, int size);

/* 导入表 */
void importDraw(import_s *import);
void importFlush();
char * importCallback(int size);

/* 模块 */
void moduleDraw(module_s *module);
void moduleFlush();


#if defined(__cplusplus)
	}
#endif

#endif   /* _SNP_ACTION_H_ */