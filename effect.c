#include	"compiler.h"
#include	"scrnmng.h"
#include	"gamemsg.h"
#include	"gamecore.h"


void effect_trush(void) {

	EFFECT		ef;
	VRAMHDL		v;

	ef = &gamecore.ef;
	v = ef->bmp;
	if (v) {
		ef->bmp = NULL;
		vram_destroy(v);
	}
}


void effect_vramdraw(int num, const RECT_T *rect) {

	DISPWIN	dispwin;

	dispwin = &gamecore.dispwin;
	if ((dispwin->flag & DISPWIN_VRAM) && (dispwin->vramnum == num)) {
		vramdraw_setrect(gamecore.vram[num], rect);
		vramdraw_draw();
	}
}


int effect_set(SCR_OPE *op) {

	VRAMHDL		dstvram;
	VRAMHDL		hdl;
	EFFECT		ef;
	SINT32		param2;
	SINT32		param3;
	SINT32		param4;
	SINT32		param5;
	UINT		version;

	dstvram = gamecore.vram[gamecore.dispwin.vramnum];
	ef = &gamecore.ef;
	switch(ef->cmd) {
		case 0x00: // type0			// 指定プレーンへコピー (透過なし)
		default:   // type6
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpyall(hdl, ef->src, &ef->pt, &ef->r);
				effect_vramdraw(ef->param, &ef->r2);
			}
			return(GAMEEV_SUCCESS);

		case 0x01: // type0			// 指定プレーンへコピー (透過あり)
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpyexa(hdl, ef->src, &ef->pt, &ef->r);
				effect_vramdraw(ef->param, &ef->r2);
			}
			return(GAMEEV_SUCCESS);

		case 0x02: // type1			// random pattern
		case 0x03: // type1			// wipe right
		case 0x04: // type1			// wipe left
		case 0x05: // type1			// wipe down
		case 0x06: // type1			// wipe up
		case 0x07: // type1			// wipe with pattern right
		case 0x08: // type1			// wipe with pattern left
		case 0x09: // type1			// wipe with pattern down
		case 0x0a: // type1			// wipe with pattern up
		case 0x0b: // type1			// zoom in
		case 0x0c: // type1			// zoom out
		case 0x0d: // type1			// mosaic in
		case 0x0e: // type1			// mosaic out
		case 0x13: // type1			// FadeIn
		case 0x18: // type1			// width out
		case 0x19: // type1			// width in
		case 0x1a: // type1			// height out
		case 0x1b: // type1			// height in
			break;

		case 0x0f: // type2			// draw-hline down
		case 0x10: // type2			// draw-vline right
		case 0x11: // type2			// draw-hline up
		case 0x12: // type2			// draw-vline left
			if ((scr_getval(op, &param2) != SUCCESS) ||
				(scr_getval(op, &param3) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			if ((param2 <= 0) || (param3 <= 0)) {
				return(GAMEEV_SUCCESS);
			}
			ef->ex.el.leng = param2;
			ef->ex.el.step = param3;
#ifdef SIZE_QVGA
			ef->ex.el.leng = vramdraw_half(ef->ex.el.leng);
			ef->ex.el.step = vramdraw_half(ef->ex.el.step);
#endif
			version = gamecore.sys.version;
			if ((ef->cmd == 0x11) &&
				((version == EXEVER_CRES) || (version == EXEVER_CRESD))) {
				ef->cmd = 0x1e;
				ef->ex.el.cnt = ef->r.bottom - ef->r.top;
				ef->ex.el.cnt += ef->ex.el.leng - 1;
				ef->ex.el.cnt /= ef->ex.el.leng;
				ef->ex.el.total = (ef->ex.el.leng + 1) / 2;
				ef->ex.el.total += ef->ex.el.step - 1;
				ef->ex.el.total /= ef->ex.el.step;
				ef->ex.el.total *= ef->ex.el.cnt;
			}
			break;

		case 0x14: // type3			// copy with alpha
		case 0x15:
			if (scr_getval(op, &param2) != SUCCESS) {
				return(GAMEEV_WRONGLENG);
			}
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpyalpha(hdl, ef->src, 255 - param2, &ef->pt, &ef->r);
				vram_fillalpha(hdl, &ef->r2, 0xff);
				effect_vramdraw(ef->param, &ef->r2);
			}
			return(GAMEEV_SUCCESS);

		case 0x1c:	// type5
		case 0x1d:	// type5
			if ((scr_getval(op, &param2) != SUCCESS) ||
				(scr_getval(op, &param3) != SUCCESS) ||
				(scr_getval(op, &param4) != SUCCESS) ||
				(scr_getval(op, &param5) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			if ((param2 <= 0) || (param5 <= 0)) {
				vramcpy_cpy(dstvram, ef->src, &ef->pt, &ef->r);
				vramdraw_setrect(dstvram, &ef->r2);
				vramdraw_draw();
				return(GAMEEV_SUCCESS);
			}
			ef->ex.eb.leng = param2;
			ef->ex.eb.dir = param4 & 3;
			ef->ex.eb.step = param5;
			if (!(ef->cmd & 1)) {
				ef->ex.eb.cnt = ef->r2.bottom - ef->r2.top;
			}
			else {
				ef->ex.eb.cnt = ef->r2.right - ef->r2.left;
			}
#ifdef SIZE_QVGA
			ef->ex.eb.leng = vramdraw_half(ef->ex.eb.leng);
#endif
			ef->ex.eb.cnt += (ef->ex.eb.leng - 1);
			ef->ex.eb.cnt /= ef->ex.eb.leng;
			break;
	}

	ef->upper = (gamecore.sys.type & GAME_TEXTLAYER)?TRUE:FALSE;
	ef->tick = GETTICK();
	return(GAMEEV_EFFECT);
}


// ----

static void effect_draw(EFFECT ef, VRAMHDL vram, RECT_T *rect) {

	if (vram == mainvram) {
		if (ef->upper) {
			vramdraw_drawupper(vram, rect);
		}
		scrnmng_draw(&ef->r2);
	}
	else {
		vramdraw_setrect(vram, rect);
		vramdraw_draw();
	}
}

static int effect02(EFFECT ef, int tick) {

	int		next;
	BYTE	pat[8];
	UINT	pos;
	int		i;
	BYTE	bit;

	next = (ef->param * ef->progress) / 64;
	if (tick >= next) {
		ZeroMemory(pat, sizeof(pat));
		pos = GETRAND();
		pos %= (64 - ef->progress);
		i = 0;
		bit = 0x80;
		do {
			if (!(ef->ex.ep.pat8[i] & bit)) {
				if (!pos) {
					ef->ex.ep.pat8[i] |= bit;
					pat[i] |= bit;
					break;
				}
				pos--;
			}
			bit >>= 1;
			if (!bit) {
				i++;
				bit = 0x80;
			}
		} while(i < 8);
		vramcpy_cpypat(mainvram, ef->src, pat, &ef->pt, &ef->r);
		scrnmng_draw(&ef->r2);
		ef->progress++;
		if (ef->progress >= 64) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect03(EFFECT ef, int tick) {

	int		pos;
	int		width;
	int		w;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	width = r2.right - r2.left;
	pos = width;
	if (tick < ef->param) {
		pos *= tick;
		pos /= ef->param;
	}
	w = pos - ef->progress;
	if (w > 0) {
		int	x;
		pt = ef->pt;
		r = ef->r;
		x = ef->progress;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&ef->r2);
		ef->progress = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect04(EFFECT ef, int tick) {

	int		pos;
	int		width;
	int		w;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	width = r2.right - r2.left;
	pos = width;
	if (tick < ef->param) {
		pos *= tick;
		pos /= ef->param;
	}
	w = pos - ef->progress;
	if (w > 0) {
		int x;
		pt = ef->pt;
		r = ef->r;
		x = width - pos;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&ef->r2);
		ef->progress = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect05(EFFECT ef, int tick) {

	int		pos;
	int		height;
	int		h;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	height = r2.bottom - r2.top;
	pos = height;
	if (tick < ef->param) {
		pos *= tick;
		pos /= ef->param;
	}
	h = pos - ef->progress;
	if (h > 0) {
		int	y;
		pt = ef->pt;
		r = ef->r;
		y = ef->progress;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&ef->r2);
		ef->progress = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect06(EFFECT ef, int tick) {

	int		pos;
	int		height;
	int		h;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	height = r2.bottom - r2.top;
	pos = height;
	if (tick < ef->param) {
		pos *= tick;
		pos /= ef->param;
	}
	h = pos - ef->progress;
	if (h > 0) {
		int	y;
		pt = ef->pt;
		r = ef->r;
		y = height - pos;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&ef->r2);
		ef->progress = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect0b(EFFECT ef, int tick) {

	int		next;

	if (tick >= ef->param) {
		vramcpy_cpy(mainvram, ef->src, &ef->pt, &ef->r);
		scrnmng_draw(&ef->r2);
		return(GAMEEV_SUCCESS);
	}
	next = ((tick * 8) / ef->param) + 1;
	if (ef->progress != next) {
		ef->progress = next;
		vramcpy_zoom(mainvram, ef->src, 512 >> next, &ef->pt, &ef->r);
		scrnmng_draw(&ef->r2);
	}
	return(GAMEEV_EFFECT);
}

static int effect0c(EFFECT ef, int tick) {

	int		next;
	int		r;
	VRAMHDL	dst;

	if (tick >= ef->param) {
		next = 8;
		r = GAMEEV_SUCCESS;
	}
	else {
		next = (tick * 8) / ef->param;
		r = GAMEEV_EFFECT;
	}
	if (ef->progress != next) {
		ef->progress = next;
		dst = mainvram;
		vramcpy_zoom(dst, ef->src, 1 << next, &ef->pt, &ef->r);
		effect_draw(ef, dst, &ef->r2);
	}
	return(r);
}

static int effect0d(EFFECT ef, int tick) {

	int		next;
	VRAMHDL	dst;

	if (tick >= ef->param) {
		vramcpy_cpy(mainvram, ef->src, &ef->pt, &ef->r);
		scrnmng_draw(&ef->r2);
		return(GAMEEV_SUCCESS);
	}
	next = ((tick * 8) / ef->param) + 1;
	if (ef->progress != next) {
		ef->progress = next;
		dst = mainvram;
		vramcpy_mosaic(dst, ef->src, 512 >> next, &ef->pt, &ef->r);
		effect_draw(ef, dst, &ef->r2);
	}
	return(GAMEEV_EFFECT);
}

static int effect0e(EFFECT ef, int tick) {

	int		next;
	int		r;
	VRAMHDL	dst;

	if (tick >= ef->param) {
		next = 8;
		r = GAMEEV_SUCCESS;
	}
	else {
		next = (tick * 8) / ef->param;
		r = GAMEEV_EFFECT;
	}
	if (ef->progress != next) {
		ef->progress = next;
		dst = mainvram;
		vramcpy_mosaic(dst, ef->src, 1 << next, &ef->pt, &ef->r);
		effect_draw(ef, dst, &ef->r2);
	}
	return(r);
}

static int effect0f(EFFECT ef, int tick) {

	int		next;

	next = ef->ex.el.leng;
	if (tick < ef->param) {
		next *= tick;
		next /= ef->param;
	}
	if (ef->progress < next) {
		POINT_T	pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		pt.y += ef->progress;
		rct.top += ef->progress;
		while(rct.top < ef->r.bottom) {
			rct.bottom = min(ef->r.bottom, rct.top + ef->ex.el.step);
			vramcpy_cpy(mainvram, ef->src, &pt, &rct);
			pt.y += ef->ex.el.leng;
			rct.top += ef->ex.el.leng;
		}
		scrnmng_draw(&ef->r2);
		ef->progress += ef->ex.el.step;
		if (ef->progress >= ef->ex.el.leng) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect10(EFFECT ef, int tick) {

	int		next;

	next = ef->ex.el.leng;
	if (tick < ef->param) {
		next *= tick;
		next /= ef->param;
	}
	if (ef->progress < next) {
		POINT_T	pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		pt.x += ef->progress;
		rct.left += ef->progress;
		while(rct.left < ef->r.right) {
			rct.right = min(ef->r.right, rct.left + ef->ex.el.step);
			vramcpy_cpy(mainvram, ef->src, &pt, &rct);
			pt.x += ef->ex.el.leng;
			rct.left += ef->ex.el.leng;
		}
		scrnmng_draw(&ef->r2);
		ef->progress += ef->ex.el.step;
		if (ef->progress >= ef->ex.el.leng) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect11(EFFECT ef, int tick) {

	int		next;

	next = ef->ex.el.leng;
	if (tick < ef->param) {
		next *= tick;
		next /= ef->param;
	}
	if (ef->progress < next) {
		POINT_T pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		rct.bottom -= ef->progress;
		while(rct.bottom > ef->r.top) {
			int y;
			rct.top = rct.bottom - ef->ex.el.step;
			y = rct.top - ef->r.top;
			if (y < 0) {
				rct.top -= y;
				y = 0;
			}
			pt.y = ef->pt.y + y;
			vramcpy_cpy(mainvram, ef->src, &pt, &rct);
			rct.bottom -= ef->ex.el.leng;
		}
		scrnmng_draw(&ef->r2);
		ef->progress += ef->ex.el.step;
		if (ef->progress >= ef->ex.el.leng) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect12(EFFECT ef, int tick) {

	int		next;

	next = ef->ex.el.leng;
	if (tick < ef->param) {
		next *= tick;
		next /= ef->param;
	}
	if (ef->progress < next) {
		POINT_T pt;
		RECT_T rct;
		pt = ef->pt;
		rct = ef->r;
		rct.right -= ef->progress;
		while(rct.right > ef->r.left) {
			int x;
			rct.left = rct.right - ef->ex.el.step;
			x = rct.left - ef->r.left;
			if (x < 0) {
				rct.left -= x;
				x = 0;
			}
			pt.x = ef->pt.x + x;
			vramcpy_cpy(mainvram, ef->src, &pt, &rct);
			rct.right -= ef->ex.el.leng;
		}
		scrnmng_draw(&ef->r2);
		ef->progress += ef->ex.el.step;
		if (ef->progress >= ef->ex.el.leng) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect13(EFFECT ef, int tick) {

	int		alpha;
	VRAMHDL	dst;

	dst = mainvram;
	if (tick >= ef->param) {
		vramcpy_cpy(dst, ef->src, &ef->pt, &ef->r);
		effect_draw(ef, dst, &ef->r2);
		return(GAMEEV_SUCCESS);
	}
	alpha = (tick << 6) / ef->param;
	if (ef->progress != alpha) {
		ef->progress = alpha;
		vramcpy_mix(dst, gamecore.vram[gamecore.dispwin.vramnum],
											ef->src, alpha, &ef->pt, &ef->r);
		effect_draw(ef, dst, &ef->r2);
	}
	return(GAMEEV_EFFECT);
}

static int effect18(EFFECT ef, int tick) {

	int		pos;
	int		width;
	int		w;
	int		pad;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	width = r2.right - r2.left;
	pad = width & 1;
	width /= 2;
	pos = width;
	if (tick < ef->param) {
		pos *= tick;
		pos /= ef->param;
	}
	w = pos - ef->progress;
	if (w > 0) {
		int x;
		pt = ef->pt;
		r = ef->r;
		x = width - pos;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		w += pad;
		x = width + ef->progress;
		pt.x = ef->pt.x + x;
		r.left = ef->r.left + x;
		r.right = r.left + w;
		r2.left = ef->r2.left + x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect19(EFFECT ef, int tick) {

	int		pos;
	int		width;
	int		w;
	int		pad;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	width = r2.right - r2.left;
	pad = width & 1;
	width /= 2;
	pos = width;
	if (tick < ef->param) {
		pos = (pos * tick) / ef->param;
	}
	w = pos - ef->progress;
	if (w > 0) {
		int x;
		pt = ef->pt;
		r = ef->r;
		x = ef->progress;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		w += pad;
		x = (width * 2) - pos;
		pt.x = ef->pt.x + x;
		r.left = ef->r.left + x;
		r.right = r.left + w;
		r2.left = ef->r2.left + x;
		r2.right = r2.left + w;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1a(EFFECT ef, int tick) {

	int		pos;
	int		height;
	int		h;
	int		pad;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	height = r2.bottom - r2.top;
	pad = height & 1;
	height /= 2;
	pos = height;
	if (tick < ef->param) {
		pos = (pos * tick) / ef->param;
	}
	h = pos - ef->progress;
	if (h > 0) {
		int y;
		pt = ef->pt;
		r = ef->r;
		y = height - pos;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		h += pad;
		y = height + ef->progress;
		pt.y = ef->pt.y + y;
		r.top = ef->r.top + y;
		r.bottom = r.top + h;
		r2.top = ef->r2.top + y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1b(EFFECT ef, int tick) {

	int		pos;
	int		height;
	int		h;
	int		pad;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	r2 = ef->r2;
	height = r2.bottom - r2.top;
	pad = height & 1;
	height /= 2;
	pos = height;
	if (tick < ef->param) {
		pos = (pos * tick) / ef->param;
	}
	h = pos - ef->progress;
	if (h > 0) {
		int y;
		pt = ef->pt;
		r = ef->r;
		y = ef->progress;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		h += pad;
		y = (height * 2) - pos;
		pt.y = ef->pt.y + y;
		r.top = ef->r.top + y;
		r.bottom = r.top + h;
		r2.top = ef->r2.top + y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1c(EFFECT ef, int tick) {

	int		cur;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;
	int		y;

	pt = ef->pt;
	r = ef->r;
	r2 = ef->r2;
	y = ef->progress * ef->ex.eb.leng;
	pt.y += y;
	r.top += y;
	r2.top += y;

	cur = tick - ef->ex.eb.base;
	while(cur >= ef->param) {
		r.bottom = r.top + ef->ex.eb.leng;
		r2.bottom = r2.top + ef->ex.eb.leng;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress++;
		if (ef->progress >= ef->ex.eb.cnt) {
			return(GAMEEV_SUCCESS);
		}
		pt.y += ef->ex.eb.leng;
		r.top += ef->ex.eb.leng;
		r2.top += ef->ex.eb.leng;
		cur -= ef->ex.eb.step;
		ef->ex.eb.base += ef->ex.eb.step;
	}
	while((cur > 0) && (r.top < ef->r.bottom)) {
		y = (ef->ex.eb.leng * cur) / ef->param;
		r.bottom = r.top + y;
		r2.bottom = r2.top + y;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		cur -= ef->ex.eb.step;
		pt.y += ef->ex.eb.leng;
		r.top += ef->ex.eb.leng;
		r2.top += ef->ex.eb.leng;
	}
	return(GAMEEV_EFFECT);
}

static int effect1d(EFFECT ef, int tick) {

	int		cur;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;
	int		x;

	pt = ef->pt;
	r = ef->r;
	r2 = ef->r2;
	x = ef->progress * ef->ex.eb.leng;
	pt.x += x;
	r.left += x;
	r2.left += x;

	cur = tick - ef->ex.eb.base;
	while(cur >= ef->param) {
		r.right = r.left + ef->ex.eb.leng;
		r2.right = r2.left + ef->ex.eb.leng;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		ef->progress++;
		if (ef->progress >= ef->ex.eb.cnt) {
			return(GAMEEV_SUCCESS);
		}
		pt.x += ef->ex.eb.leng;
		r.left += ef->ex.eb.leng;
		r2.left += ef->ex.eb.leng;
		cur -= ef->ex.eb.step;
		ef->ex.eb.base += ef->ex.eb.step;
	}
	while((cur > 0) && (r.left < ef->r.right)) {
		x = (ef->ex.eb.leng * cur) / ef->param;
		r.right = r.left + x;
		r2.right = r2.left + x;
		vramcpy_cpy(mainvram, ef->src, &pt, &r);
		scrnmng_draw(&r2);
		cur -= ef->ex.eb.step;
		pt.x += ef->ex.eb.leng;
		r.left += ef->ex.eb.leng;
		r2.left += ef->ex.eb.leng;
	}
	return(GAMEEV_EFFECT);
}

static int effect11x(EFFECT ef, int tick) {

	int		cur;
	VRAMHDL	dst;
	POINT_T	p;
	RECT_T	r;
	int		leng;
	int		pos;
	int		y;
	int		step;

	cur = tick - ef->ex.eb.base;
	if (cur >= ef->param) {
		vramcpy_cpy(mainvram, ef->src, &ef->pt, &ef->r);
		scrnmng_draw(&ef->r2);
		return(GAMEEV_SUCCESS);
	}
	cur *= ef->ex.el.total;
	cur /= ef->param;
	if (ef->progress != cur) {
		ef->progress = cur;
		dst = mainvram;
		p = ef->pt;
		r = ef->r;
		leng = cur / ef->ex.el.cnt;
		leng *= ef->ex.el.step;
		pos = cur % ef->ex.el.cnt;
		y = ef->ex.el.cnt;
		step = 0;
		while(y >= pos) {
			y--;
			step += ef->ex.el.leng;
#if 1
			r.top = ef->r.bottom - step;
			r.bottom = r.top + leng;
			r.top = max(r.top, ef->r.top);
			r.bottom = min(r.bottom, ef->r.bottom);
			p.y = ef->pt.y + (r.top - ef->r.top);
			vramcpy_cpy(dst, ef->src, &p, &r);
#endif
#if 1
			r.bottom = ef->r.top + step;
			r.top = r.bottom - leng;
			r.top = max(r.top, ef->r.top);
			r.bottom = min(r.bottom, ef->r.bottom);
			p.y = ef->pt.y + (r.top - ef->r.top);
			vramcpy_cpy(dst, ef->src, &p, &r);
#endif
		}
		leng += ef->ex.el.step;
		while(y > 0) {
			y--;
			step += ef->ex.el.leng;
#if 1
			r.top = ef->r.bottom - step;
			r.bottom = r.top + leng;
			r.top = max(r.top, ef->r.top);
			r.bottom = min(r.bottom, ef->r.bottom);
			p.y = ef->pt.y + (r.top - ef->r.top);
			vramcpy_cpy(dst, ef->src, &p, &r);
#endif
#if 1
			r.bottom = ef->r.top + step;
			r.top = r.bottom - leng;
			r.top = max(r.top, ef->r.top);
			r.bottom = min(r.bottom, ef->r.bottom);
			p.y = ef->pt.y + (r.top - ef->r.top);
			vramcpy_cpy(dst, ef->src, &p, &r);
#endif
		}
		scrnmng_draw(&ef->r2);
	}
	return(GAMEEV_EFFECT);
}


// ----

typedef int (*EFFUNC)(EFFECT ef, int tick);

static int effect00(EFFECT ef, int tick) {

	(void)ef;
	(void)tick;
	return(GAMEEV_SUCCESS);
}

static const EFFUNC effunc[0x20] = {
			effect00,	effect00,	effect02,	effect03,
			effect04,	effect05,	effect06,	effect03,
			effect04,	effect05,	effect06,	effect0b,
			effect0c,	effect0d,	effect0e,	effect0f,
			effect10,	effect11,	effect12,	effect13,
			effect00,	effect00,	effect00,	effect00,
			effect18,	effect19,	effect1a,	effect1b,
			effect1c,	effect1d,	effect11x,	effect00};

int effect_exec(void) {

	int		tick;
	EFFECT	ef;
	int		r;

	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;
	r = GAMEEV_SUCCESS;
	if (ef->cmd < 0x20) {
		r = effunc[ef->cmd](ef, tick);
	}
	if (r == GAMEEV_SUCCESS) {
		vramcpy_cpy(gamecore.vram[gamecore.dispwin.vramnum],
													ef->src, &ef->pt, &ef->r);
		textwin_setrect();
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(r);
}


// ----

int effect_fadeinset(UINT32 tick, int num, UINT32 col) {

	EFFECT	ef;

	if ((num < 0) || (num >= GAMECORE_MAXVRAM)) {
		return(GAMEEV_SUCCESS);
	}
	ef = &gamecore.ef;
	ef->param = tick;
	ef->src = gamecore.vram[num];
	ef->tick = GETTICK();
	ef->progress = -1;
	ef->ex.ec.col = col;
	return(GAMEEV_FADEIN);
}

int effect_fadeinexec(void) {

	EFFECT	ef;
	int		tick;
	int		alpha;

	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;

	if (tick >= ef->param) {
		vrammix_cpy(mainvram, ef->src, NULL);
		scrnmng_draw(NULL);

		vrammix_cpy(gamecore.vram[gamecore.dispwin.vramnum], ef->src, NULL);
		textwin_setrect();
		return(GAMEEV_SUCCESS);
	}
	alpha = (tick << 6) / ef->param;
	if (ef->progress != alpha) {
		ef->progress = alpha;
		vrammix_mixcol(mainvram, ef->src, ef->ex.ec.col, alpha, NULL);
		scrnmng_draw(NULL);
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(GAMEEV_FADEIN);
}

int effect_fadeoutset(UINT32 tick, int num, UINT32 col) {

	EFFECT	ef;

	if ((num < 0) || (num >= GAMECORE_MAXVRAM)) {
		return(GAMEEV_SUCCESS);
	}
	ef = &gamecore.ef;
	ef->param = tick;
	ef->src = gamecore.vram[num];
	ef->tick = GETTICK();
	ef->progress = -1;
	ef->ex.ec.col = col;
	return(GAMEEV_FADEOUT);
}

int effect_fadeoutexec(void) {

	EFFECT	ef;
	int		tick;
	int		alpha;

	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;

	if (tick >= ef->param) {
		vram_fill(mainvram, NULL, ef->ex.ec.col, 0);
		scrnmng_draw(NULL);

		vram_fill(gamecore.vram[gamecore.dispwin.vramnum],
													NULL, ef->ex.ec.col, 0);
		textwin_setrect();
		return(GAMEEV_SUCCESS);
	}
	alpha = (tick << 6) / ef->param;
	if (ef->progress != alpha) {
		ef->progress = alpha;
		vrammix_mixcol(mainvram, ef->src, ef->ex.ec.col, 64 - alpha, NULL);
		scrnmng_draw(NULL);
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(GAMEEV_FADEOUT);
}


// ----

int effect_grayscale(void) {

	EFFECT	ef;
	int		tick;

	ef = &gamecore.ef;
	switch(ef->ex.eg.phase) {
		case 0:
			if ((ef->ex.eg.dir) && (ef->bmp)) {
				int r;
				BYTE *p;
				r = ef->bmp->scrnsize;
				p = ef->bmp->ptr;
				do {
					*p ^= 0xff;
					p++;
				} while(--r);
			}
			ef->tick = GETTICK();
#if 0
			if (textwin_isopen()) {
				vrammix_cpy(mainvram, gamecore.vram[gamecore.dispwin.vramnum],
																		NULL);
				scrnmng_draw(NULL);
			}
#endif
			ef->ex.eg.phase++;
			break;

		case 1:
			tick = GETTICK() - ef->tick;
			if (tick < ef->param) {
				tick *= 512;
				tick /= ef->param;
				if (ef->progress != tick) {
					ef->progress = tick;
					vrammix_graybmp(mainvram,
									gamecore.vram[gamecore.dispwin.vramnum],
									ef->src, ef->bmp, tick - 256, NULL);
					if (gamecore.sys.type & GAME_TEXTLAYER) {
						vramdraw_drawupper(mainvram, NULL);
					}
					scrnmng_draw(NULL);
				}
			}
			else {
				vrammix_cpy(mainvram, ef->src, NULL);
				if (gamecore.sys.type & GAME_TEXTLAYER) {
					vramdraw_drawupper(mainvram, NULL);
				}
				scrnmng_draw(NULL);
				ef->ex.eg.phase++;
			}
			break;

		default:
			effect_trush();

			vrammix_cpy(gamecore.vram[gamecore.dispwin.vramnum],
															ef->src, NULL);
			textwin_setrect();
			return(GAMEEV_SUCCESS);
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(GAMEEV_GRAYSCALE);
}


int effect_scroll(void) {

	EFFECT	ef;
	int		tick;
	POINT_T	proc;
	RECT_T	rect;
	int		r;

	ef = &gamecore.ef;
	r = GAMEEV_SUCCESS;
	proc = ef->ex.es.step;
	tick = GETTICK() - ef->tick;
	if (tick < ef->param) {
		r = GAMEEV_SCROLL;
		proc.x *= tick;
		proc.x /= ef->param;
		proc.y *= tick;
		proc.y /= ef->param;
	}
	if ((ef->ex.es.last.x != proc.x) || (ef->ex.es.last.y != proc.y)) {
		ef->ex.es.last = proc;
		rect.left = ef->r.left + ef->ex.es.start.x + proc.x;
		rect.top = ef->r.top + ef->ex.es.start.y + proc.y;
		rect.right = rect.left + (ef->r2.right - ef->r2.left);
		rect.bottom = rect.top + (ef->r2.bottom - ef->r2.top);
		vramcpy_cpy(mainvram, ef->src, &ef->pt, &rect);
		scrnmng_draw(&ef->r2);
	}
	if (r == GAMEEV_SUCCESS) {
		rect.left = ef->r.left + ef->ex.es.start.x + ef->ex.es.step.x;
		rect.top = ef->r.top + ef->ex.es.start.y + ef->ex.es.step.y;
		rect.right = rect.left + (ef->r2.right - ef->r2.left);
		rect.bottom = rect.top + (ef->r2.bottom - ef->r2.top);
		vramcpy_cpy(gamecore.vram[gamecore.dispwin.vramnum],
													ef->src, &ef->pt, &rect);
		textwin_setrect();
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(r);
}


static void clrcb(void *arg, const RECT_T *rect) {

	vram_zerofill((VRAMHDL)arg, rect);
}

int effect_quake(void) {

	EFFECT	ef;
	int		tick;
	VRAMHDL	dst;
	int		pos;
	RECT_T	rect;
	RECT_T	base;

	ef = &gamecore.ef;
	dst = mainvram;
	tick = GETTICK() - ef->tick;
	if (tick >= ef->param) {
		vramdraw_setrect(dst, NULL);
		vramdraw_draw();
		return(GAMEEV_SUCCESS);
	}
	pos = (tick * ef->ex.eq.cnt) / ef->param;
	if (ef->progress != pos) {
		ef->progress++;
		if (dst) {
			rect.left = ef->ex.eq.last.x;
			rect.top = ef->ex.eq.last.y;
			rect.right = rect.left + dst->width;
			rect.bottom = rect.top + dst->height;
			ef->ex.eq.last = ef->pt;
			if (!(ef->progress & 1)) {
				ef->ex.eq.last.x *= -1;
				ef->ex.eq.last.y *= -1;
			}
			vramcpy_move(dst, dst, &ef->ex.eq.last, &rect);
			vram_getrect(dst, &base);
			rect.left = ef->ex.eq.last.x;
			rect.top = ef->ex.eq.last.y;
			rect.right = rect.left + dst->width;
			rect.bottom = rect.top + dst->height;
			rect_enumout(&rect, &base, dst, clrcb);
			scrnmng_draw(NULL);
		}
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(GAMEEV_QUAKE);
}

