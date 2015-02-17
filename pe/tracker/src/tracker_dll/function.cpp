/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/function.cpp   # 1312
*/


#include "base.h"




/* 双字占位符 */
#define DATA()								\
	__asm {		__emit	0x00	}			\
	__asm {		__emit	0x00	}			\
	__asm {		__emit	0x00	}			\
	__asm {		__emit	0x00	}



/* 函数通用头部 */
#define FUNCTION_HEADER()										\
	__asm {		push	dword ptr __install			}			\
	__asm {		call	__install					}


/* 函数安装开始 */
#define FUNCTION_INSTALL_BEGIN()								\
	__install:													\
	__asm {		push	ebp																			}		\
	__asm {		mov		ebp,	esp																	}		\
	__asm {		pushad																				}		\
	__asm {		mov		esi,	[ebp + 0x04]		/* ESI	= begin of	function			*/		}		\
	__asm {		mov		edi,	[ebp + 0x08]		/* EDI	= end of	function + 0x05		*/		}		\
	__asm {		sub		edi,	esi					/* EDI	= length of	function			*/		}		\
	__asm {		push	edi							/* memcpy.count	= length of	function	*/		}		\
	__asm {		push	esi							/* memcpy.src	= begin of	function	*/		}		\
	__asm {		push	dword ptr [ebp + 0x10]		/* memcpy.dest	= (ARGUMENT.block)		*/		}		\
	((void (__cdecl *) (void)) memcpy) ( );																	\
	__asm {		add		esp,	0x0C																}		\
	__asm {		mov		ebx,	[ebp + 0x10]		/* EBX	= {ARGUMENT.block}				*/		}

/* 函数安装结束 */
#define FUNCTION_INSTALL_END()									\
	__asm {		popad																				}		\
	__asm {		pop		ebp																			}		\
	__asm {		add		esp,	0x08																}		\
	__asm {		ret																					}

/* 函数数据写入 */
#define FUNCTION_INSTALL_DATA(from, to)							\
	__asm {		mov		eax,	[ebp + from]		}			\
	__asm {		mov		[ebx + to],	eax				}			\





/* WoW64 系统调用转发器 */
__declspec(naked) void __cdecl install::map_function_jumper (uintptr_t base, uintptr_t information, uintptr_t table) {
	FUNCTION_HEADER ( );


	__asm {
			call	start


			DATA	( )							; DATA.information
			DATA	( )							; DATA.table


		start:
			push	ebp
			mov		ebp,	esp

			pushad


			mov		esi,	[ebp + 0x04]					; ESI	= * {DATA}

			mov		ebx,	[esi + 0x04]					; EBX	= {DATA.table}
			mov		edx,	[ebp + 0x08]					; EDX	= syscall return address


		find:
			cmp		dword ptr [ebx],	0					; ITEM [0] == 0
			jz		notfound

			cmp		[ebx],	edx								; ITEM [0] == syscall return address
			jz		found

			add		ebx,	0x10							; EBX	= next STRUCTURE
			jmp		find


		found:
			mov		eax,	ebp
			add		eax,	0x10							; EAX	= pointer of	arguments

			mov		ecx,	ebp
			sub		ecx,	0x04							; ECX	= pointer of	register (pushad)


			push	ecx										; handler.register		= ECX
			push	eax										; handler.argument		= EAX
			push	dword ptr [esi]							; handler.information	= {DATA.information}
			push	dword ptr [ebx + 0x08]					; handler.callback

			call	dword ptr [ebx + 0x04]					; handler

			add		esp,	0x10

			mov		dword ptr [ebp + 0x04],	eax				; return code	= eax


			jmp		done


		notfound:
			mov		dword ptr [ebp + 0x04],	1				; return code	= 1


		done:
			popad

			pop		ebp
			add		esp,	0x04


			cmp		dword ptr [esp - 0x04],	0				; check return code
			jz		end

			push	dword ptr fs:[OFFSET_TEB]				; push the Wow64 system call jumper address as return address


		end:
			ret												; return to system call when return non-zero
															; return back user mode when return zero
	}


	FUNCTION_INSTALL_BEGIN	( );
		FUNCTION_INSTALL_DATA	(0x14,	0x05);				// DATA.information
		FUNCTION_INSTALL_DATA	(0x18,	0x09);				// DATA.table
	FUNCTION_INSTALL_END	( );
}




/* 注入器 */
__declspec(naked) void __cdecl install::map_function_injector (uintptr_t base, uintptr_t address, uintptr_t information) {
	FUNCTION_HEADER ( );

	// ARGUMENT		(thread: HANDLE)
	__asm {
			call	start


			DATA	( )							; DATA.address
			DATA	( )							; DATA.information


		start:
			push	ebp
			mov		ebp,	esp


			mov		esi,	[esp + 0x04]					; ESI	= {DATA}

			push	dword ptr [esi + 0x00]					; [ESP]	= {DATA.address}
			push	dword ptr [esi + 0x04]					; [ESP] = {DATA.information}


			mov		eax,	[esp + 0x04]					; EAX	= {DATA.address}

			push	dword ptr [ebp + 0x0C]					; GetProcessIdOfThread.Thread
			call	dword ptr [eax + 0x28]					; GetProcessIdOfThread

			test	eax,	eax
			jz		done


			mov		ebx,	fs:[0x20]						; EBX	= current process id

			cmp		ebx,	eax								; ignore local thread
			jz		done


			mov		edi,	[esp]							; EDI	= {ARGUMENT.information}
			add		edi,	0x28							; EDI	= information.injector_remote

			push	dword ptr [ebp + 0x0C]					; injector_remote.thread	= {ARGUMENT.thread}
			call	dword ptr [edi]							; injector_remote

			add		esp,	0x04


		done:
			add		esp,	0x10

			mov		ebp,	[esp - 0x08]

			ret
	}


	FUNCTION_INSTALL_BEGIN	( );
		FUNCTION_INSTALL_DATA	(0x14,	0x05);				// DATA.address
		FUNCTION_INSTALL_DATA	(0x18,	0x09);				// DATA.information
	FUNCTION_INSTALL_END	( );
}



/* 本地注入器 */
__declspec(naked) void __cdecl install::map_function_injector_local (uintptr_t base, uintptr_t address, uintptr_t information) {
	FUNCTION_HEADER ( );

	// ARGUMENT		(thread: HANDLE)
	__asm {
			call	start


			DATA	( )							; DATA.address
			DATA	( )							; DATA.information


		start:
			push	ebp
			mov		ebp,	esp


			mov		esi,	[esp + 0x04]					; ESI	= {DATA}
			mov		ebx,	[esi]							; EBX	= {DATA.address}

			push	ebx										; [ESP]	= {DATA.address}
			push	dword ptr [esi + 0x04]					; [ESP] = {DATA.information}


			sub		esp,	0x02CC							; CONTEXT
			mov		dword ptr [esp],	CONTEXT_ALL			; CONTEXT.ContextFlags	= CONTEXT_ALL


			push	esp										; GetThreadContext.lpContext	= CONTEXT
			push	dword ptr [ebp + 0x0C]					; GetThreadContext.hThread		= {ARGUMENT.thread}
			call	dword ptr [ebx + 0x18]					; GetThreadContext

			add		esp,	0x02CC

			test	eax,	eax
			jz		error


			sub		esp,	0x02CC							; CONTEXT
			sub		esp,	0x08							; LDT_ENTRY


			mov		esi,	[ebp - 0x04]					; ESI	= {DATA.address}


			push	esp										; GetThreadSelectorEntry.lpSelectorEntry	= LDT_ENTRY
			push	dword ptr [esp + 0x9C]					; GetThreadSelectorEntry.dwSelector			= CONTEXT.SegFs
			push	dword ptr [ebp + 0x0C]					; GetThreadSelectorEntry.hThread			= {ARGUMENT.thread}
			call	dword ptr [esi + 0x2C]					; GetThreadSelectorEntry

			add		esp,	0x2D4

			test	eax,	eax
			jz		error


			sub		esp,	0x2D4

			xor		ebx,	ebx

			mov		bh,		[esp + 0x07]					; BH	= LDT_ENTRY.HighWord.Bytes.BaseHi
			mov		bl,		[esp + 0x04]					; BL	= LDT_ENTRY.HighWord.Bytes.BaseMid

			shl		ebx,	16
			mov		bx,		[esp + 0x02]					; BX	= LDT_ENTRY.BaseLow

			add		esp,	0x08


			mov		esi,	[ebp - 0x08]					; ESI	= {DATA.information}
			mov		esi,	[esi + 0x30]					; ESI	= loader

			mov		eax,	[esp + 0xB0]					; EAX	= CONTEXT.Eax
			

			mov		[ebx + OFFSET_TEB],	eax					; FS:[OFFSET_TEB]	= CONTEXT.Eax

			mov		[esp + 0xB0],	esi						; CONTEXT.Eax	= loader


			mov		esi,	[ebp - 0x04]					; ESI	= {DATA.address}

			push	esp										; SetThreadContext.lpContext	= CONTEXT
			push	dword ptr [ebp + 0x0C]					; SetThreadContext.hThread		= {ARGUMENT.thread}
			call	dword ptr [esi + 0x1C]					; SetThreadContext

			add		esp,	0x02CC

			test	eax,	eax
			jnz		done


		error:
			mov		esi,	[ebp - 0x04]					; ESI	= {DATA.address}

			push	dword ptr 0								; TerminateThread.dwExitCode	= 0
			push	dword ptr [ebp + 0x0C]					; TerminateThread.hThread		= {ARGUMENT.thread}

			call	dword ptr [esi + 0x20]					; TerminateThread


		done:
			add		esp,	0x10

			mov		ebp,	[esp - 0x08]

			ret
	}


	FUNCTION_INSTALL_BEGIN	( );
		FUNCTION_INSTALL_DATA	(0x14,	0x05);				// DATA.address
		FUNCTION_INSTALL_DATA	(0x18,	0x09);				// DATA.information
	FUNCTION_INSTALL_END	( );
}



/* 远程注入器 */
__declspec(naked) void __cdecl install::map_function_injector_remote (uintptr_t base, uintptr_t address, uintptr_t information) {
	FUNCTION_HEADER ( );

	// ARGUMENT		(thread: HANDLE)
	__asm {
			call	start


			DATA	( )							; DATA.address
			DATA	( )							; DATA.information


			; tracker_remote, \0
			__emit	0x74
			__emit	0x72
			__emit	0x61
			__emit	0x63
			__emit	0x6B
			__emit	0x65
			__emit	0x72
			__emit	0x5F
			__emit	0x72
			__emit	0x65
			__emit	0x6D
			__emit	0x6F
			__emit	0x74
			__emit	0x65
			__emit	0x00


		start:
			push	ebp
			mov		ebp,	esp

			mov		esi,	[ebp + 0x04]					; ESI	= {DATA}

			push	esi										; [ESP]	= {DATA}
			push	dword ptr 0								; [ESP]	= HMODULE


			mov		ebx,	[esi]							; EBX	= {DATA.address}
			mov		edi,	[esi + 0x04]					; EDI	= {DATA.information}


			push	dword ptr [edi]							; LoadLibraryA.lpLibFileName
			call	dword ptr [ebx]							; LoadLibraryA

			test	eax,	eax
			jz		done

			mov		[esp],	eax


			mov		esi,	[esp + 0x04]					; ESI	= {DATA}
			mov		ebx,	[esi]							; EBX	= {DATA.address}
			
			mov		edi,	esi								; EDI	= {DATA}
			add		edi,	0x08							; EDI	= "tracker_remote"


			push	edi										; GetProcAddress.lpProcName
			push	eax										; GetProcAddress.hModule
			call	dword ptr [ebx + 0x08]					; GetProcAddress

			test	eax,	eax
			jz		done


			mov		esi,	[esp + 0x04]					; ESI	= {DATA}
			mov		esi,	[esi + 0x04]					; ESI	= {DATA.information}

			
			push	dword ptr [esi + 0x04]					; tracker_remote.controller		= {information.controller}
			push	dword ptr [esi + 0x00]					; tracker_remote.library		= {information.library}
			push	dword ptr [ebp + 0x0C]					; tracker_remote.thread			= {ARGUMENT.thread}

			call	eax
			add		esp,	0x0C


		done:
			mov		[ebp + 0x04],	eax						; save return code

			mov		esi,	[esp + 0x04]					; ESI	= {DATA}
			mov		esi,	[esi]							; ESI	= {DATA.address}

			call	dword ptr [esi + 0x04]					; FreeLibrary


			add		esp,	0x04

			pop		ebp
			pop		eax										; restore return code

			ret
	}


	FUNCTION_INSTALL_BEGIN	( );
		FUNCTION_INSTALL_DATA	(0x14,	0x05);				// DATA.address
		FUNCTION_INSTALL_DATA	(0x18,	0x09);				// DATA.information
	FUNCTION_INSTALL_END	( );
}




/* 线程启动器 */
__declspec(naked) void __cdecl install::map_function_loader (uintptr_t base, uintptr_t jumper) {
	FUNCTION_HEADER ( );


	__asm {
			call	start


			DATA	( )							; DATA.jumper


		start:
			push	dword ptr fs:[OFFSET_TEB]	; push real entry address as return address


			push	eax
			push	ebx


			mov		eax,	fs:[0xC0]			; EAX	= old syscall address

			mov		ebx,	[esp + 0x0C]		; EBX	= {DATA}
			mov		ebx,	[ebx]				; EBX	= {DATA.jumper}


			mov		fs:[0xC0],			ebx		; set syscall address
			mov		fs:[OFFSET_TEB],	eax		; backup old syscall address


			pop		ebx
			pop		eax

			add		esp,	0x08


			jmp		dword ptr [esp - 0x08]
	}


	FUNCTION_INSTALL_BEGIN	( );
		FUNCTION_INSTALL_DATA	(0x14,	0x05);				// DATA.jumper
	FUNCTION_INSTALL_END	( );
}

