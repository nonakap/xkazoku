#include	"compiler.h"
#include	"fontmng.h"
#include	"inputmng.h"
#include	"taskmng.h"
#include	"gamecore.h"
#include	"arcfile.h"


enum {
	TEXTDISP_CMDREQ		= 0x01,
	TEXTDISP_STRREQ		= 0x02,
	TEXTDISP_PRINT		= 0x04,
	TEXTDISP_MOUSEEV	= 0x08,
	TEXTDISP_NEXTPAGE	= 0x10,
	TEXTDISP_DONE		= 0x20
};

static const BYTE textsinglebyte[] = {
			0x81, 0x40, 0x81, 0x40, 0x81, 0x41, 0x81, 0x42,
			0x81, 0x45, 0x81, 0x48, 0x81, 0x49, 0x81, 0x69,
			0x81, 0x6a, 0x81, 0x75, 0x81, 0x76, 0x82, 0x4f,
			0x82, 0x50, 0x82, 0x51, 0x82, 0x52, 0x82, 0x53,
			0x82, 0x54, 0x82, 0x55, 0x82, 0x56, 0x82, 0x57,
			0x82, 0x58, 0x82, 0xa0, 0x82, 0xa2, 0x82, 0xa4,
			0x82, 0xa6, 0x82, 0xa8, 0x82, 0xa9, 0x82, 0xaa,
			0x82, 0xab, 0x82, 0xac, 0x82, 0xad, 0x82, 0xae,
			0x81, 0x40, 0x82, 0xb0, 0x82, 0xb1, 0x82, 0xb2,
			0x82, 0xb3, 0x82, 0xb4, 0x82, 0xb5, 0x82, 0xb6,
			0x82, 0xb7, 0x82, 0xb8, 0x82, 0xb9, 0x82, 0xba,
			0x82, 0xbb, 0x82, 0xbc, 0x82, 0xbd, 0x82, 0xbe,
			0x82, 0xbf, 0x82, 0xc0, 0x82, 0xc1, 0x82, 0xc2,
			0x82, 0xc3, 0x82, 0xc4, 0x82, 0xc5, 0x82, 0xc6,
			0x82, 0xc7, 0x82, 0xc8, 0x82, 0xc9, 0x82, 0xca,
			0x82, 0xcb, 0x82, 0xcc, 0x82, 0xcd, 0x82, 0xce,
			0x82, 0xd0, 0x82, 0xd1, 0x82, 0xd3, 0x82, 0xd4,
			0x82, 0xd6, 0x82, 0xd7, 0x82, 0xd9, 0x82, 0xda,
			0x82, 0xdc, 0x82, 0xdd, 0x82, 0xde, 0x82, 0xdf,
			0x82, 0xe0, 0x82, 0xe1, 0x82, 0xe2, 0x82, 0xe3,
			0x82, 0xe4, 0x82, 0xe5, 0x82, 0xe6, 0x82, 0xe7,
			0x82, 0xe8, 0x82, 0xe9, 0x82, 0xea, 0x82, 0xeb,
			0x82, 0xed, 0x82, 0xf0, 0x82, 0xf1, 0x83, 0x41,
			0x83, 0x43, 0x83, 0x45, 0x83, 0x47, 0x83, 0x49,
			0x83, 0x4a, 0x83, 0x4c, 0x83, 0x4e, 0x83, 0x50,
			0x83, 0x52, 0x83, 0x54, 0x83, 0x56, 0x83, 0x58,
			0x83, 0x5a, 0x83, 0x5c, 0x83, 0x5e, 0x83, 0x60,
			0x83, 0x62, 0x83, 0x63, 0x83, 0x65, 0x83, 0x67,
			0x83, 0x69, 0x83, 0x6a, 0x82, 0xaf, 0x83, 0x6c,
			0x83, 0x6d, 0x83, 0x6e, 0x83, 0x71, 0x83, 0x74,
			0x83, 0x77, 0x83, 0x7a, 0x83, 0x7d, 0x83, 0x7e,
			0x83, 0x80, 0x83, 0x81, 0x83, 0x82, 0x83, 0x84};


#ifdef TRACE
static void debugtextout(STRING_T *str) {

	char	buf[1024 + 1];
	char	*p = buf;
	int		r = 512;
const BYTE	*ptr;
	int		remain;

	ptr = str->ptr;
	remain = str->remain;

	do {
		BYTE	c;
		if (remain-- <= 0) {
			break;
		}
		c = *ptr++;
		if (!c) {
			break;
		}
		else if (c < 0x80) {
			*p++ = textsinglebyte[c * 2 + 0];
			*p++ = textsinglebyte[c * 2 + 1];
		}
		else {
			*p++ = c;
			if (remain-- <= 0) {
				break;
			}
			*p++ = *ptr++;
		}
	} while(--r);
	*p = '\0';
	TRACEOUT((buf));
}
#endif


// ---- number

static BOOL makenumber(TEXT_T *txt, int num) {

	SINT32		val;
	int			s;
	UINT		value;
	BYTE		*p;

	if (scr_valget(num, &val) != SUCCESS) {
		return(FAILURE);
	}
	p = txt->txtwork + (sizeof(txt->txtwork) & (~1));
	s = (val < 0)?-1:1;
	value = val * s;
	while(txt->txtwork < p) {
		p -= 2;
		num = val % 10;
		p[0] = 0x82;
		p[1] = (BYTE)(0x4f + num);
		val /= 10;
		if (!val) {
			break;
		}
	}
	if ((txt->txtwork < p) && (s < 0)) {
		p -= 2;
		txt->txtwork[0] = 0x81;
		txt->txtwork[1] = 0x7c;
	}
	txt->txt.ptr = p;
	txt->txt.remain = (txt->txtwork + sizeof(txt->txtwork)) - p;
	return(SUCCESS);
}

static BOOL makenumberzen(TEXT_T *txt, int num, int colomn, int zero) {

	SINT32		val;
	int			s;
	UINT		value;
	BYTE		*p;

	colomn = min(colomn, (int)sizeof(txt->txtwork) / 2);
	if (scr_valget(num, &val) != SUCCESS) {
		return(FAILURE);
	}
	txt->txt.ptr = txt->txtwork;
	txt->txt.remain = colomn * 2;
	p = txt->txtwork + (colomn * 2);
	s = (val < 0)?-1:1;
	value = val * s;
	while(1) {
		p -= 2;
		num = val % 10;
		p[0] = 0x82;
		p[1] = (BYTE)(0x4f + num);
		val /= 10;
		colomn--;
		if ((!val) || (!colomn)) {
			break;
		}
	}
	if ((colomn) && (s < 0)) {
		colomn--;
		txt->txtwork[0] = 0x81;
		txt->txtwork[1] = 0x7c;
	}
	if (colomn) {
		colomn--;
		p -= 2;
		p[0] = zero?0x82:0x81;
		p[1] = zero?0x4f:0x40;
		while(colomn--) {
			p -= 2;
			p[0] = p[2];
			p[1] = p[3];
		}
	}
	return(SUCCESS);
}

static BOOL makenumberhan(TEXT_T *txt, int num, int colomn, int zero) {

	SINT32		val;
	int			s;
	UINT		value;
	BYTE		*p;

	colomn = min(colomn, (int)sizeof(txt->txtwork));
	if (scr_valget(num, &val) != SUCCESS) {
		return(FAILURE);
	}
	txt->txt.ptr = txt->txtwork;
	txt->txt.remain = colomn;
	p = txt->txtwork + colomn;
	s = (val < 0)?-1:1;
	value = val * s;
	while(1) {
		p -= 1;
		num = val % 10;
		p[0] = (BYTE)('0' + num);
		val /= 10;
		colomn--;
		if ((!val) || (!colomn)) {
			break;
		}
	}
	if ((colomn) && (s < 0)) {
		colomn--;
		txt->txtwork[0] = '-';
	}
	if (colomn) {
		colomn--;
		p -= 1;
		p[0] = zero?'0':' ';
		while(colomn--) {
			p -= 1;
			p[0] = p[1];
		}
	}
	return(SUCCESS);
}


// ---- text sub

static void textdisp_print(TEXTCTRL textctrl, TEXT_T *txt,
					void (*func)(VRAMHDL dst, void *fhdl, const char *str,
							UINT32 color, POINT_T *pt, const RECT_T *rct)) {

	char		buf[4];
	RECT_U		rect;
	POINT_T		pt;
	int			fontwidth;
	POINT_T		fontsize;
	int			leng;

	if (txt->txt.remain <= 0) {
		goto tdd_done;
	}

	fontwidth = textctrl->fontsize;
	buf[0] = txt->txt.ptr[0];
	if ((((buf[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
		if (txt->txt.remain < 2) {
			goto tdd_done;
		}
		buf[1] = txt->txt.ptr[1];
		if (buf[1] == '\0') {
			goto tdd_done;
		}
		buf[2] = '\0';
		leng = 2;
	}
	else {
		if (buf[0] == '\0') {
			goto tdd_done;
		}
		buf[1] = '\0';
		leng = 1;
		fontwidth /= 2;
	}

	if (textctrl->fonttype & TEXTCTRL_CLIP) {
		if ((txt->x + fontwidth) > textctrl->clip.right) {
			txt->x = textctrl->clip.left;
			txt->y += textctrl->fontsize + 2;
			if ((txt->y + textctrl->fontsize + 2) > textctrl->clip.bottom) {
				txt->flag |= TEXTDISP_NEXTPAGE;
				return;
			}
		}
	}

	txt->txt.ptr += leng;
	txt->txt.remain -= leng;
	textctrl->intext += leng;

	fontmng_getdrawsize(textctrl->font, buf, &fontsize);

	rect.p.x = txt->x;
	rect.p.y = txt->y;
#ifdef SIZE_QVGA
	vramdraw_halfpoint(&rect.p);
#endif
	if (textctrl->fonttype & TEXTCTRL_SHADOW) {
		pt.x = rect.p.x + 1;
		pt.y = rect.p.y + 1;
		func(textctrl->vram, textctrl->font, buf, 
										textctrl->fontcolor[1], &pt, NULL);
		fontsize.x += 1;
		fontsize.y += 1;
	}
	pt.x = rect.p.x;
	pt.y = rect.p.y;
	func(textctrl->vram, textctrl->font, buf,
										textctrl->fontcolor[0], &pt, NULL);
	rect.r.right = rect.r.left + fontsize.x;
	rect.r.bottom = rect.r.top + fontsize.y;
	unionrect_add(&textctrl->drawrect, &rect.r);
	txt->x += fontwidth;
	return;

tdd_done:
	txt->flag &= ~TEXTDISP_PRINT;
}

static void textdisp_strreq(TEXTCTRL textctrl, TEXT_T *txt) {

	BYTE		c;
	BOOL		ascii;

	txt->cmd.remain -= 1;
	if (txt->cmd.remain < 0) {
		goto tdsr_done;
	}
	c = *txt->cmd.ptr++;
	ascii = FALSE;
	if (textctrl->fonttype & TEXTCTRL_ASCII) {
		if (c == 0x5c) {
			txt->cmd.remain -= 1;
			if (txt->cmd.remain < 0) {
				goto tdsr_done;
			}
			c = *txt->cmd.ptr++;
			if (c == '\0') {
				c = 0x5c;
			}
		}
		while(c == 0x7f) {
			txt->cmd.remain--;
			if (txt->cmd.remain < 0) {
				goto tdsr_done;
			}
			c = *txt->cmd.ptr++;
			ascii = TRUE;
		}
	}
	if (c == 0) {
		txt->flag &= ~TEXTDISP_STRREQ;
		return;
	}

	if ((!ascii) && (c < 0x80)) {
		txt->txtwork[0] = textsinglebyte[c * 2 + 0];
		txt->txtwork[1] = textsinglebyte[c * 2 + 1];
		txt->txt.remain = 2;
	}
	else if ((((c ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
		txt->cmd.remain -= 1;
		if (txt->cmd.remain < 0) {
			goto tdsr_done;
		}
		txt->txtwork[0] = c;
		txt->txtwork[1] = *txt->cmd.ptr++;
		if (txt->txtwork[1] == '\0') {
			txt->flag &= ~TEXTDISP_STRREQ;
			return;
		}
		txt->txt.remain = 2;
	}
	else {
		txt->txtwork[0] = c;
		txt->txt.remain = 1;
	}
	txt->txt.ptr = txt->txtwork;
	txt->flag |= TEXTDISP_PRINT;
	return;

tdsr_done:
	txt->flag = TEXTDISP_DONE;
}


// ---- cmd:5b

static void textdisp_cmd5b(TEXTCTRL textctrl, TEXT_T *txt) {

	BYTE		c;
	int			num;
	UINT32		col;
	BOOL		r;

	while(1) {
		txt->cmd.remain--;
		if (txt->cmd.remain < 0) {
			break;
		}
		c = *txt->cmd.ptr++;
		switch(c) {
			case 0x00:	// exit?
				break;

			case 0x01:
				txt->cmd.remain -= 3;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				col = MAKEPALETTE(txt->cmd.ptr[0], txt->cmd.ptr[1],
													txt->cmd.ptr[2]);
				txt->cmd.ptr += 3;
				textctrl->fontcolor[0] = col;
				break;

			case 0x08:	// 既読フラグの設定
				txt->cmd.remain -= 4;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				num = scr_cmdval(LOADINTELDWORD(txt->cmd.ptr));
				txt->cmd.ptr += 4;
				variant_set(gamecore.flags.kid, num, 1);
 				break;

			case 0x09:	// ?
				txt->cmd.remain -= 1;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				txt->cmd.ptr += 1;
				break;

			case 0x0a:	// param;
				txt->cmd.remain -= 2;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				num = LOADINTELWORD(txt->cmd.ptr);
				txt->cmd.ptr += 2;
				if (gamecore.sys.version >= EXE_VER1) {
					txt->cmd.remain -= 3;
					if (txt->cmd.remain < 0) {
						goto tdc_done;
					}
					if (!txt->cmd.ptr[2]) {
						r = makenumberzen(txt, num,
										txt->cmd.ptr[0], txt->cmd.ptr[1]);
					}
					else {
						r = makenumberhan(txt, num,
										txt->cmd.ptr[0], txt->cmd.ptr[1]);
					}
					txt->cmd.ptr += 3;
				}
				else {
					r = makenumber(txt, num);
				}
				if (r == SUCCESS) {
					txt->flag |= TEXTDISP_PRINT;
					return;
				}
				break;

			case 0xff:
#ifdef TRACE
				debugtextout(&txt->cmd);
#endif
				txt->flag |= TEXTDISP_STRREQ;
				return;

			default:
				TRACEOUT(("text err cmd:%x", c));
				goto tdc_done;
		}
	}

tdc_done:
	txt->flag = TEXTDISP_DONE;
}

void textdisp_draw(int num, POINT_T *pt, SCR_OPE *op) {

	TEXT_T		txt;
	TEXTCTRL	textctrl;
	UINT		flag;

	if ((num < 0) && (num >= GAMECORE_MAXVRAM)) {
		goto tddraw_end;
	}
	textctrl = &gamecore.textdraw;
	textctrl->vram = gamecore.vram[num];
	textctrl_renewal(textctrl);
	unionrect_rst(&textctrl->drawrect);

	txt.cmd.ptr = op->ptr;
	txt.cmd.remain = op->remain;
	txt.x = pt->x;
	txt.y = pt->y;
	txt.flag = TEXTDISP_CMDREQ;

	while(1) {
		flag = txt.flag;
		if (flag & (TEXTDISP_DONE | TEXTDISP_NEXTPAGE | TEXTDISP_MOUSEEV)) {
			break;
		}
		else if (flag & TEXTDISP_PRINT) {
			textdisp_print(textctrl, &txt, vrammix_text);
		}
		else if (flag & TEXTDISP_STRREQ) {
			textdisp_strreq(textctrl, &txt);
		}
		else if (flag & TEXTDISP_CMDREQ) {
			textdisp_cmd5b(textctrl, &txt);
		}
		else {
			break;
		}
	}
	if (textctrl->drawrect.type) {
		effect_vramdraw(num, unionrect_get(&textctrl->drawrect));
	}

tddraw_end:
	return;
}


// ----

static void textdisp_setpos(TEXTDISP textdisp, TEXTCTRL textctrl) {

	int			count;
	TEXT_T		*txt;
	int			y;

	count = textdisp->count;
	txt = textdisp->txt;

	y = textctrl->ty;
	while(count--) {
		if (txt->flag) {
			txt->x = textctrl->tx;
			txt->y = y;
			y += textctrl->fontsize;
#ifndef SIZE_QVGA
			y += 2;
#else
			y += 1;
#endif
		}
		txt++;
	}
}


static void textdisp_getpos(TEXTDISP textdisp, TEXTCTRL textctrl) {

	TEXT_T	*txt;

	txt = textdisp->txt + (textdisp->count - 1);
	textctrl->tx = txt->x;
	textctrl->ty = txt->y;
}


static int textdisp_prepare(int num, int count, UINT flag) {

	TEXTDISP	textdisp;
	TEXTWIN		textwin;
	GAMECFG		gc;
	int			skip;

	textdisp = &gamecore.textdisp;
	textwin = textwin_getwin(num);
	if (textwin == NULL) {
		return(GAMEEV_SUCCESS);
	}

	textdisp->txtnum = num;
	textdisp->count = count;
	textdisp->flag = flag;

	textdisp_setpos(textdisp, &textwin->textctrl);

	gc = &gamecore.gamecfg;
	if ((!gc->textwaittick) || (gc->skip) ||
		((gc->lastread) && (gc->readskip))) {
		skip = 1;
	}
	else {
		event_resetmouse(0);
		skip = 0;
	}
	textdisp->skip = skip;

	textctrl_renewal(&textwin->textctrl);
	return(GAMEEV_TEXTOUT);
}


static BOOL textdisp_waitkey(void) {

	GAMECFG	gc;
	UINT	btn;
	UINT	key;

	gc = &gamecore.gamecfg;
	if ((gc->skip) || ((gc->lastread) && (gc->readskip))) {
		return(TRUE);
	}

	btn = event_getmouse(NULL, NULL);
	key = event_getkey();
	if ((btn & (LBUTTON_BIT | RBUTTON_BIT)) ||
		(key & (KEY_ENTER | KEY_SKIP))) {
		event_resetmouse(0);
		event_resetkey(~KEY_ENTER);
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}


// ----

static void textdisp_print2(TEXTDISP textdisp, TEXTWIN textwin, TEXT_T *txt) {

	TEXTCTRL	textctrl;

	textctrl = &textwin->textctrl;
	if (textdisp->flag & TDFLAG_RENUM) {
		textdisp->flag &= ~TDFLAG_RENUM;
		textdisp_setpos(textdisp, textctrl);
	}
	textdisp_print(textctrl, txt, vrammix_textex);
}

static void textdisp_cmdreq(TEXTDISP textdisp, TEXTWIN textwin, TEXT_T *txt) {

	BYTE		c;
	TEXTCTRL	textctrl;
	int			num;
	UINT32		col;
	BOOL		r;
	int			i;
	char		path[ARCFILENAME_LEN+1];

	textctrl = &textwin->textctrl;
	while(1) {
		txt->cmd.remain--;
		if (txt->cmd.remain < 0) {
			break;
		}
		c = *txt->cmd.ptr++;
		switch(c) {
			case 0x00:	// exit?
				break;

			case 0x01:
				num = 0;
				if (gamecore.sys.version >= EXE_VER1) {
					txt->cmd.remain -= 1;
					if (txt->cmd.remain < 0) {
						goto tdc_done;
					}
					num = *txt->cmd.ptr++;
				}
				txt->cmd.remain -= 3;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				col = MAKEPALETTE(txt->cmd.ptr[0], txt->cmd.ptr[1],
													txt->cmd.ptr[2]);
				txt->cmd.ptr += 3;
				if (num < 3) {
					textctrl->fontcolor[num] = col;
				}
				break;

			case 0x02:	// mouseevent
				TRACEOUT(("[MOUSE EVENT WAIT]"));
				txt->flag |= TEXTDISP_MOUSEEV;
				return;

			case 0x03:	// clear
				textwin_clear(textdisp->txtnum);
				textdisp_setpos(textdisp, textctrl);
				break;

			case 0x04:	// 人名表示
				txt->cmd.remain -= 1;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				num = *txt->cmd.ptr++;
				if (num < GAMECORE_MAXNAME) {
					txt->txt.ptr = (BYTE *)(textwin->chrname[num]);
					txt->txt.remain = GAMECORE_NAMELEN;
					TRACEOUT(("NAME = %s", txt->txt.ptr));
					txt->flag |= TEXTDISP_PRINT;
					return;
				}
				break;

			case 0x06:	// cr
				txt->x = textctrl->clip.left;
				txt->y += textctrl->fontsize;
#ifndef SIZE_QVGA
				txt->y += 2;
#else
				txt->y += 1;
#endif
				break;

			case 0x08:	// 既読フラグの設定
				txt->cmd.remain -= 4;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				num = scr_cmdval(LOADINTELDWORD(txt->cmd.ptr));
				txt->cmd.ptr += 4;
				variant_set(gamecore.flags.kid, num, 1);
 				break;

			case 0x09:	// ?
				txt->cmd.remain -= 1;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				txt->cmd.ptr += 1;
				break;

			case 0x0a:	// param;
				txt->cmd.remain -= 2;
				if (txt->cmd.remain < 0) {
					goto tdc_done;
				}
				num = LOADINTELWORD(txt->cmd.ptr);
				txt->cmd.ptr += 2;
				if (gamecore.sys.version >= EXE_VER1) {
					txt->cmd.remain -= 2;
					if (txt->cmd.remain < 0) {
						goto tdc_done;
					}
					r = makenumberzen(txt, num,
										txt->cmd.ptr[0], txt->cmd.ptr[1]);
					txt->cmd.ptr += 2;
				}
				else {
					r = makenumber(txt, num);
				}
				if (r == SUCCESS) {
					txt->flag |= TEXTDISP_PRINT;
					return;
				}
				break;

			case 0x11:
				if (gamecore.sys.version >= EXE_VER2) {
					txt->cmd.remain -= 4;
					if (txt->cmd.remain < 0) {
						goto tdc_done;
					}
					num = scr_cmdval(LOADINTELDWORD(txt->cmd.ptr));
					txt->cmd.ptr += 4;
					textwin_setname(textwin, num);
				}
				else if (gamecore.sys.version >= EXE_VER1) {
					textdisp->flag |= TDFLAG_RENUM;
				}
				else {
					TRACEOUT(("[MOUSE EVENT WAIT]"));
					txt->flag |= TEXTDISP_MOUSEEV;
					return;
				}
				break;

			case 0x12:
				if (gamecore.sys.version >= EXE_VER2) {
					textdisp->flag |= TDFLAG_RENUM;
				}
				else {
					goto tdc_done;
				}
				break;

			case 0x13:
				path[ARCFILENAME_LEN] = '\0';
				for (i=0; i<ARCFILENAME_LEN; i++) {
					txt->cmd.remain -= 1;
					if (txt->cmd.remain < 0) {
						goto tdc_done;
					}
					path[i] = *txt->cmd.ptr++;
					if (path[i] == '\0') {
						break;
					}
				}
				if (!sndplay_voicecondition(path)) {
					sndplay_voiceplay();
				}
				sndplay_voicereset();
				break;

			case 0xff:
#ifdef TRACE
				debugtextout(&txt->cmd);
#endif
				txt->flag |= TEXTDISP_STRREQ;
				return;

			default:
				TRACEOUT(("text err cmd:%x", c));
				goto tdc_done;
		}
	}

tdc_done:
	txt->flag = TEXTDISP_DONE;
}

static UINT textdisp_step(TEXTDISP textdisp, TEXTWIN textwin, TEXT_T *txt) {

	UINT	flag;

	while(1) {
		flag = txt->flag;
		if (flag & (TEXTDISP_DONE | TEXTDISP_NEXTPAGE | TEXTDISP_MOUSEEV)) {
			return(flag & (TEXTDISP_DONE | TEXTDISP_NEXTPAGE |
														TEXTDISP_MOUSEEV));
		}
		else if (flag & TEXTDISP_PRINT) {
			textdisp_print2(textdisp, textwin, txt);
			if ((txt->flag & TEXTDISP_PRINT) && (!textdisp->skip)) {
				return(TEXTDISP_PRINT);
			}
		}
		else if (flag & TEXTDISP_STRREQ) {
			textdisp_strreq(&textwin->textctrl, txt);
		}
		else if (flag & TEXTDISP_CMDREQ) {
			textdisp_cmdreq(textdisp, textwin, txt);
		}
		else {
			return(TEXTDISP_DONE);
		}
	}
}


// ----

int textdisp_set(int num, SCR_OPE *op) {

	TEXTDISP	textdisp;

	textdisp = &gamecore.textdisp;
	textdisp->txt[0].cmd.ptr = op->ptr;
	textdisp->txt[0].cmd.remain = op->remain;
	textdisp->txt[0].flag = TEXTDISP_CMDREQ;
	return(textdisp_prepare(num, 1, 0));
}

int textdisp_multiset(int num, int txtnum, SCR_OPE *op) {

	TEXTDISP	textdisp;
	TEXT_T		*txt;
	STRING_T	*ext;
	int			i;
	int			cnt;

	textdisp = &gamecore.textdisp;
	txt = textdisp->txt;
	if (txtnum == 0) {
		txt->cmd.ptr = op->ptr;
		txt->cmd.remain = op->remain;
		txt->flag = TEXTDISP_CMDREQ;
		txt++;
		cnt = 1;
		for (i=0; i<GAMECORE_MAXTEXT; i++) {
			ext = textdisp->ext[i];
			if ((ext) && (ext->remain)) {
				txt->cmd.ptr = ext->ptr;
				txt->cmd.remain = ext->remain;
				txt->flag = TEXTDISP_CMDREQ;
				txt++;
				cnt++;
				ext->remain = 0;
			}
		}
		return(textdisp_prepare(num, cnt, TDFLAG_MULTI));
	}
	else if ((txtnum > 0) && (txtnum < GAMECORE_MAXTEXT)) {
		ext = textdisp->ext[txtnum];
		if (ext) {
			_MFREE(ext);
			ext = NULL;
		}
		if (op->remain > 0) {
			ext = (STRING_T *)_MALLOC(sizeof(STRING_T) + op->remain,
															"multi text");
		}
		textdisp->ext[txtnum] = ext;
		if (ext) {
			ext->ptr = (BYTE *)(ext + 1);
			ext->remain = op->remain;
			CopyMemory((ext + 1), op->ptr, op->remain);
		}
	}
	return(GAMEEV_SUCCESS);
}

void textdisp_multireset(void) {

	int			i;
	STRING_T	**ext;

	ext = gamecore.textdisp.ext;
	for (i=0; i<GAMECORE_MAXTEXT; i++) {
		if (ext[i]) {
			_MFREE(ext[i]);
			ext[i] = NULL;
		}
	}
}

int textdisp_exec(void) {

	TEXTDISP	textdisp;
	TEXTWIN		textwin;
	TEXT_T		*txt;
	UINT		flag;
	int			cnt;

	textdisp = &gamecore.textdisp;
	textwin = textwin_getwin(textdisp->txtnum);
	if (textwin == NULL) {
		return(GAMEEV_SUCCESS);
	}
	unionrect_rst(&textwin->textctrl.drawrect);

	txt = textdisp->txt;
	cnt = textdisp->count;
	flag = 0;
	while(cnt--) {
		flag |= textdisp_step(textdisp, textwin, txt);
		txt++;
	}

	if (textwin->textctrl.drawrect.type) {
		vramdraw_setrect(textwin->textctrl.vram,
								unionrect_get(&textwin->textctrl.drawrect));
		vramdraw_draw();
	}

	if (flag & TEXTDISP_PRINT) {
		if (textdisp_waitkey()) {
			textdisp->skip |= 2;
		}
		else {
			taskmng_sleep(gamecore.gamecfg.textwaittick);
		}
	}
	else if (flag & (TEXTDISP_NEXTPAGE | TEXTDISP_MOUSEEV)) {
		if (textdisp_waitkey()) {
			textdisp->skip &= ~2;
			if (flag & TEXTDISP_NEXTPAGE) {
				textwin_clear(textdisp->txtnum);
				textdisp_setpos(textdisp, &textwin->textctrl);
			}
			txt = textdisp->txt;
			cnt = textdisp->count;
			while(cnt--) {
				txt->flag &= ~(TEXTDISP_NEXTPAGE | TEXTDISP_MOUSEEV);
				txt++;
			}
		}
	}
	else {
		textdisp_getpos(textdisp, &textwin->textctrl);
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_TEXTOUT);
}

