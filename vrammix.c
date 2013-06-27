#include	"compiler.h"
#include	"vram.h"
#include	"vrammix.h"
#include	"fontmng.h"


static BOOL cpyrect(const VRAMHDL dst, const VRAMHDL src,
						const POINT_T *pt, const RECT_T *rct, MIX_RECT *r) {

	POINT_T		p;
	int			width, height;

	if ((dst == NULL) || (src == NULL) || (dst->bpp != src->bpp)) {
		return(FAILURE);
	}
	if (pt) {
		p = *pt;
	}
	else {
		p.x = 0;
		p.y = 0;
	}
	r->srcpos = 0;
	if (rct) {
		width = min(rct->right, src->width);
		if (rct->left >= 0) {
			r->srcpos += rct->left;
			width -= rct->left;
		}
		else {
			p.x -= rct->left;
		}
		height = min(rct->bottom, src->height);
		if (rct->top >= 0) {
			r->srcpos += rct->top * src->width;
			height -= rct->top;
		}
		else {
			p.y -= rct->top;
		}
	}
	else {
		width = src->width;
		height = src->height;
	}

	r->dstpos = 0;
	r->width = min(width + p.x, dst->width);
	if (p.x > 0) {
		r->dstpos += p.x;
		r->width = min(r->width, dst->width) - p.x;
	}
	else {
		r->srcpos -= p.x;
	}
	if (r->width <= 0) {
		return(FAILURE);
	}

	r->height = min(height + p.y, dst->height);
	if (p.y > 0) {
		r->dstpos += p.y * dst->width;
		r->height = min(r->height, dst->height) - p.y;
	}
	else {
		r->srcpos -= p.y * src->width;
	}
	if (r->height <= 0) {
		return(FAILURE);
	}
	return(SUCCESS);
}

static BOOL mixrect(const VRAMHDL dst, const VRAMHDL src,
											const RECT_T *rct, MIX_RECT *r) {

	int		pos;
	RECT_T	s;

	if ((dst == NULL) || (src == NULL) || (dst->bpp != src->bpp)) {
		return(FAILURE);
	}
	r->srcpos = 0;
	if (rct == NULL) {
		s.left = 0;
		s.top = 0;
		s.right = dst->width;
		s.bottom = dst->height;
		r->dstpos = 0;
	}
	else {
		if ((rct->bottom <= 0) || (rct->right <= 0) ||
			(rct->left >= dst->width) || (rct->top >= dst->height)) {
			return(FAILURE);
		}
		s.left = max(rct->left, 0);
		s.top = max(rct->top, 0);
		s.right = min(rct->right, dst->width);
		s.bottom = min(rct->bottom, dst->height);
		if ((s.top >= s.bottom) || (s.left >= s.right)) {
			return(FAILURE);
		}
		r->dstpos = s.top * dst->width;
		r->dstpos += s.left;
	}

	pos = src->posy - s.top;
	if (pos < 0) {
		r->srcpos -= pos * src->width;
		r->height = min(src->height + pos, s.bottom - s.top);
	}
	else {
		r->dstpos += pos * dst->width;
		r->height = min(s.bottom - s.top - pos, src->height);
	}
	if (r->height <= 0) {
		return(FAILURE);
	}

	pos = src->posx - s.left;
	if (pos < 0) {
		r->srcpos -= pos;
		r->width = min(src->width + pos, s.right - s.left);
	}
	else {
		r->dstpos += pos;
		r->width = min(s.right - s.left - pos, src->width);
	}
	if (r->width <= 0) {
		return(FAILURE);
	}
	return(SUCCESS);
}


// ----

static void vramsub_cpy(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	do {
		CopyMemory(q, p, mr->width * src->xalign);
		p += src->yalign;
		q += dst->yalign;
	} while(--mr->height);
}

static void vramsub_cpyall(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		height;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	height = mr->height;
	do {
		CopyMemory(q, p, mr->width * src->xalign);
		p += src->yalign;
		q += dst->yalign;
	} while(--height);
	if ((src->alpha) && (dst->alpha)) {
		p = src->alpha + mr->srcpos;
		q = dst->alpha + mr->dstpos;
		do {
			CopyMemory(q, p, mr->width);
			p += src->width;
			q += dst->height;
		} while(--mr->height);
	}
}

static void vramsub_cpy2(VRAMHDL dst, const VRAMHDL src, BYTE alpha,
															MIX_RECT *mr) {

	BYTE	*p, *q, *r;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	r = dst->alpha + mr->dstpos;
	do {
		CopyMemory(q, p, mr->width * src->xalign);
		FillMemory(r, mr->width, alpha);
		p += src->yalign;
		q += dst->yalign;
		r += dst->width;
	} while(--mr->height);
}


// ---- bpp=16

#ifdef SUPPORT_16BPP

static void vramsub_cpyp16(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
													MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;
	int		i = 0;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 2;
	do {
		UINT pat;
		x = mr->width;
		pat = pat8[i & 7];
		i++;
		do {
			pat <<= 1;
			if (pat & 0x100) {
				*(UINT16 *)q = *(UINT16 *)p;
				pat++;
			}
			p += 2;
			q += 2;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpyex16(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 2;

	do {
		x = mr->width;
		do {
			UINT16 dat;
			dat = *(UINT16 *)p;
			p += 2;
			if (dat) {
				*(UINT16 *)q = dat;
			}
			q += 2;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpyexa16(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int		x;
	int		step;

	a = src->alpha + mr->srcpos;
	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 2;

	do {
		x = mr->width;
		do {
			UINT alpha;
			alpha = *a++;
			if (alpha) {
				UINT s1, s2, d;
				alpha++;
				s1 = *(UINT16 *)p;
				s2 = *(UINT16 *)q;
				d = MAKEALPHA16(s2, s1, B16MASK, alpha, 8);
				d |= MAKEALPHA16(s2, s1, G16MASK, alpha, 8);
				d |= MAKEALPHA16(s2, s1, R16MASK, alpha, 8);
				*(UINT16 *)q = (UINT16)d;
			}
			p += 2;
			q += 2;
		} while(--x);
		a += src->width - mr->width;
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpya16(VRAMHDL dst, const VRAMHDL src,
												UINT alpha, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 2;

	do {
		x = mr->width;
		do {
			UINT s1, s2, d;
			s1 = *(UINT16 *)p;
			s2 = *(UINT16 *)q;
			d = MAKEALPHA16(s2, s1, B16MASK, alpha, 8);
			d |= MAKEALPHA16(s2, s1, G16MASK, alpha, 8);
			d |= MAKEALPHA16(s2, s1, R16MASK, alpha, 8);
			*(UINT16 *)q = (UINT16)d;
			p += 2;
			q += 2;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_mix16(VRAMHDL dst,
							const VRAMHDL src1, const VRAMHDL src2,
											UINT alpha, MIX_RECT *mr) {
	BYTE	*p, *q, *r;
	int		x;
	int		step;

	p = src1->ptr + (mr->srcpos * src1->xalign);
	q = src2->ptr + (mr->srcpos * src1->xalign);
	r = dst->ptr + (mr->dstpos * src1->xalign);
	step = mr->width * 2;

	do {
		x = mr->width;
		do {
			UINT s1, s2, d;
			s1 = *(UINT16 *)p;
			s2 = *(UINT16 *)q;
			d = MAKEALPHA16(s1, s2, B16MASK, alpha, 6);
			d |= MAKEALPHA16(s1, s2, G16MASK, alpha, 6);
			d |= MAKEALPHA16(s1, s2, R16MASK, alpha, 6);
			*(UINT16 *)r = (UINT16)d;
			p += 2;
			q += 2;
			r += 2;
		} while(--x);
		p += src1->yalign - step;
		q += src1->yalign - step;
		r += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_mixcol16(VRAMHDL dst, const VRAMHDL src, UINT32 color,
											UINT alpha, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;
	int		tmp;
	int		c16[3];

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 2;

	tmp = MAKE16PAL(color);
	c16[0] = tmp & B16MASK;
	c16[1] = tmp & G16MASK;
	c16[2] = tmp & R16MASK;
	do {
		x = mr->width;
		do {
			UINT s, d;
			s = *(UINT16 *)p;
			d = MAKEALPHA16s(c16[0], s, B16MASK, alpha, 6);
			d |= MAKEALPHA16s(c16[1], s, G16MASK, alpha, 6);
			d |= MAKEALPHA16s(c16[2], s, R16MASK, alpha, 6);
			*(UINT16 *)q = (UINT16)d;
			p += 2;
			q += 2;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_gray16(VRAMHDL dst,
							const VRAMHDL src1, const VRAMHDL src2,
							const VRAMHDL bmp, 
							int delta, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int		rm;
	int		x, y;
	int		step;
	int		xstep, ystep;

	if ((bmp == NULL) || (bmp->bpp != 8)) {
		return;
	}

	p = src1->ptr + (mr->srcpos * src1->xalign);
	q = src2->ptr + (mr->srcpos * src1->xalign);
	r = dst->ptr + (mr->dstpos * src1->xalign);
	step = mr->width * 2;

	xstep = (bmp->width << 10) / mr->width;
	ystep = (bmp->height << 10) / mr->height;

	y = 0;
	do {
		a = bmp->ptr + ((y >> 10) * bmp->yalign);
		rm = mr->width;
		x = 0;
		do {
			int alpha;
			alpha = a[x >> 10] + delta + 1;
			if (alpha >= 256) {
				*(UINT16 *)r = *(UINT16 *)q;
			}
			else if (alpha > 0) {
				UINT s1, s2, d;
				s1 = *(UINT16 *)p;
				s2 = *(UINT16 *)q;
				d = MAKEALPHA16(s1, s2, B16MASK, alpha, 8);
				d |= MAKEALPHA16(s1, s2, G16MASK, alpha, 8);
				d |= MAKEALPHA16(s1, s2, R16MASK, alpha, 8);
				*(UINT16 *)r = (UINT16)d;
			}
			p += 2;
			q += 2;
			r += 2;
			x += xstep;
		} while(--rm);
		p += src1->yalign - step;
		q += src2->yalign - step;
		r += dst->yalign - step;
		y += ystep;
	} while(--mr->height);
}

static void vramsub_mosaic16(VRAMHDL dst, const VRAMHDL src, int dot,
															MIX_RECT *mr) {

const BYTE	*p;
	BYTE	*q;
	BYTE	*r;
	int		x;
	int		sstep;
	int		dstep;
	int		xstep;
	int		ystep;
	int		xx;
	int		yy;
	int		xstep2;
	UINT16	col;

	p = src->ptr + (mr->srcpos * 2);
	q = dst->ptr + (mr->dstpos * 2);
	sstep = (src->yalign * dot) - (mr->width * 2);
	dstep = (dst->yalign * dot) - (mr->width * 2);

	do {
		ystep = min(mr->height, dot);
		x = mr->width;
		do {
			xstep = min(x, dot);
			xstep2 = xstep * 2;
			r = q;
			yy = ystep;
			col = *(UINT16 *)p;
			do {
				xx = xstep;
				do {
					*(UINT16 *)r = col;
					r += 2;
				} while(--xx);
				r += dst->yalign;
				r -= xstep2;
			} while(--yy);
			p += xstep2;
			q += xstep2;
			x -= xstep;
		} while(x);
		p += sstep;
		q += dstep;
		mr->height -= ystep;
	} while(mr->height);
}
#endif


// ---- bpp=24

#ifdef SUPPORT_24BPP

static void vramsub_cpyp24(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
														MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;
	int		i = 0;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 3;
	do {
		UINT pat;
		x = mr->width;
		pat = pat8[i & 7];
		i++;
		do {
			pat <<= 1;
			if (pat & 0x100) {
				q[0] = p[0];
				q[1] = p[1];
				q[2] = p[2];
				pat++;
			}
			p += 3;
			q += 3;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpyex24(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 3;

	do {
		x = mr->width;
		do {
			BYTE r, g, b;
			b = p[0];
			g = p[1];
			r = p[2];
			p += 3;
			if ((b) || (g) || (r)) {
				q[0] = b;
				q[1] = g;
				q[2] = r;
			}
			q += 3;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpyexa24(VRAMHDL dst, const VRAMHDL src, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int		x;
	int		step;

	a = src->alpha + mr->srcpos;
	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 3;

	do {
		x = mr->width;
		do {
			UINT alpha;
			alpha = *a++;
			if (alpha) {
				alpha++;
				q[0] = (BYTE)MAKEALPHA24(q[0], p[0], alpha, 8);
				q[1] = (BYTE)MAKEALPHA24(q[1], p[1], alpha, 8);
				q[2] = (BYTE)MAKEALPHA24(q[2], p[2], alpha, 8);
			}
			p += 3;
			q += 3;
		} while(--x);
		a += src->width - mr->width;
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_cpya24(VRAMHDL dst, const VRAMHDL src,
												UINT alpha, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 3;

	do {
		x = mr->width;
		do {
			q[0] = (BYTE)MAKEALPHA24(q[0], p[0], alpha, 8);
			q[1] = (BYTE)MAKEALPHA24(q[1], p[1], alpha, 8);
			q[2] = (BYTE)MAKEALPHA24(q[2], p[2], alpha, 8);
			p += 3;
			q += 3;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_mix24(VRAMHDL dst,
							const VRAMHDL src1, const VRAMHDL src2,
											UINT alpha, MIX_RECT *mr) {
	BYTE	*p, *q, *r;
	int		x;
	int		step;

	p = src1->ptr + (mr->srcpos * src1->xalign);
	q = src2->ptr + (mr->srcpos * src1->xalign);
	r = dst->ptr + (mr->dstpos * src1->xalign);
	step = mr->width * 3;

	do {
		x = mr->width;
		do {
			r[0] = (BYTE)MAKEALPHA24(p[0], q[0], alpha, 6);
			r[1] = (BYTE)MAKEALPHA24(p[1], q[1], alpha, 6);
			r[2] = (BYTE)MAKEALPHA24(p[2], q[2], alpha, 6);
			p += 3;
			q += 3;
			r += 3;
		} while(--x);
		p += src1->yalign - step;
		q += src1->yalign - step;
		r += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_mixcol24(VRAMHDL dst, const VRAMHDL src, UINT32 color,
											UINT alpha, MIX_RECT *mr) {

	BYTE	*p, *q;
	int		x;
	int		step;
	int		c24[3];

	p = src->ptr + (mr->srcpos * src->xalign);
	q = dst->ptr + (mr->dstpos * src->xalign);
	step = mr->width * 3;

	c24[0] = color & 0xff;
	c24[1] = (color >> 8) & 0xff;
	c24[2] = (color >> 16) & 0xff;
	do {
		x = mr->width;
		do {
			q[0] = (BYTE)MAKEALPHA24(c24[0], p[0], alpha, 6);
			q[1] = (BYTE)MAKEALPHA24(c24[1], p[1], alpha, 6);
			q[2] = (BYTE)MAKEALPHA24(c24[2], p[2], alpha, 6);
			p += 3;
			q += 3;
		} while(--x);
		p += src->yalign - step;
		q += dst->yalign - step;
	} while(--mr->height);
}

static void vramsub_gray24(VRAMHDL dst,
							const VRAMHDL src1, const VRAMHDL src2,
							const VRAMHDL bmp, 
							int delta, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int		rm;
	int		x, y;
	int		step;
	int		xstep, ystep;

	if ((bmp == NULL) || (bmp->bpp != 8)) {
		return;
	}

	p = src1->ptr + (mr->srcpos * src1->xalign);
	q = src2->ptr + (mr->srcpos * src1->xalign);
	r = dst->ptr + (mr->dstpos * src1->xalign);
	step = mr->width * 3;

	xstep = (bmp->width << 10) / mr->width;
	ystep = (bmp->height << 10) / mr->height;

	y = 0;
	do {
		a = bmp->ptr + ((y >> 10) * bmp->yalign);
		rm = mr->width;
		x = 0;
		do {
			int alpha;
			alpha = a[x >> 10] + delta + 1;
			if (alpha >= 256) {
				r[0] = q[0];
				r[1] = q[1];
				r[2] = q[2];
			}
			else if (alpha > 0) {
				r[0] = (BYTE)MAKEALPHA24(p[0], q[0], alpha, 8);
				r[1] = (BYTE)MAKEALPHA24(p[1], q[1], alpha, 8);
				r[2] = (BYTE)MAKEALPHA24(p[2], q[2], alpha, 8);
			}
			p += 3;
			q += 3;
			r += 3;
			x += xstep;
		} while(--rm);
		p += src1->yalign - step;
		q += src2->yalign - step;
		r += dst->yalign - step;
		y += ystep;
	} while(--mr->height);
}

static void vramsub_mosaic24(VRAMHDL dst, const VRAMHDL src, int dot,
															MIX_RECT *mr) {

const BYTE	*p;
	BYTE	*q;
	BYTE	*r;
	int		x;
	int		sstep;
	int		dstep;
	int		xstep;
	int		ystep;
	int		xx;
	int		yy;
	int		xstep3;

	p = src->ptr + (mr->srcpos * 3);
	q = dst->ptr + (mr->dstpos * 3);
	sstep = (src->yalign * dot) - (mr->width * 3);
	dstep = (dst->yalign * dot) - (mr->width * 3);

	do {
		ystep = min(mr->height, dot);
		x = mr->width;
		do {
			xstep = min(x, dot);
			xstep3 = xstep * 3;
			r = q;
			yy = ystep;
			do {
				xx = xstep;
				do {
					r[0] = p[0];
					r[1] = p[1];
					r[2] = p[2];
					r += 3;
				} while(--xx);
				r += dst->yalign;
				r -= xstep3;
			} while(--yy);
			p += xstep3;
			q += xstep3;
			x -= xstep;
		} while(x);
		p += sstep;
		q += dstep;
		mr->height -= ystep;
	} while(mr->height);
}
#endif


// ----

// サーフェスをバッファとして使う場合…
// dst(posx, posy) <-src:rct

void vramcpy_cpy(VRAMHDL dst, VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}
	vramsub_cpy(dst, src, &mr);
}

void vramcpy_cpypat(VRAMHDL dst, VRAMHDL src, const BYTE *pat8,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_cpyp16(dst, src, pat8, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_cpyp24(dst, src, pat8, &mr);
	}
#endif
}

void vramcpy_cpyex(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		if (!src->alpha) {
			vramsub_cpyex16(dst, src, &mr);
		}
		else {
			vramsub_cpyexa16(dst, src, &mr);
		}
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		if (!src->alpha) {
			vramsub_cpyex24(dst, src, &mr);
		}
		else {
			vramsub_cpyexa24(dst, src, &mr);
		}
	}
#endif
}

void vramcpy_cpyalpha(VRAMHDL dst, const VRAMHDL src, UINT alpha,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}
	if (alpha < 256) {
		alpha = 256 - alpha;
	}
	else {
		alpha = 0;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_cpya16(dst, src, alpha, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_cpya24(dst, src, alpha, &mr);
	}
#endif
}

void vramcpy_mix(VRAMHDL dst, const VRAMHDL src1,
								const VRAMHDL src2, UINT alpha,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src1, pt, rct, &mr) != SUCCESS) {
		return;
	}
	if ((src2 == NULL) || (src1->bpp != src2->bpp) ||
		(src1->posx != src2->posx) || (src1->posy != src2->posy) ||
		(src1->width != src2->width) || (src1->height != src2->height)) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src1->bpp == 16) {
		vramsub_mix16(dst, src1, src2, alpha, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src1->bpp == 24) {
		vramsub_mix24(dst, src1, src2, alpha, &mr);
	}
#endif
}

void vramcpy_mixcol(VRAMHDL dst, const VRAMHDL src,
									UINT32 color, UINT alpha,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_mixcol16(dst, src, color, alpha, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_mixcol24(dst, src, color, alpha, &mr);
	}
#endif
}

void vramcpy_mosaic(VRAMHDL dst, const VRAMHDL src, int dot,
									const POINT_T *pt, const RECT_T *rct) {

	MIX_RECT	mr;

	if (cpyrect(dst, src, pt, rct, &mr) != SUCCESS) {
		return;
	}

	if (dot <= 0) {
		vramsub_cpy(dst, src, &mr);
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_mosaic16(dst, src, dot, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_mosaic24(dst, src, dot, &mr);
	}
#endif
}


// ----

// サーフェスをウィンドウとして使う場合…
// dst:rct <- src(posx, posy)

void vrammix_cpy(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src, rct, &mr) != SUCCESS) {
		return;
	}
	vramsub_cpy(dst, src, &mr);
}

void vrammix_cpyall(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src, rct, &mr) != SUCCESS) {
		return;
	}
	vramsub_cpyall(dst, src, &mr);
}

void vrammix_cpy2(VRAMHDL dst, const VRAMHDL src, BYTE alpha,
													const RECT_T *rct) {

	MIX_RECT	mr;

	if ((mixrect(dst, src, rct, &mr) != SUCCESS) ||
		(dst->alpha == NULL)) {
		return;
	}
	vramsub_cpy2(dst, src, alpha, &mr);
}

void vrammix_cpypat(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
													const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_cpyp16(dst, src, pat8, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_cpyp24(dst, src, pat8, &mr);
	}
#endif
}

void vrammix_cpyex(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		if (!src->alpha) {
			vramsub_cpyex16(dst, src, &mr);
		}
		else {
			vramsub_cpyexa16(dst, src, &mr);
		}
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		if (!src->alpha) {
			vramsub_cpyex24(dst, src, &mr);
		}
		else {
			vramsub_cpyexa24(dst, src, &mr);
		}
	}
#endif
}

void vrammix_mix(VRAMHDL dst, const VRAMHDL src1, const VRAMHDL src2,
										UINT alpha, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src1, rct, &mr) != SUCCESS) {
		return;
	}
	if ((src2 == NULL) || (src1->bpp != src2->bpp) ||
		(src1->posx != src2->posx) || (src1->posy != src2->posy) ||
		(src1->width != src2->width) || (src1->height != src2->height)) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src1->bpp == 16) {
		vramsub_mix16(dst, src1, src2, alpha, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src1->bpp == 24) {
		vramsub_mix24(dst, src1, src2, alpha, &mr);
	}
#endif
}

void vrammix_mixcol(VRAMHDL dst, const VRAMHDL src, UINT32 color,
										UINT alpha, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src, rct, &mr) != SUCCESS) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src->bpp == 16) {
		vramsub_mixcol16(dst, src, color, alpha, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src->bpp == 24) {
		vramsub_mixcol24(dst, src, color, alpha, &mr);
	}
#endif
}

void vrammix_graybmp(VRAMHDL dst, const VRAMHDL src1, const VRAMHDL src2,
										const VRAMHDL bmp, 
										int delta, const RECT_T *rct) {

	MIX_RECT	mr;

	if (mixrect(dst, src1, rct, &mr) != SUCCESS) {
		return;
	}
	if ((src2 == NULL) || (src1->bpp != src2->bpp) ||
		(src1->posx != src2->posx) || (src1->posy != src2->posy) ||
		(src1->width != src2->width) || (src1->height != src2->height)) {
		return;
	}
#ifdef SUPPORT_16BPP
	if (src1->bpp == 16) {
		vramsub_gray16(dst, src1, src2, bmp, delta, &mr);
	}
#endif
#ifdef SUPPORT_24BPP
	if (src1->bpp == 24) {
		vramsub_gray24(dst, src1, src2, bmp, delta, &mr);
	}
#endif
}


// ----

static BOOL txtrect(VRAMHDL dst, const FNTDAT fnt, const POINT_T *pt,
										const RECT_T *rct, MIX_RECT *r) {

	int		pos;

	r->srcpos = 0;
	r->dstpos = pt->y * dst->width;
	r->dstpos += pt->x;

	pos = pt->y - rct->top;
	if (pos < 0) {
		r->srcpos -= pos * fnt->width;
		r->height = min(fnt->height + pos, rct->bottom - rct->top);
	}
	else {
		r->height = min(rct->bottom - rct->top - pos, fnt->height);
	}
	if (r->height <= 0) {
		return(FAILURE);
	}

	pos = pt->x - rct->left;
	if (pos < 0) {
		r->srcpos -= pos;
		r->width = min(fnt->width + pos, rct->right - rct->left);
	}
	else {
		r->width = min(rct->right - rct->left - pos, fnt->width);
	}
	if (r->width <= 0) {
		return(FAILURE);
	}
	return(SUCCESS);
}


#ifdef SUPPORT_16BPP
static void vramsub_txt16(VRAMHDL dst, const FNTDAT fnt,
							UINT32 color, POINT_T *pt, const RECT_T *rct) {

const BYTE		*p;
	BYTE		*q;
	BYTE		*a;
	UINT		alpha;
	int			cnt;
	MIX_RECT	mr;
	UINT		col16;
	int			c16[3];

	if (txtrect(dst, fnt, pt, rct, &mr) != SUCCESS) {
		goto vst16_end;
	}

	col16 = MAKE16PAL(color);
	c16[0] = col16 & B16MASK;
	c16[1] = col16 & G16MASK;
	c16[2] = col16 & R16MASK;

	p = (BYTE *)(fnt + 1);
	p += mr.srcpos;
	q = dst->ptr + (mr.dstpos * 2);
	if (dst->alpha == NULL) {
		do {
			cnt = mr.width;
			do {
				alpha = *p++;
				if (alpha) {
					if (alpha == 255) {
						*(UINT16 *)q = (UINT16)col16;
					}
					else {
						UINT d, s;
						alpha ^= 0xff;			// alpha = 255 - alpha;
						s = *(UINT16 *)q;
						d = MAKEALPHA16s(c16[0], s, B16MASK, alpha, 8);
						d |= MAKEALPHA16s(c16[1], s, G16MASK, alpha, 8);
						d |= MAKEALPHA16s(c16[2], s, R16MASK, alpha, 8);
						*(UINT16 *)q = (UINT16)d;
					}
				}
				q += 2;
			} while(--cnt);
			p += fnt->width - mr.width;
			q += (dst->width - mr.width) * 2;
		} while(--mr.height);
	}
	else {
		a = dst->alpha + mr.dstpos;
		do {
			cnt = mr.width;
			do {
				alpha = *p++;
				if (alpha) {
					*(UINT16 *)q = (UINT16)col16;
					*a = (BYTE)alpha;
				}
				q += 2;
				a += 1;
			} while(--cnt);
			p += fnt->width - mr.width;
			q += (dst->width - mr.width) * 2;
			a += dst->width - mr.width;
		} while(--mr.height);
	}

vst16_end:
	return;
}
#endif

#ifdef SUPPORT_24BPP
static void vramsub_txt24(VRAMHDL dst, const FNTDAT fnt,
							UINT32 color, POINT_T *pt, const RECT_T *rct) {

const BYTE		*p;
	BYTE		*q;
	BYTE		*a;
	UINT		alpha;
	int			cnt;
	MIX_RECT	mr;
	int			c24[3];

	if (txtrect(dst, fnt, pt, rct, &mr) != SUCCESS) {
		goto vst24_end;
	}

	p = (BYTE *)(fnt + 1);
	p += mr.srcpos;
	q = dst->ptr + (mr.dstpos * 3);
	c24[0] = color & 0xff;
	c24[1] = (color >> 8) & 0xff;
	c24[2] = (color >> 16) & 0xff;
	if (dst->alpha == NULL) {
		do {
			cnt = mr.width;
			do {
				alpha = *p++;
				if (alpha) {
					if (alpha == 255) {
						q[0] = (BYTE)c24[0];
						q[1] = (BYTE)c24[1];
						q[2] = (BYTE)c24[2];
					}
					else {
						alpha++;
						q[0] = (BYTE)MAKEALPHA24(q[0], c24[0], alpha, 8);
						q[1] = (BYTE)MAKEALPHA24(q[1], c24[1], alpha, 8);
						q[2] = (BYTE)MAKEALPHA24(q[2], c24[2], alpha, 8);
					}
				}
				q += 3;
			} while(--cnt);
			p += (fnt->width - mr.width);
			q += (dst->width - mr.width) * 3;
		} while(--mr.height);
	}
	else {
		a = dst->alpha + mr.dstpos;
		do {
			cnt = mr.width;
			do {
				alpha = *p++;
				if (alpha) {
					q[0] = (BYTE)c24[0];
					q[1] = (BYTE)c24[1];
					q[2] = (BYTE)c24[2];
					a[0] = (BYTE)alpha;
				}
				q += 3;
				a += 1;
			} while(--cnt);
			p += (fnt->width - mr.width);
			q += (dst->width - mr.width) * 3;
			a += dst->width - mr.width;
		} while(--mr.height);
	}

vst24_end:
	return;
}
#endif


void vrammix_text(VRAMHDL dst, void *fhdl, const char *str,
							UINT32 color, POINT_T *pt, const RECT_T *rct) {

	char	buf[4];
	RECT_T	rect;
	FNTDAT	fnt;

	if ((str == NULL) || (pt == NULL) ||
		(vram_cliprect(&rect, dst, rct) != SUCCESS)) {
		goto vmtxt_end;
	}

	buf[2] = '\0';
	while(*str) {
		buf[0] = *str++;
		if ((((buf[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
			buf[1] = *str++;
			if (buf[1] == '\0') {
				break;
			}
		}
		else if (buf[0]) {
			buf[1] = '\0';
		}
		else {
			break;
		}
		fnt = fontmng_get(fhdl, buf);
		if (fnt) {
#ifdef SUPPORT_16BPP
			if (dst->bpp == 16) {
				vramsub_txt16(dst, fnt, color, pt, &rect);
			}
#endif
#ifdef SUPPORT_24BPP
			if (dst->bpp == 24) {
				vramsub_txt24(dst, fnt, color, pt, &rect);
			}
#endif
			pt->x += fnt->pitch;
		}
	}

vmtxt_end:
	return;
}

