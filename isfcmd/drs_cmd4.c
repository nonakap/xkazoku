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


// HSG : 変数にまとめて値を代入
int drscmd_42(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	UINT32	val;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS) ||
		(scr_getdword(op, &val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	switch(val >> 30) {
		case 0:
			while(from <= to) {
				scr_valset(from++, val);
			}
			break;

		default:
			return(GAMEEV_FAILURE);
	}
	return(GAMEEV_SUCCESS);
}


// CALC : 計算する
int drscmd_46(SCR_OPE *op) {

	UINT16	pos;
	UINT16	leng;
	BYTE	cmd;
	BYTE	func;
	SINT32	array[32];
	int		arrays;
	SINT32	val;
	SINT32	val2;

	if ((scr_getword(op, &pos) != SUCCESS) ||
		(scr_getword(op, &leng) != SUCCESS)) {
		goto dc46_err;
	}
	arrays = 0;
	while(leng) {
		if (scr_getbyte(op, &cmd) != SUCCESS) {
			goto dc46_err;
		}
		leng--;
		if (cmd == 0) {
			if (leng < 4) {
				break;
			}
			leng -= 4;
			if (scr_getval(op, &val) != SUCCESS) {
				goto dc46_err;
			}
		}
		else {
			if (leng) {
				leng--;
				if (scr_getbyte(op, &func) != SUCCESS) {
					goto dc46_err;
				}
			}
			else {
				func = 0xff;
			}
			if (arrays) {
				val2 = array[--arrays];
			}
			else {
				val2 = 0;
			}
			if (arrays) {
				val = array[--arrays];
			}
			else {
				val = 0;
			}
			switch(func) {
				case 0:
					val += val2;
					break;
				case 1:
					val -= val2;
					break;
				case 2:
					val *= val2;
					break;
				case 3:
					if (val2) {
						val /= val2;
					}
					else {
						val = 0;
					}
					break;
				case 4:
					if (val2) {
						val %= val2;
					}
					else {
						val = 0;
					}
					break;
			}
		}
		if (arrays < (int)((sizeof(array)/sizeof(int)))) {
			array[arrays++] = val;
		}
	}

	if (arrays) {
		val = array[--arrays];
	}
	else {
		val = 0;
	}
	TRACEOUT(("val[%d] = %d", pos, val));
	scr_valset(pos, val);
	return(GAMEEV_SUCCESS);

dc46_err:
	return(GAMEEV_WRONGLENG);
}

