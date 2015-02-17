/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ pe.c, 0.1.1104
*/

#define _SNP_PE_

#include <Windows.h>
#include "base.h"
#include "image.h"
#include "gui.h"


/* 文件句柄 */
static FILE *fp;


char * peHeader(BOOL refresh) {
	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS pe;

	ZeroMemory(&dos, sizeof dos);
	ZeroMemory(&pe, sizeof pe);

	pfSeek(0, SEEK_SET);

	/* 找 DOS 头 */
	if((pfRead(&dos, sizeof dos) == FALSE) || (dos.e_magic != IMAGE_DOS_SIGNATURE)) {
		return "在所选的文件中未找到 DOS 头，它可能不是一个正确的 PE 文件。";
	}

	/* 移动到 PE 头 */
	pfSeek((long) dos.e_lfanew, SEEK_SET);

	/* 找 PE 头 */
	if((pfRead(&pe, sizeof pe) == FALSE) || (pe.Signature != IMAGE_NT_SIGNATURE)) {
		return "在所选的文件中未找到 PE 头，它可能不是一个正确的 PE 文件。";
	}

	/* 判断是不是 PE+ */
	switch(pe.OptionalHeader.Magic) {
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			return "程序不支持加载 64 位编译的 PE 文件。";

		default:
			return "无法识别文件的类型。";
	}

	/* 处理器类型 */
	switch(pe.FileHeader.Machine) {
		case IMAGE_FILE_MACHINE_I386:
			break;

		default:
			return "无法识别运行此 PE 文件所需要的处理器类型。";
	}

	/* 判断是不是 DLL */
	if((pe.FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) {
		return "不能运行 DLL 文件。";
	} else if((pe.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != IMAGE_FILE_EXECUTABLE_IMAGE) {
		return "无法识别文件的标记。";
	}

	/* 子系统 */
	switch(pe.OptionalHeader.Subsystem) {
		case IMAGE_SUBSYSTEM_WINDOWS_GUI: RUNTIME.pe.console = 0; break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI: RUNTIME.pe.console = 1; break;

		default:
			return "无法识别运行此 PE 文件所需要的子系统。";
	}

	RUNTIME.pe.entry	= pe.OptionalHeader.AddressOfEntryPoint;
	RUNTIME.pe.import	= pe.OptionalHeader.DataDirectory[1].VirtualAddress;
	RUNTIME.pe.section	= pfTell();

	if(refresh == FALSE) {
		RUNTIME.pe.base = pe.OptionalHeader.ImageBase;
	}

	return NULL;
}

char * peSection() {
	section_s				*section, *next = SECTION;
	IMAGE_SECTION_HEADER	header;

	pfSeek(RUNTIME.pe.section, SEEK_SET);

	while(next->next != NULL) {
		next = next->next;
	}

	while(!pfEnd()) {
		if(pfRead(&header, sizeof header) == 0) {
			break;
		}
		if(header.VirtualAddress == 0) {
			break;
		}

		mem_alloc(section, section_s, 1);

		section->name		= readFromBuffer((char *) header.Name, sizeof header.Name);
		section->addr		= 0;
		section->va			= header.VirtualAddress;
		section->offset		= header.PointerToRawData;
		section->sizeImage	= header.Misc.VirtualSize;
		section->sizeRaw	= header.SizeOfRawData;
		section->dynamic	= FALSE;
		section->file		= TRUE;
		section->copy		= NULL;
		section->index		= -1;

		next->next = section;
		next = section;
	}

	if(SECTION->next == NULL) {
		return "没有在 PE 文件中找到任何区段。";
	} else {
		return NULL;
	}
}

static void peImportFunction(IMAGE_IMPORT_DESCRIPTOR *info) {
	int		i;
	long	offset;
	char	*dll, *function;

	import_s			*import, *next = IMPORT;
	IMAGE_THUNK_DATA	thunk;

	/* 转发类型的跳过 */
	if(info->ForwarderChain != 0) {
		return;
	}
	
	/* 找 DLL 名称 */
	pfSeek(pfOffset(info->Name), SEEK_SET);

	/* 不允许匿名 */
	if((dll = readFromFile()) == NULL) {
		return;
	} else {
		/* 将 dll 转换为小写 */
		strtolower(dll);
	}

	/* 找链尾 */
	while(next->next != NULL) {
		next = next->next;
	}

	/* 移动到 INT 入口 */
	if(info->OriginalFirstThunk) {
		pfSeek(pfOffset(info->OriginalFirstThunk), SEEK_SET);
	} else if(info->FirstThunk) {
		pfSeek(pfOffset(info->FirstThunk), SEEK_SET);
	} else {
		return;
	}

	/* 函数计数器, 用于定位 INT */
	i = 0;

	while(!pfEnd()) {
		if(pfRead(&thunk, sizeof thunk) == FALSE) {
			break;
		}
		if(isBlank(&thunk, sizeof thunk)) {
			break;
		}

		mem_alloc(import, import_s, 1);

		/* 保留当前 INT 位置 */
		offset = pfTell();

		/* 根据序号导入 */
		if(thunk.u1.Function & 0x80000000) {
			import->ordinal = thunk.u1.Ordinal & 0x7FFFFFFF;
		} else {
			/* 函数名位置 */
			pfSeek(pfOffset(thunk.u1.AddressOfData) + sizeof(WORD), SEEK_SET);

			if((function = readFromFile()) == NULL) {
				import->function = "?";
			} else {
				import->function = function;
			}

			pfSeek(offset, SEEK_SET);
		}

		import->point		= 0;
		import->entry		= 0;

		import->dll			= dll;
		import->iatVa		= info->FirstThunk + i * sizeof thunk;
		import->iatOffset	= offset - sizeof thunk;
		import->iatData		= thunk.u1.AddressOfData;
		import->index		= -1;

		next->next = import;
		next = import;

		i++;
	}
}

void peImport() {
	long offset;
	IMAGE_IMPORT_DESCRIPTOR	import;

	if(RUNTIME.pe.import) {
		offset = (long) pfOffset(RUNTIME.pe.import);
		pfSeek(offset, SEEK_SET);
	} else {
		return;
	}

	while(!pfEnd()) {
		if(pfRead(&import, sizeof import) == FALSE) {
			break;
		}
		if(isBlank(&import, sizeof import)) {
			break;
		}

		/* 保存当前位置 */
		offset = pfTell();

		peImportFunction(&import);

		/* 回溯 */
		pfSeek(offset, SEEK_SET);
	}
}


BOOL dataReadFile(void *buffer, size_t size) {
	if(fread(buffer, size, 1, fp) == 1) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void dataSeekFile(long offset, int mode) {
	fseek(fp, offset, mode);
}

long dataTellFile() {
	return ftell(fp);
}

DWORD dataOffsetFile(DWORD va) {
	return va2offset(va);
}

BOOL dataEndFile() {
	if(feof(fp)) {
		return TRUE;
	} else {
		return FALSE;
	}
}


BOOL pe() {
	char *error;

	/* 操作指针 */
	pfRead		= dataReadFile;
	pfSeek		= dataSeekFile;
	pfTell		= dataTellFile;
	pfOffset	= dataOffsetFile;
	pfEnd		= dataEndFile;

	WORKING_SET("正在打开文件“%s” ...\r\n\r\n", RUNTIME.file);

	if((fp = fopen(RUNTIME.file, "rb")) == NULL) {
		WORKING_APPEND_ALERT("无法打开文件“%s”，请检查文件是否存在或可读。", RUNTIME.file);
		return FALSE;
	}

	/* 解析 PE 头 */
	if((error = peHeader(FALSE)) != NULL) {
		WORKING_APPEND_ALERT(error);
		return FALSE;
	}

	/* 分析区段 */
	if((error = peSection()) != NULL) {
		WORKING_APPEND_ALERT(error);
		return FALSE;
	}

	/* 导入表 */
	peImport();

	/* 设置 filename */
	if(RUNTIME.filename = strrchr(RUNTIME.file, '\\')) {
		RUNTIME.filename++;
	} else {
		RUNTIME.filename = RUNTIME.file;
	}

	return TRUE;
}


void dump() {

}


DWORD va2rva(DWORD va) {
	if(CONFIG.modBase) {
		return RUNTIME.proc.base + va;
	} else {
		return RUNTIME.pe.base + va;
	}
}

DWORD va2addr(DWORD va) {
	section_s *section = getSectionByVa(va);

	if(section == NULL) {
		return va;
	}
	if(va >= (section->va + section->sizeImage)) {
		return 0;
	}

	return (va - section->va) + section->addr;
}

DWORD va2offset(DWORD va) {
	section_s *section = getSectionByVa(va);

	if(section == NULL) {
		return va;
	}
	if(va >= (section->va + section->sizeRaw)) {
		return 0;
	}

	return (va - section->va) + section->offset;
}

DWORD rva2va(DWORD rva) {
	DWORD base;

	if(CONFIG.modBase) {
		base = RUNTIME.proc.base;
	} else {
		base = RUNTIME.pe.base;
	}

	if(base > rva) {
		return 0;
	} else {
		return rva - base;
	}
}

DWORD rva2addr(DWORD rva) {
	DWORD va = rva2va(rva);

	if(va == 0) {
		return 0;
	} else {
		return va2addr(va);
	}
}

DWORD addr2va(DWORD addr) {
	section_s *section = getSectionByAddr(addr);

	if(section == NULL) {
		return 0;
	}

	return (addr - section->addr) + section->va;
}

DWORD addr2rva(DWORD addr) {
	DWORD va = addr2va(addr);

	if(va == 0) {
		return 0;
	} else {
		return va2rva(va);
	}
}


section_s * getSectionByVa(DWORD va) {
	section_s *curr = NULL;
	section_s *next = SECTION->next;

	while(next) {
		if(va >= next->va) {
			curr = next;
			next = next->next;
		} else {
			break;
		}
	}

	if(curr == NULL) {
		return NULL;
	}
	if(va >= (curr->va + curr->sizeImage)) {
		return NULL;
	}

	return curr;
}

section_s * getSectionByAddr(DWORD addr) {
	section_s *curr = NULL;
	section_s *next = SECTION->next;

	while(next) {
		if(addr >= next->addr) {
			curr = next;
			next = next->next;
		} else {
			break;
		}
	}

	if(curr == NULL) {
		return NULL;
	}
	if(addr >= (curr->addr + curr->sizeImage)) {
		return NULL;
	}

	return curr;
}

import_s * getImportByAddr(DWORD addr) {
	import_s *import = IMPORT->next;

	while(import) {
		if((import->entry == addr) || (import->point == addr) || (import->hook)) {
			return import;
		}

		import = import->next;
	}

	return NULL;
}

module_s * getModuleByAddr(DWORD addr) {
	module_s *module = MODULE->next;

	while(module) {
		if((addr >= module->address) && (addr < (module->address + module->size))) {
			return module;
		}

		module = module->next;
	}

	return NULL;
}
