#include	"compiler.h"
#include	"moviemng.h"

#ifdef DAMEDEC_LIB

#include	"gamecore.h"
#include	<damedec.h>
#include	"inputmng.h"
#include	"taskmng.h"
#include	"xdraws.h"
#include	"dosio.h"
#include	"sound.h"
#include	"sstream.h"
#include	"menubase.h"


#ifndef _WIN32_WCE
#define	WAIT_SOUND		((((512*4*4) * 1000) / 22050) + 0)
#endif

typedef struct {
	void	(*draw)(void *mv, DMV_SURFACE *ms);
	void	*mv;
} MVCB;

static const char *extnames[] = {".WAV", ".MP2", ".MP3"};

	BOOL	movieplaying = FALSE;


static void mvcb(DEST_SURFACE *ds) {

	DMV_SURFACE	ms;

	ms.ptr = ds->ptr;
	ms.xalign = ds->xalign;
	ms.yalign = ds->yalign;
	ms.width = ds->width;
	ms.height = ds->height;
	((MVCB *)(ds->arg))->draw(((MVCB *)(ds->arg))->mv, &ms);
}

static void mvdraw(void (*draw)(void *mv, DMV_SURFACE *ds), void *mv) {

	MVCB	arg;

	arg.draw = draw;
	arg.mv = mv;
	xdraws_draws(mvcb, &arg);
}

static int mvtask(int tick) {

	UINT	btn;

	if (!tick) {
		taskmng_rol();
		if (!taskmng_isavail()) {
			return(0);
		}
	}
	else {
		if (!taskmng_sleep(tick)) {
			return(0);
		}
	}
	btn = inputmng_getmouse(NULL, NULL);
	if (btn & (LBUTTON_UPBIT | RBUTTON_UPBIT)) {
		inputmng_resetmouse(0);
		return(0);
	}
	return(1);
}

static void *mvsndplay(const char *movie, BYTE *ptr, UINT size) {

	ONMEMSTMARG		arg;
	char			path[MAX_PATH];
	UINT			i;
	BOOL			play;

	play = FALSE;
	if (!gamecore.gamecfg.bgm) {
		goto mvsp_exit;
	}
	if ((ptr) && (size)) {
		arg.ptr = ptr;
		arg.size = size;
		soundmix_load(SOUNDTRK_SOUND, onmem_ssopen, &arg);
		play = TRUE;
	}
	if (!play) {
		for (i=0; i<(sizeof(extnames)/sizeof(char *)); i++) {
			milstr_ncpy(path, movie, sizeof(path));
			cutExtName(path);
			milstr_ncat(path, extnames[i], sizeof(path));
			if (!soundmix_load(SOUNDTRK_SOUND, mp3_ssopen, path)) {
				play = TRUE;
				break;
			}
		}
	}
	if (play) {
		soundmix_play(SOUNDTRK_SOUND, 0, 0);
#ifdef WAIT_SOUND
		{
			UINT32 base = GETTICK();
			while((GETTICK() - base) < WAIT_SOUND) {
				if (!taskmng_sleep(1)) {
					break;
				}
			}
		}
#endif
	}

mvsp_exit:
	return((void *)(long)play);
}

static void mvsndstop(void *hdl) {

	if (hdl) {
		soundmix_stop(SOUNDTRK_SOUND, 0);
		soundmix_unload(SOUNDTRK_SOUND);
	}
}


// ----

void moviemng_play(const char *fname, SCRN_T *scrn) {

	DAMEDEC	prm;
	char	path[MAX_PATH];
	char	*p;
	int		r;

	menubase_close();
	inputmng_resetmouse(0);

	milstr_ncpy(path, gamecore.suf.scriptpath, sizeof(path));
	milstr_ncat(path, fname, sizeof(path));
	cutExtName(path);
	milstr_ncat(path, ".MVD", sizeof(path));
	p = getFileName(path);
	while(*p) {
		if ((((p[0] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
			if (p[1] != '\0') {
				p++;
			}
		}
		else if (((p[0] - 0x41) & 0xdf) < 26) {
			p[0] &= 0xdf;
		}
		p++;
	}
	prm.movie = path;
	prm.decproc = DAMEDEC_NOR;
	prm.draw = mvdraw;
	prm.task = mvtask;
	prm.sndplay = mvsndplay;
	prm.sndstop = mvsndstop;
	movieplaying = TRUE;
	r = damedec_play(&prm);
	if (r < 0) {
		TRACEOUT(("damelib error: %d [%s]", r, path));
	}
	movieplaying = 0;

	(void)scrn;
}

#else

void moviemng_play(const char *fname, SCRN_T *scrn) {

	(void)fname;
	(void)scrn;
}

#endif

