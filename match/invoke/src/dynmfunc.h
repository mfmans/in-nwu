/*
	$ Dynamic DLL Function Loader   (C) 2005-2012 mfboy
	$ dynmfunc.h, 0.1.1117
*/

#ifndef _DYNMFUNC_H_
#define _DYNMFUNC_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 创建函数 */
#define FUNCTION(name, ret, call, ...)  ret (call *name)(__VA_ARGS__)

/* 被动加载模式 */
#define FUNCTION_PASSIVE		0x01
/* 主动加载模式下加载失败时直接返回而非抛出异常 */
#define FUNCTION_NOT_FATAL		0x02


/* 加载函数 */
int Function(void *p, char *dll, char *function, int flag);


#if defined(__cplusplus)
	}
#endif

#endif   /* _DYNMFUNC_H_ */