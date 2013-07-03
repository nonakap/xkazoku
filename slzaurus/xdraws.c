#include	"compiler.h"
#include	"xdraws.h"
// #include	<sys/time.h>
// #include	<signal.h>
// #include	<unistd.h>
#include	"gamecore.h"


#define	APP_NAME	"xkazoku"
static	RECT_T		win_rect;


BOOL xdraws_init(int width, int height) {

	char			s[256];
const SDL_VideoInfo	*vinfo;
	SDL_Surface		*surface;
	SDL_PixelFormat	*fmt;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Error: SDL_Init: %s\n", SDL_GetError());
		return(FAILURE);
	}
	SDL_WM_SetCaption(APP_NAME, APP_NAME);
	vinfo = SDL_GetVideoInfo();
	if (vinfo == NULL) {
		fprintf(stderr, "Error: SDL_GetVideoInfo: %s\n", SDL_GetError());
		return(FAILURE);
	}
	SDL_VideoDriverName(s, sizeof(s));

	win_rect.left = 0;
	win_rect.top = 0;
	win_rect.right = width;
	win_rect.bottom = height;

	surface = SDL_SetVideoMode(width, height, vinfo->vfmt->BitsPerPixel,
		    SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	if (surface == NULL) {
		fprintf(stderr, "Error: SDL_SetVideoMode: %s\n", SDL_GetError());
		return(FAILURE);
	}

	fmt = surface->format;
	if ((fmt->BitsPerPixel != 16) || (fmt->Rmask != 0xf800) ||
		(fmt->Gmask != 0x07e0) || (fmt->Bmask != 0x001f)) {
		fprintf(stderr, "Error: Support Only 16bit[565]\n" \
						"- bpp=%d R:%x G:%x B:%x\n", fmt->BytesPerPixel,
										fmt->Rmask, fmt->Gmask, fmt->Bmask);
		return(FAILURE);
	}
	return(SUCCESS);
}

void xdraws_term(void) {
}

BOOL xdraws_draws(void (*cb)(DEST_SURFACE *), const void *arg) {

	DEST_SURFACE	ds;
	SDL_Surface		*surface;

	surface = SDL_GetVideoSurface();
	SDL_LockSurface(surface);
	ds.ptr = (BYTE *)surface->pixels;
	ds.xalign = surface->format->BytesPerPixel;
	ds.yalign = surface->pitch;
	ds.width = win_rect.right;
	ds.height = win_rect.bottom;
	ds.bit = ds.xalign * 8;
	ds.arg = arg;
	if (cb) {
		(*cb)(&ds);
	}
	SDL_UnlockSurface(surface);
	SDL_Flip(surface);
	return(SUCCESS);
}

