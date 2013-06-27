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
#include	"drs_cmd.h"


// PB : 文字の大きさ指定
int drscmd_11(SCR_OPE *op) {

	SINT32		size;
	DISPWIN		dispwin;
	int			i;
	TEXTWIN		textwin;

	if (scr_getval(op, &size) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	dispwin = &gamecore.dispwin;
	dispwin->fontsize = size;
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			textctrl_setsize(&textwin->textctrl, size);
		}
	}
	return(GAMEEV_SUCCESS);
}


// PJ : 文字の形態設定
int drscmd_12(SCR_OPE *op) {

	BYTE		type;
	DISPWIN		dispwin;
	int			i;
	TEXTWIN		textwin;

	if (scr_getbyte(op, &type) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	type &= TEXTCTRL_FONTMASK;
	dispwin = &gamecore.dispwin;
	dispwin->fonttype &= ~TEXTCTRL_FONTMASK;
	dispwin->fonttype |= type;
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if (textwin) {
			textctrl_settype(&textwin->textctrl, type);
		}
	}
	return(GAMEEV_SUCCESS);
}


// CWO : コマンドウィンドウのオープン
int drscmd_15(SCR_OPE *op) {

	SINT32	cmds;

	if (scr_getval(op, &cmds) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (cmds > GAMECORE_MAXCMDS) {
		cmds = GAMECORE_MAXCMDS;
	}
	textwin_cmdopen(0, cmds, 2);
	return(GAMEEV_SUCCESS);
}

