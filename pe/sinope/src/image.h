/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ image.h, 0.1.1104
*/

#ifndef _SNP_IMAGE_H_
#define _SNP_IMAGE_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 结构信息 */
struct section_s;
struct import_s;
struct module_s;


/* 线程 */
typedef struct thread_s {
	DWORD	id;
	HANDLE	handle;

	/* 开始地址 */
	DWORD	start;

	/* 运行信息 */
	struct {
		DWORD	eip;
		BOOL	main;
		BOOL	dynamic;

		struct section_s	*section;
		struct module_s		*module;
	} run;

	int index;
	struct thread_s *next;
} thread_s;


/* 镜像 */
typedef struct runtime_s {
	/* 文件 */
	char	*file;
	char	*filename;

	/* 进程存活 */
	BOOL	alive;
	/* 运行中 */
	BOOL	running;
	/* 单步执行 */
	BOOL	singleStep;

	/* 进程 */
	struct {
		DWORD	pid;
		HANDLE	handle;

		/* 主模块大小 */
		DWORD	size;

		/* 重定位后的基地址 */
		DWORD	base;
		/* 实际入口 */
		DWORD	entry;

		/* 调试线程 */
		thread_s	*tDebug;
		thread_s	*tBreak;

		/* 主模块 */
		struct module_s *module;
	} proc;

	/* PE */
	struct {
		/* CUI */
		BOOL	console;

		/* 默认基地址 */
		DWORD	base;

		/* 入口地址 */
		DWORD	entry;

		/* 区段地址 */
		long	section;
		/* 导入点地址 */
		DWORD	import;
	} pe;
} runtime_s;


/* 设置 */
typedef struct config_s {
	/* 已申请控制台 */
	BOOL	console;

	/* 反汇编 */
	BOOL	disasm;
	/* 用主模块起始地址作为基地址 */
	BOOL	modBase;
	/* 断点 beep */
	BOOL	beep;
	/* EIP */
	BOOL	dllEip;

	/* 区段表显示的大小为 PE 中大小 */
	BOOL	secSize;

	/* IAT  0=RVA, 1=addr, 2=offset */
	int		iatType;

	/* 导出 */
	struct {
		/* 使用内存大小 */
		BOOL	realSize;
		/* 尝试对齐 */
		BOOL	sectionAlign;
		/* 直接复制 */
		BOOL	copy;

		/* 填充 IAT */
		BOOL	fillIat;
	} dump;

	/* 断点 */
	struct {
		/* 动态代码 */
		BOOL	dynamicCode;
		int		dynamicCodeMode;

		/* 动态内存 */
		BOOL	dynamicMemory;

		/* 跨段跳转 */
		BOOL	stride;
	} bp;
} config_s;


/* 断点 */
typedef struct bp_s {
	/* 位置 */
	DWORD	addrFrom, addrTo;
	DWORD	vaFrom, vaTo;

	/* 使用 RVA */
	BOOL	rva;

	int index;
	struct bp_s *next;
} bp_s;

/* 区段表 */
typedef struct section_s {
	char	*name;

	/* 位置 */
	DWORD	addr, va, offset;
	/* 大小 */
	DWORD	sizeImage, sizeRaw;

	/* 动态申请 */
	BOOL	dynamic;
	/* 来自文件 */
	BOOL	file;

	/* 副本 */
	void	*copy;

	int index;
	struct section_s *next;
} section_s;

/* 导入表 */
typedef struct import_s {
	char	*dll;

	/* 函数 */
	char	*function;
	DWORD	ordinal;

	/* 当前指向 */
	DWORD	point;
	/* 实际入口 */
	DWORD	entry;
	/* HOOK 入口 */
	DWORD	hook;

	/* IAT 位置 */
	DWORD	iatVa, iatOffset;
	/* IAT 原始数据 */
	DWORD	iatData;

	int index;
	struct import_s *next;
} import_s;

/* 模块 */
typedef struct module_s {
	char	*path;

	DWORD	address;
	DWORD	size;

	int index;
	struct module_s *next;
} module_s;


/* 运行信息 */
#if defined(_SNP_PROCESS_)
	/* 默认设置 */
	config_s	CONFIG			= {FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, 0, {TRUE, TRUE, FALSE, TRUE}, {TRUE, 1, TRUE, TRUE}};

	runtime_s	RUNTIME;
	thread_s	THREAD_HEAD,	*THREAD		= &THREAD_HEAD;

	bp_s		BP_HEAD,		*BP			= &BP_HEAD;
	section_s	SECTION_HEAD,	*SECTION	= &SECTION_HEAD;
	import_s	IMPORT_HEAD,	*IMPORT		= &IMPORT_HEAD;
	module_s	MODULE_HEAD,	*MODULE		= &MODULE_HEAD;
#else
	extern runtime_s	RUNTIME;
	extern config_s		CONFIG;

	extern thread_s		*THREAD;

	extern bp_s			*BP;
	extern section_s	*SECTION;
	extern import_s		*IMPORT;
	extern module_s		*MODULE;
#endif


/* 操作指针 */
#if defined(_SNP_PE_)
	BOOL (*pfRead)(void *, size_t);
	void (*pfSeek)(long, int);
	long (*pfTell)();
	DWORD (*pfOffset)(DWORD);
	BOOL (*pfEnd)();
#else
	extern BOOL (*pfRead)(void *, size_t);
	extern void (*pfSeek)(long, int);
	extern long (*pfTell)();
	extern DWORD (*pfOffset)(DWORD);
	extern BOOL (*pfEnd)();
#endif


/* 进程 */
thNew(Process);

/* 进程相关 */
BOOL procMapModule();
void procMapSection();
void procImport();
void procTitle(char *status);
void procClear();

/* 线程相关 */
thread_s * threadGet(DWORD id);
thread_s * threadCreate(HANDLE handle, DWORD id, DWORD addr);
void threadExit(HANDLE handle);

/* 内存读写 */
BOOL memoryRead(DWORD address, void *buffer, DWORD size);
BOOL memoryWrite(DWORD address, void *buffer, DWORD size);

/* 程序领空 */
BOOL isPeModule(DWORD addr);

/* PE */
BOOL pe();
char * peHeader(BOOL refresh);
char * peSection();
void peImport();
void dump();

/* 地址转换 */
DWORD va2rva(DWORD va);
DWORD va2addr(DWORD va);
DWORD va2offset(DWORD va);
DWORD rva2va(DWORD rva);
DWORD rva2addr(DWORD rva);
DWORD addr2va(DWORD addr);
DWORD addr2rva(DWORD addr);

/* 归属 */
section_s * getSectionByVa(DWORD va);
section_s * getSectionByAddr(DWORD addr);
import_s * getImportByAddr(DWORD addr);
module_s * getModuleByAddr(DWORD addr);

/* 循环器 */
void loop();
void loopTerminate();
void loopDisasm(BOOL disasm);


/* 镜像读取 */
BOOL dataReadFile(void *buffer, size_t size);
BOOL dataReadMemory(void *buffer, size_t size);
void dataSeekFile(long offset, int mode);
void dataSeekMemory(long offset, int mode);
long dataTellFile();
long dataTellMemory();
DWORD dataOffsetFile(DWORD va);
DWORD dataOffsetMemory(DWORD va);
BOOL dataEndFile();
BOOL dataEndMemory();


#if defined(__cplusplus)
	}
#endif

#endif   /* _SNP_IMAGE_H_ */