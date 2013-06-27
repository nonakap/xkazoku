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
#include	"drs_cmd.h"


// STS : システムフラグの設定
int drscmd_37(SCR_OPE *op) {

	BYTE	cmd;
	BYTE	val;

	if ((scr_getbyte(op, &cmd) != SUCCESS) ||
		(scr_getbyte(op, &val) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	return(GAMEEV_SUCCESS);
}


// CGフラグの設定
int drscmd_3c(SCR_OPE *op) {

	CGFLAG_T	cgf;
	LISTARRAY	cgflag;

	if ((scr_getval(op, &cgf.num) != SUCCESS) ||
		(scr_getlabel(op, cgf.name, sizeof(cgf.name)) != SUCCESS) ||
		(scr_getval(op, &cgf.bit) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	cgflag = gamecore.drssys.cgflag;
	if (cgflag == NULL) {
		cgflag = listarray_new(sizeof(CGFLAG_T), 64);
		gamecore.drssys.cgflag = cgflag;
	}
	listarray_append(cgflag, &cgf);
	return(GAMEEV_SUCCESS);
}


// CGフラグ達成率の取得

typedef struct {
	SINT32	num;
	SINT32	avail;
	SINT32	count;
} CGREG;

static BOOL cgregcb(void *vpItem, void *vpArg) {

	BYTE	flag;

	if (((CGFLAG)vpItem)->num == ((CGREG *)vpArg)->num) {
		if (scr_flagget(((CGFLAG)vpItem)->bit, &flag) == SUCCESS) {
			((CGREG *)vpArg)->avail += flag;
		}
		((CGREG *)vpArg)->count += 1;
	}
	return(FALSE);
}

int drscmd_3d(SCR_OPE *op) {

	CGREG		cgreg;
	SINT32		reg;

	ZeroMemory(&cgreg, sizeof(cgreg));
	if ((scr_getval(op, &cgreg.num) != SUCCESS) ||
		(scr_getval(op, &reg) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	listarray_enum(gamecore.drssys.cgflag, cgregcb, &cgreg);
	if (cgreg.count) {
		cgreg.avail *= 100;
		cgreg.avail += cgreg.count >> 1;
		cgreg.avail /= cgreg.count;
	}
	scr_valset(reg, cgreg.avail);
	return(GAMEEV_SUCCESS);
}

