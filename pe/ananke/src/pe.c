/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ pe.c, 0.1.1027
*/

#include <Windows.h>
#include <Psapi.h>
#include "base.h"
#include "pe.h"


extern process_h	 PROCESS;

extern bp_s			*BP;
extern section_s	*SECTION;
extern import_s		*IMPORT;
extern module_s		*MODULE;


char * readFromFile(FILE *fp, BOOL incase) {
	size_t i;

	char *result;
	char buffer[MAX_PATH] = {0};

	for(i = 0; i < MAX_PATH; i++) {
		fread(buffer + i, sizeof(char), 1, fp);

		if(incase) {
			if((buffer[i] >= 'A') && (buffer[i] <= 'Z')) {
				buffer[i] |= 0x20;
			}
		}

		if(buffer[i] == 0) {
			break;
		}
	}

	if(buffer[0] == 0) {
		return NULL;
	} else if(buffer[MAX_PATH - 1] != 0) {
		return NULL;
	} else {
		mem_alloc(result, char, i + 1);

		strcpy(result, buffer);

		return result;
	}
}

char * readFromBuffer(char *buffer, size_t bufsize) {
	char *result;
	size_t i;

	if(buffer[0] == 0) {
		return NULL;
	} else {
		i = 0;
	}

	mem_alloc(result, char, bufsize + 1);

	while((i < bufsize) && (buffer[i] != 0)) {
		result[i] = buffer[i];

		i++;
	}

	if(i < bufsize) {
		mem_realloc(result, char, i);

		result[i] = 0;
	}

	return result;
}


DWORD addr2rva(DWORD addr) {
	section_s *prev = NULL;
	section_s *curr = SECTION->next;

	while((curr != NULL) && (curr->address <= addr)) {
		prev = curr;
		curr = curr->next;
	}

	if(prev == NULL) {
		return 0;
	} else {
		if(addr > (prev->address + prev->size)) {
			return 0;
		} else {
			return prev->virtualaddr + (addr - prev->address);
		}
	}
}

DWORD rva2offset(DWORD rva) {
	section_s *prev = NULL;
	section_s *curr = SECTION->next;

	while((curr != NULL) && (curr->virtualaddr <= rva)) {
		prev = curr;
		curr = curr->next;
	}

	if(prev == NULL) {
		return rva;
	} else {
		return prev->offset + (rva - prev->virtualaddr);
	}
}

char * addr2section(DWORD addr) {
	section_s *prev = NULL;
	section_s *curr = SECTION->next;

	while((curr != NULL) && (curr->address <= addr)) {
		prev = curr;
		curr = curr->next;
	}

	if(prev == NULL) {
		return NULL;
	} else {
		if(addr > (prev->address + prev->size)) {
			return NULL;
		} else {
			return prev->name;
		}
	}
}


void peFunction(FILE *fp, IMAGE_IMPORT_DESCRIPTOR *imp) {
	int		i;
	char	*name, *function;
	long	offset1, offset2;

	import_s			*next, *import;
	IMAGE_THUNK_DATA	thunk;

	/* 保存回溯位置 */
	offset1 = ftell(fp);

	/* 转发类型的跳过 */
	if(imp->ForwarderChain != 0) {
		return;
	}
	
	/* 找 DLL 名称 */
	fseek(fp, rva2offset(imp->Name), SEEK_SET);

	/* 不允许匿名 */
	if((name = readFromFile(fp, TRUE)) == NULL) {
		fseek(fp, offset1, SEEK_SET); return;
	}

	/* 找 INT */
	fseek(fp, rva2offset(imp->OriginalFirstThunk), SEEK_SET);

	i		= 0;
	next	= IMPORT;

	while(next->next != NULL) {
		next = next->next;
	}

	while(!feof(fp)) {
		if(fread(&thunk, sizeof thunk, 1, fp) == 0) {
			break;
		}
		if(*((DWORD *) &thunk) == 0) {
			break;
		}

		mem_alloc(import, import_s, 1);

		offset2 = ftell(fp);

		if(thunk.u1.Function & 0x80000000) {
			import->ordinal = thunk.u1.Ordinal & 0x7FFFFFFF;
		} else {
			fseek(fp, rva2offset(thunk.u1.AddressOfData) + sizeof(WORD), SEEK_SET);

			if((function = readFromFile(fp, FALSE)) == NULL) {
				mem_delete(import);
				fseek(fp, offset1, SEEK_SET);

				return;
			}

			import->function = function;

			fseek(fp, offset2, SEEK_SET);
		}

		import->dll			= name;
		import->iatAddr		= imp->FirstThunk + i * sizeof thunk;
		import->iatOffset	= rva2offset(import->iatAddr);
		import->iatPoint	= thunk.u1.AddressOfData;

		next->next = import;
		next = import;

		i++;
	}

	fseek(fp, offset1, SEEK_SET);
}


void peSection() {
	section_s *section = SECTION->next;

	while(section != NULL) {
		section->address = section->virtualaddr + PROCESS.baseAddr;

		sectionUpdate(section);

		section = section->next;
	}
}

void peImport(BOOL update) {
	DWORD	address;
	SIZE_T	count;

	import_s *import = IMPORT->next;

	while(import != NULL) {
		address = 0;

		ReadProcessMemory (
			PROCESS.procHandle,
			(LPCVOID) (import->iatAddr + PROCESS.baseAddr),
			&address, sizeof address, &count
		);

		if(count) {
			import->entryReal = address;

			if(update == FALSE)  {
				import->entryTable = address;
			}

			importUpdate(import);
		}

		import = import->next;
	}
}


void peEntry() {
	int			i, j;
	char		name[MAX_PATH], temp[MAX_PATH];

	module_s	*next, *curr;

	HMODULE		module[4096];
	DWORD		needed;
	MODULEINFO	info;

	/* 获取进程名 */
	if(GetModuleFileNameEx(PROCESS.procHandle, NULL, name, MAX_PATH) == 0) {
		return;
	}

	/* 获取所有模块 */
	if(EnumProcessModules(PROCESS.procHandle, module, sizeof module, &needed) == FALSE) {
		return;
	}

	j		= needed / sizeof(HMODULE);
	next	= MODULE;

	for(i = 0; i < j; i++) {
		if(GetModuleFileNameEx(PROCESS.procHandle, module[i], temp, MAX_PATH) == 0) {
			continue;
		}

		/* 主模块, 获取真实的加载地址 */
		if(strcmp(name, temp) == 0) {
			PROCESS.baseAddr = (DWORD) module[i];
		}

		/* 加入模块表 */
		mem_alloc(curr, module_s, 1);
		mem_alloc(curr->path, char, strlen(temp) + 1);

		curr->address = (DWORD) module[i];

		strcpy(curr->path, temp);

		/* 查询模块长度 */
		if(GetModuleInformation(PROCESS.procHandle, module[i], &info, sizeof info)) {
			curr->size = info.SizeOfImage;
		}

		next->next = curr;
		next = curr;

		modulePush(curr);
	}
}

void peBreakAddress(DWORD addr) {
	peBreakRange(addr, addr);
}

void peBreakRange(DWORD from, DWORD to) {
	bp_s *bp, *next = BP;

	mem_alloc(bp, bp_s, 1);

	bp->addrStart	= from;
	bp->addrEnd		= to;
	bp->rvaStart	= addr2rva(from);
	bp->rvaEnd		= addr2rva(to);
	bp->section = addr2section(from);

	/* 不在 PE 文件内 */
	if((bp->addrStart == bp->rvaStart) || (bp->addrEnd == bp->rvaEnd)) {
		bp->rvaStart = bp->rvaEnd = 0;
	}

	while(next->next != NULL) {
		next = next->next;
	}

	next->next = bp;

	bpPush(bp);
}
