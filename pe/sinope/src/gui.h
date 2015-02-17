/*
	$ Sinope PE Loader  (C) 2005-2012 mfboy
	$ gui.h, 0.1.1104
*/

#ifndef _SNP_GUI_H_
#define _SNP_GUI_H_

#include "europa.h"

#if defined(__cplusplus)
	extern "C" {
#endif


#if defined(_SNP_GUI_)
	#define _SNP_GUI_EXTERN_
#else
	#define _SNP_GUI_EXTERN_ extern
#endif


/* 主窗口 */
#define WIN_TITLE				"Sinope PE Loader"
#define WIN_CLASS				"sinope_window_main"
#define WIN_WIDTH				1050
#define WIN_HEIGHT				700


/* 功能区 */
#define BUTTON_EXECUTE			1001
#define BUTTON_DUMP				1002
#define BUTTON_SUSPEND			1003
#define BUTTON_CONTINUE			1004
#define BUTTON_SINGLE_STEP		1005
#define BUTTON_REFRESH			1006
#define BUTTON_REGISTER			1007
#define BUTTON_CONFIG			1008
#define BUTTON_ABOUT			1009

ERP_CALLBACK(cbButtonExecute);
ERP_CALLBACK(cbButtonDump);
ERP_CALLBACK(cbButtonSuspend);
ERP_CALLBACK(cbButtonContinue);
ERP_CALLBACK(cbButtonSingleStep);
ERP_CALLBACK(cbButtonRefresh);
ERP_CALLBACK(cbButtonRegister);
ERP_CALLBACK(cbButtonConfig);
ERP_CALLBACK(cbButtonAbout);

_SNP_GUI_EXTERN_ erp_object_button_s *bExecute, *bDump, *bSuspend, *bContinue, *bSingleStep, *bRefresh, *bRegister, *bConfig, *bAbout;


/* 指令区 */
#define LABEL_RVA				1010
#define LABEL_EIP				1011
#define LABEL_SECTION			1012
#define LABEL_CODE				1013
#define EDIT_RVA				1014
#define EDIT_EIP				1015
#define EDIT_SECTION			1016
#define EDIT_CODE				1017

_SNP_GUI_EXTERN_ erp_object_label_s	*lbRva, *lbEip, *lbSection, *lbCode;
_SNP_GUI_EXTERN_ erp_object_edit_s	*eRva, *eEip, *eSection, *eCode;


/* 线程选择器 */
#define COMBO_THREAD			1020

ERP_CALLBACK(cbComboThread);

_SNP_GUI_EXTERN_ erp_object_combo_s *cmbThread;


/* 断点区 */
#define LIST_BP					1030
#define BUTTON_BP_ADD			1031
#define BUTTON_BP_EDIT			1032
#define BUTTON_BP_DELETE		1033
#define CHECK_BP_DYNM_CODE		1034
#define CHECK_BP_DYNM_MEMORY	1035
#define CHECK_BP_STRIDE			1036
#define RADIO_BP_ALWAYS			1037
#define RADIO_BP_STRIDE			1038
#define RADIO_BP_RETURN			1039

ERP_CALLBACK(cbListBp);
ERP_CALLBACK(cbButtonBpAdd);
ERP_CALLBACK(cbButtonBpEdit);
ERP_CALLBACK(cbButtonBpDelete);
ERP_CALLBACK(cbCheckBpDynmCode);
ERP_CALLBACK(cbCheckBpDynmMemory);
ERP_CALLBACK(cbCheckBpStride);
ERP_CALLBACK(cbRadioBpAlways);
ERP_CALLBACK(cbRadioBpStride);
ERP_CALLBACK(cbRadioBpReturn);

_SNP_GUI_EXTERN_ erp_object_list_s		*lsBp;
_SNP_GUI_EXTERN_ erp_object_button_s	*bBpAdd, *bBpEdit, *bBpDelete;
_SNP_GUI_EXTERN_ erp_object_check_s		*cBpDynmCode, *cBpDynmMemory, *cBpStride;
_SNP_GUI_EXTERN_ erp_object_radio_s		*rBpAlways, *rBpStride, *rBpReturn;


/* 区段区 */
#define LIST_SECTION			1040
#define BUTTON_SECTION_EXPORT	1041
#define BUTTON_SECTION_IMPORT	1042
#define BUTTON_SECTION_BP		1043
#define BUTTON_SECTION_ALLOC	1044

ERP_CALLBACK(cbListSection);
ERP_CALLBACK(cbButtonSectionExport);
ERP_CALLBACK(cbButtonSectionImport);
ERP_CALLBACK(cbButtonSectionBp);
ERP_CALLBACK(cbButtonSectionAlloc);

_SNP_GUI_EXTERN_ erp_object_list_s		*lsSection;
_SNP_GUI_EXTERN_ erp_object_button_s	*bSectionExport, *bSectionImport, *bSectionBp, *bSectionAlloc;


/* 导入表区 */
#define LIST_IMPORT				1050
#define BUTTON_IMPORT_EDIT		1051
#define BUTTON_IMPORT_FIX		1052
#define BUTTON_IMPORT_BP		1053
#define BUTTON_IMPORT_REBUILD	1054

ERP_CALLBACK(cbListImport);
ERP_CALLBACK(cbButtonImportEdit);
ERP_CALLBACK(cbButtonImportFix);
ERP_CALLBACK(cbButtonImportBp);
ERP_CALLBACK(cbButtonImportRebuild);

_SNP_GUI_EXTERN_ erp_object_list_s		*lsImport;
_SNP_GUI_EXTERN_ erp_object_button_s	*bImportEdit, *bImportFix, *bImportBp, *bImportRebuild;


/* 模块区 */
#define LIST_MODULE				1060
#define BUTTON_MODULE_EXPORT	1061
#define BUTTON_MODULE_IMPORT	1062
#define BUTTON_MODULE_BP		1063

ERP_CALLBACK(cbListModule);
ERP_CALLBACK(cbButtonModuleExport);
ERP_CALLBACK(cbButtonModuleImport);
ERP_CALLBACK(cbButtonModuleBp);

_SNP_GUI_EXTERN_ erp_object_list_s		*lsModule;
_SNP_GUI_EXTERN_ erp_object_button_s	*bModuleExport, *bModuleImport, *bModuleBp;


/* 工作区 */
#define EDITAREA_WORKING		1070

_SNP_GUI_EXTERN_ erp_object_editarea_s	*eaWorking;

#define WORKING_SET(format, ...)			EDITAREA(eaWorking)->setText(format, __VA_ARGS__)
#define WORKING_APPEND(format, ...)			EDITAREA(eaWorking)->appendText(format, __VA_ARGS__)
#define WORKING_SET_ALERT(format, ...)		WORKING_SET(format, __VA_ARGS__); warningFormat(format, __VA_ARGS__);
#define WORKING_APPEND_ALERT(format, ...)	WORKING_APPEND(format, __VA_ARGS__); warningFormat(format, __VA_ARGS__);


/* Dump 窗口 */
#define DUMP_TITLE				"保存内存镜像"
#define DUMP_CLASS				"sinope_window_dump"
#define DUMP_WIDTH				450
#define DUMP_HEIGHT				280

#define DUMP_LABEL_EXPORT			2000
#define DUMP_CHECK_REAL_SIZE		2001
#define DUMP_CHECK_SECTION_ALIGN	2002
#define DUMP_CHECK_COPY				2003
#define DUMP_LABEL_IMPORT			2010
#define DUMP_CHECK_FILL_IAT			2011
#define DUMP_BUTTON_OK				2020

ERP_CALLBACK(cbDumpButonOk);

_SNP_GUI_EXTERN_ erp_object_label_s *lbDumpExport, *lbDumpImport;
_SNP_GUI_EXTERN_ erp_object_check_s *cDumpRealSize, *cDumpSectionAlign, *cDumpCopy, *cDumpFillIat;
_SNP_GUI_EXTERN_ erp_object_button_s *bDumpOk;


/* 寄存器窗口 */
#define REGISTER_TITLE			"寄存器"
#define REGISTER_CLASS			"sinope_window_register"
#define REGISTER_WIDTH			380
#define REGISTER_HEIGHT			600

#define REGISTER_LABEL_HEX		3000
#define REGISTER_LABEL_DEC		3001
#define REGISTER_LABEL_EAX		3010
#define REGISTER_EDIT_EAX_HEX	3011
#define REGISTER_EDIT_EAX_DEC	3012
#define REGISTER_LABEL_ECX		3020
#define REGISTER_EDIT_ECX_HEX	3021
#define REGISTER_EDIT_ECX_DEC	3022
#define REGISTER_LABEL_EDX		3030
#define REGISTER_EDIT_EDX_HEX	3031
#define REGISTER_EDIT_EDX_DEC	3032
#define REGISTER_LABEL_EBX		3040
#define REGISTER_EDIT_EBX_HEX	3041
#define REGISTER_EDIT_EBX_DEC	3042
#define REGISTER_LABEL_ESI		3050
#define REGISTER_EDIT_ESI_HEX	3051
#define REGISTER_EDIT_ESI_DEC	3052
#define REGISTER_LABEL_EDI		3060
#define REGISTER_EDIT_EDI_HEX	3061
#define REGISTER_EDIT_EDI_DEC	3062
#define REGISTER_LABEL_ESP		3070
#define REGISTER_EDIT_ESP_HEX	3071
#define REGISTER_EDIT_ESP_DEC	3072
#define REGISTER_LABEL_EBP		3080
#define REGISTER_EDIT_EBP_HEX	3081
#define REGISTER_EDIT_EBP_DEC	3082
#define REGISTER_LABEL_EIP		3090
#define REGISTER_EDIT_EIP_HEX	3091
#define REGISTER_EDIT_EIP_DEC	3092
#define REGISTER_LABEL_CS		3100
#define REGISTER_EDIT_CS_HEX	3101
#define REGISTER_EDIT_CS_DEC	3102
#define REGISTER_LABEL_DS		3110
#define REGISTER_EDIT_DS_HEX	3111
#define REGISTER_EDIT_DS_DEC	3112
#define REGISTER_LABEL_SS		3120
#define REGISTER_EDIT_SS_HEX	3121
#define REGISTER_EDIT_SS_DEC	3122
#define REGISTER_LABEL_ES		3130
#define REGISTER_EDIT_ES_HEX	3131
#define REGISTER_EDIT_ES_DEC	3132
#define REGISTER_LABEL_FS		3140
#define REGISTER_EDIT_FS_HEX	3141
#define REGISTER_EDIT_FS_DEC	3142
#define REGISTER_LABEL_GS		3150
#define REGISTER_EDIT_GS_HEX	3151
#define REGISTER_EDIT_GS_DEC	3152
#define REGISTER_LABEL_EFLAGS	3160
#define REGISTER_EDIT_EFLAGS	3161
#define REGISTER_BUTTON_OK		3180

ERP_CALLBACK(cbRegisterButtonOk);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbRegisterHex, *lbRegisterDec;
_SNP_GUI_EXTERN_ erp_object_label_s		*lbRegisterEax, *lbRegisterEcx, *lbRegisterEdx, *lbRegisterEbx, *lbRegisterEsi, *lbRegisterEdi, *lbRegisterEsp, *lbRegisterEbp, *lbRegisterEip;
_SNP_GUI_EXTERN_ erp_object_label_s		*lbRegisterCs, *lbRegisterDs, *lbRegisterSs, *lbRegisterEs, *lbRegisterFs, *lbRegisterGs, *lbRegisterEflags, *lbRegisterCr0;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eRegisterEaxHex, *eRegisterEaxDec, *eRegisterEcxHex, *eRegisterEcxDec, *eRegisterEdxHex, *eRegisterEdxDec, *eRegisterEbxHex, *eRegisterEbxDec;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eRegisterEsiHex, *eRegisterEsiDec, *eRegisterEdiHex, *eRegisterEdiDec, *eRegisterEspHex, *eRegisterEspDec, *eRegisterEbpHex, *eRegisterEbpDec, *eRegisterEipHex, *eRegisterEipDec;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eRegisterCsHex, *eRegisterCsDec, *eRegisterDsHex, *eRegisterDsDec, *eRegisterSsHex, *eRegisterSsDec, *eRegisterEsHex, *eRegisterEsDec, *eRegisterFsHex, *eRegisterFsDec, *eRegisterGsHex, *eRegisterGsDec;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eRegisterEflags;
_SNP_GUI_EXTERN_ erp_object_button_s	*bRegisterOk;


/* 设置窗口 */
#define CONFIG_TITLE			"设置"
#define CONFIG_CLASS			"sinope_window_class"
#define CONFIG_WIDTH			480
#define CONFIG_HEIGHT			330

#define CONFIG_CHECK_DISASM		4000
#define CONFIG_CHECK_MODBASE	4001
#define CONFIG_CHECK_BEEP		4002
#define CONFIG_CHECK_DLLEIP		4003
#define CONFIG_LABEL_SEC		4020
#define CONFIG_RADIO_SEC_IMAGE	4021
#define CONFIG_RADIO_SEC_RAW	4022
#define CONFIG_LABEL_IAT		4030
#define CONFIG_RADIO_IAT_RVA	4031
#define CONFIG_RADIO_IAT_ADDR	4032
#define CONFIG_RADIO_IAT_OFFSET	4033
#define CONFIG_BUTTON_OK		4040
#define CONFIG_BUTTON_CANCEL	4041

ERP_CALLBACK(cbConfigButtonOk);
ERP_CALLBACK(cbConfigButtonCancel);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbConfigSec, *lbConfigIat;
_SNP_GUI_EXTERN_ erp_object_check_s		*cConfigDisasm, *cConfigModBase, *cConfigBeep, *cConfigDllEip;
_SNP_GUI_EXTERN_ erp_object_radio_s		*rConfigSecImage, *rConfigSecRaw, *rConfigIatRva, *rConfigIatAddr, *rConfigIatOffset;
_SNP_GUI_EXTERN_ erp_object_button_s	*bConfigOk, *bConfigCancel;


/* 关于窗口 */
#define ABOUT_TITLE				"关于"
#define ABOUT_CLASS				"sinope_window_about"
#define ABOUT_WIDTH				600
#define ABOUT_HEIGHT			220

#define ABOUT_LABEL_PROGRAM		4000
#define ABOUT_LABEL_COMMENT		4001
#define ABOUT_LABEL_AUTHOR		4002
#define ABOUT_LABEL_VERSION		4003
#define ABOUT_BUTTON_OK			4010

ERP_CALLBACK(cbAboutButtonOk);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbAboutProgram, *lbAboutComment, *lbAboutAuthor, *lbAboutVersion;
_SNP_GUI_EXTERN_ erp_object_button_s	*bAboutOk;


/* 断点添加编辑 */
#define BP_TITLE_ADD			"添加断点"
#define BP_TITLE_EDIT			"修改断点"
#define BP_CLASS				"sinope_window_bp"
#define BP_WIDTH				400
#define BP_HEIGHT				270

#define BP_LABEL_FROM			5000
#define BP_EDIT_FROM			5001
#define BP_LABEL_TO				5010
#define BP_EDIT_TO				5011
#define BP_LABEL_TYPE			5020
#define BP_RADIO_TYPE_RVA		5021
#define BP_RADIO_TYPE_ADDR		5022
#define BP_LABEL_COMMENT		5030
#define BP_BUTTON_OK			5040
#define BP_BUTTON_CANCEL		5041

ERP_CALLBACK(cbBpButtonOk);
ERP_CALLBACK(cbBpButtonCancel);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbBpFrom, *lbBpTo, *lbBpType, *lbBpComment;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eBpFrom, *eBpTo;
_SNP_GUI_EXTERN_ erp_object_radio_s		*rBpTypeRva, *rBpTypeAddr;
_SNP_GUI_EXTERN_ erp_object_button_s	*bBpOk, *bBpCancel;


/* 申请动态内存 */
#define SECTION_TITLE			"申请动态内存"
#define SECTION_CLASS			"sinope_window_section"
#define SECTION_WIDTH			400
#define SECTION_HEIGHT			250

#define SECTION_LABEL_NAME		6000
#define SECTION_EDIT_NAME		6001
#define SECTION_LABEL_SIZE		6010
#define SECTION_EDIT_SIZE		6011
#define SECTION_LABEL_COMMENT	6020
#define SECTION_BUTTON_OK		6030
#define SECTION_BUTTON_CANCEL	6031

ERP_CALLBACK(cbSectionButtonOk);
ERP_CALLBACK(cbSectionButtonCancel);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbSectionName, *lbSectionSize, *lbSectionComment;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eSectionName, *eSectionSize;
_SNP_GUI_EXTERN_ erp_object_button_s	*bSectionOk, *bSectionCancel;


/* 编辑导入表函数入口 */
#define IMPORT_TITLE			"编辑入口"
#define IMPORT_CLASS			"sinope_window_import"
#define IMPORT_WIDTH			400
#define IMPORT_HEIGHT			190

#define IMPORT_LABEL_TIP		7000
#define IMPORT_EDIT_ADDR		7001
#define IMPORT_CHECK_RVA		7010
#define IMPORT_BUTTON_OK		7020
#define IMPORT_BUTTON_CANCEL	7021

ERP_CALLBACK(cbImportButtonOk);
ERP_CALLBACK(cbImportButtonCancel);

_SNP_GUI_EXTERN_ erp_object_label_s		*lbImportTip;
_SNP_GUI_EXTERN_ erp_object_edit_s		*eImportAddr;
_SNP_GUI_EXTERN_ erp_object_check_s		*cImportRva;
_SNP_GUI_EXTERN_ erp_object_button_s	*bImportOk, *bImportCancel;


/* 窗口对象 */
_SNP_GUI_EXTERN_ erp_window_s wMain;
_SNP_GUI_EXTERN_ erp_window_s wDump, wRegister, wConfig, wAbout;
_SNP_GUI_EXTERN_ erp_window_s wBp, wSection, wImport;


/* 窗口函数 */
#if defined(_SNP_GUI_)
	static int gui(HINSTANCE instance);
	static void winDraw(erp_window_s *win);
	static void winKeyboard(erp_window_s *win, DWORD key);

	static void guiDumpDraw(erp_window_s *win);
	static void * guiDumpDestory(erp_window_s *win, DWORD arg);

	static void guiRegisterDraw(erp_window_s *win);

	static void guiConfigDraw(erp_window_s *win);
	static void * guiConfigDestory(erp_window_s *win, DWORD arg);

	static void guiBpDraw(erp_window_s *win);
	static void * guiBpDestory(erp_window_s *win, DWORD arg);

	static void * guiSectionDestory(erp_window_s *win, DWORD arg);

	static void guiImportDraw(erp_window_s *win);
	static void * guiImportDestory(erp_window_s *win, DWORD arg);
#endif


/* 控件状态 */
void enable(BOOL execute, BOOL suspend, BOOL command, BOOL list);

/* 子窗口 */
BOOL guiDump();
void guiRegister(CONTEXT *reg);
void guiConfig();
void guiAbout();
BOOL guiBp(DWORD *from, DWORD *to, BOOL *rva, void *func);
void guiSection(void *func);
void guiImport(DWORD addr, void *func);


#if defined(__cplusplus)
	}
#endif

#endif   /* _SNP_GUI_H_ */