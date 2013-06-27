#include	"compiler.h"
#include	"fontmng.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cgload.h"


#define	TEXTPADDING			16


// ---- text control

void textctrl_init(TEXTCTRL textctrl) {

	DISPWIN	dispwin;

	dispwin = &gamecore.dispwin;
	textctrl->fonttype = dispwin->fonttype;
	textctrl->fontsize = dispwin->fontsize;
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
	textctrl->fonttype &= ~TEXTCTRL_SHADOW;
	textctrl->fonttype |= (type & TEXTCTRL_SHADOW);
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
		textctrl->font = fontmng_create(vramdraw_half(textctrl->fontsize),
															type, NULL);
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
		x += vramdraw_orgsize(frame->width);
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
		y += vramdraw_orgsize(frame->height);
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
	SCRN_T		clip;
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
			clip = dispwin->txtclip;
			clip.left += rct.left;
			clip.top += rct.top;
			scrn2rect(textwin, &clip, &textctrl->clip);
		}
		else {
			scrn2rect(textwin, &textwin->txtscrn, &textctrl->clip);
			if (!(textwin->flag & TEXTWIN_TEXTCLIP)) {
				textctrl->clip.left += TEXTPADDING;
				textctrl->clip.top += TEXTPADDING;
				textctrl->clip.right -= TEXTPADDING;
				textctrl->clip.bottom -= TEXTPADDING;
			}
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

		if (textwin->frame) {			// ƒtƒŒ[ƒ€‚ª‚ ‚Á‚½H
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
	vramdraw_setrect(textwin->frame, NULL);
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
		textctrl_init(&ret->textctrl);
		ret->textctrl.fonttype |= TEXTCTRL_CLIP;
		ret->namenum = -1;
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
			vram_destroy(textwin->namevram);
			vram_destroy(textwin->cmdvram);
			vram_destroy(textwin->cmdframe);
			vram_destroy(textwin->cmdicon);
			vram_destroy(textwin->frame);
			vram_destroy(textwin->textctrl.vram);
			listarray_destroy(textwin->cmdtext);
			textctrl_trash(&textwin->textctrl);
			_MFREE(textwin);
		}
	}
}

void textwin_allclose(void) {

	int		i;
	TEXTWIN	textwin;

	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			textwin_close(i);
			textwin_cmdclose(i);
		}
	}
}

void textwin_setpos(int num, const SCRN_T *scrn, const SCRN_T *clip) {

	TEXTWIN	textwin;

	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->scrn = *scrn;
		if (clip == NULL) {
			textwin->txtscrn = *scrn;
			textwin->flag &= ~TEXTWIN_TEXTCLIP;
		}
		else {
			textwin->txtscrn = *clip;
			textwin->flag |= TEXTWIN_TEXTCLIP;
		}
		textwin->hisscrn = textwin->txtscrn;
	}
}

void textwin_open(int num) {

	TEXTWIN		textwin;

	textwin = textwin_getwin(num);
	if ((textwin) && (!(textwin->flag & TEXTWIN_TEXT))) {
		textwin->flag |= TEXTWIN_TEXT;
		textwin->flag &= ~TEXTWIN_TEXTHIDE;
		textwin_winopen(textwin, TEXTWIN_WINBYTEXT);
		textwin_setname(textwin, 0);
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
	VRAMHDL	vram;
	int		x;
	int		y;

	textwin = textwin_getwin(num);
	if (textwin) {
		vram = textwin->frame;
		if (vram) {
			textwin->frame = NULL;
			x = vram->posx;
			y = vram->posy;
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				vramdraw_setrect(vram, NULL);
			}
			vram_destroy(vram);
		}
		else {
			x = 0;
			y = 0;
		}
		cgload_data(&textwin->frame, ARCTYPE_GRAPHICS, label);
		vram = textwin->frame;
		if (vram) {
			vram->posx = x;
			vram->posy = y;
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				vramdraw_setrect(vram, NULL);
			}
		}
	}
}

void textwin_setname(TEXTWIN textwin, int num) {

	char	label[16];
	VRAMHDL	frame;
	VRAMHDL	name;

	if (textwin->namenum != num) {
		textwin->namenum = num;
		vramdraw_setrect(textwin->namevram, NULL);
		vram_destroy(textwin->namevram);
		textwin->namevram = NULL;

		if ((num >= 0) && (num < 1000)) {
			sprintf(label, "FW%03u", num);
			cgload_data(&textwin->namevram, ARCTYPE_GRAPHICS, label);
			name = textwin->namevram;
			frame = textwin->frame;
			if ((name) && (frame)) {
				name->posx = frame->posx;
				name->posy = frame->posy;
			}
			vramdraw_setrect(name, NULL);
		}
	}
}


// ----

static BOOL cmdtextdraw(void *vpItem, void *vpArg) {

	TEXTWIN		textwin;
	TEXTCTRL	textctrl;
	CMDTEXT		cmdtext;
	VRAMHDL		dst;
	POINT_T		pt;

	textwin = (TEXTWIN)vpArg;
	textctrl = &textwin->textctrl;
	textctrl_renewal(textctrl);
	cmdtext = (CMDTEXT)vpItem;
	dst = textwin->cmdvram;
	if (dst == NULL) {
		dst = textwin->textctrl.vram;
	}
	if (textctrl->fonttype & TEXTCTRL_SHADOW) {
		pt.x = cmdtext->rect.left + 1;
		pt.y = cmdtext->rect.top + 1;
		vrammix_text(dst, textctrl->font, cmdtext->str,
										cmdtext->col[1], &pt, &cmdtext->rect);
	}
	pt.x = cmdtext->rect.left;
	pt.y = cmdtext->rect.top;
	vrammix_text(dst, textctrl->font, cmdtext->str,
										cmdtext->col[0], &pt, &cmdtext->rect);
	vramdraw_setrect(dst, &cmdtext->rect);
	return(FALSE);
}

void textwin_cmdopen(int num, int cmds, int type) {

	TEXTWIN		textwin;
	VRAMHDL		hdl;
	int			i;

	textwin = textwin_getwin(num);
	if (textwin == NULL) {
		goto twcop_exit;
	}

	event_resetall();

	if (!(textwin->flag & TEXTWIN_CMD)) {
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
		}
		textwin->flag |= TEXTWIN_CMD;
		vramdraw_setrect(hdl, NULL);
	}

	if (textwin->cmdtype == 1) {
		for (i=0; i<textwin->cmdmax; i++) {
			event_cmdwindraw(textwin, i, 2);
		}
	}
	else if (textwin->cmdtype == 2) {
		listarray_enum(textwin->cmdtext, cmdtextdraw, textwin);
		for (i=0; i<textwin->cmdmax; i++) {
			event_choicewindraw(textwin, i, 0);
		}
	}
	textwin->cmdret = -1;
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


// ----

BOOL textwin_isopen(void) {

	int			i;
	TEXTWIN		textwin;

	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				return(TRUE);
			}
			if (textwin->cmdvram) {
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

void textwin_setrect(void) {

	int			i;
	TEXTWIN		textwin;

	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			if ((textwin->textctrl.vram) &&
				(!(textwin->flag & TEXTWIN_TEXTHIDE))) {
				if (gamecore.sys.type & GAME_HAVEALPHA) {
					vramdraw_setrect(mainvram, NULL);
					break;
				}
				else {
					vramdraw_setrect(textwin->frame, NULL);
				}
				vramdraw_setrect(textwin->namevram, NULL);
				vramdraw_setrect(textwin->textctrl.vram, NULL);
			}
			if (textwin->cmdvram) {
				if (textwin->flag & TEXTWIN_CMDFRAME) {
					vramdraw_setrect(textwin->cmdframe, NULL);
				}
				vramdraw_setrect(textwin->cmdvram, NULL);
			}
		}
	}
}

