/*
	$ Invoke   (C) 2005-2012 MF
	$ module.c, 0.1.1124
*/

#include <Windows.h>
#include <Psapi.h>
#include <Tlhelp32.h>
#include "base.h"
#include "gui.h"

/* 最长函数名 */
#define MAX_FUNCTION	256

/* 指令检测 */
#define MOVE_STACK		"pop\0push\0"
#define MOVE_CALL		"call\0"
#define MOVE_JUMP		"jmp\0jcxz\0jecxz\0je\0jne\0jz\0jnz\0js\0jns\0jc\0jnc\0jo\0jno\0ja\0jna\0jae\0jnae\0jg\0jng\0jge\0jnge\0jb\0jnb\0jbe\0jnbe\0jl\0jnl\0jle\0jnle\0jp\0jnp\0jpe\0jpo\0"
#define MOVE_RET		"ret\0"


static void moduleAppend(module_s *module) {
	module_s *next = MOD;

	while(next->next) {
		next = next->next;
	}

	next->next = module;
}

static void moduleRemove(DWORD base) {
	module_s	*next = MOD;
	module_s	*curr = NULL;
	function_s	*function;

	while(next->next) {
		if(next->next->base == base) {
			curr = next->next;
			next->next = curr->next;
		}
	}

	if(curr) {
		while(curr->function) {
			function = curr->function->next;
			curr->function = function->next;

			if(function->hijack) {
				function->hijack->prev->next = function->hijack->next;
			}

			DEL(function->hijack);
			DEL(function);
		}

		DEL(curr);
	}
}

static function_s * moduleExport(int base) {
	unsigned int i, j, offset;
	unsigned int baseAddr, baseName, baseOrdinal, address;

	function_s **list	= NULL;
	function_s *func	= NULL;

	IMAGE_EXPORT_DIRECTORY table;

	ZeroMemory(&table, sizeof table);
	
	if(memoryRead(&table, sizeof table) == 0) {
		return NULL;
	}
	if(table.NumberOfFunctions == 0) {
		return NULL;
	}
	if(table.NumberOfNames > table.NumberOfFunctions) {
		return NULL;
	}

	NEW(list, function_s *, table.NumberOfFunctions);
	
	offset		= table.Base;
	baseAddr	= base + table.AddressOfFunctions;
	baseName	= base + table.AddressOfNames;
	baseOrdinal	= base + table.AddressOfNameOrdinals;

	/* 创建函数 */
	for(i = 0; i < table.NumberOfFunctions; i++) {
		NEW(list[i], function_s, 1);

		/* 连接链表 */
		if(i) {
			list[i - 1]->next = list[i];
		}
		
		/* 读取地址 */
		memorySeek(baseAddr + i * sizeof(DWORD), SEEK_SET);
		memoryRead(&list[i]->entry, sizeof(DWORD));

		list[i]->entry		= list[i]->entry + base;
		list[i]->bp			= list[i]->entry;
		list[i]->ordinal	= i + offset;
	}

	/* 读取名字 */
	for(i = 0; i < table.NumberOfNames; i++) {
		j = 0;

		/* 读取序号 */
		memorySeek(baseOrdinal + i * sizeof(WORD), SEEK_SET);
		memoryRead(&j, sizeof(WORD));
		
		/* 定位函数 */
		func = list[j];

		/* 名字地址 */
		memorySeek(baseName + i * sizeof(DWORD), SEEK_SET);
		memoryRead(&address, sizeof(int));

		/* 读取名字 */
		memorySeek(base + address, SEEK_SET);
		func->name = memoryReadString(MAX_FUNCTION, 1);
	}

	func = list[0];

	DEL(list);

	return func;
}

static module_s * moduleRead(MODULEENTRY32 *info) {
	module_s	*module;
	function_s	*function;

	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS pe;

	int base = (int) info->modBaseAddr;
	int size = (int) info->modBaseSize + base;

	memorySeek(base, SEEK_SET);

	/* 解析 PE 头 */
	if((memoryRead(&dos, sizeof dos) == 0) || (dos.e_magic != IMAGE_DOS_SIGNATURE)) {
		return NULL;
	} else {
		memorySeek(base + (int) dos.e_lfanew, SEEK_SET);
	}
	if((memoryRead(&pe, sizeof pe) == 0) || (pe.Signature != IMAGE_NT_SIGNATURE)) {
		return NULL;
	}

	/* 判断有没有输出表 */
	if(pe.OptionalHeader.DataDirectory[0].VirtualAddress == 0) {
		return NULL;
	}

	/* 移动到输出表 */
	memorySeek(base + pe.OptionalHeader.DataDirectory[0].VirtualAddress, SEEK_SET);

	/* 解析输出表 */
	if((function = moduleExport(base)) == NULL) {
		return NULL;
	}

	NEW(module, module_s, 1);
	NEW(module->modname, char, strlen(info->szModule) + 1);
	NEW(module->filename, char, strlen(info->szExePath) + 1);

	module->base		= base;
	module->size		= (int) info->modBaseSize;
	module->function	= function;

	strcpy(module->modname, info->szModule);
	strcpy(module->filename, info->szExePath);

	while(function) {
		function->module = module;
		function = function->next;
	}

	return module;
}

static BOOL moduleString(char *output, char *code) {
	size_t codeLen	= 0;
	size_t outLen	= strlen(output);

	while(*code) {
		codeLen = strlen(code);

		if(codeLen <= outLen) {
			if(strncmp(output, code, codeLen) == 0) {
				return TRUE;
			}
		}

		/* 寻找下一个指令 */
		while(*(code++)) {
			;
		}
	}

	return FALSE;
}


static void moduleList() {
	module_s		*module;
	MODULEENTRY32	info;

	info.dwSize = sizeof info;

	if(Module32First(RT.snapshot, &info) == FALSE) {
		return;
	}

	do {
		/* 忽略主模块 */
		if(strcmp(info.szExePath, RT.module) == 0) {
			continue;
		}

		if(module = moduleRead(&info)) {
			moduleAppend(module);
		}
	} while(Module32Next(RT.snapshot, &info));
}


void moduleShow() {
	int		i;
	int		count	= 0;
	DWORD	code	= 0;

	module_s	*next		= MOD;
	module_s	**module	= NULL;
	function_s	*function	= NULL;

	/* 判断进程是否退出 */
	if(GetExitCodeProcess(RT.handle, &code)) {
		if(code != STILL_ACTIVE) {
			exit(EXIT_SUCCESS);
		}
	}

	/* 初始化模块表 */
	if(next->next == NULL) {
		moduleList();
	}

	while(next->next) {
		count++;
		next = next->next;
	}

	NEW(module, module_s *, count);

	count	= 0;
	next	= MOD->next;

	while(next) {
		i = 0;
		function = next->function;

		while(function) {
			if(function->hook == FALSE) {
				i = 1; break;
			}

			function = function->next;
		}

		if(i) {
			module[count++] = next;
		}

		next = next->next;
	}
	
	guiBp(NULL, NULL, module, (unsigned int) count, -1);

	/* 清理 */
	DEL(module);
}

void moduleSet(module_s *module, DWORD offset, BOOL moveStack, BOOL moveCall, BOOL moveJump, BOOL moveRet) {
	int size;
	unsigned int i;

	char	buffer[64];
	char	output[128];

	function_s *func = module->function;

	while(func) {
		func->bp = func->entry;

		/* 指令测试 */
		for(i = 1; i < offset; i++) {
			ZeroMemory(buffer, sizeof buffer);
			ZeroMemory(output, sizeof output);

			/* 读取失败 */
			if(processRead(func->bp, buffer, 64) == 0) {
				break;
			}
			/* 反汇编失败 */
			if((size = disasm(func->bp, buffer, 64, output, 128)) == 0) {
				break;
			}

			/* 跳跃测试 */
			if(moveStack && moduleString(output, MOVE_STACK)) {
				break;
			}
			if(moveCall && moduleString(output, MOVE_CALL)) {
				break;
			}
			if(moveJump && moduleString(output, MOVE_JUMP)) {
				break;
			}
			if(moveRet && moduleString(output, MOVE_RET)) {
				break;
			}

			func->bp += size;
		}

		func = func->next;
	}
}

function_s * moduleFind(HWND win, char *module, char *function, unsigned long int ordinal, int *mode) {
	module_s	*mod	= MOD->next;
	function_s	*func	= NULL;

	while(mod) {
		if(strEqual(module, mod->modname) || strEqual(module, mod->filename)) {
			if(func = mod->function) {
				while(func) {
					do {
						if(func->name == NULL) {
							if((function != NULL) || (func->ordinal != ordinal)) {
								break;
							}
						} else if(function == NULL) {
							break;
						} else if(strcmp(func->name, function)) {
							break;
						}

						/* 已经手动劫持 */
						if(func->hijack) {
							if(*mode == -1) {
								if(confirm(win, "是否使用导入文件中的函数定义信息覆盖手动添加的信息？")) {
									*mode = 1;
								} else {
									*mode = 0;
								}
							}

							if(*mode) {
								return func;
							} else {
								return NULL;
							}
						} else {
							return func;
						}
					} while(0);

					func = func->next;
				}
			}
		}

		mod = mod->next;
	}

	return NULL;
}

void moduleDelete(DWORD base) {
	module_s	*module	= MOD, *mod = NULL;
	hijack_s	*hijack	= HIJ->next, *hij;

	function_s	*function;

	while(module->next) {
		if(module->next->base == base) {
			mod		= module;
			module	= module->next;
			
			break;
		}

		module = module->next;
	}

	if(mod == NULL) {
		return;
	}
	if(module->base != base) {
		return;
	}

	/* 清理劫持表 */
	while(hijack) {
		if(hijack->function->module == module) {
			hij = hijack;
			hij->prev->next = hij->next;

			hijack = hij->next;

			DEL(hij);
		} else {
			hijack = hijack->next;
		}
	}

	/* 清理函数表 */
	while(module->function) {
		function = module->function->next;

		DEL(module->function);

		module->function = function;
	}

	/* 更新链表 */
	mod->next = module->next;

	/* modname 是 filename 的一部分 */
	do {
		if(module->modname >= module->filename) {
			if(module->modname < (module->filename + strlen(module->filename))) {
				break;
			}
		}

		DEL(module->modname);
	} while(0);

	DEL(module->filename);
	DEL(module);
}

module_s * moduleLoad(HANDLE handle, DWORD base) {
	char	*name;
	char	buffer[MAX_PATH + 1];

	size_t	length;
	HANDLE	map, view;

	module_s *module, *next = MOD;
	function_s	*function;

	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS pe;

	map		= CreateFileMapping(handle, NULL, PAGE_READONLY, 0, 0, NULL);
	view	= MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0);

	if((length = GetMappedFileName(RT.handle, (LPVOID) base, buffer, MAX_PATH)) == 0) {
		return NULL;
	}

	/* 转换路径 */
	device2NtLetter(buffer);

	/* 查找是否已加载 */
	while(next->next) {
		/* 模块已存在 */
		if((next->next->base == base) && strEqual(buffer, next->next->filename)) {
			return NULL;
		}

		next = next->next;
	}

	/* 移动到基地址 */
	memorySeek(base, SEEK_SET);

	/* 解析 PE 头 */
	if((memoryRead(&dos, sizeof dos) == 0) || (dos.e_magic != IMAGE_DOS_SIGNATURE)) {
		return NULL;
	} else {
		memorySeek(base + (int) dos.e_lfanew, SEEK_SET);
	}
	if((memoryRead(&pe, sizeof pe) == 0) || (pe.Signature != IMAGE_NT_SIGNATURE)) {
		return NULL;
	}

	/* 判断有没有输出表 */
	if(pe.OptionalHeader.DataDirectory[0].VirtualAddress == 0) {
		return NULL;
	}

	/* 移动到输出表 */
	memorySeek(base + pe.OptionalHeader.DataDirectory[0].VirtualAddress, SEEK_SET);

	/* 解析输出表 */
	if((function = moduleExport(base)) == NULL) {
		return NULL;
	}

	NEW(module, module_s, 1);
	NEW(module->filename, char, strlen(buffer) + 1);

	strcpy(module->filename, buffer);

	module->base		= base;
	module->modname		= module->filename;
	module->function	= function;

	/* 处理模块名 */
	while(name = strstr(module->modname, "\\")) {
		module->modname = name + 1;
	}

	/* 更新函数 */
	while(function) {
		function->module = module;
		function = function->next;
	}

	/* 附加到链表中 */
	next->next = module;

	return module;
}

void moduleDump() {
	FILE	*fp;
	char	*file, *data;

	size_t	read;
	size_t	size = RT.mainEnd - RT.mainStart;

	if((file = selectFileSave(wMain.hwnd, "invoke.dmp", "请选择保存路径")) == NULL) {
		return;
	}

	if((fp = fopen(file, "wb")) == NULL) {
		warning(wMain.hwnd, "无法打开选定的路径。");
		DEL(file);
		
		return;
	}

	DEL(file);
	NEW(data, char, size);

	/* 读取内存 */
	if((read = processRead(RT.mainStart, data, size)) == 0) {
		error(wMain.hwnd, "读取内存数据失败。");
		DEL(data);

		return;
	}

	/* 写入 */
	fwrite(data, read, 1, fp);
	fclose(fp);

	if(read == size) {
		notice(wMain.hwnd, "导出成功。");
	} else {
		warning(wMain.hwnd, "读取内存数据不完全成功，导出的内容可能不完整。");
	}
}
