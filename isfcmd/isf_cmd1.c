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
#include	"cgload.h"


static BOOL scr_getwinnum(SCR_OPE *op, BYTE *num) {

	BOOL	r;

	if (gamecore.sys.version >= EXE_VER1) {
		r = scr_getbyte(op, num);
	}
	else {
		r = SUCCESS;
		*num = 0;
	}
	return(r);
}


// CW : ���ޥ�ɥ�����ɥ��ΰ��� �����������å� (T.Yui)
int isfcmd_10(SCR_OPE *op) {

	BYTE	num;
	BYTE	cmd;
	RECT_U	scrn;
	TEXTWIN	textwin;

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	// ����
	scr_getbyte(op, &cmd);

	textwin = textwin_getwin(num);
	if (textwin) {
		textwin->flag |= TEXTWIN_CMDRECT;
#ifdef SIZE_QVGA
		vramdraw_halfscrn(&scrn.s);
#endif
		textwin->cmdscrn = scrn.s;
	}
	return(GAMEEV_SUCCESS);
}


// CP : ���ޥ�ɥ�����ɥ��Υե졼���ɤ߹��� (T.Yui)
int isfcmd_11(SCR_OPE *op) {

	BYTE	num;
	BYTE	val;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &val) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setcmdframe(num, label);
	return(GAMEEV_SUCCESS);
}


// CIR : ���������ɤ߹��� (T.Yui)
int isfcmd_12(SCR_OPE *op) {

	BYTE	num;
	BYTE	width;
	char	label[ARCFILENAME_LEN+1];

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &width) != SUCCESS) ||
		(scr_getlabel(op, label, sizeof(label)) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_setcmdicon(num, width, label);
	return(GAMEEV_SUCCESS);
}


// CPS : ʸ���ѥ�å����� (T.Yui)
int isfcmd_13(SCR_OPE *op) {

	BYTE	num;
	BYTE	col[3];
	int		i;
	int		count;
	TEXTWIN	textwin;

	if (scr_getwinnum(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (gamecore.sys.version >= EXE_VER1) {
		count = 6;
	}
	else {
		count = 3;
	}
	textwin = textwin_getwin(num);
	if (textwin) {
		for (i=0; i<count; i++) {
			if ((scr_getbyte(op, col + 0) != SUCCESS) ||
				(scr_getbyte(op, col + 1) != SUCCESS) ||
				(scr_getbyte(op, col + 2) != SUCCESS)) {
				return(GAMEEV_WRONGLENG);
			}
			textwin->chocolor[i] = MAKEPALETTE(col[0], col[1], col[2]);
		}
	}
	return(GAMEEV_SUCCESS);
}


// CIP : ���ޥ�ɤ˥������󥻥å� (T.Yui)
int isfcmd_14(SCR_OPE *op) {

	BYTE	num;
	BYTE	cmd;
	CMD_T	item;
	TEXTWIN	textwin;

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getval(op, &item.num) != SUCCESS) ||
		(scr_getval(op, &item.pt.x) != SUCCESS) ||
		(scr_getval(op, &item.pt.y) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if (cmd < GAMECORE_MAXCMDS) {
#ifdef SIZE_QVGA
		vramdraw_halfpoint(&item.pt);
#endif
		textwin = textwin_getwin(num);
		if (textwin) {
			textwin->cmd[cmd] = item;
		}
	}
	return(GAMEEV_SUCCESS);
}


// CSET : ���ޥ�ɤ�̾�����å� Nonaka.K, T.Yui
int isfcmd_15(SCR_OPE *op) {

	BYTE		num;
	BYTE		cmd;
	int			len;
	int			fontsize;
	RECT_U		scrn;
	TEXTWIN		textwin;
	CHO_T		*cho;

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getrect(op, &scrn) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	textwin = textwin_getwin(num);
	if ((textwin == NULL) || (cmd >= GAMECORE_MAXCMDS)) {
		goto isf15_exit;
	}
	cho = textwin->cho + cmd;
	vramdraw_scrn2rect(&scrn.s, &cho->rct);

	// Lien����SJIS�����äƤ������ɡ�
	len = scr_getmsg(op, cho->str, sizeof(cho->str));
	cho->x = cho->rct.left;
	cho->y = cho->rct.top;
	if (gamecore.sys.version < EXE_VER1) {
		fontsize = textwin->textctrl.fontsize;
#ifndef SIZE_QVGA
		cho->x += (scrn.s.width - (len * fontsize / 2)) / 2;
		cho->y += (scrn.s.height - fontsize) / 2;
#else
		fontsize = vramdraw_half(fontsize);
		cho->x += (cho->rct.right - cho->rct.left - (len * fontsize / 2)) / 2;
		cho->y += (cho->rct.bottom - cho->rct.top - fontsize) / 2;
#endif
	}

isf15_exit:
	return(GAMEEV_SUCCESS);
}


// CWO : ���ޥ�ɥ�����ɥ��Υ����ץ� (T.Yui)
int isfcmd_16(SCR_OPE *op) {

	BYTE		num;
	SINT32		cmds;
	BYTE		type;

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getval(op, &cmds) != SUCCESS) ||
		(scr_getbyte(op, &type) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	if ((type != 1) && (type != 2)) {
		TRACEOUT(("CWO : unknown type %d", type));
		return(GAMEEV_FAILURE);
	}
	if (cmds > GAMECORE_MAXCMDS) {
		cmds = GAMECORE_MAXCMDS;
	}

	textwin_cmdopen(num, cmds, type);
	return(GAMEEV_SUCCESS);
}


// CWC : ���ޥ�ɥ�����ɥ��Υ����� (T.Yui)
int isfcmd_17(SCR_OPE *op) {

	BYTE	num;

	if (scr_getwinnum(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	textwin_cmdclose(num);
	return(GAMEEV_SUCCESS);
}


// CC : ���ޥ������¹� (T.Yui)
int isfcmd_18(SCR_OPE *op) {

	BYTE	num;
	UINT16	val;
	BYTE	cmd;
	TEXTWIN	textwin;
	CWEVT	cwevt;

	if ((scr_getwinnum(op, &num) != SUCCESS) ||
		(scr_getword(op, &val) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}

	textwin = textwin_getwin(num);
	if ((textwin) && (textwin->flag & TEXTWIN_CMD)) {
		switch(cmd) {
			case 0:		// Event.
				TRACEOUT(("CWO : Capture"));
				cwevt = &gamecore.cwevt;
				textwin->flag |= TEXTWIN_CMDCAPEX;
				cwevt->num = num;
				cwevt->val = val;
				return(GAMEEV_CMDWIN);

			case 1:		// Set Capture
				TRACEOUT(("CWO : Set Capture"));
				textwin->flag |= TEXTWIN_CMDCAP;
				break;

			case 2:		// Release Capture
				TRACEOUT(("CWO : Release Capture"));
				textwin->flag &= ~TEXTWIN_CMDCAP;
				break;

			case 3:		// Get Param
//				if (textwin->flag & TEXTWIN_CMDCAP) {
					scr_valset(val, event_getcmdwin(textwin));
//				}
				break;

			default:
				TRACEOUT(("CC : unknown cmd %d", cmd));
				return(GAMEEV_FAILURE);
		}
	}
	else {
		scr_valset(val, -1);		// ���åȤ��Ƥ�����
	}
	return(GAMEEV_SUCCESS);
}


// CCLR : ���ޥ�ɤ�̾�����ꥢ
int isfcmd_19(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// CRESET : ���ޥ�ɤ�̾������ν���
int isfcmd_1a(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}


// CRND : ���ޥ�ɤΥ���������
int isfcmd_1b(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}

