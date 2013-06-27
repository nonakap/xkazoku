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
#include	"sound.h"
#include	"sstream.h"


// SEP : 効果音の再生 (T.Yui)
int drscmd_77(SCR_OPE *op) {

	SINT32	num;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		soundmix_rew(SOUNDTRK_SE + num);
		soundmix_play(SOUNDTRK_SE + num, 0, 0);
	}

	return(GAMEEV_SUCCESS);
}

