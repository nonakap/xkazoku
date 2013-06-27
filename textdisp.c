#include	"compiler.h"
#include	"gamecore.h"
#include	"inputmng.h"
#include	"fontmng.h"
#include	"taskmng.h"


enum {
	TEXTDISP_DONE		= 0x00,
	TEXTDISP_CMDREQ		= 0x01,
	TEXTDISP_PRINT		= 0x02,
	TEXTDISP_MOUSEEV	= 0x04,
	TEXTDISP_NEXTPAGE	= 0x08
};


#ifdef TRACE
static void debugtextout(STRING_T *str) {

	char	buf[1024 + 1];
	char	*p = buf;
	int		r = 512;
	BYTE	*ptr;
	BYTE	remain;

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


// ---- text

typedef struct {
	BYTE	zero[2];
	BYTE	minus[2];
	BYTE	space[2];
} NUMSTR;

static const NUMSTR numstr[4] = {
			{{0x82, 0x4f}, {0x81, 0x7c}, {0x81, 0x40}},
			{{0x82, 0x4f}, {0x81, 0x7c}, {0x82, 0x4f}},
			{{0x7f, 0x30}, {0x7f, 0x2d}, {0x7f, 0x20}},
			{{0x7f, 0x30}, {0x7f, 0x2d}, {0x7f, 0x30}}};

static BOOL makenumber(STRING_T *dst, STRING_T *src) {

	int			size;
	int			num;
	SINT32		val;
	int			s;
	UINT		value;
	int			flag;
	BYTE		*p;
const NUMSTR	*ns;

	if (src->remain < 5) {
		return(FAILURE);
	}
	size = dst->remain;
	size--;
	size /= 2;
	if (size <= 0) {
		size = 0;
	}
	num = LOADINTELWORD(src->ptr);
	size = min(size, (int)src->ptr[2]);
	flag = (src->ptr[3]?1:0);				// 逆かも
	flag += (src->ptr[4]?2:0);				// 逆かも
	src->ptr += 5;
	src->remain -= 5;
	ns = numstr + flag;
	scr_valget(num, &val);
	dst->remain = size * 2 + 1;
	p = dst->ptr + (size * 2);
	p[0] = 0;
	s = (val < 0)?-1:1;
	value = val * s;
	while(1) {
		p -= 2;
		num = val % 10;
		p[0] = ns->zero[0];
		p[1] = (BYTE)(ns->zero[1] + num);
		val /= 10;
		size--;
		if ((!val) || (!size)) {
			break;
		}
	}
	if ((size) && (s < 0)) {
		size--;
		dst->ptr[0] = ns->minus[0];
		dst->ptr[1] = ns->minus[1];
	}
	while(size--) {
		p -= 2;
		p[0] = ns->space[0];
		p[1] = ns->space[1];
	}
	return(SUCCESS);
}


static UINT textdisp_put(TEXTCTRL textctrl, TEXT_T *txt, STRING_T *str) {

	int			bak;
	BYTE		c;
	char		buf[4];
	RECT_U		rect;
	POINT_T		pt;
	BOOL		ascii;
	int			fontwidth;
	POINT_T		fontsize;
	UINT32		col;

	bak = str->remain;
	if (bak <= 0) {
		return(TEXTDISP_DONE);
	}
	c = *str->ptr;
	ascii = FALSE;
	if (textctrl->fonttype & TEXTCTRL_ASCII) {
		if (c == 0x5c) {
			str->ptr++;
			str->remain--;
			if (str->remain <= 0) {
				return(TEXTDISP_DONE);
			}
			if (str->ptr[0] == 0) {
				goto zen_break;
			}
			c = str->ptr[0];
		}
		while(c == 0x7f) {
			str->ptr++;
			str->remain--;
			if (str->remain <= 0) {
				return(TEXTDISP_DONE);
			}
			c = *str->ptr;
			ascii = TRUE;
		}
	}

zen_break:
	str->ptr++;
	str->remain--;
	if (c == 0) {
		return(TEXTDISP_CMDREQ);
	}

	fontwidth = textctrl->fontsize;
	if (!ascii) {
		if (c < 0x80) {
			buf[0] = textsinglebyte[c * 2 + 0];
			buf[1] = textsinglebyte[c * 2 + 1];
		}
		else {
			if (str->remain <= 0) {
				return(TEXTDISP_DONE);
			}
			str->remain--;
			buf[0] = c;
			buf[1] = *str->ptr++;
			if (!buf[1]) {
				return(TEXTDISP_CMDREQ);
			}
		}
		buf[2] = '\0';
		textctrl->intext += 2;
	}
	else {
		buf[0] = c;
		buf[1] = '\0';
		fontwidth >>= 1;
		textctrl->intext++;
	}

	if (textctrl->fonttype & TEXTCTRL_CLIP) {
		if ((txt->x + fontwidth) > textctrl->clip.right) {
			txt->x = textctrl->clip.left;
			txt->y += textctrl->fontsize + 2;
			if ((txt->y + textctrl->fontsize + 2) > textctrl->clip.bottom) {
				str->ptr -= (bak - str->remain);
				str->remain = bak;
				return(TEXTDISP_NEXTPAGE);
			}
		}
	}

	if (c == 0x20) {
		txt->x += fontwidth;
		return(TEXTDISP_PRINT);
	}

	fontmng_getdrawsize(textctrl->font, buf, &fontsize);

	rect.p.x = txt->x;
	rect.p.y = txt->y;
#ifdef SIZE_QVGA
	vramdraw_halfpoint(&rect.p);
#endif
	col = textctrl->fontcolor[1];
	if (col) {
		pt.x = rect.p.x + 1;
		pt.y = rect.p.y + 1;
		vrammix_text(textctrl->vram, textctrl->font, buf, col, &pt, NULL);
		fontsize.x += 1;
		fontsize.y += 1;
	}
	pt.x = rect.p.x;
	pt.y = rect.p.y;
	vrammix_text(textctrl->vram, textctrl->font, buf,
										textctrl->fontcolor[0], &pt, NULL);
	rect.r.right = rect.r.left + fontsize.x;
	rect.r.bottom = rect.r.top + fontsize.y;
	unionrect_add(&textctrl->drawrect, &rect.r);
	txt->x += fontwidth;
	return(TEXTDISP_PRINT);
}


// ----

void textdisp_draw(int num, POINT_T *pt, SCR_OPE *op) {

	TEXT_T		txt;
	BYTE		c;
	int			kid;
	TEXTCTRL	textctrl;
	int			flag;
	STRING_T	str2;
	BYTE		work[32];

	textctrl = &gamecore.textdraw;
	textctrl_renewal(textctrl);
	unionrect_rst(&textctrl->drawrect);

	if ((num < 0) && (num >= GAMECORE_MAXVRAM)) {
		goto tddraw_end;
	}
	textctrl->vram = gamecore.vram[num];

	txt.str.ptr = op->ptr;
	txt.str.remain = op->remain;
	txt.x = pt->x;
	txt.y = pt->y;

	while(1) {
		if (txt.str.remain <= 0) {
			break;
		}
		txt.str.remain--;
		c = *txt.str.ptr++;
		switch(c) {
			case 0x00:	// ?
			case 0x02:	// mouseevent
			case 0x03:	// clear
			case 0x06:	// cr
			case 0x11:
				break;

			case 0x01:
				if (txt.str.remain < 3) {
					goto tddraw_end;
				}
				textctrl->fontcolor[0] = MAKEPALETTE(txt.str.ptr[0],
													txt.str.ptr[1],
													txt.str.ptr[2]);
				txt.str.remain -= 3;
				txt.str.ptr += 3;
				break;

			case 0x08:	// 既読フラグの設定
				if (txt.str.remain >= 4) {
					txt.str.remain -= 4;
					kid = scr_cmdval(LOADINTELDWORD(txt.str.ptr));
					txt.str.ptr += 4;
					variant_set(gamecore.flags.kid, kid, 1);
				}
				else {
					goto tddraw_end;
 				}
 				break;

			case 0x0a:	// param;			これ いつから変更になった？
				str2.ptr = work;
				str2.remain = sizeof(work);
				if (makenumber(&str2, &txt.str)) {
					goto tddraw_end;
				}
				do {
					flag = textdisp_put(textctrl, &txt, &str2);
				} while(flag & TEXTDISP_PRINT);
				break;

			case 0xff:
				do {
					flag = textdisp_put(textctrl, &txt, &txt.str);
				} while(flag & TEXTDISP_PRINT);
				break;

			default:
				TRACEOUT(("text err cmd:%x", c));
				goto tddraw_end;
		}
	}

tddraw_end:
	if (textctrl->drawrect.type) {
		vramdraw_setrect(textctrl->vram, unionrect_get(&textctrl->drawrect));
		vramdraw_draw();
	}
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
			y += textctrl->fontsize + 2;
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


static int textdisp_prepare(int num, int count) {

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

	textdisp_setpos(textdisp, &textwin->textctrl);

	gc = &gamecore.gamecfg;
	if ((!gc->textwaittick) || (gc->skip) || (gc->readskip)) {
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

	UINT	btn;
	UINT	key;

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

static UINT textdisp_out(TEXTDISP textdisp, TEXTWIN textwin,
												TEXT_T *txt, STRING_T *str) {

	TEXTCTRL	textctrl;

	textctrl = &textwin->textctrl;
	if (textdisp->renum) {
		textdisp->renum = FALSE;
		textdisp_setpos(textdisp, textctrl);
	}
	return(textdisp_put(textctrl, txt, str));
}


static UINT textdisp_ctrl(TEXTDISP textdisp, TEXTWIN textwin, TEXT_T *txt) {

	BYTE		c;
	UINT		ret;
	int			num;
	STRING_T	name;
	TEXTCTRL	textctrl;
	UINT32		col;

	textctrl = &textwin->textctrl;
	ret = 0;
	while(!ret) {
		if (txt->str.remain <= 0) {
			ret = TEXTDISP_DONE;
			break;
		}
		txt->str.remain--;
		c = *txt->str.ptr++;
		switch(c) {
			case 0x00:		// exit?
				break;

			case 0x01:
				num = 0;
				if (gamecore.sys.version >= EXE_VER1) {
					if (txt->str.remain <= 0) {
						ret = TEXTDISP_DONE;
						break;
					}
					num = txt->str.ptr[0];
					txt->str.remain -= 1;
					txt->str.ptr += 1;
				}
				if (txt->str.remain < 3) {
					ret = TEXTDISP_DONE;
					break;
				}
				col = MAKEPALETTE(txt->str.ptr[0], txt->str.ptr[1],
													txt->str.ptr[2]);
				txt->str.remain -= 3;
				txt->str.ptr += 3;
				if (num < 3) {
					textctrl->fontcolor[num] = col;
				}
				break;

			case 0x02:	// mouseevent
				TRACEOUT(("[MOUSE EVENT WAIT]"));
				ret = TEXTDISP_MOUSEEV;
				break;

			case 0x03:	// clear
				textwin_clear(textdisp->txtnum);
				textdisp_setpos(textdisp, textctrl);
				break;

			case 0x04:	// 人名表示
				if (txt->str.remain > 0) {
					txt->str.remain--;
					num = *txt->str.ptr++;
					if (num >= GAMECORE_MAXNAME) {
						break;
					}
					name.ptr = (BYTE *)(textwin->chrname[num]);
					name.remain = GAMECORE_NAMELEN;
					TRACEOUT(("NAME = %s", name.ptr));
					while(textdisp_out(textdisp, textwin, txt, &name)
													& TEXTDISP_PRINT) { }
				}
				else {
					ret = TEXTDISP_DONE;
				}
				break;

			case 0x06:	// cr
				txt->x = textctrl->clip.left;
				txt->y += textctrl->fontsize + 2;
				break;

			case 0x08:	// 既読フラグの設定
				if (txt->str.remain >= 4) {
					txt->str.remain -= 4;
					num = scr_cmdval(LOADINTELDWORD(txt->str.ptr));
					txt->str.ptr += 4;
					variant_set(gamecore.flags.kid, num, 1);
				}
				else {
					ret = TEXTDISP_DONE;
 				}
 				break;

			case 0x0a:	// param;
				if (txt->str.remain >= 2) {
					txt->str.remain -= 2;
					txt->str.ptr += 2;
				}
				else {
					ret = TEXTDISP_DONE;
				}
				break;

			case 0x11:
				textdisp->renum = TRUE;
				break;

			case 0xff:
#ifdef TRACE
				debugtextout(&txt->str);
#endif
				ret = TEXTDISP_PRINT;
				break;

			default:
				TRACEOUT(("text err cmd:%x", c));
				ret = TEXTDISP_DONE;
				break;
		}
	}
	return(ret);
}


// ----

int textdisp_set(int num, SCR_OPE *op) {

	TEXTDISP	textdisp;

	textdisp = &gamecore.textdisp;
	textdisp->txt[0].str.ptr = op->ptr;
	textdisp->txt[0].str.remain = op->remain;
	textdisp->txt[0].flag = TEXTDISP_CMDREQ;
	return(textdisp_prepare(num, 1));
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
		txt->str.ptr = op->ptr;
		txt->str.remain = op->remain;
		txt->flag = TEXTDISP_CMDREQ;
		txt++;
		cnt = 1;
		for (i=0; i<GAMECORE_MAXTEXT; i++) {
			ext = textdisp->ext[i];
			if ((ext) && (ext->remain)) {
				txt->str.ptr = ext->ptr;
				txt->str.remain = ext->remain;
				txt->flag = TEXTDISP_CMDREQ;
				txt++;
				cnt++;
				ext->remain = 0;
			}
		}
		return(textdisp_prepare(num, cnt));
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
	while(1) {
		if (txt->flag & TEXTDISP_CMDREQ) {
			txt->flag = textdisp_ctrl(textdisp, textwin, txt);
			continue;
		}
		if (txt->flag & TEXTDISP_PRINT) {
			txt->flag = textdisp_out(textdisp, textwin, txt, &txt->str);
			if ((!(txt->flag & TEXTDISP_PRINT)) || (textdisp->skip)) {
				continue;
			}
		}
		flag |= txt->flag;
		txt++;
		cnt--;
		if (!cnt) {
			break;
		}
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
	else if (flag & (TEXTDISP_MOUSEEV | TEXTDISP_NEXTPAGE)) {
		if (textdisp_waitkey()) {
			textdisp->skip &= ~2;
			if (flag & TEXTDISP_NEXTPAGE) {
				textwin_clear(textdisp->txtnum);
				textdisp_setpos(textdisp, &textwin->textctrl);
			}
			txt = textdisp->txt;
			cnt = textdisp->count;
			while(cnt--) {
				if (txt->flag & TEXTDISP_MOUSEEV) {
					txt->flag = TEXTDISP_CMDREQ;
				}
				else if (txt->flag & TEXTDISP_NEXTPAGE) {
					txt->flag = TEXTDISP_PRINT;
				}
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

