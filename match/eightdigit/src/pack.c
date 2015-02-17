/*
	$ pack.c, 0.1.0113
*/

#include "base.h"


/*
	$ pack				封装数字

	@ void *number

	# size_t
*/
size_t pack(void *number) {
	__asm {
			pushfd
			cld

					; 数字源
			mov		esi, number
					; 数字个数
			mov		ecx, 8

					; 读取空格位置
			lods	BYTE PTR [esi]
			and		eax, 0x0F

					; 封装空格
			mov		edx, eax

		parse:
			lods	BYTE PTR [esi]

					; 清空高位
			dec		al
			and		al, 7

					; 合并数字
			shl		edx, 3
			or		dl, al

			loop	parse

					; 返回
			mov		eax, edx

			popfd
	}
}

/*
	$ unpack			解封数字

	@ void *number
	@ size_t data
*/
void unpack(void *number, size_t data) {
	__asm {
			pushfd
			std

					; 封装结果
			mov		edx, data
					; 数字个数
			mov		ecx, 8

					; 数字源
			mov		edi, number
			add		edi, 8

		parse:
					; 取出当前数字
			mov		al, dl
			and		al, 7
			inc		al

					; 保存数字
			stos	BYTE PTR [edi]

					; 移出数字
			shr		edx, 3

			loop	parse

					; 保存空格位置
			mov		al, dl
			and		al, 0x0F
			stos	BYTE PTR [edi]

			popfd
	}
}
