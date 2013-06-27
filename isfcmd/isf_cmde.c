// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"timemng.h"
#include	"gamemsg.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"savefile.h"
#include	"isf_cmd.h"
#ifndef DISABLE_DIALOG
#include	"menubase.h"
#include	"dlgcfg.h"
#include	"dlgsave.h"
#endif


// TAGSET : ダイアログのタグの設定 (T.Yui)
int isfcmd_e0(SCR_OPE *op) {

	_GCDLG	gcd;

	ZeroMemory(&gcd, sizeof(gcd));
	gcd.cmd = CFGTYPE_TAG;
	if ((scr_getbyte(op, &gcd.page) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.t.str, sizeof(gcd.c.t.str)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_append(gamecore.cfglist, &gcd);
	return(GAMEEV_SUCCESS);
}


// FRAMESET : ダイアログのフレーム設定 (T.Yui)
int isfcmd_e1(SCR_OPE *op) {

	_GCDLG	gcd;

	ZeroMemory(&gcd, sizeof(gcd));
	gcd.cmd = CFGTYPE_FRAME;
	if ((scr_getbyte(op, &gcd.page) != SUCCESS) ||
		(scr_getbyte(op, &gcd.group) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.f.str, sizeof(gcd.c.f.str)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_append(gamecore.cfglist, &gcd);
	return(GAMEEV_SUCCESS);
}


// RBSET : ダイアログのラジオボタン設定 (T.Yui)
int isfcmd_e2(SCR_OPE *op) {

	_GCDLG	gcd;

	ZeroMemory(&gcd, sizeof(gcd));
	gcd.cmd = CFGTYPE_RADIO;
	if ((scr_getbyte(op, &gcd.page) != SUCCESS) ||
		(scr_getbyte(op, &gcd.group) != SUCCESS) ||
		(scr_getbyte(op, &gcd.num) != SUCCESS) ||
		(scr_getword(op, &gcd.c.r.val) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.r.str, sizeof(gcd.c.r.str)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_append(gamecore.cfglist, &gcd);
	return(GAMEEV_SUCCESS);
}


// CBSET : ダイアログのチェックボックス設定 (T.Yui)
int isfcmd_e3(SCR_OPE *op) {

	_GCDLG	gcd;

	ZeroMemory(&gcd, sizeof(gcd));
	gcd.cmd = CFGTYPE_CHECK;
	if ((scr_getbyte(op, &gcd.page) != SUCCESS) ||
		(scr_getbyte(op, &gcd.group) != SUCCESS) ||
		(scr_getbyte(op, &gcd.num) != SUCCESS) ||
		(scr_getword(op, &gcd.c.c.val) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.c.str, sizeof(gcd.c.c.str)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_append(gamecore.cfglist, &gcd);
	return(GAMEEV_SUCCESS);
}


// SLDRSET : ダイアログのスライダー設定 (T.Yui)
int isfcmd_e4(SCR_OPE *op) {

	_GCDLG	gcd;
	BYTE	dummy;

	ZeroMemory(&gcd, sizeof(gcd));
	gcd.cmd = CFGTYPE_SLIDER;
	if ((scr_getbyte(op, &gcd.page) != SUCCESS) ||
		(scr_getbyte(op, &gcd.group) != SUCCESS) ||
		(scr_getbyte(op, &gcd.num) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.s.str, sizeof(gcd.c.s.str)) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.s.min, sizeof(gcd.c.s.min)) != SUCCESS) ||
		(scr_getlabel(op, gcd.c.s.max, sizeof(gcd.c.s.max)) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &gcd.c.s.minval) != SUCCESS) ||
		(scr_getval(op, &gcd.c.s.maxval) != SUCCESS) ||
		(scr_getval(op, &gcd.c.s.mul) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getword(op, &gcd.c.s.val) != SUCCESS) ||			// dummy!
		(scr_getword(op, &gcd.c.s.val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_append(gamecore.cfglist, &gcd);
	return(GAMEEV_SUCCESS);
}


// OPSL : SAVE・LOADダイアログのオープン (T.Yui)
int isfcmd_e5(SCR_OPE *op) {

	BYTE	cmd;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (cmd == 0) {			// save
#ifndef DISABLE_DIALOG
		dlgsave_save();
		menubase_modalproc();
#endif
#ifdef GAMEMSG_SAVE
		return(gamemsg_send(GAMEMSG_SAVE, 0));
#endif
	}
	else if (cmd == 1) {	// load
#ifndef DISABLE_DIALOG
		dlgsave_load();
		menubase_modalproc();
#endif
#ifdef GAMEMSG_LOAD
		return(gamemsg_send(GAMEMSG_LOAD, 0));
#endif
	}

	return(GAMEEV_SUCCESS);
}


// OPPROP : 設定ダイアログのオープン
int isfcmd_e6(SCR_OPE *op) {

#ifndef DISABLE_DIALOG
	cfgdlg_open();
	menubase_modalproc();
#endif
	(void)op;
	return(GAMEEV_SUCCESS);
}


// DISABLE : ダイアログコントロールのディセイブル (T.Yui)
int isfcmd_e7(SCR_OPE *op) {

	_GCDLGD	prm;

	if ((scr_getbyte(op, &prm.page) != SUCCESS) ||
		(scr_getbyte(op, &prm.group) != SUCCESS) ||
		(scr_getbyte(op, &prm.num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	prm.disable = 1;
	gamecfg_setdisable(&prm);
	return(GAMEEV_SUCCESS);
}


// ENABLE : ダイアログコントロールのイネイブル (T.Yui)
int isfcmd_e8(SCR_OPE *op) {

	_GCDLGD	prm;

	if ((scr_getbyte(op, &prm.page) != SUCCESS) ||
		(scr_getbyte(op, &prm.group) != SUCCESS) ||
		(scr_getbyte(op, &prm.num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	prm.disable = 0;
	gamecfg_setdisable(&prm);
	return(GAMEEV_SUCCESS);
}


// EXT : 拡張処理 (T.Yui)
static int isfcmd_ef1(SCR_OPE *op) {

	SINT32	num;
	SINT32	vnum;
	SAVEHDL	sh;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	vnum = 0;
	if ((gamecore.sys.type & GAME_SAVEGRPH) &&
		(scr_getval(op, &vnum) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	sh = savefile_open(TRUE);
	sh->writegame(sh, num, vnum);
	sh->close(sh);
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef2(SCR_OPE *op) {

	SINT32	num;
	SAVEHDL	sh;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	sh = savefile_open(FALSE);
	sh->readgame(sh, num);
	sh->close(sh);
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef3(SCR_OPE *op) {

	BYTE	cmd;
	SINT32	prm[5];
	int		cmds;
	int		i;
	SAVEHDL	sh;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	switch(cmd) {
		case 0:
			cmds = 5;
			break;
		case 1:
		case 2:
			cmds = 4;
			break;
		case 3:
			cmds = 1;
			break;
		default:
			return(GAMEEV_WRONGPARAM);
	}
	for (i=0; i<cmds; i++) {
		if (scr_getval(op, prm + i) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	sh = savefile_open(FALSE);
	switch(cmd) {
		case 0:				// ??
			break;
		case 1:				// get flags
			if (prm[3] > 0) {
				sh->readflags(sh, prm[1], prm[0], prm[2], prm[3]);
			}
			break;
		case 2:
			while(prm[2] > 0) {
				scr_flagop(prm[0], (BYTE)(sh->exist(sh, prm[1])?1:0));
				prm[0]++;
				prm[1]++;
				prm[2]--;
			}
			break;
		case 3:
			scr_valset(prm[0], sh->getnewdate(sh));
			break;
	}
	sh->close(sh);
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef4(SCR_OPE *op) {

	SINT32		prm[7];
	int			i;
	SAVEHDL		sh;
	_SAVEINF	inf;
	BOOL		r;
	_SYSTIME	st;

	for (i=0; i<7; i++) {
		if (scr_getval(op, prm + i) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	ZeroMemory(&inf, sizeof(inf));
	sh = savefile_open(FALSE);
	r = sh->readinf(sh, prm[0], &inf, 0, 0);
	sh->close(sh);
	vram_destroy((VRAMHDL)inf.preview);
	if (r != SUCCESS) {
		timemng_gettime(&st);
		savefile_cnvdate(&inf.date, &st);
	}
	scr_valset(prm[1], inf.date.year);
	scr_valset(prm[2], inf.date.month);
	scr_valset(prm[3], inf.date.day);
	scr_valset(prm[4], inf.date.hour);
	scr_valset(prm[5], inf.date.min);
	scr_valset(prm[6], inf.date.sec);
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef5(SCR_OPE *op) {

	SINT32		vnum;
	SINT32		snum;
	POINT_T		pt;
	SAVEHDL		sh;
	_SAVEINF	inf;
	TEXTCTRL	textctrl;

	if ((scr_getval(op, &vnum) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS) ||
		(scr_getval(op, &snum) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((vnum >= 0) && (vnum < GAMECORE_MAXVRAM)) {
		ZeroMemory(&inf, sizeof(inf));
		sh = savefile_open(FALSE);
		sh->readinf(sh, snum, &inf, 0, 0);
		sh->close(sh);
		vram_destroy((VRAMHDL)inf.preview);
#ifdef SIZE_QVGA
		vramdraw_halfpoint(&pt);
#endif
		textctrl = &gamecore.textdraw;
		textctrl_renewal(textctrl);
		vrammix_text(gamecore.vram[vnum], textctrl->font, inf.comment,
										textctrl->fontcolor[0], &pt, NULL);
		effect_vramdraw(vnum, NULL);
	}
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef6(SCR_OPE *op) {

	int		r;
	int		i;
	SINT32	prm[5];
	SCOMCFG	scomcfg;

	r = 5;
	for (i=0; i<r; i++) {
		if (scr_getval(op, prm + i) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	if (r == 4) {
		prm[4] = prm[3];
		prm[3] = 80;
	}
	scomcfg = &gamecore.scomcfg;
	scomcfg->width = prm[0];
	scomcfg->indenty = prm[1];
	scomcfg->indentx = prm[2];
	scomcfg->length = prm[3];
	scomcfg->flag = (prm[4] != 0);
	return(GAMEEV_SUCCESS);
}

static int isfcmd_ef7(SCR_OPE *op) {

	SINT32	prm1;
	SINT32	prm2;

	if ((scr_getval(op, &prm1) != SUCCESS) ||
		(scr_getval(op, &prm2) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd ef 07 : %d %d", prm1, prm2));
	return(GAMEEV_SUCCESS);
}

int isfcmd_ef(SCR_OPE *op) {

	BYTE	cmd;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	switch(cmd) {
		case 1:
			return(isfcmd_ef1(op));
		case 2:
			return(isfcmd_ef2(op));
		case 3:
			return(isfcmd_ef3(op));
		case 4:
			return(isfcmd_ef4(op));
		case 5:
			return(isfcmd_ef5(op));
		case 6:
			return(isfcmd_ef6(op));
		case 7:
			return(isfcmd_ef7(op));
	}
	return(GAMEEV_WRONGPARAM);
}

