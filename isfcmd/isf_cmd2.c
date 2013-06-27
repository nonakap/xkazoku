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
#include	"arcfile.h"
#include	"isf_cmd.h"
#include	"fontmng.h"
#include	"cgload.h"


// WS : ������ɥ�ɽ���������� Nonaka.K, T.Yui
int isfcmd_20(SCR_OPE *op) {

	BYTE	num;
	RECT_U	scrn;
	BYTE	val;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	scr_getbyte(op, &val);

	textwin_setpos(num, &scrn.s, NULL);
	return(GAMEEV_SUCCESS);
}


// WP : ������ɥ��ѡ����ɤ߹��� (Nonaka.K)
int isfcmd_21(SCR_OPE *op) {

	BYTE	num;
	BYTE	dummy;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &dummy) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setframe(num, label);
	return(GAMEEV_SUCCESS);
}


// CN : ��ʪ̾ʸ�������� (T.Yui)
int isfcmd_24(SCR_OPE *op) {

	BYTE	cmd;
	BYTE	len;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getbyte(op, &len) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_SUCCESS);
}


// CNS : ��ʪ̾���å� Nonaka.K, T.Yui
int isfcmd_25(SCR_OPE *op) {

	BYTE	num;
	BYTE	cmd;
	TEXTWIN	textwin;
	int		len;
	char	*ptr;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (cmd < GAMECORE_MAXNAME) {
		textwin = textwin_getwin(num);
		if (textwin) {
			ptr = textwin->chrname[cmd];
			len = min(op->remain, GAMECORE_NAMELEN - 1);
			if (len) {
				CopyMemory(ptr, op->ptr, len);
			}
			ptr[len] = '\0';
		}
	}
	return(GAMEEV_SUCCESS);
}


// PF : ��å�����ɽ�����ԡ�������
int isfcmd_26(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// PB : ʸ�����礭������ (Nonaka.K)
int isfcmd_27(SCR_OPE *op) {

	BYTE		num;
	SINT32		size;
	TEXTWIN		textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getval(op, &size) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_setsize(&textwin->textctrl, size);
	}
	return(GAMEEV_SUCCESS);
}


// PJ : ʸ���η������� (T.Yui)
int isfcmd_28(SCR_OPE *op) {

#if 0							// �ºݤ˥��ޥ�ɤ򸫤��櫓�ǤϤʤ��Τǡ�
	BYTE		num;
	BYTE		type;
	TEXTWIN		textwin;

	if ((scr_getbyte(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &type) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		textctrl_settype(&textwin->textctrl, type);
	}
#else
	(void)op;
#endif
	return(GAMEEV_SUCCESS);
}


// WO : ������ɥ������ץ� Nonaka.K, T.Yui
int isfcmd_29(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_open(num);
	return(GAMEEV_SUCCESS);
}


// WC : ������ɥ��Υ����� (T.Yui)
int isfcmd_2a(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_close(num);
	return(GAMEEV_SUCCESS);
}


// PM : ʸ����ɽ��
int isfcmd_2b(SCR_OPE *op) {

	BYTE	num;

	if (scr_getbyte(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	return(textdisp_set(num, op));
}


// WSH : ��å�����������ɥ�����ɽ�� (T.Yui)
int isfcmd_2d(SCR_OPE *op) {

	SINT32	num;
	TEXTWIN	textwin;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(0);
	if (textwin) {
		textwin->flag |= TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}


// WSS : ��å�����������ɥ���ɽ�� (T.Yui)
int isfcmd_2e(SCR_OPE *op) {

	SINT32	val;
	TEXTWIN	textwin;

	if (scr_getval(op, &val) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin = textwin_getwin(0);
	if (textwin) {
		textwin->flag &= ~TEXTWIN_TEXTHIDE;
		vramdraw_setrect(textwin->textctrl.vram, NULL);
		vramdraw_draw();
	}
	return(GAMEEV_SUCCESS);
}

