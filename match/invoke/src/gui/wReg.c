/*
	$ Invoke   (C) 2005-2012 MF
	$ wReg.c, 0.1.1202
*/

#include "../base.h"
#include "../gui.h"

#define CLASSNAME		"INVOKE_REGISTER"
#define TITLE			"寄存器"
#define WIDTH			380
#define HEIGHT			600


static void guiRegDraw(erp_window_s *win) {
	CONTEXT	*reg = (CONTEXT *) win->param;

	EDIT(eRegEaxHex)->setText("%08X", reg->Eax);
	EDIT(eRegEaxDec)->setText("%lu", reg->Eax);
	EDIT(eRegEcxHex)->setText("%08X", reg->Ecx);
	EDIT(eRegEcxDec)->setText("%lu", reg->Ecx);
	EDIT(eRegEdxHex)->setText("%08X", reg->Edx);
	EDIT(eRegEdxDec)->setText("%lu", reg->Edx);
	EDIT(eRegEbxHex)->setText("%08X", reg->Ebx);
	EDIT(eRegEbxDec)->setText("%lu", reg->Ebx);
	EDIT(eRegEsiHex)->setText("%08X", reg->Esi);
	EDIT(eRegEsiDec)->setText("%lu", reg->Esi);
	EDIT(eRegEdiHex)->setText("%08X", reg->Edi);
	EDIT(eRegEdiDec)->setText("%lu", reg->Edi);
	EDIT(eRegEspHex)->setText("%08X", reg->Esp);
	EDIT(eRegEspDec)->setText("%lu", reg->Esp);
	EDIT(eRegEbpHex)->setText("%08X", reg->Ebp);
	EDIT(eRegEbpDec)->setText("%lu", reg->Ebp);
	EDIT(eRegEipHex)->setText("%08X", reg->Eip);
	EDIT(eRegEipDec)->setText("%lu", reg->Eip);

	EDIT(eRegCsHex)->setText("%08X", reg->SegCs);
	EDIT(eRegCsDec)->setText("%lu", reg->SegCs);
	EDIT(eRegDsHex)->setText("%08X", reg->SegDs);
	EDIT(eRegDsDec)->setText("%lu", reg->SegDs);
	EDIT(eRegSsHex)->setText("%08X", reg->SegSs);
	EDIT(eRegSsDec)->setText("%lu", reg->SegSs);
	EDIT(eRegEsHex)->setText("%08X", reg->SegEs);
	EDIT(eRegEsDec)->setText("%lu", reg->SegEs);
	EDIT(eRegFsHex)->setText("%08X", reg->SegFs);
	EDIT(eRegFsDec)->setText("%lu", reg->SegFs);
	EDIT(eRegGsHex)->setText("%08X", reg->SegGs);
	EDIT(eRegGsDec)->setText("%lu", reg->SegGs);

	EDIT(eRegEflags)->setText("0x%08X", reg->EFlags);

	EDIT(eRegEaxHex)->readonly(TRUE);
	EDIT(eRegEaxDec)->readonly(TRUE);
	EDIT(eRegEcxHex)->readonly(TRUE);
	EDIT(eRegEcxDec)->readonly(TRUE);
	EDIT(eRegEdxHex)->readonly(TRUE);
	EDIT(eRegEdxDec)->readonly(TRUE);
	EDIT(eRegEbxHex)->readonly(TRUE);
	EDIT(eRegEbxDec)->readonly(TRUE);
	EDIT(eRegEsiHex)->readonly(TRUE);
	EDIT(eRegEsiDec)->readonly(TRUE);
	EDIT(eRegEdiHex)->readonly(TRUE);
	EDIT(eRegEdiDec)->readonly(TRUE);
	EDIT(eRegEspHex)->readonly(TRUE);
	EDIT(eRegEspDec)->readonly(TRUE);
	EDIT(eRegEbpHex)->readonly(TRUE);
	EDIT(eRegEbpDec)->readonly(TRUE);
	EDIT(eRegEipHex)->readonly(TRUE);
	EDIT(eRegEipDec)->readonly(TRUE);

	EDIT(eRegCsHex)->readonly(TRUE);
	EDIT(eRegCsDec)->readonly(TRUE);
	EDIT(eRegDsHex)->readonly(TRUE);
	EDIT(eRegDsDec)->readonly(TRUE);
	EDIT(eRegSsHex)->readonly(TRUE);
	EDIT(eRegSsDec)->readonly(TRUE);
	EDIT(eRegEsHex)->readonly(TRUE);
	EDIT(eRegEsDec)->readonly(TRUE);
	EDIT(eRegFsHex)->readonly(TRUE);
	EDIT(eRegFsDec)->readonly(TRUE);
	EDIT(eRegGsHex)->readonly(TRUE);
	EDIT(eRegGsDec)->readonly(TRUE);

	EDIT(eRegEflags)->readonly(TRUE);
}

void guiReg(HWND win, CONTEXT *reg) {
	ZeroMemory(&wReg, sizeof wReg);

	wReg.classname	= CLASSNAME;
	wReg.title		= TITLE;
	wReg.width		= WIDTH;
	wReg.height		= HEIGHT;
	wReg.instance	= RT.instance;
	wReg.draw		= guiRegDraw;
	wReg.center		= TRUE;
	wReg.tool		= TRUE;
	wReg.param		= (DWORD) reg;
	wReg.parent		= win;

	lbRegHex		= DRAW_LABEL	(wReg, REG_LABEL_HEX,		100,  20,  60, 20, "十六进制");
	lbRegDec		= DRAW_LABEL	(wReg, REG_LABEL_DEC,		230,  20,  60, 20, "十进制");

	lbRegEax		= DRAW_LABEL	(wReg, REG_LABEL_EAX,		 35,  40,  60, 20, "EAX");
	lbRegEcx		= DRAW_LABEL	(wReg, REG_LABEL_ECX,		 35,  65,  60, 20, "ECX");
	lbRegEdx		= DRAW_LABEL	(wReg, REG_LABEL_EDX,		 35,  90,  60, 20, "EDX");
	lbRegEbx		= DRAW_LABEL	(wReg, REG_LABEL_EBX,		 35, 115,  60, 20, "EBX");
	lbRegEsi		= DRAW_LABEL	(wReg, REG_LABEL_ESI,		 35, 140,  60, 20, "ESI");
	lbRegEdi		= DRAW_LABEL	(wReg, REG_LABEL_EDI,		 35, 165,  60, 20, "EDI");
	lbRegEsp		= DRAW_LABEL	(wReg, REG_LABEL_ESP,		 35, 190,  60, 20, "ESP");
	lbRegEbp		= DRAW_LABEL	(wReg, REG_LABEL_EBP,		 35, 215,  60, 20, "EBP");
	lbRegEip		= DRAW_LABEL	(wReg, REG_LABEL_EIP,		 35, 240,  60, 20, "EIP");

	lbRegCs			= DRAW_LABEL	(wReg, REG_LABEL_CS,		 37, 280,  60, 20, "CS");
	lbRegDs			= DRAW_LABEL	(wReg, REG_LABEL_DS,		 37, 305,  60, 20, "DS");
	lbRegSs			= DRAW_LABEL	(wReg, REG_LABEL_SS,		 37, 330,  60, 20, "SS");
	lbRegEs			= DRAW_LABEL	(wReg, REG_LABEL_ES,		 37, 355,  60, 20, "ES");
	lbRegFs			= DRAW_LABEL	(wReg, REG_LABEL_FS,		 37, 380,  60, 20, "FS");
	lbRegGs			= DRAW_LABEL	(wReg, REG_LABEL_GS,		 37, 405,  60, 20, "GS");

	lbRegEflags		= DRAW_LABEL	(wReg, REG_LABEL_EFLAGS,	 25, 445,  60, 20, "EFLAGS");

	eRegEaxHex		= DRAW_EDIT		(wReg, REG_EDIT_EAX_HEX,	 90,  38, 110, 20, NULL);
	eRegEaxDec		= DRAW_EDIT		(wReg, REG_EDIT_EAX_DEC,	220,  38, 110, 20, NULL);
	eRegEcxHex		= DRAW_EDIT		(wReg, REG_EDIT_ECX_HEX,	 90,  63, 110, 20, NULL);
	eRegEcxDec		= DRAW_EDIT		(wReg, REG_EDIT_ECX_DEC,	220,  63, 110, 20, NULL);
	eRegEdxHex		= DRAW_EDIT		(wReg, REG_EDIT_EDX_HEX,	 90,  88, 110, 20, NULL);
	eRegEdxDec		= DRAW_EDIT		(wReg, REG_EDIT_EDX_DEC,	220,  88, 110, 20, NULL);
	eRegEbxHex		= DRAW_EDIT		(wReg, REG_EDIT_EBX_HEX,	 90, 113, 110, 20, NULL);
	eRegEbxDec		= DRAW_EDIT		(wReg, REG_EDIT_EBX_DEC,	220, 113, 110, 20, NULL);
	eRegEsiHex		= DRAW_EDIT		(wReg, REG_EDIT_ESI_HEX,	 90, 138, 110, 20, NULL);
	eRegEsiDec		= DRAW_EDIT		(wReg, REG_EDIT_ESI_DEC,	220, 138, 110, 20, NULL);
	eRegEdiHex		= DRAW_EDIT		(wReg, REG_EDIT_EDI_HEX,	 90, 163, 110, 20, NULL);
	eRegEdiDec		= DRAW_EDIT		(wReg, REG_EDIT_EDI_DEC,	220, 163, 110, 20, NULL);
	eRegEspHex		= DRAW_EDIT		(wReg, REG_EDIT_ESP_HEX,	 90, 188, 110, 20, NULL);
	eRegEspDec		= DRAW_EDIT		(wReg, REG_EDIT_ESP_DEC,	220, 188, 110, 20, NULL);
	eRegEbpHex		= DRAW_EDIT		(wReg, REG_EDIT_EBP_HEX,	 90, 213, 110, 20, NULL);
	eRegEbpDec		= DRAW_EDIT		(wReg, REG_EDIT_EBP_DEC,	220, 213, 110, 20, NULL);
	eRegEipHex		= DRAW_EDIT		(wReg, REG_EDIT_EIP_HEX,	 90, 238, 110, 20, NULL);
	eRegEipDec		= DRAW_EDIT		(wReg, REG_EDIT_EIP_DEC,	220, 238, 110, 20, NULL);

	eRegCsHex		= DRAW_EDIT		(wReg, REG_EDIT_CS_HEX,		 90, 278, 110, 20, NULL);
	eRegCsDec		= DRAW_EDIT		(wReg, REG_EDIT_CS_DEC,		220, 278, 110, 20, NULL);
	eRegDsHex		= DRAW_EDIT		(wReg, REG_EDIT_DS_HEX,		 90, 303, 110, 20, NULL);
	eRegDsDec		= DRAW_EDIT		(wReg, REG_EDIT_DS_DEC,		220, 303, 110, 20, NULL);
	eRegSsHex		= DRAW_EDIT		(wReg, REG_EDIT_SS_HEX,		 90, 328, 110, 20, NULL);
	eRegSsDec		= DRAW_EDIT		(wReg, REG_EDIT_SS_DEC,		220, 328, 110, 20, NULL);
	eRegEsHex		= DRAW_EDIT		(wReg, REG_EDIT_ES_HEX,		 90, 353, 110, 20, NULL);
	eRegEsDec		= DRAW_EDIT		(wReg, REG_EDIT_ES_DEC,		220, 353, 110, 20, NULL);
	eRegFsHex		= DRAW_EDIT		(wReg, REG_EDIT_FS_HEX,		 90, 378, 110, 20, NULL);
	eRegFsDec		= DRAW_EDIT		(wReg, REG_EDIT_FS_DEC,		220, 378, 110, 20, NULL);
	eRegGsHex		= DRAW_EDIT		(wReg, REG_EDIT_GS_HEX,		 90, 403, 110, 20, NULL);
	eRegGsDec		= DRAW_EDIT		(wReg, REG_EDIT_GS_DEC,		220, 403, 110, 20, NULL);

	eRegEflags		= DRAW_EDIT		(wReg, REG_EDIT_EFLAGS,		 90, 443, 240, 20, NULL);

	bRegOk			= DRAW_BUTTON	(wReg, REG_BUTTON_OK,		-30, -20, 100, 28, "确定", cbRegButtonOk);

	erpWindow(&wReg);
}


ERP_CALLBACK(cbRegButtonOk) {
	win->quit(0);
}
