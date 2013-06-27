#include	"compiler.h"
#include	"gamecore.h"
#include	"savefile.h"


// ポインタエラーフリー。

BOOL scr_valset(UINT num, SINT32 val) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if (num >= flags->maxval) {
		return(FAILURE);
	}
	flags->val[num] = val;
	return(SUCCESS);
}

BOOL scr_valget(UINT num, SINT32 *val) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if (num >= flags->maxval) {
		return(FAILURE);
	}
	*val = flags->val[num];
	return(SUCCESS);
}


// ----

BOOL scr_flagop(UINT num, BYTE method) {

	FLAGS	flags;
	BYTE	*p;
	BYTE	bit;

	flags = &gamecore.flags;
	if (num >= flags->maxflag) {
		return(FAILURE);
	}
	p = flags->flag + (num/8);
	bit = 1 << (num & 7);
	switch(method) {
		case 0:			// clear
			*p &= ~bit;
			break;
		case 1:			// set
			*p |= bit;
			break;
		case 2:			// xor
			*p ^= bit;
			break;
	}
	return(SUCCESS);
}

BOOL scr_flagget(UINT num, BYTE *flag) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if (num >= flags->maxflag) {
		return(FAILURE);
	}
	*flag = (flags->flag[num/8] >> (num & 7)) & 1;
	return(SUCCESS);
}

BOOL scr_flagget8(UINT num, BYTE *flag) {

	FLAGS	flags;
	UINT	maxflag;

	flags = &gamecore.flags;
	maxflag = (flags->maxflag + 8) / 8;

	if (num >= maxflag) {
		return(FAILURE);
	}
	*flag = flags->flag[num];
	return(SUCCESS);
}


// ----

SINT32 scr_cmdval(UINT32 val) {

	SINT32	ret;

	ret = val & 0x1fffffff;
	val >>= 29;
	if (val & 1) {					// sign
		ret = ret - 0x20000000;		// orで符合復元するのはよろしくない…
	}
	if (val & 4) {
		if (scr_valget(ret, &ret) != SUCCESS) {
			ret = 0;
		}
	}
	if ((val & 2) && (ret)) {
		SINT32 rn;
		rn = (GETRAND() & 0x7fff) * 0x7fff;	// 何故 0x8000じゃないんだろ
		rn += (GETRAND() & 0x7fff);
		if (ret > 0) {
			ret = rn % ret;
		}
		else {
			ret = (rn % (ret * -1)) * -1;	// CPU依存だと困るので冗長ぽく
		}
	}
	return(ret);
}


// ----

BOOL scr_getbyte(SCR_OPE *op, BYTE *val) {

	if (op->remain <= 0) {
		return(FAILURE);
	}
	op->remain--;
	*val = op->ptr[0];
	op->ptr++;
	return(SUCCESS);
}

BOOL scr_getword(SCR_OPE *op, UINT16 *val) {

	if (op->remain < 2) {
		return(FAILURE);
	}
	op->remain -= 2;
	*val = LOADINTELWORD(op->ptr);
	op->ptr += 2;
	return(SUCCESS);
}

BOOL scr_getdword(SCR_OPE *op, UINT32 *val) {

	if (op->remain < 4) {
		return(FAILURE);
	}
	op->remain -= 4;
	*val = LOADINTELDWORD(op->ptr);
	op->ptr += 4;
	return(SUCCESS);
}

BOOL scr_getval(SCR_OPE *op, SINT32 *val) {

	UINT32	r;

	if (op->remain < 4) {
		return(FAILURE);
	}
	op->remain -= 4;
	r = LOADINTELDWORD(op->ptr);
	op->ptr += 4;
	*val = scr_cmdval(r);
	return(SUCCESS);
}

BOOL scr_getpt(SCR_OPE *op, POINT_T *pt) {

	BOOL	r;
	SINT32	num;

	r = scr_getval(op, &num);
	pt->x = num;
	r |= scr_getval(op, &num);
	pt->y = num;
	return(r);
}

BOOL scr_getrect(SCR_OPE *op, RECT_U *rect) {

	BOOL	r;
	SINT32	num;

	r = scr_getval(op, &num);
	rect->s.left = num;
	r |= scr_getval(op, &num);
	rect->s.top = num;
	r |= scr_getval(op, &num);
	rect->s.width = num;
	r |= scr_getval(op, &num);
	rect->s.height = num;
	return(r);
}

BOOL scr_getlabel(SCR_OPE *op, char *label, int size) {

	int		i;
	int		len;

	len = min(op->remain, size);
	if (len < 0) {
		return(FAILURE);
	}
	for (i=0; i<len; i++) {
		if (!op->ptr[i]) {
			break;
		}
		label[i] = op->ptr[i];
	}
	if (i < size) {
		label[i] = '\0';
	}

	// skip...
	len = min(op->remain, size);
	for (i=0; i<len; ) {
		if (!op->ptr[i++]) {
			break;
		}
	}
	op->remain -= i;
	op->ptr += i;

	return(SUCCESS);
}


// ----

BOOL scr_getcurscr(char *label, int leng) {

	SCENE_HDL	sc;
	SCRIPT_HDL	scr;

	sc = gamecore.curscene;
	if (sc == NULL) {
		return(FAILURE);
	}
	scr = sc->script;
	if (scr == NULL) {
		return(FAILURE);
	}
	milstr_ncpy(label, scr->name, leng);
	return(SUCCESS);
}

BOOL scr_scriptcall(const char *label) {

	SCENE_HDL	sc;
	SCRIPT_HDL	scr;
	int			scenes;

	scenes = gamecore.scenes;
	if (scenes >= GAMECORE_MAXSCENE) {
		TRACEOUT(("scene overflow"));
		return(FAILURE);
	}
	scr = script_create(label);
	if (scr == NULL) {
		TRACEOUT(("couldn't create script: %12s", label));
		return(FAILURE);
	}
	sc = gamecore.scene + scenes;
	sc->script = scr;
	sc->ptr = scr->scr;
	sc->pos = 0;
	gamecore.curscene = sc;
	gamecore.scenes++;
	return(SUCCESS);
}

BOOL src_scriptret(void) {

	SCENE_HDL	sc;
	int			scenes;

	scenes = gamecore.scenes;
	if (scenes) {
		scenes--;
		gamecore.scenes = scenes;
		sc = gamecore.scene + scenes;
		script_destroy(sc->script);
		if (scenes) {
			gamecore.curscene = sc - 1;
			TRACEOUT(("script ret"));
			return(SUCCESS);
		}
		gamecore.curscene = NULL;
	}
	return(FAILURE);
}

BOOL scr_jump(int ptr) {

	SCENE_HDL	sc;
	SCRIPT_HDL	script;

	sc = gamecore.curscene;
	script = sc->script;
	if ((ptr < 0) || (ptr >= (int)script->jmptbls)) {
		return(FAILURE);
	}
	sc->pos = LOADINTELDWORD(script->jmptbl + ptr * 4);
	sc->ptr = script->scr + sc->pos;
	return(SUCCESS);
}

BOOL scr_call(int ptr) {

	SCENE_HDL	sc;

	sc = gamecore.curscene;
	if ((ptr < 0) || (ptr >= (int)sc->script->jmptbls)) {
		return(FAILURE);
	}
	if (gamecore.stacks >= GAMECORE_MAXSTACK) {
		return(FAILURE);
	}
	gamecore.stack[gamecore.stacks++] = sc->pos;
	sc->pos = LOADINTELDWORD(sc->script->jmptbl + ptr * 4);
	sc->ptr = sc->script->scr + sc->pos;
	return(SUCCESS);
}

BOOL scr_ret(void) {

	SCENE_HDL	sc;

	sc = gamecore.curscene;
	if (!gamecore.stacks) {
		return(FAILURE);
	}
	sc->pos = gamecore.stack[--gamecore.stacks];
	sc->ptr = sc->script->scr + sc->pos;
	return(SUCCESS);
}

BOOL scr_setdrspos(UINT16 pnum) {

	SCENE_HDL	sc;
	SCRIPT_HDL	script;
const BYTE		*ptr;
	int			size;
	UINT		pos;
	int			len;
	int			cmdpos;
	BYTE		cmd;

	sc = gamecore.curscene;
	script = sc->script;

	pos = 0;
	ptr = script->scr;
	size = script->scrsize;
	while(size >= 2) {
		cmd = ptr[0];
		len = ptr[1];
		cmdpos = 2;
		if (len & 0x80) {
			if (size < 3) {
				break;
			}
			len &= 0x7f;
			len <<= 8;
			len += ptr[cmdpos++];
		}
		if (len > size) {
			len = size;
		}
		if ((cmd == 0x0e) && ((len - cmdpos) >= 2)) {
			if (pnum == LOADINTELWORD(ptr + cmdpos)) {
				sc->pos = ptr - script->scr;
				sc->ptr = ptr;
				return(SUCCESS);
			}
		}
		ptr += len;
		size -= len;
	}
	return(FAILURE);
}

// ----

void scr_exaclear(void) {

	FLAGS	flags;
	SAVEHDL	sh;

	flags = &gamecore.flags;
	if ((flags->exaval) || (flags->exaflag)) {
		sh = savefile_open(TRUE);
		if (flags->exaval) {
			sh->writeexaval(sh, flags->exaval);
			variant_destory(flags->exaval);
			flags->exaval = NULL;
		}
		if (flags->exaflag) {
			sh->writeexaflag(sh, flags->exaflag);
			variant_destory(flags->exaflag);
			flags->exaflag = NULL;
		}
		sh->close(sh);
	}
}

void scr_kidclear(void) {

	FLAGS	flags;
	SAVEHDL	sh;

	flags = &gamecore.flags;
	if (flags->kid) {
		sh = savefile_open(TRUE);
		if (sh) {
			sh->writekid(sh, flags->kid);
			sh->close(sh);
		}
		variant_destory(flags->kid);
		flags->kid = NULL;
	}
}


BOOL scr_init(UINT bits, UINT values) {

	FLAGS	flags;
	UINT	bitbytes;

	flags = &gamecore.flags;

	if ((!bits) || (!values)) {
		return(FAILURE);
	}
	bitbytes = (bits + 7) >> 3;

	// Lienではflagテーブル確保コマンド(cmd:30)を行わないので…
	// ついでにれすとあも確保しないでやんの…
	flags->val = (int *)_MALLOC(values * sizeof(int), "value table");
	flags->flag = (BYTE *)_MALLOC(bitbytes, "flag table");
	if ((!flags->val) || (!flags->flag)) {
		return(FAILURE);
	}
	flags->maxval = values;
	flags->maxflag = bits;
	flags->flagsize = bitbytes;
	return(SUCCESS);
}


void scr_term(void) {

	FLAGS	flags;
	int		i;

	flags = &gamecore.flags;

	while(src_scriptret() == SUCCESS) { }

	if (flags->val) {
		_MFREE(flags->val);
		flags->val = NULL;
	}
	flags->maxval = 0;

	if (flags->flag) {
		_MFREE(flags->flag);
		flags->flag = NULL;
	}
	flags->maxflag = 0;
	scr_exaclear();
	scr_kidclear();

	for (i=0; i<GAMECORE_PATTBLS; i++) {
		if (flags->pattbl[i]) {
			_MFREE(flags->pattbl[i]);
			flags->pattbl[i] = NULL;
		}
	}
}


BOOL scr_restart(const char *label) {

	gamecore_reset();
	while(src_scriptret() == SUCCESS) { }		// スクリプト破棄
	return(scr_scriptcall(label));				// ロードスクリプト
}

