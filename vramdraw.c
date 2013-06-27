#include	"compiler.h"
#include	"dosio.h"
#include	"scrnmng.h"
#include	"taskmng.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cgload.h"
#include	"bmpdata.h"


// gamecore.cpp ‚©‚çØ‚èo‚µ

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

	// vram 0‚Æ1‚ÍÅ‰‚©‚çŠJ‚¢‚Ä‚Ü‚·c
#ifndef SIZE_QVGA
	gamecore.vram[0] = vram_create(sys->width, sys->height,
														TRUE, DEFAULT_BPP);
	gamecore.vram[1] = vram_create(sys->width, sys->height,
														TRUE, DEFAULT_BPP);
#else
	gamecore.vram[0] = vram_create(320, 240, TRUE, DEFAULT_BPP);
	gamecore.vram[1] = vram_create(320, 240, TRUE, DEFAULT_BPP);
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

int vramdraw_orgsize(int size) {

#ifndef SIZE_QVGA
	return(size);
#else
	return(size * 2);
#endif
}

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

static void clrcb(void *arg, const RECT_T *rect) {

	vram_zerofill((VRAMHDL)arg, rect);
}

static void vramdraw_drawvram(VRAMHDL vram, const RECT_T *rect) {

	DISPWIN		dispwin;
	VRAMHDL		src;
	RECT_T		srcrect;
	RECT_T		vramrect;

	dispwin = &gamecore.dispwin;
	if (dispwin->flag & DISPWIN_VRAM) {
		src = gamecore.anime.ganvram;
		if (src == NULL) {
			src = gamecore.vram[dispwin->vramnum];
		}
		if (src != NULL) {
			vram_getrect(src, &srcrect);
			if (rect == NULL) {
				vram_getrect(vram, &vramrect);
				rect_enumout(&srcrect, &vramrect, vram, clrcb);
			}
			else {
				rect_enumout(&srcrect, rect, vram, clrcb);
			}
			vrammix_cpy(vram, src, rect);
		}
		else {
			vram_zerofill(vram, rect);
		}
	}
	else {
		vram_zerofill(vram, rect);
		if (gamecore.sys.version >= EXE_VER1) {
			return;
		}
	}
	vramdraw_drawupper(vram, rect);
}

void vramdraw_drawupper(VRAMHDL vram, const RECT_T *rect) {

	TEXTWIN		textwin;
	GAMECFG		gc;
	int			i;

	gc = &gamecore.gamecfg;
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				if (gamecore.sys.type & GAME_HAVEALPHA) {
#if 1
					RECT_T r;
					if (vram_cliprectex(&r, textwin->textctrl.vram, rect)
																== SUCCESS) {
						vram_fillex(vram, &r, gc->winrgb,
												(BYTE)(64 - gc->winalpha));
					}
#else
					vram_fillex(vram, rect, gc->winrgb,
												(BYTE)(64 - gc->winalpha));
#endif
				}
				else {
					vrammix_cpyex(vram, textwin->frame, rect);
				}
				vrammix_cpyex(vram, textwin->namevram, rect);
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


// ----

static void creditmix(VRAMHDL vram, int pos, const char *label, UINT32 col) {

	VRAMHDL	tmp;

	if ((vram == NULL) || (label == NULL)) {
		goto cmix_exit;
	}
	tmp = NULL;
	cgload_data(&tmp, ARCTYPE_GRAPHICS, label);
	if (tmp == NULL) {
		goto cmix_exit;
	}
	tmp->posy = vram->height - tmp->height;
	if (pos) {
		tmp->posx = vram->width - tmp->width;
	}
	vrammix_mixalpha(vram, tmp, col, NULL);
	vram_destroy(tmp);

cmix_exit:
	return;
}

void vramdraw_savebmp(int num, const char *dir, const char *name,
						int pos, const char *credit0, const char *credit1) {

	int		i;
	char	path[MAX_PATH];
	char	work[32];
	BMPDATA	inf;
	VRAMHDL	vram;
	UINT	tmp;
	UINT	bmpsize;
	BMPFILE	bf;
	BMPINFO	bi;
	BYTE	*dat;
	FILEH	fh;

	if ((num < 0) || (num >= GAMECORE_MAXVRAM) || (name == NULL)) {
		goto vdsb_err1;
	}
	if (dir == NULL) {
		dir = "";
	}
	for (i=0; i<100; i++) {
		milstr_ncpy(path, dir, sizeof(path));
		milstr_ncat(path, name, sizeof(path));
		sprintf(work, "%02u.bmp", i);
		milstr_ncat(path, work, sizeof(path));
		if (file_attr(path) == -1) {
			break;
		}
		taskmng_rol();
	}
	if (i >= 100) {
		goto vdsb_err1;
	}

	vram = vram_copy(gamecore.vram[num]);
	if (vram == NULL) {
		goto vdsb_err1;
	}
	creditmix(vram, pos, credit1, 0x000000);
	creditmix(vram, pos, credit0, 0xffffff);

	inf.width = vram->width;
	inf.height = vram->height;
	inf.bpp = 24;

	bmpdata_setinfo(&bi, &inf, TRUE);
	bmpsize = bmpdata_getdatasize(&bi);

	ZeroMemory(&bf, sizeof(bf));
	bf.bfType[0] = 'B';
	bf.bfType[1] = 'M';
	tmp = sizeof(BMPFILE) + sizeof(BMPINFO);
	STOREINTELDWORD(bf.bfOffBits, tmp);
	tmp += bmpsize;
	STOREINTELDWORD(bf.bfSize, tmp);

	dat = bmpdata_bmp24cnv(&bi, vram);
	if (dat == NULL) {
		goto vdsb_err2;
	}
	fh = file_create(path);
	if (fh == FILEH_INVALID) {
		goto vdsb_err3;
	}
	file_write(fh, &bf, sizeof(bf));
	file_write(fh, &bi, sizeof(bi));
	file_write(fh, dat, bmpsize);
	file_close(fh);

vdsb_err3:
	_MFREE(dat);

vdsb_err2:
	vram_destroy(vram);

vdsb_err1:
	return;
}

