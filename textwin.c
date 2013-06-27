#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cgload.h"
#include	"fontmng.h"


#define	TEXTPADDING			16


// ---- text control

void textctrl_init(TEXTCTRL textctrl, BOOL ascii) {

	UINT	fonttype;

	fonttype = TEXTCTRL_BOLD;
	if (ascii) {
		fonttype |= TEXTCTRL_ASCII;
	}
	textctrl->fonttype = fonttype;
	textctrl->fontsize = 16;
	textctrl->fontcolor[0] = MAKEPALETTE(0xff, 0xff, 0xff);
	textctrl->fontcolor[1] = 0;
	textctrl->fontcolor[2] = 0;
}

void textctrl_setsize(TEXTCTRL textctrl, int size) {

	if (textctrl->fontsize != size) {
		textctrl->fonttype &= ~TEXTCTRL_READY;
		textctrl->fontsize = size;
	}
}

void textctrl_settype(TEXTCTRL textctrl, BYTE type) {

	if ((textctrl->fonttype ^ type) & TEXTCTRL_BOLD) {
		textctrl->fonttype &= ~(TEXTCTRL_READY | TEXTCTRL_FONTMASK);
		textctrl->fonttype |= (type & TEXTCTRL_FONTMASK);
	}
}

void textctrl_trash(TEXTCTRL textctrl) {

	void	*font;

	font = textctrl->font;
	if (font != NULL) {
		textctrl->font = NULL;
		fontmng_destroy(font);
		textctrl->fonttype &= ~TEXTCTRL_READY;
	}
}

void textctrl_renewal(TEXTCTRL textctrl) {

	UINT	type;

	if (!(textctrl->fonttype & TEXTCTRL_READY)) {
		textctrl->fonttype |= TEXTCTRL_READY;
		fontmng_destroy(textctrl->font);
		type = 0;
		if (textctrl->fonttype & TEXTCTRL_BOLD) {
			type |= FDAT_BOLD;
		}
#ifndef SIZE_QVGA
		textctrl->font = fontmng_create(textctrl->fontsize, type, NULL);
#else
		textctrl->font = fontmng_create(textctrl->fontsize / 2, type, NULL);
#endif
	}
}

static void textctrl_txtclr(TEXTCTRL textctrl) {

	textctrl->intext = 0;
	vram_zerofill(textctrl->vram, NULL);
	vramdraw_setrect(textctrl->vram, NULL);
	textctrl->tx = textctrl->clip.left;
	textctrl->ty = textctrl->clip.top;
}


// ----

static void scrn2rect(TEXTWIN textwin, const SCRN_T *scrn, RECT_T *rect) {

	int		x;
	int		y;
	VRAMHDL	frame;
	SYS_T	*sys;

	sys = &gamecore.sys;
	frame = textwin->frame;

	x = scrn->left;
	if (x < 0) {
		x = 0;
	}
	else if (x > sys->width) {
		x = sys->width;
	}
	rect->left = x;
	if (scrn->width) {
		x += scrn->width;
		if (x > sys->width) {
			x = sys->width;
		}
	}
	else if (frame) {
#ifndef SIZE_QVGA
		x += frame->width;
#else
		x += frame->width * 2;
#endif
	}
	else {
		x = sys->width;
	}
	rect->right = x;

	y = scrn->top;
	if (y < 0) {
		y = 0;
	}
	else if (y > sys->height) {
		y = sys->height;
	}
	rect->top = y;
	if (scrn->height) {
		y += scrn->height;
		if (y > sys->height) {
			y = sys->height;
		}
	}
	else if (frame) {
#ifndef SIZE_QVGA
		y += frame->height;
#else
		y += frame->height * 2;
#endif
	}
	else {
		y = sys->height;
	}
	rect->bottom = y;
}

static void textwin_winopen(TEXTWIN textwin, UINT flag) {

	DISPWIN		dispwin;
	RECT_T		rct;
	TEXTCTRL	textctrl;

	flag &= TEXTWIN_WINBYALL;
	if (flag == 0) {
		goto twwo_exit;
	}

	textctrl = &textwin->textctrl;
	if (textwin->flag & TEXTWIN_WINBYALL) {
		vram_zerofill(textctrl->vram, NULL);
		vramdraw_setrect(textctrl->vram, NULL);
	}
	else {
		scrn2rect(textwin, &textwin->scrn, &rct);
		dispwin = &gamecore.dispwin;
		if (dispwin->flag & DISPWIN_CLIPTEXT) {
			scrn2rect(textwin, &dispwin->txtclip, &textctrl->clip);
		}
		else {
			scrn2rect(textwin, &textwin->txtscrn, &textctrl->clip);
			textctrl->clip.left += TEXTPADDING;
			textctrl->clip.top += TEXTPADDING;
			textctrl->clip.right -= TEXTPADDING;
			textctrl->clip.bottom -= TEXTPADDING;
		}
		textctrl->clip.left -= rct.left;
		textctrl->clip.top -= rct.top;
		textctrl->clip.right -= rct.left;
		textctrl->clip.bottom -= rct.top;
		textctrl->tx = textctrl->clip.left;
		textctrl->ty = textctrl->clip.top;

#ifdef SIZE_QVGA
		vramdraw_halfrect(&rct);
#endif
		textctrl->vram = vram_create(rct.right - rct.left,
									rct.bottom - rct.top, TRUE, DEFAULT_BPP);
		if (textctrl->vram == NULL) {
			goto twwo_exit;
		}
		textctrl->vram->posx = rct.left;
		textctrl->vram->posy = rct.top;
		vramdraw_setrect(textctrl->vram, NULL);

		if (textwin->frame) {			// フレームがあった？
			textwin->frame->posx = rct.left;
			textwin->frame->posy = rct.top;
			vramdraw_setrect(textwin->frame, NULL);
		}
		textctrl_txtclr(&textwin->textctrl);
	}
	textwin->flag |= flag;

twwo_exit:
	return;
}

static void textwin_winclose(TEXTWIN textwin, UINT flag) {

	TEXTCTRL	textctrl;

	flag &= TEXTWIN_WINBYALL;
	if (!(textwin->flag & flag)) {
		goto twwc_exit;
	}

	textctrl = &textwin->textctrl;
	vramdraw_setrect(textctrl->vram, NULL);
	textwin->flag &= ~flag;
	if (textwin->flag & TEXTWIN_WINBYALL) {
		vram_zerofill(textctrl->vram, NULL);
	}
	else {
		vram_destroy(textctrl->vram);
		textctrl->vram = NULL;
	}

twwc_exit:
	return;
}


// ----

TEXTWIN textwin_getwin(int num) {

	TEXTWIN	ret;

	if ((num < 0) || (num >= GAMECORE_MAXTXTWIN)) {
		goto twgw_err;
	}
	ret = gamecore.textwin[num];
	if (ret == NULL) {
		ret = (TEXTWIN)_MALLOC(sizeof(TEXTWIN_T), "TEXTWIN");
		if (ret == NULL) {
			goto twgw_err;
		}
		ZeroMemory(ret, sizeof(TEXTWIN_T));
		gamecore.textwin[num] = ret;
		textctrl_init(&ret->textctrl, gamecore.sys.type & GAME_TEXTASCII);
		ret->textctrl.fonttype |= TEXTCTRL_CLIP;
	}
	return(ret);

twgw_err:
	return(NULL);
}

void textwin_create(void) {
}

void textwin_destroy(void) {

	int		i;
	TEXTWIN	textwin;

	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			gamecore.textwin[i] = NULL;
			vram_destroy(textwin->cmdvram);
			vram_destroy(textwin->cmdframe);
			vram_destroy(textwin->cmdicon);
			vram_destroy(textwin->frame);
			vram_destroy(textwin->textctrl.vram);
			textctrl_trash(&textwin->textctrl);
			_MFREE(textwin);
		}
	}
}

void textwin_setpos(int num, const SCRN_T *scrn, const SCRN_T *clip) {

	TEXTWIN	tw;

	tw = textwin_getwin(num);
	if (tw) {
		tw->scrn = *scrn;
		if (clip == NULL) {
			tw->txtscrn = *scrn;
		}
		else {
			tw->txtscrn = *clip;
		}
		tw->hisscrn = tw->txtscrn;
	}
}

void textwin_open(int num) {

	TEXTWIN		textwin;

	textwin = textwin_getwin(num);
	if ((textwin) && (!(textwin->flag & TEXTWIN_TEXT))) {
		textwin->flag |= TEXTWIN_TEXT;
		textwin_winopen(textwin, TEXTWIN_WINBYTEXT);
		vramdraw_draw();
	}
}

void textwin_close(int num) {

	TEXTWIN		textwin;

	textwin = textwin_getwin(num);
	if ((textwin) && (textwin->flag & TEXTWIN_TEXT)) {
		textwin->flag &= ~TEXTWIN_TEXT;
		textwin_winclose(textwin, TEXTWIN_WINBYTEXT);
		vramdraw_draw();
	}
}

void textwin_clear(int num) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_txtclr(&textwin->textctrl);
		vramdraw_draw();
	}
}

void textwin_setframe(int num, const char *label) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if (textwin) {
		vram_destroy(textwin->frame);
		textwin->frame = NULL;
		cgload_data(&textwin->frame, ARCTYPE_GRAPHICS, label);
	}
}

// ----

void textwin_cmdopen(int num, int cmds, int type) {

	TEXTWIN		textwin;
	VRAMHDL		hdl;
	int			i;

	textwin = textwin_getwin(num);
	if (textwin == NULL) {
		goto twcop_exit;
	}
	if (textwin->flag & TEXTWIN_CMD) {
		goto twcop_exit;
	}
	textwin->cmdtype = type;
	textwin->cmdmax = cmds;
	textwin->cmdfocus = -1;

	if (type == 1) {							// icon
		if (!(textwin->flag & TEXTWIN_CMDRECT)) {
			goto twcop_exit;
		}
		hdl = vram_create(textwin->cmdscrn.width, textwin->cmdscrn.height,
														FALSE, DEFAULT_BPP);
		textwin->cmdvram = hdl;
		if (hdl == NULL) {
			goto twcop_exit;
		}
		hdl->posx = textwin->cmdscrn.left;
		hdl->posy = textwin->cmdscrn.top;
		vramcpy_cpy(hdl, textwin->cmdframe, NULL, NULL);
		for (i=0; i<cmds; i++) {
			event_cmdwindraw(textwin, i, 2);
		}
	}
	else {
		hdl = NULL;
		if (textwin->flag & TEXTWIN_CMDRECT) {
			hdl = vram_create(textwin->cmdscrn.width, textwin->cmdscrn.height,
														TRUE, DEFAULT_BPP);
		}
		textwin->cmdvram = hdl;
		if (hdl) {
			hdl->posx = textwin->cmdscrn.left;
			hdl->posy = textwin->cmdscrn.top;
			if (textwin->cmdframe) {
				textwin->flag |= TEXTWIN_CMDFRAME;
				textwin->cmdframe->posx = textwin->cmdscrn.left;
				textwin->cmdframe->posy = textwin->cmdscrn.top;
				vramdraw_setrect(textwin->cmdframe, NULL);
			}
		}
		else {
			textwin_winopen(textwin, TEXTWIN_WINBYCMD);
		}
		for (i=0; i<cmds; i++) {
			event_choicewindraw(textwin, i, 0);
		}
	}
	textwin->flag |= TEXTWIN_CMD;
	textwin->cmdret = -1;
	vramdraw_setrect(hdl, NULL);
	vramdraw_draw();

twcop_exit:
	return;
}

void textwin_cmdclose(int num) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if ((textwin) && (textwin->flag & TEXTWIN_CMD)) {
		if (textwin->cmdvram) {
			vramdraw_setrect(textwin->cmdvram, NULL);
			vram_destroy(textwin->cmdvram);
			textwin->cmdvram = NULL;
		}
		else {
			textwin_winclose(textwin, TEXTWIN_WINBYCMD);
		}
		if (textwin->flag & TEXTWIN_CMDFRAME) {
			vramdraw_setrect(textwin->cmdframe, NULL);
		}
		textwin->flag &= ~(TEXTWIN_CMD | TEXTWIN_CMDFRAME |
										TEXTWIN_CMDCAP | TEXTWIN_CMDCAPEX);
		vramdraw_draw();
	}
}

void textwin_setcmdframe(int num, const char *label) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if (textwin) {
		vram_destroy(textwin->cmdframe);
		textwin->cmdframe = NULL;
		cgload_data(&textwin->cmdframe, ARCTYPE_GRAPHICS, label);
	}
}

void textwin_setcmdicon(int num, int width, const char *label) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if (textwin) {
//		textwin->iconwidth = width;
		vram_destroy(textwin->cmdicon);
		textwin->cmdicon = NULL;
		cgload_data(&textwin->cmdicon, ARCTYPE_GRAPHICS, label);
	}
	(void)width;
}

