#include	"compiler.h"
#include	<stddef.h>
#include	"gamecore.h"
#include	"gamemsg.h"
#include	"dosio.h"


enum {
	TYPE_INT,
	TYPE_UINT32
};

typedef struct {
const char	*key;
	int		type;
	int		ptr;
} GAMEINI;

static const GAMEINI gameini[] = {
			{"MESSAGE",		TYPE_INT,	offsetof(GAMECFG_T, msgtype)},
			{"MEGTICK",		TYPE_INT,	offsetof(GAMECFG_T, textwaittick)},
			{"BGM",			TYPE_INT,	offsetof(GAMECFG_T, bgm)},
			{"BGMVOL",		TYPE_INT,	offsetof(GAMECFG_T, bgmvol)},
			{"SE",			TYPE_INT,	offsetof(GAMECFG_T, se)},
			{"SEVOL",		TYPE_INT,	offsetof(GAMECFG_T, sevol)},
			{"VOICE",		TYPE_INT,	offsetof(GAMECFG_T, voice)},
			{"VOICEVOL",	TYPE_INT,	offsetof(GAMECFG_T, voicevol)},
			{"WINRGB",		TYPE_UINT32,offsetof(GAMECFG_T, winrgb)},
			{"WINALPHA",	TYPE_INT,	offsetof(GAMECFG_T, winalpha)},
			{"FULLSCREEN",	TYPE_INT,	offsetof(GAMECFG_T, fullscreen)}};

static BOOL iniread(void *arg, const char *para,
									const char *key, const char *data) {

const GAMEINI	*ini;
	int			cnt;
	BYTE		*ptr;

	ini = gameini;
	cnt = sizeof(gameini)/sizeof(GAMEINI);
	while(cnt--) {
		if (!milstr_cmp(key, ini->key)) {
			ptr = (BYTE *)arg + ini->ptr;
			switch(ini->type) {
				case TYPE_INT:
					*(int *)ptr = milstr_solveINT(data);
					break;

				case TYPE_UINT32:
					*(UINT32 *)ptr = milstr_solveHEX(data);
					break;
			}
			break;
		}
		ini++;
	}
	(void)para;
	return(FALSE);
}


static void iniwrite(const char *path, void *arg) {

	FILEH		fh;
const GAMEINI	*ini;
	int			cnt;
	BYTE		*ptr;
	char		work[64];

	fh = file_create(path);
	if (fh != FILEH_INVALID) {
		ini = gameini;
		cnt = sizeof(gameini)/sizeof(GAMEINI);
		while(cnt--) {
			ptr = (BYTE *)arg + ini->ptr;
			switch(ini->type) {
				case TYPE_INT:
					sprintf(work, "%s = %d\r\n", ini->key, *(int *)ptr);
					file_write(fh, work, strlen(work));
					break;

				case TYPE_UINT32:
					sprintf(work, "%s = %x\r\n", ini->key, *(UINT32 *)ptr);
					file_write(fh, work, strlen(work));
					break;
			}
			ini++;
		}
		file_close(fh);
	}
}


// ----

static void gamecfg_getname(char *path, int leng) {

	SUF_T	*suf;

	suf = &gamecore.suf;
	milstr_ncpy(path, suf->scriptpath, leng);
	milstr_ncat(path, suf->key, leng);
	milstr_ncat(path, ".ini", leng);
}


// ----

void gamecfg_init(void) {

	GAMECFG	gamecfg;

	gamecfg = &gamecore.gamecfg;
	gamecfg->msgtype = (gamecore.sys.type & GAME_VOICE)?2:1;
	gamecfg->textwaittick = 25;
	gamecfg->bgm = 1;
	gamecfg->bgmvol = 0;
	gamecfg->se = 1;
	gamecfg->sevol = 0;
	gamecfg->voice = 1;
	gamecfg->voicevol = 0;
	gamecfg->winrgb = 0;
	gamecfg->winalpha = 32;
}

void gamecfg_reset(void) {

	GAMECFG	gamecfg;

	gamecfg = &gamecore.gamecfg;
	gamecfg->skip = 0;
	gamecfg->readskip = 0;
	gamecfg->autoclick = 0;
}

void gamecfg_load(void) {

	char		path[MAX_PATH];

	gamecfg_getname(path, sizeof(path));
	profile_enum(path, &gamecore.gamecfg, iniread);
}

void gamecfg_save(void) {

	char		path[MAX_PATH];

	gamecfg_getname(path, sizeof(path));
	iniwrite(path, &gamecore.gamecfg);
}

void gamecfg_setskip(int skip) {

	GAMECFG	gamecfg;

	gamecfg = &gamecore.gamecfg;
	gamecfg->skip = skip;
#ifdef GAMEMSG_MSGWAIT
	gamemsg_send(GAMEMSG_MSGWAIT, skip);
#endif
	if (skip) {
		gamecfg->readskip = 0;
	}
}

void gamecfg_setreadskip(int readskip) {

	GAMECFG	gamecfg;

	gamecfg = &gamecore.gamecfg;
	gamecfg->readskip = readskip;
	if (readskip) {
		gamecfg->skip = 0;
#ifdef GAMEMSG_MSGWAIT
		gamemsg_send(GAMEMSG_MSGWAIT, 0);
#endif
	}
}

