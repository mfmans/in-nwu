/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ win32.h, 0.1.1005
*/

#ifndef _WIN32_H_
#define _WIN32_H_

/*
	$ explorer				打开资源管理器, 并指向某个文件

	@ char *filename
*/
void explorer(char *filename);

/*
	$ fileext				获取文件扩展名

	@ char *filename

	# char *				如果没有扩展名, 返回 NULL
*/
char *fileext(char *filename);

/*
	$ select_file_open		选择一个文件以供打开

	@ const char *filter	文件筛选器, 设置为 NULL 使用默认筛选器
	@ const char *title		窗口标题

	# char *				打开了文件返回文件名, 取消返回 NULL
*/
char *select_file_open(const char *filter, const char *title);

/*
	$ select_file_save		选择一个路径以供保存

	@ char *filename		默认文件名
	@ const char *title		窗口标题

	# char *				选择了路径返回路径, 取消返回 NULL
*/
char *select_file_save(char *filename, const char *title);


/*
	$ inputbox				弹出输入窗口

	@ const char *title
	@ const char *message
	@ const char *deftext	输入框中默认的文本

	# char *				按下确定返回输入的文本, 取消返回 NULL
*/
char *inputbox(const char *title, char *message, char *deftext);


#endif
