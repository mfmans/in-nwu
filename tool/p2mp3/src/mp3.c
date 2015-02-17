/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ mp3.c, 0.1.1005
*/

#include "base.h"
#include "mp3.h"

/* 文件读取缓冲区大小 */
#define BUFFER_READ_SIZE		4096
/* 写文件缓冲区大小 */
#define BUFFER_WRITE_SIZE		4 * 1024 * 1024
/* 扩展区隔离带 */
#define EXTEND_SEPARATE	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"


/* 数据帧长度计算系数 */
static unsigned int tableRatio[][4] = {
	/* MPEG2.5 */		{0,  72000,  72000,  24000},
	/* undefined */		{0,      0,      0,      0},
	/* MPEG2 */			{0,  72000,  72000,  24000},
	/* MPEG1 */			{0, 144000, 144000,  48000}
};

/* 比特率表 */
static unsigned int tableBitrates[][16] = {
	/*  0 = 00 00, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  1 = 00 01, MPEG2.5 LY3 */	{0,   8,  16,  24,  32,  64,  80,  56,  64, 128, 160, 112, 128, 256, 320, 0},
	/*  2 = 00 10, MPEG2.5 LY2 */	{0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
	/*  3 = 00 11, MPEG2.5 LY1 */	{0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
	/*  4 = 01 00, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  5 = 01 01, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  6 = 01 10, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  7 = 01 11, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  8 = 10 00, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/*  9 = 10 01, MPEG2 LY3 */		{0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0},
	/* 10 = 10 10, MPEG2 LY2 */		{0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0},
	/* 11 = 10 11, MPEG2 LY1 */		{0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
	/* 12 = 11 00, undefined */		{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0},
	/* 13 = 11 01, MPEG1 LY3 */		{0,  32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0},
	/* 14 = 11 10, MPEG1 LY2 */		{0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
	/* 15 = 11 11, MPEG1 LY1 */		{0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}
};

/* 采样信息表 */
static unsigned int tableFrequency[][4] = {
	/* 00 = MPEG 2.5 */		{11025, 12000,  8000, 0},
	/* 01 = undefined */	{    0,     0,     0, 0},
	/* 10 = MPEG 2 */		{22050, 24000, 16000, 0},
	/* 11 = MPEG 1 */		{44100, 48000, 32000, 0}
};


mp3_s *mp3_info(FILE *fp) {
	mp3_s *mp3;
	id3v2_s id3v2;
	frame_s frame;

	/* 文件信息 */
	count_t count = 0;
	seek_t position = 0;
	/* 帧信息 */
	var_t value[4];
	seek_t length = 0;

	rewind(fp);

	/* 找 id3v2, 如找到, 跳过 */
	if(fread(&id3v2, sizeof id3v2, 1, fp) == 0) {
		return NULL;
	}
	if(id3v2_check(id3v2)) {
		fseek(fp, id3v2_length(id3v2), SEEK_CUR);
	} else {
		rewind(fp);
	}

	while(!feof(fp)) {
		/* 找到第 1 帧 */
		if(fread(value, sizeof(unsigned char), 4, fp) == 4) {
			if(frame_check(value)) {
				frame_decode(value, frame);
				frame_length(frame, length);

				if(length > 4) {
					position = (count == 0) ? (ftell(fp) - 4) : position;
					count++;

					/* 移动到下一帧 */
					fseek(fp, length - 4, SEEK_CUR);

					continue;
				}
			}
		}

		/* 没有找到数据帧 */
		if(count == 0) {
			fseek(fp, -3, SEEK_CUR);
		} else {
			break;
		}
	}

	/* 没有找到数据帧, 可能不是 MP3 文件 */
	if(count == 0) {
		return NULL;
	}

	m_new(mp3, mp3_s, 1);

	mp3->count		= count;
	mp3->position	= position;

	return mp3;
}

char *mp3_read(FILE *fp, mp3_s *mp3, seek_t *size, void *arg, int (*callback)(seek_t, char, char *, void *)) {
	frame_s frame;
	id3v1_s id3v1;
	extend_s extend;

	seek_t i, j;
	seek_t length, end, extsize, bufsize = 0;
	var_t value[4];

	char read, code, *buffer = NULL;

	*size = 0;

	/* 计算帧数 */
	i = 0;
	j = ((int) (mp3->count / 2)) * 2;

	/* 前往第 1 帧 */
	fseek(fp, mp3->position, SEEK_SET);

	while((i++) < j) {
		frame_next(value, frame, length);

		/* 前 2 帧不保存数据 */
		if(i > 2) {
			/* 奇数次填充低 4 位, 偶数次填充高 4 位 */
			if((i % 2) == 0) {
				frame_unpack_high(value, read, code);

				/* 进行校验 */
				if(BP(read) != code) {
					break;
				}

				/* 回调函数进行判断 */
				if(callback(*size, read, buffer, arg) == 0) {
					break;
				}

				if(bufsize == 0) {
					m_renew(buffer, char, *size + BUFFER_READ_SIZE);

					bufsize = BUFFER_READ_SIZE;
				}

				buffer[*size] = read;

				(*size)++;
				bufsize--;
			} else {
				read = 0;

				frame_unpack_low(value, read, code);

				/* 奇数次的校验码必为 1 */
				if(code != 1) {
					break;
				}
			}
		}

		fseek(fp, length, SEEK_CUR);
	}

	/* 从数据帧中没有读到数据 */
	if(buffer == NULL) {
		return NULL;
	}
	/* 异常中断 */
	if(i < j) {
		return buffer;
	}

	/* 找 id3v1 */
	fseek(fp, -128, SEEK_END);

	/* 读取并判断标签是否合法 */
	if(id3v1_check(id3v1)) {
		fseek(fp, -135, SEEK_END);
	} else {
		fseek(fp, -7, SEEK_END);
	}
	
	if(fread(&extend, sizeof extend, 1, fp) == 1) {
		/* 判断附加信息 */
		if(extend_check(extend)) {
			end = ftell(fp) - 7;
			extsize = end - extend.start;

			m_renew(buffer, char, *size + extsize);

			/* 读取扩展区的内容 */
			fseek(fp, extend.start, SEEK_SET);
			fread(buffer + *size, sizeof(char), extsize, fp);

			*size += extsize;
		}
	}

	return buffer;
}

int mp3_save(FILE *fp, mp3_s *mp3, char *data, seek_t size, void (*callback)(double)) {
	frame_s frame;
	id3v1_s id3v1;
	extend_s extend;

	seek_t i, j, length;
	var_t value[4];

	/* 保存原大小 */
	double allsize = (double) size;
	/* 计算总的保存帧数 */
	i = 0;
	j = ((int) (mp3->count / 2)) * 2;

	/* 前往第 1 帧 */
	fseek(fp, mp3->position, SEEK_SET);

	while((i++) < j) {
		frame_next(value, frame, length);

		if(i > 2) {
			/* 奇数次保存低 4 位及标识符, 偶数次保存高 4 位及奇校检 */
			if((i % 2) == 0) {
				frame_pack_high(value, *data, BP(*data));

				data++;
				size--;

				/* 更新处理进度显示 */
				callback(1 - ((double) size) / allsize);
			} else {
				frame_pack_low(value, *data, 1);
			}

			/* 更新帧头 */
			fseek(fp, -4, SEEK_CUR);
			fwrite(value, sizeof(char), 4, fp);

			/* 所有数据处理完毕 */
			if(size == 0) {
				callback(1.);

				return 1;
			}
		}

		fseek(fp, length, SEEK_CUR);
	}

	/* 此时还剩余数据, 移动到最后 id3v1 的标签 */
	fseek(fp, -128, SEEK_END);

	/* 判断最后是否有 id3v1 标签 */
	if(id3v1_check(id3v1)) {
		fseek(fp, -128, SEEK_END);
	} else {
		fseek(fp, 0, SEEK_END);

		id3v1.header[0] = 0;
	}

	/* 写入 16 字节的隔离信息, 防止扩展区数据被播放器当做数据帧解析 */
	fwrite(EXTEND_SEPARATE, sizeof(char), 16, fp);
	
	/* 设置扩展信息 */
	extend.start		= ftell(fp);
	extend.before[0]	= 0x5E;
	extend.before[1]	= 0x39;
	extend.after		= 0x9B;

	/* 将剩余内容写入文件 */
	while(size > 0) {
		i = BUFFER_WRITE_SIZE;
		i = (size < i) ? size : i;

		fwrite(data, sizeof(char), i, fp);

		size -= i;
		data += i;

		callback(1 - ((double) size) / allsize);
	}

	/* 写入扩展信息 */
	fwrite(&extend, sizeof extend, 1, fp);

	/* 恢复 id3v1 */
	if(id3v1.header[0]) {
		fwrite(&id3v1, sizeof id3v1, 1, fp);
	}

	callback(1.);

	return 2;
}
