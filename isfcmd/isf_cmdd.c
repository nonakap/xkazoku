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


// MPM : �����s�����\���̎��s (T.Yui)
int isfcmd_d6(SCR_OPE *op) {

	BYTE	num;
	BYTE	line;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &line) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(textdisp_multiset(num, line, op));
}


// MPC : �o�^�s�̔j�� (T.Yui)		// �ꉞ����Ƃ�
int isfcmd_d7(SCR_OPE *op) {

	textdisp_multireset();
	(void)op;
	return(GAMEEV_SUCCESS);
}

