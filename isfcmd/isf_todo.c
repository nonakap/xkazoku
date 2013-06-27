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

// IM : �ޥ�����������ǡ������ɤ߹���
int isfcmd_80(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// IXY : �ޥ����ΰ����ѹ�
int isfcmd_83(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ---- todo

// CTEXT : �ƥ�����ɽ��
int isfcmd_1c(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// WW : ����å�������
int isfcmd_23(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// SETINSIDEVOL : ������������
int isfcmd_9f(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// KIDCLR : ����ʸ�Ϥν����
int isfcmd_a0(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDMOJI : ����ʸ�Ϥ�ʸ���ο������ꤹ��
int isfcmd_a1(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDPAGE : ����ʸ�Ϥ��Ǿ���
int isfcmd_a2(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDHABA : ����ʸ�ϤΣ��Ԥ������ʸ����
int isfcmd_a6(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

