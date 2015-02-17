/*
	$ sse2.c, 0.2.1221
*/

#include "base.h"


/*
	检测 CPU 是否支持 SSE2 指令集
	
	EAX = 1
		EDX [23]	MMX		MMX technology
		EDX [25]	SSE		Streaming SIMD Extensions
		EDX [26]	SSE2	Streaming SIMD Extensions 2
		ECX [00]	SSE3	Streaming SIMD Extensions 3
		ECX [09]	SSSE3	Supplemental Streaming SIMD Extensions 3
		ECX [19]	SSE4.1	Streaming SIMD Extensions 4.1
		ECX [20]	SSE4.2	Streaming SIMD Extensions 4.2
*/
int cpuSSE() {
	__asm {
		mov		eax, 1

		cpuid
		mov		eax, edx

				; EDX [26]
		shr		eax, 26
		and		eax, 1
	}
}
