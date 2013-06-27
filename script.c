#include	"compiler.h"
#include	"dosio.h"
#include	"gamecore.h"
#include	"isf_cmd.h"
#ifdef SUPPORT_DRS
#include	"drs_cmd.h"
#endif
#include	"arcfile.h"


static void script_solve(BYTE *p, UINT pos, UINT size) {

	p += pos;
	while(pos < size) {
		*p = ((*p) << 6) | ((*p) >> 2);
		p++;
		pos++;
	}
}

static void script_solve2(BYTE *p, UINT pos, UINT size, BYTE key) {

	p += pos;
	while(pos < size) {
		*p ^= key;
		p++;
		pos++;
	}
}

static void script_solve3(BYTE *p, UINT pos, UINT size) {

	p += pos;
	while(pos < size) {
		*p ^= 0xff;
		p++;
		pos++;
	}
}


SCRIPT_HDL script_create(const char *fname) {

	ARCFILEH	hdl;
	SCRIPT_HDL	ret = NULL;
	BYTE		*p;
	UINT		type = SCRIPT_ERR;
	UINT		size;

	hdl = arcfile_open(ARCTYPE_SCRIPT, fname);
	if (hdl == NULL) {
		return(NULL);
	}

	do {
		TRACEOUT(("script load:%s", hdl->name));
		size = hdl->size;
		if (hdl->ext == NULL) {
			break;
		}
		if ((!milstr_cmp(hdl->ext, "isf")) && (size > 8)) {
			type = SCRIPT_ISF;
		}
		else if ((!milstr_cmp(hdl->ext, "drs")) && (size > 53)) {
			type = SCRIPT_DRS;
		}
		else {
			break;
		}

		ret = (SCRIPT_HDL)_MALLOC(sizeof(SCRIPT_T) + size, hdl->name);
		if (ret == NULL) {
			break;
		}
		ZeroMemory(ret, sizeof(SCRIPT_T) + size);
		ret->type = SCRIPT_ERR;
		milstr_ncpy(ret->name, hdl->name, sizeof(ret->name));

		p = (BYTE *)(ret + 1);
		if (arcfile_read(hdl, p, size) != size) {
			break;
		}
		if (type == SCRIPT_ISF) {
			UINT pos;
			ret->rev = (p[4] << 8) | p[5];
			if (ret->rev == 0x9597) {			// 9597
				script_solve(p, 8, size);
			}
			else if (ret->rev == 0x89ce) {		// 89ce
				script_solve2(p, 8, size, p[6]);
			}
			else if (ret->rev == 0x97d1) {		// 97d1
				script_solve3(p, 8, size);
			}
			else {
				type = SCRIPT_ERR;
			}

			pos = min(LOADINTELDWORD(p), size);
			ret->jmptbl = p + 8;
			ret->jmptbls = (pos - 8) / 4;

			ret->scr = p + pos;
			ret->scrsize = size - pos;
			ret->type = SCRIPT_ISF;
		}
		else if (type == SCRIPT_DRS) {
			UINT	pos;
			UINT	rsize;
			UINT	num;

			pos = 0x19;							// head check?
			script_solve(p, pos, size);

			// call table
			if ((pos + 8) > size) {
				break;
			}
			rsize = LOADINTELDWORD(p + pos);
			num = LOADINTELDWORD(p + pos + 4);
			pos += 8;
			if ((pos + rsize) > size) {
				break;
			}
			ret->jmptbl = p + pos;
			ret->jmptbls = min(rsize / 4, num);
			pos += rsize;

			// unknown1
			if ((pos + 8) > size) {
				break;
			}
			rsize = LOADINTELDWORD(p + pos);
			num = LOADINTELDWORD(p + pos + 4);
			pos += 8;
			if ((pos + rsize) > size) {
				break;
			}
			ret->unknown2 = p + pos;
			ret->unknown2size = min(rsize / 4, num);
			pos += rsize;

			// jmp table
			if ((pos + 8) > size) {
				break;
			}
			rsize = LOADINTELDWORD(p + pos);
			num = LOADINTELDWORD(p + pos + 4);
			pos += 8;
			if ((pos + rsize) > size) {
				break;
			}
			ret->calltbl = p + pos;
			ret->calltbls = min(rsize / 4, num);
			pos += rsize;

			// script table
			if ((pos + 4) > size) {
				break;
			}
			rsize = LOADINTELDWORD(p + pos);
			pos += 4;
			if ((pos + rsize) > size) {
				rsize = size - pos;
			}
			ret->scr = p + pos;
			ret->scrsize = rsize;
			pos += rsize;

			// dummy?
			// ret->dummy = p[pos];

			ret->type = SCRIPT_DRS;
		}
	} while(0);
	if ((ret) && (ret->type == SCRIPT_ERR)) {
		_MFREE(ret);
		ret = NULL;
	}
	arcfile_close(hdl);
	return(ret);
}


void script_destroy(SCRIPT_HDL hdl) {

	if (hdl) {
		_MFREE(hdl);
	}
}


// ----

#ifndef ARCDEBUG

typedef int (*ISFCMD)(SCR_OPE *op);

static const ISFCMD isf_cmd[256] = {
			isfcmd_00,	isfcmd_01,	isfcmd_02,	isfcmd_03,		// 00
			isfcmd_04,	isfcmd_05,	isfcmd_06,	isfcmd_07,		// 04
			isfcmd_08,	isfcmd_09,	NULL,		NULL,			// 08
			NULL,		NULL,		NULL,		NULL,			// 0c

			isfcmd_10,	isfcmd_11,	isfcmd_12,	isfcmd_13,		// 10
			isfcmd_14,	isfcmd_15,	isfcmd_16,	isfcmd_17,		// 14
			isfcmd_18,	isfcmd_19,	isfcmd_1a,	isfcmd_1b,		// 18
			isfcmd_1c,	NULL,		NULL,		NULL,			// 1c

			isfcmd_20,	isfcmd_21,	NULL,		isfcmd_23,		// 20
			isfcmd_24,	isfcmd_25,	isfcmd_26,	isfcmd_27,		// 24
			isfcmd_28,	isfcmd_29,	isfcmd_2a,	isfcmd_2b,		// 28
			NULL,		isfcmd_2d,	isfcmd_2e,	NULL,			// 2c

			isfcmd_30,	isfcmd_31,	isfcmd_32,	isfcmd_33,		// 30
			isfcmd_34,	isfcmd_35,	isfcmd_36,	isfcmd_37,		// 34
			isfcmd_38,	isfcmd_39,	isfcmd_3a,	isfcmd_3b,		// 38
			isfcmd_3c,	NULL,		NULL,		NULL,			// 3c

			isfcmd_40,	isfcmd_41,	isfcmd_42,	isfcmd_43,		// 40
			isfcmd_44,	isfcmd_45,	isfcmd_46,	isfcmd_47,		// 44
			isfcmd_48,	isfcmd_49,	isfcmd_4a,	isfcmd_4b,		// 48
			isfcmd_4c,	NULL,		NULL,		NULL,			// 4c

			isfcmd_50,	isfcmd_51,	isfcmd_52,	isfcmd_53,		// 50
			NULL,		isfcmd_55,	isfcmd_56,	isfcmd_57,		// 54
			isfcmd_58,	isfcmd_59,	isfcmd_5a,	isfcmd_5b,		// 58
			isfcmd_5c,	isfcmd_5d,	isfcmd_5e,	isfcmd_5f,		// 5c

			isfcmd_60,	NULL,		isfcmd_62,	isfcmd_63,		// 60
			isfcmd_64,	isfcmd_65,	isfcmd_66,	NULL,			// 64
			isfcmd_68,	isfcmd_69,	NULL,		NULL,			// 68
			NULL,		NULL,		NULL,		isfcmd_6f,		// 6c

			isfcmd_70,	isfcmd_71,	isfcmd_72,	isfcmd_73,		// 70
			isfcmd_74,	isfcmd_75,	isfcmd_76,	isfcmd_77,		// 74
			isfcmd_78,	isfcmd_79,	isfcmd_7a,	isfcmd_7b,		// 78
			NULL,		NULL,		isfcmd_7e,	isfcmd_7f,		// 7c

			isfcmd_80,	isfcmd_81,	isfcmd_82,	isfcmd_83,		// 80
			isfcmd_84,	isfcmd_85,	isfcmd_86,	isfcmd_87,		// 84
			isfcmd_88,	isfcmd_89,	isfcmd_8a,	isfcmd_8b,		// 88
			NULL,		isfcmd_8d,	isfcmd_8e,	isfcmd_8f,		// 8c

			isfcmd_90,	isfcmd_91,	isfcmd_92,	NULL,			// 90
			NULL,		NULL,		NULL,		NULL,			// 94
			NULL,		NULL,		NULL,		NULL,			// 98
			NULL,		NULL,		NULL,		isfcmd_9f,		// 9c

			isfcmd_a0,	isfcmd_a1,	isfcmd_a2,	NULL,			// a0
			NULL,		isfcmd_a5,	isfcmd_a6,	isfcmd_a7,		// a4
			NULL,		NULL,		NULL,		NULL,			// a8
			NULL,		NULL,		isfcmd_ae,	isfcmd_af,		// ac

			NULL,		NULL,		NULL,		NULL,			// b0
			NULL,		NULL,		NULL,		NULL,			// b4
			NULL,		NULL,		NULL,		NULL,			// b8
			NULL,		isfcmd_bd,	isfcmd_be,	isfcmd_bf,		// bc

			NULL,		NULL,		NULL,		NULL,			// c0
			NULL,		NULL,		NULL,		NULL,			// c4
			NULL,		NULL,		NULL,		NULL,			// c8
			NULL,		NULL,		NULL,		NULL,			// cc

			NULL,		NULL,		NULL,		NULL,			// d0
			NULL,		NULL,		isfcmd_d6,	isfcmd_d7,		// d4
			NULL,		NULL,		NULL,		NULL,			// d8
			NULL,		NULL,		NULL,		NULL,			// dc

			isfcmd_e0,	isfcmd_e1,	isfcmd_e2,	isfcmd_e3,		// e0
			isfcmd_e4,	isfcmd_e5,	NULL,		NULL,			// e4
			NULL,		NULL,		NULL,		NULL,			// e8
			NULL,		NULL,		NULL,		isfcmd_ef,		// ec

			isfcmd_f0,	isfcmd_f1,	isfcmd_f2,	isfcmd_f3,		// f0
			isfcmd_f4,	isfcmd_f5,	NULL,		isfcmd_f7,		// f4
			isfcmd_f8,	isfcmd_f9,	isfcmd_fa,	isfcmd_fb,		// f8
			isfcmd_fc,	NULL,		NULL,		NULL,			// fc
};

#ifdef SUPPORT_DRS

static int unsupport(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}

static const ISFCMD drs_cmd[256] = {
			NULL,		isfcmd_01,	NULL,		NULL,			// 00
			isfcmd_04,	drscmd_05,	drscmd_06,	drscmd_07,		// 04
			drscmd_08,	unsupport,	unsupport,	unsupport,		// 08
			unsupport,	unsupport,	NULL,		NULL,			// 0c

			isfcmd_10,	unsupport,	unsupport,	isfcmd_13,		// 10
			NULL,		NULL,		NULL,		NULL,			// 14
			NULL,		NULL,		NULL,		NULL,			// 18
			NULL,		NULL,		NULL,		NULL,			// 1c

			drscmd_20,	drscmd_21,	unsupport,	NULL,			// 20
			NULL,		NULL,		NULL,		NULL,			// 24
			drscmd_28,	drscmd_29,	drscmd_2a,	drscmd_2b,		// 28
			NULL,		NULL,		NULL,		NULL,			// 2c

			isfcmd_30,	isfcmd_31,	isfcmd_32,	NULL,			// 30
			isfcmd_33,	NULL,		NULL,		isfcmd_37,		// 34
			NULL,		NULL,		unsupport,	NULL,			// 38
			unsupport,	NULL,		NULL,		NULL,			// 3c

			isfcmd_40,	isfcmd_41,	drscmd_42,	isfcmd_42,		// 40
			isfcmd_43,	NULL,		drscmd_46,	isfcmd_47,		// 44
			NULL,		NULL,		NULL,		NULL,			// 48
			NULL,		NULL,		NULL,		NULL,			// 4c

			isfcmd_50,	isfcmd_51,	NULL,		isfcmd_53,		// 50
			drscmd_54,	isfcmd_56,	isfcmd_57,	NULL,			// 54
			NULL,		NULL,		NULL,		NULL,			// 58
			NULL,		NULL,		NULL,		NULL,			// 5c

			NULL,		NULL,		NULL,		NULL,			// 60
			NULL,		NULL,		NULL,		NULL,			// 64
			NULL,		NULL,		NULL,		NULL,			// 68
			NULL,		NULL,		NULL,		NULL,			// 6c

			NULL,		NULL,		NULL,		NULL,			// 70
			NULL,		NULL,		isfcmd_74,	drscmd_77,		// 74
			isfcmd_76,	unsupport,	isfcmd_91,	isfcmd_92,		// 78
			NULL,		NULL,		NULL,		NULL,			// 7c

			unsupport,	unsupport,	NULL,		NULL,			// 80
			isfcmd_84,	isfcmd_85,	NULL,		NULL,			// 84
			NULL,		NULL,		NULL,		NULL,			// 88
			NULL,		NULL,		NULL,		NULL,			// 8c

			NULL,		NULL,		NULL,		NULL,			// 90
			NULL,		NULL,		NULL,		NULL,			// 94
			NULL,		NULL,		NULL,		NULL,			// 98
			NULL,		NULL,		NULL,		NULL,			// 9c

			NULL,		NULL,		NULL,		NULL,			// a0
			NULL,		NULL,		NULL,		NULL,			// a4
			NULL,		NULL,		NULL,		NULL,			// a8
			NULL,		NULL,		NULL,		NULL,			// ac

			NULL,		NULL,		NULL,		NULL,			// b0
			NULL,		NULL,		NULL,		NULL,			// b4
			NULL,		NULL,		NULL,		NULL,			// b8
			NULL,		NULL,		NULL,		NULL,			// bc

			NULL,		NULL,		NULL,		NULL,			// c0
			NULL,		NULL,		NULL,		NULL,			// c4
			NULL,		NULL,		NULL,		NULL,			// c8
			NULL,		NULL,		NULL,		NULL,			// cc

			NULL,		NULL,		NULL,		NULL,			// d0
			NULL,		NULL,		NULL,		NULL,			// d4
			NULL,		NULL,		NULL,		NULL,			// d8
			NULL,		NULL,		NULL,		NULL,			// dc

			NULL,		NULL,		NULL,		NULL,			// e0
			NULL,		NULL,		NULL,		NULL,			// e4
			NULL,		NULL,		NULL,		NULL,			// e8
			NULL,		NULL,		NULL,		NULL,			// ec

			NULL,		isfcmd_f1,	isfcmd_f2,	unsupport,		// f0
			isfcmd_f4,	isfcmd_f5,	NULL,		NULL,			// f4
			unsupport,	NULL,		unsupport,	NULL,			// f8
			NULL,		NULL,		NULL,		NULL,			// fc
};
#endif


int script_dispatch(void) {

	SCENE_T		*sc;
	SCR_OPE		op;
	ISFCMD		func;

	sc = gamecore.curscene;
	if (sc == NULL) {
		return(GAMEEV_EOS);
	}
	op.ptr = sc->ptr;
	op.size = sc->script->scrsize;

	if ((sc->pos + 2) > op.size) {
		return(GAMEEV_EOS);
	}
	op.cmd = *op.ptr++;
	op.len = *op.ptr++;
	op.pos = 2;
	if (op.len & 0x80) {
		if ((sc->pos + 3) > op.size) {
			return(GAMEEV_EOS);
		}
		op.len &= 0x7f;
		op.len <<= 8;
		op.len |= *op.ptr++;
		op.pos++;
	}
	if (op.len > op.size - sc->pos) {
		op.len = op.size - sc->pos;
	}
	op.remain = op.len - op.pos;
	if (op.remain < 0) {
		return(GAMEEV_WRONGLENG);
	}
	sc->ptr += op.len;
	sc->pos += op.len;

	if (sc->script->type == SCRIPT_ISF) {
		func = isf_cmd[op.cmd];
	}
#ifdef SUPPORT_DRS
	else if (sc->script->type == SCRIPT_DRS) {
		func = drs_cmd[op.cmd];
	}
#endif
	else {
		func = NULL;
	}
	if (func) {
		int r = func(&op);
#ifdef TRACE
		if (r < 0) {
			TRACEOUT(("error cmd:%x", op.cmd));
			TRACEOUT(("IP: %x", sc->pos));
		}
#endif
		return(r);
	}
	TRACEOUT(("wrong cmd:%x", op.cmd));
	return(GAMEEV_WRONGCMD);
}

#endif

