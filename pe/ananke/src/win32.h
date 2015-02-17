/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ win32.h, 0.1.1027
*/

#ifndef _ANK_WIN32_H_
#define _ANK_WIN32_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 格式化 MessageBox */
int MessageBoxFormat(UINT type, char *format, ...);

/* 选择文件打开或保存 */
char * SelectFileOpen(char *filter, char *title);
char * SelectFileSave(char *file, char *title);


#if defined(__cplusplus)
	}
#endif

#endif   /* _ANK_WIN32_H_ */