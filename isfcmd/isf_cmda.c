// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"gamecore.h"
#include	"isf_cmd.h"
#include	"savefile.h"


// KIDFN : 既読フラグ数設定 (Nonaka.K)
int isfcmd_a5(SCR_OPE *op) {

	FLAGS	flags;
	UINT32	cnt;
	SAVEHDL	sh;

	flags = &gamecore.flags;

	if (scr_getdword(op, &cnt) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}

	scr_kidclear();
	flags->kid = variant_create(cnt, VARIANT_BIT);

	sh = savefile_open(FALSE);
	savefile_readkid(sh, flags->kid);
	savefile_close(sh);

	return(GAMEEV_SUCCESS);
}


// KIDSCAN : 既読機能と既読フラグの判定
int isfcmd_a7(SCR_OPE *op) {

	UINT16	pos;
	SINT32	num;
	BYTE	flag;
	int		readflag;
	GAMECFG	gamecfg;

	if ((scr_getword(op, &pos) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	readflag = 0;
	if (variant_get(gamecore.flags.kid, num, &flag) == SUCCESS) {
		readflag = flag;
	}
	gamecfg = &gamecore.gamecfg;
	gamecfg->lastread = readflag;
	readflag |= gamecfg->readskip << 1;
	scr_valset(pos, readflag);
	return(GAMEEV_SUCCESS);
}


// SETKIDWNDPUTPOS : 既読ウィンドウのプット位置指定
int isfcmd_ae(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	DISPWIN	dispwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	dispwin = &gamecore.dispwin;
	dispwin->flag |= DISPWIN_CLIPHIS;
	dispwin->hisclip = scrn.s;
	return(GAMEEV_SUCCESS);
}


// SETMESWNDPUTPOS : メッセージウィンドウのプット位置指定
int isfcmd_af(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	DISPWIN	dispwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	dispwin = &gamecore.dispwin;
	dispwin->flag |= DISPWIN_CLIPTEXT;
	dispwin->txtclip = scrn.s;
	return(GAMEEV_SUCCESS);
}

