#include	"compiler.h"
#include	"taskmng.h"
#include	"arcfile.h"
#include	"vram.h"
#include	"vramdraw.h"
#include	"cgload.h"
#include	"bmpdata.h"


#if defined(__GNUC__)
typedef struct {
	char	sig[4];
	BYTE	pos[4];
} __attribute__ ((packed)) GADHEAD;
typedef struct {
	BYTE	width[2];
	BYTE	height[2];
} __attribute__ ((packed)) GGDFULL;
typedef struct {
	char	sig[8];
	BYTE	width[2];
	BYTE	height[2];
	BYTE	padding[4];
	BYTE	data_off[4];
	BYTE	datasize[4];
} __attribute__ ((packed)) GG0HEAD;
#else /* !__GNUC__ */
#pragma pack(push, 1)
typedef struct {
	char	sig[4];
	BYTE	pos[4];
} GADHEAD;
typedef struct {
	BYTE	width[2];
	BYTE	height[2];
} GGDFULL;
typedef struct {
	char	sig[8];
	BYTE	width[2];
	BYTE	height[2];
	BYTE	padding[4];
	BYTE	data_off[4];
	BYTE	datasize[4];
} GG0HEAD;
#pragma pack(pop)
#endif /* __GNUC__ */

enum {
	CGTYPE_OTHER		= 0,
	CGTYPE_GGD,
	CGTYPE_GG0,
	CGTYPE_GAD
};

typedef struct {
	VRAMHDL		vram;
	int			width;
	int			height;
	int			align;
	int			cnt;
	BYTE		*tmp;

	BYTE		pal[256*4];

	ARCFILEH	afh;
	int			eof;
	BYTE		*rptr;
	int			rrem;
} _CGLOAD, *CGLOAD;


static int checkext(const char *ext) {

	if (ext) {
		if ((!milstr_cmp(ext, "ggd")) || (!milstr_cmp(ext, "drg"))) {
			return(CGTYPE_GGD);
		}
		else if ((!milstr_cmp(ext, "gg0")) || (!milstr_cmp(ext, "gg2")) ||
				(!milstr_cmp(ext, "gg3"))) {
			return(CGTYPE_GG0);
		}
		else if ((!milstr_cmp(ext, "dga")) || (!milstr_cmp(ext, "gad"))) {
			return(CGTYPE_GAD);
		}
	}
	return(CGTYPE_OTHER);
}


// ---- arcfile load

#define	CGLOADBUFFERS	1024

static CGLOAD cgload_prepare(VRAMHDL *vram, ARCFILEH afh,
											int width, int height, int bpp) {

	CGLOAD		ret;
	VRAMHDL		hdl;
	VRAMHDL		tmp;
	POINT_T		pt;
	int			size;
	BOOL		remake;
	RECT_T		clr;

	ret = NULL;
	if (bpp == 24) {
		bpp = DEFAULT_BPP;
	}
	pt.x = width;
	pt.y = height;
	if (pt.y < 0) {
		pt.y *= -1;
	}
	if ((pt.x <= 0) || (pt.y == 0)) {
		goto clpp_exit;
	}
#ifdef SIZE_QVGA
	vramdraw_halfsize(&pt);
#endif

	hdl = *vram;
	remake = FALSE;
	if (hdl) {
		if (pt.x > hdl->width) {
			remake = TRUE;
		}
		else {
			pt.x = hdl->width;
		}
		if (pt.y > hdl->height) {
			remake = TRUE;
		}
		else {
			pt.y = hdl->height;
		}
	}
	else {
		remake = TRUE;
	}
	if (remake) {
		tmp = vram_create(pt.x, pt.y, (bpp != 8)?TRUE:FALSE, bpp);
		if (tmp == NULL) {
			goto clpp_exit;
		}
		if (hdl) {
			tmp->posx = hdl->posx;
			tmp->posy = hdl->posy;
			vram_destroy(hdl);
		}
		hdl = tmp;
		*vram = hdl;
	}
	else {
		clr.left = width;
		clr.top = 0;
		clr.right = hdl->width;
		clr.bottom = height;
		vram_zerofill(hdl, &clr);
		clr.left = 0;
		clr.top = height;
		clr.right = hdl->width;
		clr.bottom = hdl->height;
		vram_zerofill(hdl, &clr);
	}

	size = sizeof(_CGLOAD);
	size += CGLOADBUFFERS;
	ret = (CGLOAD)_MALLOC(size, "cgload");
	if (ret) {
		ZeroMemory(ret, size);
		ret->vram = hdl;
		ret->afh = afh;
		ret->width = width;
		ret->height = height;
		ret->rptr = (BYTE *)(ret + 1);
	}
clpp_exit:
	return(ret);
}

static void cgload_preread(CGLOAD hdl) {

	BYTE	*ptr;
	UINT	size;
	UINT	rsize;

	if ((hdl->eof) || (hdl->rrem < 0)) {
		goto clpr_exit;
	}
	ptr = (BYTE *)(hdl + 1);
	if (hdl->rrem) {
		CopyMemory(ptr, hdl->rptr, hdl->rrem);
	}
	hdl->rptr = ptr;
	size = CGLOADBUFFERS - hdl->rrem;
	rsize = arcfile_read(hdl->afh, ptr + hdl->rrem, size);
	hdl->rrem += rsize;
	if (size != rsize) {
		hdl->eof = 1;
	}

clpr_exit:
	return;
}

static BOOL cgload_read(CGLOAD hdl, BYTE *buf, int size) {

	int		rsize;

	if (hdl->rrem < 0) {
		return(FAILURE);
	}
	while(size > 0) {
		rsize = min(size, hdl->rrem);
		if (rsize) {
			CopyMemory(buf, hdl->rptr, rsize);
			hdl->rptr += rsize;
			hdl->rrem -= rsize;
			size -= rsize;
			buf += rsize;
			if (!size) {
				break;
			}
		}
		hdl->rptr = (BYTE *)(hdl + 1);
		hdl->rrem = arcfile_read(hdl->afh, hdl + 1, CGLOADBUFFERS);
		if (!hdl->rrem) {
			hdl->eof = 1;
			return(FAILURE);
		}
	}
	return(SUCCESS);
}


// ---- ggd 256g

static BOOL ggd256g_check(ARCFILEH afh, int *width, int *height) {

	BMPINFO	bmi;
	UINT	tmp;

	if (arcfile_read(afh, &bmi, sizeof(bmi)) != sizeof(bmi)) {
		goto g2c_err;
	}
	tmp = LOADINTELDWORD(bmi.biSize);
	if (tmp != sizeof(bmi)) {
		goto g2c_err;
	}
	tmp = LOADINTELWORD(bmi.biPlanes);
	if (tmp != 1) {
		goto g2c_err;
	}
	tmp = LOADINTELWORD(bmi.biBitCount);
	if (tmp != 8) {
		goto g2c_err;
	}
	tmp = LOADINTELDWORD(bmi.biCompression);
	if (tmp != 0) {
		goto g2c_err;
	}
	*width = (SINT32)LOADINTELDWORD(bmi.biWidth);
	*height = (SINT32)LOADINTELDWORD(bmi.biHeight);
	return(SUCCESS);

g2c_err:
	return(FAILURE);
}

static BOOL ggd256g_phase0(CGLOAD hdl) {					// €”õ`

	BYTE	*tmp;
	int		align;
	int		cnt;
	BYTE	work[4];

	if (cgload_read(hdl, hdl->pal, 256*4) != SUCCESS) {
		goto g2p0_err;
	}
	cnt = 256;
	tmp = hdl->pal;
	do {
		if (tmp[0] || tmp[1] || tmp[2]) {
			tmp[3] = 0xff;
		}
		else {
			tmp[3] = 0x00;
		}
		tmp += 4;
	} while(--cnt);
	if (cgload_read(hdl, work, 4) != SUCCESS) {
		goto g2p0_err;
	}
	align = (hdl->width + 3) & (~3);
	hdl->align = align;
	cnt = hdl->height;
	if (cnt < 0) {
		cnt = 0 - cnt;
	}
	cnt *= align;
	tmp = (BYTE *)_MALLOC(cnt, "ggd 256g tmp");
	if (tmp == NULL) {
		goto g2p0_err;
	}
	hdl->tmp = tmp;
	ZeroMemory(tmp, cnt);
	hdl->cnt = LOADINTELDWORD(work);
	hdl->cnt = min(hdl->cnt, cnt);
	return(SUCCESS);

g2p0_err:
	return(FAILURE);
}

static BOOL ggd256g_phase1(CGLOAD hdl) {					// ‰ð“€`

	BYTE	ctrl;
	int		cnt;
	int		p;
	int		breakp;
	int		leng;
	int		zero;
	int		backword;
	BYTE	*dst;
	int		dstrem;

	p = 0;
	breakp = 1024 * 2;
	dst = hdl->tmp;
	dstrem = hdl->cnt;

	while(dstrem) {
		if (hdl->rrem < 17) {
			cgload_preread(hdl);
		}
		hdl->rrem--;
		if (hdl->rrem < 0) {
			goto g2p1_done;
		}
		ctrl = *hdl->rptr++;

		cnt = 8;
		do {
			if (ctrl & 1) {
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto g2p1_done;
				}
				if (dstrem) {
					dstrem--;
					*dst++ = *hdl->rptr;
					p++;
				}
				hdl->rptr++;
			}
			else {
				hdl->rrem -= 2;
				if (hdl->rrem < 0) {
					goto g2p1_done;
				}
				backword = hdl->rptr[0];
				backword |= (hdl->rptr[1] & 0xf0) << 4;
				backword = (p - backword - 19) & 0xfff;
				backword += 1;
				leng = (hdl->rptr[1] & 0xf) + 3;
				leng = min(leng, dstrem);
				dstrem -= leng;
				hdl->rptr += 2;
				zero = backword - p;
				p += leng;
				if (zero > 0) {
					zero = min(leng, zero);
					leng -= zero;
					ZeroMemory(dst, zero);
					dst += zero;
				}
				while(leng--) {
					*dst = *(dst - backword);
					dst++;
				}
			}
			ctrl >>= 1;
		} while(--cnt);
		if (p >= breakp) {
			breakp += 1024 * 2;
			taskmng_rol();
		}
	}

g2p1_done:
	return(SUCCESS);
}

#ifndef SIZE_QVGA
static BOOL ggd256g_phase2(CGLOAD hdl) {					// ƒRƒs`

	VRAMHDL	vram;
const BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
const BYTE	*pal;
	int		salign;
	int		dalign;
	int		aalign;
	int		r;
	int		height;

	vram = hdl->vram;
	src = hdl->tmp;
	dst = vram->ptr;
	alpha = vram->alpha;
	height = hdl->height;
	salign = hdl->align;
	if (height < 0) {
		height = 0 - height;
	}
	else {
		src += hdl->align * (height - 1);
		salign = 0 - salign;
	}
	salign -= hdl->width;
	aalign = vram->width - hdl->width;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		dalign = vram->yalign - (hdl->width * 2);
		do {
			r = hdl->width;
			do {
				UINT col;
				pal = hdl->pal;
				pal += (UINT)src[0] * 4;
				col = MAKEPALETTE(pal[2], pal[1], pal[0]);
				if (col) {
					col = MAKE16PAL(col);
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				*(UINT16 *)dst = (UINT16)col;
				src += 1;
				dst += 2;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		dalign = vram->yalign - (hdl->width * 3);
		do {
			r = hdl->width;
			do {
				pal = hdl->pal;
				pal += (UINT)src[0] * 4;
				dst[0] = pal[0];
				dst[1] = pal[1];
				dst[2] = pal[2];
				*alpha = pal[3];
				src += 1;
				dst += 3;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#else
static BOOL ggd256g_phase2(CGLOAD hdl) {					// ƒRƒs`

	VRAMHDL	vram;
const BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
const BYTE	*pal;
	int		salign;
	int		dalign;
	int		aalign;
	int		width2;
	int		height;
	int		r;

	src = hdl->tmp;
	width2 = (hdl->width + 1) >> 1;
	height = hdl->height;
	salign = (hdl->align * 2);
	if (height < 0) {
		height = 0 - height;
	}
	else {
		src += hdl->align * (height - 1);
		salign = 0 - salign;
	}
	salign -= width2 * 2;
	height = (height + 1) >> 1;
	vram = hdl->vram;
	dst = vram->ptr;
	alpha = vram->alpha;
	aalign = vram->width - width2;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		dalign = vram->yalign - (width2 * 2);
		do {
			r = width2;
			do {
				UINT col;
				pal = hdl->pal;
				pal += (UINT)src[0] * 4;
				col = MAKEPALETTE(pal[2], pal[1], pal[0]);
				if (col) {
					col = MAKE16PAL(col);
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				*(UINT16 *)dst = (UINT16)col;
				src += 2;
				dst += 2;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		dalign = vram->yalign - (width2 * 3);
		do {
			r = width2;
			do {
				pal = hdl->pal;
				pal += (UINT)src[0] * 4;
				dst[0] = pal[0];
				dst[1] = pal[1];
				dst[2] = pal[2];
				*alpha = pal[3];
				src += 2;
				dst += 3;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#endif

static BOOL ggd256g_abort(CGLOAD hdl) {

	if (hdl->tmp) {
		_MFREE(hdl->tmp);
	}
	return(SUCCESS);
}

static BOOL cgload_ggd256g(VRAMHDL *vram, ARCFILEH afh) {

	int		width;
	int		height;
	CGLOAD	hdl;
	BOOL	r;

	if (ggd256g_check(afh, &width, &height) != SUCCESS) {
		goto clgm_err;
	}
	hdl = cgload_prepare(vram, afh, width, height, 24);
	if (hdl == NULL) {
		goto clgm_err;
	}
	r = ggd256g_phase0(hdl);
	if (r == SUCCESS) {
		r = ggd256g_phase1(hdl);
	}
	if (r == SUCCESS) {
		r = ggd256g_phase2(hdl);
	}
	ggd256g_abort(hdl);
	_MFREE(hdl);
	return(r);

clgm_err:
	return(FAILURE);
}


// ---- ggd full

static BOOL ggdfull_phase0(CGLOAD hdl) {					// €”õ`

	BYTE	*tmp;
	int		align;
	int		cnt;

	align = ((hdl->width + 1) * 3) & (~3);
	cnt = align * hdl->height;
	tmp = (BYTE *)_MALLOC(cnt + 0x300, "ggd full tmp");
	if (tmp == NULL) {
		return(FAILURE);
	}
	hdl->align = align;
	hdl->cnt = cnt;
	hdl->tmp = tmp;
	return(SUCCESS);
}

static BOOL ggdfull_phase1(CGLOAD hdl) {					// ‰ð“€`

	BYTE	*tmp;
	BYTE	cmd;
	int		p;
	int		q;
	int		r;
	int		breakp;

	p = 0;
	breakp = 1024 * 3;
	tmp = hdl->tmp;

	while(p < hdl->cnt) {
		if (hdl->rrem < 5) {
			cgload_preread(hdl);
		}

		hdl->rrem--;
		if (hdl->rrem < 0) {
			goto gfp1_done;
		}
		cmd = *hdl->rptr++;
		switch(cmd) {
			case 0:
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto gfp1_done;
				}
				r = *hdl->rptr++;
				r *= 3;
				if (p) {
					p += r;
					while(r--) {
						*tmp = *(tmp - 3);
						tmp++;
					}
				}
				break;

			case 1:
				hdl->rrem -= 2;
				if (hdl->rrem < 0) {
					goto gfp1_done;
				}
				q = hdl->rptr[1] * 3;
				if (q <= p) {
					r = hdl->rptr[0] * 3;
					p += r;
					while(r--) {
						*tmp = *(tmp - q);
						tmp++;
					}
				}
				hdl->rptr += 2;
				break;

			case 2:
				hdl->rrem -= 3;
				if (hdl->rrem < 0) {
					goto gfp1_done;
				}
				q = LOADINTELWORD(hdl->rptr+1);
				q *= 3;
				if (q <= p) {
					r = hdl->rptr[0] * 3;
					p += r;
					while(r--) {
						*tmp = *(tmp - q);
						tmp++;
					}
				}
				hdl->rptr += 3;
				break;

			case 3:
			case 4:
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto gfp1_done;
				}
				q = *hdl->rptr++;
				if (cmd == 4) {
					hdl->rrem--;
					if (hdl->rrem < 0) {
						goto gfp1_done;
					}
					q |= (*hdl->rptr++) << 8;
				}
				q *= 3;
				if (q <= p) {
					*(tmp + 0) = *(tmp - q + 0);
					*(tmp + 1) = *(tmp - q + 1);
					*(tmp + 2) = *(tmp - q + 2);
					tmp += 3;
					p += 3;
				}
				break;

			default:
				r = cmd - 4;
				r *= 3;
				if (cgload_read(hdl, tmp, r) != SUCCESS) {
					return(SUCCESS);
				}
				tmp += r;
				p += r;
				break;
		}
		if (p >= breakp) {
			breakp += 1024 * 3;
			taskmng_rol();
		}
	}

gfp1_done:
	return(SUCCESS);
}

#ifndef SIZE_QVGA
static BOOL ggdfull_phase2(CGLOAD hdl) {					// ƒRƒs`

	VRAMHDL	vram;
const BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
	int		salign;
	int		dalign;
	int		aalign;
	int		height;
	int		r;

	src = hdl->tmp;
	vram = hdl->vram;
	salign = hdl->align - (hdl->width * 3);
	dst = vram->ptr;
	alpha = vram->alpha;
	aalign = vram->width - hdl->width;
	height = hdl->height;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		dalign = vram->yalign - (hdl->width * 2);
		do {
			r = hdl->width;
			do {
				UINT col;
				col = MAKEPALETTE(src[2], src[1], src[0]);
				if (col) {
					col = MAKE16PAL(col);
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				*(UINT16 *)dst = (UINT16)col;
				src += 3;
				dst += 2;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha+= aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		dalign = vram->yalign - (hdl->width * 3);
		do {
			r = hdl->width;
			do {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				if (src[0] || src[1] || src[2]) {
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				src += 3;
				dst += 3;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#else
static BOOL ggdfull_phase2(CGLOAD hdl) {					// ƒRƒs`

	VRAMHDL	vram;
const BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
	int		salign;
	int		dalign;
	int		aalign;
	int		width2;
	int		height;
	int		r;

	src = hdl->tmp;
	width2 = (hdl->width + 1) >> 1;
	salign = (hdl->align * 2) - (width2 * 2 * 3);
	height = (hdl->height + 1) >> 1;
	vram = hdl->vram;
	dst = vram->ptr;
	alpha = vram->alpha;
	aalign = vram->width - width2;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		dalign = vram->yalign - (width2 * 2);
		do {
			r = width2;
			do {
				UINT col;
				col = MAKEPALETTE(src[2], src[1], src[0]);
				if (col) {
					col = MAKE16PAL(col);
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				*(UINT16 *)dst = (UINT16)col;
				src += 6;
				dst += 2;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		dalign = vram->yalign - (width2 * 3);
		do {
			r = width2;
			do {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				if (src[0] || src[1] || src[2]) {
					*alpha = 0xff;
				}
				else {
					*alpha = 0x00;
				}
				src += 6;
				dst += 3;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#endif

static BOOL ggdfull_abort(CGLOAD hdl) {

	if (hdl->tmp) {
		_MFREE(hdl->tmp);
	}
	return(SUCCESS);
}

static BOOL cgload_ggdfull(VRAMHDL *vram, ARCFILEH afh) {

	GGDFULL	gf;
	CGLOAD	hdl;
	BOOL	r;

	if (arcfile_read(afh, &gf, sizeof(gf)) != sizeof(gf)) {
		goto clgf_err;
	}
	hdl = cgload_prepare(vram, afh, LOADINTELWORD(gf.width),
									LOADINTELWORD(gf.height), 24);
	if (hdl == NULL) {
		goto clgf_err;
	}
	r = ggdfull_phase0(hdl);
	if (r == SUCCESS) {
		r = ggdfull_phase1(hdl);
	}
	if (r == SUCCESS) {
		r = ggdfull_phase2(hdl);
	}
	ggdfull_abort(hdl);
	_MFREE(hdl);
	return(r);

clgf_err:
	return(FAILURE);
}


// --- ggd

static BOOL cgload_ggd(VRAMHDL *vram, ARCFILEH afh) {

	BYTE	type[4];

	if (arcfile_read(afh, type, 4) != 4) {
		goto clggd_err;
	}
	type[0] ^= 0xff;
	type[1] ^= 0xff;
	type[2] ^= 0xff;
	type[3] ^= 0xff;
	if (!memcmp(type, "FULL", 4)) {
		return(cgload_ggdfull(vram, afh));
	}
	else if (!memcmp(type, "256G", 4)) {
		return(cgload_ggd256g(vram, afh));
	}

clggd_err:
	return(FAILURE);
}

static BOOL cgload_gad(VRAMHDL *vram, ARCFILEH afh) {

	GADHEAD	gadhead;
	long	pos;

	if (arcfile_read(afh, &gadhead, sizeof(gadhead)) != sizeof(gadhead)) {
		goto clgad_err;
	}
	if (memcmp(gadhead.sig, "GAD ", 4)) {
		goto clgad_err;
	}
	pos = LOADINTELDWORD(gadhead.pos);
	if (arcfile_seek(afh, pos, 0) != pos) {
		goto clgad_err;
	}
	return(cgload_ggd(vram, afh));

clgad_err:
	return(FAILURE);
}


// ---- gg0

static const int delta[3] = {0, 1, -1};

static BOOL gg0_phase0(CGLOAD hdl) {						// €”õ`

	BYTE	*tmp;
	int		cnt;

	cnt = hdl->width * hdl->height * 4;
	tmp = (BYTE *)_MALLOC(cnt + 0x400, "gg0 tmp");
	if (tmp == NULL) {
		return(FAILURE);
	}
	ZeroMemory(tmp, cnt);
	hdl->cnt = cnt;
	hdl->tmp = tmp;
	return(SUCCESS);
}

static BOOL gg0_phase1(CGLOAD hdl) {						// ‰ð“€`

	BYTE	*tmp;
	BYTE	cmd;
	BYTE	buf;
	int		p;
	int		q;
	int		r;
	int		breakp;

	p = 0;
	breakp = 1024 * 4;
	tmp = hdl->tmp;

	while(p < hdl->cnt) {
		if (cgload_read(hdl, &cmd, 1) != SUCCESS) {
			goto g0p1_done;
		}
		switch(cmd) {
			case 0x00:
			case 0x01:
				if (cgload_read(hdl, &buf, 1) != SUCCESS) {
					goto g0p1_done;
				}
				r = buf;
				if (cmd & 1) {
					if (cgload_read(hdl, &buf, 1) != SUCCESS) {
						goto g0p1_done;
					}
					r |= (buf << 8);
				}
				r = min(r * 4, hdl->cnt - p);
				if (p) {
					p += r;
					while(r--) {
						*tmp = *(tmp - 4);
						tmp++;
					}
				}
				break;

			case 0x02:
			case 0x03:
				if (cgload_read(hdl, &buf, 1) != SUCCESS) {
					goto g0p1_done;
				}
				q = buf;
				if (cmd & 1) {
					if (cgload_read(hdl, &buf, 1) != SUCCESS) {
						goto g0p1_done;
					}
					q |= (buf << 8);
				}
				q *= 4;
				if (q <= p) {
					*(tmp + 0) = *(tmp - q + 0);
					*(tmp + 1) = *(tmp - q + 1);
					*(tmp + 2) = *(tmp - q + 2);
					*(tmp + 3) = *(tmp - q + 3);
					tmp += 4;
					p += 4;
				}
				break;

			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
				if (cgload_read(hdl, &buf, 1) != SUCCESS) {
					goto g0p1_done;
				}
				q = buf;
				if (cmd & 2) {
					if (cgload_read(hdl, &buf, 1) != SUCCESS) {
						goto g0p1_done;
					}
					q |= (buf << 8);
				}

				if (cgload_read(hdl, &buf, 1) != SUCCESS) {
					goto g0p1_done;
				}
				r = buf;
				if (cmd & 1) {
					if (cgload_read(hdl, &buf, 1) != SUCCESS) {
						goto g0p1_done;
					}
					r |= (buf << 8);
				}
				q *= 4;
				if (q <= p) {
					r = min(r * 4, hdl->cnt - p);
					p += r;
					while(r--) {
						*tmp = *(tmp - q);
						tmp++;
					}
				}
				break;

			case 0x08:
				if (p) {
					*(tmp + 0) = *(tmp - 4);
					*(tmp + 1) = *(tmp - 3);
					*(tmp + 2) = *(tmp - 2);
					*(tmp + 3) = *(tmp - 1);
					tmp += 4;
					p += 4;
				}
				break;

			case 0x09:
			case 0x0a:
			case 0x0b:
				q = (hdl->width + delta[cmd - 0x09]) * 4;
				if (q <= p) {
					*(tmp + 0) = *(tmp - q + 0);
					*(tmp + 1) = *(tmp - q + 1);
					*(tmp + 2) = *(tmp - q + 2);
					*(tmp + 3) = *(tmp - q + 3);
					tmp += 4;
					p += 4;
				}
				break;

			default:
				r = cmd - 0x0b;
				r *= 4;
				if (cgload_read(hdl, tmp, r) != SUCCESS) {
					goto g0p1_done;
				}
				tmp += r;
				p += r;
				break;
		}
		if (p >= breakp) {
			breakp += 1024 * 4;
			taskmng_rol();
		}
	}

g0p1_done:
	return(SUCCESS);
}

#ifndef SIZE_QVGA
static BOOL gg0_phase2(CGLOAD hdl) {						// ƒRƒs`

	BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
	int		dalign;
	int		aalign;
	int		height;
	int		r;

	src = hdl->tmp;
	dst = hdl->vram->ptr;
	alpha = hdl->vram->alpha;
	aalign = hdl->vram->width - hdl->width;
	height = hdl->height;
#ifdef SUPPORT_16BPP
	if (hdl->vram->bpp == 16) {
		dalign = hdl->vram->yalign - (hdl->width * 2);
		do {
			r = hdl->width;
			do {
				UINT32 col;
				col = MAKEPALETTE(src[2], src[1], src[0]);
				*(UINT16 *)dst = MAKE16PAL(col);
				*alpha = src[3];
				src += 4;
				dst += 2;
				alpha += 1;
			} while(--r);
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (hdl->vram->bpp == 24) {
		dalign = hdl->vram->yalign - (hdl->width * 3);
		do {
			r = hdl->width;
			do {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				*alpha = src[3];
				src += 4;
				dst += 3;
				alpha += 1;
			} while(--r);
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#else
static BOOL gg0_phase2(CGLOAD hdl) {						// ƒRƒs`

	BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
	int		salign;
	int		dalign;
	int		aalign;
	int		width2;
	int		height;
	int		r;

	src = hdl->tmp;
	dst = hdl->vram->ptr;
	alpha = hdl->vram->alpha;
	width2 = (hdl->width + 1) >> 1;
	salign = (hdl->width * 2 * 4) - (width2 * 2 * 4);
	aalign = hdl->vram->width - width2;
	height = (hdl->height + 1) >> 1;
#ifdef SUPPORT_16BPP
	if (hdl->vram->bpp == 16) {
		dalign = hdl->vram->yalign - (width2 * 2);
		do {
			r = width2;
			do {
				UINT32 col;
				col = MAKEPALETTE(src[2], src[1], src[0]);
				*(UINT16 *)dst = MAKE16PAL(col);
				*alpha = src[3];
				src += 8;
				dst += 2;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
#ifdef SUPPORT_24BPP
	if (hdl->vram->bpp == 24) {
		dalign = hdl->vram->yalign - (width2 * 3);
		do {
			r = width2;
			do {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				*alpha = src[3];
				src += 8;
				dst += 3;
				alpha += 1;
			} while(--r);
			src += salign;
			dst += dalign;
			alpha += aalign;
		} while(--height);
	}
#endif
	return(SUCCESS);
}
#endif

static void gg0_abort(CGLOAD hdl) {

	if (hdl->tmp) {
		_MFREE(hdl->tmp);
	}
}

static BOOL cgload_gg0(VRAMHDL *vram, ARCFILEH afh) {

	GG0HEAD	gg0head;
	long	pos;
	CGLOAD	hdl;
	BOOL	r;

	if (arcfile_read(afh, &gg0head, sizeof(gg0head)) != sizeof(gg0head)) {
		goto clgg0_err;
	}
	if (memcmp(gg0head.sig, "GGA00000", 8)) {
		goto clgg0_err;
	}
	pos = LOADINTELDWORD(gg0head.data_off);
	if (arcfile_seek(afh, pos, 0) != pos) {
		goto clgg0_err;
	}
	hdl = cgload_prepare(vram, afh, LOADINTELWORD(gg0head.width),
										LOADINTELWORD(gg0head.height), 24);
	if (hdl == NULL) {
		goto clgg0_err;
	}
	r = gg0_phase0(hdl);
	if (r == SUCCESS) {
		r = gg0_phase1(hdl);
	}
	if (r == SUCCESS) {
		r = gg0_phase2(hdl);
	}
	gg0_abort(hdl);
	_MFREE(hdl);
	return(r);

clgg0_err:
	return(FAILURE);
}


// ----

#ifndef SIZE_QVGA
static BOOL gmask_phase2(CGLOAD hdl) {					// ƒRƒs`

	BYTE	*src;
	BYTE	*dst;
	int		r;
	int		salign;

	src = hdl->tmp;
	salign = hdl->align;
	dst = hdl->vram->ptr;
	r = hdl->height;
	if (r > 0) {
		src += (r - 1) * salign;
		salign = 0 - salign;
	}
	else {
		r = 0 - r;
	}
	do {
		CopyMemory(dst, src, hdl->width);
		src += salign;
		dst += hdl->width;
	} while(--r);
	return(SUCCESS);
}
#else
static BOOL gmask_phase2(CGLOAD hdl) {					// ƒRƒs`

	BYTE	*src;
	BYTE	*dst;
	int		width2;
	int		height;
	int		salign;
	int		r;

	src = hdl->tmp;
	dst = hdl->vram->ptr;
	salign = hdl->align * 2;
	width2 = (hdl->width + 1) >> 1;
	height = hdl->height;
	if (height > 0) {
		src += (height - 1) * salign;
		salign = 0 - salign;
	}
	else {
		height = 0 - height;
	}
	salign -= (width2 * 2);
	height = (height + 1) >> 1;
	do {
		r = width2;
		do {
			dst[0] = src[0];
			src += 2;
			dst++;
		} while(--r);
		src += salign;
	} while(--height);
	return(SUCCESS);
}
#endif

static void gmask_abort(CGLOAD hdl) {

	if (hdl->tmp) {
		_MFREE(hdl->tmp);
	}
}

static BOOL cgload_gmask(VRAMHDL *vram, ARCFILEH afh) {

	BYTE	type[4];
	int		width;
	int		height;
	CGLOAD	hdl;
	BOOL	r;

	if (arcfile_read(afh, type, 4) != 4) {
		goto clgm_err;
	}
	type[0] ^= 0xff;
	type[1] ^= 0xff;
	type[2] ^= 0xff;
	type[3] ^= 0xff;
	if (memcmp(type, "256G", 4)) {
		goto clgm_err;
	}
	if (ggd256g_check(afh, &width, &height) != SUCCESS) {
		goto clgm_err;
	}
	hdl = cgload_prepare(vram, afh, width, height, 8);
	if (hdl == NULL) {
		goto clgm_err;
	}
	r = ggd256g_phase0(hdl);
	if (r == SUCCESS) {
		r = ggd256g_phase1(hdl);
	}
	if (r == SUCCESS) {
		r = gmask_phase2(hdl);
	}
	gmask_abort(hdl);
	_MFREE(hdl);
	return(r);

clgm_err:
	return(FAILURE);
}


// ----

BOOL cgload_data(VRAMHDL *vram, UINT type, const char *name) {

	BOOL		ret;
	ARCFILEH	afh;
	int			cgtype;

	ret = FAILURE;
	afh = arcfile_open(type, name);
	if (afh == NULL) {
		goto cldat_err;
	}
	cgtype = checkext(afh->ext);
	if (cgtype == CGTYPE_GGD) {
		ret = cgload_ggd(vram, afh);
	}
	else if (cgtype == CGTYPE_GG0) {
		ret = cgload_gg0(vram, afh);
	}
	else if (cgtype == CGTYPE_GAD) {
		ret = cgload_gad(vram, afh);
	}
	arcfile_close(afh);

cldat_err:
	return(ret);
}

BOOL cgload_mask(VRAMHDL *vram, UINT type, const char *name) {

	BOOL		ret;
	ARCFILEH	afh;
	int			cgtype;

	ret = FAILURE;
	afh = arcfile_open(type, name);
	if (afh == NULL) {
		goto clmc_err;
	}
	cgtype = checkext(afh->ext);
	if (cgtype == CGTYPE_GGD) {
		ret = cgload_gmask(vram, afh);
	}
	arcfile_close(afh);

clmc_err:
	return(ret);
}


// ----

#ifdef SUPPORT_16BPP
static void solvegan16a(BYTE *dst, int dstrem, const BYTE *src, int srcrem) {

	UINT	cnt;
	UINT32	col;
	UINT32	last;

	last = 0xffffffff;
	while(1) {
		srcrem -= 3;
		if (srcrem < 0) {
			break;
		}
		dstrem -= 1;
		if (dstrem < 0) {
			break;
		}
		col = MAKEPALETTE(src[2], src[1], src[0]);
		col = MAKE16PAL(col);
		*(UINT16 *)dst = (UINT16)col;
		src += 3;
		dst += 2;
		if (last == col) {
			srcrem -= 1;
			if (srcrem < 0) {
				break;
			}
			cnt = *src++;
			if (cnt & 0x80) {
				srcrem -= 1;
				if (srcrem < 0) {
					break;
				}
				cnt &= 0x7f;
				cnt <<= 8;
				cnt |= *src++;
				if (cnt == 0x7fff) {
					srcrem -= 4;
					if (srcrem < 0) {
						break;
					}
					cnt = LOADINTELDWORD(src);
					src += 4;
				}
			}
			if (cnt > 2) {
				cnt = min(cnt - 2, (UINT)dstrem);
				dstrem -= cnt;
				while(cnt--) {
					*(UINT16 *)dst = (UINT16)last;
					dst += 2;
				}
			}
		}
		last = col;
	}
}

static void solvegan16b(BYTE *dst, int dstrem, const BYTE *src, int srcrem) {

const BYTE	*head;
	int		headrem;
	UINT	leng;
	UINT	cnt;
	UINT32	col;
	UINT32	last;

	srcrem -= 4;
	if (srcrem < 0) {
		return;
	}
	headrem = LOADINTELDWORD(src);
	src += 4;
	head = src;
	headrem -= 4;
	if (headrem < 0) {
		return;
	}
	src += headrem;
	srcrem -= headrem;
	if (srcrem < 0) {
		return;
	}

	cnt = 0;
	last = 0xffffffff;

	while(1) {
		headrem -= 1;
		if (headrem < 0) {
			break;
		}
		leng = *head++;
		if (leng & 0x80) {
			headrem -= 1;
			if (headrem < 0) {
				break;
			}
			leng &= 0x7f;
			leng <<= 8;
			leng |= *head++;
			if (leng == 0x7fff) {
				headrem -= 4;
				if (headrem < 0) {
					break;
				}
				leng = LOADINTELDWORD(head);
				head += 4;
			}
		}
		while(leng--) {
			dstrem -= 1;
			if (dstrem < 0) {
				break;
			}
			if (!cnt) {
				srcrem -= 3;
				if (srcrem < 0) {
					break;
				}
				col = MAKEPALETTE(src[2], src[1], src[0]);
				col = MAKE16PAL(col);
				*(UINT16 *)dst = (UINT16)col;
				src += 3;
				dst += 2;
				if (last == col) {
					srcrem -= 1;
					if (srcrem < 0) {
						break;
					}
					cnt = *src++;
					if (cnt & 0x80) {
						srcrem -= 1;
						if (srcrem < 0) {
							break;
						}
						cnt &= 0x7f;
						cnt <<= 8;
						cnt |= *src++;
						if (cnt == 0x7fff) {
							srcrem -= 4;
							if (srcrem < 0) {
								break;
							}
							cnt = LOADINTELDWORD(src);
							src += 4;
						}
					}
					if (cnt > 2) {
						cnt -= 2;
					}
					else {
						cnt = 0;
					}
				}
				last = col;
			}
			else {
				cnt--;
				*(UINT16 *)dst = (UINT16)last;
				dst += 2;
			}
		}

		headrem -= 1;
		if (headrem < 0) {
			break;
		}
		leng = *head++;
		if (leng & 0x80) {
			headrem -= 1;
			if (headrem < 0) {
				break;
			}
			leng &= 0x7f;
			leng <<= 8;
			leng |= *head++;
			if (leng == 0x7fff) {
				headrem -= 4;
				if (headrem < 0) {
					break;
				}
				leng = LOADINTELDWORD(head);
				head += 4;
				if (leng < 0) {
					break;
				}
			}
		}
		leng = min(leng, (UINT)dstrem);
		dstrem -= leng;
		dst += leng * 2;
	}
}

#endif
#ifdef SUPPORT_24BPP
static void solvegan24a(BYTE *dst, int dstrem, const BYTE *src, int srcrem) {

	UINT	cnt;
	UINT32	col;
	UINT32	last;

	last = 0xffffffff;
	while(1) {
		srcrem -= 3;
		if (srcrem < 0) {
			break;
		}
		dstrem -= 1;
		if (dstrem < 0) {
			break;
		}
		col = MAKEPALETTE(src[2], src[1], src[0]);
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		src += 3;
		dst += 3;
		if (last == col) {
			srcrem -= 1;
			if (srcrem < 0) {
				break;
			}
			cnt = *src++;
			if (cnt & 0x80) {
				srcrem -= 1;
				if (srcrem < 0) {
					break;
				}
				cnt &= 0x7f;
				cnt <<= 8;
				cnt |= *src++;
				if (cnt == 0x7fff) {
					srcrem -= 4;
					if (srcrem < 0) {
						break;
					}
					cnt = LOADINTELDWORD(src);
					src += 4;
				}
			}
			if (cnt > 2) {
				cnt = min(cnt - 2, (UINT)dstrem);
				dstrem -= cnt;
				while(cnt--) {
					*(dst+0) = *(dst-3);
					*(dst+1) = *(dst-2);
					*(dst+2) = *(dst-1);
					dst += 3;
				}
			}
		}
		last = col;
	}
}

static void solvegan24b(BYTE *dst, int dstrem, const BYTE *src, int srcrem) {

const BYTE	*head;
	int		headrem;
	UINT	leng;
	UINT	cnt;
	UINT32	col;
	UINT32	last;

	srcrem -= 4;
	if (srcrem < 0) {
		return;
	}
	headrem = LOADINTELDWORD(src);
	src += 4;
	head = src;
	headrem -= 4;
	if (headrem < 0) {
		return;
	}
	src += headrem;
	srcrem -= headrem;
	if (srcrem < 0) {
		return;
	}

	cnt = 0;
	last = 0xffffffff;

	while(1) {
		headrem -= 1;
		if (headrem < 0) {
			break;
		}
		leng = *head++;
		if (leng & 0x80) {
			headrem -= 1;
			if (headrem < 0) {
				break;
			}
			leng &= 0x7f;
			leng <<= 8;
			leng |= *head++;
			if (leng == 0x7fff) {
				headrem -= 4;
				if (headrem < 0) {
					break;
				}
				leng = LOADINTELDWORD(head);
				head += 4;
			}
		}
		while(leng--) {
			dstrem -= 1;
			if (dstrem < 0) {
				break;
			}
			if (!cnt) {
				srcrem -= 3;
				if (srcrem < 0) {
					goto g24b_done;
				}
				col = MAKEPALETTE(src[2], src[1], src[0]);
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				src += 3;
				dst += 3;
				if (last == col) {
					srcrem -= 1;
					if (srcrem < 0) {
						goto g24b_done;
					}
					cnt = *src++;
					if (cnt & 0x80) {
						srcrem -= 1;
						if (srcrem < 0) {
							goto g24b_done;
						}
						cnt &= 0x7f;
						cnt <<= 8;
						cnt |= *src++;
						if (cnt == 0x7fff) {
							srcrem -= 4;
							if (srcrem < 0) {
								goto g24b_done;
							}
							cnt = LOADINTELDWORD(src);
							src += 4;
						}
					}
					if (cnt > 2) {
						cnt -= 2;
					}
					else {
						cnt = 0;
					}
				}
				last = col;
			}
			else {
				cnt--;
				dst[0] = (BYTE)last;
				dst[1] = (BYTE)(last >> 8);
				dst[2] = (BYTE)(last >> 16);
				dst += 3;
			}
		}

		headrem -= 1;
		if (headrem < 0) {
			break;
		}
		leng = *head++;
		if (leng & 0x80) {
			headrem -= 1;
			if (headrem < 0) {
				break;
			}
			leng &= 0x7f;
			leng <<= 8;
			leng += *head++;
			if (leng == 0x7fff) {
				headrem -= 4;
				if (headrem < 0) {
					break;
				}
				leng = LOADINTELDWORD(head);
				head += 4;
			}
		}
		leng = min(leng, (UINT)dstrem);
		dstrem -= leng;
		dst += leng * 3;
	}

g24b_done:
	return;
}
#endif

BOOL cgload_gan(VRAMHDL vram, int ref, const BYTE *ptr, int size) {

	BYTE	*dst;
	int		dstrem;

	if (vram == NULL) {
		goto clgn_err;
	}
	dst = vram->ptr;
	dstrem = vram->width * vram->height;
#ifdef SUPPORT_16BPP
	if (vram->bpp == 16) {
		if (!ref) {
			solvegan16a(dst, dstrem, ptr, size);
		}
		else {
			solvegan16b(dst, dstrem, ptr, size);
		}
	}
#endif
#ifdef SUPPORT_24BPP
	if (vram->bpp == 24) {
		if (!ref) {
			solvegan24a(dst, dstrem, ptr, size);
		}
		else {
			solvegan24b(dst, dstrem, ptr, size);
		}
	}
#endif
	return(SUCCESS);

clgn_err:
	return(FAILURE);
}

