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
#include	"arcfile.h"
#include	"cgload.h"
#include	"sound.h"
#include	"cddamng.h"


// VSET : 仮想ＶＲＡＭの設定 Nonaka.K, T.Yui
int isfcmd_50(SCR_OPE *op) {

	SINT32	num;
	POINT_T	pt;
	VRAMHDL	*vram;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		vram = gamecore.vram + num;
		if (*vram) {
			vram_destroy(*vram);
		}
#ifdef SIZE_QVGA
		vramdraw_halfsize(&pt);
#endif
		*vram = vram_create(pt.x, pt.y, FALSE, DEFAULT_BPP);
	}
	return(GAMEEV_SUCCESS);
}


// GN : グラフィック表示オン (Nonaka.K)
int isfcmd_51(SCR_OPE *op) {

	SINT32	num;
	POINT_T	pt;
	DISPWIN	dispwin;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	dispwin = &gamecore.dispwin;
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		dispwin->flag |= DISPWIN_VRAM;
		dispwin->vramnum = num;
		dispwin->posx = pt.x;
		dispwin->posy = pt.y;
		vramdraw_setrect(gamecore.vram[num], NULL);
		vramdraw_draw();
	}
	else {
		dispwin->flag &= ~DISPWIN_VRAM;
	}
	TRACEOUT(("vram disp: %d", num));
	return(GAMEEV_SUCCESS);
}


// GF : グラフィック表示オフ (T.Yui)
int isfcmd_52(SCR_OPE *op) {

	DISPWIN	dispwin;

	dispwin = &gamecore.dispwin;
	dispwin->flag &= ~DISPWIN_VRAM;
	vramdraw_setrect(gamecore.vram[dispwin->vramnum], NULL);
	vramdraw_draw();

	(void)op;
	return(GAMEEV_SUCCESS);
}


// GC : グラフィッククリア (T.Yui)
int isfcmd_53(SCR_OPE *op) {

	SINT32	num;
	BYTE	col[4];

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &col[0]) != SUCCESS) ||
		(scr_getbyte(op, &col[1]) != SUCCESS) ||
		(scr_getbyte(op, &col[2]) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
#if 1
	col[3] = 0;
#else
	// Lien->24bit 家族計画->32bit? (チェックする事)
	if (scr_getbyte(op, &col[3]) != SUCCESS) {
		col[3] = 0;
	}
#endif
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		vram_fill(gamecore.vram[num], NULL,
							MAKEPALETTE(col[0], col[1], col[2]), col[3]);
	}
	return(GAMEEV_SUCCESS);
}


// GO : グラフィックフェードアウト (T.Yui)
int isfcmd_55(SCR_OPE *op) {

	SINT32	tick;
	BYTE	col[3];
	BYTE	cmd = 0;

	if ((scr_getval(op, &tick) != SUCCESS) ||
		(scr_getbyte(op, &col[0]) != SUCCESS) ||
		(scr_getbyte(op, &col[1]) != SUCCESS) ||
		(scr_getbyte(op, &col[2]) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	if (gamecore.sys.version >= EXEVER_CRES) {
		if (scr_getbyte(op, &cmd) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}

	if (cmd) {
		cddamng_stop(tick);
		soundmix_stop(SOUNDTRK_CDDA, tick);
	}
	return(effect_fadeout(tick, col));
}


// GL : グラフィックロード表示 (Nonaka.K)							DRS cmd:55
int isfcmd_56(SCR_OPE *op) {

	SINT32	num;
	char	label[ARCFILENAME_LEN+1];
	DISPWIN	dispwin;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("load: %d %-12s", num, label));
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		cgload_data(&gamecore.vram[num], ARCTYPE_GRAPHICS, label);
		dispwin = &gamecore.dispwin;
		if ((dispwin->flag & DISPWIN_VRAM) && (dispwin->vramnum == num)) {
			vramdraw_setrect(gamecore.vram[dispwin->vramnum], NULL);
			vramdraw_draw();
		}
	}
	return(GAMEEV_SUCCESS);
}


// GP : グラフィックのコピー Nonaka.K, T.Yui
int isfcmd_57(SCR_OPE *op) {

	BYTE	cmd;
	SINT32	src;
	RECT_U	scrn;
	SINT32	param;
	POINT_T	pt;
	EFFECT	ef;

	ef = &gamecore.ef;
	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getval(op, &src) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getval(op, &param) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
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


// GB : 矩形を描画 (T.Yui)
int isfcmd_58(SCR_OPE *op) {

	SINT32	num;
	BYTE	col[4];
	RECT_U	r;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &col[0]) != SUCCESS) ||
		(scr_getbyte(op, &col[1]) != SUCCESS) ||
		(scr_getbyte(op, &col[2]) != SUCCESS) ||
		(scr_getbyte(op, &col[3]) != SUCCESS) ||
		(scr_getrect(op, &r) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		vramdraw_scrn2rect(&r.s, &r.r);
		vram_fill(gamecore.vram[num], &r.r,
							MAKEPALETTE(col[0], col[1], col[2]), col[3]);
	}
	return(GAMEEV_SUCCESS);
}


// GPB : 文字サイズ設定 (T.Yui)
int isfcmd_59(SCR_OPE *op) {

	SINT32	size;

	if (scr_getval(op, &size) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textctrl_setsize(&gamecore.textdraw, size);
	return(GAMEEV_FORCE);
}


// GPJ : 文字形態の設定 (T.Yui)
int isfcmd_5a(SCR_OPE *op) {

	BYTE	type;

	if (scr_getbyte(op, &type) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textctrl_settype(&gamecore.textdraw, type);
	return(GAMEEV_FORCE);
}


// PR : 文字表示 (T.Yui)
int isfcmd_5b(SCR_OPE *op) {

	SINT32	num;
	POINT_T	pt;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textdisp_draw(num, &pt, op);
	return(GAMEEV_SUCCESS);
}


// GASTART: アニメーションのスタート (T.Yui)
int isfcmd_5c(SCR_OPE *op) {

	ANIME	anime;

	TRACEOUT(("anime start"));

	anime = &gamecore.anime;
	if (!anime->enable) {
		anime->enable = TRUE;
		anime->frame = 0;
		anime->basetick = GETTICK();
		anime->nexttick = 0;
	}
	(void)op;
	return(GAMEEV_SUCCESS);
}


// GASTOP : アニメーションのストップ (T.Yui)
int isfcmd_5d(SCR_OPE *op) {

	ANIME	anime;

	anime = &gamecore.anime;
	anime->enable = FALSE;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// GPI : グラフィックエフェクトとBGMのフェードイン
int isfcmd_5e(SCR_OPE *op) {

	int		r;
	SINT32	cmd;
	SINT32	track;

	r = isfcmd_57(op);
	if (r >= GAMEEV_SUCCESS) {
		if ((scr_getval(op, &cmd) == SUCCESS) &&
			(scr_getval(op, &track) == SUCCESS)) {
			sndplay_cddaplay(track, 0, gamecore.ef.param);
		}
	}
	return(r);
}


// GPO : グラフィックエフェクトとBGMのフェードアウト (T.Yui)
int isfcmd_5f(SCR_OPE *op) {

	int		r;

	r = isfcmd_57(op);
	if (r >= GAMEEV_SUCCESS) {
		sndplay_cddastop(gamecore.ef.param);
	}
	return(r);
}

