
enum {
	MENUSYS_MAX			= 8
};

enum {
	SMSG_SETHIDE		= 0,
	SMSG_GETHIDE,
	SMSG_SETENABLE,
	SMSG_GETENABLE,
	SMSG_SETCHECK,
	SMSG_GETCHECK,
	SMSG_SETTEXT
};

enum {
	MENUS_POPUP			= 0x0010,
	MENUS_SYSTEM		= 0x0020,
	MENUS_CLOSE			= 0x0030,
	MENUS_CTRLMASK		= 0x0030
};

typedef struct _smi {
const		char	*string;
const struct _smi	*child;
			MENUID	id;
			MENUFLG	flag;
} MSYSITEM;


#ifdef __cplusplus
extern "C" {
#endif

BOOL menusys_create(const MSYSITEM *item, VRAMHDL icon,
								void (*cmd)(MENUID id), const char *title);
void menusys_destroy(void);

BOOL menusys_open(void);
void menusys_close(void);

void menusys_moving(int x, int y, int btn);

void *menusys_msg(int ctrl, MENUID id, void *arg);

#ifdef __cplusplus
}
#endif


// ---- MACRO

#define menusys_sethide(id, hide)		\
				menusys_msg(SMSG_SETHIDE, (id), (void *)(long)(hide))
#define menusys_gethide(id)				\
				(int)(menusys_msg(SMSG_GETHIDE, (id), NULL))

#define menusys_setenable(id, enable)	\
				menusys_msg(SMSG_SETENABLE, (id), (void *)(long)(enable))
#define menusys_getenable(id)			\
				(int)(menusys_msg(SMSG_GETENABLE, (id), NULL))

#define menusys_setcheck(id, checked)	\
				menusys_msg(SMSG_SETCHECK, (id), (void *)(long)(checked))
#define menusys_getcheck(id)			\
				(int)(menusys_msg(SMSG_GETCHECK, (id), NULL))

#define menusys_settext(id, str)		\
				menusys_msg(SMSG_SETTEXT, (id), (str))

