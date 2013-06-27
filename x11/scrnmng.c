/*	$Id: scrnmng.c,v 1.3 2003/08/13 05:01:54 yui Exp $	*/

#include "compiler.h"
#include "gamecore.h"
#include "x11.h"
#include "scrnmng.h"
#include "vramdraw.h"
#include "menubase.h"

static PALETTE_TABLE pal16bit;
static BYTE r16b;
static BYTE l16r;
static BYTE l16g;
RECT_T win_rect;

void
make16mask(UINT bmask, UINT rmask, UINT gmask)
{
	UINT16 bit;
	BYTE cnt;

	pal16bit.d = 0;
	for (bit = 1; bit && ((bmask & bit) == 0); bit <<= 1)
		continue;
	for (r16b = 8; r16b && (bmask & bit); r16b--, bit <<= 1) {
		pal16bit.p.b >>= 1;
		pal16bit.p.b |= 0x80;
	}

	for (l16r = 0, bit = 1; bit && ((rmask & bit) == 0); l16r++, bit <<= 1)
		continue;
	for (cnt = 0x80; cnt && (rmask & bit); cnt >>= 1, bit <<= 1) {
		pal16bit.p.r |= cnt;
	}
	for (; cnt; cnt >>= 1) {
		l16r--;
	}

	for (l16g = 0, bit = 1; bit && ((gmask & bit) == 0); l16g++, bit <<= 1)
		continue;
	for (cnt = 0x80; cnt && (gmask & bit); cnt >>= 1, bit <<= 1) {
		pal16bit.p.g |= cnt;
	}
	for (; cnt; cnt >>= 1) {
		l16g--;
	}
}

static BOOL
norrect(DEST_SURFACE *ds, VRAMHDL s, MIX_RECT *rct)
{

	memset(rct, 0, sizeof(MIX_RECT));

	rct->width = min(ds->width, s->width);
	rct->height = min(ds->height, s->height);

	if (ds->rct) {
		int pos;
		pos = max(ds->rct->left, 0);
		rct->srcpos += pos;
		rct->dstpos += pos * ds->xalign;
		rct->width = min(ds->rct->right, rct->width) - pos;

		pos = max(ds->rct->top, 0);
		rct->srcpos += pos * s->width;
		rct->dstpos += pos * ds->yalign;
		rct->height = min(ds->rct->bottom, rct->height) - pos;
	}

	if ((rct->width <= 0) || (rct->height <= 0))
		return FAILURE;
	return SUCCESS;
}


#if defined(SUPPORT_16BPP)

// ---- 16bpp

static void draw16_nor16(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q;
	int	x;
	BYTE	g, r, b;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = ds->ptr + mr->dstpos;
	g = l16g;
	r = l16r;
	b = r16b;

	do {
		x = mr->width;
		do {
			PALETTE_TABLE pix;
			UINT s;
			s = *(UINT16 *)p;
			pix.p.b = MAKE24B(s);
			pix.p.g = MAKE24G(s);
			pix.p.r = MAKE24R(s);
			pix.d &= pal16bit.d;
			*(UINT16 *)q = (((UINT16)pix.p.g) << g) |
			    (((UINT16)pix.p.r) << r) | (pix.p.b >> b);
			p += 2;
			q += 2;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += ds->yalign - (mr->width * 2);
	} while(--mr->height);
}

static void draw16_nor32(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q;
	int	x;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = ds->ptr + mr->dstpos;
	do {
		x = mr->width;
		do {
			UINT s;
			s = *(UINT16 *)p;
			q[0] = MAKE24B(s);
			q[1] = MAKE24G(s);
			q[2] = MAKE24R(s);
			q[3] = 0;
			p += 2;
			q += 4;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += ds->yalign - (mr->width * 4);
	} while(--mr->height);
}

static void draw16_nor16onmenu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int	x;
	BYTE	g, r, b;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	g = l16g;
	r = l16r;
	b = r16b;

	do {
		x = mr->width;
		do {
			if (a[0] == 0) {
				PALETTE_TABLE pix;
				UINT s;
				s = *(UINT16 *)p;
				pix.p.b = MAKE24B(s);
				pix.p.g = MAKE24G(s);
				pix.p.r = MAKE24R(s);
				pix.d &= pal16bit.d;
				*(UINT16 *)q = (((UINT16)pix.p.g) << g) |
				    (((UINT16)pix.p.r) << r) | (pix.p.b >> b);
			}
			p += 2;
			q += 2;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += ds->yalign - (mr->width * 2);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw16_nor32onmenu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int	x;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] == 0) {
				UINT s;
				s = *(UINT16 *)p;
				q[0] = MAKE24B(s);
				q[1] = MAKE24G(s);
				q[2] = MAKE24R(s);
				q[3] = 0;
			}
			p += 2;
			q += 4;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += ds->yalign - (mr->width * 4);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw16_nor16menu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int	x;
	BYTE	sg, sr, sb;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = menuvram->ptr + (mr->srcpos * 2);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	sg = l16g;
	sr = l16r;
	sb = r16b;

	do {
		x = mr->width;
		do {
			if (a[0]) {
				PALETTE_TABLE pix;
				UINT s;
				if (a[0] & 2) {
					s = *(UINT16 *)q;
				}
				else {
					a[0] = 0;
					s = *(UINT16 *)p;
				}
				pix.p.b = MAKE24B(s);
				pix.p.g = MAKE24G(s);
				pix.p.r = MAKE24R(s);
				pix.d &= pal16bit.d;
				*(UINT16 *)r = (((UINT16)pix.p.g) << sg) |
				    (((UINT16)pix.p.r) << sr) | (pix.p.b >> sb);
			}
			p += 2;
			q += 2;
			r += 2;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += (menuvram->width - mr->width) * 2;
		r += ds->yalign - (mr->width * 2);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw16_nor32menu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int	x;

	p = mainvram->ptr + (mr->srcpos * 2);
	q = menuvram->ptr + (mr->srcpos * 2);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			UINT s;
			if (a[0] & 2) {
				s = *(UINT16 *)q;
				r[0] = MAKE24B(s);
				r[1] = MAKE24G(s);
				r[2] = MAKE24R(s);
				r[3] = 0;
			}
			else if (a[0]) {
				a[0] = 0;
				s = *(UINT16 *)p;
				r[0] = MAKE24B(s);
				r[1] = MAKE24G(s);
				r[2] = MAKE24R(s);
				r[3] = 0;
			}
			p += 2;
			q += 2;
			r += 4;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 2;
		q += (menuvram->width - mr->width) * 2;
		r += ds->yalign - (mr->width * 4);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}
#endif	/* SUPPORT_16BPP */

#if defined(SUPPORT_24BPP)

// ---- 24bpp

static void draw24_nor16(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q;
	int	x;
	BYTE	g, r, b;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = ds->ptr + mr->dstpos;
	g = l16g;
	r = l16r;
	b = r16b;

	do {
		x = mr->width;
		do {
			PALETTE_TABLE	pix;
			pix.p.b = p[0];
			pix.p.g = p[1];
			pix.p.r = p[2];
			pix.d &= pal16bit.d;
			*(UINT16 *)q = (((UINT16)pix.p.g) << g) |
			    (((UINT16)pix.p.r) << r) | (pix.p.b >> b);
			p += 3;
			q += 2;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * 2);
	} while(--mr->height);
}

static void draw24_nor32(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q;
	int	x;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = ds->ptr + mr->dstpos;
	do {
		x = mr->width;
		do {
			q[0] = p[0];
			q[1] = p[1];
			q[2] = p[2];
			q[3] = 0;
			p += 3;
			q += 4;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * 4);
	} while(--mr->height);
}

static void draw24_nor16onmenu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int	x;
	BYTE	g, r, b;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	g = l16g;
	r = l16r;
	b = r16b;

	do {
		x = mr->width;
		do {
			if (a[0] == 0) {
				PALETTE_TABLE	pix;
				pix.p.b = p[0];
				pix.p.g = p[1];
				pix.p.r = p[2];
				pix.d &= pal16bit.d;
				*(UINT16 *)q = (((UINT16)pix.p.g) << g) |
				    (((UINT16)pix.p.r) << r) | (pix.p.b >> b);
			}
			p += 3;
			q += 2;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * 2);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw24_nor32onmenu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *a;
	int		x;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] == 0) {
				q[0] = p[0];
				q[1] = p[1];
				q[2] = p[2];
				q[3] = 0;
			}
			p += 3;
			q += 4;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * 4);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw24_nor16menu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int	x;
	BYTE	sg, sr, sb;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = menuvram->ptr + (mr->srcpos * 3);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	sg = l16g;
	sr = l16r;
	sb = r16b;

	do {
		x = mr->width;
		do {
			if (a[0]) {
				PALETTE_TABLE	pix;
				if (a[0] & 2) {
					pix.p.b = q[0];
					pix.p.g = q[1];
					pix.p.r = q[2];
				}
				else {
					a[0] = 0;
					pix.p.b = p[0];
					pix.p.g = p[1];
					pix.p.r = p[2];
				}
				pix.d &= pal16bit.d;
				*(UINT16 *)r = (((UINT16)pix.p.g) << sg) |
				    (((UINT16)pix.p.r) << sr) | (pix.p.b >> sb);
			}
			p += 3;
			q += 3;
			r += 2;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += (menuvram->width - mr->width) * 3;
		r += ds->yalign - (mr->width * 2);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}

static void draw24_nor32menu(DEST_SURFACE *ds, MIX_RECT *mr) {

	BYTE	*p, *q, *r, *a;
	int	x;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = menuvram->ptr + (mr->srcpos * 3);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] & 2) {
				r[0] = q[0];
				r[1] = q[1];
				r[2] = q[2];
				r[3] = 0;
			}
			else if (a[0]) {
				a[0] = 0;
				r[0] = p[0];
				r[1] = p[1];
				r[2] = p[2];
				r[3] = 0;
			}
			p += 3;
			q += 3;
			r += 4;
			a += 1;
		} while(--x);
		p += (mainvram->width - mr->width) * 3;
		q += (menuvram->width - mr->width) * 3;
		r += ds->yalign - (mr->width * 4);
		a += menuvram->width - mr->width;
	} while(--mr->height);
}
#endif	/* SUPPORT_24BPP */


// ----

void
drawcb_nor(DEST_SURFACE *ds)
{
	MIX_RECT mr;

	if (norrect(ds, mainvram, &mr) != SUCCESS)
		return;

#if defined(SUPPORT_16BPP)
	if (mainvram->bpp == 16) {
		if (menuvram == NULL) {
			if (ds->bit == 16) {
				draw16_nor16(ds, &mr);
			} else if (ds->bit == 32) {
				draw16_nor32(ds, &mr);
			}
		} else {
			if (ds->bit == 16) {
				draw16_nor16onmenu(ds, &mr);
			} else if (ds->bit == 32) {
				draw16_nor32onmenu(ds, &mr);
			}
		}
	}
#endif
#if defined(SUPPORT_24BPP)
	if (mainvram->bpp == 24) {
		if (menuvram == NULL) {
			if (ds->bit == 16) {
				draw24_nor16(ds, &mr);
			} else if (ds->bit == 32) {
				draw24_nor32(ds, &mr);
			}
		} else {
			if (ds->bit == 16) {
				draw24_nor16onmenu(ds, &mr);
			} else if (ds->bit == 32) {
				draw24_nor32onmenu(ds, &mr);
			}
		}
	}
#endif
}

void
scrnmng_draw(const RECT_T *rct)
{

	if (mainvram)
		xdraws_draws(drawcb_nor, rct);
}

// ----

void
drawcb_menu(DEST_SURFACE *ds)
{
	MIX_RECT mr;

	if (norrect(ds, menuvram, &mr) != SUCCESS)
		return;

#if defined(SUPPORT_16BPP)
	if (menuvram->bpp == 16) {
		if (ds->bit == 16) {
			draw16_nor16menu(ds, &mr);
		} else if (ds->bit == 32) {
			draw16_nor32menu(ds, &mr);
		}
	}
#endif
#if defined(SUPPORT_24BPP)
	if (menuvram->bpp == 24) {
		if (ds->bit == 16) {
			draw24_nor16menu(ds, &mr);
		} else if (ds->bit == 32) {
			draw24_nor32menu(ds, &mr);
		}
	}
#endif
}

void
scrnmng_menudraw(const RECT_T *rct)
{

	if (mainvram && menuvram)
		xdraws_draws(drawcb_menu, rct);
}
