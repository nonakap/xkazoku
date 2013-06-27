#include	"compiler.h"
#include	"vram.h"
#include	"arcfile.h"
#include	"bmpdata.h"


UINT bmpdata_getalign(BMPINFO *bi) {

	UINT	ret;
	int		width;
	int		bit;

	width = LOADINTELDWORD(bi->biWidth);
	bit = LOADINTELWORD(bi->biBitCount);
	ret = ((width * bit) + 7) / 8;
	ret = (ret + 3) & (~3);
	return(ret);
}


UINT bmpdata_getdatasize(BMPINFO *bi) {

	int		height;

	height = LOADINTELDWORD(bi->biHeight);
	if (height < 0) {
		height = 0 - height;
	}
	return(height * bmpdata_getalign(bi));
}


void bmpdata_setinfo(BMPINFO *bi, const BMPDATA *inf) {

	UINT	bmpsize;

	if ((bi == NULL) || (inf == NULL)) {
		goto bdsi_err;
	}
	ZeroMemory(bi, sizeof(BMPINFO));
	STOREINTELDWORD(bi->biSize, sizeof(BMPINFO));
	STOREINTELDWORD(bi->biWidth, inf->width);
	STOREINTELDWORD(bi->biHeight, inf->height);
	STOREINTELWORD(bi->biPlanes, 1);
	STOREINTELWORD(bi->biBitCount, inf->bpp);
//	STOREINTELDWORD(bi->biCompression, BI_RGB);
	bmpsize = bmpdata_getdatasize(bi);
	STOREINTELDWORD(bi->biSizeImage, bmpsize);
//	STOREINTELDWORD(bi->biXPelsPerMeter, 0);
//	STOREINTELDWORD(bi->biYPelsPerMeter, 0);
//	STOREINTELDWORD(bi->biClrUsed, 0);
//	STOREINTELDWORD(bi->biClrImportant, 0);

bdsi_err:
	return;
}


BOOL bmpdata_getinfo(const BMPINFO *bi, BMPDATA *inf) {

	UINT	tmp;
	int		width;
	int		height;

	if ((bi == NULL) || (inf == NULL)) {
		goto bdgi_err;
	}

	tmp = LOADINTELDWORD(bi->biSize);
	if (tmp != sizeof(BMPINFO)) {
		goto bdgi_err;
	}
	tmp = LOADINTELWORD(bi->biPlanes);
	if (tmp != 1) {
		goto bdgi_err;
	}
	tmp = LOADINTELDWORD(bi->biCompression);
	if (tmp != 0) {
		goto bdgi_err;
	}
	width = LOADINTELDWORD(bi->biWidth);
	height = LOADINTELDWORD(bi->biHeight);
	if ((width <= 0) || (height == 0)) {
		goto bdgi_err;
	}
	inf->width = width;
	inf->height = height;
	inf->bpp = LOADINTELWORD(bi->biBitCount);
	return(SUCCESS);

bdgi_err:
	return(FAILURE);
}


// ----

VRAMHDL bmpdata_load8(const char *name) {

	ARCFILEH	hdl;
	VRAMHDL		ret;
	BMPFILE		bmf;
	BMPINFO		bmi;
	BMPDATA		bd;
	long		fptr;
	int			align;
	BYTE		*p;

	hdl = arcfile_open(ARCTYPE_DATA, name);
	if (hdl == NULL) {
		goto bdl8_err1;
	}
	if (arcfile_read(hdl, &bmf, sizeof(bmf)) != sizeof(bmf)) {
		goto bdl8_err2;
	}
	if ((bmf.bfType[0] != 'B') || (bmf.bfType[1] != 'M')) {
		goto bdl8_err2;
	}
	if (arcfile_read(hdl, &bmi, sizeof(bmi)) != sizeof(bmi)) {
		goto bdl8_err2;
	}
	if (bmpdata_getinfo(&bmi, &bd) != SUCCESS) {
		goto bdl8_err2;
	}
	if (bd.bpp != 8) {
		goto bdl8_err2;
	}
	ret = vram_create(bd.width, bd.height, FALSE, 8);
	if (ret == NULL) {
		goto bdl8_err2;
	}
	align = (bd.width + 3) & (~3);
	fptr = LOADINTELDWORD(bmf.bfOffBits);
	// 上下逆にしてるっぽい…
	p = ret->ptr;
	while(bd.height-- > 0) {
		if (arcfile_seek(hdl, fptr, 0) != fptr) {
			break;
		}
		if (arcfile_read(hdl, p, bd.width) != (UINT)bd.width) {
			break;
		}
		fptr += align;
		p += bd.width;
	}
	arcfile_close(hdl);
	return(ret);

bdl8_err2:
	arcfile_close(hdl);

bdl8_err1:
	return(NULL);
}


// ----




VRAMHDL bmpdata_vram24cnv(int width, int height,
											BMPINFO *bi, const BYTE *data) {

	VRAMHDL	ret;
	BMPDATA	inf;
	int		align;
const BYTE	*p;
const BYTE	*q;
	BYTE	*r;
	int		rm;
	int		x;
	int		y;
	int		xstep;
	int		ystep;

	ret = vram_create(width, height, FALSE, DEFAULT_BPP);
	if (ret == NULL) {
		goto bdcv_exit;
	}
	if ((bmpdata_getinfo(bi, &inf) != SUCCESS) ||
		(inf.bpp != 24) || (data == NULL)) {
		goto bdcv_exit;
	}
	align = bmpdata_getalign(bi);
	if (inf.height > 0) {
		data += align * (inf.height - 1);
		align *= -1;
	}
	else {
		inf.height *= -1;
	}
	xstep = (inf.width << 10) / width;
	ystep = (inf.height << 10) / height;

	r = ret->ptr;
	y = 0;
#ifdef SUPPORT_16BPP
	if (ret->bpp == 16) {
		do {
			p = data + ((y >> 10) * align);
			rm = width;
			x = 0;
			do {
				UINT c24;
				q = p + ((x >> 10) * 3);
				c24 = MAKEPALETTE(q[2], q[1], q[0]);
				*(UINT16 *)r = MAKE16PAL(c24);
				r += 2;
				x += xstep;
			} while(--rm);
			y += ystep;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (ret->bpp == 24) {
		do {
			p = data + ((y >> 10) * align);
			rm = width;
			x = 0;
			do {
				q = p + ((x >> 10) * 3);
				r[0] = q[0];
				r[1] = q[1];
				r[2] = q[2];
				r += 3;
				x += xstep;
			} while(--rm);
			y += ystep;
		} while(--height);
	}
#endif

bdcv_exit:
	return(ret);
}


BYTE *bmpdata_bmp24cnv(BMPINFO *bi, VRAMHDL vram) {

	BYTE	*ret;
	BMPDATA	inf;
	int		width;
	int		height;
	int		align;
	int		size;
const BYTE	*p;
	BYTE	*r;
	int		x;
	int		y;
	int		xstep;
	int		ystep;

	ret = NULL;
	if ((bmpdata_getinfo(bi, &inf) != SUCCESS) || (inf.bpp != 24)) {
		goto bdcb_exit;
	}
	align = bmpdata_getalign(bi);
	height = inf.height;
	if (height < 0) {
		height *= -1;
	}
	size = align * height;
	ret = (BYTE *)_MALLOC(size, "bmp");
	if (ret == NULL) {
		goto bdcb_exit;
	}
	ZeroMemory(ret, size);
	if (vram == NULL) {
		goto bdcb_exit;
	}
	r = ret;
	if (inf.height > 0) {
		r += align * (height - 1);
		align *= -1;
	}
	align -= (inf.width * 3);

	xstep = (vram->width << 10) / inf.width;
	ystep = (vram->height << 10) / height;
	y = 0;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		do {
			p = vram->ptr + ((y >> 10) * vram->yalign);
			width = inf.width;
			x = 0;
			do {
				UINT q;
				q = *(UINT16 *)(p + ((x >> 10) * 2));
				r[0] = (BYTE)MAKE24B(q);
				r[1] = (BYTE)MAKE24G(q);
				r[2] = (BYTE)MAKE24R(q);
				r += 3;
				x += xstep;
			} while(--width);
			r += align;
			y += ystep;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		do {
			p = vram->ptr + ((y >> 10) * vram->yalign);
			width = inf.width;
			x = 0;
			do {
				const BYTE *q;
				q = p + ((x >> 10) * 3);
				r[0] = q[0];
				r[1] = q[1];
				r[2] = q[2];
				r += 3;
				x += xstep;
			} while(--width);
			r += align;
			y += ystep;
		} while(--height);
	}
#endif
bdcb_exit:
	return(ret);
}

