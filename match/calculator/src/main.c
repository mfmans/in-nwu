/*
	$ ExprCalc   (C) 2014 MF
	$ main.c   #1403
*/


#include "header/base.h"
#include "header/bridge.h"



/* 表达式计算接口 */
__declspec(dllexport) int __cdecl expression_calculate (const char *input, char *output, char *error) {
	/* 执行状态 */
	int	status	= 0;

	/* 打开 */
	calc_open (input, output, error);

	/* 执行 */
	status	= calc_execute ( );

	/* 关闭 */
	calc_close ( );

	return status;
}




/* DLL 入口 */
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			/* 加载 DLL 时生成随机数种子 */
			srand ((unsigned int) time (NULL));

			break;
	}

	return TRUE;
}

