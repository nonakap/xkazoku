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


// WS : �E�B���h�E�\���ʒu�ݒ�
int drscmd_20(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn, clip;
	BYTE	val;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS) ||
		(scr_getrect(op, &clip) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_getbyte(op, &val);
	textwin_setpos(num, &scrn.s, &clip.s);
	return(GAMEEV_SUCCESS);
}


// WP : �E�B���h�E�p�[�c�ǂݍ���
int drscmd_21(SCR_OPE *op) {

	BYTE	num;
	BYTE	dummy;
	SINT32	width, height;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getval(op, &width) != SUCCESS) ||
		(scr_getval(op, &height) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setframe(num, label);
	return(GAMEEV_SUCCESS);
}


// WL : �N���b�N�҃p�[�c�ǂݍ���
int drscmd_22(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// WSS : ���b�Z�[�W�E�B���h�E�̕\��
int drscmd_2a(SCR_OPE *op) {

	BYTE	num;
	TEXTWIN	textwin;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->flag &= ~TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// WSH : ���b�Z�[�W�E�B���h�E�̔�\��
int drscmd_2b(SCR_OPE *op) {

	BYTE	num;
	TEXTWIN	textwin;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->flag |= TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// PM : �����̕\��
int drscmd_2c(SCR_OPE *op) {

	BYTE	num;
	UINT16	leng;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getword(op, &leng) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(textdisp_set(num, op));
}

