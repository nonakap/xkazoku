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


// ---- not supported

// IM : マウスカーソルデータの読み込み
int isfcmd_80(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// IXY : マウスの位置変更
int isfcmd_83(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ---- todo

// CTEXT : テキスト表示
int isfcmd_1c(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// WW : クリック待設定
int isfcmd_23(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// SETINSIDEVOL : 内部音量設定
int isfcmd_9f(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// KIDCLR : 既読文章の初期化
int isfcmd_a0(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDMOJI : 既読文章の文字の色を設定する
int isfcmd_a1(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDPAGE : 既読文章の頁情報
int isfcmd_a2(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDHABA : 既読文章の１行あたりの文字数
int isfcmd_a6(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

