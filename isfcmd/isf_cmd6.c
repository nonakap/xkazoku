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
#include	"cgload.h"
#include	"isf_cmd.h"
#include	"bmpdata.h"
#include	"savefile.h"


// GGE : グレースケールを使用したエフェクト
int isfcmd_60(SCR_OPE *op) {

	EFFECT	ef;
	SINT32	cmd1, cmd2;
	SINT32	num;
	SINT32	cnt;
	SINT32	tick;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getval(op, &cmd1) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS) ||
		(scr_getval(op, &cmd2) != SUCCESS) ||
		(scr_getval(op, &cnt) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
//	TRACEOUT(("cnt=%d", cnt));
	if (cnt <= 0) {
		cnt = 1;
	}
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		ef = &gamecore.ef;
		ZeroMemory(ef, sizeof(gamecore.ef));
		ef->src = gamecore.vram[num];
		ef->bmp = bmpdata_load8(label);
		ef->param = tick * 2;
		ef->param2 = cmd2 & 1;
		return(GAMEEV_GRAYSCALE);
	}
	return(GAMEEV_SUCCESS);
}


// GSCRL : スクロール処理
int isfcmd_62(SCR_OPE *op) {

	EFFECT	ef;
	BYTE	cmd;
	SINT32	num;
	RECT_U	src;
	RECT_U	dst;
	SINT32	dummy;
	SINT32	from;
	SINT32	to;
	SINT32	tick;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getrect(op, &src) != SUCCESS) ||
		(scr_getrect(op, &dst) != SUCCESS) ||
		(scr_getval(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &from) != SUCCESS) ||
		(scr_getval(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &to) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num >= 0) && (num < GAMECORE_MAXVRAM)) {
		ef = &gamecore.ef;
		ZeroMemory(ef, sizeof(gamecore.ef));
		ef->src = gamecore.vram[num];
		vramdraw_scrn2rect(&src.s, &ef->r);
		vramdraw_scrn2rect(&dst.s, &ef->r2);
		ef->pt.x = ef->r2.left;
		ef->pt.y = ef->r2.top;
#ifdef SIZE_QVGA
		from = vramdraw_half(from);
		to = vramdraw_half(to);
#endif
		ef->param = from;
		ef->param2 = to - from;
		ef->param3 = tick & 0xffff;
		ef->lastalpha = 0 - ef->param2;
		ef->tick = GETTICK();
		return(GAMEEV_SCROLL);
	}
	return(GAMEEV_SUCCESS);
}


// GV : 画面揺らし処理
int isfcmd_63(SCR_OPE *op) {

	UINT16	num;
	BYTE	x, y;
	SINT32	tick;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &x) != SUCCESS) ||
		(scr_getbyte(op, &y) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	return(GAMEEV_FORCE);
}


// GAL : アニメーションループ設定 (T.Yui)
int isfcmd_64(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// GAOPEN : アニメーションファイルのオープン (T.Yui)
int isfcmd_65(SCR_OPE *op) {

	SINT32	num;
	char	label[ARCFILENAME_LEN+1];
	ANIME	anime;
	GADHDL	gad;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	milstr_ncat(label, ".gad", sizeof(label));
	TRACEOUT(("anime load: %d %-12s", num, label));

	anime = &gamecore.anime;
	anime->enable = FALSE;
	gad_destroy(anime->hdl);
	gad = gad_create(ARCTYPE_GRAPHICS, label);
	anime->hdl = gad;
	if (gad) {
		cgload_data(&gad->vram, ARCTYPE_GRAPHICS, label);
	}
	return(GAMEEV_SUCCESS);
}


// GASET : アニメーションデータのセット (T.Yui)
int isfcmd_66(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// GACLOSE : アニメーションファイルのクローズ (T.Yui)
int isfcmd_68(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// GADELETE :アニメーションの削除 (T.Yui)
int isfcmd_69(SCR_OPE *op) {

	ANIME	anime;

	anime = &gamecore.anime;
	anime->enable = FALSE;
	gad_destroy(anime->hdl);
	anime->hdl = NULL;
	(void)op;
	return(GAMEEV_FORCE);
}


// SGL : セーブイメージを読み込む
int isfcmd_6f(SCR_OPE *op) {

	SINT32		savenum;
	SINT32		num;
	POINT_T		pt;
	VRAMHDL		dst;
	SAVEHDL		fh;
	SAVEINF_T	inf;
	BOOL		r;

	if ((scr_getval(op, &savenum) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((num < 0) || (num >= GAMECORE_MAXVRAM)) {
		goto cmd6f_exit;
	}
	dst = gamecore.vram[num];
	if (dst == NULL) {
		goto cmd6f_exit;
	}

	fh = savefile_open(FALSE);
#ifndef SIZE_QVGA
	r = savefile_readinf(fh, savenum, &inf, 240, 180);
#else
	r = savefile_readinf(fh, savenum, &inf, 160, 90);
#endif
	if (r == SUCCESS) {
#ifdef SIZE_QVGA
		vramdraw_halfpoint(&pt);
#endif
		vramcpy_cpy(dst, (VRAMHDL)inf.preview, &pt, NULL);
		vram_destroy((VRAMHDL)inf.preview);
	}
	savefile_close(fh);

cmd6f_exit:
	return(GAMEEV_SUCCESS);
}

