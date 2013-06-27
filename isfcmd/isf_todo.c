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

// IM : �}�E�X�J�[�\���f�[�^�̓ǂݍ���
int isfcmd_80(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// ---- todo

// WL : �N���b�N�҃p�[�c�ǂݍ���
int isfcmd_22(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// WW : �N���b�N�Ґݒ�
int isfcmd_23(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// SETINSIDEVOL : �������ʐݒ�
int isfcmd_9f(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}



// KIDCLR : ���Ǖ��͂̏�����
int isfcmd_a0(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDMOJI : ���Ǖ��͂̕����̐F��ݒ肷��
int isfcmd_a1(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDPAGE : ���Ǖ��͂̕ŏ��
int isfcmd_a2(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// KIDHABA : ���Ǖ��͂̂P�s������̕�����
int isfcmd_a6(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

// ????
int isfcmd_da(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_FORCE);
}

