// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
//                                          ※ ダメなチト順
// -------------------------------------------------------------

#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"isf_cmd.h"


// ED : 終了 (Nonaka.K)
int isfcmd_00(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_EXIT);
}


// LS : シナリオのロード実行 (Nonaka.K)
int isfcmd_01(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN];

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	src_scriptret();	// 強制リターン
	if (scr_scriptcall(label) != SUCCESS) {
		return(GAMEEV_FILENOTFOUND);
	}
	return(GAMEEV_SUCCESS);
}


// LSBS : サブシナリオのロード実行 (Nonaka.K)
int isfcmd_02(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN];

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_scriptcall(label) != SUCCESS) {
		return(GAMEEV_FILENOTFOUND);
	}
	return(GAMEEV_SUCCESS);
}


// SRET : サブシナリオからの復帰 (Nonaka.K)
int isfcmd_03(SCR_OPE *op) {

	if (src_scriptret()) {
		return(GAMEEV_FILENOTFOUND);
	}

	(void)op;
	return(GAMEEV_SUCCESS);
}


// JP : ジャンプ (Nonaka.K)
int isfcmd_04(SCR_OPE *op) {

	UINT16	ptr;

	if (scr_getword(op, &ptr) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_jump(ptr)) {
		return(GAMEEV_FAILURE);
	}
	return(GAMEEV_SUCCESS);
}


// JS : サブルーチンジャンプ (Nonaka.K)
int isfcmd_05(SCR_OPE *op) {

	UINT16	ptr;

	if (scr_getword(op, &ptr) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_call(ptr)) {
		return(GAMEEV_FAILURE);
	}
	return(GAMEEV_SUCCESS);
}


// RT : サブルーチンから復帰 (Nonaka.K)
int isfcmd_06(SCR_OPE *op) {

	if (scr_ret()) {
		return(GAMEEV_FAILURE);
	}

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ONJP : 条件ジャンプ (Nonaka.K)
int isfcmd_07(SCR_OPE *op) {

	SINT32	val;
	BYTE	limit;

	if ((scr_getval(op, &val) != SUCCESS) ||
		(scr_getbyte(op, &limit) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((val >= 0) && (val < (int)limit)) {
		UINT16 ptr;
		if (op->remain < (val*2)) {
			return(GAMEEV_WRONGLENG);
		}
		op->ptr += val * 2;
		op->remain -= val * 2;
		if (scr_getword(op, &ptr) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		if (scr_jump(ptr)) {
			return(GAMEEV_FAILURE);
		}
	}
	return(GAMEEV_SUCCESS);
}


// ONJS : 条件サブルーチン呼び出し (Nonaka.K)
int isfcmd_08(SCR_OPE *op) {

	SINT32	val;
	BYTE	limit;

	if ((scr_getval(op, &val) != SUCCESS) ||
		(scr_getbyte(op, &limit) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((val >= 0) && (val < (int)limit)) {
		UINT16 ptr;
		if (op->remain < (val*2)) {
			return(GAMEEV_WRONGLENG);
		}
		op->ptr += val * 2;
		op->remain -= val * 2;
		if (scr_getword(op, &ptr) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		if (scr_call(ptr)) {
			return(GAMEEV_FAILURE);
		}
	}
	return(GAMEEV_SUCCESS);
}

// CHILD : 子プロセスの実行 (T.Yui)
int isfcmd_09(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN+1];

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}

	// Win32ネイティブコードを対応するなら…
#if 0
	if (!milstr_cmp(label, "PLANETOP")) {
		return(GAMEEV_PLANETOP);
	}
	if (!milstr_cmp(label, "OPENDVD")) {
		return(GAMEEV_OPENDVD);
	}
#endif
	return(GAMEEV_SUCCESS);
}

