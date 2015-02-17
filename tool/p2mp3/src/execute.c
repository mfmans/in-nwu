/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ code.c, 0.1.1005
*/

#include "base.h"
#include "execute.h"
#include "win32.h"
#include "code.h"
#include "mp3.h"
#include "gui.h"

/* 复制缓冲区大小 */
#define COPY_BUFFER		4 * 1024 * 1024
/* MP3 类型筛选器 */
#define MP3_FILTER		"MP3 文件 (*.mp3)\0*.mp3\0"


void push_callback(double progress) {
	static int oldpercent = 0;

	int curpercent = (int) (progress * 100);
	char text[5];

	curpercent = (curpercent < 0) ? 0 : curpercent;
	curpercent = (curpercent > 100) ? 100 : curpercent;

	if(curpercent != oldpercent) {
		oldpercent = curpercent;

		sprintf(text, "%d%%", curpercent);

		buttonPushText(text);
	}
}

int pop_callback(seek_t length, char read, char *buffer, void *arg) {
	header_s *header = (header_s *) arg;

	/* 读取第一个字节 */
	if(length == 0) {
		if(((unsigned char) read) == 0xA9) {
			return 1;
		} else {
			return 0;
		}
	}

	/* 等待信息头 */
	if(length < sizeof(header_s)) {
		return 1;
	}
	/* 信息头解码 */
	if(length == sizeof(header_s)) {
		m_copy(header, buffer, length);
	}

	/* 判断是否读取完毕 */
	if(length < (header->size + sizeof(header_s))) {
		return 1;
	} else {
		return 0;
	}
}


t_register(push) {
	void **argv		= (void **) arg;

	mp3_s *mp3		= (mp3_s *) argv[0];	/* 文件信息 */
	FILE *mp3fp		= (FILE *) argv[1];		/* MP3 文件句柄 */
	FILE *dstfp		= (FILE *) argv[2];		/* 目标文件句柄 */
	char *data		= (char *) argv[3];		/* 需要保存的数据 */
	seek_t size		= (seek_t) argv[4];		/* 需要保存的数据尺寸 */
	char *dstfile	= (char *) argv[5];		/* 目标文件路径 */

	char *buffer;
	int result;

	seek_t bufsize = COPY_BUFFER;

	m_new(buffer, char, COPY_BUFFER);

	buttonEnable(0);
	running = 1;

	/* 复制文件 */
	while(!feof(mp3fp)) {
		if(bufsize = (seek_t) fread(buffer, sizeof(char), COPY_BUFFER, mp3fp)) {
			fwrite(buffer, sizeof(char), bufsize, dstfp);
		}

		bufsize = COPY_BUFFER;
	}

	result = mp3_save(dstfp, mp3, data, size, push_callback);

	fclose(mp3fp);
	fclose(dstfp);

	m_del(buffer);
	m_del(data);

	buttonEnable(1);
	buttonPushText(NULL);
	running	= 0;

	switch(result) {
		case 0:
			alert("合并文件失败，请检查文件是否正确。");
			unlink(dstfile);

			return;

		case 1: success("文件合并成功完成。"); break;
		case 2: success("文件合并成功完成，本次合并使用了扩展模式。"); break;
	}

	explorer(dstfile);
}

t_register(pop) {
	void **argv	= (void **) arg;

	mp3_s *mp3	= (mp3_s *) argv[0];	/* 文件信息 */
	FILE *mp3fp	= (FILE *) argv[1];		/* MP3 文件句柄 */

	FILE *fp;
	header_s header;
	seek_t size;

	char *all, *data = NULL, *temp, *key;
	char *filename;
	int done = 0;

	buttonEnable(0);
	running = 1;

	m_zero(&header, sizeof header);

	do {
		done = 1;

		if((all = mp3_read(mp3fp, mp3, &size, &header, pop_callback)) == NULL) {
			alert("读取文件信息失败，可能该文件不含合并数据或数据已损坏。"); break;
		}
		if(!header_check(header)) {
			alert("读取到的合并信息不可使用，可能是文件已损坏。"); break;
		}

		/* 修正长度 */
		size -= sizeof header;

		if(size != header.size) {
			alert("读取到的数据长度不正确，可能是文件已损坏。"); break;
		}
		if(header.crcdat != crc32(all + sizeof header, size)) {
			alert("读取的信息无法通过验证，可能是文件已损坏。"); break;
		}

		/* 复制内容 */
		m_new(data, char, size);
		m_copy(data, all + sizeof header, size);
		m_del(all);

		done = 0;
		temp = data;

		while(header.crcdat != header.crcraw) {
			if((key = inputbox("密钥", "请输入密钥，解密合并的数据：", "")) == NULL) {
				done = 1; break;
			}
			if(key[0] == 0) {
				continue;
			}

			key = md5(key);

			/* 解密失败后释放上次的空间 */
			if(temp != data) {
				m_del(temp);
			}

			/* 解密 */
			temp = rc4(data, size, key);
			header.crcdat = crc32(temp, size);

			m_del(key);
		}

		/* 如果 done = 0, 要么没有加密, 要么解密成功 */
		if(done == 0) {
			if(temp != data) {
				m_del(data);

				data = temp;
			}
		}
	} while(0);

	if(done == 0) {
		/* 根据原来的扩展名生成默认文件名 */
		if(header.ext[0]) {
			ms_new(filename, strlen(header.ext) + 16);

			strcat(filename, "result.");
			strcat(filename, header.ext);
		} else {
			filename = NULL;
		}
	}

	while(done == 0) {
		if((filename = select_file_save(filename, "请选择文件保存路径")) == NULL) {
			break;
		}
		if((fp = fopen(filename, "wb")) == NULL) {
			alert("选定的文件路径无法打开，请重新选择。"); continue;
		}

		fwrite(data, sizeof(char), size, fp);
		fclose(fp);

		explorer(filename);

		break;
	}

	fclose(mp3fp);

	m_del(data);

	buttonEnable(1);
	running = 0;
}


void push() {
	static void *arg[6] = {NULL};

	mp3_s *mp3;
	header_s header;
	seek_t maxsize;
	
	FILE *mp3fp, *datfp, *dstfp;

	char *mp3file, *datfile, *dstfile, *key;
	char *data, *temp, title[50], message[256];

	if(running) {
		return;
	}

	while(1) {
		if((mp3file = select_file_open(MP3_FILTER, "请选择 MP3 源文件")) == NULL) {
			return;
		}

		if((mp3fp = fopen(mp3file, "rb+")) == NULL) {
			alert("选定的文件无法打开，请检查该文件是否存在或正在被使用。"); continue;
		}

		if((mp3 = mp3_info(mp3fp)) == NULL) {
			alert("分析选定的文件失败，该文件可能不是一个合法的 MP3 文件。"); fclose(mp3fp); continue;
		}
		if(mp3->count < 100) {
			alert("选择的文件中包含的音频时长可能太短，不适宜存储数据。"); fclose(mp3fp); continue;
		}

		break;
	}

	/* 计算最大容纳量 */
	maxsize = (mp3->count - 2) / 2;

	/* 生成标题 */
	if(maxsize < 1024) {
		sprintf(title, "请选择需要合并的文件（建议小于 %d Byte）", maxsize);
		sprintf(message, "这个 MP3 源文件适宜的储存容量大约为 %d Byte，建议稍后所选的合并文件不要超过这个体积。", maxsize);
	} else {
		sprintf(title, "请选择需要合并的文件（建议小于 %.2f KByte）", ((double) maxsize) / 1024);
		sprintf(message, "这个 MP3 源文件适宜的储存容量大约为 %.2f KByte，建议稍后所选的合并文件不要超过这个体积。", ((double) maxsize) / 1024);
	}

	success(message);

	while(1) {
		if((datfile = select_file_open(NULL, title)) == NULL) {
			fclose(mp3fp); return;
		}

		if((datfp = fopen(datfile, "rb")) == NULL) {
			alert("选定的文件无法打开，请检查该文件是否存在或正在被使用。"); continue;
		}

		/* 获取文件长度 */
		fseek(datfp, 0, SEEK_END);
		header.size = ftell(datfp);

		/* 容纳提示 */
		if(header.size == 0) {
			alert("选定的文件是一个空文件，程序不支持合并空文件，请重新选择。"); fclose(datfp); continue;
		}
		if(header.size > maxsize) {
			if(!confirm("选定的文件的大小超过 MP3 源文件所能提供的最大储存容量，合并可能会导致 MP3 的结构发生变化，是否继续？")) {
				fclose(datfp); continue;
			}
		}

		break;
	}

	while(1) {
		if((dstfile = select_file_save("result.mp3", "请选择保存的路径")) == NULL) {
			fclose(mp3fp); fclose(datfp); return;
		}

		if(strcmp(dstfile, mp3file) == 0) {
			alert("保存路径不能与 MP3 源文件路径相同。"); continue;
		}
		if(strcmp(dstfile, datfile) == 0) {
			alert("保存路径不能与需要合并的文件路径相同。"); continue;
		}

		if((dstfp = fopen(dstfile, "wb+")) == NULL) {
			alert("无法访问选定的路径，请重新选择。"); continue;
		}

		break;
	}

	if((key = inputbox("密钥", "如果您需要对合并的数据进行加密，请输入密钥：", "")) != NULL) {
		if(key[0] == 0) {
			key = NULL;
		}
	}

	rewind(mp3fp);
	rewind(datfp);
	rewind(dstfp);

	/* 读取需要合并的内容到缓冲区 */
	m_new(data, char, header.size);
	fread(data, sizeof(char), header.size, datfp);
	fclose(datfp);

	header.id = 0xA9;
	header.crcraw = crc32(data, header.size);

	m_zero(header.ext, 5);

	/* 如果需要加密 */
	if(key != NULL) {
		key = md5(key);

		temp = data;
		data = rc4(temp, header.size, key);

		m_del(key);
		m_del(temp);

		header.crcdat = crc32(data, header.size);
	} else {
		header.crcdat = header.crcraw;
	}

	/* 寻找扩展名 */
	do {
		if((temp = fileext(datfile)) == NULL) {
			break;
		}
		if(temp[0] == 0) {
			break;
		}
		if(strlen(temp) > 4) {
			break;
		}

		strcpy(header.ext, temp);
	} while(0);

	/* 更新数据 */
	m_new(temp, char, header.size + sizeof header);
	m_copy(temp, &header, sizeof header);
	m_copy(temp + sizeof header, data, header.size);
	m_del(data);
	
	data = temp;
	header.size += sizeof header;

	/* 设置参数 */
	arg[0] = mp3;
	arg[1] = mp3fp;
	arg[2] = dstfp;
	arg[3] = data;
	arg[4] = (void *) header.size;
	arg[5] = dstfile;

	t_start(push, arg);
}

void pop() {
	static void *arg[2] = {NULL};

	mp3_s *mp3;
	FILE *fp;

	char *mp3file;
	if(running) {
		return;
	}

	while(1) {
		if((mp3file = select_file_open(MP3_FILTER, "请选择需要提取数据的 MP3 文件")) == NULL) {
			return;
		}

		if((fp = fopen(mp3file, "rb+")) == NULL) {
			alert("选定的文件无法打开，请检查该文件是否存在或正在被使用。"); continue;
		}

		if((mp3 = mp3_info(fp)) == NULL) {
			alert("分析选定的文件失败，该文件可能不是一个合法的 MP3 文件。"); fclose(fp); continue;
		}
		if(mp3->count < 100) {
			alert("选择的文件中包含的音频时长可能太短，不适宜存储数据。"); fclose(fp); continue;
		}

		break;
	}

	arg[0] = mp3;
	arg[1] = fp;

	t_start(pop, arg);
}
