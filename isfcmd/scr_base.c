#include	"compiler.h"
#include	"gamecore.h"
#include	"savefile.h"


BYTE textsinglebyte[] = {
			0x81, 0x40, 0x81, 0x40, 0x81, 0x41, 0x81, 0x42,
			0x81, 0x45, 0x81, 0x48, 0x81, 0x49, 0x81, 0x69,
			0x81, 0x6a, 0x81, 0x75, 0x81, 0x76, 0x82, 0x4f,
			0x82, 0x50, 0x82, 0x51, 0x82, 0x52, 0x82, 0x53,
			0x82, 0x54, 0x82, 0x55, 0x82, 0x56, 0x82, 0x57,
			0x82, 0x58, 0x82, 0xa0, 0x82, 0xa2, 0x82, 0xa4,
			0x82, 0xa6, 0x82, 0xa8, 0x82, 0xa9, 0x82, 0xaa,
			0x82, 0xab, 0x82, 0xac, 0x82, 0xad, 0x82, 0xae,
			0x81, 0x40, 0x82, 0xb0, 0x82, 0xb1, 0x82, 0xb2,
			0x82, 0xb3, 0x82, 0xb4, 0x82, 0xb5, 0x82, 0xb6,
			0x82, 0xb7, 0x82, 0xb8, 0x82, 0xb9, 0x82, 0xba,
			0x82, 0xbb, 0x82, 0xbc, 0x82, 0xbd, 0x82, 0xbe,
			0x82, 0xbf, 0x82, 0xc0, 0x82, 0xc1, 0x82, 0xc2,
			0x82, 0xc3, 0x82, 0xc4, 0x82, 0xc5, 0x82, 0xc6,
			0x82, 0xc7, 0x82, 0xc8, 0x82, 0xc9, 0x82, 0xca,
			0x82, 0xcb, 0x82, 0xcc, 0x82, 0xcd, 0x82, 0xce,
			0x82, 0xd0, 0x82, 0xd1, 0x82, 0xd3, 0x82, 0xd4,
			0x82, 0xd6, 0x82, 0xd7, 0x82, 0xd9, 0x82, 0xda,
			0x82, 0xdc, 0x82, 0xdd, 0x82, 0xde, 0x82, 0xdf,
			0x82, 0xe0, 0x82, 0xe1, 0x82, 0xe2, 0x82, 0xe3,
			0x82, 0xe4, 0x82, 0xe5, 0x82, 0xe6, 0x82, 0xe7,
			0x82, 0xe8, 0x82, 0xe9, 0x82, 0xea, 0x82, 0xeb,
			0x82, 0xed, 0x82, 0xf0, 0x82, 0xf1, 0x83, 0x41,
			0x83, 0x43, 0x83, 0x45, 0x83, 0x47, 0x83, 0x49,
			0x83, 0x4a, 0x83, 0x4c, 0x83, 0x4e, 0x83, 0x50,
			0x83, 0x52, 0x83, 0x54, 0x83, 0x56, 0x83, 0x58,
			0x83, 0x5a, 0x83, 0x5c, 0x83, 0x5e, 0x83, 0x60,
			0x83, 0x62, 0x83, 0x63, 0x83, 0x65, 0x83, 0x67,
			0x83, 0x69, 0x83, 0x6a, 0x82, 0xaf, 0x83, 0x6c,
			0x83, 0x6d, 0x83, 0x6e, 0x83, 0x71, 0x83, 0x74,
			0x83, 0x77, 0x83, 0x7a, 0x83, 0x7d, 0x83, 0x7e,
			0x83, 0x80, 0x83, 0x81, 0x83, 0x82, 0x83, 0x84};


// ポインタエラーフリー。

BOOL scr_valset(int num, SINT32 val) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if ((num < 0) || (num >= flags->maxval)) {
		return(FAILURE);
	}
	flags->val[num] = val;
	return(SUCCESS);
}

BOOL scr_valget(int num, SINT32 *val) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if ((num < 0) || (num >= flags->maxval)) {
		return(FAILURE);
	}
	*val = flags->val[num];
	return(SUCCESS);
}


// ----

BOOL scr_flagop(int num, BYTE method) {

	FLAGS	flags;
	BYTE	*p;
	BYTE	bit;

	flags = &gamecore.flags;
	if ((num < 0) || (num >= flags->maxflag)) {
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

BOOL scr_flagget(int num, BYTE *flag) {

	FLAGS	flags;

	flags = &gamecore.flags;
	if ((num < 0) || (num >= flags->maxflag)) {
		return(FAILURE);
	}
	*flag = (flags->flag[num/8] >> (num & 7)) & 1;
	return(SUCCESS);
}

BOOL scr_flagget8(int num, BYTE *flag) {

	FLAGS	flags;
	int		maxflag;

	flags = &gamecore.flags;
	maxflag = (flags->maxflag + 8) / 8;

	if ((num < 0) || (num >= maxflag)) {
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
		rn = (rand() & 0x7fff) * 0x7fff;	// 何故 0x8000じゃないんだろ
		rn += (rand() & 0x7fff);
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

int scr_getmsg(SCR_OPE *op, char *string, int len) {

	int		pos = 0;

	while(((pos + 2) < len) && (op->remain > 0)) {
		BYTE c;
		op->remain--;
		c = *op->ptr++;
		if (!c) {
			break;
		}
		if (c < 0x80) {
			string[pos++] = textsinglebyte[c * 2 + 0];
			string[pos++] = textsinglebyte[c * 2 + 1];
		}
		else {
			BYTE c2;
			if (op->remain <= 0) {
				break;
			}
			op->remain--;
			c2 = *op->ptr++;
			if (!c2) {
				break;
			}
			string[pos++] = c;
			string[pos++] = c2;
		}
	}
	if (pos < len) {
		string[pos] = '\0';
	}
	return(pos);
}


// ----

BOOL scr_scriptcall(const char *label) {

	SCENE_HDL	sc;
	SCRIPT_HDL	scr;

	if (gamecore.scenes >= GAMECORE_MAXSCENE) {
		TRACEOUT(("scene overflow"));
		return(FAILURE);
	}
	scr = script_create(label);
	if (scr == NULL) {
		TRACEOUT(("couldn't create script: %12s", label));
		return(FAILURE);
	}
	sc = gamecore.scene + gamecore.scenes;
	sc->script = scr;
	sc->ptr = scr->scr;
	sc->pos = 0;
	gamecore.curscene = sc;
	gamecore.scenes++;
	return(SUCCESS);
}


BOOL src_scriptret(void) {

	SCENE_HDL	sc;

	if (gamecore.scenes) {
		gamecore.scenes--;
		sc = gamecore.scene + gamecore.scenes;
		script_destroy(sc->script);
		if (gamecore.scenes) {
			gamecore.curscene = sc - 1;
			TRACEOUT(("script ret"));
			return(SUCCESS);
		}
	}
	return(FAILURE);
}


BOOL scr_jump(int ptr) {

	SCENE_HDL	sc;

	sc = gamecore.curscene;
	if ((ptr < 0) || (ptr >= (int)sc->script->jmptbls)) {
		return(FAILURE);
	}
	sc->pos = LOADINTELDWORD(sc->script->jmptbl + ptr * 4);
	sc->ptr = sc->script->scr + sc->pos;
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


// ----

void scr_exaclear(void) {

	FLAGS	flags;
	SAVEHDL	sh;

	flags = &gamecore.flags;
	if ((flags->exaval) || (flags->exaflag)) {
		sh = savefile_open(TRUE);
		if (flags->exaval) {
			savefile_writeexaval(sh, flags->exaval);
			variant_destory(flags->exaval);
			flags->exaval = NULL;
		}
		if (flags->exaflag) {
			savefile_writeexaflag(sh, flags->exaflag);
			variant_destory(flags->exaflag);
			flags->exaflag = NULL;
		}
		savefile_close(sh);
	}
}

void scr_kidclear(void) {

	FLAGS	flags;
	SAVEHDL	sh;

	flags = &gamecore.flags;
	if (flags->kid) {
		sh = savefile_open(TRUE);
		savefile_writekid(sh, flags->kid);
		variant_destory(flags->kid);
		flags->kid = NULL;
		savefile_close(sh);
	}
}


BOOL scr_init(void) {

	FLAGS		flags;

	flags = &gamecore.flags;

	// Lienではflagテーブル確保コマンド(cmd:30)を行わないので…
	flags->val = (int *)_MALLOC(0x401 * sizeof(int), "value table");
	flags->flag = (BYTE *)_MALLOC((0x400/8) * sizeof(BYTE), "flag table");
	if ((!flags->val) || (!flags->flag)) {
		return(FAILURE);
	}
	flags->maxval = 0x400;
	flags->maxflag = 0x400;
	flags->flagsize = (0x400 / 8) * sizeof(BYTE);
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

