
typedef unsigned short		MENUID;
typedef unsigned short		MENUFLG;

#include	"menusys.h"
#include	"menudlg.h"
#include	"menuvram.h"
#include	"menumbox.h"
#include	"menures.h"


enum {
	MENU_DISABLE		= 0x0001,
	MENU_GRAY			= 0x0002,
	MENU_CHECKED		= 0x0004,
	MENU_SEPARATOR		= 0x0008,
	MENU_REDRAW			= 0x2000,
	MENU_TABSTOP		= 0x4000,
	MENU_DELETED		= 0x8000,
	MENU_STYLE			= 0x0ff0
};

enum {
	DID_STATIC			= 0,
	DID_OK,
	DID_CANCEL,
	DID_ABORT,
	DID_RETRY,
	DID_IGNORE,
	DID_YES,
	DID_NO,
	DID_APPLY,
	DID_USER
};

enum {
	SID_CAPTION			= 0x7ffd,
	SID_MINIMIZE		= 0x7ffe,
	SID_CLOSE			= 0x7fff
};

typedef struct {
	int			num;
	void		*font;
	int			del;
	UNIRECT		rect;
	int			width;
	int			height;

	VRAMHDL		icon[4];
} MENUBASE;


#ifdef __cplusplus
extern "C" {
#endif

extern	VRAMHDL		menuvram;
extern	MENUBASE	menubase;

BOOL menubase_create(void);
void menubase_destroy(void);

BOOL menubase_open(int num);
void menubase_close(void);
BOOL menubase_moving(int x, int y, int btn);
BOOL menubase_key(UINT key);

void menubase_setrect(VRAMHDL vram, const RECT_T *rect);
void menubase_clrrect(VRAMHDL vram);
void menubase_draw(void (*draw)(VRAMHDL dst, const RECT_T *rect, void *arg),
																void *arg);

void menubase_proc(void);
void menubase_modalproc(void);

#ifdef __cplusplus
}
#endif

