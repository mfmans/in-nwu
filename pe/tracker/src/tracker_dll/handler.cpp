/*
	$ Tracker w64   (C) 2005-2014 MF
	$ tracker.dll/handler.cpp   # 1312
*/


#pragma warning (disable: 4102)


#include "base.h"




/* 处理器标签 */
#define HANDLER_BEGIN(name)			function_##name##_begin
#define HANDLER_END(name)			function_##name##_end



/* 安装处理器 */
#define INSTALL(name)														\
	__asm {		mov		eax,	offset HANDLER_BEGIN(name)		}			\
	__asm {		sub		eax,	edi								}			\
	__asm {		add		eax,	ebx								}			\
	__asm {		mov		[esi],	eax								}			\
	__asm {		add		esi,	0x10							}



/* 开跳转 */
#define JUMPER_ON()															\
	__asm {		pop		dword ptr fs:[0xC0]						}

/* 关跳转 */
#define JUMPER_OFF()														\
	__asm {		push	dword ptr fs:[0xC0]						}			\
	__asm {		push	dword ptr fs:[OFFSET_TEB]				}			\
	__asm {		pop		dword ptr fs:[0xC0]						}





__declspec(naked) void install::install_handler (void) {
	__asm {
			push	dword ptr	install::base_table
			push	dword ptr	install::base_handler
			push	dword ptr	__install
			call	__install
	}






HANDLER_BEGIN	(ZwResumeThread):			// ======================================
	__asm {
			push	ebp
			mov		ebp,	esp

			mov		edi,	[ebp + 0x10]						; EDI	= {ARGUMENT.argument}
			mov		esi,	[ebp + 0x0C]						; ESI	= {ARGUMENT.information}

			mov		ebx,	esi									; EBX	= {ARGUMENT.information}
			sub		ebx,	SIZE_ADDRESS						; EBX	= {address}

			push	ebx
			push	esi
			push	edi


			mov		edi,	[esp + 0x00]						; EDI	= {ARGUMENT.argument}
			mov		ebx,	[esp + 0x04]						; EBX	= {ARGUMENT.information}

			add		ebx,	SIZE_INFORMATION
			add		ebx,	SIZE_TABLE
			add		ebx,	SIZE_HANDLER
			add		ebx,	SIZE_FUNCTION_JUMPER				; EBX	= {injector}


			JUMPER_OFF	( )

			push	dword ptr [edi + 0x00]						; injector.thread		= ZwResumeThread.hThread

			call	ebx											; injector
			add		esp,	0x04

			JUMPER_ON	( )

			test	eax,	eax
			jz		rt_error


			jmp		rt_end


		rt_error:
			mov		esi,	[esp + 0x08]						; ESI	= {address}

			push	dword ptr 0									; TerminateThread.dwExitCode	= 0
			push	dword ptr [edi + 0x00]						; TerminateThread.hThread		= ZwResumeThread.hThread
			call	dword ptr [ebx + 0x20]						; TerminateThread

			xor		eax,	eax


		rt_end:
			add		esp,	0x0C

			pop		ebp

			ret
	}
HANDLER_END		(ZwResumeThread):			// ======================================





HANDLER_BEGIN	(ZwCreateThreadEx):			// ======================================
	__asm {
			push	ebp
			mov		ebp,	esp

			mov		edi,	[ebp + 0x10]						; EDI	= {ARGUMENT.argument}


			push	dword ptr [edi + 0x28]						; ZwCreateThreadEx.AttributeList
			push	dword ptr [edi + 0x24]						; ZwCreateThreadEx.MaximumStackSize
			push	dword ptr [edi + 0x20]						; ZwCreateThreadEx.StackSize
			push	dword ptr [edi + 0x1C]						; ZwCreateThreadEx.ZeroBits
			push	dword ptr [edi + 0x18]						; ZwCreateThreadEx.CreateThreadFlags
			push	dword ptr [edi + 0x14]						; ZwCreateThreadEx.StartContext
			push	dword ptr [edi + 0x10]						; ZwCreateThreadEx.StartRoutine
			push	dword ptr [edi + 0x0C]						; ZwCreateThreadEx.ProcessHandle
			push	dword ptr [edi + 0x08]						; ZwCreateThreadEx.ObjectAttributes
			push	dword ptr [edi + 0x04]						; ZwCreateThreadEx.DesiredAccess
			push	dword ptr [edi + 0x00]						; ZwCreateThreadEx.ThreadHandle
			push	dword ptr [edi - 0x04]						; return to ntdll.dll

			or		dword ptr [esp + 0x1C],	0x01				; ZwCreateThreadEx.CreateThreadFlags	|= CREATE_THREAD_SUSPENDED
			

			mov		edi,	[ebp + 0x14]						; EDI	= {ARGUMENT.register}

			mov		eax,	[edi - 0x00]						; old EAX
			mov		ecx,	[edi - 0x04]						; old ECX
			mov		edx,	[edi - 0x08]						; old EDX
			mov		ebx,	[edi - 0x0C]						; old EBX
			mov		esi,	[edi - 0x18]						; old ESI
			mov		edi,	[edi - 0x1C]						; old EDI


			call	dword ptr fs:[OFFSET_TEB]					; ZwCreateThreadEx
			add		esp,	0x04


			push	edi
			mov		edi,	[ebp + 0x14]						; EDI	= {ARGUMENT.register}

			mov		[edi - 0x00],	eax							; save EAX
			mov		[edi - 0x04],	ecx							; save ECX
			mov		[edi - 0x08],	edx							; save EDX
			mov		[edi - 0x0C],	ebx							; save EBX
			mov		[edi - 0x18],	esi							; save ESI
			pop		dword ptr [edi - 0x1C]						; save EDI


			mov		eax,	[esp]								; EAX	= OUT PHANDLE ZwCreateThreadEx.ThreadHandle
			mov		eax,	[eax]								; EAX	= new thread handle

			test	eax,	eax
			jz		ct_done


			mov		edi,	[ebp + 0x0C]						; EDI	= {ARGUMENT.information}


			push	eax											; injector_local.handler	= new thread handle
			call	dword ptr [edi + 0x24]						; injector_local

			add		esp,	0x04

			test	eax,	eax
			jz		ct_error


			mov		edi,	[ebp + 0x10]						; EDI	= {ARGUMENT.argument}

			mov		eax,	[edi + 0x18]						; EAX	= ZwCreateThreadEx.CreateThreadFlags
			and		eax,	0x01								; EAX	= ZwCreateThreadEx.CreateThreadFlags & CREATE_THREAD_SUSPENDED

			jnz		ct_done


			mov		edi,	[ebp + 0x0C]						; EDI	= {ARGUMENT.information}
			sub		edi,	SIZE_ADDRESS						; EDI	= {address}


			sub		esp,	0x04								; ResumeThread.hThread
			call	dword ptr [edi + 0x14]						; ResumeThread


			jmp		ct_done


		ct_error:
			mov		edi,	[ebp + 0x0C]						; EDI	= {ARGUMENT.information}
			mov		eax,	[esp - 0x04]						; EAX	= new thread handle


			push	dword ptr 0									; TerminateThread.dwExitCode	= 0
			push	eax											; TerminateThread.hThread		= new thread handle
			call	dword ptr [edi + 0x20]						; TerminateThread


		ct_done:
			xor		eax,	eax

			add		esp,	0x2C
			pop		ebp

			ret
	}
HANDLER_END		(ZwCreateThreadEx):			// ======================================





HANDLER_BEGIN	(ZwCreateUserProcess):		// ======================================
	__asm {
			mov		esi,	[esp + 0x04]						; ESI	= {ARGUMENT.callback}
			mov		edi,	[esp + 0x0C]						; EDI	= {ARGUMENT.argument}

			mov		eax,	[edi + 0x20]						; EAX	= ZwCreateUserProcess.ProcessParameters


			JUMPER_OFF	( )
			
			push	edi											; callback.argument		= {ARGUMENT.argument}
			push	dword ptr [eax + 0x44]						; callback.command		= ZwCreateUserProcess.ProcessParameters.command
			push	dword ptr [eax + 0x3C]						; callback.application	= ZwCreateUserProcess.ProcessParameters.application

			call	esi
			add		esp,	0x0C

			JUMPER_ON	( )


			ret
	}
HANDLER_END		(ZwCreateUserProcess):		// ======================================





HANDLER_BEGIN	(ZwOpenFile):				// ======================================
	__asm {
			mov		esi,	[esp + 0x04]						; ESI	= {ARGUMENT.callback}
			mov		edi,	[esp + 0x0C]						; EDI	= {ARGUMENT.argument}


			mov		eax,	[edi + 0x04]						; EAX	= ZwOpenFile.DesiredAccess

			mov		ebx,	[edi + 0x08]						; EBX	= ZwOpenFile.ObjectAttributes
			mov		ebx,	[ebx + 0x08]						; EBX	= ZwOpenFile.ObjectAttributes.ObjectName


			movzx	ecx,	word ptr [ebx]						; ECX	= ZwOpenFile.ObjectAttributes.ObjectName.Length


			JUMPER_OFF	( )

			push	eax											; callback.access	= ZwCreateFile.DesiredAccess
			push	ecx											; callback.length	= ZwCreateFile.ObjectAttributes.ObjectName.Length
			push	dword ptr [ebx + 0x04]						; callback.buffer	= ZwCreateFile.ObjectAttributes.ObjectName.Buffer

			call	esi											; callback
			add		esp,	0x0C

			JUMPER_ON	( )

			
			ret
	}
HANDLER_END		(ZwOpenFile):				// ======================================





HANDLER_BEGIN	(ZwCreateFile):				// ======================================
	__asm {
			mov		esi,	[esp + 0x04]						; ESI	= {ARGUMENT.callback}
			mov		edi,	[esp + 0x0C]						; EDI	= {ARGUMENT.argument}


			mov		eax,	[edi + 0x04]						; EAX	= ZwCreateFile.DesiredAccess

			mov		ebx,	[edi + 0x08]						; EBX	= ZwCreateFile.ObjectAttributes
			mov		ebx,	[ebx + 0x08]						; EBX	= ZwCreateFile.ObjectAttributes.ObjectName


			movzx	ecx,	word ptr [ebx]						; ECX	= ZwCreateFile.ObjectAttributes.ObjectName.Length


			JUMPER_OFF	( )

			push	eax											; callback.access	= ZwCreateFile.DesiredAccess
			push	ecx											; callback.length	= ZwCreateFile.ObjectAttributes.ObjectName.Length
			push	dword ptr [ebx + 0x04]						; callback.buffer	= ZwCreateFile.ObjectAttributes.ObjectName.Buffer

			call	esi											; callback
			add		esp,	0x0C

			JUMPER_ON	( )

			
			ret
	}
HANDLER_END		(ZwCreateFile):				// ======================================






	__asm {
		__install:
			push	ebp
			mov		ebp,	esp

			pushad


			mov		ecx,	[ebp + 0x04]					; ECX	= start address
			mov		edx,	[ebp + 0x08]					; EDX	= end address
			sub		edx,	ecx								; EDX	= length


			push	edx										; memcpy.count	= length
			push	dword ptr [ebp + 0x04]					; memcpy.src	= __install
			push	dword ptr [ebp + 0x0C]					; memcpy.dest	= {handler}
	}

	((void (__cdecl *) (void)) memcpy) ( );

	__asm {
			add		esp,	0x0C


			mov		esi,	[ebp + 0x10]					; ESI	= {table}
			add		esi,	0x04							; ESI	= first STRUCT.handler

			mov		ebx,	[ebp + 0x0C]					; EBX	= {handler}
			mov		edi,	[ebp + 0x04]					; EDI	= start address
	}

	


	// ------------------------------------ INSTALL BEGIN
		INSTALL	(ZwResumeThread);
		INSTALL	(ZwCreateThreadEx);
		INSTALL	(ZwCreateUserProcess);
		INSTALL	(ZwOpenFile);
		INSTALL	(ZwCreateFile);
	// ------------------------------------ INSTALL END




	__asm {
			popad

			pop		ebp
			add		esp,	0x10

			ret
	}
}

