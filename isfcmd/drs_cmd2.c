// -------------------------------------------------------------
//
//	DRS script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"drs_cmd.h"
#include	"fontmng.h"
#include	"cgload.h"


// WS : ウィンドウ表示位置設定 (T.Yui)
int drscmd_20(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn, clip;
	BYTE	val;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getrect(op, &clip) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_getbyte(op, &val);

	textwin_setpos(num, &scrn.s, &clip.s);
	return(GAMEEV_SUCCESS);
}


// WP : ウィンドウパーツ読み込み (T.Yui)
int drscmd_21(SCR_OPE *op) {

	BYTE	num;
	BYTE	dummy;
	SINT32	width, height;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getval(op, &width) != SUCCESS) ||
		(scr_getval(op, &height) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setframe(num, label);
	return(GAMEEV_SUCCESS);
}


// WO : ウィンドウオープン
int drscmd_28(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_open(num);
	return(GAMEEV_SUCCESS);
}


// WC : ウィンドウのクローズ
int drscmd_29(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_close(num);
	return(GAMEEV_SUCCESS);
}


// WSS : メッセージウィンドウの表示
int drscmd_2a(SCR_OPE *op) {

	BYTE	cmd;
	TEXTWIN	textwin;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(0);
	if (textwin) {
		textwin->flag |= TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// WSH : メッセージウィンドウの非表示
int drscmd_2b(SCR_OPE *op) {

	BYTE	cmd;
	TEXTWIN	textwin;

	if (scr_getbyte(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(0);
	if (textwin) {
		textwin->flag &= ~TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}

