/*
	$ Europa Win32 GUI Interface  (C) 2005-2012 mfboy
	$ europa.h, 0.1.1104
*/

#ifndef _EUROPA_H_
#define _EUROPA_H_

#if defined(__cplusplus)
	extern "C" {
#endif


/* 定位和尺寸类型 */
typedef signed short int erp_loc_t;
typedef unsigned short int erp_size_t;

/* 窗口类型 */
struct erp_window_s;


/* 控件类型 */
typedef enum { LABEL, BUTTON, EDIT, EDITAREA, RADIO, CHECK, COMBO, LIST } erp_object_e;

/* 控件事件类型 */
typedef enum { CLICK, SELECT, FOCUS } erp_event_e;


/* 控件基类型 */
typedef struct erp_object_s {
	/* 类型 */
	erp_object_e type;

	/* 控件 ID 和句柄 */
	WORD	id;
	HWND	hwnd;

	/* 位置和大小 */
	erp_loc_t	x;
	erp_loc_t	y;
	erp_size_t	width;
	erp_size_t	height;

	/* 基本属性 */
	BOOL	hidden;
	BOOL	disabled;
	char	*text;

	/* 禁用 */
	void (*hide)(BOOL hidden);
	void (*disable)(BOOL disabled);
	void (*caption)(char *text);

	/* 回调函数 */
	void (*callback)(struct erp_window_s *, struct erp_object_s *, erp_event_e, DWORD arg);

	/* 链表 */
	struct erp_object_s *next;
} erp_object_s;


/* 标签 */
typedef struct {
	erp_object_s object;
} erp_object_label_s;

/* 按钮 */
typedef struct {
	erp_object_s object;

	/* 单击 */
	void (*click)();
} erp_object_button_s;

/* 单行文本框 */
typedef struct {
	erp_object_s object;

	/* 只读 */
	void (*readonly)(BOOL readonly);

	/* 设置及获取文本 */
	void (*setText)(char *format, ...);
	char * (*getText)();
} erp_object_edit_s;

/* 多行文本框 */
typedef struct {
	erp_object_s object;

	/* 只读 */
	void (*readonly)(BOOL readonly);

	/* 设置文字 */
	void (*setText)(char *format, ...);
	void (*appendText)(char *format, ...);
	/* 获取文字 */
	char * (*getText)();
} erp_object_editarea_s;

/* 单选框 */
typedef struct {
	erp_object_s object;

	/* 分组 */
	int		group;
	/* 已选中 */
	BOOL	selected;

	/* 选择 */
	void (*select)();
} erp_object_radio_s;

/* 复选框 */
typedef struct {
	erp_object_s object;

	/* 已选中 */
	BOOL checked;

	/* 选择 */
	void (*check)(BOOL checked);
} erp_object_check_s;

/* 组合框 */
typedef struct {
	erp_object_s object;

	/* 选中项 */
	int		selected;
	/* 选项数 */
	size_t	count;

	/* 添加、修改或删除某项 */
	int (*append)(char *text);
	void (*remove)(int index);
	void (*clear)();

	/* 选择 */
	void (*select)(int index);
} erp_object_combo_s;

/* 列表框 */
typedef struct {
	erp_object_s object;

	/* 列 */
	char	**colTitle;
	size_t	*colWidth;
	size_t	colCount;

	/* 选中项 */
	int		selected;
	/* 列表项数 */
	size_t	count;

	/* 添加、修改或删除某项 */
	int (*append)(char **data);
	void (*modify)(int index, char **data);
	void (*remove)(int index);
	void (*clear)();

	/* 选择 */
	void (*select)(int index);
} erp_object_list_s;


/* 窗口 */
typedef struct erp_window_s {
	HWND		parent;
	HWND		hwnd;
	HINSTANCE	instance;

	char		*classname;

	/* 内部信息 */
	struct {
		BOOL	initiated;		/* 初始化完成 */
		DWORD	quit;			/* 退出信息 */
	} s;

	/* 窗口大小 */
	erp_size_t	width;
	erp_size_t	height;

	/* 标题 */
	char		*title;

	/* 工具窗口 */
	BOOL		tool;
	/* 居中显示 */
	BOOL		center;

	/* 参数传递 */
	DWORD		param;
	/* 取消操作 */
	BOOL		cancel;

	/* 回调函数 */
	void (*draw)(struct erp_window_s *);
	void (*loop)(struct erp_window_s *, HWND, UINT, WPARAM, LPARAM);
	void (*keyboard)(struct erp_window_s *, DWORD key);
	void * (*destroy)(struct erp_window_s *, DWORD arg);

	/* 操作 */
	void (*quit)(DWORD arg);
	void (*hidden)(BOOL hidden);
	void (*disable)(BOOL disable);
	void (*caption)(char *format, ...);

	/* 控件 */
	erp_object_s *object;
} erp_window_s;



#if defined(_EUROPA_)
	/* 窗口 */
	static void erpWindowQuit(DWORD arg);
	static void erpWindowHidden(BOOL hidden);
	static void erpWindowDisable(BOOL disable);
	static void erpWindowCaption(char *format, ...);

	/* 基类型 */
	static void erpObjectHide(BOOL hidden);
	static void erpObjectDisable(BOOL disabled);
	static void erpObjectCaption(char *text);

	/* 按钮 */
	static void erpButtonClick();

	/* 单行文本框 */
	static void erpEditReadonly(BOOL readonly);
	static void erpEditSetText(char *format, ...);
	static char * erpEditGetText();

	/* 多行文本框 */
	static void erpEditareaAppendText(char *format, ...);

	/* 单选框 */
	static void erpRadioSelect();

	/* 复选框 */
	static void erpCheckCheck(BOOL checked);

	/* 组合框 */
	static int erpComboAppend(char *text);
	static void erpComboRemove(int index);
	static void erpComboClear();
	static void erpComboSelect(int index);

	/* 列表框 */
	static int erpListAppend(char **data);
	static void erpListModify(int index, char **data);
	static void erpListRemove(int index);
	static void erpListClear();
	static void erpListSelect(int index);
#else
	/* this 指针 */
	extern erp_object_s *erpThis;
#endif


/* 初始化 */
void erpInit();
/* 运行窗口 */
void * erpWindow(erp_window_s *win);
/* 控件绘制 */
erp_object_s * erpDraw(erp_window_s *win, erp_object_e type, WORD id, void *callback, erp_loc_t x, erp_loc_t y, erp_size_t width, erp_size_t height, ...);



/* 回调函数 */
#define ERP_CALLBACK(func) void func(erp_window_s *win, erp_object_s *obj, erp_event_e evt, DWORD arg)


/* 类型转换 */
#define P_OBJECT(p)		((erp_object_s *)			(p))
#define P_LABEL(p)		((erp_object_label_s *)		(p))
#define P_BUTTON(p)		((erp_object_button_s *)	(p))
#define P_EDIT(p)		((erp_object_edit_s *)		(p))
#define P_EDITAREA(p)	((erp_object_editarea_s *)	(p))
#define P_RADIO(p)		((erp_object_radio_s *)		(p))
#define P_CHECK(p)		((erp_object_check_s *)		(p))
#define P_COMBO(p)		((erp_object_combo_s *)		(p))
#define P_LIST(p)		((erp_object_list_s *)		(p))


/* 控件调用 */
#define OBJECT(p)		P_OBJECT(erpThis = (erp_object_s *) p)
#define LABEL(p)		P_LABEL(erpThis = (erp_object_s *) p)
#define BUTTON(p)		P_BUTTON(erpThis = (erp_object_s *) p)
#define EDIT(p)			P_EDIT(erpThis = (erp_object_s *) p)
#define EDITAREA(p)		P_EDITAREA(erpThis = (erp_object_s *) p)
#define RADIO(p)		P_RADIO(erpThis = (erp_object_s *) p)
#define CHECK(p)		P_CHECK(erpThis = (erp_object_s *) p)
#define COMBO(p)		P_COMBO(erpThis = (erp_object_s *) p)
#define LIST(p)			P_LIST(erpThis = (erp_object_s *) p)


/* 控件绘制 */
#if defined(ERP_WINDOW)
	#define ERP_DRAW(type, tid, id, cb, x, y, w, h, ...) \
		(erp_object_##type##_s *) erpDraw(&ERP_WINDOW, tid, id, cb, x, y, w, h, __VA_ARGS__)

	#define DRAW_LABEL(id, x, y, w, h, t)				ERP_DRAW(label,		LABEL,		id, NULL,	x, y, w, h, t)
	#define DRAW_BUTTON(id, x, y, w, h, t, cb)			ERP_DRAW(button,	BUTTON,		id, cb,		x, y, w, h, t)
	#define DRAW_EDIT(id, x, y, w, h, t)				ERP_DRAW(edit,		EDIT,		id, NULL,	x, y, w, h, t)
	#define DRAW_EDITAREA(id, x, y, w, h, t)			ERP_DRAW(editarea,	EDITAREA,	id, NULL,	x, y, w, h, t)
	#define DRAW_RADIO(id, x, y, w, h, t, g, cb)		ERP_DRAW(radio,		RADIO,		id, cb,		x, y, w, h, t, g)
	#define DRAW_CHECK(id, x, y, w, h, t, cb)			ERP_DRAW(check,		CHECK,		id, cb,		x, y, w, h, t)
	#define DRAW_COMBO(id, x, y, w, h, cb)				ERP_DRAW(combo,		COMBO,		id, cb,		x, y, w, h)
	#define DRAW_LIST(id, x, y, w, h, ct, cw, cc, cb)	ERP_DRAW(list,		LIST,		id, cb,		x, y, w, h, ct, cw, cc)
#else
	#define ERP_DRAW(win, type, tid, id, cb, x, y, w, h, ...) \
		(erp_object_##type##_s *) erpDraw(&win, tid, id, cb, x, y, w, h, __VA_ARGS__)

	#define DRAW_LABEL(win, id, x, y, w, h, t)				ERP_DRAW(win, label,	LABEL,		id, NULL,	x, y, w, h, t)
	#define DRAW_BUTTON(win, id, x, y, w, h, t, cb)			ERP_DRAW(win, button,	BUTTON,		id, cb,		x, y, w, h, t)
	#define DRAW_EDIT(win, id, x, y, w, h, t)				ERP_DRAW(win, edit,		EDIT,		id, NULL,	x, y, w, h, t)
	#define DRAW_EDITAREA(win, id, x, y, w, h, t)			ERP_DRAW(win, editarea,	EDITAREA,	id, NULL,	x, y, w, h, t)
	#define DRAW_RADIO(win, id, x, y, w, h, t, g, cb)		ERP_DRAW(win, radio,	RADIO,		id, cb,		x, y, w, h, t, g)
	#define DRAW_CHECK(win, id, x, y, w, h, t, cb)			ERP_DRAW(win, check,	CHECK,		id, cb,		x, y, w, h, t)
	#define DRAW_COMBO(win, id, x, y, w, h, cb)				ERP_DRAW(win, combo,	COMBO,		id, cb,		x, y, w, h)
	#define DRAW_LIST(win, id, x, y, w, h, ct, cw, cc, cb)	ERP_DRAW(win, list,		LIST,		id, cb,		x, y, w, h, ct, cw, cc)
#endif


#if defined(__cplusplus)
	}
#endif

#endif   /* _EUROPA_H_ */