#include	"compiler.h"
#include	"resize.h"
#include	"arcfile.h"
#include	"vram.h"
#include	"bmpdata.h"


UINT bmpdata_getalignex(BMPINFO *bi, BOOL pad) {

	UINT	ret;
	int		width;
	int		bit;

	width = LOADINTELDWORD(bi->biWidth);
	bit = LOADINTELWORD(bi->biBitCount);
	ret = ((width * bit) + 7) / 8;
	if (pad) {
		ret = (ret + 3) & (~3);
	}
	return(ret);
}


UINT bmpdata_getdatasizeex(BMPINFO *bi, BOOL pad) {

	int		height;

	height = (SINT32)LOADINTELDWORD(bi->biHeight);
	if (height < 0) {
		height = 0 - height;
	}
	return(height * bmpdata_getalignex(bi, pad));
}


void bmpdata_setinfo(BMPINFO *bi, const BMPDATA *inf, BOOL setsize) {

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
	if (setsize) {
		bmpsize = bmpdata_getdatasizeex(bi, FALSE);
		STOREINTELDWORD(bi->biSizeImage, bmpsize);
	}
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
	height = (SINT32)LOADINTELDWORD(bi->biHeight);
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
	// ã‰º‹t‚É‚µ‚Ä‚é‚Á‚Û‚¢c
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

VRAMHDL bmpdata_vram24cnvex(int width, int height,
								BMPINFO *bi, const BYTE *data, BOOL pad) {

	VRAMHDL	ret;
	BMPDATA	inf;
	RSDIMG	dst;
	RSSIMG	src;

	ret = vram_create(width, height, FALSE, DEFAULT_BPP);
	if (ret == NULL) {
		goto bdcv_exit;
	}
	if ((bmpdata_getinfo(bi, &inf) != SUCCESS) ||
		(inf.bpp != 24) || (data == NULL)) {
		goto bdcv_exit;
	}
	dst.ptr = ret->ptr;
	dst.width = ret->width;
	dst.height = ret->height;
	dst.align = ret->yalign;
	dst.bpp = ret->bpp;

	src.ptr = data;
	src.width = inf.width;
	src.height = inf.height;
	src.align = bmpdata_getalignex(bi, pad);
	src.bpp = 24;
	if (src.height > 0) {
		src.ptr += src.align * (src.height - 1);
		src.align *= -1;
	}
	else {
		src.height *= -1;
	}
	resize(&dst, &src);

bdcv_exit:
	return(ret);
}

BYTE *bmpdata_bmp24cnvex(BMPINFO *bi, VRAMHDL vram, BOOL pad) {

	BYTE	*ret;
	BMPDATA	inf;
	RSDIMG	dst;
	RSSIMG	src;
	UINT	size;

	ret = NULL;
	if ((bmpdata_getinfo(bi, &inf) != SUCCESS) || (inf.bpp != 24)) {
		goto bdcb_exit;
	}
	dst.width = inf.width;
	dst.height = inf.height;
	dst.bpp = 24;
	dst.align = bmpdata_getalignex(bi, pad);

	if (dst.height < 0) {
		dst.height *= -1;
	}
	size = dst.align * dst.height;
	ret = (BYTE *)_MALLOC(size, "bmp");
	if (ret == NULL) {
		goto bdcb_exit;
	}
	ZeroMemory(ret, size);
	if (vram == NULL) {
		goto bdcb_exit;
	}

	dst.ptr = ret;
	if (inf.height > 0) {
		dst.ptr += dst.align * (dst.height - 1);
		dst.align *= -1;
	}
	src.ptr = vram->ptr;
	src.width = vram->width;
	src.height = vram->height;
	src.align = vram->yalign;
	src.bpp = vram->bpp;
	resize(&dst, &src);

bdcb_exit:
	return(ret);
}

