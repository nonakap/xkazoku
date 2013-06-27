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


// JS : サブルーチンジャンプ
int drscmd_05(SCR_OPE *op) {

	UINT16	bptr;
	UINT16	ptr;

	if ((scr_getword(op, &bptr) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_call(ptr)) {
		return(GAMEEV_FAILURE);
	}
	return(GAMEEV_SUCCESS);
}


// RT : サブルーチンから復帰
int drscmd_06(SCR_OPE *op) {

	if (scr_ret()) {
		return(GAMEEV_FAILURE);
	}

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ONJP : 条件ジャンプ
int drscmd_07(SCR_OPE *op) {

	SINT32	val;
	UINT16	limit;

	if ((scr_getval(op, &val) != SUCCESS) ||
		(scr_getword(op, &limit) != SUCCESS)) {
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


// ONJS : 条件サブルーチン呼び出し
int drscmd_08(SCR_OPE *op) {

	SINT32	val;
	UINT16	limit;

	if ((scr_getval(op, &val) != SUCCESS) ||
		(scr_getword(op, &limit) != SUCCESS)) {
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

