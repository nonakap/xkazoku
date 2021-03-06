#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"
#include	"isfcheck.h"
#include	"isfcmd.tbl"


typedef struct {
	BYTE	used[0x20];
	BYTE	usedef[0x20];
	BYTE	usedef3[0x20];
} _CMDLIST, *CMDLIST;


static int cmdlist(SCRHDL hdl, SCROPE ope, void *arg) {

	CMDLIST	cl;
	BYTE	cmd;
	BYTE	cmd2;

	cl = (CMDLIST)arg;
	cl->used[(ope->cmd >> 3) & 0x1f] |= (1 << (ope->cmd & 7));
	if ((ope->cmd == 0xef) && (ope->remain >= 1)) {
		cmd = ope->ptr[0];
		cl->usedef[(cmd >> 3) & 0x1f] |= (1 << (cmd & 7));
		if ((cmd == 3) && (ope->remain >= 2)) {
			cmd2 = ope->ptr[1];
			cl->usedef3[(cmd2 >> 3) & 0x1f] |= (1 << (cmd2 & 7));
		}
	}
	return(0);
}

static void listdisp(BYTE *bit, int range) {

	int		i;
	char	*p;
	char	work[128];

	i = 0;
	while(i < range) {
		p = work + ((i & 0x0f) * 3);
		if (bit[i >> 3] & (1 << (i & 7))) {
			sprintf(p, "%02x ", i);
		}
		else {
			strcpy(p, "   ");
		}
		i++;
		if ((i & 0xf) == 0) {
			log_disp("%s", work);
		}
	}
	if ((i & 0xf) != 0) {
		log_disp("%s", work);
	}
}

void isfcheck_cmdlist(const char *path, const char *filename) {

	_CMDLIST	cl;
	ARCFH		afh;
	int			i;
	char		name[ARCFILENAME_LEN + 1];
	SCRHDL		sh;

	ZeroMemory(&cl, sizeof(cl));
	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto iccl_err;
	}
	if (filename == NULL) {
		for (i=0; i<afh->files; i++) {
			if (arcread_getname(afh, i, name, sizeof(name))) {
				break;
			}
			sh = scr_create(afh, name);
			if (sh == NULL) {
				log_disp("%s: couldn't open", name);
				continue;
			}
			printf("%s\n", name);
			scr_enum(sh, &cl, cmdlist);
			scr_destroy(sh);
		}
	}
	else {
		sh = scr_create(afh, filename);
		if (sh == NULL) {
			log_disp("%s: couldn't open", filename);
		}
		else {
			printf("%s\n", filename);
			scr_enum(sh, &cl, cmdlist);
			scr_destroy(sh);
		}
	}
	arcread_close(afh);

	log_disp("%s: use command", path);
	listdisp(cl.used, 256);
	if (cl.used[0xef >> 3] & (1 << (0xef & 7))) {
		log_disp("use command - ef");
		listdisp(cl.usedef, 16);
		if (cl.usedef[3 >> 3] & (1 << (3 & 7))) {
			log_disp("use command - ef:03");
			listdisp(cl.usedef3, 16);
		}
	}

iccl_err:
	return;
}


// ----

static int isfdump(SCRHDL hdl, SCROPE ope, void *arg) {

	int		rem;
	char	work[128];
	char	hex[16];

	sprintf(work, "%05x: %02x", ope->pos, ope->cmd);
	while(ope->remain > 0) {
		strcat(work, " -");
		rem = min(ope->remain, 16);
		ope->remain -= rem;
		while(rem--) {
			sprintf(hex, " %02x", *ope->ptr++);
			strcat(work, hex);
		}
		if (ope->remain > 0) {
			log_disp("%s", work);
			strcpy(work, "         ");
		}
	}
	log_disp("%s", work);
	if ((ope->debug) && (ope->debugleng)) {
		rem = min(ope->debugleng, sizeof(work) - 1);
		CopyMemory(work, ope->debug, rem);
		work[rem] = '\0';
		log_disp("line %d: %s", ope->debugline, work);
	}
	return(0);
}

void isfcheck_dump(const char *path, const char *filename) {

	ARCFH		afh;
	UINT		i;
	SCRHDL		sh;
	char		workfile[MAX_PATH];

	if (filename == NULL) {
		goto icd_err1;
	}
	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto icd_err1;
	}
	sh = scr_create(afh, filename);
	if (sh == NULL) {
		log_disp("%s: couldn't open", filename);
		goto icd_err2;
	}
	milstr_ncpy(workfile, sh->ah->name, sizeof(workfile));
	cutExtName(workfile);
	milstr_ncat(workfile, ".txt", sizeof(workfile));
	log_ext(workfile);
	log_disp("jump table");
	for (i=0; i<sh->jmptbls; i++) {
		log_disp("%04x - jmp %05x", i, LOADINTELDWORD(sh->jmptbl + i*4));
	}
	log_disp("");
	log_disp("script");
	scr_enum(sh, NULL, isfdump);
	log_ext(NULL);
	scr_destroy(sh);

icd_err2:
	arcread_close(afh);

icd_err1:
	return;
}


// ----

typedef unsigned char	CCERR;

typedef struct {
	int		exever;
	UINT	gametype;
	int		errors;
	int		warnings;
	int		warnlevel;
	CCERR	err[256];
	CCERR	text[256];
	CCERR	effect[256];
	CCERR	cmdef[256];
	CCERR	cmdef03[256];
} _CMDCHECK, *CMDCHECK;

enum {
	CC_WARNING		= 0x01,
	CC_ERROR		= 0x02,
	CC_SKIP			= 0x04
};

static CCERR ccget(SCROPE ope, int bytes, int *r) {

	int		val;

	val = 0;
	while(bytes--) {
		if (ope->remain <= 0) {
			return(CC_ERROR);
		}
		val <<= 8;
		val += *ope->ptr++;
		ope->remain--;
	}
	if (r) {
		*r = val;
	}
	return(0);
}

static CCERR ccskip(SCROPE ope, int bytes) {

	if (ope->remain < bytes) {
		return(CC_ERROR);
	}
	ope->ptr += bytes;
	ope->remain -= bytes;
	return(0);
}

static CCERR ccstringskip(SCROPE ope, int bytes) {

	char	c;

	while(bytes--) {
		if (ope->remain <= 0) {
			return(CC_ERROR);
		}
		c = *ope->ptr++;
		ope->remain--;
		if (c == 0) {
			break;
		}
	}
	return(0);
}

static CCERR ccjmptbl(SCROPE ope) {

	CCERR	err;
	int		tbl;

	err = ccget(ope, 1, &tbl);
	if (err == 0) {
		err = ccskip(ope, tbl * 2);
	}
	return(err);
}

static CCERR cctext1(CMDCHECK cc, SCROPE ope, BYTE cmdnum) {

	CCERR	err;
	BOOL	ascii;
	BYTE	c;
	BOOL	enableascii;

	enableascii = ((cc->gametype & GAME_TEXTASCII) != 0);
	err = 0;
	while(1) {
		if (ope->remain <= 0) {
			break;
		}
		c = *ope->ptr;
		ascii = FALSE;
		if (enableascii) {
			if (c == 0x5c) {
				ope->ptr++;
				ope->remain--;
				if (ope->remain <= 0) {
					break;
				}
				if (ope->ptr[0] == 0) {
					goto zen_break;
				}
				c = ope->ptr[0];
			}
			while(c == 0x7f) {
				ope->ptr++;
				ope->remain--;
				if (ope->remain <= 0) {
					return(err);
				}
				c = *ope->ptr;
				ascii = TRUE;
			}
		}

zen_break:
		ope->ptr++;
		ope->remain--;
		if (c == 0) {
			break;
		}
		if ((!ascii) && (c >= 0x80)) {
			if ((ope->remain <= 0) || (*ope->ptr == '\0')) {
				log_disp("warning: text sjis broken down");
				cc->warnings += 1;
				break;
			}
			ope->ptr++;
			ope->remain--;
		}
	}
	return(err);
}

static CCERR cctext(CMDCHECK cc, SCROPE ope, BYTE cmdnum) {

	CCERR	err;
	int		cmd;

	err = 0;
	while(!err) {
		if (ope->remain <= 0) {
			break;
		}
		cmd = *ope->ptr++;
		ope->remain--;
		if (cc->text[cmd]) {
			err |= CC_SKIP;
			break;
		}
		switch(cmd) {
			case 0x00:	// ?
			case 0x02:	// mouseevent
			case 0x03:	// clear
			case 0x06:	// cr
#if 0
				if (!(cmdnum == 0x2b)) {
					log_disp("warning: textcmd %x unsupport??", cmd);
					cc->warnings += 1;
					err |= CC_WARNING;
				}
#endif
				break;

			case 0x12:	// renum!
				if (!((cmdnum == 0x2b) && (cc->exever >= EXE_VER2))) {
					cc->warnings += 1;
					err = CC_WARNING;
				}
				break;

			case 0x11:
				if ((cmdnum == 0x2b) && (cc->exever >= EXE_VER2)) {
					err |= ccskip(ope, 4);
				}
				break;

			case 0x01:	// fontcolor
				if ((cmdnum == 0x2b) && (cc->exever >= EXE_VER1)) {
					err = ccskip(ope, 1);
				}
				err |= ccskip(ope, 3);
				break;

			case 0x04:	// 人名表示
				err = ccskip(ope, 1);
				break;

			case 0x08:	// 既読フラグの設定
				err = ccskip(ope, 4);
 				break;

			case 0x09:	// ?
				err = ccskip(ope, 1);
 				break;

			case 0x0a:	// param;
				if (cc->exever == EXEVER_MYU) {
					err = ccskip(ope, 2);
				}
				else if (cmdnum == 0x2b) {
					err = ccskip(ope, 4);
				}
				else {
					err = ccskip(ope, 5);
				}
				break;

			case 0x13:
				err = ccstringskip(ope, 13);
				break;

			case 0xff:
				err = cctext1(cc, ope, cmdnum);
				break;

			default:
				log_disp("warning: unsupport textcmd %x", cmd);
				cc->warnings += 1;
				err = CC_WARNING;
				break;
		}
		cc->text[cmd] |= err;
	}
	return(err);
}

static CCERR cc13(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	err = ccskip(ope, 9);
	if (cc->exever >= EXE_VER1) {
		err |= ccskip(ope, 9);
	}
	return(err);
}

static CCERR cc1c(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	err = 0;
	if (cc->exever != EXEVER_ROSYU2) {
		err = ccskip(ope, 4 * 6);
	}
	return(err);
}

static CCERR cc44(CMDCHECK cc, SCROPE ope) {

	CCERR	err;
	int		op1;
	int		op2;

	err = ccget(ope, 1, &op1);
	if (err) {
		goto cc44_exit;
	}
	while(op1 < 5) {
		err = ccskip(ope, 4);
		err |= ccget(ope, 1, &op2);
		if (err) {
			break;
		}
		op1 = op2;
	}

cc44_exit:
	return(err);
}

static CCERR cc47(CMDCHECK cc, SCROPE ope) {

	CCERR	err;
	int		cmd;

	while(1) {
		err = ccskip(ope, 4);
		err |= ccget(ope, 1, &cmd);
		err |= ccskip(ope, 4);
		if (err) {
			break;
		}
		err |= ccget(ope, 1, &cmd);
		if (cmd == 0) {
			err |= ccskip(ope, 2);
			err |= ccget(ope, 1, &cmd);
			if ((!err) && (cmd != 0xff)) {
				err |= CC_ERROR;
			}
			break;
		}
		else if (cmd == 1) {
			err |= ccskip(ope, 2);
			err |= ccskip(ope, 4);
			err |= ccget(ope, 1, &cmd);
			if ((!err) && (cmd != 0xff)) {
				err |= CC_ERROR;
			}
			break;
		}
		else if (cmd == 0xff) {
			break;
		}
	}
	return(err);
}

static CCERR cc55(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	err = 0;
	if (cc->exever >= EXEVER_CRES) {
		err = ccskip(ope, 1);
	}
	return(err);
}

static CCERR cc57(CMDCHECK cc, SCROPE ope) {

	CCERR	err;
	int		cmd;
	CCERR	effecterr;

	err = ccget(ope, 1, &cmd);
	err |= ccskip(ope, 4 + 16 + 4 + 8);
	if (err) {
		goto cc57_exit;
	}
	if (cc->effect[cmd]) {
		err |= CC_SKIP;
		goto cc57_exit;
	}
	effecterr = 0;
	switch(cmd) {
		case 0x00: // type0
		case 0x01: // type0
		case 0x02: // type1			// random pattern
		case 0x03: // type1			// wipe right
		case 0x04: // type1			// wipe left
		case 0x05: // type1			// wipe down
		case 0x06: // type1			// wipe up
		case 0x07: // type1			// wipe with pattern right
		case 0x08: // type1			// wipe with pattern left
		case 0x09: // type1			// wipe with pattern down
		case 0x0a: // type1			// wipe with pattern up
		case 0x0b: // type1			// zoom in
		case 0x0c: // type1			// zoom out
		case 0x0d: // type1			// mosaic in
		case 0x0e: // type1			// mosaic out
		case 0x13: // type1			// FadeIn
		case 0x18: // type1			// width out
		case 0x19: // type1			// width in
		case 0x1a: // type1			// height out
		case 0x1b: // type1			// height in
			break;

		case 0x0f: // type2			// draw-hline down
		case 0x10: // type2			// draw-vline right
		case 0x11: // type2			// draw-hline up
		case 0x12: // type2			// draw-vline left
			effecterr = ccskip(ope, 4 + 4);
			break;

		case 0x14: // type3			// copy with alpha
		case 0x15:
			effecterr = ccskip(ope, 4);
			break;

		case 0x1c:	// type5
		case 0x1d:	// type5
			effecterr = ccskip(ope, 4 + 4 + 4 + 4);
			break;

		default:
			log_disp("warning: unsupport effect %x", cmd);
			cc->warnings += 1;
			effecterr = CC_WARNING;
			break;
	}
	cc->effect[cmd] = effecterr;
	err |= effecterr;

cc57_exit:
	return(err);
}

static CCERR cc71(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	err = 0;
	if (cc->exever != EXEVER_TEA2) {
		err |= ccskip(ope, 1);
	}
	return(err);
}

static CCERR cc7b(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	if (cc->exever <= EXEVER_TSUKU) {
		err = ccskip(ope, 4);
		err |= ccskip(ope, 4);
	}
	else {
		err = ccskip(ope, 4);
		err |= ccskip(ope, 4);
	}
	return(err);
}

static CCERR cc7c(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	if (cc->exever <= EXEVER_TSUKU) {
		err = ccskip(ope, 4);
		err |= ccskip(ope, 1);
	}
	else {
		err = ccskip(ope, 1);
		err |= ccskip(ope, 2);
	}
	return(err);
}

static CCERR cc7d(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	if (cc->exever <= EXEVER_TSUKU) {
		err = ccskip(ope, 4);
	}
	else {
		err = ccskip(ope, 4);
		err |= ccskip(ope, 1);
		err |= ccskip(ope, 2);
	}
	return(err);
}

static CCERR cc81(CMDCHECK cc, SCROPE ope) {

	CCERR	err;

	if ((cc->exever == EXEVER_KONYA2) || (cc->exever >= EXEVER_HEART)) {
		err = ccskip(ope, 4);
	}
	else {
		err = ccskip(ope, 1);
	}
	return(err);
}

static CCERR ccef(CMDCHECK cc, SCROPE ope) {

	CCERR	err;
	int		cmd;
	int		cmd2;

	err = ccget(ope, 1, &cmd);
	if (err) {
		goto ccef_exit;
	}
	if (cc->cmdef[cmd]) {
		err |= CC_SKIP;
		goto ccef_exit;
	}
	switch(cmd) {
		case 1:
			err = ccskip(ope, 4);
			if (!(cc->gametype & GAME_SAVEGRPH)) {
				if (ope->remain) {
					log_disp("warning: set configure GAME_SAVEGRPH");
					cc->warnings += 1;
					err |= CC_WARNING;
				}
			}
			else {
				if (ccskip(ope, 4)) {
					log_disp("warning: reset configure GAME_SAVEGRPH");
					cc->warnings += 1;
					err |= CC_WARNING;
				}
			}
			break;

		case 2:
			err = ccskip(ope, 4);
			break;

		case 3:
			err |= ccget(ope, 1, &cmd2);
			if (err) {
				break;
			}
			if (cc->cmdef03[cmd2]) {
				err |= CC_SKIP;
				break;
			}
			switch(cmd2) {
				case 1:
				case 2:
					err |= ccskip(ope, 4 * 4);
					break;
				case 3:
					err |= ccskip(ope, 4);
					break;
				default:
					log_disp("warning: unsupport ef 03 %x", cmd2);
					cc->warnings += 1;
					err = CC_WARNING;
					break;
			}
			cc->cmdef03[cmd2] = err;
			break;

		case 4:
			err = ccskip(ope, 4 * 7);
			break;

		case 5:
			err = ccskip(ope, 4 + 8 + 4);
			break;

		case 6:
			err = ccskip(ope, 4 * 5);
			if (!err) {
				log_disp("warning: xkazoku forces this command");
				cc->warnings += 1;
				err = CC_ERROR;
			}
			break;

		case 7:
			err = ccskip(ope, 4 * 2);
			if (!err) {
				log_disp("warning: xkazoku forces this command");
				cc->warnings += 1;
				err = CC_ERROR;
			}
			break;

		default:
			log_disp("warning: unsupport ef %x", cmd);
			cc->warnings += 1;
			err = CC_WARNING;
			break;
	}
	cc->cmdef[cmd] = err;

ccef_exit:
	return(err);
}


static CCERR cmdcheck_nor(CMDCHECK cc, SCROPE ope, const BYTE *cmdtype) {

	BYTE	c;
	CCERR	err;

	err = 0;
	while(1) {
		c = *cmdtype++;
		if (c == 0) {
			if (ope->remain) {
				err |= CC_WARNING;
				log_disp("warning: wrong delimiter");
				cc->warnings += 1;
			}
			break;
		}
		if (c == ISF_EXIT) {
			break;
		}

		switch(c) {
			case ISF_BYTE:
				err = ccskip(ope, 1);
				break;

			case ISF_WORD:
			case ISF_ADRS:
				err = ccskip(ope, 2);
				break;

			case ISF_DWORD:
			case ISF_VAL:
				err = ccskip(ope, 4);
				break;

			case ISF_FNAME:
				err = ccstringskip(ope, 13);
				break;

			case ISF_STR:
				err = ccstringskip(ope, -1);
				break;

			case ISF_WIN:
				if (cc->exever >= EXE_VER1) {
					err = ccskip(ope, 1);
				}
				break;

			case ISF_MSG:
				err = 1;
				break;

			case ISF_POINT:
				err = ccskip(ope, 8);
				break;

			case ISF_RECT:
				err = ccskip(ope, 16);
				break;

			case ISF_COLOR:
				err = ccskip(ope, 3);
				break;

			case ISF_JMPTBL:
				err = ccjmptbl(ope);
				break;

			case ISF_TICK:
				if (cc->exever >= EXEVER_KAZOKU) {
					err = ccskip(ope, 4);
				}
				break;

			case ISF_CMD13:
				err = cc13(cc, ope);
				break;

			case ISF_CMD1c:
				err = cc1c(cc, ope);
				break;

			case ISF_CMD2b:
			case ISF_CMDd6:
				err = cctext(cc, ope, 0x2b);
				break;

			case ISF_CMD44:
				err = cc44(cc, ope);
				break;

			case ISF_CMD47:
				err = cc47(cc, ope);
				break;

			case ISF_CMD55:
				err = cc55(cc, ope);
				break;

			case ISF_CMD57:
				err = cc57(cc, ope);
				break;

			case ISF_CMD5b:
				err = cctext(cc, ope, 0x5b);
				break;

			case ISF_CMD71:
				err = cc71(cc, ope);
				break;

			case ISF_CMD7b:
				err = cc7b(cc, ope);
				break;

			case ISF_CMD7c:
				err = cc7c(cc, ope);
				break;

			case ISF_CMD7d:
				err = cc7d(cc, ope);
				break;

			case ISF_CMD81:
				err = cc81(cc, ope);
				break;

			case ISF_CMDef:
				err = ccef(cc, ope);
				break;

			case ISF_WARN1:
				if (cc->warnlevel >= 1) {
					log_disp("warning: xkazoku forces this command");
					cc->warnings += 1;
					return(CC_ERROR);
				}
				break;

			case ISF_WARN2:
				if (cc->warnlevel >= 2) {
					log_disp("warning: xkazoku forces this parameter");
					cc->warnings += 1;
					return(CC_ERROR);
				}
				break;

			default:
				err = CC_ERROR;
				break;
		}
		if (err & CC_ERROR) {
			log_disp("error: wrong parameter");
			cc->errors += 1;
		}
		if (err) {
			break;
		}
	}
	return(err);
}

static int cmdcheck(SCRHDL hdl, SCROPE ope, void *arg) {

	CMDCHECK	cc;
const BYTE		*cmdtype;
	_SCROPE		opebak;
	CCERR		err;
	int			cmd;

	cc = (CMDCHECK)arg;

	cmd = ope->cmd;
	if (cc->err[cmd] & CC_ERROR) {
		return(0);
	}

	opebak = *ope;
	err = 0;

	if (hdl->type == SCRIPT_ISF) {
		cmdtype = isfcmds[cmd];
	}
	else if (hdl->type == SCRIPT_DRS) {
		cmdtype = drscmds[cmd];
	}
	else {
		cmdtype = NULL;
	}
	if (cmdtype) {
		err = cmdcheck_nor(cc, ope, cmdtype);
	}
	else {
		log_disp("error: unsupport command %x", cmd);
		err = CC_ERROR;
		cc->errors += 1;
	}

	cc->err[cmd] |= err;
	if (err & (CC_ERROR | CC_WARNING)) {
		isfdump(hdl, &opebak, NULL);
		log_disp("");
	}
	return(0);
}

void isfcheck_cmdcheck(const char *path, const char *filename,
								int exever, UINT gametype, int warnlevel) {

	_CMDCHECK	cc;
	ARCFH		afh;
	int			i;
	char		name[ARCFILENAME_LEN + 1];
	SCRHDL		sh;

	ZeroMemory(&cc, sizeof(cc));
	cc.exever = exever;
	cc.gametype = gametype;
	cc.warnlevel = warnlevel;

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto iccc_err;
	}
	if (filename == NULL) {
		for (i=0; i<afh->files; i++) {
			if (arcread_getname(afh, i, name, sizeof(name))) {
				break;
			}
			sh = scr_create(afh, name);
			if (sh == NULL) {
				log_disp("%s: couldn't open", name);
				continue;
			}
			printf("%s\n", name);
			scr_enum(sh, &cc, cmdcheck);
			scr_destroy(sh);
		}
	}
	else {
		sh = scr_create(afh, filename);
		if (sh == NULL) {
			log_disp("%s: couldn't open", filename);
		}
		else {
			printf("%s\n", filename);
			scr_enum(sh, &cc, cmdcheck);
			scr_destroy(sh);
		}
	}
	arcread_close(afh);

	log_disp("%5d warning errors", cc.warnings);
	log_disp("%5d  severe errors", cc.errors);
	log_disp("");

iccc_err:
	return;
}


// ----

static int cmddisp(SCRHDL hdl, SCROPE ope, void *arg) {

	_SCROPE		opebak;

	if (ope->cmd == (BYTE)(long)arg) {
		opebak = *ope;
		isfdump(hdl, &opebak, NULL);
		log_disp("");
	}
	return(0);
}

void isfcheck_cmddisp(const char *path, const char *filename, BYTE cmd) {

	ARCFH		afh;
	int			i;
	char		name[ARCFILENAME_LEN + 1];
	SCRHDL		sh;

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto iccc_err;
	}
	if (filename == NULL) {
		for (i=0; i<afh->files; i++) {
			if (arcread_getname(afh, i, name, sizeof(name))) {
				break;
			}
			sh = scr_create(afh, name);
			if (sh == NULL) {
				log_disp("%s: couldn't open", name);
				continue;
			}
			printf("%s\n", name);
			scr_enum(sh, (void *)(long)cmd, cmddisp);
			scr_destroy(sh);
		}
	}
	else {
		sh = scr_create(afh, filename);
		if (sh == NULL) {
			log_disp("%s: couldn't open", filename);
		}
		else {
			printf("%s\n", filename);
			scr_enum(sh, (void *)(long)cmd, cmddisp);
			scr_destroy(sh);
		}
	}
	arcread_close(afh);
	log_disp("");

iccc_err:
	return;
}

