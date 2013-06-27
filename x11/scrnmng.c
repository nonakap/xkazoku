/*	$Id: scrnmng.c,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#include "compiler.h"
#include "gamecore.h"
#include "x11.h"
#include "scrnmng.h"
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

	bzero(rct, sizeof(MIX_RECT));

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

void
drawcb_nor16(DEST_SURFACE *ds, MIX_RECT *mr)
{
	PALETTE_TABLE pix;
	BYTE *p, *q;
	int x;

	p = mainvram->ptr + mr->srcpos * 3;
	q = ds->ptr + mr->dstpos;
	do {
		x = mr->width;
		do {
			pix.p.b = p[0];
			pix.p.g = p[1];
			pix.p.r = p[2];
			pix.d &= pal16bit.d;
			*(UINT16 *)q = (((UINT16)pix.p.g) << l16g) |
			    (((UINT16)pix.p.r) << l16r) | (pix.p.b >> r16b);
			p += 3;
			q += ds->xalign;
		} while (--x > 0);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * ds->xalign);
	} while (--mr->height);
}

void
drawcb_nor16onmenu(DEST_SURFACE *ds, MIX_RECT *mr)
{
	PALETTE_TABLE pix;
	BYTE *p, *q, *a;
	int x;

	p = mainvram->ptr + mr->srcpos * 3;
	q = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] == 0) {
				pix.p.b = p[0];
				pix.p.g = p[1];
				pix.p.r = p[2];
				pix.d &= pal16bit.d;
				*(UINT16 *)q = (((UINT16)pix.p.g) << l16g) |
				             (((UINT16)pix.p.r) << l16r) |
					     (pix.p.b >> r16b);
			}
			p += 3;
			q += ds->xalign;
			a += 1;
		} while (--x > 0);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * ds->xalign);
		a += menuvram->width - mr->width;
	} while (--mr->height);
}

void
drawcb_nor32(DEST_SURFACE *ds, MIX_RECT *mr)
{
	BYTE *p, *q;
	int x;

	p = mainvram->ptr + mr->srcpos * 3;
	q = ds->ptr + mr->dstpos;
	do {
		x = mr->width;
		do {
			q[0] = p[0];
			q[1] = p[1];
			q[2] = p[2];
			q[3] = 0;
			p += 3;
			q += ds->xalign;
		} while (--x > 0);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * ds->xalign);
	} while (--mr->height);
}

void
drawcb_nor32onmenu(DEST_SURFACE *ds, MIX_RECT *mr)
{
	BYTE *p, *q, *a;
	int x;

	p = mainvram->ptr + mr->srcpos * 3;
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
			q += ds->xalign;
			a += 1;
		} while (--x > 0);
		p += (mainvram->width - mr->width) * 3;
		q += ds->yalign - (mr->width * ds->xalign);
		a += menuvram->width - mr->width;
	} while (--mr->height);
}

void
drawcb_nor(DEST_SURFACE *ds)
{
	MIX_RECT mr;

	if (ds->bit == 16) {
		if (norrect(ds, mainvram, &mr) != SUCCESS)
			return;

		if (menuvram == NULL)
			drawcb_nor16(ds, &mr);
		else
			drawcb_nor16onmenu(ds, &mr);
	} else if (ds->bit == 32) {
		if (norrect(ds, mainvram, &mr) != SUCCESS)
			return;

		if (menuvram == NULL)
			drawcb_nor32(ds, &mr);
		else
			drawcb_nor32onmenu(ds, &mr);
	} else {
		fprintf(stderr, "%d bit isn't support\n", ds->bit);
		__ASSERT(0);
	}
}

void
scrnmng_draw(const RECT_T *rct)
{

	if (mainvram)
		xdraws_draws(drawcb_nor, rct);
}

/* ----- */

void
drawcb_nor16menu(DEST_SURFACE *ds, MIX_RECT *mr)
{
	PALETTE_TABLE pix;
	BYTE *p, *q, *r, *a;
	int x;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = menuvram->ptr + (mr->srcpos * 3);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] & 2) {
				pix.p.b = q[0];
				pix.p.g = q[1];
				pix.p.r = q[2];
				pix.d &= pal16bit.d;
				*(UINT16 *)r = (((UINT16)pix.p.g) << l16g) |
				             (((UINT16)pix.p.r) << l16r) |
					     (pix.p.b >> r16b);
			}
			else if (a[0]) {
				a[0] = 0;
				pix.p.b = p[0];
				pix.p.g = p[1];
				pix.p.r = p[2];
				pix.d &= pal16bit.d;
				*(UINT16 *)r = (((UINT16)pix.p.g) << l16g) |
				             (((UINT16)pix.p.r) << l16r) |
					     (pix.p.b >> r16b);
			}
			p += 3;
			q += 3;
			r += ds->xalign;
			a += 1;
		} while (--x);
		p += (mainvram->width - mr->width) * 3;
		q += (menuvram->width - mr->width) * 3;
		r += ds->yalign - (mr->width * ds->xalign);
		a += menuvram->width - mr->width;
	} while (--mr->height);
}

void
drawcb_nor32menu(DEST_SURFACE *ds, MIX_RECT *mr)
{
	BYTE *p, *q, *r, *a;
	int x;

	p = mainvram->ptr + (mr->srcpos * 3);
	q = menuvram->ptr + (mr->srcpos * 3);
	r = ds->ptr + mr->dstpos;
	a = menuvram->alpha + mr->srcpos;
	do {
		x = mr->width;
		do {
			if (a[0] & 2) {
				r[0] = 0;
				r[1] = q[2];
				r[2] = q[1];
				r[3] = q[0];
			}
			else if (a[0]) {
				a[0] = 0;
				r[0] = 0;
				r[1] = p[2];
				r[2] = p[1];
				r[3] = p[0];
			}
			p += 3;
			q += 3;
			r += ds->xalign;
			a += 1;
		} while (--x);
		p += (mainvram->width - mr->width) * 3;
		q += (menuvram->width - mr->width) * 3;
		r += ds->yalign - (mr->width * ds->xalign);
		a += menuvram->width - mr->width;
	} while (--mr->height);
}

void
drawcb_menu(DEST_SURFACE *ds)
{
	MIX_RECT mr;

	if (norrect(ds, menuvram, &mr) != SUCCESS)
		return;

	if (ds->bit == 16) {
		drawcb_nor16menu(ds, &mr);
	} else if (ds->bit == 32) {
		drawcb_nor32menu(ds, &mr);
	} else {
		fprintf(stderr, "%d bit isn't support\n", ds->bit);
		__ASSERT(0);
	}
}

void
scrnmng_menudraw(const RECT_T *rct)
{

	if (mainvram && menuvram)
		xdraws_draws(drawcb_menu, rct);
}
