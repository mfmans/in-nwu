/*
	$ Tracker w64   (C) 2005-2014 MF
	$ demo/main.cpp   # 1312
*/


#pragma warning (disable: 4996)


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>



// MessageBoxTimeoutA
static	int (__stdcall * MessageBoxTimeoutW) (HWND hwnd, wchar_t *message, wchar_t *title, unsigned int type, void *language, unsigned int timeout);


// 日志文件
static	FILE *	fp	= NULL;

// 临界区
static	CRITICAL_SECTION	section;



/* 主入口 */
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch(fdwReason) {
		// 附加到进程
		case DLL_PROCESS_ATTACH:
			{
				unsigned int	pid	= (unsigned int) GetCurrentProcessId ( );


				HMODULE		module	= LoadLibraryA ("user32.dll");

				MessageBoxTimeoutW	= (int (__stdcall *) (HWND, wchar_t *, wchar_t *, unsigned int, void *, unsigned int)) GetProcAddress (module, "MessageBoxTimeoutW");


				char	output		[MAX_PATH + 32];

				char	path_dll	[MAX_PATH + 1];
				char	path_exe	[MAX_PATH + 1];

				ZeroMemory (path_dll, sizeof path_dll);
				ZeroMemory (path_exe, sizeof path_exe);


				// 获取当前追踪器文件所在路径
				GetModuleFileNameA (hinstDLL,	path_dll,	MAX_PATH);
				// 获取 EXE 文件所在路径
				GetModuleFileNameA (NULL,		path_exe,	MAX_PATH);

				// 查找结尾
				char *	end_dll	= strrchr (path_dll, '\\');
				char *	end_exe	= strrchr (path_exe, '\\');

				// 生成输出文件路径
				if(end_dll == NULL) {
					if(end_exe == NULL) {
						sprintf (output, "pid_%d.txt", pid);
					} else {
						sprintf (output, "%s.pid_%d.txt", end_exe + 1, pid);
					}
				} else {
					* end_dll	= '\0';

					if(end_exe == NULL) {
						sprintf (output, "%s\\pid_%d.txt", path_dll, pid);
					} else {
						sprintf (output, "%s\\%s.pid_%d.txt", path_dll, end_exe + 1, pid);
					}
				}

				// 打开文件
				fp	= fopen (output, "wb");

				// 初始化临界区
				InitializeCriticalSection (& section);
			}
			break;

		// 从进程中退出
		case DLL_PROCESS_DETACH:
			// 关闭文件
			fclose (fp);

			break;

		// 附加到线程
		case DLL_THREAD_ATTACH:		break;
		// 从线程中退出
		case DLL_THREAD_DETACH:		break;
	}

	return TRUE;
}





/* @ ZwCreateUserProcess */
extern "C" __declspec(dllexport) int __cdecl tCreateProcess (wchar_t *application, wchar_t *command, uintptr_t argument) {
	// 文本长度
	size_t		length	= wcslen(application) + wcslen(command) + 256;
	// 文本缓冲区
	wchar_t *	message	= new wchar_t [length];

	// 生成提示信息
	wsprintfW (message, L"应用程序正在尝试启动新的进程，是否允许？\n\n进程名称：%ls\n启动命令：%ls\n\n如果选择否，该应用程序将收到启动失败的返回信息。\n如果 15 秒内不做出任何选择将自动允许启动。", application, command);

	// 询问用户
	int		result	= MessageBoxTimeoutW (NULL, message, L"Tracker w64", MB_ICONQUESTION | MB_YESNO, NULL, 15 * 1000);

	// 释放文本
	delete [] message;

	if(result == IDYES) {
		return 1;
	} else if(result == 32000) {			// TIMEOUT
		return 1;
	} else {
		return 0;
	}
}




/* @ ZwCreateFile */
extern "C" __declspec(dllexport) int __cdecl tCreateFile (wchar_t *buffer, unsigned int length, unsigned int access) {
	// 获取时间
	time_t	timestamp	= time(NULL);
	// 转换为本地时间
	tm *	timeinfo	= localtime(& timestamp);

	// 时间缓冲区
	char	timebuffer	[128];

	// 生成时间字符串
	strftime(timebuffer, sizeof timebuffer, "[%b %d %H:%M:%S]", timeinfo);


	// 进入临界区
	EnterCriticalSection (& section);

	// 输出文件名
	fprintf (fp, "%s \"%ls\"   [", timebuffer, buffer);

	// 输出请求权限
	if((access & FILE_ADD_FILE)				== FILE_ADD_FILE)				fprintf(fp, " FILE_ADD_FILE");
	if((access & FILE_ADD_SUBDIRECTORY)		== FILE_ADD_SUBDIRECTORY)		fprintf(fp, " FILE_ADD_SUBDIRECTORY");
	if((access & FILE_ALL_ACCESS)			== FILE_ALL_ACCESS)				fprintf(fp, " FILE_ALL_ACCESS");
	if((access & FILE_APPEND_DATA)			== FILE_APPEND_DATA)			fprintf(fp, " FILE_APPEND_DATA");
	if((access & FILE_CREATE_PIPE_INSTANCE)	== FILE_CREATE_PIPE_INSTANCE)	fprintf(fp, " FILE_CREATE_PIPE_INSTANCE");
	if((access & FILE_DELETE_CHILD)			== FILE_DELETE_CHILD)			fprintf(fp, " FILE_DELETE_CHILD");
	if((access & FILE_EXECUTE)				== FILE_EXECUTE)				fprintf(fp, " FILE_EXECUTE");
	if((access & FILE_LIST_DIRECTORY)		== FILE_LIST_DIRECTORY)			fprintf(fp, " FILE_LIST_DIRECTORY");
	if((access & FILE_READ_ATTRIBUTES)		== FILE_READ_ATTRIBUTES)		fprintf(fp, " FILE_READ_ATTRIBUTES");
	if((access & FILE_READ_DATA)			== FILE_READ_DATA)				fprintf(fp, " FILE_READ_DATA");
	if((access & FILE_READ_EA)				== FILE_READ_EA)				fprintf(fp, " FILE_READ_EA");
	if((access & FILE_TRAVERSE)				== FILE_TRAVERSE)				fprintf(fp, " FILE_TRAVERSE");
	if((access & FILE_WRITE_ATTRIBUTES)		== FILE_WRITE_ATTRIBUTES)		fprintf(fp, " FILE_WRITE_ATTRIBUTES");
	if((access & FILE_WRITE_DATA)			== FILE_WRITE_DATA)				fprintf(fp, " FILE_WRITE_DATA");
	if((access & FILE_WRITE_EA)				== FILE_WRITE_EA)				fprintf(fp, " FILE_WRITE_EA");
	if((access & STANDARD_RIGHTS_READ)		== STANDARD_RIGHTS_READ)		fprintf(fp, " STANDARD_RIGHTS_READ");
	if((access & STANDARD_RIGHTS_WRITE)		== STANDARD_RIGHTS_WRITE)		fprintf(fp, " STANDARD_RIGHTS_WRITE");

	// 输出换行
	fprintf (fp, " ]\r\n");

	// 退出临界区
	LeaveCriticalSection (& section);

	return 1;
}

