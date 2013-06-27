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
#include	"savefile.h"
#include	"isf_cmd.h"


// HLN : 変数の数をセット (Nonaka.K)
int isfcmd_40(SCR_OPE *op) {

	FLAGS	flags;
	UINT16	vals;
	int		size;

	flags = &gamecore.flags;

	if (scr_getword(op, &vals) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}

	if (flags->val) {
		_MFREE(flags->val);
		flags->maxval = 0;
	}
	size = (vals + 1) * sizeof(int);
	flags->val = (int *)_MALLOC(size, "value table");
	if (flags->val == NULL) {
		return(GAMEEV_MEMORYERROR);
	}
	ZeroMemory(flags->val, size);
	flags->maxval = (vals + 1);
	return(GAMEEV_SUCCESS);
}


// HS : 変数に値を代入 (T.Yui)
int isfcmd_41(SCR_OPE *op) {

	UINT16	num;
	SINT32	val;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getval(op, &val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_valset(num, val);
	return(GAMEEV_SUCCESS);
}


// HINC : 変数をインクリメント (T.Yui)								DRS cmd:43
int isfcmd_42(SCR_OPE *op) {

	UINT16	num;
	SINT32	val;

	if (scr_getword(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_valget(num, &val) == SUCCESS) {
		scr_valset(num, val + 1);
	}
	return(GAMEEV_SUCCESS);
}


// HDEC : 変数をデクリメント (T.Yui)								DRS cmd:44
int isfcmd_43(SCR_OPE *op) {

	UINT16	num;
	SINT32	val;

	if (scr_getword(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_valget(num, &val) == SUCCESS) {
		scr_valset(num, val - 1);
	}
	return(GAMEEV_SUCCESS);
}


// CALC : 計算する (Nonaka.K)
int isfcmd_44(SCR_OPE *op) {

	UINT16	num;
	SINT32	val;
	SINT32	val2;
	BYTE	op1;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &op1) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	val = 0;
	val2 = 0;
	while(op1 < 5) {
		int		v;
		BYTE	op2;

		if ((scr_getval(op, &v) != SUCCESS) ||
			(scr_getbyte(op, &op2) != SUCCESS)) {
			return(GAMEEV_WRONGLENG);
		}
		if ((op2 & 0x0f) < 2) {
			switch(op1) {
				case 0:
					val2 += v;
					break;
				case 1:
					val2 -= v;
					break;
				case 2:
					val2 += v * val;
					val = 0;
					break;
				case 3:
					if (v) {
						val2 += (val / v);
					}
					val = 0;
					break;
				case 4:
					if (v) {
						val2 += (val % v);
					}
					val = 0;
					break;
			}
		}
		else {
			switch(op1) {
				case 0:
					val = v;
					break;
				case 1:
					val = -v;
					break;
				case 2:
					val *= v;
					break;
				case 3:
					if (v) {
						val /= v;
					}
					else {
						val = 0;
					}
					break;
				case 4:
					if (v) {
						val %= v;
					}
					else {
						val = 0;
					}
					break;
			}
		}
		op1 = op2;
	}
	scr_valset(num, val + val2);

	return(GAMEEV_SUCCESS);
}


// HSG : 変数にまとめて値を代入 Nonaka.K, T.Yui
int isfcmd_45(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	SINT32	val;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS) ||
		(scr_getval(op, &val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	while(from <= to) {					// こうじゃないかな？
		scr_valset(from++, val);
	}
	return(GAMEEV_SUCCESS);
}


// HT : 変数の転送
int isfcmd_46(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	UINT16	size;
	SINT32	val;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS) ||
		(scr_getword(op, &size) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (from < to) {
		from += size;
		to += size;
		while(size--) {
			scr_valget(--from, &val);
			scr_valset(--to, val);
		}
	}
	else {
		while(size--) {
			scr_valget(from++, &val);
			scr_valset(to++, val);
		}
	}
	return(GAMEEV_SUCCESS);
}


// IF : IF-THEN (Nonaka.K)
int isfcmd_47(SCR_OPE *op) {

	SINT32	val1;
	SINT32	val2;
	BYTE	cmd;
	BYTE	cmd2;
	BOOL	flag;

	while(1) {
		if ((scr_getval(op, &val1) != SUCCESS) ||
			(scr_getbyte(op, &cmd) != SUCCESS) ||
			(scr_getval(op, &val2) != SUCCESS)) {
			return(GAMEEV_WRONGLENG);
		}
		flag = FALSE;
		switch(cmd) {
			case 0:		// e
				if (val1 == val2) {
					flag = TRUE;
				}
				break;

			case 1:		// l
				if (val1 < val2) {
					flag = TRUE;
				}
				break;

			case 2:		// le
				if (val1 <= val2) {
					flag = TRUE;
				}
				break;

			case 3:		// g
				if (val1 > val2) {
					flag = TRUE;
				}
				break;

			case 4:		// ge
				if (val1 >= val2) {
					flag = TRUE;
				}
				break;

			case 5:		// ne
				if (val1 != val2) {
					flag = TRUE;
				}
				break;

			default:
				return(GAMEEV_FAILURE);
		}
		if (!flag) {
			break;
		}

		if (scr_getbyte(op, &cmd2) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		if (cmd2 == 0x00) {
			UINT16 ptr;
			if (scr_getword(op, &ptr) != SUCCESS) {
				return(GAMEEV_WRONGLENG);
			}
			scr_jump(ptr);
			break;
		}
		else if (cmd2 == 0x01) {
			UINT16 ptr;
			SINT32 val;
			if ((scr_getword(op, &ptr) != SUCCESS) ||
				(scr_getval(op, &val) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			scr_valset(ptr, val);
			break;
		}
		else if (cmd2 == 0xff) {
			break;
		}
	}
	return(GAMEEV_SUCCESS);
}


// EXA : フラグと変数を別途に記憶する領域を確保 Nonaka.K T.Yui
int isfcmd_48(SCR_OPE *op) {

	FLAGS	flags;
	UINT16	exavals;
	UINT16	exaflags;
	SAVEHDL	sh;

	flags = &gamecore.flags;

	if ((scr_getword(op, &exaflags) != SUCCESS) ||			// ここ逆…
		(scr_getword(op, &exavals) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_exaclear();

	flags->exaflag = variant_create(exaflags, VARIANT_BIT);
	flags->exaval = variant_create(exavals, VARIANT_SINT32);
	TRACEOUT(("exa - %d, %d", exavals, exaflags));

	sh = savefile_open(FALSE);
	sh->readexaflag(sh, flags->exaflag);
	sh->readexaval(sh, flags->exaval);
	sh->close(sh);
	return(GAMEEV_SUCCESS);
}


// EXS : EXAコマンドで確保した領域から指定フラグ／変数を書込 (Nonaka.K)
int isfcmd_49(SCR_OPE *op) {

	FLAGS	flags;
	SINT32	to;
	SINT32	from;
	SINT32	num;
	BYTE	kind;

	flags = &gamecore.flags;

	if ((scr_getval(op, &to) != SUCCESS) ||
		(scr_getval(op, &from) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &kind) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	switch(kind) {
		case 0:
			while(num-- > 0) {
				BYTE b;
				if (scr_flagget(from++, &b) == SUCCESS) {
					variant_set(flags->exaflag, to++, b);
				}
			}
			break;

		case 1:
			while(num-- > 0) {
				int	v;
				if (scr_valget(from++, &v) == SUCCESS) {
					variant_set(flags->exaval, to++, v);
				}
			}
			break;
	}
	return(GAMEEV_SUCCESS);
}


// EXC : EXAコマンドで確保した領域から指定フラグ／変数に読込 (Nonaka.K)
int isfcmd_4a(SCR_OPE *op) {

	FLAGS	flags;
	SINT32	from;
	SINT32	to;
	SINT32	num;
	BYTE	kind;

	flags = &gamecore.flags;

	if ((scr_getval(op, &from) != SUCCESS) ||
		(scr_getval(op, &to) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &kind) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	switch(kind) {
		case 0:
			while(num-- > 0) {
				BYTE b;
				if (variant_get(flags->exaflag, from++, &b) == SUCCESS) {
					scr_flagop(to++, b);
				}
			}
			break;

		case 1:
			while(num-- > 0) {
				int	v;
				if (variant_get(flags->exaval, from++, &v) == SUCCESS) {
					scr_valset(to++, v);
				}
			}
			break;
	}
	return(GAMEEV_SUCCESS);
}


// SCP : システム変数のコピー
int isfcmd_4b(SCR_OPE *op) {

	UINT16	num;
	BYTE	cmd;
	SINT32	val;
	TEXTWIN	textwin;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	val = 0;
	switch(cmd) {
		case 0x09:		// anime?
			val = (gamecore.anime.ganenable != NULL)?1:0;
			break;

		case 0x14:
			textwin = textwin_getwin(0);
			if (textwin) {
				val = textwin->textctrl.intext;
			}
			break;

		default:
			return(GAMEEV_FORCE);
	}
	scr_valset(num, val);
	return(GAMEEV_SUCCESS);
}


// SSP : システム変数にパラメータをコピーする
int isfcmd_4c(SCR_OPE *op) {

	UINT16	num;
	BYTE	cmd;
	SINT32	val;
	TEXTWIN	textwin;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (scr_valget(num, &val) == SUCCESS) {
		if (cmd == 0x14) {
			textwin = textwin_getwin(0);
			if (textwin) {
				textwin->textctrl.intext = val;
			}
		}
	}
	return(GAMEEV_SUCCESS);
}

