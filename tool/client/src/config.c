/*
	$ MF Client   (C) 2005-2013 MF
	$ config.c
*/

#include "base.h"


/* 配置文件 */
#define CONFIG				"client.dat"

/* 文件头标识符 */
#define MAGIC				0xAB89FF00



/* 文件头 */
typedef struct {
	DWORD	magic;					/* 标识符 */
	BYTE	len_username;			/* 用户名长度 */
	BYTE	len_password;			/* 密码长度 */
	BYTE	adapter[6];				/* MAC 地址 */
	BOOL	hidden;
	BOOL	login;
} config_header_t;



/* 账号 */
static char	*username	= NULL;
static char	*password	= NULL;

/* 接入设备 */
static BYTE	*adapter	= NULL;

/* 设置 */
static BOOL	hidden		= FALSE;
static BOOL	login		= FALSE;



/*
	configRead										读取配置信息

	# int											是否成功读取信息
*/
int configRead() {
	int				result = 0;

	FILE			*fp;
	config_header_t	header;

	if(fopen_s(&fp, CONFIG, "rb")) {
		return result;
	}

	if(fread(&header, sizeof header, 1, fp)) {
		if(header.magic == MAGIC) {
			result	= 1;

			hidden	= header.hidden;
			login	= header.login;

			MEMORY_ALLOC	(adapter,	BYTE,				6);
			memcpy			(adapter,	header.adapter,		6);

			do {
				/* 读取用户名 */
				if(header.len_username && (header.len_username < 64)) {
					MEMORY_ALLOC(username, char, header.len_username + 2);

					if(fread(username, sizeof(char), header.len_username, fp) != header.len_username) {
						MEMORY_FREE(username);

						break;
					}
				}

				/* 读取密码 */
				if(header.len_password && (header.len_password < 64)) {
					MEMORY_ALLOC(password, char, header.len_password + 2);

					if(fread(password, sizeof(char), header.len_password, fp) != header.len_password) {
						MEMORY_FREE(password);

						break;
					}
				}
			} while(0);
		}
	}

	fclose(fp);

	return result;
}

/*
	configWrite										写入配置信息
*/
void configWrite() {
	FILE			*fp;
	config_header_t	header;

	ZeroMemory(&header, sizeof header);

	header.magic		= MAGIC;
	header.hidden		= hidden;
	header.login		= login;

	if(adapter) {
		memcpy_s	(header.adapter, sizeof header.adapter, adapter, 6);
	}  else {
		ZeroMemory	(header.adapter, sizeof header.adapter);
	}

	/* 用户名密码长度 */
	if(username) {
		header.len_username	= strnlen_s(username, 0xFF);
	}
	if(password) {
		header.len_password = strnlen_s(password, 0xFF);
	}

	if(fopen_s(&fp, CONFIG, "wb")) {
		return;
	}

	fwrite(&header, sizeof header, 1, fp);

	/* 写入用户名密码 */
	if(header.len_username) {
		fwrite(username, sizeof(char), header.len_username, fp);
	}
	if(header.len_password) {
		fwrite(password, sizeof(char), header.len_password, fp);
	}

	fclose(fp);
}

/*
	configClear										删除配置信息
*/
void configClear() {
	_unlink(CONFIG);
}


/*
	configGetUsername								返回配置项
	configGetPassword
	configGetAdapter
	configGetHidden
	configGetLogin
*/
char *	configGetUsername	() { return username;	}
char *	configGetPassword	() { return password;	}
BYTE *	configGetAdapter	() { return adapter;	}
BOOL	configGetHidden		() { return hidden;		}
BOOL	configGetLogin		() { return login;		}


/*
	configSetUsername								设置配置项
	configSetPassword
	configSetAdapter
	configSetHidden
	configSetLogin
*/
void	configSetUsername	(char	*value)	{ if(username)	{ MEMORY_FREE (username);	}	username	= value; }
void	configSetPassword	(char	*value)	{ if(password)	{ MEMORY_FREE (password);	}	password	= value; }
void	configSetAdapter	(BYTE	*value)	{ if(adapter)	{ MEMORY_FREE (adapter);	}	adapter		= value; }
void	configSetHidden		(BOOL	value)	{ hidden	= value; }
void	configSetLogin		(BOOL	value)	{ login		= value; }
