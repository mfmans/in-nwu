/*
	$ pow.c, 0.2.1221
*/

#include "base.h"


/* 使用 ALU 进行乘方运算 */
size_t powWithALU(bigint_t data, int_t ratio, int_t exp) {
	__asm {
			mov		edi, data

					; 设置乘数
			mov		ebx, 43046721

					; 写入长度
			mov		DWORD PTR [esp-4], 1
					; 写入系数
			mov		eax, ratio
			mov		[edi], eax

					; 测试指数是否为 0
			cmp		exp, 0
			je		loopDone

		loopStart:
					; 清空进位信息
			xor		esi, esi

					; 连乘次数, 即当前数目
			mov		ecx, [esp-4]

		loopMul:
					; 读取当前要处理的被乘数
			mov		eax, [edi]

					; EAX = EAX * 3
			mul		ebx
			add		eax, esi
			
					; 发生进位
			jnc		loopSave
			inc		edx

		loopSave:
					; 保存进位
			mov		esi, edx
			mov		[edi], eax

			add		edi, 8

					; 判断是否循环结束
			loop	loopMul

					; 判断高位溢出
			test	edx, edx
			jz		loopContinue

					; 增加数目
			mov		[edi], edx

			inc		DWORD PTR [esp-4]

		loopContinue:
					; 降指数
			mov		ecx, exp
			dec		ecx

			jz		loopDone

					; 保存新指数
			mov		exp, ecx
					; 恢复 edi
			mov		edi, data

			jmp		loopStart

		loopDone:
			mov		eax, [esp-4]
	}
}


/* 使用 SSE2 进行乘方运算 */
size_t powWithSSE(bigint_t data, int_t ratio, int_t exp) {
	__asm {
			mov		edi, data

					; 写入系数
			mov		eax, ratio
			mov		[edi], eax

					; 写入长度
			mov		DWORD PTR [esp-4], 1

					; 判断指数是否为 0
			cmp		exp, 0
			je		loopDone

					; 设置乘数
			mov		eax, 43046721
			movd	xmm1, eax
			movlhps	xmm1, xmm1

		loopStart:
					; 高 1 位进位信息
			xor		edx, edx

					; 连乘次数, 即当前数目
			mov		ecx, [esp-4]

		loopMul:
					; 高 1 位辅助进位信息
			xor		ebx, ebx

					; 读取被乘数
			movapd	xmm0, [edi]

					; EAX = EAX * N
			pmuludq	xmm0, xmm1

					; 0~31 = 0:[0~31]
			movd	eax, xmm0
			add		eax, edx
					; 31~63 = 0:[31~63]
			psrldq	xmm0, 4
			movd	edx, xmm0

					; 保存低 32 位
			mov		[edi], eax

					; 判断加上前一段的高 32 位的进位后是否再次进位
			jnc		loopHigh
			inc		ebx

		loopHigh:
					; 空出 1 个空的 DW
			add		edi, 8
			psrldq	xmm0, 4

					; 64~95 = 1:[0~31]
			movd	eax, xmm0
			add		eax, edx
					; 96~127 = 1:[32~64]
			psrldq	xmm0, 4
			movd	edx, xmm0

					; 判断加上低 32 位的进位后是否再次进位
			jnc		loopCarry

			inc		edx

		loopCarry:
					; 加上低 32 位的辅助进位
			add		eax, ebx
			jnc		loopSave

			inc		edx

		loopSave:
					; 保存高 32 位
			mov		[edi], eax

			add		edi, 8

					; 循环结束
			sub		ecx, 2
			jc		loopNextOdd
			jnz		loopMul
			jmp		loopNextCarry

		loopNextOdd:
			test	eax, eax
			jz		loopNextCarry

			inc		DWORD PTR [esp-4]

		loopNextCarry:
			test	edx, edx
			jz		loopContinue

			mov		[edi], edx

			inc		DWORD PTR [esp-4]

		loopContinue:
					; 降指数
			mov		ecx, exp
			dec		ecx

			jz		loopDone

					; 保存新指数
			mov		exp, ecx
					; 恢复 edi
			mov		edi, data

			jmp		loopStart

		loopDone:
					; 返回长度
			mov		eax, [esp-4]
	}
}
