#include	"compiler.h"
#include	"timemng.h"
#include	"gamecore.h"
#include	"savefile.h"
#include	"savedrs.h"
#include	"saveisf.h"


typedef struct {
	BYTE	*ptr;
	int		size;
} _SAVEDATA, *SAVEDATA;


int savefile_getsaves(void) {

	int		ret;

	switch(gamecore.sys.type & GAME_SAVEMASK) {
		case GAME_SAVEMAX9:
			ret = 9;
			break;

		case GAME_SAVEMAX27:
			ret = 27;
			break;

		case GAME_SAVEMAX30:
		default:
			ret = 30;
			break;

		case GAME_SAVEMAX50:
			ret = 50;
			break;
	}
	return(ret);
}

void savefile_cnvdate(SAVEDATE sd, const void *st) {

	sd->year = LOADINTELWORD(((_SYSTIME *)st)->year);
	sd->month = LOADINTELWORD(((_SYSTIME *)st)->month);
	sd->day = LOADINTELWORD(((_SYSTIME *)st)->day);
	sd->hour = LOADINTELWORD(((_SYSTIME *)st)->hour);
	sd->min = LOADINTELWORD(((_SYSTIME *)st)->minute);
	sd->sec = LOADINTELWORD(((_SYSTIME *)st)->second);
}

int savefile_cmpdate(const SAVEDATE sd1, const SAVEDATE sd2) {

	int		r;

	r = sd2->year - sd1->year;
	if (r) {
		goto cd_exit;
	}
	r = sd2->month - sd1->month;
	if (r) {
		goto cd_exit;
	}
	r = sd2->day - sd1->day;
	if (r) {
		goto cd_exit;
	}
	r = sd2->hour - sd1->hour;
	if (r) {
		goto cd_exit;
	}
	r = sd2->min - sd1->min;
	if (r) {
		goto cd_exit;
	}
	r = sd2->sec - sd1->sec;

cd_exit:
	return(r);
}


// ---- dummy function

void savenone_close(SAVEHDL hdl) {

	(void)hdl;
}

BOOL savenone_exist(SAVEHDL hdl, int num) {

	(void)hdl;
	(void)num;
	return(FALSE);
}

int savenone_getnewdate(SAVEHDL hdl) {

	(void)hdl;
	return(-1);
}

BOOL savenone_readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height) {
	(void)hdl;
	(void)num;
	(void)inf;
	(void)width;
	(void)height;
	return(FAILURE);
}

BOOL savenone_readgame(SAVEHDL hdl, int num) {

	(void)hdl;
	(void)num;
	return(FAILURE);
}

BOOL savenone_readflags(SAVEHDL hdl, int num, UINT reg, UINT pos, UINT cnt) {

	(void)hdl;
	(void)num;
	(void)reg;
	(void)pos;
	(void)cnt;
	return(FAILURE);
}

BOOL savenone_writegame(SAVEHDL hdl, int num, int vnum) {

	(void)hdl;
	(void)num;
	(void)vnum;
	return(FAILURE);
}

BOOL savenone_sys(SAVEHDL hdl, UINT pos, UINT size) {

	(void)hdl;
	(void)pos;
	(void)size;
	return(FAILURE);
}

BOOL savenone_exa(SAVEHDL hdl, void *val) {

	(void)hdl;
	(void)val;
	return(FAILURE);
}


// ----

static const _SAVEHDL savedummy = {
			savenone_close,
			savenone_exist, savenone_getnewdate, savenone_readinf,
			savenone_readgame, savenone_readflags,
			savenone_sys, savenone_sys,
			savenone_exa, savenone_exa, savenone_exa,
			savenone_writegame, savenone_sys,
			savenone_exa, savenone_exa, savenone_exa};

SAVEHDL savefile_open(BOOL create) {

	SAVEHDL		ret;

	if (gamecore.sys.type & GAME_DRS) {
		ret = savedrs_open(create);
	}
	else {
		ret = saveisf_open(create);
	}

	if (ret == NULL) {
		ret = (SAVEHDL)&savedummy;
	}
	return(ret);
}

