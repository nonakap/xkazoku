#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"taskmng.h"
#include	"scrnmng.h"
#include	"fontmng.h"
#include	"dosio.h"
#include	"cgload.h"
#include	"cddamng.h"
#include	"gamemsg.h"
#include	"sound.h"
#include	"sstream.h"
#include	"timemng.h"
#ifndef DISABLE_DIALOG
#include	"menubase.h"
#include	"sysmenu.h"
#include	"inputmng.h"
#endif


	GAMECORE	gamecore;


typedef struct {
const char	*company;
const char	*key;
	int		exever;
	UINT	gametype;
} GAMETBL;


// ゲームシステムを合わせるには ここで調整…

static const GAMETBL gametbl[] = {
	{NULL,		"KANA",		EXEVER_KANA,	GAME_HAVEALPHA},
	{"Purple",	NULL,		EXEVER_MYU,		GAME_SAVEMYU | GAME_VOICE},

	{NULL,		"TEA2SMPL",	EXEVER_TEA2DEMO,GAME_SAVEMAX30 | GAME_SAVECOM |
				GAME_SAVESYS | GAME_SAVEGRPH | GAME_VOICE | GAME_VOICEONLY},
	{NULL,		"TEA2DEMO",	EXEVER_TEA2DEMO,GAME_SAVEMAX30 | GAME_SAVECOM |
				GAME_SAVESYS | GAME_SAVEGRPH | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"CRES",		EXEVER_CRES,	GAME_SAVEMAX30 | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_SAVEGRPH | GAME_HAVEALPHA},

	{NULL,		"PLANET",	EXEVER_PLANET,	GAME_SAVEMAX27 | GAME_SAVECOM},
	{NULL,		"PLANDVD",	EXEVER_PLANET,	GAME_SAVEMAX27 | GAME_SAVECOM |
																GAME_SVGA},

	{NULL,		"KAZOKU",	EXEVER_KAZOKU,	GAME_SAVEMAX27 | GAME_SAVECOM},
	{NULL,		"KAZOKUK",	EXEVER_KAZOKUK,	GAME_SAVEMAX27 | GAME_SAVECOM |
																GAME_VOICE},
	{NULL,		"KAZOKUKU",	EXEVER_KAZOKUK,	GAME_SAVEMAX27 | GAME_SAVECOM |
																GAME_VOICE},

	{NULL,		"KONYA",	EXEVER_KONYA,	GAME_SAVEMAX27 | GAME_SAVECOM |
												GAME_VOICE | GAME_VOICEONLY},
	{NULL,		"KONYA2",	EXEVER_KONYA2,	GAME_SAVEMAX30 | GAME_SAVECOM |
								GAME_TEXTASCII | GAME_SAVEGRPH | GAME_VOICE},

	{NULL,		"PrivateNurse",
							EXEVER_NURSE,	GAME_SAVEMAX27 | GAME_VOICE},
	{NULL,		"SISKON",	EXEVER_SISKON,	GAME_SAVEMAX30 | GAME_SAVECOM |
																GAME_VOICE},
	{NULL,		"HEART",	EXEVER_HEART,	GAME_SAVEMAX30 | GAME_SAVECOM |
								GAME_TEXTASCII | GAME_SAVEGRPH | GAME_VOICE},

	{NULL,		"DM",		EXEVER_DM,		GAME_SAVEMAX30 | GAME_SAVECOM |
								GAME_TEXTASCII | GAME_SAVEGRPH | GAME_VOICE},

	{NULL,		"MOEKKO",	EXEVER_MOEKKO,	GAME_SAVEMAX50 | GAME_SAVECOM |
								GAME_TEXTASCII | GAME_SAVEGRPH | GAME_VOICE},

	{NULL,		"fuku",		EXEVER_KONYA,	GAME_SAVEMAX30 | GAME_SAVECOM |
								GAME_TEXTASCII |
								GAME_SAVEGRPH | GAME_VOICE | GAME_VOICEONLY},
	{NULL,		"SAMOV",	EXEVER_DM,		GAME_SAVEMAX30 | GAME_SAVECOM |
								GAME_TEXTASCII | GAME_SAVEGRPH | GAME_VOICE},

	{"TRABULANCE", NULL,	EXEVER_KONYA,	GAME_SAVEMAX27 | GAME_SAVECOM |
												GAME_VOICE | GAME_VOICEONLY},

	{NULL,		NULL,		EXEVER_PLANET,	GAME_SAVEMAX27 | GAME_SAVECOM}
};


static BOOL sufload(void *arg, const char *para,
									const char *key, const char *data) {

	SUF_T	*suf;

	suf = (SUF_T *)arg;
	if (!milstr_cmp(para, "StartUpInfo")) {
		if (!milstr_cmp(key, "COMPANY")) {
			milstr_ncpy(suf->company, data, sizeof(suf->company));
		}
		else if (!milstr_cmp(key, "TITLE")) {
			milstr_ncpy(suf->title, data, sizeof(suf->title));
		}
		else if (!milstr_cmp(key, "KEY")) {
			// とらぶーのゲームには「KEY=SILKDVD\SILK_AD」という
			// のがあるので '\' 以前は無視させる… (NONAKA.K)
			// 漢字だった場合も一応考慮 (NONAKA.K)
			// どーせなら ':' '/'も対応しまぷ (T.Yui)
			char c;
			const char *p;
			p = data;
			do {
				c = *data++;
				if ((((c ^ 0x20) - 0xa1) & 0xff) >= 0x3c) {
					if ((c == '\\') || (c == '/') || (c == ':')) {
						p = data;
					}
				}
				else if (data[0]) {
					data++;
				}
			} while(c);
			milstr_ncpy(suf->key, p, sizeof(suf->key));
		}
	}
	return(FALSE);
}


static void readsuf(const char *path) {

	SYS_T		*sys;
	SUF_T		*suf;
const GAMETBL	*tbl;
	UINT		cnt;

	sys = &gamecore.sys;
	suf = &gamecore.suf;
	ZeroMemory(suf, sizeof(SUF_T));
	if (path) {
		profile_enum(path, suf, sufload);
		milstr_ncpy(suf->scriptpath, path, sizeof(suf->scriptpath));
		cutFileName(suf->scriptpath);
	}

	TRACEOUT(("COMPANY: %s", suf->company));
	TRACEOUT(("TITLE: %s", suf->title));
	TRACEOUT(("KEY: %s", suf->key));

	tbl = gametbl;
	cnt = sizeof(gametbl) / sizeof(GAMETBL);
	while(cnt--) {
		if (((tbl->company == NULL) ||
				(!milstr_cmp(tbl->company, suf->company))) &&
			((tbl->key == NULL) || (!milstr_cmp(tbl->key, suf->key)))) {
			sys->version = tbl->exever;
			sys->type = tbl->gametype;
			break;
		}
		tbl++;
	}
	if (sys->type & GAME_SVGA) {
		sys->width = 800;
		sys->height = 600;
	}
	else {
		sys->width = 640;
		sys->height = 480;
	}
	archive_throwall(suf->scriptpath);
}


BOOL gamecore_init(const char *path) {

	ZeroMemory(&gamecore, sizeof(gamecore));
	if (archive_create()) {
		return(FAILURE);
	}
	readsuf(path);
	gamecfg_init();
	gamecfg_load();
	gamecore.cfglist = listarray_new(sizeof(_GCDLG), 32);

	if (vramdraw_create() != SUCCESS) {
		return(FAILURE);
	}

	if (scr_init()) {
		TRACEOUT(("scr_init fail"));
		return(FAILURE);
	}

	sndplay_init();
	textctrl_init(&gamecore.textdraw, TRUE);
	textwin_create();
	event_mousereset();

	return(scr_scriptcall("START"));
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
	textwin_cmdclose(0);
	textwin_close(0);

	timemng_gettime(&st);
	srand(((st.day[0] << 24) | (st.hour[0] << 16) |
			(st.minute[0] << 8) | st.second[0]));

	gamecore.event = GAMEEV_SUCCESS;
}


void gamecore_destroy(void) {

	gamecfg_save();

	gamecore_reset();
	cddamng_stop(0);
	scr_term();
	textdisp_multireset();
	textctrl_trash(&gamecore.textdraw);
	vramdraw_destroy();
	textwin_destroy();
	listarray_destroy(gamecore.cfglist);
	archive_destory();
}


// ----

typedef struct {
	int		errcode;
	char	*str;
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


BOOL gamecore_proc(void) {

	int		event;

#ifndef DISABLE_DIALOG
	if (gamecore.gamecfg.enablepopup) {
		UINT btn;
		btn = inputmng_getmouse(NULL, NULL);
		if (btn & RBUTTON_DOWNBIT) {
			sysmenu_menuopen();
			menubase_modalproc();
			vramdraw_setrect(mainvram, NULL);
			vramdraw_draw();
		}
	}
#endif

	event = gamecore.event;
	switch(event) {
		case GAMEEV_SUCCESS:
			anime_exec();
			event = script_dispatch();
			break;

		case GAMEEV_TEXTOUT:
			event = textdisp_exec();
			break;

		case GAMEEV_WAITMOUSE:
			event = event_mouse();
			break;

		case GAMEEV_EFFECT:
			event = effect_exec();
			break;

		case GAMEEV_FADEOUT:
			event = effect_fadeoutexec();
			break;

		case GAMEEV_CMDWIN:
			event = event_cmdwin();
			break;

		case GAMEEV_GRAYSCALE:
			event = effect_grayscale();
			break;

		case GAMEEV_SCROLL:
			event = effect_scroll();
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

		case GAMEEV_FORCE:
			event = GAMEEV_SUCCESS;
			break;

		case GAMEEV_EXIT:
			return(FAILURE);
	}

	gamecore.event = event;
	if (event < 0) {
		const CMDERROR *e;
		int i;
		TRACEOUT(("error: %d", event));
		TRACEOUT(("system terminate."));
		e = cmderr;
		for (i=0; i<(int)(sizeof(cmderr)/sizeof(CMDERROR)); i++) {
			if (e->errcode == event) {
				menumbox(e->str, gamecore.suf.key, MBOX_ICONSTOP | MBOX_OK);
				return(FAILURE);
			}
			e++;
		}
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

