/*
	$ Invoke   (C) 2005-2012 MF
	$ type.c, 0.1.1129
*/

#include "base.h"


/* 可供选择的类型 */
typedef struct type_list_t {
	char	*name;
	type_s	type;
} type_list_t;

/* 类型表 */
static type_list_t typelist[] = {
	{"void",							{VOID_0,	0, 0, 0}},
	{"signed short int  [16bit]",		{SG_INT_16,	0, 0, 2}},
	{"unsigned short int  [16bit]",		{US_INT_16,	0, 0, 2}},
	{"signed int  [32bit]",				{SG_INT_32,	0, 0, 4}},
	{"unsigned int  [32bit]",			{US_INT_32,	0, 0, 4}},
	{"signed long long int  [64bit]",	{SG_INT_64,	0, 0, 8}},
	{"unsigned long long int  [64bit]",	{US_INT_64,	0, 0, 8}},
	{"float  [32bit]",					{REAL_32,	0, 0, 4}},
	{"double  [64bit]",					{REAL_64,	0, 0, 8}},
	{"signed char",						{SG_CHAR,	0, 0, 1}},
	{"unsigned char",					{US_CHAR,	0, 0, 1}},
	{"WCHAR",							{WDCHAR,	0, 0, 2}},
	{"BYTE",							{BYTE_8,	0, 0, 1}},
	{"void *",							{VOID_0,	1, 0, 4}},
	{"void **",							{VOID_0,	2, 0, 4}},
	{"signed short int *",				{SG_INT_16,	1, 0, 4}},
	{"signed short int **",				{SG_INT_16,	2, 0, 4}},
	{"unsigned short int *",			{US_INT_16,	1, 0, 4}},
	{"unsigned short int **",			{US_INT_16,	2, 0, 4}},
	{"signed int *",					{SG_INT_32,	1, 0, 4}},
	{"signed int **",					{SG_INT_32,	2, 0, 4}},
	{"unsigned int *",					{US_INT_32,	1, 0, 4}},
	{"unsigned int **",					{US_INT_32,	2, 0, 4}},
	{"signed long long int *",			{SG_INT_64,	1, 0, 4}},
	{"signed long long int **",			{SG_INT_64,	2, 0, 4}},
	{"unsigned long long int *",		{US_INT_64,	1, 0, 4}},
	{"unsigned long long int **",		{US_INT_64,	2, 0, 4}},
	{"float *",							{REAL_32,	1, 0, 4}},
	{"float **",						{REAL_32,	2, 0, 4}},
	{"double *",						{REAL_64,	1, 0, 4}},
	{"double **",						{REAL_64,	2, 0, 4}},
	{"signed char *",					{SG_CHAR,	1, 0, 4}},
	{"signed char **",					{SG_CHAR,	2, 0, 4}},
	{"unsigned char *",					{US_CHAR,	1, 0, 4}},
	{"unsigned char **",				{US_CHAR,	2, 0, 4}},
	{"WCHAR *",							{WDCHAR,	1, 0, 4}},
	{"WCHAR **",						{WDCHAR,	2, 0, 4}},
	{"BYTE *",							{BYTE_8,	1, 0, 4}},
	{"BYTE **",							{BYTE_8,	2, 0, 4}},
};


#define COUNT	(sizeof typelist / sizeof(type_list_t))

/* 字符可读 */
#define IS_READABLE(chr)	(((chr) >= 0x20) && ((chr) <= 0x7E))


static DWORD ptrRead(DWORD addr) {
	DWORD data;

	if(processRead(addr, &data, sizeof data) == 0) {
		return 0;
	}

	return data;
}

static DWORD ptrReturn(DWORD addr, size_t lvFrom, size_t lvTo) {
	for(; lvFrom > lvTo; lvFrom--) {
		addr = ptrRead(addr);
	}

	return addr;
}

static char * ptrReturnRoute(DWORD *addr, size_t lvFrom, size_t lvTo, type_e typekey) {
	char	*tname;
	char	*buffer	= NULL;

	size_t	length	= 0;
	DWORD	address	= *addr;

	type_s	type;

	/* 初始化类型 */
	type.type		= typekey;
	type.arrindex	= 0;
	type.size		= 0;

	for(; lvFrom > lvTo; lvFrom--) {
		/* 指针地址 */
		*addr = ptrRead(address);

		/* 类型名 */
		type.pointer = lvFrom;
		tname = type2string(&type);

		RENEW(buffer, char, length + strlen(tname) + 32);
		buffer[length] = 0;

		sprintf(buffer + length, "%08X => %08X  [%s]\r\n", address, *addr, tname);

		DEL(tname);

		length = strlen(buffer);

		address = *addr;
	}

	return buffer;
}


char * strReadAnsi(DWORD addr, size_t max) {
	char	*buffer = NULL;
	size_t	i;

	for(i = 0; i < max; i++) {
		RENEW(buffer, char, i + 1);
		processRead(addr + i, buffer + i, 1);

		if(buffer[i] == 0) {
			return buffer;
		}
	}

	RENEW(buffer, char, i + 1);
	buffer[i] = 0;

	return buffer;
}

wchar_t * strReadUnicode(DWORD addr, size_t max) {
	size_t	i, j;
	wchar_t	*buffer = NULL;

	for(i = j = 0; i < max; i++, j += sizeof(wchar_t)) {
		RENEW(buffer, wchar_t, j + sizeof(wchar_t));

		processRead(addr + j, buffer + i, sizeof(wchar_t));

		if(buffer[i] == 0) {
			return buffer;
		}
	}

	RENEW(buffer, wchar_t, j + sizeof(wchar_t));
	buffer[i] = 0;

	return buffer;
}


static char * readSimpleVoid(DWORD address, type_s *type, char *buffer) {
	return NULL;
}

static char * readSimpleSgChar(DWORD address, type_s *type, char *buffer) {
	char	chr;
	DWORD	addr;

	if(type->pointer) {
		addr = ptrReturn(address, type->pointer + 1, 1);

		/* 读取字符串 */
		return strReadAnsi(addr, 128);
	} else {
		/* 读取字符 */
		processRead(address, &chr, sizeof chr);

		if(IS_READABLE(chr)) {
			buffer[0] = chr;
		} else {
			sprintf(buffer, "\\x%02X  (%d)", (unsigned int) chr, (int) chr);
		}

		return buffer;
	}
}

static char * readSimpleSgInt16(DWORD address, type_s *type, char *buffer) {
	INT16 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%d  (0x%04X)", data, (UINT32) data);

	return buffer;
}

static char * readSimpleSgInt32(DWORD address, type_s *type, char *buffer) {
	INT32 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%ld  (0x%08X)", data, (UINT32) data);

	return buffer;
}

static char * readSimpleSgInt64(DWORD address, type_s *type, char *buffer) {
	INT64 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%I64d  (0x%08X%08X)", data, (UINT32) (data >> 32), (UINT32) data);

	return buffer;
}

static char * readSimpleUsChar(DWORD address, type_s *type, char *buffer) {
	char	chr;
	DWORD	addr;

	if(type->pointer) {
		addr = ptrReturn(address, type->pointer + 1, 1);

		/* 读取字符串 */
		return strReadAnsi(addr, 128);
	} else {
		/* 读取字符 */
		processRead(address, &chr, sizeof chr);

		if(IS_READABLE(chr)) {
			buffer[0] = chr;
		} else {
			sprintf(buffer, "\\x%02X  (%d)", (unsigned int) chr, (unsigned int) chr);
		}

		return buffer;
	}
}

static char * readSimpleUsInt16(DWORD address, type_s *type, char *buffer) {
	UINT16 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%u  (0x%04X)", data, (UINT32) data);

	return buffer;
}

static char * readSimpleUsInt32(DWORD address, type_s *type, char *buffer) {
	UINT32 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%lu  (0x%08X)", data, (UINT32) data);

	return buffer;
}

static char * readSimpleUsInt64(DWORD address, type_s *type, char *buffer) {
	UINT64 data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%I64u  (0x%08X%08X)", data, (UINT32) (data >> 32), (UINT32) data);

	return buffer;
}

static char * readSimpleWchar(DWORD address, type_s *type, char *buffer) {
	wchar_t	wchr;
	DWORD	addr;

	if(type->pointer) {
		addr = ptrReturn(address, type->pointer + 1, 1);

		/* 读取字符串 */
		return wchar2char(strReadUnicode(addr, 256));
	} else {
		/* 读取字符 */
		processRead(address, &wchr, sizeof wchr);

		sprintf(buffer, "\\x%04X  (%d)", (unsigned int) wchr, (unsigned int) wchr);

		return buffer;
	}
}

static char * readSimpleByte(DWORD address, type_s *type, char *buffer) {
	unsigned char data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%02X", (unsigned int) data);

	return buffer;
}

static char * readSimpleReal32(DWORD address, type_s *type, char *buffer) {
	float data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%f", data);

	return buffer;
}

static char * readSimpleReal64(DWORD address, type_s *type, char *buffer) {
	double data;
	address = ptrReturn(address, type->pointer, 0);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%lf", data);

	return buffer;
}


static char * readDetailVoid(DWORD address, type_s *type, char *buffer, char **pointer) {
	/* 指针 */
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	sprintf(buffer, "%%lu = %lu\r\n%%ld = %ld\r\n%%08X = %08X", address, address, address);

	return buffer;
}

static char * readDetailChar(DWORD address, type_s *type, char *buffer, char **pointer) {
	char chr;

	if(type->pointer) {
		*pointer = ptrReturnRoute(&address, type->pointer + 1, 1, type->type);

		/* 读取字符串 */
		return strReadAnsi(address, 16384);
	} else {
		/* 读取字符 */
		processRead(address, &chr, sizeof chr);

		if(IS_READABLE(chr)) {
			buffer[0] = chr;
		} else {
			sprintf(buffer, "%%02X = %02X\r\n%%u = %u\r\n%%d = %d\r\n%%c = %c", (unsigned int) chr, (unsigned int) chr, (int) chr, (int) chr);
		}

		return buffer;
	}
}

static char * readDetailInt16(DWORD address, type_s *type, char *buffer, char **pointer) {
	INT16 data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%u = %u\r\n%%d = %d\r\n%%04X = %04X", (UINT16) data, data, (UINT32) data);

	return buffer;
}

static char * readDetailInt32(DWORD address, type_s *type, char *buffer, char **pointer) {
	INT32 data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%lu = %lu\r\n%%ld = %ld\r\n%%08X = %08X", (UINT32) data, data, (UINT32) data);

	return buffer;
}

static char * readDetailInt64(DWORD address, type_s *type, char *buffer, char **pointer) {
	INT64 data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%llu = %I64u\r\n%%lld = %I64d\r\n%%X = %08X%08X", (UINT64) data, data, (UINT32) (data >> 32), (UINT32) data);

	return buffer;
}

static char * readDetailWchar(DWORD address, type_s *type, char *buffer, char **pointer) {
	wchar_t wchr[2] = {0};

	if(type->pointer) {
		*pointer = ptrReturnRoute(&address, type->pointer + 1, 1, type->type);

		/* 读取字符串 */
		return wchar2char(strReadUnicode(address, 65536));
	} else {
		/* 读取字符 */
		processRead(address, wchr, sizeof(wchar_t));

		sprintf(buffer, "%%04X = %04X\r\n%%u = %u\r\n%%d = %d\r\n%%s = %s", (unsigned int) wchr, (unsigned int) wchr, (int) wchr, wchr);

		return buffer;
	}
}

static char * readDetailByte(DWORD address, type_s *type, char *buffer, char **pointer) {
	unsigned char data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%02X = %02X", (unsigned int) data);

	return buffer;
}

static char * readDetailReal32(DWORD address, type_s *type, char *buffer, char **pointer) {
	float data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%f = %f", data);

	return buffer;
}

static char * readDetailReal64(DWORD address, type_s *type, char *buffer, char **pointer) {
	double data;
	*pointer = ptrReturnRoute(&address, type->pointer, 0, type->type);

	processRead(address, &data, sizeof data);
	sprintf(buffer, "%%lf = %lf", data);

	return buffer;
}


char * typeReadSimple(DWORD addr, type_s *type) {
	char buffer[128] = {0};

	char *result = NULL;
	char * (*handle)(DWORD, type_s *, char *) = NULL;

	type_s types;

	memcpy(&types, type, sizeof(type_s));

	switch(type->type) {
		case VOID_0:		handle = readSimpleVoid;	break;
		case SG_CHAR:		handle = readSimpleSgChar;	break;
		case SG_INT_16:		handle = readSimpleSgInt16;	break;
		case SG_INT_32:		handle = readSimpleSgInt32;	break;
		case SG_INT_64:		handle = readSimpleSgInt64;	break;
		case US_CHAR:		handle = readSimpleUsChar;	break;
		case US_INT_16:		handle = readSimpleUsInt16;	break;
		case US_INT_32:		handle = readSimpleUsInt32;	break;
		case US_INT_64:		handle = readSimpleUsInt64;	break;
		case WDCHAR:		handle = readSimpleWchar;	break;
		case BYTE_8:		handle = readSimpleByte;	break;
		case REAL_32:		handle = readSimpleReal32;	break;
		case REAL_64:		handle = readSimpleReal64;	break;
	}

	if(handle) {
		result = handle(addr, type, buffer);
	}

	if(result) {
		if(result == buffer) {
			NEW(result, char, strlen(buffer) + 1);

			strcpy(result, buffer);
		}
	} else {
		NEW(result, char, 1);
	}

	return result;
}

char * typeReadDetail(DWORD addr, type_s *type, char **pointer) {
	char buffer[128] = {0};

	char *result = NULL;
	char * (*handle)(DWORD, type_s *, char *, char **) = NULL;

	type_s types;

	memcpy(&types, type, sizeof(type_s));

	*pointer = NULL;

	switch(type->type) {
		case VOID_0:
			handle = readDetailVoid;	break;

		case SG_CHAR:
		case US_CHAR:
			handle = readDetailChar;	break;

		case SG_INT_16:
		case US_INT_16:
			handle = readDetailInt16;	break;

		case SG_INT_32:
		case US_INT_32:
			handle = readDetailInt32;	break;

		case SG_INT_64:
		case US_INT_64:
			handle = readDetailInt64;	break;

		case WDCHAR:
			handle = readDetailWchar;	break;
		case BYTE_8:
			handle = readDetailByte;	break;
		case REAL_32:
			handle = readDetailReal32;	break;
		case REAL_64:
			handle = readDetailReal64;	break;
	}

	if(handle) {
		result = handle(addr, type, buffer, pointer);
	}

	if(result) {
		if(result == buffer) {
			NEW(result, char, strlen(buffer) + 1);

			strcpy(result, buffer);
		}
	} else {
		NEW(result, char, 1);
	}

	return result;
}

void typeSet(type_s *p, type_e type, size_t pointer, size_t arrindex) {
	p->type		= type;
	p->pointer	= pointer;
	p->arrindex	= arrindex;

	if(p->pointer || p->arrindex) {
		p->size = 4;
	} else {
		switch(type) {
			case VOID_0:
				p->size = 0; break;

			case SG_CHAR:
			case US_CHAR:
			case BYTE_8:
				p->size = 1; break;

			case SG_INT_16:
			case US_INT_16:
			case WDCHAR:
				p->size = 2; break;

			case SG_INT_32:
			case US_INT_32:
			case REAL_32:
				p->size = 4; break;

			case SG_INT_64:
			case US_INT_64:
			case REAL_64:
				p->size = 8; break;
		}
	}
}

char * type2string(type_s *type) {
	size_t	i;
	char	*output;

	NEW(output, char, type->pointer + 32);

	switch(type->type) {
		case VOID_0:	strcpy(output, "void");						break;
		case SG_CHAR:	strcpy(output, "signed char");				break;
		case SG_INT_16:	strcpy(output, "signed short int");			break;
		case SG_INT_32:	strcpy(output, "signed int");				break;
		case SG_INT_64:	strcpy(output, "signed long long int");		break;
		case US_CHAR:	strcpy(output, "unsigned char");			break;
		case US_INT_16:	strcpy(output, "unsigned short int");		break;
		case US_INT_32:	strcpy(output, "unsigned int");				break;
		case US_INT_64:	strcpy(output, "unsigned long long int");	break;
		case WDCHAR:	strcpy(output, "WCHAR");					break;
		case BYTE_8:	strcpy(output, "BYTE");						break;
		case REAL_32:	strcpy(output, "float");					break;
		case REAL_64:	strcpy(output, "double");					break;
	}

	if(type->pointer) {
		strcat(output, " ");

		for(i = 0; i < type->pointer; i++) {
			strcat(output, "*");
		}
	}

	if(type->arrindex) {
		strcat(output, " ");
		sprintf(output + strlen(output), "[%ld]", type->arrindex);
	}

	return output;
}


void typeComboInit(erp_object_combo_s *obj, type_s *type, BOOL allowVoid) {
	int i, j;
	
	if(allowVoid) {
		i = 0;
	} else {
		i = 1;
	}

	for(j = -1; i < COUNT; i++) {
		COMBO(obj)->append(typelist[i].name);
		
		if(type->type == typelist[i].type.type) {
			if(type->arrindex) {
				if((type->pointer + 1) == typelist[i].type.pointer) {
					j = i;
				}
			} else if(type->pointer == typelist[i].type.pointer) {
				j = i;
			}
		}
	}

	if(j == -1) {
		COMBO(obj)->append("其它类型");

		if(allowVoid) {
			COMBO(obj)->select(COUNT);
		} else {
			COMBO(obj)->select(COUNT - 1);
		}
	} else {
		COMBO(obj)->select(j);
	}
}

int typeComboChange(int index, erp_object_combo_s *obj, type_s *type, BOOL allowVoid) {
	/* 修正索引 */
	if(allowVoid == FALSE) {
		index++;
	}
	/* 选择非法类型 */
	if(index >= COUNT) {
		return 0;
	}

	memcpy(type, &typelist[index].type, sizeof(type_s));

	return 1;
}


char * typeArgument(argument_s *arg, BOOL vararg) {
	char	*type;
	char	*list = NULL;

	size_t	length = 0, offset = 0;

	if(arg == NULL) {
		NEW(list, char, 8);

		strcat(list, "void");

		return list;
	}

	while(arg) {
		type = type2string(&arg->type);

		/* 扩展字符串长度 */
		offset = strlen(arg->name) + strlen(type) + 8;

		if(list) {
			RENEW(list, char, length + offset);

			list[length] = 0;
		} else {
			NEW(list, char, offset);
		}

		strcat(list, arg->name);
		strcat(list, " (");
		strcat(list, type);
		strcat(list, ")");

		DEL(type);

		if(arg = arg->next) {
			strcat(list, ", ");
		}

		length += offset;
	}

	if(vararg) {
		RENEW(list, char, length + 8);

		strcat(list, ", ...");
	}

	return list;
}
