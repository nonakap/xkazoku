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



// IC : マウスカーソルの変更 (T.Yui)
int isfcmd_81(SCR_OPE *op) {

	SINT32	type;
	BYTE	type2;
	int		version;

	version = gamecore.sys.version;
	if ((version == EXEVER_KONYA2) || (version >= EXEVER_HEART)) {
		if (scr_getval(op, &type) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	else {
		if (scr_getbyte(op, &type2) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		type = type2;
	}
	type = (type + CUR_SYSTEM) % (CUR_SYSTEM + CUR_USER);
	gamecore.mouseevt.curnum = type;
	return(GAMEEV_SUCCESS);
}


// IMS : マウス移動範囲の設定 (T.Yui)
int isfcmd_82(SCR_OPE *op) {

	RECT_U		scrn;
	MOUSEEVT	mouseevt;

	if (scr_getrect(op, &scrn) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	mouseevt = &gamecore.mouseevt;
	if ((scrn.s.width <= 0) || (!scrn.s.height <= 0)) {
		mouseevt->flag &= ~MEVT_CLIP;
		mouseevt->flag |= MEVT_RENEWAL;
	}
	else {
		mouseevt->flag |= MEVT_CLIP | MEVT_RENEWAL;
		mouseevt->clip.left = scrn.s.left;
		mouseevt->clip.top = scrn.s.top;
		mouseevt->clip.right = scrn.s.left + scrn.s.width;
		mouseevt->clip.bottom = scrn.s.top + scrn.s.height;
	}
	TRACEOUT(("cmd82: renewal"));
	return(GAMEEV_SUCCESS);
}


// IH : IGコマンドの選択範囲指定 (Nonaka.K)
int isfcmd_84(SCR_OPE *op) {

	BYTE		num;
	REGIONPRM	rgn;
	RECT_U		r;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &r) != SUCCESS) ||
		(scr_getbyte(op, &rgn.kind) != SUCCESS) ||
		(scr_getword(op, &rgn.num) != SUCCESS) ||
		(scr_getbyte(op, &rgn.c2) != SUCCESS) ||
		(scr_getbyte(op, &rgn.c3) != SUCCESS) ||
		(scr_getbyte(op, &rgn.c4) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	num &= 0x3f;
	if (num < GAMECORE_MAXRGN) {
		MOUSEEVT mouseevt = &gamecore.mouseevt;
#ifdef SIZE_QVGA
		vramdraw_halfrect(&r.r);
#endif
		rgn.r = r.r;
		mouseevt->rgn[num] = rgn;
		mouseevt->flag |= MEVT_RENEWAL;
	}
	return(GAMEEV_SUCCESS);
}


// IG : 画面内マウス入力 Nonaka.K, T.Yui
int isfcmd_85(SCR_OPE *op) {

	MOUSEPRM	prm;

	if ((scr_getword(op, &prm.cur) != SUCCESS) ||
		(scr_getword(op, &prm.btn) != SUCCESS) ||
		(scr_getbyte(op, &prm.cnt) != SUCCESS) ||
		(scr_getbyte(op, &prm.btnflg) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	gamecore.mouseevt.prm = prm;
	return(GAMEEV_WAITMOUSE);
}


// IGINIT : 画面内マウス入力−初期化
int isfcmd_86(SCR_OPE *op) {

	event_mouserel(1);
	(void)op;
	return(GAMEEV_SUCCESS);
}


// IGRELEASE : 画面内マウス入力−解放
int isfcmd_87(SCR_OPE *op) {

	event_mouserel(0);
	(void)op;
	return(GAMEEV_SUCCESS);
}


// IHK : キーボード拡張−移動先データの設定 (T.Yui)
int isfcmd_88(SCR_OPE *op) {

	BYTE		num;
	int			i;
	REGIONKEY	key;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	for (i=0; i<8; i++) {
		if (scr_getval(op, key.move + i) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	num &= 0x3f;
	if (num < GAMECORE_MAXRGN) {
		key.enable = TRUE;
		gamecore.mouseevt.key[num] = key;
	}
	return(GAMEEV_SUCCESS);
}


// IHKDEF : キーボード拡張−デフォルト番号の設定
int isfcmd_89(SCR_OPE *op) {

	SINT32		defpos;
	MOUSEEVT	mouseevt;

	if (scr_getval(op, &defpos) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	mouseevt = &gamecore.mouseevt;
	mouseevt->keypos = -1;
	mouseevt->defpos = defpos;
	return(GAMEEV_SUCCESS);
}


// IHGL : 選択レイアウト画像イメージ読込
int isfcmd_8a(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN+1];
	POINT_T	pt;

	if ((scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getpt(op, &pt) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
#ifndef SUPPORT_PPCARC
	cgload_mask(&gamecore.mouseevt.map, ARCTYPE_GRAPHICS, label);
#endif
	return(GAMEEV_SUCCESS);
}


// IHGC : 選択レイアウトゼロクリア
int isfcmd_8b(SCR_OPE *op) {

	MOUSEEVT	mouseevt;

	mouseevt = &gamecore.mouseevt;
	vram_destroy(mouseevt->map);
	mouseevt->map = NULL;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// CLK : クリック待ち
int isfcmd_8d(SCR_OPE *op) {

	BYTE	dummy;
	SINT32	num;

	if ((scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_MSGCLK);
}


// IGN : カーソルＮＯ取得 (T.Yui)
int isfcmd_8e(SCR_OPE *op) {

	SINT32	num;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	scr_valset(num, gamecore.mouseevt.curnum - CUR_SYSTEM);
	return(GAMEEV_SUCCESS);
}


// IHKINIT : キーボード拡張−移動先データ設定の初期化
int isfcmd_8f(SCR_OPE *op) {

	(void)op;
	ZeroMemory(gamecore.mouseevt.key, sizeof(gamecore.mouseevt.key));
	return(GAMEEV_SUCCESS);
}

