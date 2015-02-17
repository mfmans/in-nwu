/*
	$ conv.c, 0.2.1221
*/

#include "base.h"


/*
	进制转换

	@ FROM:	base on 2 ^ 32
	@ TO:	base on 10 ^ 10
*/
size_t convert(bigint_t data, size_t count) {
	__asm {
					; 设置输入来源
			mov		esi, data

					; 设置输出来源, 即为输入偏移 32 位
			mov		edi, data
			add		edi, 4

					; 除数, 即进制数
			mov		ebx, NOTATION

					; 设置输出数据数目
			mov		DWORD PTR [esp-4], 0
					; 设置输入数据数目
			mov		ecx, count

			jecxz	convDone
			loop	convHigh

			jmp		convReady

		convHigh:
					; 移动 esi 到输入数据的最高 DW
			add		esi, 8
			loop	convHigh

		convReady:
					; 保存 esi 并恢复 ecx
			mov		[esp-8], esi
			mov		ecx, count

		convStart:
			xor		edx, edx

		convDiv:
					; 被除数 = 更高 4 位除商 * 每段大小 + 本段大小
			mov		eax, [esi]
					; EDX:EAX / EBX = EAX ... EDX
			div		ebx

					; 保存商
			mov		[esi], eax

					; 移动 esi 到前 32 位
			sub		esi, 8

			loop	convDiv

					; 保存余数, 增大输出数据的数目
			mov		[edi], edx
			add		edi, 8
			inc		DWORD PTR [esp-4]

					; 判断是否减小输入数据的数目
			mov		eax, [esp-8]
			mov		eax, [eax]
			test	eax, eax
			jnz		convNext

					; 更新 esi
			sub		DWORD PTR [esp-8], 8

					; 减小数目
			mov		ecx, count
			dec		ecx
			mov		count, ecx

			jz		convDone

		convNext:
					; 恢复 esi 指向最高位
			mov		esi, [esp-8]
					; 恢复 ecx
			mov		ecx, count

			jmp		convStart

		convDone:
					; 返回数目
			mov		eax, [esp-4]
	}
}
