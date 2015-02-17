/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ pe.h, 0.1.1027
*/

#ifndef _ANK_PE_H_
#define _ANK_PE_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 从文件或缓冲区中读取字符串*/
char * readFromFile(FILE *fp, BOOL incase);
char * readFromBuffer(char *buffer, size_t bufsize);


/* 内存地址 => RVA */
DWORD addr2rva(DWORD addr);
/* RVA => 磁盘偏移量 */
DWORD rva2offset(DWORD rva);
/* 由内存地址获得区段 */
char * addr2section(DWORD addr);


/* 导入表函数处理 */
void peFunction(FILE *fp, IMAGE_IMPORT_DESCRIPTOR *imp);


/* 区段表初始化 */
void peSection();
/* 导入表处理 */
void peImport(BOOL update);


/* 程序入口计算 */
void peEntry();


/* 添加断点 */
void peBreakAddress(DWORD addr);
void peBreakRange(DWORD from, DWORD to);


/* 断点总循环 */
void loop(DWORD eip);


#if defined(__cplusplus)
	}
#endif

#endif   /* _ANK_PE_H_ */