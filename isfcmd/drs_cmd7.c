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


// SEP : Œø‰Ê‰¹‚ÌÄ¶
int drscmd_77(SCR_OPE *op) {

	SINT32	num;
	BYTE	cmd;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		sndplay_seplay(num | 0x20, 0);
	}

	return(GAMEEV_SUCCESS);
}


// DAE : CDDA‚Ìİ’è
int drscmd_79(SCR_OPE *op) {

	SINT32	cddaend;

	if (scr_getval(op, &cddaend) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_cddaset(2, cddaend);
	return(GAMEEV_SUCCESS);
}

