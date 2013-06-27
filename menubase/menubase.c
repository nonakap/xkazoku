#include	"compiler.h"
#include	"vram.h"
#include	"vramdraw.h"
#include	"menudeco.inc"
#include	"menubase.h"
#include	"scrnmng.h"
#include	"fontmng.h"
#include	"inputmng.h"
#include	"taskmng.h"


	VRAMHDL		menuvram;
	MENUBASE	menubase;


BOOL menubase_create(void) {

	MENUBASE	*mb;
	int			i;
	int			bpp;

	mb = &menubase;
	mb->font = fontmng_create(MENU_FONTSIZE, FDAT_PROPORTIONAL, NULL);
	bpp = SCREEN_BPP;
#ifdef SUPPORT_16BPP
	if (bpp == 16) {
		for (i=0; i<4; i++) {
			mb->icon[i] = menuvram_resload(menures_icon16 + i, 16);
		}
	}
#endif
#ifdef SUPPORT_24BPP
	if (bpp == 24) {
		for (i=0; i<4; i++) {
			mb->icon[i] = menuvram_resload(menures_icon24 + i, 24);
		}
	}
#endif
	return(SUCCESS);
}


void menubase_destroy(void) {

	MENUBASE	*mb;
	int			i;

	mb = &menubase;
	for (i=0; i<4; i++) {
		vram_destroy(mb->icon[i]);
	}
	fontmng_destroy(mb->font);
	ZeroMemory(mb, sizeof(MENUBASE));
}


BOOL menubase_open(int num) {

	MENUBASE	*mb;
	VRAMHDL		hdl;

	mb = &menubase;
	menubase_close();
	hdl = mainvram;
	if (hdl == NULL) {
		goto mbopn_err;
	}
	mb->width = hdl->width;
	mb->height = hdl->height;
	hdl = vram_create(mb->width, mb->height, TRUE, DEFAULT_BPP);
	menuvram = hdl;
	if (hdl == NULL) {
		goto mbopn_err;
	}
	unionrect_rst(&mb->rect);
	mb->num = num;
	return(SUCCESS);

mbopn_err:
	return(FAILURE);
}


void menubase_close(void) {

	MENUBASE	*mb;
	VRAMHDL		hdl;
	int			num;

	mb = &menubase;
	num = mb->num;
	if (num) {
		mb->num = 0;
		if (num == 1) {
			menusys_close();
		}
		else {
			menudlg_destroy();
		}
		hdl = menuvram;
		if (hdl) {
			menubase_draw(NULL, NULL);
			menuvram = NULL;
			vram_destroy(hdl);
		}
	}
}


void menubase_moving(int x, int y, int btn) {

	int		num;

	num = menubase.num;
	if (num == 1) {
		menusys_moving(x, y, btn);
	}
	else if (num) {
		menudlg_moving(x, y, btn);
	}
}


void menubase_setrect(VRAMHDL vram, const RECT_T *rect) {

	RECT_T	rct;

	if (vram) {
		if (rect == NULL) {
			vram_getrect(vram, &rct);
		}
		else {
			rct.left = vram->posx + rect->left;
			rct.top = vram->posy + rect->top;
			rct.right = vram->posx + rect->right;
			rct.bottom = vram->posy + rect->bottom;
		}
		unionrect_add(&menubase.rect, &rct);
	}
}


void menubase_clrrect(VRAMHDL vram) {

	RECT_T	rct;

	if (vram) {
		vram_getrect(vram, &rct);
		vram_fillalpha(menuvram, &rct, 1);
		menubase_setrect(vram, NULL);
//		movieredraw = 1;
	}
}


void menubase_draw(void (*draw)(VRAMHDL dst, const RECT_T *rect, void *arg),
																void *arg) {

	MENUBASE	*mb;
const	RECT_T	*rect;

	mb = &menubase;
	if (mb->rect.type) {
		rect = unionrect_get(&mb->rect);
		if (draw) {
			draw(menuvram, rect, arg);
		}
		scrnmng_menudraw(rect);
		unionrect_rst(&mb->rect);
	}
}


// ----

void menubase_modalproc(void) {

	int		x;
	int		y;
	UINT	btn;

	inputmng_resetmouse(0);
	while(taskmng_sleep(5)) {
		if (menuvram == NULL) {
			break;
		}
		btn = inputmng_getmouse(&x, &y);
		inputmng_resetmouse(0);
		if (btn & MOUSE_MOVEBIT) {
			menubase_moving(x, y, 0);
		}
		if (btn & LBUTTON_DOWNBIT) {
			menubase_moving(x, y, 1);
		}
		if (btn & LBUTTON_UPBIT) {
			menubase_moving(x, y, 2);
		}
	}
}

