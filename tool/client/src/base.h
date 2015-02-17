/*
	$ MF Client   (C) 2005-2013 MF
	$ base.h
*/

#ifndef _CLIENT_H_
#define _CLIENT_H_

/* unicode */
#if !defined(UNICODE)
	#define UNICODE
#endif
#if !defined(_UNICODE)
	#define _UNICODE
#endif

#if !defined(_CLIENT_HEADER_)
	#include <stdio.h>

	/* windows */
	#include <Windows.h>

	#include <tchar.h>
#endif

/* for gui extern */
#if !defined(_CLIENT_GUI_)
	#define _CLIENT_GUI_		extern
#endif



/* @ auth.c */
int		authStart			(char *username, char *password, char *device, BYTE *mac, char **error);
void	authStop			();

/* @ config.c */
int		configRead			();
void	configWrite			();
void	configClear			();
char *	configGetUsername	();
char *	configGetPassword	();
BYTE *	configGetAdapter	();
BOOL	configGetHidden		();
BOOL	configGetLogin		();
void	configSetUsername	(char	*value);
void	configSetPassword	(char	*value);
void	configSetAdapter	(BYTE	*value);
void	configSetHidden		(BOOL	value);
void	configSetLogin		(BOOL	value);

/* @ device.c */
void	deviceInit			();
int		deviceGet			(int	basic,	BYTE	*mac);
size_t	deviceList			(int	basic,	void	*list);
char *	deviceSelect		(int	basic,	int		index,	BYTE	**mac);
int		deviceCompare		(WCHAR	*name);

/* @ md5.c */
void	md5					(char	*source,	size_t	length,	char	*output);

/* @ gui.c */
void	guiInit				(HINSTANCE	instance);
void	guiHide				();
void	guiShow				(char		*message);

/* @ icon.c */
void	iconCreate			();
void	iconDestroy			();
void	iconTip				(PTCHAR	title,	PTCHAR	message,	int	type);
void	iconDraw			();
void	iconInvoke			(WPARAM	id);

/* @ call.c */
void	callStart			();
int		callAuth			();
void	callRefresh			();
void	callDisconnect		();
void	callClose			();
int		callStatus			();
void	callPower			(WPARAM	action);

/* @ main.c */
void	restart				();



/* 窗口句柄 */
_CLIENT_GUI_ HWND handle_window;
/* 对象句柄 */
_CLIENT_GUI_ HWND handle_object_tip;
_CLIENT_GUI_ HWND handle_object_username, handle_object_password, handle_object_adapter;
_CLIENT_GUI_ HWND handle_object_hidden, handle_object_login, handle_object_remember;
_CLIENT_GUI_ HWND handle_object_button;



/* # Windows Message */
#define WM_CLIENT			WM_USER + 101


/* # memory */
#define MEMORY_ALLOC(v, t, c)		\
	if((v = (t *) calloc(c, sizeof(t))) == NULL) {			\
		exit(0);											\
	}
#define MEMORY_ALLOCS(v, t, s)		\
	if((v = (t *) calloc(s, 1)) == NULL) {					\
		exit(0);											\
	}
#define MEMORY_FREE(v)				\
	if(v) {													\
		free(v);											\
		v = NULL;											\
	}



/* GUI 操作定义 */
#define CHECK_T(obj)		SendMessage(handle_object_##obj, BM_SETCHECK, BST_CHECKED,		0)
#define CHECK_F(obj)		SendMessage(handle_object_##obj, BM_SETCHECK, BST_UNCHECKED,	0)
#define CHECK_GET(obj)		SendMessage(handle_object_##obj, BM_GETCHECK, 0,				0)


/* 提示 */
#define WARNING(msg)		MessageBox(handle_window, TEXT(msg), TEXT(""), MB_ICONWARNING | MB_OK);		\
							return 0;



#endif   /* _CLIENT_H_ */