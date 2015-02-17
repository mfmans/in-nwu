/*
	$ Invoke   (C) 2005-2012 MF
	$ package.c, 0.1.1201
*/

#include <stdio.h>
#include <io.h>
#include "base.h"


/* 文件标识 */
#define IDENTIFIER_HEADER		0x1E76
#define IDENTIFIER_FUNCTION		0x9C
#define IDENTIFIER_TYPE			0x9D


/* 关闭结构体对齐 */
#pragma pack(1)

/* 文件头格式 */
typedef struct {
	/* 标识信息 */
	unsigned short int	identifier;
	/* 版本号 */
	unsigned long int	version;

	/* 函数位置 */
	size_t	offsetFunc;
	/* 字符串位置 */
	size_t	offsetString;
} ivh_header_s;

/* 函数格式 */
typedef struct {
	/* 标识信息 */
	unsigned char identifier;

	/* 模块名 */
	size_t	nameModule;
	/* 函数名 */
	size_t	nameFunction;
	/* 导入序号 */
	unsigned long int ordinal;

	/* 调用规范 */
	unsigned char invoke;
	/* 可变参数 */
	unsigned char argVar;
	/* 参数数量 */
	unsigned short int argCount;
} ivh_function_s;

/* 类型格式 */
typedef struct {
	/* 标识信息 */
	unsigned char identifier;

	/* 名称 */
	size_t name;

	/* 类型编号 */
	unsigned char typekey;
	/* 指针级别 */
	unsigned char pointer;
	/* 数组索引 */
	size_t arrindex;
} ivh_type_s;

/* 恢复结构体对齐 */
#pragma pack()


static size_t writeString(FILE *fp, char *str, BOOL incase) {
	int		i, len;
	char	*buffer;

	len = strlen(str);

	NEW(buffer, char, len + 1);

	for(i = 0; i < len; i++) {
		if(incase && ((str[i] >= 'A') && (str[i] <= 'Z'))) {
			buffer[i] = str[i] | 0x20;
		} else {
			buffer[i] = str[i];
		}
	}

	fwrite(buffer, len + 1, 1, fp);
	fflush(fp);

	DEL(buffer);

	return (size_t) len + 1;
}

static void writeType(FILE *fp, size_t *offset, size_t fix, char *name, type_s *type) {
	size_t pos;
	ivh_type_s dat;

	/* 保存名字 */
	if(name) {
		dat.name = *offset - fix;

		pos = ftell(fp);

		fseek(fp, *offset, SEEK_SET);
		*offset += writeString(fp, name, FALSE);
		fseek(fp, pos, SEEK_SET);
	} else {
		dat.name = 0;
	}

	dat.identifier	= IDENTIFIER_TYPE;
	dat.typekey		= type->type;
	dat.pointer		= type->pointer;
	dat.arrindex	= type->arrindex;

	fwrite(&dat, sizeof dat, 1, fp);
	fflush(fp);
}

static char * readString(FILE *fp, size_t section, size_t offset) {
	size_t	pos;

	char	*str	= NULL;
	size_t	len		= 0;

	pos = ftell(fp);
	fseek(fp, section + offset, SEEK_SET);

	while(!feof(fp)) {
		RENEW(str, char, len + 1);

		if(fread(str + len, sizeof(char), 1, fp)) {
			if(str[len] == 0) {
				break;
			}

			len++;
		} else {
			break;
		}
	}

	if(str[len]) {
		len++;

		RENEW(str, char, len + 1);

		str[len] = 0;
	}

	fseek(fp, pos, SEEK_SET);

	return str;
}

static int readType(ivh_type_s *ivh, type_s *type) {
	if(ivh->identifier != IDENTIFIER_TYPE) {
		return 0;
	}
	if((ivh->typekey > 12) || (ivh->pointer > 4) || (ivh->arrindex > 1024)) {
		return 0;
	}
	
	typeSet(type, (type_e) ivh->typekey, (size_t) ivh->pointer, ivh->arrindex);

	return 1;
}

static int readFunction(FILE *fp, function_s *func, ivh_function_s *ivh, size_t section) {
	unsigned int i = 0;

	ivh_type_s	type;

	type_s		ret;
	hijack_s	*hijlist;
	argument_s	*args, *curr;

	/* 调用规范 */
	if(ivh->invoke > 2) {
		return 0;
	}

	/* 返回类型 */
	if(fread(&type, sizeof type, 1, fp)) {
		if(readType(&type, &ret) == 0) {
			return 0;
		}
	} else {
		return 0;
	}

	/* 分配临时参数表 */
	NEW(args, argument_s, 1);
	curr = args;

	/* 读取参数表 */
	if(ivh->argCount) {
		while(!feof(fp)) {
			NEW(curr->next, argument_s, 1);
			curr = curr->next;

			if(fread(&type, sizeof type, 1, fp) == 0) {
				break;
			}

			/* 没有名字 */
			if(type.name == 0) {
				break;
			}
			/* 类型无效 */
			if(readType(&type, &curr->type) == 0) {
				break;
			}

			/* 读取参数名 */
			curr->name = readString(fp, section, type.name);

			if((++i) >= ivh->argCount) {
				break;
			}
		}
	}

	/* 读取参数不完整 */
	if(i != ivh->argCount) {
		/* 删除所有临时参数 */
		while(args) {
			curr = args->next;

			DEL(args->name);
			DEL(args);

			args = curr;
		}

		return 0;
	}

	if(func->hijack) {
		/* 删除原有参数表 */
		while(func->hijack->argList) {
			curr = func->hijack->argList->next;

			DEL(func->hijack->argList->name);
			DEL(func->hijack->argList);

			func->hijack->argList = curr;
		}
	} else {
		hijlist = HIJ;

		while(hijlist->next) {
			hijlist = hijlist->next;
		}

		/* 分配新的劫持信息 */
		NEW(func->hijack, hijack_s, 1);

		hijlist->next			= func->hijack;

		func->hijack->prev		= hijlist;
		func->hijack->function	= func;
	}

	func->hijack->invoke	= (call_e) ivh->invoke;
	func->hijack->argCount	= i;
	func->hijack->argList	= args->next;

	if(ivh->argVar) {
		func->hijack->vararg = 1;
	} else {
		func->hijack->vararg = 0;
	}

	memcpy(&func->hijack->ret, &ret, sizeof ret);

	return 1;
}


void packageCreate(HWND win) {
	int i;
	char *file, *temp;
	char buffer[1024];

	FILE	*fp;
	size_t	offset = 0, pos, fix;
	size_t	addrMod, addrFunc;

	module_s	*mod	= MOD->next;
	function_s	*func	= NULL;
	hijack_s	*hij	= NULL;
	argument_s	*arg	= NULL;

	size_t sizeString	= 0;

	ivh_header_s	ivhHeader;
	ivh_function_s	ivhFunction;

	if((file = selectFileSave(win, "data.ivh", "请选择保存路径")) == NULL) {
		return;
	}
	if((fp = fopen(file, "wb+")) == NULL) {
		warningFormat(win, "无法打开指定的文件路径。", file); return;
	}

	ZeroMemory(buffer, sizeof buffer);

	/* 计算函数块大小 */
	while(mod) {
		func = mod->function;

		while(func) {
			if(func->hijack) {
				offset += sizeof(ivh_function_s);
				offset += sizeof(ivh_type_s) * (1 + func->hijack->argCount);
			}

			func = func->next;
		}

		mod = mod->next;
	}

	/* 区块对齐 */
	if(offset % 128) {
		offset = ((offset / 128) + 1) * 128;
	}

	/* 创建文件头 */
	ivhHeader.identifier	= IDENTIFIER_HEADER;
	ivhHeader.version		= 0x00010001;
	ivhHeader.offsetFunc	= 128;
	ivhHeader.offsetString	= 128 + offset;
	/* 写入文件头 */
	fwrite(&ivhHeader, sizeof ivhHeader, 1, fp);
	fwrite(buffer, 128 - sizeof ivhHeader, 1, fp);

	/* 预留区块空间 */
	NEW(temp, char, offset);

	fwrite(temp, offset, 1, fp);
	fseek(fp, 128, SEEK_SET);

	DEL(temp);

	offset	+= 128;

	fix		 = offset;
	mod		 = MOD->next;

	while(mod) {
		i = 0;
		func = mod->function;

		/* 判断是否需要保存 */
		while(func) {
			if(func->hijack) {
				i = 1; break;
			}

			func = func->next;
		}

		/* 如果不需要保存 */
		if(i == 0) {
			mod = mod->next; continue;
		}

		/* 保存当前位置以便回溯 */
		pos		= ftell(fp);
		addrMod	= offset;

		fseek(fp, offset, SEEK_SET);
		offset += writeString(fp, mod->modname, TRUE);
		fseek(fp, pos, SEEK_SET);

		func = mod->function;

		while(func) {
			if((hij = func->hijack) == NULL) {
				func = func->next; continue;
			}

			/* 写入函数名 */
			if(func->name) {
				pos			= ftell(fp);
				addrFunc	= offset;

				fseek(fp, offset, SEEK_SET);
				offset += writeString(fp, func->name, FALSE);
				fseek(fp, pos, SEEK_SET);
			} else {
				addrFunc = 0;
			}

			/* 函数结构 */
			ivhFunction.identifier		= IDENTIFIER_FUNCTION;
			ivhFunction.nameFunction	= addrFunc - fix;
			ivhFunction.nameModule		= addrMod - fix;
			ivhFunction.ordinal			= func->ordinal;
			ivhFunction.invoke			= hij->invoke;
			ivhFunction.argVar			= hij->vararg;
			ivhFunction.argCount		= hij->argCount;

			fwrite(&ivhFunction, sizeof ivhFunction, 1, fp);

			/* 写入返回类型 */
			writeType(fp, &offset, fix, NULL, &hij->ret);

			/* 参数表 */
			if(arg = hij->argList) {
				while(arg) {
					writeType(fp, &offset, fix, arg->name, &arg->type);
					arg = arg->next;
				}
			}

			func = func->next;
		}

		mod = mod->next;
	}

	fclose(fp);

	noticeFormat(win, "当前添加的所有劫持函数信息已经保存到了指定的文件中。", file);
}

int packageLoad(HWND win, char *file, BOOL silent) {
	/* 覆盖模式 */
	int mode = -1;
	/* 定位 */
	size_t strsec, count = 0;

	int		run = 1;
	FILE	*fp;
	char	*nameMod, *nameFunc;

	function_s		*function;

	ivh_header_s	ivhHeader;
	ivh_function_s	ivhFunction;

	if(silent) {
		mode = 1;
	}

	if(file == NULL) {
		if((file = selectFileOpen(win, "ivh", "请选择文件")) == NULL) {
			return 0;
		}
	}
	if((fp = fopen(file, "rb")) == NULL) {
		if(!silent) {
			warningFormat(win, "无法打开指定的文件路径。", file);
		}
		
		return 0;
	}

	/* 读文件头 */
	if((fread(&ivhHeader, sizeof ivhHeader, 1, fp) == 0) || (ivhHeader.identifier != IDENTIFIER_HEADER)) {
		if(!silent) {
			warning(win, "在指定的文件中找不到文件头信息。");
		}
		
		return 0;
	}
	/* 版本号 */
	if(ivhHeader.version != 0x00010001) {
		if(!silent) {
			warning(win, "不支持指定的文件对应的版本。");
		}
		
		return 0;
	}

	strsec = ivhHeader.offsetString;

	/* 移动到函数表开始 */
	fseek(fp, ivhHeader.offsetFunc, SEEK_SET);

	while(!feof(fp) && run) {
		/* 读取函数 */
		if(fread(&ivhFunction, sizeof ivhFunction, 1, fp) == 0) {
			break;
		}
		if(ivhFunction.identifier != IDENTIFIER_FUNCTION) {
			break;
		}

		/* 模块名 */
		nameMod = readString(fp, strsec, ivhFunction.nameModule);
		/* 函数名 */
		if(ivhFunction.nameFunction) {
			nameFunc = readString(fp, strsec, ivhFunction.nameFunction);
		} else {
			nameFunc = NULL;
		}

		/* 找到函数 */
		if(function = moduleFind(win, nameMod, nameFunc, ivhFunction.ordinal, &mode)) {
			if(readFunction(fp, function, &ivhFunction, strsec)) {
				count++;
			} else {
				run = 0;
			}
		} else {
			fseek(fp, sizeof(ivh_type_s) * (1 + ivhFunction.argCount), SEEK_CUR);
		}

		DEL(nameMod);
		DEL(nameFunc);
	}

	fclose(fp);

	if(silent) {
		if(count) {
			return 1;
		} else {
			return 0;
		}
	} else {
		if(count) {
			noticeFormat(win, "成功导入了 %ld 个函数。", count);
			return 1;
		} else {
			warning(win, "没有导入任何函数。");
			return 0;
		}
	}
}

void packageAuto() {
	int		check = 0;
	long	handle;

	struct _finddata_t	file;

	if((handle = _findfirst("*.ivh", &file)) == -1) {
		return;
	}

	do {
		if(check == 0) {
			if(!confirm(NULL, "是否自动加载程序目录下所有的 .ivh 文件？")) {
				break;
			}

			check = 1;
		}

		packageLoad(NULL, file.name, TRUE);
	} while(_findnext(handle, &file) == 0);

	_findclose(handle);
}
