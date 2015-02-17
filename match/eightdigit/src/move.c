/*
	$ move.c, 0.1.0113
*/

#include "base.h"


/*
	$ moveLeft			将空格移向左边

	@ size_t data

	# size_t			不能移动返回 0, 否则返回移动后的新封装数据
*/
size_t moveLeft(size_t data) {
	__asm {
			mov		eax, data

					; 分解空格位置
			mov		edx, eax
			shr		edx, 24
			and		edx, 0x0F

					; (1, 1)
			cmp		edx, 1
			jz		edge
					; (1, 2)
			cmp		edx, 4
			je		edge
					; (1, 3)
			cmp		edx, 7
			je		edge

					; 向左移动
			dec		edx
			shl		edx, 24

					; 合并结果
			and		eax, 0xFFFFFF
			or		eax, edx

			jmp		done

		edge:
					; 左边界
			xor		eax, eax

		done:
	}
}

/*
	$ moveRight			将空格向右移动

	@ size_t data

	# size_t
*/
size_t moveRight(size_t data) {
	__asm {
			mov		eax, data

					; 分解空格位置
			mov		edx, eax
			shr		edx, 24
			and		edx, 0x0F

					; (3, 1)
			cmp		edx, 3
			je		edge
					; (3, 2)
			cmp		edx, 6
			je		edge
					; (3, 3)
			cmp		edx, 9
			je		edge

					; 向右移动
			inc		edx
			shl		edx, 24

					; 合并结果
			and		eax, 0xFFFFFF
			or		eax, edx

			jmp		done

		edge:
					; 右边界
			xor		eax, eax

		done:
	}
}

/*
	$ moveUp			将空格向上移动

	@ size_t data

	# size_t
*/
size_t moveUp(size_t data) {
	__asm {
					; 结果
			xor		eax, eax

					; 源数据
			mov		esi, data
			ror		esi, 24

					; 空格位置
			mov		edi, esi
			and		edi, 0x0F

					; (1, 1) (2, 1) (3, 1)
			cmp		edi, 4
			jl		done

					; 循环控制
			mov		ecx, 1

					; 计算新空格位置
			mov		edx, edi
			sub		edx, 3
			
					; 保存新空格位置
			mov		eax, edx

					; 移动数据
			rol		esi, 3

					; 判断是否移动到 (1, 1)
			cmp		ecx, edx
			je		current

		fore:
			shl		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			rol		esi, 3

					; 保存完空格新位置前的数字
			inc		ecx
			cmp		ecx, edx

			jl		fore

		current:
					; 新位置的数字
			mov		edx, esi
			and		edx, 7

			rol		esi, 3
			inc		ecx

		next:
			shl		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			rol		esi, 3

					; 保存完新旧位置间的数字
			inc		ecx
			cmp		ecx, edi

			jl		next

					; 保存新空格位置的数字
			shl		eax, 3
			or		eax, edx

					; 判断从 (3, 3) 移出
			cmp		ecx, 9
			je		done

		left:
			shl		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			rol		esi, 3

					; 保存完所有数字
			inc		ecx
			cmp		ecx, 9

			jl		left

		done:
	}
}

/*
	$ moveDown			将空格向下移动

	@ size_t data

	# size_t
*/
size_t moveDown(size_t data) {
	__asm {
					; 结果
			xor		eax, eax

					; 源数据
			mov		esi, data

					; 空格位置
			mov		edi, esi
			shr		edi, 24
			and		edi, 0x0F

					; (1, 3) (2, 3) (3, 3)
			cmp		edi, 6
			jg		done

					; 循环控制
			mov		ecx, 8

					; 计算新空格位置
			mov		edx, edi
			add		edx, 3

					; 保存新空格位置
			mov		eax, edx
			shl		eax, 27

					; 判断是否移动到 (3, 3)
			dec		edx
			cmp		ecx, edx
			je		current

		fore:
			ror		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			shr		esi, 3

					; 保存完空格新位置后的数字
			dec		ecx
			cmp		ecx, edx

			jg		fore

		current:
					; 新位置的数字
			mov		edx, esi
			and		edx, 7

			shr		esi, 3
			dec		ecx

		next:
			ror		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			shr		esi, 3

					; 保存完新旧位置间的数字
			dec		ecx
			cmp		ecx, edi

			jge		next

					; 保存新空格位置的数字
			ror		eax, 3
			or		eax, edx

					; 判断从 (1, 1) 移出
			test	ecx, ecx
			jz		position

		left:
			ror		eax, 3

			mov		ebx, esi
			and		ebx, 7

			or		eax, ebx

			shr		esi, 3

					; 保存完所有数字
			loop	left

		position:
					; 规格化结果
			rol		eax, 21

		done:
	}
}
