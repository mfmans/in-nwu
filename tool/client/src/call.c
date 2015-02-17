/*
	$ MF Client   (C) 2005-2013 MF
	$ call.c
*/

#include "base.h"

#include <iphlpapi.h>


/* 最多重试次数 */
#define MAX_RETRY				1

/* 接入时段限制 */
#define ERROR_TIME				63027


/* 已连接上 */
static int		connected	= 0;
/* 重试次数 */
static int		retried		= 0;
/* 休眠 */
static int		suspend		= 0;

/* 调度线程 */
static HANDLE	thread		= NULL;


/*
	errid
*/
static int errid(char *message) {
	int result;

	if(message[0] == 'E') {
		if(sscanf_s(message + 1, "%u", &result) == 1) {
			return result;
		}
	}

	return 0;
}


/*
	threadCall
*/
static DWORD WINAPI threadCall(LPVOID dummy) {
	BYTE	*mac;

	int		basic,			index;
	char	*username,		*password,		*device;
	DWORD	username_len,	password_len;
	
	if(CHECK_GET(hidden)) {
		basic = 1;
	} else {
		basic = 0;
	}

	if((username_len = GetWindowTextLengthA(handle_object_username)) == 0) {
		WARNING("请输入连接网络需要的用户名。");
	}
	if((password_len = GetWindowTextLengthA(handle_object_password)) == 0) {
		WARNING("请输入连接网络需要的密码。");
	}

	if(username_len > 20) {
		WARNING("输入的用户名太长，请重新输入。");
	}
	if(password_len > 20) {
		WARNING("输入的密码太长，请重新输入。");
	}

	if((index = SendMessage(handle_object_adapter, CB_GETCURSEL, 0, 0)) == CB_ERR) {
		WARNING("请选择一个网卡设备以接入网络。");
	}
	if((device = deviceSelect(basic, index, &mac)) == NULL) {
		WARNING("请选择一个正确的网卡设备。");
	}
	
	MEMORY_ALLOC	(username, char, username_len + 2);
	MEMORY_ALLOC	(password, char, password_len + 2);
	
	GetWindowTextA	(handle_object_username, username, username_len + 1);
	GetWindowTextA	(handle_object_password, password, password_len + 1);

	configSetUsername	(username);
	configSetPassword	(password);
	configSetAdapter	(mac);
	
	if(CHECK_GET(hidden)) {
		configSetHidden	(TRUE);
	} else {
		configSetHidden	(FALSE);
	}
	if(CHECK_GET(login)) {
		configSetLogin	(TRUE);
	} else {
		configSetLogin	(FALSE);
	}

	if(CHECK_GET(remember)) {
		configWrite();
	} else {
		configClear();
	}

	guiHide();

	/* 创建提示 */
	iconCreate();
	iconTip(TEXT("请稍后"), TEXT("正在努力尝试连接网络。"), 0);

	/* 调度器 */
	{
		int		status;
		char	*error;

		/* 设置正在连接状态 */
		connected = 2;

		while(1) {
			status = authStart(username, password, device, mac, &error);

			/* 正常断开 */
			if(status == 0) {
				connected = 0;

				iconTip(TEXT("连接断开"), TEXT("网络连接已经断开。"), 1);

				return 0;
			}

			/* 出现错误 */
			if(status == 6) {
				if(error) {
					/* 接入时段限制 */
					if(errid(error) == ERROR_TIME) {
						if(connected == 1) {
							iconTip(TEXT("网络断开"), TEXT("现在受到接入时段的限制，网络已经断开连接。"), 1);
						} else {
							iconTip(TEXT("连接失败"), TEXT("现在受到接入时段的限制，不能连接网络。"), 2);
						}

						MEMORY_FREE(error);

						connected = 0;

						return 0;
					}

					guiShow(error);

					MEMORY_FREE(error);
				} else {
					guiShow("连接出现未知的错误。");
				}

				break;
			}

			if((retried++) >= MAX_RETRY) {
				switch(status) {
					case 1:
						guiShow("无法初始化接入设备。");
						break;

					case 2:
					case 3:
						guiShow("连接服务器超时。");
						break;

					case 4:
						guiShow("无法分析服务器返回的数据。");
						break;

					case 5:
						guiShow("服务器自动断开连接。");
						break;
				}

				break;
			}

			Sleep(1000);
		}
	}

	connected = 0;

	iconDestroy();

	return 0;
}


/*
	refresh
*/
static int refresh(int allocate) {
	LONG	i;
	ULONG	size = 0;

	PIP_INTERFACE_INFO	table = NULL;

	if(GetInterfaceInfo(NULL, &size) != ERROR_INSUFFICIENT_BUFFER) {
		return 0;
	}

	MEMORY_ALLOCS(table, IP_INTERFACE_INFO, size);

	if(GetInterfaceInfo(table, &size) == NO_ERROR) {
		for(i = 0; i < table->NumAdapters; i++) {
			/* 找到设备 */
			if(deviceCompare(table->Adapter[i].Name)) {
				goto execute;
			}
		}
	}

	MEMORY_FREE(table);

	return 0;

execute:
	{
		DWORD result;

		if(allocate) {
			result = IpRenewAddress		(&table->Adapter[i]);
		} else {
			result = IpReleaseAddress	(&table->Adapter[i]);
		}
		
		MEMORY_FREE(table);

		if(result == NO_ERROR) {
			return 1;
		} else {
			return 0;
		}
	}
}


/*
	disconnect
*/
static void disconnect() {
	/* 终止调度线程 */
	TerminateThread(thread, 0);

	/* 等待线程终止 */
	if(connected == 1) {
		WaitForSingleObject(thread, INFINITE);
	} else {
		WaitForSingleObject(thread, 2000);
	}

	/* 释放线程对象 */
	CloseHandle(thread);

	/* 发送断开数据包 */
	if(connected) {
		authStop();
		Sleep(500);
	}

	/* 释放 IP */
	refresh(0);

	connected = 0;
}


/*
	callStart										执行开始
*/
void callStart() {
	thread = CreateThread(NULL, 0, threadCall, NULL, 0, NULL);
}

/*
	callRefresh										认证成功需要刷新 IP
*/
void callRefresh() {
	if(refresh(1) == 0) {
		iconTip(TEXT("连接失败"), TEXT("现在已经通过了登录信息的验证，但无法从服务器上申请到 IP 地址，这可能由于服务器上的 IP 已分配完毕。\r\n\r\n稍后请手动使用 ipconfig /renew 命令或重新连接。"), 1);
	} else {
		if(suspend) {
			iconTip(TEXT("连接恢复"), TEXT("计算机从休眠中唤醒后，网络已经自动恢复连接。"), 0);
		} else {
			if(connected == 1) {
				iconTip(TEXT("连接恢复"), TEXT("网络已经自动恢复连接。"), 0);
			} else {
				iconTip(TEXT("连接成功"), TEXT("已经成功连接上了网络。"), 0);
			}
		}
	}

	connected	= 1;
	retried		= 0;
	suspend		= 0;
}

/*
	callDisconnect									终止连接
*/
void callDisconnect() {
	disconnect();

	iconDestroy();
	guiShow("");
}

/*
	callClose										终止连接退出
*/
void callClose() {
	disconnect();

	iconDestroy();
	guiHide();

	/* 退出 */
	SendMessage(handle_window, WM_DESTROY, 0, 0);
}


/*
	callStatus										当前连接状态

	# int											0=未连接
													1=已连接
*/
int callStatus() {
	return connected;
}


/*
	callPower										电源操作处理

	@ WPARAM		action
*/
void callPower(WPARAM action) {
	switch(action) {
		/* 休眠 */
		case PBT_APMSUSPEND:
			suspend = 1;

			/* 断开网络 */
			if(connected) {
				disconnect();
			}

			break;

		/* 唤醒 */
		case PBT_APMRESUMEAUTOMATIC:
			disconnect();
			callStart();

			break;
	}
}
