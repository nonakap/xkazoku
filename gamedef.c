#include	"compiler.h"
#include	"dosio.h"
#include	"gamedef.h"


typedef struct {
const char	*company;
const char	*key;
	UINT	exever;
	UINT	bmpsize;
	UINT	flagsize;
	UINT	gametype;
} GAMEDEF;

#define	PVAL(a, b)			(UINT)(((a) & 0xffff) + ((b) << 16))
#define	DEFAULTBMP			PVAL(240, 180)
#define	DEFAULTFLAG			PVAL(1024, 1024)


static const GAMEDEF gamedef[] = {
	// DRS
	{NULL,		"KANA",		EXEVER_KANA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_DRS | GAME_HAVEALPHA |
							GAME_SAVEMAX30 | GAME_SAVECOM},

	// AngelSmile
	{NULL,		"Romance",	EXEVER_VECHO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_NOKID | GAME_VOICE},

	{NULL,	"PrivateNurse",	EXEVER_NURSE,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVESYS | GAME_VOICE},

	{NULL,	"VelvetEcho",	EXEVER_VECHO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_NOKID | GAME_VOICE},

	{NULL,		"SISKON",	EXEVER_SISKON,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"HEART",	EXEVER_HEART,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"AKIBA",	EXEVER_AKIBA,	PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM},

	// CDPA
	{NULL,		"AOI",		EXEVER_AOI,		PVAL(110, 83), DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_TEXTLAYER | GAME_CMDWINNOBG},

	// D.O.
	{NULL,		"TEA2",		EXEVER_TEA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVESYS | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"TEA2SMPL",	EXEVER_TEA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVESYS | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"TEA2DEMO",	EXEVER_TEA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVESYS | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"TEA3",		EXEVER_KONYA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE},

	{NULL,		"TEA3TRY",	EXEVER_KONYA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE},

	{NULL,		"CRES",		EXEVER_CRES,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_HAVEALPHA},

	{NULL,		"CRESD",	EXEVER_CRESD,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_HAVEALPHA | GAME_VOICE},

	{NULL,		"PLANET",	EXEVER_PLANET,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_NOKID |
							GAME_SAVEGRPH | GAME_SAVECOM},

	{NULL,		"PLANDVD",	EXEVER_PLANET,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_NOKID |
							GAME_SAVEGRPH | GAME_SAVECOM | GAME_SVGA},

	{NULL, 		"RENAI",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"KAZOKU",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_SELECTEX},

	{NULL,		"KAZOKUK",	EXEVER_KAZOKUK,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM |
							GAME_VOICE | GAME_SELECTEX},

	{NULL,		"KAZOKUKU",	EXEVER_KAZOKUK,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM |
							GAME_VOICE | GAME_SELECTEX},

	{NULL,		"KA_MOOK",	EXEVER_KAMOOK,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII},

	{NULL,		"RESTORE",	EXEVER_RESTORE, PVAL(96, 72), PVAL(1208, 512),
							GAME_SAVEMAX50 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_SAVEAUTOCOM | GAME_TEXTLAYER},

	// FORTUNE
	{NULL,		"DM",		EXEVER_DM,		DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"KT_TRY",	EXEVER_KT_TRY,	PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM | GAME_HAVEALPHA},

	// Purple
	{"Purple",	NULL,		EXEVER_MYU,		DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX9 | GAME_SAVESYS |
							GAME_VOICE | GAME_TEXTLAYER},

	// SEKILALA
	{NULL,		"KONYA",	EXEVER_KONYA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM |
							GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"Maid",		EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVECOM | GAME_TEXTASCII |
							GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"ROSYU2",	EXEVER_ROSYU2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"KONYA2",	EXEVER_KONYA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"HIMEKU",	EXEVER_HEART,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"OSHI_TAI",	EXEVER_OSHI_TAI,PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM},

	{NULL,		"OSHIHA",	EXEVER_AKIBA,	PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM},

	// TRABULANCE
	{NULL,		"TSUKUSHI",	EXEVER_TSUKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_SAVESYS},

	{NULL,		"TSUKU2",	EXEVER_PLANET,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_SAVESYS},

	{NULL,		"2KANGO",	EXEVER_KONYA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"NORIKA",	EXEVER_KONYA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"NANGOKU",	EXEVER_KONYA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"TUKU3",	EXEVER_KAZOKUK,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"SILK1D",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"SILK2D",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"SILK_AD",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"NYAN",		EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL, 		"KANGO2",	EXEVER_KAZOKU,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_VOICE},

	{NULL,		"fuku",		EXEVER_KONYA2,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"OSHIETE",	EXEVER_HEART,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"TUKU4",	EXEVER_HEART,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"SHISYO",	EXEVER_SHISYO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"SAMOV",	EXEVER_SHISYO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE},

	{NULL,		"ANATUMA",	EXEVER_AKIBA,	PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM},

	// WILL
	{NULL,		"SnowDrop",	EXEVER_AMEIRO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX9 | GAME_VOICE | GAME_VOICEONLY},

	// YAMIKUMO
	{NULL,		"PEE",		EXEVER_HEART,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"HIMITSU",	EXEVER_DM,		DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE},

	// ZACKZACK
	{NULL,		"MOEKKO",	EXEVER_MOEKKO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_SAVEAUTOCOM | GAME_VOICE},

	// ZyX
	{NULL,		"AMEIRO",	EXEVER_AMEIRO,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVESYS | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"INNAI2",	EXEVER_PLANET,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX27 | GAME_SAVEGRPH |
							GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"BUNNY",	EXEVER_NURSE,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE},

	{NULL,		"CHAIN",	EXEVER_KONYA,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_VOICE},

	{NULL,		"BLEED",	EXEVER_BLEED,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"SHIKOMI",	EXEVER_DM,		DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_TEXTASCII | GAME_SAVEGRPH |
							GAME_SAVECOM | GAME_VOICE | GAME_VOICEONLY},

	{NULL,		"SAGARA",	EXEVER_AKIBA,	PVAL(96, 72), PVAL(2048, 2048),
							GAME_SAVEMAX50 | GAME_SAVEGRPH | GAME_SAVECOM |
							GAME_TEXTASCII | GAME_VOICE | GAME_TEXTLAYER |
							GAME_SAVEAUTOCOM},

	// ZEROCOOL
	{NULL,		"JEWEL",	EXEVER_DM,	DEFAULTBMP, DEFAULTFLAG,
							GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_TEXTLAYER |
							GAME_SAVECOM | GAME_VOICE},

	// error (force parametor)
	{"TRABULANCE", NULL,	EXEVER_KONYA,	DEFAULTBMP, PVAL(2048, 2048),
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_SAVEALIAS |
							GAME_VOICE | GAME_VOICEONLY},

	{NULL,		NULL,		EXEVER_PLANET,	DEFAULTBMP, PVAL(2048, 2048),
							GAME_SAVEMAX27 | GAME_SAVECOM | GAME_SAVEALIAS}
};


// ----

typedef struct {
	char	title[64];
	char	company[64];
	char	key[32];
	UINT	version;
	UINT	type;
	int		bmpwidth;
	int		bmpheight;
	UINT	bits;
	UINT	flags;
} _DEFTBL, *DEFTBL;

typedef struct {
const char	*str;
	UINT	val;
} _STRTBL, *STRTBL;


static	LISTARRAY	deftbl = NULL;


#define	STR2NUM(str)		{#str, (str)}

static const _STRTBL exever[] = {
	STR2NUM(EXE_VER0),
	STR2NUM(EXEVER_KANA),
	STR2NUM(EXEVER_MYU),

	STR2NUM(EXE_VER1),
	STR2NUM(EXEVER_TSUKU),
	STR2NUM(EXEVER_AMEIRO),
	STR2NUM(EXEVER_TEA2),
	STR2NUM(EXEVER_PLANET),
	STR2NUM(EXEVER_PLANDVD),
	STR2NUM(EXEVER_NURSE),
	STR2NUM(EXEVER_KONYA),
	STR2NUM(EXEVER_VECHO),
	STR2NUM(EXEVER_CRES),
	STR2NUM(EXEVER_KAZOKU),
	STR2NUM(EXEVER_BLEED),
	STR2NUM(EXEVER_ROSYU2),
	STR2NUM(EXEVER_OSHIETE),
	STR2NUM(EXEVER_SISKON),
	STR2NUM(EXEVER_KONYA2),
	STR2NUM(EXEVER_KAZOKUK),
	STR2NUM(EXEVER_HEART),
	STR2NUM(EXEVER_SHISYO),

	STR2NUM(EXE_VER2),
	STR2NUM(EXEVER_HIMITSU),
	STR2NUM(EXEVER_DM),
	STR2NUM(EXEVER_MOEKKO),
	STR2NUM(EXEVER_AOI),
	STR2NUM(EXEVER_KAMOOK),
	STR2NUM(EXEVER_RESTORE),
	STR2NUM(EXEVER_AKIBA),
	STR2NUM(EXEVER_CRESD)
};

static const _STRTBL gametype[] = {
	STR2NUM(GAME_VOICE),
	STR2NUM(GAME_VOICEONLY),
	STR2NUM(GAME_HAVEALPHA),
	STR2NUM(GAME_TEXTASCII),
	STR2NUM(GAME_DRS),
	STR2NUM(GAME_SVGA),
	STR2NUM(GAME_NOKID),
	STR2NUM(GAME_ENGSTYLE),
	STR2NUM(GAME_SAVEMAX9),
	STR2NUM(GAME_SAVEMAX27),
	STR2NUM(GAME_SAVEMAX30),
	STR2NUM(GAME_SAVEMAX50),
	STR2NUM(GAME_SAVEGRPH),
	STR2NUM(GAME_SAVECOM),
	STR2NUM(GAME_SAVESYS),
	STR2NUM(GAME_SAVEAUTOCOM),
	STR2NUM(GAME_SAVEALIAS),
	STR2NUM(GAME_SELECTEX),
	STR2NUM(GAME_TEXTLAYER),
	STR2NUM(GAME_CMDWINNOBG)};


// とらぶーのゲームには「KEY=SILKDVD\SILK_AD」という
// のがあるので '\' 以前は無視させる… (NONAKA.K)
// 漢字だった場合も一応考慮 (NONAKA.K)
// どーせなら ':' '/'も対応しまぷ (T.Yui)
static const char *keyseek(const char *key) {

	char	c;
const char *ret;

	ret = key;
	do {
		c = *key++;
		if ((((c ^ 0x20) - 0xa1) & 0xff) >= 0x3c) {
			if ((c == '\\') || (c == '/') || (c == ':')) {
				ret = key;
			}
		}
		else if (key[0]) {
			key++;
		}
	} while(c);
	return(ret);
}


// ----

typedef struct {
	char	para[64];
	DEFTBL	def;
} _DEFARG, *DEFARG;

static BOOL tblsea(void *vpItem, void *vpArg) {

	if (!milstr_cmp(((DEFTBL)vpItem)->title, (char *)vpArg)) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

static BOOL defload(void *arg, const char *para,
									const char *key, const char *data) {

	DEFARG		defarg;
	DEFTBL		tbl;
const _STRTBL	*str;
const _STRTBL	*strterm;
	int			len;
	char		ctrl;

	defarg = (DEFARG)arg;
	tbl = defarg->def;
	if (milstr_cmp(defarg->para, para)) {
		milstr_ncpy(defarg->para, para, sizeof(defarg->para));
		tbl = (DEFTBL)listarray_enum(deftbl, tblsea, defarg->para);
		if (tbl == NULL) {
			if (deftbl == NULL) {
				deftbl = listarray_new(sizeof(_DEFTBL), 32);
			}
			tbl = (DEFTBL)listarray_append(deftbl, NULL);
			if (tbl) {
				milstr_ncpy(tbl->title, defarg->para, sizeof(tbl->title));
				tbl->bmpwidth = 160;
				tbl->bmpheight = 120;
				tbl->bits = 1024;
				tbl->flags = 1024;
			}
		}
		defarg->def = tbl;
	}
	if (tbl == NULL) {
		return(FALSE);
	}
	if (!milstr_cmp(key, "company")) {
		milstr_ncpy(tbl->company, data, sizeof(tbl->company));
	}
	else if (!milstr_cmp(key, "key")) {
		milstr_ncpy(tbl->key, keyseek(data), sizeof(tbl->key));
	}
	else if ((!milstr_cmp(key, "ver")) || (!milstr_cmp(key, "exever")) ||
			(!milstr_cmp(key, "version"))) {
		str = exever;
		strterm = str + (sizeof(exever)/sizeof(_STRTBL));
		do {
			if (!milstr_cmp(data, str->str)) {
				tbl->version = str->val;
				break;
			}
			str++;
		} while(str < strterm);
	}
	else if ((!milstr_cmp(key, "type")) || (!milstr_cmp(key, "gametype"))) {
		ctrl = data[0];
		if ((ctrl == '+') || (ctrl == '|') || (ctrl == '-')) {
			data++;
		}
		else {
			tbl->type = 0;
		}
		str = gametype;
		strterm = str + (sizeof(gametype)/sizeof(_STRTBL));
		do {
			if (!(milstr_memcmp(data, str->str))) {
				len = strlen(str->str);
				if ((data[len] == '\0') || (data[len] == '+') ||
					(data[len] == '|') || (data[len] == '-')) {
					if (str->val & GAME_SAVEMASK) {
						tbl->type &= ~GAME_SAVEMASK;
					}
					if (ctrl != '-') {
						tbl->type |= str->val;
					}
					else {
						tbl->type &= ~str->val;
					}
					ctrl = data[len];
					if (ctrl == '\0') {
						break;
					}
					str = gametype;
					data += len + 1;
					continue;
				}
			}
			str++;
		} while(str < strterm);
	}
	else if (!milstr_cmp(key, "bmpwidth")) {
		tbl->bmpwidth = milstr_solveINT(data);
	}
	else if (!milstr_cmp(key, "bmpheight")) {
		tbl->bmpheight = milstr_solveINT(data);
	}
	else if (!milstr_cmp(key, "bits")) {
		tbl->bits = milstr_solveINT(data);
	}
	else if (!milstr_cmp(key, "flags")) {
		tbl->flags = milstr_solveINT(data);
	}
	return(FALSE);
}

void gamedef_create(void) {

	_DEFARG	arg;

	gamedef_destroy();
	ZeroMemory(&arg, sizeof(arg));
	profile_enum(file_getcd("gamedef.txt"), &arg, defload);
}

void gamedef_destroy(void) {

	listarray_destroy(deftbl);
	deftbl = NULL;
}


// ----

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
			milstr_ncpy(suf->key, keyseek(data), sizeof(suf->key));
		}
	}
	return(FALSE);
}

static BOOL tblsea2(void *vpItem, void *vpArg) {

	if ((!milstr_cmp(((DEFTBL)vpItem)->company, ((SUF_T *)vpArg)->company)) &&
		(!milstr_cmp(((DEFTBL)vpItem)->key, ((SUF_T *)vpArg)->key))) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

void gamedef_loadsuf(const char *path, SUF_T *suf, SYS_T *sys) {

	DEFTBL		dtbl;
const GAMEDEF	*tbl;
	UINT		cnt;

	ZeroMemory(suf, sizeof(SUF_T));
	profile_enum(path, suf, sufload);
	milstr_ncpy(suf->scriptpath, path, sizeof(suf->scriptpath));
	cutFileName(suf->scriptpath);

	// 黒の図書館
	if (suf->key[0] == '\0') {
		milstr_ncpy(suf->key, getFileName((char *)path), sizeof(suf->key));
		cutExtName(suf->key);
	}

	TRACEOUT(("COMPANY: %s", suf->company));
	TRACEOUT(("TITLE: %s", suf->title));
	TRACEOUT(("KEY: %s", suf->key));

	dtbl = (DEFTBL)listarray_enum(deftbl, tblsea2, suf);
	if (dtbl) {
		sys->version = dtbl->version;
		sys->type = dtbl->type;
		sys->bmpwidth = dtbl->bmpwidth;
		sys->bmpheight = dtbl->bmpheight;
		sys->defbits = dtbl->bits;
		sys->defflags = dtbl->flags;
	}
	else {
		tbl = gamedef;
		cnt = sizeof(gamedef) / sizeof(GAMEDEF);
		while(cnt--) {
			if (((tbl->company == NULL) ||
					(!milstr_cmp(tbl->company, suf->company))) &&
				((tbl->key == NULL) || (!milstr_cmp(tbl->key, suf->key)))) {
				sys->version = tbl->exever;
				sys->type = tbl->gametype;
				sys->bmpwidth = (int)(SINT16)(tbl->bmpsize & 0xffff);
				sys->bmpheight = (int)(SINT16)(tbl->bmpsize >> 16);
				sys->defbits = tbl->flagsize & 0xffff;
				sys->defflags = tbl->flagsize >> 16;
				break;
			}
			tbl++;
		}
	}

	TRACEOUT(("ver/type = %d/%08x", sys->version, sys->type));

	if (sys->type & GAME_SVGA) {
		sys->width = 800;
		sys->height = 600;
	}
	else {
		sys->width = 640;
		sys->height = 480;
	}
}

