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
#include	"savefile.h"
#include	"bmpdata.h"
#include	"isf_cmd.h"


// GGE : グレースケールを使用したエフェクト (T.Yui)
int isfcmd_60(SCR_OPE *op) {

	EFFECT	ef;
	SINT32	type;
	SINT32	num;
	SINT32	dir;
	SINT32	tick;
	SINT32	leng;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getval(op, &type) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS) ||
		(scr_getval(op, &dir) != SUCCESS) ||
		(scr_getval(op, &leng) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((type < 0) || (type >= 3) ||
		(num < 0) || (num >= GAMECORE_MAXVRAM) ||
		((type == 2) && (leng <= 0))) {
		goto cmd60_exit;
	}

	ef = &gamecore.ef;
	ZeroMemory(ef, sizeof(gamecore.ef));
	ef->src = gamecore.vram[num];
	ef->bmp = bmpdata_load8(label);
	ef->param = tick;
	ef->ex.eg.type = type;
	ef->ex.eg.dir = dir & 1;
	ef->ex.eg.leng = leng;
	return(GAMEEV_GRAYSCALE);

cmd60_exit:
	return(GAMEEV_SUCCESS);
}


// GPE : 拡大・縮小処理 (T.Yui)
int isfcmd_61(SCR_OPE *op) {

	BYTE	num;
	SINT32	snum;
	RECT_U	srect;
	SINT32	dnum;
	RECT_U	drect;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getval(op, &snum) != SUCCESS) ||
		(scr_getrect(op, &srect) != SUCCESS) ||
		(scr_getval(op, &dnum) != SUCCESS) ||
		(scr_getrect(op, &drect) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((snum >= 0) && (snum < GAMECORE_MAXVRAM) &&
		(dnum >= 0) && (dnum < GAMECORE_MAXVRAM)) {
		vramdraw_scrn2rect(&srect.s, &srect.r);
		vramdraw_scrn2rect(&drect.s, &drect.r);
		vrammix_resize(gamecore.vram[dnum], &drect.r,
						gamecore.vram[snum], &srect.r);
		effect_vramdraw(dnum, &drect.r);
	}
	return(GAMEEV_SUCCESS);
}


// GSCRL : スクロール処理 (T.Yui)
static BOOL setscrpt(POINT_T *pt, const RECT_T *src, const RECT_T *dst) {

	int		pos;

	pos = src->right - (dst->right - dst->left);
	if (pos < src->left) {
		return(FAILURE);
	}
	if (pt->x < src->left) {
		pt->x = src->left;
	}
	else if (pt->x > pos) {
		pt->x = pos;
	}
	pos = src->bottom - (dst->bottom - dst->top);
	if (pos < src->top) {
		return(FAILURE);
	}
	if (pt->y < src->top) {
		pt->y = src->top;
	}
	else if (pt->y > pos) {
		pt->y = pos;
	}
	return(SUCCESS);
}

int isfcmd_62(SCR_OPE *op) {

	EFFECT	ef;
	BYTE	cmd;
	SINT32	num;
	RECT_U	src;
	RECT_U	dst;
	POINT_T	from;
	POINT_T	to;
	SINT32	tick;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getrect(op, &src) != SUCCESS) ||
		(scr_getrect(op, &dst) != SUCCESS) ||
		(scr_getpt(op, &from) != SUCCESS) ||
		(scr_getpt(op, &to) != SUCCESS) ||
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
		vramdraw_halfpoint(&from);
		vramdraw_halfpoint(&to);
#endif
		if ((setscrpt(&from, &ef->r, &ef->r2) == SUCCESS) &&
			(setscrpt(&to, &ef->r, &ef->r2) == SUCCESS)) {
			ef->param = tick & 0xffff;
			ef->ex.es.start.x = from.x;
			ef->ex.es.step.x = to.x - from.x;
			ef->ex.es.last.x = 0 - ef->ex.es.step.x;
			ef->ex.es.start.y = from.y;
			ef->ex.es.step.y = to.y - from.y;
			ef->ex.es.last.y = 0 - ef->ex.es.step.y;
			ef->tick = GETTICK();
			return(GAMEEV_SCROLL);
		}
	}
	return(GAMEEV_SUCCESS);
}


// GV : 画面揺らし処理 (T.Yui)
int isfcmd_63(SCR_OPE *op) {

	UINT16	count;
	BYTE	x, y;
	SINT32	tick;
	EFFECT	ef;

	if ((scr_getword(op, &count) != SUCCESS) ||
		(scr_getbyte(op, &x) != SUCCESS) ||
		(scr_getbyte(op, &y) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (count) {
		ef = &gamecore.ef;
		ZeroMemory(ef, sizeof(gamecore.ef));
		ef->param = tick;
		ef->ex.eq.cnt = count * 2;
#ifdef SIZE_QVGA
		ef->pt.x = x;
		ef->pt.y = y;
#else
		ef->pt.x = vramdraw_half(x);
		ef->pt.y = vramdraw_half(y);
#endif
		ef->progress = 0;
		ef->tick = GETTICK();
		ef->src = gamecore.vram[gamecore.dispwin.vramnum];
		return(GAMEEV_QUAKE);
	}
	return(GAMEEV_SUCCESS);
}


// GAL : アニメーションループ設定 (T.Yui)
int isfcmd_64(SCR_OPE *op) {

	SINT32	num;
	BYTE	loop;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &loop) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	anime_setloop(num, loop);
	return(GAMEEV_SUCCESS);
}


// GAOPEN : アニメーションファイルのオープン (T.Yui)
int isfcmd_65(SCR_OPE *op) {

	SINT32	num;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("anime open: %d %s", num, label));
	anime_open(num, label);
	return(GAMEEV_SUCCESS);
}


// GASET : アニメーションデータのセット (T.Yui)
int isfcmd_66(SCR_OPE *op) {

	SINT32	num;
	SINT32	major;
	SINT32	minor;
	SINT32	reg;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getval(op, &major) != SUCCESS) ||
		(scr_getval(op, &minor) != SUCCESS) ||
		(scr_getval(op, &reg) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("anime data set %d %d %d %d", num, major, minor, reg));
	anime_setdata(num, major, minor, reg);
	return(GAMEEV_SUCCESS);
}


// GAPOS : アニメーションの表示位置のセット (T.Yui)
int isfcmd_67(SCR_OPE *op) {

	SINT32	num;
	UINT16	x;
	UINT16	y;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getword(op, &x) != SUCCESS) ||
		(scr_getword(op, &y) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	// 家族計画だとダミーになってるんだけど…位置セットしなくていいの？
	// anime_setloc(num, x, y);
	return(GAMEEV_SUCCESS);
}


// GACLOSE : アニメーションファイルのクローズ (T.Yui)
int isfcmd_68(SCR_OPE *op) {

	anime_close();
	(void)op;
	return(GAMEEV_SUCCESS);
}


// GADELETE :アニメーションの削除 (T.Yui)
int isfcmd_69(SCR_OPE *op) {

	SINT32	num;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("anime del %d", num));
	anime_trush(num);
	return(GAMEEV_SUCCESS);
}


// SGL : セーブイメージを読み込む (T.Yui)
int isfcmd_6f(SCR_OPE *op) {

	SINT32		savenum;
	SINT32		num;
	POINT_T		pt;
	VRAMHDL		dst;
	SAVEHDL		sh;
	_SAVEINF	inf;
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

	sh = savefile_open(FALSE);
	r = sh->readinf(sh, savenum, &inf, -1, -1);
	sh->close(sh);
	if (r == SUCCESS) {
#ifdef SIZE_QVGA
		vramdraw_halfpoint(&pt);
#endif
		vramcpy_cpy(dst, (VRAMHDL)inf.preview, &pt, NULL);
		vram_destroy((VRAMHDL)inf.preview);
	}

cmd6f_exit:
	return(GAMEEV_SUCCESS);
}

