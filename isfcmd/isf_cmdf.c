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
#include	"gamemsg.h"
#include	"arcfile.h"
#include	"moviemng.h"


// CNF : 連結ファイルのファイル名設定 (T.Yui)
int isfcmd_f0(SCR_OPE *op) {

	BYTE	type;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getbyte(op, &type) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	TRACEOUT(("arcfile change: type:%d name:%s", type, label));
	archive_set(gamecore.suf.scriptpath, label, type, 0);
	return(GAMEEV_SUCCESS);
}


// ATIMES : ウェイトの開始 (T.Yui)
int isfcmd_f1(SCR_OPE *op) {

	TIMEEVT_T	evt;

	if (scr_getval(op, &evt.val) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	evt.base = GETTICK();
	gamecore.wait = evt;
	return(GAMEEV_SUCCESS);
}


// AWAIT : ウェイト待ち (T.Yui)
int isfcmd_f2(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_WAIT);
}


// AVIP : AVI
int isfcmd_f3(SCR_OPE *op) {

	RECT_U	scrn;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	moviemng_play(label, &scrn.s);
	return(GAMEEV_SUCCESS);
}


// PPF : ポップアップメニューの表示設定 (Nonaka.K)
int isfcmd_f4(SCR_OPE *op) {

	BYTE	cmd;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	gamecore.gamecfg.enablepopup = cmd;

#ifdef GAMEMSG_ENABLEPOPUP
	return(gamemsg_send(GAMEMSG_ENABLEPOPUP, cmd));
#endif

	return(GAMEEV_SUCCESS);
}


// SVF : セーブの可・不可の設定 (Nonaka.K)
int isfcmd_f5(SCR_OPE *op) {

	BYTE	cmd;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	gamecore.gamecfg.enablesave = cmd;

#ifdef GAMEMSG_ENABLESAVE
	return(gamemsg_send(GAMEMSG_ENABLESAVE, cmd));
#endif

	return(GAMEEV_SUCCESS);
}


// SETGAMEINFO : ゲーム内情報の設定 (T.Yui)
int isfcmd_f7(SCR_OPE *op) {

	int		size;

	ZeroMemory(gamecore.comment, sizeof(gamecore.comment));
	size = min((int)sizeof(gamecore.comment), op->remain);
	if (size) {
		CopyMemory(gamecore.comment, op->ptr, size);
	}
	return(GAMEEV_SUCCESS);
}


// SETFONTSTYLE : 表示フォントスタイル指定
int isfcmd_f8(SCR_OPE *op) {

	BYTE	num;
	BYTE	style;
	TEXTWIN	textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &style) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_settype(&textwin->textctrl, style);
	}
	return(GAMEEV_SUCCESS);
}


// SETFONTCOLOR : 表示フォントカラー指定 (T.Yui)
int isfcmd_f9(SCR_OPE *op) {

	BYTE	num;
	BYTE	cnum;
	BYTE	col[3];
	TEXTWIN	textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cnum) != SUCCESS) ||
		(scr_getbyte(op, col + 0) != SUCCESS) ||
		(scr_getbyte(op, col + 1) != SUCCESS) ||
		(scr_getbyte(op, col + 2) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (cnum < 3) {
		textwin = textwin_getwin(num);
		if (textwin) {
			textwin->textctrl.fontcolor[cnum] =
										MAKEPALETTE(col[0], col[1], col[2]);
		}
	}
	return(GAMEEV_SUCCESS);
}


// TIMERSET : タイムカウンター設定 (Nonaka.K)
int isfcmd_fa(SCR_OPE *op) {

	TIMEEVT_T	evt;

	if (scr_getval(op, &evt.val) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	evt.base = GETTICK();
	gamecore.timer = evt;
	return(GAMEEV_SUCCESS);
}


// TIMEREND : タイムカウンター終了 (Nonaka.K)
int isfcmd_fb(SCR_OPE *op) {

	TIMEEVT	evt;

	evt = &gamecore.timer;
	evt->val = 0;
	evt->base = 0;

	(void)op;
	return(GAMEEV_SUCCESS);
}


// TIMERGET : タイムカウンター取得 (Nonaka.K)
int isfcmd_fc(SCR_OPE *op) {

	TIMEEVT	evt;
	UINT16	num;
	UINT32	now;

	if (scr_getword(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	evt = &gamecore.timer;
	now = GETTICK();
	evt->val += now - evt->base;
	evt->base = now;
	scr_valset(num, evt->val);
	return(GAMEEV_SUCCESS);
}

