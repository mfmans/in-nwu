/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ code.h, 0.1.1005
*/

#ifndef _CODE_H
#define _CODE_H_

/*
	$ rc4						RC4 编解码

	@ char *source				源数据
	@ const seek_t length		源数据长度
	@ char *key					密钥

	# char *					处理后的数据
*/
char *rc4(char *source, const seek_t length, char *key);


/*
	$ crc32						CRC32 运算

	@ char *source				数据
	@ seek_t length				数据长度

	# crc32_t					哈希值
*/
crc32_t crc32(char *source, seek_t length);


/*
	$ md5						MD5 运算

	@ char *source				MD5 运算后, source 将会被 free() 掉

	# char *
*/
char *md5(char *source);


#endif
