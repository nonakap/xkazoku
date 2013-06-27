#include	"compiler.h"
#include	"gamecore.h"
#include	"scrnmng.h"


// gamecore.cpp から切り出し

	VRAMHDL		mainvram = NULL;


BOOL vramdraw_create(void) {

	SYS_T	*sys;
	VRAMHDL	v;

	sys = &gamecore.sys;
#ifndef SIZE_QVGA
	v = vram_create(sys->width, sys->height, FALSE, DEFAULT_BPP);
#else
	v = vram_create(320, 240, FALSE, DEFAULT_BPP);
#endif
	mainvram = v;
	if (v == NULL) {
		TRACEOUT(("couldn't create mainvram"));
		return(FAILURE);
	}

	// vram 0と1は最初から開いてます…
#ifndef SIZE_QVGA
	gamecore.vram[0] = vram_create(sys->width, sys->height,
														FALSE, DEFAULT_BPP);
	gamecore.vram[1] = vram_create(sys->width, sys->height,
														FALSE, DEFAULT_BPP);
#else
	gamecore.vram[0] = vram_create(320, 240, FALSE, DEFAULT_BPP);
	gamecore.vram[1] = vram_create(320, 240, FALSE, DEFAULT_BPP);
#endif

	unionrect_rst(&gamecore.vramrect);
	return(SUCCESS);
}

void vramdraw_destroy(void) {

	int		i;

	for (i=0; i<GAMECORE_MAXVRAM; i++) {
		vram_destroy(gamecore.vram[i]);
	}

	vram_destroy(mainvram);
	mainvram = NULL;
}


// ----

int vramdraw_half(int size) {

#ifndef SIZE_QVGA
	return(size);
#else
	return((size + 1) >> 1);
#endif
}

void vramdraw_halfpoint(POINT_T *pt) {

#ifndef SIZE_QVGA
	(void)pt;
#else
	pt->x >>= 1;
	pt->y >>= 1;
#endif
}

void vramdraw_halfsize(POINT_T *pt) {

#ifndef SIZE_QVGA
	(void)pt;
#else
	pt->x = (pt->x + 1) >> 1;
	pt->y = (pt->y + 1) >> 1;
#endif
}

void vramdraw_halfscrn(SCRN_T *scrn) {

#ifndef SIZE_QVGA
	(void)scrn;
#else
	scrn->left >>= 1;
	scrn->top >>= 1;
	scrn->width = (scrn->width + 1) >> 1;
	scrn->height = (scrn->height + 1) >> 1;
#endif
}

void vramdraw_halfrect(RECT_T *rct) {

#ifndef SIZE_QVGA
	(void)rct;
#else
	rct->left >>= 1;
	rct->top >>= 1;
	rct->right = (rct->right + 1) >> 1;
	rct->bottom = (rct->bottom + 1) >> 1;
#endif
}

void vramdraw_scrn2rect(SCRN_T *scs, RECT_T *rct) {

	int		tmp1;
	int		tmp2;

	tmp1 = scs->left;
	tmp2 = scs->width;
#ifdef SIZE_QVGA
	tmp2 = (tmp1 + tmp2 + 1) >> 1;
	tmp1 >>= 1;
	tmp2 -= tmp1;
#endif
	rct->left = tmp1;
	rct->right = tmp1 + tmp2;

	tmp1 = scs->top;
	tmp2 = scs->height;
#ifdef SIZE_QVGA
	tmp2 = (tmp1 + tmp2 + 1) >> 1;
	tmp1 >>= 1;
	tmp2 -= tmp1;
#endif
	rct->top = tmp1;
	rct->bottom = tmp1 + tmp2;
}


// ----

void vramdraw_setrect(VRAMHDL vram, const RECT_T *rect) {

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
		unionrect_add(&gamecore.vramrect, &rct);
	}
}


void vramdraw_drawvram(VRAMHDL vram, const RECT_T *rect) {

	DISPWIN		dispwin;
	TEXTWIN		textwin;
	GAMECFG		gc;
	int			i;

	vram_zerofill(vram, rect);

	dispwin = &gamecore.dispwin;
	if (!(dispwin->flag & DISPWIN_VRAM)) {
		return;
	}
	vrammix_cpyex(vram, gamecore.vram[dispwin->vramnum], rect);

	gc = &gamecore.gamecfg;
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				if (gamecore.sys.type & GAME_HAVEALPHA) {
					vram_fillex(vram, rect, gc->winrgb, (BYTE)gc->winalpha);
				}
				else {
					vrammix_cpyex(vram, textwin->frame, rect);
				}
				vrammix_cpyex(vram, textwin->textctrl.vram, rect);
			}
			if (textwin->cmdvram) {
				if (textwin->flag & TEXTWIN_CMDFRAME) {
					vrammix_cpyex(vram, textwin->cmdframe, rect);
				}
				vrammix_cpyex(vram, textwin->cmdvram, rect);
			}
		}
	}
}


void vramdraw_draw(void) {

	UNIRECT		*unirect;
const RECT_T	*rect;

	unirect = &gamecore.vramrect;
	if (unirect->type) {
		rect = unionrect_get(unirect);
		vramdraw_drawvram(mainvram, rect);
		scrnmng_draw(rect);
		unionrect_rst(unirect);
	}
}


VRAMHDL vramdraw_createtmp(const RECT_T *rect) {

	SYS_T	*sys;
	VRAMHDL	ret;

	sys = &gamecore.sys;
#ifndef SIZE_QVGA
	ret = vram_create(sys->width, sys->height, FALSE, DEFAULT_BPP);
#else
	ret = vram_create(320, 240, FALSE, DEFAULT_BPP);
#endif
	vramdraw_drawvram(ret, rect);
	return(ret);
}

