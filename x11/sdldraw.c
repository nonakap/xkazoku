/*	$Id: sdldraw.c,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

/*
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
 *      This product includes software developed by NONAKA Kimihiro.
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
 */

#include "compiler.h"

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "gamecore.h"
#include "x11.h"

#include <SDL.h>


int fullscreen_flag = 0;


/*
 * Video
 */
BOOL
xdraws_init(LONG width, LONG height)
{
	char s[256];
	const SDL_VideoInfo *vinfo;
	SDL_Surface *surface;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Error: SDL_Init: %s\n", SDL_GetError());
		return FAILURE;
	}
	SDL_WM_SetCaption(APP_NAME, APP_NAME);
	vinfo = SDL_GetVideoInfo();
	SDL_VideoDriverName(s, sizeof(s));

	win_rect.left = win_rect.top = 0;
	win_rect.right = width;
	win_rect.bottom = height;

	surface = SDL_SetVideoMode(width, height, vinfo->vfmt->BitsPerPixel,
	    SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF
	    | (fullscreen_flag ? SDL_FULLSCREEN : 0)
	    | (gamecore.gamecfg.fullscreen ? SDL_FULLSCREEN : 0)
	    );
	if (surface == 0) {
		fprintf(stderr, "Error: SDL_SetVideoMode: %s\n",SDL_GetError());
		return FAILURE;
	}

	switch (surface->format->BytesPerPixel) {
	case 4:
	case 3:
		/* Nothing to do */
		break;

	case 2:
		make16mask(surface->format->Bmask, surface->format->Rmask,
		    surface->format->Gmask);
		break;

	case 1:
	default:
		return FAILURE;
	}

	return SUCCESS;
}

void
xdraws_term(void)
{

	/* Nothing to do */
}

BOOL
xdraws_draws(void (*cb)(DEST_SURFACE *), const RECT_T *rct)
{
	DEST_SURFACE ds;
	SDL_Surface *surface = SDL_GetVideoSurface();
	SDL_LockSurface(surface);
	ds.ptr = (BYTE *)surface->pixels;
	ds.xalign = surface->format->BytesPerPixel;
	ds.yalign = ds.xalign * win_rect.right;
	ds.width = win_rect.right;
	ds.height = win_rect.bottom;
	ds.bit = ds.xalign * 8;
	ds.rct = rct;
	if (cb) {
		(*cb)(&ds);
	}
	SDL_UnlockSurface(surface);

	SDL_Flip(surface);

	return SUCCESS;
}
