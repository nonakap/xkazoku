#include	"compiler.h"
#include	"gamecore.h"
#include	"scrnmng.h"
#include	"gamemsg.h"


void effect_trush(void) {

	EFFECT		ef;
	VRAMHDL		v;

	ef = &gamecore.ef;
	v = ef->tmp2;
	if (v) {
		ef->tmp2 = NULL;
		vram_destroy(v);
	}
	v = ef->tmp1;
	if (v) {
		ef->tmp1 = NULL;
		vram_destroy(v);
	}
	v = ef->bmp;
	if (v) {
		ef->bmp = NULL;
		vram_destroy(v);
	}
}


int effect_set(SCR_OPE *op) {

	VRAMHDL		dstvram;
	VRAMHDL		hdl;
	EFFECT		ef;

	dstvram = gamecore.vram[gamecore.dispwin.vramnum];
	ef = &gamecore.ef;
	switch(ef->cmd) {
		case 0x00: // type0			// 指定プレーンへコピー (透過なし)
		default:   // type6
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpy(hdl, ef->src, &ef->pt, &ef->r);
				if (hdl == dstvram) {
					vramdraw_setrect(hdl, &ef->r2);
					vramdraw_draw();
				}
			}
			return(GAMEEV_SUCCESS);

		case 0x01: // type0			// 指定プレーンへコピー (透過あり)
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpyex(hdl, ef->src, &ef->pt, &ef->r);
				if (hdl == dstvram) {
					vramdraw_setrect(hdl, &ef->r2);
					vramdraw_draw();
				}
			}
			return(GAMEEV_SUCCESS);

		case 0x02: // type1			// random pattern
			vramcpy_cpy(dstvram, ef->src, &ef->pt, &ef->r);
			ef->tmp1 = vramdraw_createtmp(&ef->r2);
			break;

		case 0x03: // type1			// wipe right
		case 0x04: // type1			// wipe left
		case 0x05: // type1			// wipe down
		case 0x06: // type1			// wipe up
		case 0x07: // type1			// wipe with pattern right
		case 0x08: // type1			// wipe with pattern left
		case 0x09: // type1			// wipe with pattern down
		case 0x0a: // type1			// wipe with pattern up
		case 0x0d: // type1			// mosaic in
		case 0x0e: // type1			// mosaic out
			break;

		case 0x0b: // type1			// not support?
		case 0x0c: // type1			// not support?
			TRACEOUT(("effect cmd err: %x (not support?)", ef->cmd));
			break;

		case 0x0f: // type2			// draw-hline down
		case 0x10: // type2			// draw-vline right
		case 0x11: // type2			// draw-hline up
		case 0x12: // type2			// draw-vline left
			if ((scr_getval(op, &ef->param2) != SUCCESS) ||
				(scr_getval(op, &ef->param3) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			if (ef->param2 <= 0) {
				ef->param2 = 1;
			}
			if (ef->param3 <= 0) {
				ef->param3 = 1;
			}
#ifdef SIZE_QVGA
			ef->param2 = (ef->param2 + 1) >> 1;
			ef->param3 = (ef->param3 + 1) >> 1;
#endif
			break;

		case 0x13: // type1			// FadeIn
			ef->tmp2 = vramdraw_createtmp(&ef->r2);
			vramcpy_cpy(dstvram, ef->src, &ef->pt, &ef->r);
			ef->tmp1 = vramdraw_createtmp(&ef->r2);
			break;

		case 0x14: // type3			// copy with alpha
			if (scr_getval(op, &ef->param2) != SUCCESS) {
				return(GAMEEV_WRONGLENG);
			}
			if ((ef->param >= 0) && (ef->param < GAMECORE_MAXVRAM)) {
				hdl = gamecore.vram[ef->param];
				vramcpy_cpyalpha(hdl, ef->src,
										255 - ef->param2, &ef->pt, &ef->r);
				if (hdl == dstvram) {
					vramdraw_setrect(hdl, &ef->r2);
					vramdraw_draw();
				}
			}
			return(GAMEEV_SUCCESS);

		// todo
		case 0x15:	// type4
			TRACEOUT(("effect cmd err: %x", ef->cmd));
			vramcpy_cpy(dstvram, ef->src, &ef->pt, &ef->r);
			vramdraw_setrect(dstvram, &ef->r2);
			vramdraw_draw();
			return(GAMEEV_FORCE);

		case 0x18: // type1			// width out
		case 0x19: // type1			// width in
		case 0x1a: // type1			// height out
		case 0x1b: // type1			// height in
			break;

		case 0x1c:	// type5
		case 0x1d:	// type5
			if ((scr_getval(op, &ef->param2) != SUCCESS) ||
				(scr_getval(op, &ef->param3) != SUCCESS) ||
				(scr_getval(op, &ef->param4) != SUCCESS) ||
				(scr_getval(op, &ef->param5) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			ef->param4 &= 3;
			if ((ef->param2 <= 0) || (ef->param3 <= 0) || (ef->param5 <= 0)) {
				vramcpy_cpy(dstvram, ef->src, &ef->pt, &ef->r);
				vramdraw_setrect(dstvram, &ef->r2);
				vramdraw_draw();
				return(GAMEEV_SUCCESS);
			}
			if (!(ef->cmd & 1)) {
				ef->cnt = ef->r2.bottom - ef->r2.top;
			}
			else {
				ef->cnt = ef->r2.right - ef->r2.left;
			}
#ifdef SIZE_QVGA
			ef->param2 = (ef->param2 + 1) >> 1;
#endif
			ef->cnt += (ef->param2 - 1);
			ef->cnt /= ef->param2;
			break;
	}

	ef->tick = GETTICK();
	return(GAMEEV_EFFECT);
}


// ----

static int effect02(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	next = (ef->param * ef->lastalpha) / 64;
	if (tick >= next) {
		BYTE	pat[8];
		UINT	pos;
		int		i;
		BYTE	bit;

		ZeroMemory(pat, sizeof(pat));
		pos = rand();
		pos %= (64 - ef->lastalpha);
		i = 0;
		bit = 0x80;
		do {
			if (!(ef->pat8[i] & bit)) {
				if (!pos) {
					ef->pat8[i] |= bit;
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
		vrammix_cpypat(mainvram, ef->tmp1, pat, &ef->r2);
		scrnmng_draw(&ef->r2);
		ef->lastalpha++;
		if (ef->lastalpha >= 64) {
			vram_destroy(ef->tmp1);
			ef->tmp1 = NULL;
			return(GAMEEV_SUCCESS);
		}
	}
	(void)vram;
	return(GAMEEV_EFFECT);
}

static int effect03(EFFECT ef, VRAMHDL vram, int tick) {

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
		pos = (pos * tick) / ef->param;
	}
	w = pos - ef->lastalpha;
	if (w > 0) {
		int	x;
		pt = ef->pt;
		r = ef->r;
		x = ef->lastalpha;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect04(EFFECT ef, VRAMHDL vram, int tick) {

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
		pos = (pos * tick) / ef->param;
	}
	w = pos - ef->lastalpha;
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
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect05(EFFECT ef, VRAMHDL vram, int tick) {

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
		pos = (pos * tick) / ef->param;
	}
	h = pos - ef->lastalpha;
	if (h > 0) {
		int	y;
		pt = ef->pt;
		r = ef->r;
		y = ef->lastalpha;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect06(EFFECT ef, VRAMHDL vram, int tick) {

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
		pos = (pos * tick) / ef->param;
	}
	h = pos - ef->lastalpha;
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
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect0b(EFFECT ef, VRAMHDL vram, int tick) {

	vramcpy_cpy(vram, ef->src, &ef->pt, &ef->r);
	vramdraw_setrect(vram, &ef->r2);
	vramdraw_draw();
	(void)tick;
	return(GAMEEV_SUCCESS);
}

static int effect0d(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	if (tick >= ef->param) {
		vramcpy_cpy(vram, ef->src, &ef->pt, &ef->r);
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		return(GAMEEV_SUCCESS);
	}
	next = ((tick * 8) / ef->param) + 1;
	if (ef->lastalpha != next) {
		ef->lastalpha = next;
		vramcpy_mosaic(vram, ef->src, 512 >> next, &ef->pt, &ef->r);
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
	}
	return(GAMEEV_EFFECT);
}

static int effect0e(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;
	int		r;

	if (tick >= ef->param) {
		next = 8;
		r = GAMEEV_SUCCESS;
	}
	else {
		next = (tick * 8) / ef->param;
		r = GAMEEV_EFFECT;
	}
	if (ef->lastalpha != next) {
		ef->lastalpha = next;
		vramcpy_mosaic(vram, ef->src, 1 << next, &ef->pt, &ef->r);
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
	}
	return(r);
}

static int effect0f(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	next = (tick * ef->param2) / ef->param;
	if (ef->lastalpha < next) {
		POINT_T	pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		pt.y += ef->lastalpha;
		rct.top += ef->lastalpha;
		while(rct.top < ef->r.bottom) {
			rct.bottom = min(ef->r.bottom, rct.top + ef->param3);
			vramcpy_cpy(vram, ef->src, &pt, &rct);
			pt.y += ef->param2;
			rct.top += ef->param2;
		}
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		ef->lastalpha += ef->param3;
		if (ef->lastalpha >= ef->param2) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect10(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	next = (tick * ef->param2) / ef->param;
	if (ef->lastalpha < next) {
		POINT_T	pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		pt.x += ef->lastalpha;
		rct.left += ef->lastalpha;
		while(rct.left < ef->r.right) {
			rct.right = min(ef->r.right, rct.left + ef->param3);
			vramcpy_cpy(vram, ef->src, &pt, &rct);
			pt.x += ef->param2;
			rct.left += ef->param2;
		}
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		ef->lastalpha += ef->param3;
		if (ef->lastalpha >= ef->param2) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect11(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	next = (tick * ef->param2) / ef->param;
	if (ef->lastalpha < next) {
		POINT_T pt;
		RECT_T	rct;
		pt = ef->pt;
		rct = ef->r;
		rct.bottom -= ef->lastalpha;
		while(rct.bottom > ef->r.top) {
			int y;
			rct.top = rct.bottom - ef->param3;
			y = rct.top - ef->r.top;
			if (y < 0) {
				rct.top -= y;
				y = 0;
			}
			pt.y = ef->pt.y + y;
			vramcpy_cpy(vram, ef->src, &pt, &rct);
			rct.bottom -= ef->param2;
		}
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		ef->lastalpha += ef->param3;
		if (ef->lastalpha >= ef->param2) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect12(EFFECT ef, VRAMHDL vram, int tick) {

	int		next;

	next = (tick * ef->param2) / ef->param;
	if (ef->lastalpha < next) {
		POINT_T pt;
		RECT_T rct;
		pt = ef->pt;
		rct = ef->r;
		rct.right -= ef->lastalpha;
		while(rct.right > ef->r.left) {
			int x;
			rct.left = rct.right - ef->param3;
			x = rct.left - ef->r.left;
			if (x < 0) {
				rct.left -= x;
				x = 0;
			}
			pt.x = ef->pt.x + x;
			vramcpy_cpy(vram, ef->src, &pt, &rct);
			rct.right -= ef->param2;
		}
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		ef->lastalpha += ef->param3;
		if (ef->lastalpha >= ef->param2) {
			return(GAMEEV_SUCCESS);
		}
	}
	return(GAMEEV_EFFECT);
}

static int effect13(EFFECT ef, VRAMHDL vram, int tick) {

	int		alpha;

	if (tick >= ef->param) {
		effect_trush();
		vramdraw_setrect(vram, &ef->r2);
		vramdraw_draw();
		return(GAMEEV_SUCCESS);
	}
	alpha = (tick << 6) / ef->param;
	if (ef->lastalpha != alpha) {
		ef->lastalpha = alpha;
		vrammix_mix(mainvram, ef->tmp2, ef->tmp1, alpha, &ef->r2);
		scrnmng_draw(&ef->r2);
	}
	return(GAMEEV_EFFECT);
}

static int effect18(EFFECT ef, VRAMHDL vram, int tick) {

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
	w = pos - ef->lastalpha;
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
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		w += pad;
		x = width + ef->lastalpha;
		pt.x = ef->pt.x + x;
		r.left = ef->r.left + x;
		r.right = r.left + w;
		r2.left = ef->r2.left + x;
		r2.right = r2.left + w;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect19(EFFECT ef, VRAMHDL vram, int tick) {

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
	w = pos - ef->lastalpha;
	if (w > 0) {
		int x;
		pt = ef->pt;
		r = ef->r;
		x = ef->lastalpha;
		pt.x += x;
		r.left += x;
		r.right = r.left + w;
		r2.left += x;
		r2.right = r2.left + w;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		w += pad;
		x = (width * 2) - pos;
		pt.x = ef->pt.x + x;
		r.left = ef->r.left + x;
		r.right = r.left + w;
		r2.left = ef->r2.left + x;
		r2.right = r2.left + w;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= width) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1a(EFFECT ef, VRAMHDL vram, int tick) {

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
	h = pos - ef->lastalpha;
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
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		h += pad;
		y = height + ef->lastalpha;
		pt.y = ef->pt.y + y;
		r.top = ef->r.top + y;
		r.bottom = r.top + h;
		r2.top = ef->r2.top + y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1b(EFFECT ef, VRAMHDL vram, int tick) {

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
	h = pos - ef->lastalpha;
	if (h > 0) {
		int y;
		pt = ef->pt;
		r = ef->r;
		y = ef->lastalpha;
		pt.y += y;
		r.top += y;
		r.bottom = r.top + h;
		r2.top += y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		h += pad;
		y = (height * 2) - pos;
		pt.y = ef->pt.y + y;
		r.top = ef->r.top + y;
		r.bottom = r.top + h;
		r2.top = ef->r2.top + y;
		r2.bottom = r2.top + h;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha = pos;
	}
	if (pos >= height) {
		return(GAMEEV_SUCCESS);
	}
	return(GAMEEV_EFFECT);
}

static int effect1c(EFFECT ef, VRAMHDL vram, int tick) {

	int		cur = 0;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;
	int		y;

	pt = ef->pt;
	r = ef->r;
	r2 = ef->r2;
	y = ef->lastalpha * ef->param2;
	pt.y += y;
	r.top += y;
	r2.top += y;

	cur = tick - ef->basetick;
	while(cur >= ef->param) {
		r.bottom = r.top + ef->param2;
		r2.bottom = r2.top + ef->param2;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha++;
		if (ef->lastalpha >= ef->cnt) {
			return(GAMEEV_SUCCESS);
		}
		pt.y += ef->param2;
		r.top += ef->param2;
		r2.top += ef->param2;
		cur -= ef->param5;
		ef->basetick += ef->param5;
	}
	while((cur > 0) && (r.top < ef->r.bottom)) {
		y = (ef->param2 * cur) / ef->param;
		r.bottom = r.top + y;
		r2.bottom = r2.top + y;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		cur -= ef->param5;
		pt.y += ef->param2;
		r.top += ef->param2;
		r2.top += ef->param2;
	}
	return(GAMEEV_EFFECT);
}

static int effect1d(EFFECT ef, VRAMHDL vram, int tick) {

	int		cur = 0;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;
	int		x;

	pt = ef->pt;
	r = ef->r;
	r2 = ef->r2;
	x = ef->lastalpha * ef->param2;
	pt.x += x;
	r.left += x;
	r2.left += x;

	cur = tick - ef->basetick;
	while(cur >= ef->param) {
		r.right = r.left + ef->param2;
		r2.right = r2.left + ef->param2;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		ef->lastalpha++;
		if (ef->lastalpha >= ef->cnt) {
			return(GAMEEV_SUCCESS);
		}
		pt.x += ef->param2;
		r.left += ef->param2;
		r2.left += ef->param2;
		cur -= ef->param5;
		ef->basetick += ef->param5;
	}
	while((cur > 0) && (r.left < ef->r.right)) {
		x = (ef->param2 * cur) / ef->param;
		r.right = r.left + x;
		r2.right = r2.left + x;
		vramcpy_cpy(vram, ef->src, &pt, &r);
		vramdraw_setrect(vram, &r2);
		vramdraw_draw();
		cur -= ef->param5;
		pt.x += ef->param2;
		r.left += ef->param2;
		r2.left += ef->param2;
	}
	return(GAMEEV_EFFECT);
}


// ----

int effect_exec(void) {

	VRAMHDL		dstvram;
	int			tick;
	EFFECT		ef;

	dstvram = gamecore.vram[gamecore.dispwin.vramnum];
	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;
	switch(ef->cmd) {
		case 0x02:
			return(effect02(ef, dstvram, tick));

		case 0x03:
		case 0x07:	// todo
			return(effect03(ef, dstvram, tick));

		case 0x04:
		case 0x08:	// todo
			return(effect04(ef, dstvram, tick));

		case 0x05:
		case 0x09:	// todo
			return(effect05(ef, dstvram, tick));

		case 0x06:
		case 0x0a:	// todo
			return(effect06(ef, dstvram, tick));

		case 0x0b:
		case 0x0c:
			return(effect0b(ef, dstvram, tick));

		case 0x0d:
			return(effect0d(ef, dstvram, tick));

		case 0x0e:
			return(effect0e(ef, dstvram, tick));

		case 0x0f:
			return(effect0f(ef, dstvram, tick));

		case 0x10:
			return(effect10(ef, dstvram, tick));

		case 0x11:
			return(effect11(ef, dstvram, tick));

		case 0x12:
			return(effect12(ef, dstvram, tick));

		case 0x13:
			return(effect13(ef, dstvram, tick));

		case 0x18:
			return(effect18(ef, dstvram, tick));

		case 0x19:
			return(effect19(ef, dstvram, tick));

		case 0x1a:
			return(effect1a(ef, dstvram, tick));

		case 0x1b:
			return(effect1b(ef, dstvram, tick));

		case 0x1c:
			return(effect1c(ef, dstvram, tick));

		case 0x1d:
			return(effect1d(ef, dstvram, tick));
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(GAMEEV_SUCCESS);
}


// ----

int effect_fadeout(UINT32 tick, BYTE *col) {

	EFFECT	ef;

	ef = &gamecore.ef;
	ef->tmp1 = vramdraw_createtmp(NULL);

	ef->param = tick;
	ef->tick = GETTICK();
	ef->lastalpha = 0;
	ef->col = MAKEPALETTE(col[0], col[1], col[2]);
	return(GAMEEV_FADEOUT);
}

int effect_fadeoutexec(void) {

	EFFECT	ef;
	int		tick;
	VRAMHDL	hdl;

	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;

	if (tick >= ef->param) {
		hdl = gamecore.vram[gamecore.dispwin.vramnum];
		vram_fill(hdl, NULL, ef->col, 0);
		vramdraw_setrect(hdl, NULL);
		vramdraw_draw();
		vram_destroy(ef->tmp1);
		ef->tmp1 = NULL;
		return(GAMEEV_SUCCESS);
	}
	else {
		int alpha;
		alpha = (tick << 6) / ef->param;
		if (ef->lastalpha != alpha) {
			ef->lastalpha = alpha;
			vrammix_mixcol(mainvram, ef->tmp1, ef->col, 64 - alpha, NULL);
			scrnmng_draw(NULL);
		}
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

	switch(ef->cnt) {
		case 0:
			ef->tmp1 = vramdraw_createtmp(NULL);
			ef->cnt++;
			break;

		case 1:
			vramcpy_cpy(gamecore.vram[gamecore.dispwin.vramnum],
								ef->src, NULL, NULL);
			ef->tmp2 = vramdraw_createtmp(NULL);
			ef->cnt++;
			break;

		case 2:
			if ((ef->param2) && (ef->bmp)) {
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
			ef->cnt++;
			break;

		case 3:
			tick = GETTICK() - ef->tick;
			if (tick < ef->param) {
				tick *= 512;
				tick /= ef->param;
				if (ef->lastalpha != tick) {
					ef->lastalpha = tick;
					vrammix_graybmp(mainvram, ef->tmp1, ef->tmp2,
										ef->bmp, tick - 256, NULL);
					scrnmng_draw(NULL);
				}
			}
			else {
				vramdraw_setrect(gamecore.vram[gamecore.dispwin.vramnum],
																	NULL);
				vramdraw_draw();
				ef->cnt++;
				break;
			}
			break;

		default:
			effect_trush();
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
	int		procy;
	RECT_T	rect;
	VRAMHDL	dstvram;
	int		r;

	ef = &gamecore.ef;
	tick = GETTICK() - ef->tick;
	if (tick >= ef->param3) {
		r = GAMEEV_SUCCESS;
		procy = ef->param2;
	}
	else {
		r = GAMEEV_SCROLL;
		procy = (tick * ef->param2) / ef->param3;
	}
	if (ef->lastalpha != procy) {
		ef->lastalpha = procy;
		rect.left = ef->r.left;
		rect.top = ef->r.top + ef->param + procy;
		rect.right = rect.left + (ef->r2.right - ef->r2.left);
		rect.bottom = rect.top + (ef->r2.bottom - ef->r2.top);
		dstvram = gamecore.vram[gamecore.dispwin.vramnum];
		vramcpy_cpy(dstvram, ef->src, &ef->pt, &rect);
		vramdraw_setrect(dstvram, &ef->r2);
		vramdraw_draw();
	}
#ifdef GAMEMSG_EFFECT
	gamemsg_send(GAMEMSG_EFFECT, 0);
#endif
	return(r);
}

