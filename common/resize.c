#include	"compiler.h"
#include	"resize.h"


#define ENABLE_FASTCOPY

//#define RESIZE_SIMPLE
//#define RESIZE_NEAREST
//#define RESIZE_BILINEAR
//#define RESIZE_BICUBIC
#define RESIZE_AREAAVG


enum {
	RESIZE_S24BIT	= 0x01,
	RESIZE_D24BIT	= 0x02,
	RESIZE_HDIF		= 0x04,
	RESIZE_VDIF		= 0x08,
	RESIZE_ERROR	= 0x80
};

enum {
	MXBITS		= 8,
	MYBITS		= 8,
	MULTIX		= (1 << MXBITS),
	MULTIY		= (1 << MYBITS),
	MXMASK		= (MULTIX - 1),
	MYMASK		= (MULTIY - 1)
};

enum {
	B16BIT		= 5,
	G16BIT		= 6,
	R16BIT		= 5,
	B16SFT		= 0,
	G16SFT		= 5,
	R16SFT		= 11
};

#define	GET16B(c)	(((c) >> B16SFT) & ((1 << B16BIT) - 1))
#define	GET16G(c)	(((c) >> G16SFT) & ((1 << G16BIT) - 1))
#define	GET16R(c)	(((c) >> R16SFT) & ((1 << R16BIT) - 1))


typedef struct {
	int		dstwidth;
	int		srcwidth;
	UINT	*h;
	UINT	*buf;
	UINT	bufsize;
} _RESHDL, *RESHDL;


static UINT sizecheck(const RSDIMG *dst, const RSSIMG *src) {

	UINT	ret;

	ret = 0;
	if ((dst == NULL) || (src == NULL)) {
		goto sc_err;
	}
	if ((dst->width <= 0) || (src->width <= 0)) {
		goto sc_err;
	}
	if (dst->width != src->width) {
		ret |= RESIZE_HDIF;
	}
	if ((dst->height <= 0) || (src->height <= 0)) {
		goto sc_err;
	}
	if (dst->height != src->height) {
		ret |= RESIZE_VDIF;
	}
	if (src->bpp == 24) {
		ret |= RESIZE_S24BIT;
	}
	else if (src->bpp != 16) {
		goto sc_err;
	}
	if (dst->bpp == 24) {
		ret |= RESIZE_D24BIT;
	}
	else if (dst->bpp != 16) {
		goto sc_err;
	}
	return(ret);

sc_err:
	return(RESIZE_ERROR);
}


// ---- convert sub

typedef void (*FNCNV)(RESHDL hdl, BYTE *dst, const BYTE *src);

#if defined(ENABLE_FASTCOPY) || defined(RESIZE_BILINEAR)
static void cc16by24(RESHDL hdl, BYTE *dst, const BYTE *src) {

	UINT	width;
	UINT	col;

	width = hdl->dstwidth;
	do {
		col = (src[0] >> (8 - B16BIT)) << B16SFT;
		col += (src[1] >> (8 - G16BIT)) << G16SFT;
		col += (src[2] >> (8 - R16BIT)) << R16SFT;
		*(UINT16 *)dst = (UINT16)col;
		src += 3;
		dst += 2;
	} while(--width);
}

static void cc24by16(RESHDL hdl, BYTE *dst, const BYTE *src) {

	UINT	width;
	UINT	col;
	UINT	tmp;

	width = hdl->dstwidth;
	do {
		col = *(UINT16 *)src;
		tmp = (col >> B16SFT) & ((1 << B16BIT) - 1);
		dst[0] = (BYTE)((tmp << (8 - B16BIT)) + (tmp >> (B16BIT * 2 - 8)));
		tmp = (col >> G16SFT) & ((1 << G16BIT) - 1);
		dst[1] = (BYTE)((tmp << (8 - G16BIT)) + (tmp >> (G16BIT * 2 - 8)));
		tmp = (col >> R16SFT) & ((1 << R16BIT) - 1);
		dst[2] = (BYTE)((tmp << (8 - R16BIT)) + (tmp >> (R16BIT * 2 - 8)));
		src += 2;
		dst += 3;
	} while(--width);
}
#endif

#if defined(ENABLE_FASTCOPY)
static void cc16(RESHDL hdl, BYTE *dst, const BYTE *src) {

	CopyMemory(dst, src, hdl->dstwidth * 2);
}

static void cc24(RESHDL hdl, BYTE *dst, const BYTE *src) {

	CopyMemory(dst, src, hdl->dstwidth * 3);
}

static const FNCNV cnvcpy[4] = {cc16, cc16by24, cc24by16, cc24};
#endif

#if defined(RESIZE_SIMPLE) || defined(RESIZE_NEAREST)
static void cm16(RESHDL hdl, BYTE *dst, const BYTE *src) {

	int		i;

	for (i=0; i<hdl->dstwidth; i++) {
		((UINT16 *)dst)[i] = *(UINT16 *)(src + (hdl->h[i] * 2));
	}
}

static void cm24(RESHDL hdl, BYTE *dst, const BYTE *src) {

	int		i;
const BYTE	*p;

	for (i=0; i<hdl->dstwidth; i++) {
		p = src + (hdl->h[i] * 3);
		dst[0] = p[0];
		dst[1] = p[1];
		dst[2] = p[2];
		dst += 3;
	}
}

static void cm16by24(RESHDL hdl, BYTE *dst, const BYTE *src) {

	int		i;
const BYTE	*p;
	UINT	col;

	for (i=0; i<hdl->dstwidth; i++) {
		p = src + (hdl->h[i] * 3);
		col = (p[0] >> (8 - B16BIT)) << B16SFT;
		col += (p[1] >> (8 - G16BIT)) << G16SFT;
		col += (p[2] >> (8 - R16BIT)) << R16SFT;
		((UINT16 *)dst)[i] = (UINT16)col;
	}
}

static void cm24by16(RESHDL hdl, BYTE *dst, const BYTE *src) {

	int		i;
	UINT	col;
	UINT	tmp;

	for (i=0; i<hdl->dstwidth; i++) {
		col = *(UINT16 *)(src + (hdl->h[i] * 2));
		tmp = (col >> B16SFT) & ((1 << B16BIT) - 1);
		dst[0] = (BYTE)((tmp << (8 - B16BIT)) + (tmp >> (B16BIT * 2 - 8)));
		tmp = (col >> G16SFT) & ((1 << G16BIT) - 1);
		dst[1] = (BYTE)((tmp << (8 - G16BIT)) + (tmp >> (G16BIT * 2 - 8)));
		tmp = (col >> R16SFT) & ((1 << R16BIT) - 1);
		dst[2] = (BYTE)((tmp << (8 - R16BIT)) + (tmp >> (R16BIT * 2 - 8)));
		dst += 3;
	}
}

static const FNCNV cnvmix[4] = {cm16, cm16by24, cm24by16, cm24};
#endif


// ---- copy

#if defined(ENABLE_FASTCOPY)
static void fastcopy(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	FNCNV	cnv;
const BYTE	*p;
	BYTE	*q;
	UINT	height;
	UINT	width;

	p = src->ptr;
	q = dst->ptr;
	height = dst->height;
	width = dst->width;
	cnv = cnvcpy[type & 3];
	do {
		cnv((RESHDL)&width, q, p);
		p += src->align;
		q += dst->align;
	} while(--height);
}
#endif


// ---- simple

#if defined(RESIZE_SIMPLE)
static void simpleresize(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	RESHDL	reshdl;
	UINT	i;
	UINT	*ptr;
	UINT	ssize;
	UINT	dsize;
	FNCNV	cnv;
const BYTE	*p;
	BYTE	*q;

#if defined(ENABLE_FASTCOPY)
	if (type & RESIZE_HDIF) {
#endif
		reshdl = (RESHDL)_MALLOC(sizeof(_RESHDL) +
									(dst->width * sizeof(UINT)), "reshdl");
		if (reshdl == NULL) {
			goto sr_exit;
		}
		ptr = (UINT *)(reshdl + 1);
		reshdl->srcwidth = (UINT)src->width;
		reshdl->dstwidth = (UINT)dst->width;

		reshdl->h = ptr;
		ssize = src->width - 1;
		dsize = dst->width - 1;
		for (i=0; i<dsize; i++) {
			*ptr++ = (i * ssize) / dsize;
		}
		*ptr = ssize;
		cnv = cnvmix[type & 3];
#if defined(ENABLE_FASTCOPY)
	}
	else {
		reshdl = (RESHDL)&dst->width;
		cnv = cnvcpy[type & 3];
	}
#endif

	p = src->ptr;
	q = dst->ptr;
	if (type & RESIZE_VDIF) {
		ssize = src->height - 1;
		dsize = dst->height - 1;
		for (i=0; i<=dsize; i++) {
			cnv(reshdl, q, p + (((i * ssize) / dsize) * src->align));
			q += dst->align;
		}
	}
	else {
		dsize = dst->height;
		do {
			cnv(reshdl, q, p);
			p += src->align;
			q += dst->align;
		} while(--dsize);
	}

#if defined(ENABLE_FASTCOPY)
	if (type & RESIZE_HDIF) {
#endif
		_MFREE(reshdl);
#if defined(ENABLE_FASTCOPY)
	}
#endif

sr_exit:
	return;
}
#endif


// ---- nearest neighbor

#if defined(RESIZE_NEAREST)
static void nearestneighbor(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	RESHDL	reshdl;
	UINT	i;
	UINT	*ptr;
	UINT	ssize;
	UINT	dsize;
	FNCNV	cnv;
const BYTE	*p;
	BYTE	*q;

#if defined(ENABLE_FASTCOPY)
	if (type & RESIZE_HDIF) {
#endif
		reshdl = (RESHDL)_MALLOC(sizeof(_RESHDL) +
									(dst->width * sizeof(UINT)), "reshdl");
		if (reshdl == NULL) {
			goto nn_exit;
		}
		ptr = (UINT *)(reshdl + 1);
		reshdl->srcwidth = src->width;
		reshdl->dstwidth = dst->width;

		reshdl->h = ptr;
		ssize = src->width - 1;
		dsize = dst->width - 1;
		for (i=0; i<dsize; i++) {
			*ptr++ = ((i * ssize) + (dsize / 2)) / dsize;
		}
		*ptr = ssize;
		cnv = cnvmix[type & 3];
#if defined(ENABLE_FASTCOPY)
	}
	else {
		reshdl = (RESHDL)&dst->width;
		cnv = cnvcpy[type & 3];
	}
#endif

	p = src->ptr;
	q = dst->ptr;
	if (type & RESIZE_VDIF) {
		ssize = src->height - 1;
		dsize = dst->height - 1;
		for (i=0; i<=dsize; i++) {
			cnv(reshdl, q, p +
						((((i * ssize) + (dsize / 2)) / dsize) * src->align));
			q += dst->align;
		}
	}
	else {
		dsize = dst->height;
		do {
			cnv(reshdl, q, p);
			p += src->align;
			q += dst->align;
		} while(--dsize);
	}

#if defined(ENABLE_FASTCOPY)
	if (type & RESIZE_HDIF) {
#endif
	_MFREE(reshdl);
#if defined(ENABLE_FASTCOPY)
	}
#endif

nn_exit:
	return;
}
#endif


// ---- bilinear

#if defined(RESIZE_BILINEAR)
static void bl16(RESHDL hdl, BYTE *dst, const BYTE *src1,
										const BYTE *src2, int volume) {

	int		i;
	UINT	off;
	UINT	frac;
	int		p00, p10, p01, p11;
	UINT	tmp;

	for (i=0; i<(hdl->dstwidth - 1); i++) {
		off = (hdl->h[i] >> MXBITS) * 2;
		frac = hdl->h[i] & MXMASK;
		p00 = GET16B(*(UINT16 *)(src1 + off + 0));
		p10 = GET16B(*(UINT16 *)(src1 + off + 2));
		p01 = GET16B(*(UINT16 *)(src2 + off + 0));
		p11 = GET16B(*(UINT16 *)(src2 + off + 2));
		tmp = (p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
												(MXBITS + MYBITS))) << B16SFT;
		p00 = GET16G(*(UINT16 *)(src1 + off + 0));
		p10 = GET16G(*(UINT16 *)(src1 + off + 2));
		p01 = GET16G(*(UINT16 *)(src2 + off + 0));
		p11 = GET16G(*(UINT16 *)(src2 + off + 2));
		tmp += (p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
												(MXBITS + MYBITS))) << G16SFT;
		p00 = GET16R(*(UINT16 *)(src1 + off + 0));
		p10 = GET16R(*(UINT16 *)(src1 + off + 2));
		p01 = GET16R(*(UINT16 *)(src2 + off + 0));
		p11 = GET16R(*(UINT16 *)(src2 + off + 2));
		tmp += (p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
												(MXBITS + MYBITS))) << R16SFT;
		*(UINT16 *)dst = (UINT16)tmp;
		dst += 2;
	}
	off = (hdl->h[i] >> MXBITS) * 2;
	p00 = GET16B(*(UINT16 *)(src1 + off));
	p01 = GET16B(*(UINT16 *)(src2 + off));
	tmp = (p00 + (((p01 - p00) * volume) >> MYBITS)) << B16SFT;
	p00 = GET16G(*(UINT16 *)(src1 + off));
	p01 = GET16G(*(UINT16 *)(src2 + off));
	tmp += (p00 + (((p01 - p00) * volume) >> MYBITS)) << G16SFT;
	p00 = GET16R(*(UINT16 *)(src1 + off));
	p01 = GET16R(*(UINT16 *)(src2 + off));
	tmp += (p00 + (((p01 - p00) * volume) >> MYBITS)) << R16SFT;
	*(UINT16 *)dst = (UINT16)tmp;
}

static void bl24(RESHDL hdl, BYTE *dst, const BYTE *src1,
										const BYTE *src2, int volume) {

	int		i;
	UINT	off;
	UINT	frac;
	int		p00, p10, p01, p11;

	for (i=0; i<(hdl->dstwidth - 1); i++) {
		off = (hdl->h[i] >> MXBITS) * 3;
		frac = hdl->h[i] & MXMASK;
		p00 = src1[off+0];
		p10 = src1[off+3];
		p01 = src2[off+0];
		p11 = src2[off+3];
		dst[0] = (BYTE)(p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
														(MXBITS + MYBITS)));
		p00 = src1[off+1];
		p10 = src1[off+4];
		p01 = src2[off+1];
		p11 = src2[off+4];
		dst[1] = (BYTE)(p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
														(MXBITS + MYBITS)));
		p00 = src1[off+2];
		p10 = src1[off+5];
		p01 = src2[off+2];
		p11 = src2[off+5];
		dst[2] = (BYTE)(p00 + (((((p01 - p00) * volume) << MXBITS) +
							((((p10 - p00) << MYBITS) +
							((p11 - p10 - p01 + p00) * volume)) * frac)) >>
														(MXBITS + MYBITS)));
		dst += 3;
	}
	off = (hdl->h[i] >> MXBITS) * 3;
	p00 = src1[off+0];
	p01 = src2[off+0];
	dst[0] = (BYTE)(p00 + (((p01 - p00) * volume) >> MYBITS));
	p00 = src1[off+1];
	p01 = src2[off+1];
	dst[1] = (BYTE)(p00 + (((p01 - p00) * volume) >> MYBITS));
	p00 = src1[off+2];
	p01 = src2[off+2];
	dst[2] = (BYTE)(p00 + (((p01 - p00) * volume) >> MYBITS));
}

typedef void (*FNBL)(RESHDL hdl, BYTE *dst, const BYTE *src1,
										const BYTE *src2, int volume);

static void bilinear(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	UINT	worksize;
	FNBL	bl;
	FNCNV	cnv;
	RESHDL	reshdl;
	UINT	i;
	UINT	*ptr;
	UINT	ssize;
	UINT	dsize;
	UINT	cur;
const BYTE	*p;
	BYTE	*q;

	worksize = dst->width * sizeof(UINT);
	switch(type & (RESIZE_S24BIT | RESIZE_D24BIT)) {
		case RESIZE_S24BIT:
			worksize += dst->width * 3;
			bl = bl24;
			cnv = cc16by24;
			break;

		case RESIZE_D24BIT:
			worksize += dst->width * 2;
			bl = bl16;
			cnv = cc24by16;
			break;

		default:
			bl = (type & RESIZE_S24BIT)?bl24:bl16;
			cnv = NULL;
			break;
	}

	reshdl = (RESHDL)_MALLOC(sizeof(_RESHDL) + worksize, "reshdl");
	if (reshdl == NULL) {
		goto bl_exit;
	}
	ptr = (UINT *)(reshdl + 1);
	reshdl->srcwidth = src->width;
	reshdl->dstwidth = dst->width;

	reshdl->h = ptr;
	ssize = src->width - 1;
	dsize = dst->width - 1;
	for (i=0; i<dsize; i++) {
		*ptr++ = ((i << MXBITS) * ssize) / dsize;
	}
	*ptr++ = ssize << MXBITS;

	q = dst->ptr;
	ssize = src->height - 1;
	dsize = dst->height - 1;

	if (cnv == NULL) {
		for (i=0; i<dsize; i++) {
			cur = ((i << MYBITS) * ssize) / dsize;
			p = src->ptr + ((cur >> MYBITS) * src->align);
			bl(reshdl, q, p, p + src->align, cur & MYMASK);
			q += dst->align;
		}
		p = src->ptr + (ssize * src->align);
		bl(reshdl, q, p, p, 0);
	}
	else {
		for (i=0; i<dsize; i++) {
			cur = ((i << MYBITS) * ssize) / dsize;
			p = src->ptr + ((cur >> MYBITS) * src->align);
			bl(reshdl, (BYTE *)ptr, p, p + src->align, cur & MYMASK);
			cnv(reshdl, q, (BYTE *)ptr);
			q += dst->align;
		}
		p = src->ptr + (ssize * src->align);
		bl(reshdl, (BYTE *)ptr, p, p, 0);
		cnv(reshdl, q, (BYTE *)ptr);
	}
	_MFREE(reshdl);

bl_exit:
	return;
}
#endif


// ---- bicubic

#if defined(RESIZE_BICUBIC)

enum {
	CUBICTABLE	= 8,
	CUBICLEVEL	= 16
};

static int *cubictbl = NULL;

static void cubic_init(void) {

	int		*ptr;
	int		i;
	float	v;

	if (cubictbl == NULL) {
		ptr = (int *)_MALLOC(sizeof(int) << (CUBICTABLE + 2), "cubic");
		if (ptr) {
			cubictbl = ptr;
			for (i=0; i<(1 << CUBICTABLE); i++) {
				v = (float)((1 << CUBICTABLE) + i) / (float)(1 << CUBICTABLE);
				v = 4.0f - (8.0f * v) + (5.0f * v * v) - (v * v * v);
				ptr[0] = (int)(v * (float)(1 << CUBICLEVEL));
				v = (float)((0 << CUBICTABLE) + i) / (float)(1 << CUBICTABLE);
				v = 1.0f - (2.0f * v * v) + (v * v * v);
				ptr[1] = (int)(v * (float)(1 << CUBICLEVEL));
				v = (float)((1 << CUBICTABLE) - i) / (float)(1 << CUBICTABLE);
				v = 1.0f - (2.0f * v * v) + (v * v * v);
				ptr[2] = (int)(v * (float)(1 << CUBICLEVEL));
				v = (float)((2 << CUBICTABLE) - i) / (float)(1 << CUBICTABLE);
				v = 4.0f - (8.0f * v) + (5.0f * v * v) - (v * v * v);
				ptr[3] = (int)(v * (float)(1 << CUBICLEVEL));
				ptr += 4;
			}
		}
	}
}

static void cubic_term(void) {

	int		*ptr;

	ptr = cubictbl;
	if (ptr) {
		cubictbl = NULL;
		_MFREE(ptr);
	}
}

static void bci16(RESHDL hdl, BYTE *dst, const BYTE *src) {

const BYTE	*srclimit;
	int		i;
	int		off;
const int	*tbl;
const BYTE	*p;
	UINT	col0;
	UINT	col1;
	UINT	col2;
	UINT	col3;
	int		col;

	srclimit = src + ((hdl->srcwidth - 1) * 2);
	for (i=0; i<hdl->dstwidth; i++) {
		off = hdl->h[i] >> MXBITS;
		tbl = cubictbl + ((hdl->h[i] & MXMASK) * 4);
		p = src + (off * 2);
		col1 = *(UINT16 *)(p + 0);
		col0 = (off != 0)?*(UINT16 *)(p - 2):col1;
		col2 = ((p + 2) <= srclimit)?*(UINT16 *)(p + 2):col1;
		col3 = ((p + 4) <= srclimit)?*(UINT16 *)(p + 4):col2;
		col = (int)GET16B(col0) * tbl[0];
		col += (int)GET16B(col1) * tbl[1];
		col += (int)GET16B(col2) * tbl[2];
		col += (int)GET16B(col3) * tbl[3];
		col >>= CUBICLEVEL - (8 - B16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[0] = (BYTE)col;
		col = (int)GET16G(col0) * tbl[0];
		col += (int)GET16G(col1) * tbl[1];
		col += (int)GET16G(col2) * tbl[2];
		col += (int)GET16G(col3) * tbl[3];
		col >>= CUBICLEVEL - (8 - G16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[1] = (BYTE)col;
		col = (int)GET16R(col0) * tbl[0];
		col += (int)GET16R(col1) * tbl[1];
		col += (int)GET16R(col2) * tbl[2];
		col += (int)GET16R(col3) * tbl[3];
		col >>= CUBICLEVEL - (8 - R16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[2] = (BYTE)col;
		dst += 3;
	}
}

static void bci24(RESHDL hdl, BYTE *dst, const BYTE *src) {

const BYTE	*srclimit;
	int		i;
	int		off;
const int	*tbl;
const BYTE	*p0;
const BYTE	*p1;
const BYTE	*p2;
const BYTE	*p3;
	int		col;

	srclimit = src + ((hdl->srcwidth - 1) * 3);
	for (i=0; i<hdl->dstwidth; i++) {
		off = hdl->h[i] >> MXBITS;
		tbl = cubictbl + ((hdl->h[i] & MXMASK) * 4);
		p1 = src + (off * 3);
		p0 = max(p1 - 3, src);
		p2 = min(p1 + 3, srclimit);
		p3 = min(p1 + 6, srclimit);
		col = (int)p0[0] * tbl[0];
		col += (int)p1[0] * tbl[1];
		col += (int)p2[0] * tbl[2];
		col += (int)p3[0] * tbl[3];
		col >>= CUBICLEVEL;
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[0] = (BYTE)col;
		col = (int)p0[1] * tbl[0];
		col += (int)p1[1] * tbl[1];
		col += (int)p2[1] * tbl[2];
		col += (int)p3[1] * tbl[3];
		col >>= CUBICLEVEL;
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[1] = (BYTE)col;
		col = (int)p0[2] * tbl[0];
		col += (int)p1[2] * tbl[1];
		col += (int)p2[2] * tbl[2];
		col += (int)p3[2] * tbl[3];
		col >>= CUBICLEVEL;
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[2] = (BYTE)col;
		dst += 3;
	}
}

static void bco16(BYTE *dst, int width, const BYTE **src, const int *tbl) {

	int		spos;
	int		i;
	int		col;
	UINT	tmp;

	spos = 0;
	for (i=0; i<width; i++) {
		col = (int)src[0][spos + 0] * tbl[0];
		col += (int)src[1][spos + 0] * tbl[1];
		col += (int)src[2][spos + 0] * tbl[2];
		col += (int)src[3][spos + 0] * tbl[3];
		col >>= CUBICLEVEL + (8 - B16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << B16BIT)) {
			col = (1 << B16BIT) - 1;
		}
		tmp = col << B16SFT;
		col = (int)src[0][spos + 1] * tbl[0];
		col += (int)src[1][spos + 1] * tbl[1];
		col += (int)src[2][spos + 1] * tbl[2];
		col += (int)src[3][spos + 1] * tbl[3];
		col >>= CUBICLEVEL + (8 - G16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << G16BIT)) {
			col = (1 << G16BIT) - 1;
		}
		tmp += col << G16SFT;
		col = (int)src[0][spos + 2] * tbl[0];
		col += (int)src[1][spos + 2] * tbl[1];
		col += (int)src[2][spos + 2] * tbl[2];
		col += (int)src[3][spos + 2] * tbl[3];
		col >>= CUBICLEVEL + (8 - R16BIT);
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << R16BIT)) {
			col = (1 << R16BIT) - 1;
		}
		tmp += col << R16SFT;
		*(UINT16 *)(dst + (i * 2)) = (UINT16)tmp;
		spos += 3;
	}
}

static void bco24(BYTE *dst, int width, const BYTE **src, const int *tbl) {

	int		i;
	int		col;

	width *= 3;
	for (i=0; i<width; i++) {
		col = (int)src[0][i] * tbl[0];
		col += (int)src[1][i] * tbl[1];
		col += (int)src[2][i] * tbl[2];
		col += (int)src[3][i] * tbl[3];
		col >>= CUBICLEVEL;
		if (col < 0) {
			col = 0;
		}
		else if (col >= (1 << 8)) {
			col = (1 << 8) - 1;
		}
		dst[i] = (BYTE)col;
	}
}

typedef void (*FNBCI)(RESHDL hdl, BYTE *dst, const BYTE *src);
typedef void (*FNBCO)(BYTE *dst, int width, const BYTE **src, const int *tbl);

static void bicubic(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	UINT	worksize;
	UINT	bcsize;
	RESHDL	reshdl;
	UINT	i;
	UINT	*ptr;
	UINT	ssize;
	UINT	dsize;
	UINT	cur;
const BYTE	*limit;
const BYTE	*p;
	BYTE	*q;
	FNBCI	bci;
	FNBCO	bco;
	BYTE	*bcw[4];

	if (cubictbl == NULL) {
		goto bc_exit;
	}
	worksize = dst->width * sizeof(UINT);
	bcsize = ((dst->width * 3) + 3) & (~3);
	worksize += bcsize * 4;
	reshdl = (RESHDL)_MALLOC(sizeof(_RESHDL) + worksize, "reshdl");
	if (reshdl == NULL) {
		goto bc_exit;
	}
	ptr = (UINT *)(reshdl + 1);
	reshdl->srcwidth = src->width;
	reshdl->dstwidth = dst->width;

	reshdl->h = ptr;
	ssize = src->width - 1;
	dsize = dst->width - 1;
	for (i=0; i<dsize; i++) {
		*ptr++ = ((i << MXBITS) * ssize) / dsize;
	}
	*ptr++ = ssize << MXBITS;

	bci = (type & RESIZE_S24BIT)?bci24:bci16;
	bco = (type & RESIZE_D24BIT)?bco24:bco16;
	bcw[0] = (BYTE *)ptr;
	bcw[1] = bcw[0] + bcsize;
	bcw[2] = bcw[1] + bcsize;
	bcw[3] = bcw[2] + bcsize;

	q = dst->ptr;
	ssize = src->height - 1;
	dsize = dst->height - 1;
	limit = src->ptr + (ssize * src->align);
	for (i=0; i<=dsize; i++) {
		cur = ((i << MYBITS) * ssize) / dsize;
		p = src->ptr + ((cur >> MYBITS) * src->align);
		// ToDo: “¯‚¶ƒ‰ƒCƒ“‚Í‚Q“xŒvŽZ‚µ‚È‚¢‚æ‚¤‚É‚µ‚Ü‚µ‚å‚¤`
		bci(reshdl, bcw[0], max(p - src->align, src->ptr));
		bci(reshdl, bcw[1], p);
		bci(reshdl, bcw[2], min(p + src->align, limit));
		bci(reshdl, bcw[3], min(p + src->align * 2, limit));
		bco(q, dst->width, bcw, cubictbl + ((cur & MYMASK) * 4));
		q += dst->align;
	}
	_MFREE(reshdl);

bc_exit:
	return;
}
#endif


// ---- area average

#if defined(RESIZE_AREAAVG)
static void aamix16(RESHDL hdl, const BYTE *src, int volume) {

	int		i;
	UINT	*buf;
	UINT	posx;
	UINT	curx;
	UINT	col;
	int		r, g, b;
	int		vol;

	buf = hdl->buf;
	posx = 0;
	for (i=0; i<hdl->srcwidth; i++) {
		col = *(UINT16 *)src;
		src += 2;
		b = (col >> B16SFT) & ((1 << B16BIT) - 1);
		g = (col >> G16SFT) & ((1 << G16BIT) - 1);
		r = (col >> R16SFT) & ((1 << R16BIT) - 1);
		curx = hdl->h[i];
		while((curx ^ posx) >> MXBITS) {
			vol = (int)(MULTIX - (posx & MXMASK)) * volume;
			buf[0] += b * vol;
			buf[1] += g * vol;
			buf[2] += r * vol;
			buf += 3;
			posx &= ~MXMASK;
			posx += MULTIX;
		}
		curx -= posx;
		if (curx) {
			posx += curx;
			vol = (int)curx * volume;
			buf[0] += b * vol;
			buf[1] += g * vol;
			buf[2] += r * vol;
		}
	}
}

static void aamix24(RESHDL hdl, const BYTE *src, int volume) {

	int		i;
	UINT	*buf;
	UINT	posx;
	UINT	curx;
	int		vol;

	buf = hdl->buf;
	posx = 0;
	for (i=0; i<hdl->srcwidth; i++) {
		curx = hdl->h[i];
		while((curx ^ posx) >> MXBITS) {
			vol = (int)(MULTIX - (posx & MXMASK)) * volume;
			buf[0] += (int)src[0] * vol;
			buf[1] += (int)src[1] * vol;
			buf[2] += (int)src[2] * vol;
			buf += 3;
			posx &= ~MXMASK;
			posx += MULTIX;
		}
		curx -= posx;
		if (curx) {
			posx += curx;
			vol = (int)curx * volume;
			buf[0] += (int)src[0] * vol;
			buf[1] += (int)src[1] * vol;
			buf[2] += (int)src[2] * vol;
		}
		src += 3;
	}
}

static void aaout16(RESHDL hdl, BYTE *dst) {

	UINT	*buf;
	int		rem;
	UINT	tmp;
	UINT	col;

	buf = hdl->buf;
	rem = hdl->dstwidth;
	do {
		tmp = buf[0] + (buf[0] >> (8 - B16BIT));
		col = (tmp >> (MXBITS + MYBITS - B16SFT)) &
											(((1 << B16BIT) - 1) << B16SFT);
		tmp = buf[1] + (buf[1] >> (8 - G16BIT));
		col += (tmp >> (MXBITS + MYBITS - G16SFT)) &
											(((1 << G16BIT) - 1) << G16SFT);
		tmp = buf[2] + (buf[2] >> (8 - R16BIT));
		col += (tmp >> (MXBITS + MYBITS - R16SFT)) &
											(((1 << R16BIT) - 1) << R16SFT);
		*(UINT16 *)dst = (UINT16)col;
		dst += 2;
		buf += 3;
	} while(--rem);
}

static void aaout16by24(RESHDL hdl, BYTE *dst) {

	UINT	*buf;
	int		rem;
	UINT	col;

	buf = hdl->buf;
	rem = hdl->dstwidth;
	do {
		col = (buf[0] >> (MXBITS + MYBITS + 8 - B16BIT - B16SFT)) &
											(((1 << B16BIT) - 1) << B16SFT);
		col += (buf[1] >> (MXBITS + MYBITS + 8 - G16BIT - G16SFT)) &
											(((1 << G16BIT) - 1) << G16SFT);
		col += (buf[2] >> (MXBITS + MYBITS + 8 - R16BIT - R16SFT)) &
											(((1 << R16BIT) - 1) << R16SFT);
		*(UINT16 *)dst = (UINT16)col;
		dst += 2;
		buf += 3;
	} while(--rem);
}

static void aaout24(RESHDL hdl, BYTE *dst) {

	UINT	*buf;
	int		rem;

	buf = hdl->buf;
	rem = hdl->dstwidth * 3;
	do {
		*dst++ = (BYTE)((*buf++) >> (MXBITS + MYBITS));
	} while(--rem);
}

static void aaout24by16(RESHDL hdl, BYTE *dst) {

	UINT	*buf;
	int		rem;

	buf = hdl->buf;
	rem = hdl->dstwidth;
	do {
		dst[0] = (BYTE)(buf[0] >> (MXBITS + MYBITS - 8 + B16BIT));
		dst[1] = (BYTE)(buf[1] >> (MXBITS + MYBITS - 8 + G16BIT));
		dst[2] = (BYTE)(buf[2] >> (MXBITS + MYBITS - 8 + R16BIT));
		dst += 3;
		buf += 3;
	} while(--rem);
}

typedef void (*FNMIX)(RESHDL hdl, const BYTE *src, int volume);
typedef void (*FNOUT)(RESHDL hdl, BYTE *dst);

static const FNMIX aamix[2] = {aamix16, aamix24};
static const FNOUT aaout[4] = {aaout16, aaout16by24, aaout24by16, aaout24};

static void areaaverage(const RSDIMG *dst, const RSSIMG *src, UINT type) {

	UINT	dstcnt;
	UINT	tbls;
	RESHDL	reshdl;
	UINT	i;
	UINT	*ptr;
const BYTE	*p;
	BYTE	*q;
	UINT	posy;
	UINT	cury;
	FNMIX	fnmix;
	FNOUT	fnout;

	dstcnt = dst->width * 3;
	tbls = src->width + src->height + dstcnt;
	reshdl = (RESHDL)_MALLOC(sizeof(_RESHDL) + (tbls * sizeof(UINT)), "rt");
	if (reshdl == NULL) {
		goto aa_err;
	}

	ptr = (UINT *)(reshdl + 1);
	reshdl->srcwidth = src->width;
	reshdl->dstwidth = dst->width;
	reshdl->h = ptr;
	i = 0;
	do {
		i++;
		*ptr++ = ((i << MXBITS) * (UINT)dst->width) / (UINT)src->width;
	} while(i<(UINT)src->width);
	reshdl->buf = ptr;
	reshdl->bufsize = dstcnt * sizeof(UINT);
	ZeroMemory(ptr, reshdl->bufsize);

	fnmix = aamix[type & 1];
	fnout = aaout[type & 3];

	p = src->ptr;
	q = dst->ptr;
	posy = 0;
	for (i=0; i<(UINT)src->height; i++) {
		cury = (((i + 1) << MYBITS) * (UINT)dst->height) / (UINT)src->height;
		while((cury ^ posy) >> MYBITS) {
			fnmix(reshdl, p, MULTIY - (posy & MYMASK));
			fnout(reshdl, q);
			q += dst->align;
			ZeroMemory(reshdl->buf, reshdl->bufsize);
			posy &= ~MYMASK;
			posy += MULTIY;
		}
		cury -= posy;
		if (cury) {
			posy += cury;
			fnmix(reshdl, p, cury);
		}
		p += src->align;
	}
	_MFREE(reshdl);

aa_err:
	return;
}
#endif


// ---- main

void resize_init(void) {

#if defined(RESIZE_BICUBIC)
	cubic_init();
#endif
}

void resize_term(void) {

#if defined(RESIZE_BICUBIC)
	cubic_term();
#endif
}

void resize(const RSDIMG *dst, const RSSIMG *src) {

	UINT	type;

	type = sizecheck(dst, src);
	if (type & RESIZE_ERROR) {
	}
#if defined(ENABLE_FASTCOPY)
	else if (!(type & (RESIZE_HDIF | RESIZE_VDIF))) {
		fastcopy(dst, src, type);
	}
#endif
	else {
#if defined(RESIZE_AREAAVG)
		areaaverage(dst, src, type);
#elif defined(RESIZE_BICUBIC)
		bicubic(dst, src, type);
#elif defined(RESIZE_BILINEAR)
		bilinear(dst, src, type);
#elif defined(RESIZE_NEAREST)
		nearestneighbor(dst, src, type);
#elif defined(RESIZE_SIMPLE)
		simpleresize(dst, src, type);
#endif
	}
}

