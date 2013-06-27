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
#include	"arcfile.h"
#include	"isf_cmd.h"


// WS : ウィンドウ表示位置設定 Nonaka.K, T.Yui
int isfcmd_20(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	BYTE	val;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_getbyte(op, &val);
	TRACEOUT(("window: %d %d %d %d", scrn.s.left, scrn.s.top, scrn.s.width, scrn.s.height));

	textwin_setpos(num, &scrn.s, NULL);
	return(GAMEEV_SUCCESS);
}


// WP : ウィンドウパーツ読み込み (Nonaka.K)
int isfcmd_21(SCR_OPE *op) {

	BYTE	num;
	BYTE	dummy;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setframe(num, label);
	return(GAMEEV_SUCCESS);
}


// CN : 人物名文字数設定 (T.Yui)
int isfcmd_24(SCR_OPE *op) {

	BYTE	cmd;
	BYTE	len;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getbyte(op, &len) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_SUCCESS);
}


// CNS : 人物名セット Nonaka.K, T.Yui
int isfcmd_25(SCR_OPE *op) {

	BYTE	num;
	BYTE	cmd;
	TEXTWIN	textwin;
	int		len;
	char	*ptr;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (cmd < GAMECORE_MAXNAME) {
		textwin = textwin_getwin(num);
		if (textwin) {
			ptr = textwin->chrname[cmd];
			len = min(op->remain, GAMECORE_NAMELEN - 1);
			if (len) {
				CopyMemory(ptr, op->ptr, len);
			}
			ptr[len] = '\0';
		}
	}
	return(GAMEEV_SUCCESS);
}


// PF : メッセージ表示スピード設定
int isfcmd_26(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// PB : 文字の大きさ指定 (Nonaka.K)
int isfcmd_27(SCR_OPE *op) {

	BYTE		num;
	SINT32		size;
	TEXTWIN		textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getval(op, &size) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_setsize(&textwin->textctrl, size);
	}
	return(GAMEEV_SUCCESS);
}


// PJ : 文字の形態設定 (T.Yui)
int isfcmd_28(SCR_OPE *op) {

	BYTE		num;
	BYTE		type;
	TEXTWIN		textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &type) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_settype(&textwin->textctrl, type);
	}
	return(GAMEEV_SUCCESS);
}


// WO : ウィンドウオープン Nonaka.K, T.Yui							DRS cmd:28
int isfcmd_29(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd 29"));
	textwin_open(num);
	return(GAMEEV_SUCCESS);
}


// WC : ウィンドウのクローズ (T.Yui)								DRS cmd:29
int isfcmd_2a(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd 2a"));
	textwin_close(num);
	textwin_cmdclose(num);	// ドーターメーカーだとコマンドも閉じるらしい…
	return(GAMEEV_SUCCESS);
}


// PM : 文字の表示
int isfcmd_2b(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	return(textdisp_set(num, op));
}


// PMP : 音声フラグチェック付き文字の表示
int isfcmd_2c(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	return(textdisp_set(num, op));
}


// WSH : メッセージウィンドウの非表示 (T.Yui)
int isfcmd_2d(SCR_OPE *op) {

	SINT32	num;
	TEXTWIN	textwin;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd 2d"));
	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->flag |= TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// WSS : メッセージウィンドウの表示 (T.Yui)
int isfcmd_2e(SCR_OPE *op) {

	SINT32	num;
	TEXTWIN	textwin;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("cmd 2e"));
	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->flag &= ~TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// WJP : メッセージウィンドウの位置共有 (T.Yui)
int isfcmd_2f(SCR_OPE *op) {

	BYTE	num;
	BYTE	num2;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &num2) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_SUCCESS);
}

