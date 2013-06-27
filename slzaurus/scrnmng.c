#include	"compiler.h"
#include	"vram.h"
#include	"vramdraw.h"
#include	"xdraws.h"
#include	"scrnmng.h"
#include	"menubase.h"


static BOOL norrect(DEST_SURFACE *ds, VRAMHDL s, MIX_RECT *rct) {

	ZeroMemory(rct, sizeof(MIX_RECT));

	rct->width = min(ds->width, s->width);
	rct->height = min(ds->height, s->height);
	if (ds->arg) {
		int pos;
		pos = max(((RECT_T *)ds->arg)->left, 0);
		rct->srcpos += pos;
		rct->dstpos += pos * ds->xalign;
		rct->width = min(((RECT_T *)ds->arg)->right, rct->width) - pos;

		pos = max(((RECT_T *)ds->arg)->top, 0);
		rct->srcpos += pos * s->width;
		rct->dstpos += pos * ds->yalign;
		rct->height = min(((RECT_T *)ds->arg)->bottom, rct->height) - pos;
	}
	if ((rct->width <= 0) || (rct->height <= 0)) {
		return(FAILURE);
	}
	return(SUCCESS);
}


// ----

void drawcb_nor565(DEST_SURFACE *ds) {

	BYTE		*p, *q;
	int			x;
	MIX_RECT	mr;
	int			salign;
	int			dalign;

	if (norrect(ds, mainvram, &mr) != SUCCESS) {
		return;
	}
	p = mainvram->ptr + (mr.srcpos * 2);
	q = ds->ptr + mr.dstpos;

	if (ds->xalign == 2) {
		do {
			CopyMemory(q, p, mr.width * 2);
			p += mainvram->width * 2;
			q += ds->yalign;
		} while(--mr.height);
	}
	else {
		salign = (mainvram->width - mr.width) * 2;
		dalign = ds->yalign - (mr.width * ds->xalign);
		do {
			x = mr.width;
			do {
				*(UINT16 *)q = *(UINT16 *)p;
				p += 2;
				q += ds->xalign;
			} while(--x);
			p += salign;
			q += dalign;
		} while(--mr.height);
	}
}

void drawcb_nor565_onmenu(DEST_SURFACE *ds) {

	BYTE		*p, *q, *a;
	int			x;
	MIX_RECT	mr;
	int			salign;
	int			dalign;

	if (norrect(ds, mainvram, &mr) != SUCCESS) {
		return;
	}

	p = mainvram->ptr + (mr.srcpos * 2);
	q = ds->ptr + mr.dstpos;
	a = menuvram->alpha + mr.srcpos;
	salign = mainvram->width - mr.width;
	dalign = ds->yalign - (mr.width * ds->xalign);
	do {
		x = mr.width;
		do {
			if (a[0] == 0) {
				*(UINT16 *)q = *(UINT16 *)p;
			}
			p += 2;
			q += ds->xalign;
			a += 1;
		} while(--x);
		p += salign * 2;
		q += dalign;
		a += salign;
	} while(--mr.height);
}

void scrnmng_draw(const RECT_T *rct) {

	if (mainvram) {
		if (menuvram == NULL) {
			xdraws_draws(drawcb_nor565, rct);
		}
		else {
			xdraws_draws(drawcb_nor565_onmenu, rct);
		}
	}
}


// ----

void drawcb_nor16menu(DEST_SURFACE *ds) {

	BYTE		*p, *q, *r, *a;
	int			x;
	MIX_RECT	mr;
	int			salign;
	int			malign;
	int			dalign;

	if (norrect(ds, menuvram, &mr) != SUCCESS) {
		return;
	}

	p = mainvram->ptr + (mr.srcpos * 2);
	q = menuvram->ptr + (mr.srcpos * 2);
	r = ds->ptr + mr.dstpos;
	a = menuvram->alpha + mr.srcpos;
	salign = (mainvram->width - mr.width) * 2;
	malign = menuvram->width - mr.width;
	dalign = ds->yalign - (mr.width * ds->xalign);
	do {
		x = mr.width;
		do {
			if (a[0]) {
				if (a[0] & 2) {
					*(UINT16 *)r = *(UINT16 *)q;
				}
				else {
					a[0] = 0;
					*(UINT16 *)r = *(UINT16 *)p;
				}
			}
			p += 2;
			q += 2;
			r += ds->xalign;
			a += 1;
		} while(--x);
		p += salign;
		q += malign * 2;
		r += dalign;
		a += malign;
	} while(--mr.height);
}

void scrnmng_menudraw(const RECT_T *rct) {

	if ((mainvram) && (menuvram)) {
		xdraws_draws(drawcb_nor16menu, rct);
	}
}

