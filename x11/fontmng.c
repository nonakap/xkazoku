/*	$Id: fontmng.c,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

/* 
 * Tenshi no Hashigo for X
 * Original X11/xfont.c
 *    Copyright (c) 2001 TF <tf@denpa.org>
 * Copyright (c) 2001 TAJIRI Yasuhiro <tajiri@venus.dti.ne.jp>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by TF.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $angelladder-Id: sdlfont.c,v 1.9 2002/03/03 15:37:01 tf Exp $
 */

#include "compiler.h"
#include "fontmng.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include "codeconv.h"

#ifndef	FONTNAME_DEFAULT
#define	FONTNAME_DEFAULT	"./default.ttf"
#endif
char fontname[MAX_PATH] = FONTNAME_DEFAULT;

static BOOL fontmng_inited = FALSE;

typedef struct {
	int		size;
	UINT		type;
	int		width;
	int		height;

	int		align;
	TTF_Font	*ttf_font;
} _FNTMNG, *FNTMNG;

void *
fontmng_create(int size, UINT type, const TCHAR *fontface)
{
	FNTMNG ret;
	_FNTMNG fnt;
	int allocsize;

	UNUSED(fontface);

	if (size < 0) {
		size = -size;
	}
	if (size < 6) {
		size = 6;
	} else if (size > 128) {
		size = 128;
	}
	fnt.size = size;
	fnt.type = type;
	fnt.width = size + 1;
	fnt.height = size + 1;

	fnt.align = sizeof(_FNTDAT) + fnt.width * fnt.height;
	fnt.align = roundup(fnt.align, 4);

	allocsize = sizeof(fnt);
	allocsize += fnt.align;

	if (!fontmng_inited) {
		if (TTF_Init() < 0) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",
			    SDL_GetError());
			return NULL;
		}
		atexit(TTF_Quit);
		fontmng_inited = TRUE;
	}

	fnt.ttf_font = TTF_OpenFont(fontname, size);
	if (fnt.ttf_font == NULL) {
		fprintf(stderr, "Couldn't load %d points font from %s: %s\n",
		    size, fontname, SDL_GetError());
		return NULL;
	}

	ret = (FNTMNG)_MALLOC(allocsize, "font mng");
	if (ret == NULL)
		return NULL;
	bzero(ret, allocsize);

	memcpy(ret, &fnt, sizeof(fnt));
	return ret;
}

void
fontmng_destroy(void *hdl)
{

	if (hdl) {
		FNTMNG fnt = (FNTMNG)hdl;
		TTF_CloseFont(fnt->ttf_font);
		_MFREE(hdl);
	}
}

static UINT
getpixel(SDL_Surface *s, int x, int y)
{
	int bpp = s->format->BytesPerPixel;
	BYTE *p = (BYTE *)s->pixels + y * s->pitch + x * bpp;

	if (x < 0 || y < 0)
		return 0;

	switch (bpp) {
	case 1:
		return *p;

	case 2:
		return *(UINT16 *)p;

	case 3:
#if 0
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return (p[0] << 16) | (p[1] << 8) | p[2];
		else
			return p[0] | (p[1] << 8) | (p[2] << 16);
#else
		return p[0] || p[1] || p[2];
#endif

	case 4:
		return *(UINT *)p;

	default:
		return 0;	/* shouldn't happen, but avoids warnings */
	}
}

static unsigned int
han2zen(unsigned char h, unsigned char a)
{
	static unsigned int z[64] = {
		0x2121, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572, 0x2521,
		0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567, 0x2543,
		0x213c, 0x2522, 0x2524, 0x2526, 0x2528, 0x252a, 0x252b, 0x252d,
		0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253b, 0x253d,
		0x253f, 0x2541, 0x2544, 0x2546, 0x2548, 0x254a, 0x254b, 0x254c,
		0x254d, 0x254e, 0x254f, 0x2552, 0x2555, 0x2558, 0x255b, 0x255e,
		0x255f, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568, 0x2569,
		0x256a, 0x256b, 0x256c, 0x256d, 0x256f, 0x2573, 0x212b, 0x212c,
	};

	typedef struct {
		unsigned char	han;
		unsigned int	zen;
	} TBL;
	static TBL daku[] = {
		{0xb3, 0x2574}, {0xb6, 0x252c}, {0xb7, 0x252e}, {0xb8, 0x2530}, 
		{0xb9, 0x2532}, {0xba, 0x2534}, {0xbb, 0x2536}, {0xbc, 0x2538}, 
		{0xbd, 0x253a}, {0xbe, 0x253c}, {0xbf, 0x253e}, {0xc0, 0x2540}, 
		{0xc1, 0x2542}, {0xc2, 0x2545}, {0xc3, 0x2547}, {0xc4, 0x2549}, 
		{0xca, 0x2550}, {0xcb, 0x2553}, {0xcc, 0x2556}, {0xcd, 0x2559}, 
		{0xce, 0x255c},
		{0, 0}
	};
	static TBL handaku[] = {
		{0xca, 0x2551}, {0xcb, 0x2554}, {0xcc, 0x2557}, {0xcd, 0x255a}, 
		{0xce, 0x255d},
		{0, 0}
	};
	int i;

	/* 濁点 */
	if (a == 0xde) {
		for (i = 0; i < NELEMENTS(daku); ++i) {
			if (h == daku[i].han) {
				return daku[i].zen;
			}
		}
	}

	/* 半濁点 */
	if (a == 0xdf) {
		for (i = 0; i < NELEMENTS(handaku); ++i) {
			if (h == handaku[i].han) {
				return handaku[i].zen;
			}
		}
	}

	return z[h - 0xa0];
}
static void
getlength1(FNTMNG fnt, FNTDAT fdat, const unsigned char *str, BOOL draw)
{
	static SDL_Color white = { 0xff, 0xff, 0xff, 0 };
	SDL_Surface *text;
	int length;
	int xoff, yoff;
	UINT16 utext[2];
	UINT16 euc;

	/* とりあえず… */
	fdat->width = fnt->width;
	fdat->height = fnt->height;
	fdat->pitch = fnt->size;

	utext[1] = 0;
	if ((((str[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
		euc = codeconv_sjis_to_euc(((UINT16)str[0] << 8) | str[1]);
		utext[0] = codeconv_euc_to_unicode(euc);
		length = 2;
	} else if (str[0] >= 0xa1 && str[0] <= 0xdf) {
		euc = codeconv_jis_to_euc(han2zen(str[0], str[1]));
		utext[0] = codeconv_euc_to_unicode(euc);
		length = 2;
	} else if (str[0] != '\0') {
		euc = str[0];
		utext[0] = str[0];
		length = 1;
	} else 
		return;

	if (utext[0] == 0) {
		/* ローマ数字の1: utext[0] == 0, 0/1/euc = 87/54/adb5 */
		printf("utext == 0, 0/1/euc = %x/%x/%x\n", str[0],str[1],euc);
		printf("jis = %x\n", codeconv_euc_to_jis(euc));
		return;
	}
	text = TTF_RenderUNICODE_Solid(fnt->ttf_font, utext, white);
	if (text == NULL) {
		printf("text == NULL, 0/1/euc = %x/%x/%x\n", str[0],str[1],euc);
		return;
	}

	if (text->w < fnt->width) {
		fdat->width = text->w;
		xoff = 0;
	} else {
		fdat->width = fnt->width;
		xoff = (fdat->width - fnt->width) / 2;
	}
	if (text->h < fnt->height) {
		fdat->height = text->h;
		yoff = 0;
	} else {
		fdat->height = fnt->height;
		yoff = (fdat->height - fnt->height) / 2;
	}
	if (length < 2)
		fdat->pitch = (fnt->size + 1) / 2;

	if (draw) {
		BYTE *p;
		int h, w;

		bzero(fdat + 1, fdat->width * fdat->height);
		for (h = 0; h < fdat->height; h++) {
			p = ((BYTE *)(fdat + 1)) + h * fdat->width;
			for (w = 0; w < fdat->width; w++, p++) {
				if (getpixel(text, w + xoff, h + yoff))
					*p = 0xff;
			}
		}
	}

	SDL_FreeSurface(text);
}

BOOL
fontmng_getsize(void *hdl, const char *str, POINT_T *pt)
{
	FNTMNG fnt = (FNTMNG)hdl;
	_FNTDAT fdat;
	unsigned char buf[3];
	int width;
	int length;

	if (hdl == NULL || str == NULL)
		return FAILURE;

	width = 0;
	buf[2] = '\0';
	for (;;) {
		buf[0] = *str++;
		if ((((buf[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
			/* kanji(Shift-JIS) */
			buf[1] = *str++;
			if (buf[1] == '\0')
				break;
			length = 2;
		} else if (buf[0] >= 0xa1 && buf[0] <= 0xdf) {
			buf[1] = '\0';
			length = 2;
		} else if (buf[0] != '\0') {
			/* alphabet & number */
			buf[1] = '\0';
			length = 1;
		} else
			break;

		getlength1(fnt, &fdat, buf, FALSE);
		width += fdat.pitch;
	}

	if (pt) {
		pt->x = width;
		pt->y = fnt->size;
	}
	return SUCCESS;
}

BOOL
fontmng_getdrawsize(void *hdl, const char *str, POINT_T *pt)
{
	FNTMNG fnt = (FNTMNG)hdl;
	_FNTDAT	fdat;
	unsigned char buf[3];
	int width;
	int posx;
	int length;

	if (hdl == NULL || str == NULL)
		return FAILURE;

	width = 0;
	posx = 0;
	buf[2] = '\0';
	for (;;) {
		buf[0] = *str++;
		if ((((buf[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
			buf[1] = *str++;
			if (buf[1] == '\0')
				break;
		} else if (buf[0] >= 0xa1 && buf[0] <= 0xdf) {
			buf[1] = '\0';
		} else if (buf[0] != '\0') {
			buf[1] = '\0';
		} else
			break;

		getlength1(fnt, &fdat, buf, FALSE);
		width = posx + max(fdat.width, fdat.pitch);
		posx += fdat.pitch;
	}

	if (pt) {
		pt->x = width;
		pt->y = fnt->size;
	}
	return SUCCESS;
}

static void
fontmng_getchar(FNTMNG fnt, FNTDAT fdat, const unsigned char *str)
{

	getlength1(fnt, fdat, str, TRUE);
}

FNTDAT
fontmng_get(void *hdl, const char *str)
{
	FNTMNG fnt;
	FNTDAT fdat;

	if (hdl == NULL || str == NULL)
		return NULL;
	
	fnt = (FNTMNG)hdl;
	fdat = (FNTDAT)(fnt + 1);
	bzero(fdat + 1, fnt->align - sizeof(_FNTDAT));
	fontmng_getchar(fnt, fdat, (unsigned char *)str);
	return fdat;
}
