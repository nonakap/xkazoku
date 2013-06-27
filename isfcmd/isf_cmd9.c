// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"cddamng.h"
#include	"gamecore.h"
#include	"isf_cmd.h"


// DAE : CDDA‚ÌÝ’è (T.Yui)
int isfcmd_90(SCR_OPE *op) {

	SINT32	cddastart;
	SINT32	cddaend;

	if ((scr_getval(op, &cddastart) != SUCCESS) ||
		(scr_getval(op, &cddaend) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_cddaset(cddastart, cddaend);
	return(GAMEEV_SUCCESS);
}


// DAP : CDDA‚ÌÄ¶ (T.Yui)
int isfcmd_91(SCR_OPE *op) {

	SINT32	track;
	BYTE	cmd;
	SINT32	tick = 0;

	if ((scr_getval(op, &track) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (gamecore.sys.version >= EXEVER_KAZOKU) {
		if (scr_getval(op, &tick) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	sndplay_cddaplay(track, cmd, tick);
	return(GAMEEV_SUCCESS);
}


// DAS : CDDA‚Ì’âŽ~ (T.Yui)
int isfcmd_92(SCR_OPE *op) {

	SINT32	tick;

	if (scr_getval(op, &tick) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_cddastop(tick);

	return(GAMEEV_SUCCESS);
}

