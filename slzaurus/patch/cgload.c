#include	"compiler.h"
#include	"vram.h"
#include	"vrammix.h"
#include	"arcfile.h"
#include	"cgload.h"
#include	"taskmng.h"


// QVGAóp CGÉçÅ[Éh


#if defined(__GNUC__)
typedef struct {
	char	sig[4];
	BYTE	pos[4];
} __attribute__ ((packed)) GAD_HEAD;
typedef struct {
	char	sig[4];
	BYTE	width[2];
	BYTE	height[2];
	BYTE	datasize[4];
} __attribute__ ((packed)) PGG0_HEAD;
#else /* !__GNUC__ */
#pragma pack(push, 1)
typedef struct {
	char	sig[4];
	BYTE	pos[4];
} GAD_HEAD;
typedef struct {
	char	sig[4];
	BYTE	width[2];
	BYTE	height[2];
	BYTE	datasize[4];
} PGG0_HEAD;
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
	BOOL		remake;
	RECT_T		clr;

	ret = NULL;
	pt.x = width;
	pt.y = height;
	if ((pt.x <= 0) || (pt.y <= 0)) {
		goto clpp_exit;
	}

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

	ret = (CGLOAD)_MALLOC(sizeof(_CGLOAD) + CGLOADBUFFERS, "cgload");
	if (ret) {
		ZeroMemory(ret, sizeof(_CGLOAD) + CGLOADBUFFERS);
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


// ---- pgg0

static const int delta[3] = {0, 1, -1};

static BOOL pgg0_phase0(CGLOAD hdl) {					// èÄîıÅ`

	BYTE	*tmp;
	int		cnt;

	cnt = hdl->width * hdl->height * 3;
	tmp = (BYTE *)_MALLOC(cnt + 0x300, "pgg0 tmp");
	if (tmp == NULL) {
		return(FAILURE);
	}
	ZeroMemory(tmp, cnt);

	hdl->cnt = cnt;
	hdl->tmp = tmp;
	return(SUCCESS);
}

static BOOL pgg0_phase1(CGLOAD hdl) {					// âìÄÅ`

	BYTE	*tmp;
	BYTE	cmd;
	int		p;
	int		q;
	int		r;
	int		breakp;

	p = 0;
	breakp = 512 * 3;
	tmp = hdl->tmp;

	while(p < hdl->cnt) {
		if (hdl->rrem < 5) {
			cgload_preread(hdl);
		}

		hdl->rrem--;
		if (hdl->rrem < 0) {
			goto p0p1_done;
		}
		cmd = *hdl->rptr++;
		switch(cmd) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto p0p1_done;
				}
				q = *hdl->rptr++;
				if (cmd & 1) {
					hdl->rrem--;
					if (hdl->rrem < 0) {
						goto p0p1_done;
					}
					q |= (*hdl->rptr++) << 8;
				}
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto p0p1_done;
				}
				r = *hdl->rptr++;
				if (cmd & 2) {
					hdl->rrem--;
					if (hdl->rrem < 0) {
						goto p0p1_done;
					}
					r |= (*hdl->rptr++) << 8;
				}
				q++;
				q *= 3;
				r++;
				if (q <= p) {
					r = min(r * 3, hdl->cnt - p);
					p += r;
					do {
						*tmp = *(tmp - q);
						tmp++;
					} while(--r);
				}
				break;

			case 0x04:
			case 0x05:
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto p0p1_done;
				}
				r = *hdl->rptr++;
				if (cmd & 1) {
					hdl->rrem--;
					if (hdl->rrem < 0) {
						goto p0p1_done;
					}
					r |= (*hdl->rptr++) << 8;
				}
				r += 2;
				if (p >= 3) {
					r = min(r * 3, hdl->cnt - p);
					p += r;
					do {
						*tmp = *(tmp - 3);
						tmp++;
					} while(--r);
				}
				break;

			case 0x06:
				if (p) {
					*(tmp + 0) = *(tmp - 3);
					*(tmp + 1) = *(tmp - 2);
					*(tmp + 2) = *(tmp - 1);
					tmp += 3;
					p += 3;
				}
				break;

			case 0x07:
			case 0x08:
			case 0x09:
				goto p0p1_done;

			default:
				r = cmd - 0x09;
				r *= 3;
				if (cgload_read(hdl, tmp, r) != SUCCESS) {
					goto p0p1_done;
				}
				tmp += r;
				p += r;
				break;
		}
		if (p >= breakp) {
			breakp += 512 * 3;
			taskmng_rol();
		}
	}

p0p1_done:
	return(SUCCESS);
}

static BOOL pgg0_phase2(CGLOAD hdl) {					// ÉRÉsÅ`

	VRAMHDL	vram;
const BYTE	*src;
	BYTE	*dst;
	BYTE	*alpha;
	int		width;
	int		height;
	int		dststep;

	src = hdl->tmp;
	width = hdl->width;
	height = hdl->height;
	vram = hdl->vram;
	dst = vram->ptr;
	alpha = vram->alpha;
	dststep = vram->width - width;

	do {
		int r = width;
		do {
			dst[0] = src[0];
			dst[1] = src[1];
			*alpha++ = src[2];
			src += 3;
			dst += 2;
		} while(--r);
		dst += dststep * 2;
		alpha += dststep;
	} while(--height);
	return(SUCCESS);
}

static void pgg0_abort(CGLOAD hdl) {

	if (hdl->tmp) {
		_MFREE(hdl->tmp);
	}
}

static BOOL cgload_pgg0(VRAMHDL *vram, ARCFILEH afh) {

	PGG0_HEAD	pgg0head;
	CGLOAD		hdl;
	BOOL		r;

	if (arcfile_read(afh, &pgg0head, sizeof(pgg0head)) != sizeof(pgg0head)) {
		goto clpgg0_err;
	}
	if (memcmp(pgg0head.sig, "PGG0", 4)) {
		goto clpgg0_err;
	}
	hdl = cgload_prepare(vram, afh, LOADINTELWORD(pgg0head.width),
									LOADINTELWORD(pgg0head.height), 16);
	if (hdl == NULL) {
		goto clpgg0_err;
	}
	r = pgg0_phase0(hdl);
	if (r == SUCCESS) {
		r = pgg0_phase1(hdl);
	}
	if (r == SUCCESS) {
		r = pgg0_phase2(hdl);
	}
	pgg0_abort(hdl);
	_MFREE(hdl);
	return(r);

clpgg0_err:
	return(FAILURE);
}


static BOOL cgload_gad(VRAMHDL *vram, ARCFILEH afh) {

	GAD_HEAD	gadhead;
	long		pos;

	if (arcfile_read(afh, &gadhead, sizeof(gadhead)) != sizeof(gadhead)) {
		goto gadcre_err;
	}
	if (memcmp(gadhead.sig, "GAD ", 4)) {
		goto gadcre_err;
	}
	pos = LOADINTELDWORD(gadhead.pos);
	if (arcfile_seek(afh, pos, 0) != pos) {
		goto gadcre_err;
	}
	return(cgload_pgg0(vram, afh));

gadcre_err:
	return(FAILURE);
}


// ----

static BOOL gmask_phase0(CGLOAD hdl) {						// èÄîıÅ`

	hdl->cnt = hdl->width * hdl->height;
	return(SUCCESS);
}

static BOOL gmask_phase1(CGLOAD hdl) {						// âìÄÅ`

	BYTE	ctrl;
	int		cnt;
	int		p;
	int		breakp;
	int		leng;
	int		backword;
	BYTE	*dst;
	int		dstrem;
	int		zero;
	int		level;
	UINT	mask;
	UINT	tmp;

	p = 0;
	breakp = 512 * 2;
	dst = hdl->vram->ptr;
	dstrem = hdl->width * hdl->height;
	cgload_preread(hdl);
	hdl->rrem--;
	if (hdl->rrem < 0) {
		goto gmp1_done;
	}
	level = *hdl->rptr++;
	mask = (1 << level) - 1;

	while(dstrem) {
		if (hdl->rrem < 17) {
			cgload_preread(hdl);
		}
		hdl->rrem--;
		if (hdl->rrem < 0) {
			goto gmp1_done;
		}
		ctrl = *hdl->rptr++;

		cnt = 8;
		do {
			if (ctrl & 1) {
				hdl->rrem--;
				if (hdl->rrem < 0) {
					goto gmp1_done;
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
					goto gmp1_done;
				}
				tmp = *hdl->rptr++;
				tmp <<= 8;
				tmp |= *hdl->rptr++;
				backword = -1 - (tmp >> level);
				leng = (tmp & mask) + 1;
				leng = min(leng, dstrem);
				dstrem -= leng;
				zero = backword + p;
				p += leng;
				if (zero >= 0) {
					while(leng--) {
						*dst = *(dst + backword);
						dst++;
					}
				}
			}
			ctrl >>= 1;
		} while(--cnt);
		if (p >= breakp) {
			breakp += 512 * 2;
			taskmng_rol();
		}
	}

gmp1_done:
	return(SUCCESS);
}

static BOOL cgload_gmask(VRAMHDL *vram, ARCFILEH afh) {

	PGG0_HEAD	pgg0head;
	long		fpos;
	CGLOAD		hdl;
	BOOL		r;

	if (arcfile_read(afh, &pgg0head, sizeof(pgg0head)) != sizeof(pgg0head)) {
		goto clgm_err;
	}
	if (!memcmp(pgg0head.sig, "PGG0", 4)) {
		fpos = LOADINTELDWORD(pgg0head.datasize);
		fpos += sizeof(pgg0head);
		if ((arcfile_seek(afh, fpos, 0) != fpos) ||
			(arcfile_read(afh, &pgg0head, sizeof(pgg0head))
													!= sizeof(pgg0head))) {
			goto clgm_err;
		}
	}

	if (memcmp(pgg0head.sig, "PLZ0", 4)) {
		goto clgm_err;
	}
	hdl = cgload_prepare(vram, afh, LOADINTELWORD(pgg0head.width),
									LOADINTELWORD(pgg0head.height), 8);
	if (hdl == NULL) {
		goto clgm_err;
	}
	r = gmask_phase1(hdl);
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
	if ((cgtype == CGTYPE_GGD) || (cgtype == CGTYPE_GG0)) {
		ret = cgload_pgg0(vram, afh);
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

static void solvegan16a(BYTE *dst, int dstcnt, const BYTE *src, int srcrem) {

	int		cnt;
	UINT	col;
	UINT	last;

	last = 0xffffffff;
	while(dstcnt) {
		srcrem -= 2;
		if (srcrem < 0) {
			break;
		}
		col = LOADINTELWORD(src);
		src += 2;
		*(UINT16 *)dst = (UINT16)col;
		dst += 2;
		dstcnt--;
		if (last == col) {
			srcrem -= 1;
			if (srcrem < 0) {
				break;
			}
			cnt = *src++;
			cnt = min(cnt, dstcnt);
			dstcnt -= cnt;
			while(cnt > 0) {
				cnt--;
				*(UINT16 *)dst = (UINT16)col;
				dst += 2;
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
				srcrem -= 2;
				if (srcrem < 0) {
					break;
				}
				col = LOADINTELWORD(src);
				src += 2;
				*(UINT16 *)dst = (UINT16)col;
				dst += 2;
				if (last == col) {
					srcrem -= 1;
					if (srcrem < 0) {
						break;
					}
					cnt = *src++;
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

BOOL cgload_gan(VRAMHDL vram, int ref, const BYTE *ptr, int size) {

	if (vram == NULL) {
		goto clgn_err;
	}
	if (vram->bpp == 16) {
		if (!ref) {
			solvegan16a(vram->ptr, vram->width * vram->height, ptr, size);
		}
		else {
			solvegan16b(vram->ptr, vram->width * vram->height, ptr, size);
		}
	}
	return(SUCCESS);

clgn_err:
	return(FAILURE);
}

