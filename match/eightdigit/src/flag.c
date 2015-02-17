/*
	$ flag.c, 0.1.0113
*/

#include "base.h"


/* 标志信息空间 */
static void *flag = NULL;


/*
	$ flagInit			初始化标志信息空间

	# int
*/
void flagInit() {
	if(flag) {
		flagClear();
		return;
	}

	if((flag = calloc(0xFFFFFF, 4)) == NULL) {
		exit(1);
	}
}

/*
	$ flagClear			清空所有标志信息
*/
void flagClear() {
	__asm {
		pushfd
		cld

				; 标志位空间
		mov		edi, flag
				; 处理字节数
		mov		ecx, 0xFFFFFF

				; 写入的内容
		xor		eax, eax

				; 清空空间
		rep		stos DWORD PTR [edi]

		popfd
	}
}

/*
	$ flagSet			检测标志位

	@ size_t data
	@ size_t direction

	# int				如果标志位已设置, 返回非 0, 否则设置标志位并返回 0
*/
int flagSet(size_t data, size_t direction) {
	__asm {
			mov		ecx, data
			mov		esi, ecx

					; 生成索引
			and		esi, 0xFFFFFF
			shl		esi, 2

					; 定位内存位置
			add		esi, flag

					; 读取标志双字
			mov		eax, [esi]

					; 生成偏移量
			shr		ecx, 24
			and		ecx, 0x0F

					; 检测模版
			mov		edx, 7

					; 设置模版
			mov		ebx, direction
			and		ebx, 7

					; 模版移位
			dec		ecx
			jz		check

		move:
			shl		edx, 3
			shl		ebx, 3

					; 移位完成
			loop	move

		check:
					; 检测标志位
			test	eax, edx
			jnz		done

					; 设置位信息
			or		eax, ebx
					; 保存位信息
			mov		[esi], eax

			xor		eax, eax

		done:
	}
}

/*
	$ flagGet			提取标志位

	@ size_t data

	# int				父节点移动方式, 如未访问过返回 0
*/
size_t flagGet(size_t data) {
	__asm {
			mov		ecx, data
			mov		ebx, ecx

					; 生成索引
			and		ebx, 0xFFFFFF
			shl		ebx, 2

					; 定位内存位置
			add		ebx, flag

					; 读取标志双字
			mov		eax, [ebx]

					; 生成偏移量
			shr		ecx, 24
			and		ecx, 0x0F

					; 判断标志位是否需要移位
			dec		ecx
			jz		done

		move:
			shr		eax, 3
			loop	move

		done:
			and		eax, 7
	}
}
