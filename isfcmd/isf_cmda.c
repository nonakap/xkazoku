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


// KIDFN : ���ǃt���O���ݒ� (Nonaka.K)
int isfcmd_a5(SCR_OPE *op) {

	FLAGS	flags;
	UINT32	cnt;
	SAVEHDL	sh;

	flags = &gamecore.flags;

	if (scr_getdword(op, &cnt) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}

	scr_kidclear();
	cnt += 8;
	cnt &= ~7;
	flags->kid = variant_create(cnt, VARIANT_BIT);

	sh = savefile_open(FALSE);
	if (sh) {
		sh->readkid(sh, flags->kid);
		sh->close(sh);
	}
	return(GAMEEV_SUCCESS);
}


// KIDSCAN : ���ǋ@�\�Ɗ��ǃt���O�̔��� (T.Yui)
int isfcmd_a7(SCR_OPE *op) {

	UINT16	pos;
	SINT32	num;
	BYTE	flag;
	int		readflag;
	GAMECFG	gamecfg;

	if ((scr_getword(op, &pos) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	readflag = 0;
	if (variant_get(gamecore.flags.kid, num, &flag) == SUCCESS) {
		readflag = flag;
	}
	gamecfg = &gamecore.gamecfg;
	gamecfg->lastread = readflag;
	readflag |= gamecfg->readskip << 1;
	scr_valset(pos, readflag);
	return(GAMEEV_SUCCESS);
}


// SETKIDWNDPUTPOS : ���ǃE�B���h�E�̃v�b�g�ʒu�w�� (T.Yui)
int isfcmd_ae(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	DISPWIN	dispwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	dispwin = &gamecore.dispwin;
	dispwin->flag |= DISPWIN_CLIPHIS;
	dispwin->hisclip = scrn.s;
	return(GAMEEV_SUCCESS);
}


// SETMESWNDPUTPOS : ���b�Z�[�W�E�B���h�E�̃v�b�g�ʒu�w�� (T.Yui)
int isfcmd_af(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	DISPWIN	dispwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("msg: %d %d %d %d", scrn.s.left, scrn.s.top, scrn.s.width, scrn.s.height));
	dispwin = &gamecore.dispwin;
	dispwin->flag |= DISPWIN_CLIPTEXT;
	dispwin->txtclip = scrn.s;
	return(GAMEEV_SUCCESS);
}

