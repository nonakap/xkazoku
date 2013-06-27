#include	"compiler.h"
#include	"dosio.h"
#include	"cddamng.h"
#include	"fontmng.h"
#include	"scrnmng.h"
#include	"taskmng.h"
#include	"timemng.h"
#include	"gamemsg.h"
#include	"gamecore.h"
#include	"sound.h"
#include	"arcfile.h"
#include	"cgload.h"
#include	"savefile.h"
#include	"savedrs.h"
#include	"sstream.h"
#ifndef DISABLE_DIALOG
#include	"inputmng.h"
#include	"menubase.h"
#include	"sysmenu.h"
#endif


	GAMECORE	gamecore;


static void readsuf(const char *path) {

	SYS_T	*sys;
	SUF_T	*suf;
	char	subpath[MAX_PATH];

	sys = &gamecore.sys;
	suf = &gamecore.suf;
	gamedef_loadsuf(path, suf, sys);
	if (archive_throwall(suf->scriptpath) != SUCCESS) {
		milstr_ncpy(subpath, suf->scriptpath, sizeof(subpath));
		plusyen(subpath, sizeof(subpath));
		milstr_ncat(subpath, suf->key, sizeof(subpath));
		archive_throwall(subpath);
	}
}

BOOL gamecore_init(const char *path) {

const char	*e;

	e = NULL;
	ZeroMemory(&gamecore, sizeof(gamecore));
	if (path == NULL) {
		e = "no suf file (NULL)";
		goto gcinit_err;
	}

	gamedef_create();

	if (archive_create()) {
		e = "couldn't read archives";
		goto gcinit_err;
	}
	readsuf(path);
	gamecfg_init();
	gamecfg_reset();
	gamecfg_load();
	gamecore.cfglist = listarray_new(sizeof(_GCDLG), 32);

	if (vramdraw_create() != SUCCESS) {
		e = "couldn't create vram";
		goto gcinit_err;
	}

	if (scr_init(gamecore.sys.defbits, gamecore.sys.defflags) != SUCCESS) {
		e = "couldn't create flags";
		goto gcinit_err;
	}

	sndplay_init();
	gamecore.dispwin.fontsize = 16;
	gamecore.dispwin.fonttype = TEXTCTRL_BOLD |
					((gamecore.sys.type & GAME_TEXTASCII)?TEXTCTRL_ASCII:0);
	textctrl_init(&gamecore.textdraw);
	textwin_create();
	event_mousereset();

	if (gamecore.sys.type & GAME_DRS) {
		savedrs_sysread();
	}

	if (scr_scriptcall("START")) {
		e = "couldn't reat start script";
		goto gcinit_err;
	}

	gamecore.initialized = TRUE;
	return(SUCCESS);

gcinit_err:
	gamecore.err = e;
	return(FAILURE);
}

void gamecore_resetvolume(void) {

	GAMECFG	gc;
	int		i;

	gc = &gamecore.gamecfg;
	soundmix_setgain(SOUNDTRK_SOUND, gc->bgmvol);
	soundmix_setgain(SOUNDTRK_VOICE, gc->voicevol);
	for (i=0; i<SOUNDTRK_MAXSE; i++) {
		soundmix_setgain(SOUNDTRK_SE + i, gc->sevol);
	}
}

void gamecore_reset(void) {

	_SYSTIME	st;

	vram_destroy(gamecore.mouseevt.map);
	gamecore.mouseevt.map = NULL;
	textdisp_multireset();
	effect_trush();
	textwin_allclose();
	anime_alltrush();

	timemng_gettime(&st);
	srand(((st.day[0] << 24) | (st.hour[0] << 16) |
			(st.minute[0] << 8) | st.second[0]));

	gamecore.event = GAMEEV_SUCCESS;
}

void gamecore_destroy(void) {

	if (gamecore.initialized) {
#if 0
		if (gamecore.sys.type & GAME_DRS) {
			savedrs_syswrite();
		}
#endif
		gamecfg_save();
	}
	gamecore_reset();
	cddamng_stop(0);
	scr_term();
	textdisp_multireset();
	textctrl_trash(&gamecore.textdraw);
	vramdraw_destroy();
	textwin_destroy();
	anime_alltrush();
	listarray_destroy(gamecore.drssys.cgflag);
	listarray_destroy(gamecore.cfglist);
	archive_destory();

	gamedef_destroy();
}


// ----

typedef struct {
	int		errcode;
const char	*str;
} CMDERROR;

static const BYTE strerr100[] = {		// スクリプト終端エラーです
		0x83,0x58,0x83,0x4e,0x83,0x8a,0x83,0x76,0x83,0x67,0x8f,0x49,0x92,0x5b,
		0x83,0x47,0x83,0x89,0x81,0x5b,0x82,0xc5,0x82,0xb7,0};
static const BYTE strerr101[] = {		// コマンド長エラー
		0x83,0x52,0x83,0x7d,0x83,0x93,0x83,0x68,0x92,0xb7,0x83,0x47,0x83,0x89,
		0x81,0x5b,0};
static const BYTE strerr102[] = {		// サポートしていない命令です
		0x83,0x54,0x83,0x7c,0x81,0x5b,0x83,0x67,0x82,0xb5,0x82,0xc4,0x82,0xa2,
		0x82,0xc8,0x82,0xa2,0x96,0xbd,0x97,0xdf,0x82,0xc5,0x82,0xb7,0};
static const BYTE strerr103[] = {		// パラメータ長が間違っています
		0x83,0x70,0x83,0x89,0x83,0x81,0x81,0x5b,0x83,0x5e,0x92,0xb7,0x82,0xaa,
		0x8a,0xd4,0x88,0xe1,0x82,0xc1,0x82,0xc4,0x82,0xa2,0x82,0xdc,0x82,0xb7,
		0};
static const BYTE strerr104[] = {		// 未対応のパラメータを使用しました
		0x96,0xa2,0x91,0xce,0x89,0x9e,0x82,0xcc,0x83,0x70,0x83,0x89,0x83,0x81,
		0x81,0x5b,0x83,0x5e,0x82,0xf0,0x8e,0x67,0x97,0x70,0x82,0xb5,0x82,0xdc,
		0x82,0xb5,0x82,0xbd,0};
static const BYTE strerr110[] = {		// メモリエラーです
		0x83,0x81,0x83,0x82,0x83,0x8a,0x83,0x47,0x83,0x89,0x81,0x5b,0x82,
		0xc5,0x82,0xb7,0};
static const BYTE strerr111[] = {		// 既に初期化されています
		0x8a,0xf9,0x82,0xc9,0x8f,0x89,0x8a,0xfa,0x89,0xbb,0x82,0xb3,0x82,0xea,
		0x82,0xc4,0x82,0xa2,0x82,0xdc,0x82,0xb7,0};
static const BYTE strerr112[] = {		// スクリプトファイルが見つかりません
		0x83,0x58,0x83,0x4e,0x83,0x8a,0x83,0x76,0x83,0x67,0x83,0x74,0x83,0x40,
		0x83,0x43,0x83,0x8b,0x82,0xaa,0x8c,0xa9,0x82,0xc2,0x82,0xa9,0x82,0xe8,
		0x82,0xdc,0x82,0xb9,0x82,0xf1,0};
static const BYTE strerr200[] = {		// スクリプト処理で失敗しました
		0x83,0x58,0x83,0x4e,0x83,0x8a,0x83,0x76,0x83,0x67,0x8f,0x88,0x97,0x9d,
		0x82,0xc5,0x8e,0xb8,0x94,0x73,0x82,0xb5,0x82,0xdc,0x82,0xb5,0x82,0xbd,
		0};
static const BYTE strerr002[] = {		// システムエラーです
		0x83,0x56,0x83,0x58,0x83,0x65,0x83,0x80,0x83,0x47,0x83,0x89,0x81,0x5b,
		0x82,0xc5,0x82,0xb7,0};

static const CMDERROR cmderr[] = {
	{GAMEEV_EOS,			(char *)strerr100},
	{GAMEEV_WRONGCMDLENG,	(char *)strerr101},
	{GAMEEV_WRONGCMD,		(char *)strerr102},
	{GAMEEV_WRONGLENG,		(char *)strerr103},
	{GAMEEV_WRONGPARAM,		(char *)strerr104},
	{GAMEEV_MEMORYERROR,	(char *)strerr110},
	{GAMEEV_ALREADYVAL,		(char *)strerr111},
	{GAMEEV_FILENOTFOUND,	(char *)strerr112},
	{GAMEEV_FAILURE,		(char *)strerr200},
	{GAMEEV_ERROR,			(char *)strerr002}};

const char *gamecore_seterrorevent(int event) {

const CMDERROR	*e;
	int			i;

	e = cmderr;
	for (i=0; i<(int)(sizeof(cmderr)/sizeof(CMDERROR)); i++) {
		if (e->errcode == event) {
			gamecore.err = e->str;
			return(e->str);
			break;
		}
		e++;
	}
	return(NULL);
}

const char *gamecore_geterror(void) {

const char	*ret;

	ret = gamecore.err;
	if (ret == NULL) {
		ret = "no error";
	}
	return(ret);
}

BOOL gamecore_proc(void) {

	int		event;

#ifndef DISABLE_DIALOG
	if (gamecore.gamecfg.enablepopup) {
		int x, y;
		UINT btn;
		UINT key;
		BOOL r;
		btn = inputmng_getmouse(&x, &y);
		key = inputmng_getkey();
		if (btn & RBUTTON_DOWNBIT) {
			r = sysmenu_menuopen(TRUE, x, y);
		}
		else if (key & KEY_MENU) {
			r = sysmenu_menuopen(TRUE, 0, 0);
		}
		else {
			r = FAILURE;
		}
		if (r == SUCCESS) {
			inputmng_resetmouse(0);
			inputmng_resetkey(0);
			menubase_modalproc();
			event = gamecore.event;
			if ((event >= GAMEEV_SUCCESS) && (event < GAMEEV_FADEIN)) {
				vramdraw_setrect(mainvram, NULL);
				vramdraw_draw();
			}
		}
	}
	if (menuvram != NULL) {
		menubase_proc();
	}
#endif

	event = gamecore.event;
	if ((event >= GAMEEV_SUCCESS) && (event < GAMEEV_FADEIN)) {
		anime_exec();
	}
	switch(event) {
		case GAMEEV_SUCCESS:
			event = script_dispatch();
			break;

		case GAMEEV_TEXTOUT:
			event = textdisp_exec();
			break;

		case GAMEEV_WAITMOUSE:
			event = event_mouse();
			break;

		case GAMEEV_CMDWIN:
			event = event_cmdwin();
			break;

		case GAMEEV_WAITPCMEND:
			event = event_waitpcmend();
			break;

		case GAMEEV_MSGCLK:
			event = event_msgclk();
			break;

		case GAMEEV_IRCLK:
			event = event_irclk();
			break;

		case GAMEEV_WAIT:
			event = event_wait();
			break;

		case GAMEEV_FADEIN:
			event = effect_fadeinexec();
			break;

		case GAMEEV_FADEOUT:
			event = effect_fadeoutexec();
			break;

		case GAMEEV_EFFECT:
			event = effect_exec();
			break;

		case GAMEEV_GRAYSCALE:
			event = effect_grayscale();
			break;

		case GAMEEV_SCROLL:
			event = effect_scroll();
			break;

		case GAMEEV_QUAKE:
			event = effect_quake();
			break;

		case GAMEEV_FORCE:
			event = GAMEEV_SUCCESS;
			break;

		case GAMEEV_EXIT:
			return(FAILURE);
	}

	gamecore.event = event;
	if (event < 0) {
		const char *e;
		e = gamecore_seterrorevent(event);
#ifndef DISABLE_DIALOG
		if (e) {
			menumbox(e, gamecore.suf.key, MBOX_ICONSTOP | MBOX_OK);
			return(FAILURE);
		}
#endif
		TRACEOUT(("error: %d", event));
		TRACEOUT(("system terminate."));
	}
	return(SUCCESS);
}

BOOL gamecore_exec(void) {

	gamecore_resetvolume();
	gamecore_reset();

	do {
		if (gamecore_proc()) {
			break;
		}
		taskmng_rol();
	} while(taskmng_isavail());
	return(SUCCESS);
}

