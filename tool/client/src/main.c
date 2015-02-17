/*
	$ MF Client   (C) 2005-2013 MF
	$ main.c
*/

#include "base.h"


/* 启动路径 */
static PTCHAR command;


/*
	uac
*/
static int uac() {
	int		result	= 0;

	DWORD	length;
	HANDLE	handle;

	TOKEN_ELEVATION elevation;

	/* 进程句柄 */
	handle = GetCurrentProcess();

	/* 获取令牌 */
	if(OpenProcessToken(handle, TOKEN_QUERY, &handle) == FALSE) {
		return 0;
	}

	if(GetTokenInformation(handle, (TOKEN_INFORMATION_CLASS) 20, &elevation, sizeof elevation, &length)) {
		if(length == sizeof elevation) {
			if(elevation.TokenIsElevated) {
				result = 1;
			}
		}
	}

	CloseHandle(handle);

	return result;
}

/*
	restart
*/
void restart() {
	TCHAR	filepath	[MAX_PATH + 1];
	SHELLEXECUTEINFO	shl;

	/* 已经以管理员权限启动 */
	if(uac()) {
		return;
	}

	/* EXE 路径 */
	GetModuleFileName(NULL, filepath, _countof(filepath));

	ZeroMemory(&shl, sizeof shl);

	shl.cbSize			= sizeof shl;
	shl.lpVerb			= TEXT("runas");
	shl.lpFile			= filepath;
	shl.lpParameters	= command;
	shl.nShow			= SW_NORMAL;

	ShellExecuteEx	(&shl);

	ExitProcess		(0);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	command = pCmdLine;

	deviceInit();
	guiInit(hInstance);

	return 0;
}
