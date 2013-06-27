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
#include	"savefile.h"
#include	"savedrs.h"
#include	"drs_cmd.h"


// LSBS : サブシナリオのロード実行
int drscmd_02(SCR_OPE *op) {

	UINT16	bptr;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getword(op, &bptr) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_scriptcall(label) != SUCCESS) {
		return(GAMEEV_FILENOTFOUND);
	}
	gamecore.drssys.curpage = 0;
	return(GAMEEV_SUCCESS);
}


// SRET : サブシナリオからの復帰
int drscmd_03(SCR_OPE *op) {

	if (src_scriptret()) {
		return(GAMEEV_FILENOTFOUND);
	}

	(void)op;
	return(GAMEEV_SUCCESS);
}


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

	UINT16	self;
	SINT32	val;
	UINT16	limit;

	if ((scr_getword(op, &self) != SUCCESS) ||
		(scr_getval(op, &val) != SUCCESS) ||
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


// 履歴リセット
int drscmd_0a(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// 履歴スキップページ指定
int drscmd_0b(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ページイン
int drscmd_0c(SCR_OPE *op) {

	UINT16	num;
	int		lastread;
	char	label[ARCFILENAME_LEN+1];

	if (scr_getword(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	gamecore.drssys.curpage = num;
	lastread = 0;
	if (scr_getcurscr(label, sizeof(label)) == SUCCESS) {
		lastread = savedrs_getkid(num, label)?1:0;
		TRACEOUT(("%s %u = %d", label, num, lastread));
	}
	gamecore.gamecfg.lastread = lastread;
	return(GAMEEV_SUCCESS);
}


// ページアウト
int drscmd_0d(SCR_OPE *op) {

	BYTE	num;
	char	label[ARCFILENAME_LEN+1];

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (!gamecore.gamecfg.lastread) {
		if (scr_getcurscr(label, sizeof(label)) == SUCCESS) {
			savedrs_setkid(gamecore.drssys.curpage, label);
		}
	}
	return(GAMEEV_SUCCESS);
}


// 指定フラグ拡張ロード?
int drscmd_0e(SCR_OPE *op) {


	UINT16	from;
	UINT16	to;
	SAVEHDL	sh;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (from <= to) {
		sh = savefile_open(FALSE);
		sh->readsysflagex(sh, from, (UINT)to - (UINT)from + 1);
		sh->close(sh);
	}
	return(GAMEEV_SUCCESS);
}

