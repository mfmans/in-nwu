/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ process.h, 0.1.1028
*/

#ifndef _ANK_PROCESS_H_
#define _ANK_PROCESS_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 状态 */
typedef enum { LOAD, RUN, PAUSE, BREAK } status_e;

/* PE 结构 */
typedef enum { DOS_HEADER, PE_HEADER, OPTIONAL_HEADER, SECTION_TABLE, IMPORT_TABLE } struct_e;

/* 所有者 */
typedef enum { SYSTEM, PE, DYNAMIC, USER } owner_e;


/* 进程信息 */
typedef struct {
	char	*file;

	/* 状态 */
	status_e status;

	/* 进程信息 */
	DWORD	procId;
	HANDLE	procHandle;

	/* 入口 */
	DWORD	entry;

	/* 基地址定位 */
	DWORD	baseAddr;
	DWORD	baseRVAddr;
	long	baseOffset;
} process_h;

/* 断点 */
typedef struct bp_s {
	/* 开始和结束地址 */
	DWORD	addrStart;
	DWORD	addrEnd;

	/* RVA */
	DWORD	rvaStart;
	DWORD	rvaEnd;

	/* 区段 */
	char	*section;

	int index;
	struct bp_s *next;
} bp_s;

/* 区段 */
typedef struct section_s {
	/* 开始位置 */
	DWORD	address;
	DWORD	virtualaddr;
	DWORD	offset;

	/* 大小 */
	DWORD	size;

	/* 名称 */
	char	*name;

	/* 所有者 */
	owner_e owner;

	int index;
	struct section_s *next;
} section_s;

/* 导入表 */
typedef struct import_s {
	char	*dll;

	/* 函数名或序号 */
	char	*function;
	DWORD	ordinal;

	/* 入口地址 */
	DWORD	entryTable;
	DWORD	entryReal;

	/* IAT 位置 */
	DWORD	iatAddr;
	DWORD	iatOffset;
	/* 原始 IAT 指向 */
	DWORD	iatPoint;

	int index;
	struct import_s *next;
} import_s;

/* 模块 */
typedef struct module_s {
	char	*path;

	/* 地址和大小 */
	DWORD	address;
	DWORD	size;

	int index;
	struct module_s *next;
} module_s;



/* 运行进程 */
void threadProcess(char *arg);


#if defined(__cplusplus)
	}
#endif

#endif   /* _ANK_PROCESS_H_ */