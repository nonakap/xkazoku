/*	$Id: x11.h,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#ifndef	__AKIRA_X11_H__
#define	__AKIRA_X11_H__

#define	APP_NAME	"xkazoku"

#ifndef SIZE_QVGA
#define	SCREEN_WIDTH	gamecore.sys.width
#define	SCREEN_HEIGHT	gamecore.sys.height
#define	AUDIO_RATE	44100
#else
#define	SCREEN_WIDTH	320
#define	SCREEN_HEIGHT	240
#define	AUDIO_RATE	22050
#endif

typedef union {
	struct {
		BYTE	r;
		BYTE	g;
		BYTE	b;
		BYTE	f;
	} p;
	BYTE		b[4];
	UINT		d;
} PALETTE_TABLE;

typedef struct {
	BYTE		*ptr;
	int		xalign;
	int		yalign;
	int		width;
	int		height;
	int		bit;
	const RECT_T	*rct;
} DEST_SURFACE;

#ifdef	__cplusplus
extern "C" {
#endif

extern int fullscreen_flag;
extern RECT_T win_rect;

typedef void sigfunc(int);
sigfunc *setup_signal(int, sigfunc *);
void sighandler(int);

int gui_init(int *, char ***);
int gui_main(void);

BOOL xdraws_init(LONG, LONG);
void xdraws_term(void);
BOOL xdraws_draws(void (*cb)(DEST_SURFACE *), const RECT_T *);
void make16mask(UINT bmask, UINT rmask, UINT gmask);

int sdlcdda_setup(void);

extern UINT audio_rate;

#ifdef	__cplusplus
};
#endif

#endif /* __AKIRA_X11_H__ */
