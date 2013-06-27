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
#include	"savefile.h"
#include	"gamemsg.h"
#include	"inputmng.h"


// FLN : フラグ数の設定 (Nonaka.K)
int isfcmd_30(SCR_OPE *op) {

	FLAGS	flags;
	UINT16	maxflag;
	int		flagsize;

	flags = &gamecore.flags;
	if (scr_getword(op, &maxflag) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}

	if (flags->flag) {
		_MFREE(flags->flag);
		flags->maxflag = 0;
	}
	flagsize = (int)maxflag + 1;
	flagsize = ((flagsize + 8) / 8) * sizeof(BYTE);		// +8じゃないとダメ
	flags->flag = (BYTE *)_MALLOC(flagsize, "flag table");
	flags->flagsize = flagsize;
	if (flags->flag == NULL) {
		return(GAMEEV_MEMORYERROR);
	}
	ZeroMemory(flags->flag, flagsize);
	flags->maxflag = maxflag + 1;
	return(GAMEEV_SUCCESS);
}


// SK : フラグのセット・クリア・反転 (Nonaka.K)
int isfcmd_31(SCR_OPE *op) {

	UINT16	num;
	BYTE	cmd;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_flagop(num, cmd);
	return(GAMEEV_SUCCESS);
}


// SKS : フラグをまとめてセット・クリア・反転 Nonaka.K, T.Yui
int isfcmd_32(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	BYTE	cmd;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	while(from <= to) {				// タイトルに戻れないのはこれが原因か…
		scr_flagop(from++, cmd);
	}
	return(GAMEEV_SUCCESS);
}


// SKS : フラグ判定ジャンプ (Nonaka.K)
int isfcmd_33(SCR_OPE *op) {

	UINT16	num;
	UINT16	ptr;
	BYTE	bit;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((scr_flagget(num, &bit) == SUCCESS) && (bit)) {
		scr_jump(ptr);
	}
	return(GAMEEV_SUCCESS);
}


// FT : フラグ転送 (T.Yui)
int isfcmd_34(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	UINT16	size;
	BYTE	bit;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS) ||
		(scr_getword(op, &size) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (from < to) {
		from += size;
		to += size;
		while(size--) {
			scr_flagget(--from, &bit);
			scr_flagop(--to, bit);
		}
	}
	else {
		while(size--) {
			scr_flagget(from++, &bit);
			scr_flagop(to++, bit);
		}
	}
	return(GAMEEV_SUCCESS);
}


// SP : パターンフラグのセット (T.Yui)
int isfcmd_35(SCR_OPE *op) {

	FLAGS		flags;
	BYTE		num;
	PATTBL		pat;

	flags = &gamecore.flags;
	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (num >= GAMECORE_PATTBLS) {
		goto ic35_exit;
	}
	pat = flags->pattbl[num];
	if (pat) {
		_MFREE(pat);
		pat = NULL;
	}
	if (op->remain) {
		pat = (PATTBL_T *)_MALLOC(sizeof(PATTBL_T) + op->remain, "pattern");
		if (pat) {
			pat->size = op->remain;
			CopyMemory(pat + 1, op->ptr, op->remain);
		}
	}
	flags->pattbl[num] = pat;

ic35_exit:
	return(GAMEEV_SUCCESS);
}


// HP : パターンフラグ判定ジャンプ (T.Yui)
static int __hitpattern(BYTE num, BYTE cmd) {

	FLAGS		flags;
	PATTBL_T	*pat;
	BYTE		*p;
	int			size;
	UINT		pos;
	UINT		cnt;
	BYTE		flag;
	BYTE		bit;
	int			ret;

	flags = &gamecore.flags;
	ret = 0;
	if (num >= GAMECORE_PATTBLS) {
		goto hp_nonhit;
	}
	pat = flags->pattbl[num];
	if (pat == NULL) {
		goto hp_nonhit;
	}
	size = pat->size;
	p = (BYTE *)(pat + 1);

	ret = (cmd & 2)?1:0;
	while(size) {
		if (p[0] == 0xff) {
			ret ^= 1;
			break;
		}
		size -= 4;
		if (size < 0) {
			ret ^= 1;						// というかコマンドエラーなんだが
			break;
		}
		pos = LOADINTELWORD(p);
		p += 2;
		cnt = LOADINTELWORD(p);
		p += 2;
		while(cnt--) {
			size--;
			if (size < 0) {
				goto hp_nonhit;
			}
			bit = *p++;
			if (scr_flagget8(pos, &flag)) {
				goto hp_nonhit;
			}
			pos++;
			flag &= bit;
			if (cmd & 1) {
				bit = 0;
			}
			if (flag != bit) {
				goto hp_nonhit;
			}
		}
	}

hp_nonhit:
	return(ret);
}

int isfcmd_36(SCR_OPE *op) {

	BYTE	num;
	UINT16	ptr;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (__hitpattern(num, 0)) {
		scr_jump(ptr);
	}
	return(GAMEEV_SUCCESS);
}


// STS : システムフラグの設定 (T.Yui)
int isfcmd_37(SCR_OPE *op) {

	BYTE	cmd;
	BYTE	val;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getbyte(op, &val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	switch(cmd) {
		case 0x08:	// メッセージスキップ
			if ((val == 0) || (val == 2)) {
				gamecfg_setskip(1);
			}
			else if (val == 1) {
				gamecfg_setskip(0);
			}
			break;

		case 0x0e:	// 既読スキップ
			if (val < 4) {
				gamecore.gamecfg.lastread = val & 1;
				gamecfg_setreadskip((val >> 1) & 1);
			}
			break;

		default:
//			TRACEOUT(("STS : cmd:%x param:%x", cmd, val));
			return(GAMEEV_FORCE);
	}
	return(GAMEEV_SUCCESS);
}


// ES : 指定フラグのセーブ (T.Yui)
int isfcmd_38(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	SAVEHDL	sh;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (from <= to) {
		sh = savefile_open(TRUE);
		savefile_writesysflag(sh, from, (UINT)to - (UINT)from + 1);
		savefile_close(sh);
	}
	return(GAMEEV_SUCCESS);
}


// EC : 指定フラグのロード (T.Yui)
int isfcmd_39(SCR_OPE *op) {

	UINT16	from;
	UINT16	to;
	SAVEHDL	sh;

	if ((scr_getword(op, &from) != SUCCESS) ||
		(scr_getword(op, &to) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (from <= to) {
		sh = savefile_open(FALSE);
		savefile_readsysflag(sh, from, (UINT)to - (UINT)from + 1);
		savefile_close(sh);
	}
	return(GAMEEV_FORCE);
}


// STC : システムフラグの判定ジャンプ (T.Yui)
int isfcmd_3a(SCR_OPE *op) {

	BYTE	cmd;
	BYTE	val;
	UINT16	ptr;
	BOOL	r;
	GAMECFG	gamecfg;
	int		flag;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getbyte(op, &val) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	gamecfg = &gamecore.gamecfg;
	r = FALSE;

	switch(cmd) {
		case 0x01:	// 音声のみ = 0, TEXTのみ = 1, TEXT/音声 = 2
			r = (gamecfg->msgtype == (int)val);
			break;

		case 0x08:	// メッセージスキップ
			flag = val?0:1;
			if (event_getkey() & KEY_SKIP) {
				r = (flag == 1);
			}
			else {
				r = (flag == gamecfg->skip);
			}
			break;

		case 0x0c:	// オート
			flag = val?0:1;
			r = (gamecfg->autoclick == flag);
			break;

		case 0x0e:	// 既読スキップ
			flag = gamecfg->lastread;
			if (!(event_getkey() & KEY_SKIP)) {
				flag |= (gamecfg->readskip << 1);
			}
			r = (flag == (int)val);
			break;

		default:
			// TRACEOUT(("STC : cmd:%x param:%x", cmd, val));
			break;
	}
	if (r) {
		scr_jump(ptr);
	}
	return(GAMEEV_SUCCESS);
}


// HN : フラグ判定ジャンプ (Nonaka.K)
int isfcmd_3b(SCR_OPE *op) {

	UINT16	num;
	UINT16	ptr;
	BYTE	bit;

	if ((scr_getword(op, &num) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((scr_flagget(num, &bit) == SUCCESS) && (!bit)) {
		scr_jump(ptr);
	}
	return(GAMEEV_SUCCESS);
}


// HXP : パターンフラグ判定ジャンプ２ (T.Yui)
int isfcmd_3c(SCR_OPE *op) {

	BYTE	num;
	BYTE	func;
	UINT16	ptr;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &func) != SUCCESS) ||
		(scr_getword(op, &ptr) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (__hitpattern(num, func)) {
		scr_jump(ptr);
	}
	return(GAMEEV_SUCCESS);
}

