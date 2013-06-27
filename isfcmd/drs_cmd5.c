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


// 画面フェード
int drscmd_54(SCR_OPE *op) {

	SINT32	tick;
	SINT32	dst;
	SINT32	src;

	if ((scr_getval(op, &tick) != SUCCESS) ||
		(scr_getval(op, &src) != SUCCESS) ||
		(scr_getval(op, &dst) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((dst >= 0) && (dst < GAMECORE_MAXVRAM) &&
		(src >= 0) && (src < GAMECORE_MAXVRAM)) {
		if (src != gamecore.dispwin.vramnum) {
			TRACEOUT(("fade: %d -> %d", src, dst));
			vrammix_cpy(gamecore.vram[dst], gamecore.vram[src], NULL);
			vramdraw_setrect(gamecore.vram[dst], NULL);
			vramdraw_draw();
		}
	}
	return(GAMEEV_SUCCESS);
}

