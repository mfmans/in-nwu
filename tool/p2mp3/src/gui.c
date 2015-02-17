#define _GUI_

/* windows 桌面程序方式链接 */
#pragma comment (linker, "/subsystem:windows /entry:mainCRTStartup")

/* 在 manifest 中实现 XP 样式 */
#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


#include "base.h"
#include "gui.h"
#include "execute.h"


/* 窗口标题 */
#define WIN_TITLE	"Push To MP3"
/* 窗口长宽 */
#define WIN_WIDTH	600
#define WIN_HEIGHT	195


static HWND buttonPush, buttonPop;
int running = 0;


static int WINAPI gui_callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int i;

	HDC hdc;
	HFONT font;

	switch(uMsg) {
		case WM_CREATE:
			/* 说明 */
			CreateWindow("Static", TEXT("点击“合并文件”，可将一个任意类型的文件合并到 MP3 文件中"), WS_CHILD | WS_VISIBLE, 15, 18, 430, 15, hWnd, (HMENU) 1, NULL, 0);
			CreateWindow("Static", TEXT("点击后，请依次选择 MP3 源文件、需要合并的文件和合并结果保存的路径"), WS_CHILD | WS_VISIBLE, 15, 34, 430, 15, hWnd, (HMENU) 2, NULL, 0);
			CreateWindow("Static", TEXT("如果需要对合并的文件数据进行加密，在合并过程中根据提示输入密钥即可"), WS_CHILD | WS_VISIBLE, 15, 50, 430, 15, hWnd, (HMENU) 3, NULL, 0);
			CreateWindow("Static", TEXT("点击“提取文件”，可将使用本程序合并到 MP3 文件中的数据提取出来"), WS_CHILD | WS_VISIBLE, 15, 90, 430, 15, hWnd, (HMENU) 5, NULL, 0);
			CreateWindow("Static", TEXT("如果文件在合并时数据经过加密，则在提取过程中需要您提供正确的密钥"), WS_CHILD | WS_VISIBLE, 15, 106, 430, 15, hWnd, (HMENU) 6, NULL, 0);
			CreateWindow("Static", TEXT("提取完毕后，请选择输出路径，程序将会自动把提取出来的内容保存到该路径"), WS_CHILD | WS_VISIBLE, 15, 122, 430, 15, hWnd, (HMENU) 7, NULL, 0);

			/* 按钮 */
			buttonPush	= CreateWindowEx(0, "Button", "合并文件", WS_CHILD | WS_VISIBLE, 450, 15, 120, 55, hWnd, (HMENU) 4, NULL, 0);
			buttonPop	= CreateWindowEx(0, "Button", "提取文件", WS_CHILD | WS_VISIBLE, 450, 85, 120, 55, hWnd, (HMENU) 8, NULL, 0);

			hdc = GetDC(hWnd);
			font = CreateFont(12, 6, 0, 0, 12, 0, 0, 0, GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "宋体");

			SelectObject(hdc, font);
			ReleaseDC(hWnd, hdc);

			for(i = 1; i <= 8; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM) font, 0);
			}

			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case 4: push(); break;
				case 8: pop(); break;
			}

			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return (int) DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 1;
}


void gui() {
	HINSTANCE inst;
	HWND hDesktop;
	RECT rcDesktop;
	MSG msg;
	WNDCLASS wnd;

	int posX, posY;

	inst		= GetModuleHandle(NULL);
	hDesktop	= GetDesktopWindow();

	GetWindowRect(hDesktop, &rcDesktop);

	posX = (rcDesktop.right - WIN_WIDTH) / 2;
	posY = (rcDesktop.bottom - WIN_HEIGHT - 20) / 2;

	m_zero(&wnd, sizeof wnd);

	wnd.hInstance		= inst;
	wnd.hbrBackground	= (HBRUSH) COLOR_BTNSHADOW;
	wnd.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wnd.hCursor			= LoadCursor(0, IDC_ARROW);
	wnd.lpfnWndProc		= (WNDPROC) gui_callback;
	wnd.lpszClassName	= "P2MP3_CLASS";
	wnd.style			= CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wnd);

	if((hwnd = CreateWindow("P2MP3_CLASS", WIN_TITLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, posX, posY, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, inst, NULL)) == NULL) {
		die("创建应用程序窗口失败。");
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, 0, 0, 0) > 0) {
		if(msg.message == WM_KEYDOWN) {
			SendMessage(hwnd, msg.message, msg.wParam, msg.wParam);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void buttonEnable(int enable) {
	EnableWindow(buttonPush, enable);
	EnableWindow(buttonPop, enable);
}

void buttonPushText(char *text) {
	if(text == NULL) {
		SetWindowText(buttonPush, "合并文件");
	} else {
		SetWindowText(buttonPush, text);
	}
}
