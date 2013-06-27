// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"inputmng.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"isf_cmd.h"


#ifndef DISABLE_DIALOG
#include	"menubase.h"

static const BYTE mboxtype[6] = {MBOX_OK, MBOX_OKCANCEL, MBOX_YESNO,
								MBOX_YESNOCAN, MBOX_RETRY, MBOX_ABORT};
static const BYTE mboxval[7] = {0, 1, 5, 4, 6, 2, 3};
#endif


// CHANGEWALL : 壁紙変更 (T.Yui)
int isfcmd_b2(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN+1];

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd b2"));
	return(GAMEEV_SUCCESS);		// 意味なし
}


// MSGBOX : メッセージボックス表示 (T.Yui)
int isfcmd_b3(SCR_OPE *op) {

	SINT32	num;
	char	msg[256];
	BYTE	type;
	int		val;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getlabel(op, msg, sizeof(msg)) != SUCCESS) ||
		(scr_getbyte(op, &type) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
#ifndef DISABLE_DIALOG
	if (type < 6) {
		type = mboxtype[type];
	}
	else {
		type = MBOX_OK;
	}
	val = menumbox(msg, gamecore.suf.key, type) - 1;
	if ((val >= 0) && (val < 7)) {
		val = mboxval[val];
	}
	else {
		val = 0;
	}
#else
	val = 0;
#endif
	scr_valset(num, val);
	return(GAMEEV_SUCCESS);
}


// SETSMPRATE : サンプリングレート設定
int isfcmd_b4(SCR_OPE *op) {

	SINT32	hz;

	if (scr_getval(op, &hz) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
#ifdef TRACE
	if (hz == 0) {
		TRACEOUT(("set sampling ratio 22kHz"));
	}
	else if (hz == 1) {
		TRACEOUT(("set sampling ratio 44kHz"));
	}
#endif
	return(GAMEEV_SUCCESS);
}


// CLKEXMCSET : クリック待ち拡張機能のマウスカーソルＩＤ初期化 (T.Yui)
int isfcmd_bd(SCR_OPE *op) {

	SINT32	dummy;

	if ((scr_getval(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &dummy) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_SUCCESS);		// 意味なし
}


// 0xBE: IRCLK (T.Yui)
int isfcmd_be(SCR_OPE *op) {

	event_resetmouse(0);
	event_resetkey(~KEY_ENTER);
	(void)op;
	return(GAMEEV_IRCLK);
}


// 0xBF: IROPN (T.Yui)
int isfcmd_bf(SCR_OPE *op) {

	textwin_open(0);
	textwin_clear(0);
	(void)op;
	return(GAMEEV_SUCCESS);
}

