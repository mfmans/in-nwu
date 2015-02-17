/*
	$ Invoke   (C) 2005-2012 MF
	$ gui.h, 0.1.1124
*/

#ifndef _INVOKE_GUI_H_
#define _INVOKE_GUI_H_


#if defined(__cplusplus)
	extern "C" {
#endif


/* 进程窗口 */
#define PROC_LABEL_TIP			1000
#define PROC_BUTTON_REFRESH		1010
#define PROC_LIST_PROCESS		1011
#define PROC_BUTTON_FILE		1020
#define PROC_BUTTON_START		1021

erp_window_s			wProc;
erp_object_label_s		*lbProcTip;
erp_object_button_s		*bProcRefresh, *bProcFile, *bProcStart;
erp_object_list_s		*lsProcProcess;

ERP_CALLBACK(cbProcButtonRefresh);
ERP_CALLBACK(cbProcButtonFile);
ERP_CALLBACK(cbProcButtonStart);
ERP_CALLBACK(cbProcListProcess);

void guiProc();
void guiProcDisable(BOOL disable);


/* 断点窗口 */
#define BP_LIST_MODULE			2000
#define BP_LIST_FUNCTION		2010
#define BP_BUTTON_DISASM		2011
#define BP_BUTTON_BATCH			2012
#define BP_CHECK_HIJ			2020
#define BP_CHECK_SKIP			2021
#define BP_BUTTON_OK			2030

erp_window_s			wBp;
erp_object_list_s		*lsBpModule, *lsBpFunction;
erp_object_check_s		*cBpHij, *cBpSkip;
erp_object_button_s		*bBpDisasm, *bBpBatch, *bBpOk;

ERP_CALLBACK(cbBpListModule);
ERP_CALLBACK(cbBpListFunction);
ERP_CALLBACK(cbBpButtonDisasm);
ERP_CALLBACK(cbBpButtonBatch);
ERP_CALLBACK(cbBpButtonOk);

int guiBp(HWND win, char *title, module_s **module, unsigned int count, int check);


/* 反汇编窗口 */
#define ASM_EDIT_ADDRESS		3000
#define ASM_BUTTON_GOTO			3001
#define ASM_COMBO_COUNT			3010
#define ASM_LIST_CODE			3020
#define ASM_BUTTON_OK			3030

erp_window_s			wAsm;
erp_object_edit_s		*eAsmAddress;
erp_object_combo_s		*cmbAsmCount;
erp_object_list_s		*lsAsmCode;
erp_object_button_s		*bAsmGoto, *bAsmOk;

ERP_CALLBACK(cbAsmComboCount);
ERP_CALLBACK(cbAsmListCode);
ERP_CALLBACK(cbAsmButtonGoto);
ERP_CALLBACK(cbAsmButtonOk);

int guiAsm(HWND parent, char *title, DWORD address, DWORD *selected, BOOL selAllow, DWORD selMin, DWORD selMax);


/* 批量设置断点 */
#define BTCH_LABEL_OFFSET		4000
#define BTCH_EDIT_OFFSET		4001
#define BTCH_LABEL_MOVE			4010
#define BTCH_CHECK_MOVE_STACK	4011
#define BTCH_CHECK_MOVE_CALL	4012
#define BTCH_CHECK_MOVE_JUMP	4013
#define BTCH_CHECK_MOVE_RET		4014
#define BTCH_BUTTON_OK			4020

erp_window_s			wBtch;
erp_object_label_s		*lbBtchOffset, *lbBtchMove;
erp_object_edit_s		*eBtchOffset;
erp_object_check_s		*cBtchMoveStack, *cBtchMoveCall, *cBtchMoveJump, *cBtchMoveRet;
erp_object_button_s		*bBtchOk;

ERP_CALLBACK(cbBtchButtonOk);

int guiBtch(HWND parent, DWORD *offset, BOOL *moveStack, BOOL *moveCall, BOOL *moveJump, BOOL *moveRet);


/* 劫持设置 */
#define HIJ_COMBO_MODULE		5000
#define HIJ_LIST_FUNCTION		5001
#define HIJ_BUTTON_ADD			5010
#define HIJ_BUTTON_PROTO		5020
#define HIJ_BUTTON_REMOVE		5021
#define HIJ_BUTTON_IMPORT		5030
#define HIJ_BUTTON_EXPORT		5031
#define HIJ_BUTTON_OK			5040

erp_window_s			wHij;
erp_object_combo_s		*cmbHijModule;
erp_object_list_s		*lsHijFunction;
erp_object_button_s		*bHijAdd, *bHijProto, *bHijRemove, *bHijImport, *bHijExport, *bHijOk;

ERP_CALLBACK(cbHijComboModule);
ERP_CALLBACK(cbHijListFunction);
ERP_CALLBACK(cbHijButtonAdd);
ERP_CALLBACK(cbHijButtonProto);
ERP_CALLBACK(cbHijButtonRemove);
ERP_CALLBACK(cbHijButtonImport);
ERP_CALLBACK(cbHijButtonExport);
ERP_CALLBACK(cbHijButtonOk);

void guiHij(HWND win);


/* 函数查找 */
#define FUNC_EDIT_NAME			6000
#define FUNC_CHECK_VAGUE		6001
#define FUNC_CHECK_SENCASE		6002
#define FUNC_COMBO_MODULE		6003
#define FUNC_BUTTON_SEARCH		6004
#define FUNC_LIST_RESULT		6010
#define FUNC_BUTTON_ADD			6011
#define FUNC_LIST_CONFIRM		6020
#define FUNC_BUTTON_REMOVE		6021

erp_window_s			wFunc;
erp_object_edit_s		*eFuncName;
erp_object_check_s		*cFuncVague, *cFuncSencase;
erp_object_combo_s		*cmbFuncModule;
erp_object_list_s		*lsFuncResult, *lsFuncConfirm;
erp_object_button_s		*bFuncSearch, *bFuncAdd, *bFuncRemove;

ERP_CALLBACK(cbFuncListResult);
ERP_CALLBACK(cbFuncListConfirm);
ERP_CALLBACK(cbFuncButtonSearch);
ERP_CALLBACK(cbFuncButtonAdd);
ERP_CALLBACK(cbFuncButtonRemove);

int guiFunc(HWND win);


/* 函数原型 */
#define PRT_LABEL_INVOKE		7000
#define PRT_RADIO_INVOKE_C		7001
#define PRT_RADIO_INVOKE_STD	7002
#define PRT_RADIO_INVOKE_FAST	7003
#define PRT_LABEL_RETURN		7010
#define PRT_COMBO_RETURN		7011
#define PRT_LABEL_BK			7020
#define PRT_EDIT_BK				7021
#define PRT_BUTTON_BK			7022
#define PRT_LABEL_ARGS			7030
#define PRT_LIST_ARGS			7031
#define PRT_BUTTON_ARGS_ADD		7032
#define PRT_BUTTON_ARGS_EDIT	7033
#define PRT_BUTTON_ARGS_DEL		7034
#define PRT_BUTTON_ARGS_MVUP	7035
#define PRT_BUTTON_ARGS_MVDOWN	7036
#define PRT_LABEL_MISC			7040
#define PRT_CHECK_VARARG		7041
#define PRT_CHECK_ESP			7042
#define PRT_BUTTON_OK			7050

erp_window_s			wPrt;
erp_object_label_s		*lbPrtInvoke, *lbPrtReturn, *lbPrtBk, *lbPrtArgs, *lbPrtMisc;
erp_object_radio_s		*rPrtInvokeC, *rPrtInvokeStd, *rPrtInvokeFast;
erp_object_combo_s		*cmbPrtReturn;
erp_object_edit_s		*ePrtBk;
erp_object_list_s		*lsPrtArgs;
erp_object_check_s		*cPrtVararg, *cPrtEsp;
erp_object_button_s		*bPrtBk, *bPrtArgsAdd, *bPrtArgsEdit, *bPrtArgsDel, *bPrtArgsMvUp, *bPrtArgsMvDown, *bPrtOk;

ERP_CALLBACK(cbPrtRadioInvokeC);
ERP_CALLBACK(cbPrtRadioInvokeStd);
ERP_CALLBACK(cbPrtRadioInvokeFast);
ERP_CALLBACK(cbPrtComboReturn);
ERP_CALLBACK(cbPrtButtonBk);
ERP_CALLBACK(cbPrtListArgs);
ERP_CALLBACK(cbPrtButtonArgsAdd);
ERP_CALLBACK(cbPrtButtonArgsEdit);
ERP_CALLBACK(cbPrtButtonArgsDel);
ERP_CALLBACK(cbPrtButtonArgsMvUp);
ERP_CALLBACK(cbPrtButtonArgsMvDown);
ERP_CALLBACK(cbPrtCheckVararg);
ERP_CALLBACK(cbPrtCheckEsp);
ERP_CALLBACK(cbPrtButtonOk);

void guiPrt(hijack_s *hij, HWND win);


/* 参数 */
#define ARG_LABEL_NAME			8000
#define ARG_EDIT_NAME			8001
#define ARG_LABEL_TYPE			8010
#define ARG_COMBO_TYPE			8011
#define ARG_BUTTON_OK			8020
#define ARG_BUTTON_CANCEL		8021

erp_window_s			wArg;
erp_object_label_s		*lbArgName, *lbArgType;
erp_object_edit_s		*eArgName;
erp_object_combo_s		*cmbArgType;
erp_object_button_s		*bArgOk, *bArgCancel;

ERP_CALLBACK(cbArgComboType);
ERP_CALLBACK(cbArgButtonOk);
ERP_CALLBACK(cbArgButtonCancel);

argument_s * guiArg(argument_s *argv, HWND win);


/* 主窗口 */
#define MAIN_BUTTON_SUSPEND		9000
#define MAIN_BUTTON_DUMP		9001
#define MAIN_BUTTON_HIJACK		9002
#define MAIN_BUTTON_REGISTER	9010
#define MAIN_BUTTON_DISASM		9011
#define MAIN_BUTTON_MEMORY		9012
#define MAIN_BUTTON_VIEWER		9013
#define MAIN_LIST_STACK			9020
#define MAIN_LABEL_MODULE		9030
#define MAIN_EDIT_MODULE		9031
#define MAIN_LABEL_FUNCTION		9032
#define MAIN_EDIT_ORDINAL		9033
#define MAIN_EDIT_FUNCNAME		9034
#define MAIN_LABEL_BP			9040
#define MAIN_EDIT_BP			9041
#define MAIN_LABEL_ESP			9042
#define MAIN_EDIT_ESP			9043
#define MAIN_LABEL_RET			9044
#define MAIN_EDIT_RET			9045
#define MAIN_LABEL_EAX			9046
#define MAIN_EDIT_EAX			9047
#define MAIN_BUTTON_CALC		9050
#define MAIN_BUTTON_FUNCTION	9051
#define MAIN_BUTTON_CONTINUE	9060
#define MAIN_BUTTON_RUNTORET	9061
#define MAIN_BUTTON_RUNTOCON	9062
#define MAIN_BUTTON_RETURN		9063
#define MAIN_BUTTON_ABOUT		9070

erp_window_s			wMain;
erp_object_list_s		*lsMainStack;
erp_object_label_s		*lbMainModule, *lbMainFunction, *lbMainBp, *lbMainEsp, *lbMainRet, *lbMainEax;
erp_object_edit_s		*eMainModule, *eMainOrdinal, *eMainFuncname, *eMainBp, *eMainEsp, *eMainRet, *eMainEax;
erp_object_button_s		*bMainSuspend, *bMainDump, *bMainHijack, *bMainRegister, *bMainDisasm, *bMainMemory, *bMainViewer, *bMainCalc, *bMainFunction, *bMainContinue, *bMainRuntoret, *bMainRuntocon, *bMainReturn, *bMainAbout;

ERP_CALLBACK(cbMainListStack);
ERP_CALLBACK(cbMainButtonSuspend);
ERP_CALLBACK(cbMainButtonDump);
ERP_CALLBACK(cbMainButtonHijack);
ERP_CALLBACK(cbMainButtonRegister);
ERP_CALLBACK(cbMainButtonDisasm);
ERP_CALLBACK(cbMainButtonMemory);
ERP_CALLBACK(cbMainButtonViewer);
ERP_CALLBACK(cbMainButtonCalc);
ERP_CALLBACK(cbMainButtonFunction);
ERP_CALLBACK(cbMainButtonContinue);
ERP_CALLBACK(cbMainButtonRuntoret);
ERP_CALLBACK(cbMainButtonRuntocon);
ERP_CALLBACK(cbMainButtonReturn);
ERP_CALLBACK(cbMainButtonAbout);

void guiMainTitle(char *module, char *function, size_t ordinal);
void guiMainStatus(BOOL suspend, BOOL proc, BOOL run);
void guiMain();


/* 寄存器 */
#define REG_LABEL_HEX			10000
#define REG_LABEL_DEC			10001
#define REG_LABEL_EAX			10010
#define REG_EDIT_EAX_HEX		10011
#define REG_EDIT_EAX_DEC		10012
#define REG_LABEL_ECX			10020
#define REG_EDIT_ECX_HEX		10021
#define REG_EDIT_ECX_DEC		10022
#define REG_LABEL_EDX			10030
#define REG_EDIT_EDX_HEX		10031
#define REG_EDIT_EDX_DEC		10032
#define REG_LABEL_EBX			10040
#define REG_EDIT_EBX_HEX		10041
#define REG_EDIT_EBX_DEC		10042
#define REG_LABEL_ESI			10050
#define REG_EDIT_ESI_HEX		10051
#define REG_EDIT_ESI_DEC		10052
#define REG_LABEL_EDI			10060
#define REG_EDIT_EDI_HEX		10061
#define REG_EDIT_EDI_DEC		10062
#define REG_LABEL_ESP			10070
#define REG_EDIT_ESP_HEX		10071
#define REG_EDIT_ESP_DEC		10072
#define REG_LABEL_EBP			10080
#define REG_EDIT_EBP_HEX		10081
#define REG_EDIT_EBP_DEC		10082
#define REG_LABEL_EIP			10090
#define REG_EDIT_EIP_HEX		10091
#define REG_EDIT_EIP_DEC		10092
#define REG_LABEL_CS			10100
#define REG_EDIT_CS_HEX			10101
#define REG_EDIT_CS_DEC			10102
#define REG_LABEL_DS			10110
#define REG_EDIT_DS_HEX			10111
#define REG_EDIT_DS_DEC			10112
#define REG_LABEL_SS			10120
#define REG_EDIT_SS_HEX			10121
#define REG_EDIT_SS_DEC			10122
#define REG_LABEL_ES			10130
#define REG_EDIT_ES_HEX			10131
#define REG_EDIT_ES_DEC			10132
#define REG_LABEL_FS			10140
#define REG_EDIT_FS_HEX			10141
#define REG_EDIT_FS_DEC			10142
#define REG_LABEL_GS			10150
#define REG_EDIT_GS_HEX			10151
#define REG_EDIT_GS_DEC			10152
#define REG_LABEL_EFLAGS		10160
#define REG_EDIT_EFLAGS			10161
#define REG_BUTTON_OK			10180

erp_window_s			wReg;
erp_object_label_s		*lbRegHex, *lbRegDec;
erp_object_label_s		*lbRegEax, *lbRegEcx, *lbRegEdx, *lbRegEbx, *lbRegEsi, *lbRegEdi, *lbRegEsp, *lbRegEbp, *lbRegEip;
erp_object_label_s		*lbRegCs, *lbRegDs, *lbRegSs, *lbRegEs, *lbRegFs, *lbRegGs, *lbRegEflags, *lbRegCr0;
erp_object_edit_s		*eRegEaxHex, *eRegEaxDec, *eRegEcxHex, *eRegEcxDec, *eRegEdxHex, *eRegEdxDec, *eRegEbxHex, *eRegEbxDec;
erp_object_edit_s		*eRegEsiHex, *eRegEsiDec, *eRegEdiHex, *eRegEdiDec, *eRegEspHex, *eRegEspDec, *eRegEbpHex, *eRegEbpDec, *eRegEipHex, *eRegEipDec;
erp_object_edit_s		*eRegCsHex, *eRegCsDec, *eRegDsHex, *eRegDsDec, *eRegSsHex, *eRegSsDec, *eRegEsHex, *eRegEsDec, *eRegFsHex, *eRegFsDec, *eRegGsHex, *eRegGsDec;
erp_object_edit_s		*eRegEflags;
erp_object_button_s		*bRegOk;

ERP_CALLBACK(cbRegButtonOk);

void guiReg(HWND win, CONTEXT *reg);


/* 内存窗口 */
#define MEM_EDIT_ADDRESS		11000
#define MEM_BUTTON_READ			11010
#define MEM_BUTTON_WRITE		11011
#define MEM_EDITAREA_DATA		11020
#define MEM_BUTTON_OK			11030

erp_window_s			wMem;
erp_object_edit_s		*eMemAddress;
erp_object_editarea_s	*eaMemData;
erp_object_button_s		*bMemRead, *bMemWrite, *bMemOk;

ERP_CALLBACK(cbMemButtonRead);
ERP_CALLBACK(cbMemButtonWrite);
ERP_CALLBACK(cbMemButtonOk);

void guiMem(HWND win, DWORD address);


/* 变量查看窗口 */
#define VIEW_EDIT_ADDRESS		12000
#define VIEW_BUTTON_ADDRESS		12001
#define VIEW_COMBO_TYPE			12010
#define VIEW_EDITAREA_DATA		12020
#define VIEW_EDITAREA_POINTER	12021
#define VIEW_BUTTON_OK			12030

erp_window_s			wView;
erp_object_edit_s		*eViewAddress;
erp_object_editarea_s	*eaViewData, *eaViewPointer;
erp_object_combo_s		*cmbViewType;
erp_object_button_s		*bViewAddress, *bViewOk;

ERP_CALLBACK(cbViewComboType);
ERP_CALLBACK(cbViewButtonAddress);
ERP_CALLBACK(cbViewButtonOk);

void guiView(HWND win, DWORD address, type_s *type);


#if defined(__cplusplus)
	}
#endif

#endif   /* _INVOKE_GUI_H_ */