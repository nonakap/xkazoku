// -------------------------------------------------------------
//
//	DRS script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"moviemng.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"drs_cmd.h"


// AVIP : AVI
int drscmd_f3(SCR_OPE *op) {

	RECT_U	scrn;
	char	label[ARCFILENAME_LEN+1];
	BYTE	cmd;

	if ((scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	moviemng_play(label, &scrn.s);
	return(GAMEEV_SUCCESS);
}


// SaveData Enable?
int drscmd_f8(SCR_OPE *op) {

	gamecore.drssys.savepre = 1;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// SaveData Disable?
int drscmd_f9(SCR_OPE *op) {

	gamecore.drssys.savepre = 0;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// SaveData Reset
int drscmd_fa(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}

