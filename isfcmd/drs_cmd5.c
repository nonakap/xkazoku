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
#include	"cgload.h"
#include	"drs_cmd.h"


// 画面フェード
int drscmd_54(SCR_OPE *op) {

	SINT32	tick;
	SINT32	src;
	SINT32	dir;

	if ((scr_getval(op, &tick) != SUCCESS) ||
		(scr_getval(op, &src) != SUCCESS) ||
		(scr_getval(op, &dir) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (!dir) {
		return(effect_fadeinset(tick, src, 0));
	}
	else {
		return(effect_fadeoutset(tick, src, 0));
	}
}


// GL : グラフィックロード表示
static BOOL cgsetcb(void *vpItem, void *vpArg) {

	if (!milstr_cmp((char *)vpArg, ((CGFLAG)vpItem)->name)) {
		scr_flagop(((CGFLAG)vpItem)->bit, 1);
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

int drscmd_55(SCR_OPE *op) {

	SINT32	num;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("load: %d %-12s", num, label));
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		listarray_enum(gamecore.drssys.cgflag, cgsetcb, label);
		milstr_ncpy(gamecore.ggdname[num], label,
												sizeof(gamecore.ggdname[0]));
		cgload_data(&gamecore.vram[num], ARCTYPE_GRAPHICS, label);
		effect_vramdraw(num, NULL);
	}
	return(GAMEEV_SUCCESS);
}


// GP : グラフィックのコピー
int drscmd_56(SCR_OPE *op) {

	BYTE	cmd;
	SINT32	src;
	RECT_U	scrn;
	SINT32	param;
	POINT_T	pt;
	EFFECT	ef;
//	VRAMHDL	vram;

	ef = &gamecore.ef;
	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getval(op, &src) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getval(op, &param) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
#if 0
	if ((src >= 0) && (src < GAMECORE_MAXVRAM)) {
		vram = gamecore.vram[src];
		if (vram) {
			return(effect_set(cmd, vram, &scrn.s, param, &pt, op));
		}
	}
#endif
	if ((gamecore.dispwin.flag & DISPWIN_VRAM) &&
		(src >= 0) && (src < GAMECORE_MAXVRAM) && (gamecore.vram[src])) {
		ZeroMemory(ef, sizeof(EFFECT_T));
		ef->cmd = cmd;
		ef->param = param;
		ef->src = gamecore.vram[src];
#ifdef SIZE_QVGA
		vramdraw_halfpoint(&pt);
#endif
		ef->pt = pt;
		vramdraw_scrn2rect(&scrn.s, &ef->r);
		ef->r2.left = pt.x;
		ef->r2.top = pt.y;
#ifndef SIZE_QVGA
		ef->r2.right = pt.x + scrn.s.width;
		ef->r2.bottom = pt.y + scrn.s.height;
#else
		ef->r2.right = pt.x + (ef->r.right - ef->r.left);
		ef->r2.bottom = pt.y + (ef->r.bottom - ef->r.top);
#endif
		return(effect_set(op));
	}
	return(GAMEEV_SUCCESS);
}

