#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"


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

SCRHDL scr_create(ARCFH afh, const char *filename) {

	SCRHDL	ret;
	BYTE	*ptr;
	UINT	type = SCRIPT_ERR;
	UINT	size;
	ARCHDL	ah;
	char	*ext;
	UINT	pos;

	ah = arcread_get(afh, filename);
	if (ah == NULL) {
		goto scre_err1;
	}
	ptr = ah->ptr;
	size = ah->size;
	ext = getExtName(ah->name);
	if ((!milstr_cmp(ext, "isf")) && (size > 8)) {
		type = SCRIPT_ISF;
	}
	else if ((!milstr_cmp(ext, "drs")) && (size > 53)) {
		type = SCRIPT_DRS;
	}
	else {
		goto scre_err2;
	}
	ret = (SCRHDL)_MALLOC(sizeof(_SCRHDL), ah->name);
	if (ret == NULL) {
		goto scre_err2;
	}

	ZeroMemory(ret, sizeof(_SCRHDL));
	ret->type = type;
	ret->ah = ah;

	if (type == SCRIPT_ISF) {
		ret->rev = (ptr[4] << 8) | ptr[5];
		if (ret->rev == 0x9597) {			// 9597
			script_solve(ptr, 8, size);
		}
		else if (ret->rev == 0x89ce) {		// 89ce
			script_solve2(ptr, 8, size, ptr[6]);
		}
		else if (ret->rev == 0x97d1) {		// 97d1
			script_solve3(ptr, 8, size);
		}
		else if (ret->rev == 0x8df7) {		// 8df7
		}
		else {
			goto scre_err3;
		}

		pos = min(LOADINTELDWORD(ptr), size);
		ret->jmptbl = ptr + 8;
		ret->jmptbls = (pos - 8) / 4;

		ret->scr = ptr + pos;
		ret->scrsize = size - pos;
	}
	else if (type == SCRIPT_DRS) {
		UINT rsize;
		UINT num;

		pos = 0x19;							// head check?
		script_solve(ptr, pos, size);

		// call table
		if ((pos + 8) > size) {
			goto scre_err3;
		}
		rsize = LOADINTELDWORD(ptr + pos);
		num = LOADINTELDWORD(ptr + pos + 4);
		pos += 8;
		if ((pos + rsize) > size) {
			goto scre_err3;
		}
		ret->jmptbl = ptr + pos;
		ret->jmptbls = min(rsize / 4, num);
		pos += rsize;

		// unknown1
		if ((pos + 8) > size) {
			goto scre_err3;
		}
		rsize = LOADINTELDWORD(ptr + pos);
		num = LOADINTELDWORD(ptr + pos + 4);
		pos += 8;
		if ((pos + rsize) > size) {
			goto scre_err3;
		}
		ret->unknown2 = ptr + pos;
		ret->unknown2size = min(rsize / 4, num);
		pos += rsize;

		// jmp table
		if ((pos + 8) > size) {
			goto scre_err3;
		}
		rsize = LOADINTELDWORD(ptr + pos);
		num = LOADINTELDWORD(ptr + pos + 4);
		pos += 8;
		if ((pos + rsize) > size) {
			goto scre_err3;
		}
		ret->calltbl = ptr + pos;
		ret->calltbls = min(rsize / 4, num);
		pos += rsize;

		// script table
		if ((pos + 4) > size) {
			goto scre_err3;
		}
		rsize = LOADINTELDWORD(ptr + pos);
		pos += 4;
		if ((pos + rsize) > size) {
			rsize = size - pos;
		}
		ret->scr = ptr + pos;
		ret->scrsize = rsize;
		pos += rsize;

		// dummy?
		// ret->dummy = ptr[pos];
	}
	else {
		goto scre_err3;
	}
	ret->type = type;
	return(ret);

scre_err3:
	_MFREE(ret);

scre_err2:
	_MFREE(ah);

scre_err1:
	return(NULL);
}

void scr_destroy(SCRHDL hdl) {

	if (hdl) {
		_MFREE(hdl->ah);
		_MFREE(hdl);
	}
}

void scr_enum(SCRHDL hdl, void *arg,
						int (*proc)(SCRHDL hdl, SCROPE ope, void *arg)) {

	_SCROPE	ope;
	BYTE	*ptr;
	int		size;
	UINT	pos;
	int		len;
	int		cmdpos;

	if ((hdl == NULL) || (proc == NULL)) {
		goto sr_err;
	}

	pos = 0;
	ptr = hdl->scr;
	size = hdl->scrsize;
	while(size >= 2) {
		ope.cmd = ptr[0];
		ope.pos = pos;
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
		ope.ptr = ptr + cmdpos;
		ope.remain = len - cmdpos;
		if ((hdl->type == SCRIPT_DRS) && (ope.remain > 0)) {
			ope.remain -= cmdpos - 1;
		}
		if (ope.remain < 0) {
			break;
		}
		ptr += len;
		pos += len;
		size -= len;

		ope.debug = NULL;
		if ((hdl->rev == 0x8df7) && (size >= 2)) {
			len = LOADINTELWORD(ptr);
			len = min(len, size);
			if (len >= 4) {
				ope.debug = ptr + 4;
				ope.debugleng = len - 4;
				ope.debugline = LOADINTELWORD(ptr+2);
			}
			ptr += len;
			pos += len;
			size -= len;
		}

		if (proc(hdl, &ope, arg)) {
			break;
		}
	}

sr_err:
	return;
}

