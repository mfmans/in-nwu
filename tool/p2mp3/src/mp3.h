/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ mp3.h, 0.1.1005
*/

#ifndef _MP3_H_
#define _MP3_H_

/* 提取 v 的第 p 位开始连续的第 N 位 */
#define BR1(v, p) (((v) >> (p)) & 0x01)
#define BR2(v, p) (((v) >> (p)) & 0x03)
#define BR3(v, p) (((v) >> (p)) & 0x07)
#define BR4(v, p) (((v) >> (p)) & 0x0F)
/* 设置 v 的第 p 位为 s */
#define BS(v, p, s) v = v & ~(0x01 << (p)) | ((s) << (p))
/* 校验 v */
#define BP(v) ( \
		(((v) & 0x01) >> 0) ^ (((v) & 0x02) >> 1) ^ \
		(((v) & 0x04) >> 2) ^ (((v) & 0x08) >> 3) ^ \
		(((v) & 0x10) >> 4) ^ (((v) & 0x20) >> 5) ^ \
		(((v) & 0x40) >> 6) ^ (((v) & 0x80) >> 7) \
	)


/* 结构体对齐会出问题的 */
#pragma pack(1)


/* MP3 文件信息 */
typedef struct {
	count_t count;				/* 文件中可识别的总数据帧数目 */
	seek_t position;			/* 第 1 帧数据帧的位置 */
} mp3_s;

/* id3v1 */
typedef struct {
	var_t header[3];			/* 固定值 "TAG" */
	var_t data[125];			/* 剩余的 125 字节 */
} id3v1_s;

/* id3v2 头 */
typedef struct {
	var_t header[3];			/* 固定值 "ID3" */
	var_t version;				/* 主版本 */
	var_t revision;				/* 子版本 */
	var_t flag;					/* 标识符 */
	var_t size[4];				/* 标签长度 */
} id3v2_s;

/* 数据帧帧头 */
typedef struct {
	var_t version;				/* [v1, 3-4] MPEG 版本 */
	var_t layer;				/* [v1, 5-6] MPEG Layer */
	var_t crc;					/* [v1, 7]   CRC 校检 */
	var_t bitrate;				/* [v2, 0-3] 比特率 */
	var_t frequency;			/* [v2, 4-5] 采样频率 */
	var_t padding;				/* [v2, 6]   帧长调节 */
} frame_s;

/* 合并信息头 */
typedef struct {
	var_t id;					/* 固定值 0xA9 */
	seek_t size;				/* 保存数据的尺寸 */
	crc32_t crcraw;				/* 原始数据 CRC32 */
	crc32_t crcdat;				/* 保存数据 CRC32 */
	char ext[5];				/* 文件扩展名 */
} header_s;

/* 扩展信息表 */
typedef struct {
	var_t before[2];			/* 固定值 0x395E */
	seek_t start;				/* 扩展数据开始位置 */
	var_t after;				/* 固定值 0x9B */
} extend_s;


/* 恢复对齐 */
#pragma pack()


/* 判断是不是 ID3v1 标签 */
#define id3v1_check(v) ((fread(&v, sizeof v, 1, fp) == 1) && (ms_compare(v.header, "TAG") == 0))
/* 判断是不是 ID3v2 标签 */
#define id3v2_check(v) (ms_compare(v.header, "ID3") == 0)
/* 计算 ID3v2 所有标签帧长度 */
#define id3v2_length(v) ((((int) (v.size[0] & 0x7F)) << 21) | (((int) (v.size[1] & 0x7F)) << 14) | (((int) (v.size[2] & 0x7F)) << 7) | ((int) (v.size[3] & 0x7F)))

/* 判断是不是数据帧 */
#define frame_check(v) ((v[0] == 0xFF) && ((v[1] & 0xE0) == 0xE0))
/* 数据帧解码 */
#define frame_decode(v, f) \
	f.version	= BR2(v[1], 3); \
	f.layer		= BR2(v[1], 1); \
	f.crc		= BR1(v[1], 0); \
	f.bitrate	= BR4(v[2], 4); \
	f.frequency	= BR2(v[2], 2); \
	f.padding	= BR1(v[2], 1);
/* 计算帧长度 */
#define frame_length(f, len) do { \
		if(tableFrequency[f.version][f.frequency] == 0) break; \
		if((len = tableRatio[f.version][f.layer] * tableBitrates[(f.version << 2) | f.layer][f.bitrate]) == 0) break; \
		len /= tableFrequency[f.version][f.frequency]; \
		len += (f.crc == 0) ? 2 : 0; \
		len += (f.padding == 0) ? 0 : 1; \
	} while(0);
/* 循环中寻找下一帧 */
#define frame_next(v, f, len) \
	if(fread(v, sizeof(unsigned char), 4, fp) < 4) break; \
	if(!frame_check(v)) break; \
	frame_decode(v, f); \
	frame_length(f, len); \
	if(len <= 4) break; \
	len -= 4;
/* 将字节低 4 位压入帧头 */
#define frame_pack_low(v, s, c) \
	BS(v[2], 0, BR1(s, 0)); \
	BS(v[3], 3, BR1(s, 1)); \
	BS(v[3], 2, BR1(s, 2)); \
	BS(v[3], 1, BR1(s, 3)); \
	BS(v[3], 0, c);
/* 将字节高 4 位压入帧头 */
#define frame_pack_high(v, s, c) \
	BS(v[2], 0, BR1(s, 4)); \
	BS(v[3], 3, BR1(s, 5)); \
	BS(v[3], 2, BR1(s, 6)); \
	BS(v[3], 1, BR1(s, 7)); \
	BS(v[3], 0, c);
/* 从帧头读出数据, 写入低 4 位 */
#define frame_unpack_low(v, s, c) \
	s |= BR1(v[2], 0) << 0; \
	s |= BR1(v[3], 3) << 1; \
	s |= BR1(v[3], 2) << 2; \
	s |= BR1(v[3], 1) << 3; \
	c = BR1(v[3], 0);
/* 从帧头读出数据, 写入高 4 位 */
#define frame_unpack_high(v, s, c) \
	s |= BR1(v[2], 0) << 4; \
	s |= BR1(v[3], 3) << 5; \
	s |= BR1(v[3], 2) << 6; \
	s |= BR1(v[3], 1) << 7; \
	c = BR1(v[3], 0);

/* 判断是不是合并头 */
#define header_check(h) (h.id == 0xA9)
/* 判断是不是扩展头 */
#define extend_check(e) ((e.before[0] == 0x5E) && (e.before[1] == 0x39) && (e.after == 0x9B))


/*
	$ mp3_info					获取 MP3 文件信息

	@ FILE *fp

	# mp3_s *					成功返回 t_mp3 结构体指针, 失败返回 NULL
*/
mp3_s *mp3_info(FILE *fp);

/*
	$ mp3_read					从 MP3 中读取数据
	
	@ FILE *fp
	@ mp3_s *mp3
	@ seek_t *size				读取信息的长度
	@ void *arg					提供给回调函数的参数
	@ int (*callback)(seek_t, char, char *, void *)

	# char *					成功返回内容指针, 失败返回 NULL
*/
char *mp3_read(FILE *fp, mp3_s *mp3, seek_t *size, void *arg, int (*callback)(seek_t, char, char *, void *));

/*
	$ mp3_save					向 MP3 中保存信息

	@ FILE *fp
	@ mp3_s *mp3				MP3 文件信息
	@ char *data				需要保存的信息
	@ seek_t size				需要保存的信息的长度
	@ void (*callback)(double)

	# int						0=失败, 1=成功, 2=成功 (使用了扩展方式)
*/
int mp3_save(FILE *fp, mp3_s *mp3, char *data, seek_t size, void (*callback)(double));


#endif
