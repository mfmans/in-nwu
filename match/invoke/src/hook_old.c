/*
	$ Invoke   (C) 2005-2012 MF
	$ hook_old.c, v1 @ 0.1.1125

	!!! !!! DO NOT LINK IN PROJECT
*/

#include "base.h"


/* 指令转换 */
#define CODE(code, count, ...) \
	if(outOffset = hookJumpCreate(&ncodeSize, ncode, output, code, count, __VA_ARGS__)) { \
		break; \
	}


/* 转发器 */
static DWORD routeAddr = 0;
static DWORD routeSize = 0;


/* 跳转代码创建 */
static int hookJumpCreate(int *ncodeSize, char *ncode, char *output, char *code, int count, ...) {
	va_list arg;

	int i;
	int address;
	size_t length = strlen(code);

	/* 前缀不正确 */
	if(memcmp(output, code, length)) {
		return 0;
	}
	/* 指令后为 \t */
	if(output[length] != '\t') {
		return 0;
	}
	/* 获取指令后地址 */
	if(sscanf(output + length + 1, "%x", &address) == 0) {
		return 0;
	}

	/* 可变参 */
	va_start(arg, count);

	/* 为 buffer 写入 */
	for(i = 0; i < count; i++) {
		*((unsigned char *) (ncode + i)) = va_arg(arg, unsigned char);
	}

	va_end(arg);

	*ncodeSize = count;

	return length + 1;
}


/* 跳转地址计算 */
static DWORD hookAddress(function_s *func, int codeSize, char *buffer, char *output) {
	/* 指令中地址偏移 */
	int		outOffset	= 0;
	/* 转换指令大小 */
	int		ncodeSize	= 0;
	/* 转换指令内容 */
	char	ncode[16];

	/* 跳转转移范围 */
	int		jump = 0;
	/* 转发器大小 */
	DWORD	size = 5;
	/* 地址 */
	DWORD	address;

	/* 判断是否是跳转命令 */
	do {
		CODE("call",	1,	0xE8);
		CODE("jmp",		1,	0xE9);

		CODE("jcxz",	6,	0x66, 0x83, 0xF9, 0x00,			/* CMP CX, 0 */
							0x0F, 0x84);					/* JE ... */
		CODE("jecxz",	5,	0x83, 0xF9, 0x00,				/* CMP ECX, 0 */
							0x0F, 0x84);					/* JE ... */

		CODE("je",		2,	0x0F, 0x84);
		CODE("jne",		2,	0x0F, 0x85);
		CODE("jz",		2,	0x0F, 0x84);
		CODE("jnz",		2,	0x0F, 0x85);

		CODE("js",		2,	0x0F, 0x88);
		CODE("jns",		2,	0x0F, 0x89);

		CODE("jc",		2,	0x0F, 0x82);
		CODE("jnc",		2,	0x0F, 0x83);
		CODE("jb",		2,	0x0F, 0x82);
		CODE("jnb",		2,	0x0F, 0x83);

		CODE("jc",		2,	0x0F, 0x82);
		CODE("jnc",		2,	0x0F, 0x83);

		CODE("jo",		2,	0x0F, 0x80);
		CODE("jno",		2,	0x0F, 0x81);

		CODE("ja",		2,	0x0F, 0x87);
		CODE("jna",		2,	0x0F, 0x86);
		CODE("jae",		2,	0x0F, 0x83);
		CODE("jnae",	2,	0x0F, 0x82);

		CODE("jg",		2,	0x0F, 0x8F);
		CODE("jng",		2,	0x0F, 0x8E);
		CODE("jge",		2,	0x0F, 0x8D);
		CODE("jnge",	2,	0x0F, 0x8C);

		CODE("jb",		2,	0x0F, 0x82);
		CODE("jnb",		2,	0x0F, 0x83);
		CODE("jbe",		2,	0x0F, 0x86);
		CODE("jnbe",	2,	0x0F, 0x87);

		CODE("jl",		2,	0x0F, 0x8C);
		CODE("jnl",		2,	0x0F, 0x8D);
		CODE("jle",		2,	0x0F, 0x8E);
		CODE("jnle",	2,	0x0F, 0x8F);

		CODE("jp",		2,	0x0F, 0x8A);
		CODE("jnp",		2,	0x0F, 0x8B);
		CODE("jpe",		2,	0x0F, 0x8A);
		CODE("jpo",		2,	0x0F, 0x8B);
	} while(0);

	/* 计算转发器所需空间大小 */
	if(ncodeSize) {
		size += ncodeSize + 4;
	} else {
		size += codeSize;
	}
	/* 对齐 */
	if(size % 16) {
		size = ((size / 16) + 1) * 16;
	}

	/* 分配空间 */
	if(routeSize < size) {
		if((routeAddr = (DWORD) VirtualAllocEx(RT.handle, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) == 0) {
			die("向调试进程的空间申请内存失败。");
		}

		routeSize = 4096;
	}

	/* 备份原始指令 */
	func->codeSize = codeSize;
	memcpy(func->codeData, buffer, codeSize);

	/* 写入转换后的指令或移动原始指令 */
	if(ncodeSize) {
		/* 读入地址 */
		if(sscanf(output + outOffset, "%x", &address) == 0) {
			die("创建断点过程中无法读取返回地址。");
		}

		/* 计算转移范围 */
		jump = (int) address - ((int) routeAddr + ncodeSize + 4);

		/* 写入转换后的指令 */
		processWrite(routeAddr, ncode, ncodeSize);
		processWrite(routeAddr + ncodeSize, &jump, 4);

		/* 计算总指令大小 */
		ncodeSize += 4;
	} else {
		processWrite(routeAddr, buffer, codeSize);

		ncodeSize = codeSize;
	}

	/* 计算返回地址 */
	address	= routeAddr;
	jump	= ((int) func->bp + codeSize) - ((int) routeAddr + ncodeSize + 5);

	/* 生成 JMP */
	ncode[0] = 0xE9;

	/* 写入 JMP */
	processWrite(routeAddr + ncodeSize, ncode, 1);
	processWrite(routeAddr + ncodeSize + 1, &jump, 4);

	/* 空间计算 */
	routeSize -= size;
	routeAddr += size;

	return address;
}

/* 安装断点 */
static int hookFunction(function_s *func, char *breakpoint) {
	int		size;
	char	buffer[64];
	char	output[128];

	if(processRead(func->bp, buffer, 64) == 0) {
		return 0;
	}

	/* 原来就是 INT3 */
	if((unsigned char) *((unsigned char *) buffer) == 0xCC) {
		func->code = func->bp + 1;
		
		return 1;
	}
	
	/* 首先进行反汇编 */
	if((size = disasm(func->bp, buffer, 64, output, 128)) == 0) {
		return 0;
	}
	/* 对跳转命令进行处理 */
	if((func->code = hookAddress(func, size, buffer, output)) == 0) {
		return 0;
	}

	/* 写入 INT 3 */
	if(processWrite(func->bp, breakpoint, (unsigned int) size) == 0) {
		return 0;
	}

	return 1;
}


/* hooking */
int hookInstall(erp_window_s *win) {
	int failed = 0;

	unsigned int i;
	char breakpoint[32];

	module_s	*module		= NULL;
	function_s	*function	= NULL;

	DWORD		count		= packVar(win->param, 1, DWORD);
	module_s	**modules	= packPtr(win->param, 0, module_s *);

	/* INT 3 */
	memset(breakpoint, 0xCC, sizeof breakpoint);
	
	for(i = 0; i < count; i++) {
		module = modules[i];

		function = module->function;
		
		while(function) {
			if(function->hook == FALSE) {
				if(hookFunction(function, breakpoint)) {
					function->hook = TRUE;
				} else {
					failed = 1;
				}
			}

			function = function->next;
		}
	}

	return failed;
}

void hookUninstall() {
	int failed = 0;

	module_s	*module = MOD->next;
	function_s	*function;

	while(module) {
		function = module->function;

		while(function) {
			if(function->hook && function->codeSize) {
				if(processWrite(function->bp, function->codeData, function->codeSize) == 0) {
					failed = 1;
				}
			}

			function = function->next;
		}

		module = module->next;
	}

	if(failed) {
		warning(NULL, "部分断点卸载失败，被调试的进程可能工作不正常。");
	}
}

function_s * hookFind(DWORD address, module_s **module) {
	module_s	*mod		= MOD->next;
	function_s	*function	= NULL;

	while(mod) {
		if((address >= mod->base) && (address < (mod->base + mod->size))) {
			function = mod->function;

			while(function) {
				if(address == function->bp) {
					*module = mod;

					return function;
				}

				function = function->next;
			}
		}

		mod = mod->next;
	}

	return NULL;
}
