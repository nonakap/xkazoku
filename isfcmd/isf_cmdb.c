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
#include	"inputmng.h"


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


// 0xBE: IRCLK
int isfcmd_be(SCR_OPE *op) {

	event_resetmouse(0);
	event_resetkey(~KEY_ENTER);
	(void)op;
	return(GAMEEV_IRCLK);
}


// 0xBF: IROPN
int isfcmd_bf(SCR_OPE *op) {

	textwin_open(0);
	textwin_clear(0);
	(void)op;
	return(GAMEEV_SUCCESS);
}

