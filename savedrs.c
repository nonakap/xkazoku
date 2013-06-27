#include	"compiler.h"
#include	"dosio.h"
#include	"timemng.h"
#include	"gamecore.h"
#include	"savefile.h"
#include	"savedrs.h"


typedef struct {
	BYTE	*ptr;
	int		size;
} _SAVEDATA, *SAVEDATA;


static const char header[] = "1999 D.O.corp. SaveData ";


static void savegetname(int num, char *path, int leng) {

	char	work[128];

	milstr_ncpy(path, gamecore.suf.scriptpath, leng);
	sprintf(work, "%s%02u.sav", gamecore.suf.key, num);
	milstr_ncat(path, work, leng);
	TRACEOUT(("savefile: %s", path));
}

static SAVEDATA saveunpack(const BYTE *ptr, int size) {

	SAVEDATA	ret;
const BYTE		*p;
	BYTE		*q;
	int			rem;
	UINT		datasize;
	BYTE		c;
	UINT		cnt;
	UINT		leng;

	p = ptr;
	rem = size;
	datasize = 0;

	// 解凍サイズを得る。
	while(1) {
		rem--;
		if (rem < 0) {
			break;
		}
		c = *p++;
		if (c) {
			datasize++;
		}
		else {
			rem--;
			if (rem < 0) {
				break;
			}
			c = *p++;
			if (!c) {
				datasize++;
			}
			else {
				rem--;
				if (rem < 0) {
					break;
				}
				leng = *p++;
				rem -= leng;
				if (rem < 0) {
					break;
				}
				datasize += (c + 1) * leng;
				p += leng;
			}
		}
	}
	if (datasize == 0) {
		goto sup_err;
	}

	ret = (SAVEDATA)_MALLOC(sizeof(_SAVEDATA) + datasize, "save buffer");
	if (ret == NULL) {
		goto sup_err;
	}
	ret->ptr = (BYTE *)(ret + 1);
	ret->size = datasize;

	p = ptr;
	rem = size;
	q = (BYTE *)(ret + 1);

	// 解凍
	while(1) {
		rem--;
		if (rem < 0) {
			break;
		}
		c = *p++;
		if (c) {
			*q++ = c;
		}
		else {
			rem--;
			if (rem < 0) {
				break;
			}
			c = *p++;
			if (!c) {
				*q++ = '\0';
			}
			else {
				rem--;
				if (rem < 0) {
					break;
				}
				cnt = c + 1;
				leng = *p++;
				rem -= leng;
				if (rem < 0) {
					break;
				}
				if (leng) {
					do {
						CopyMemory(q, p, leng);
						q += leng;
					} while(--cnt);
				}
				p += leng;
			}
		}
	}
	return(ret);

sup_err:
	return(NULL);
}

static void savesolve(BYTE *ptr, int size) {

const char	*p;
	BYTE	c;
	UINT	pos;
	UINT	keylen;

	pos = 0;
	p = gamecore.suf.title;
	keylen = strlen(p);
	if (keylen == 0) {
		keylen = 1;
	}
	while(size-- > 0) {
		c = p[pos];
		c ^= *ptr;
		c ^= 0xff;
		c = (c << 2) | (c >> 6);
		*ptr++ = c;
		pos = (pos + 1) % keylen;
	}
}

static SAVEDATA saveread(int num, BOOL pack) {

	UINT		headlen;
	SAVEDATA	ret;
	SAVEDATA	tmp;
	FILEH		fh;
	char		path[MAX_PATH];
	UINT		size;

	headlen = strlen(header);

	ret = NULL;
	savegetname(num, path, sizeof(path));
	fh = file_open_rb(path);
	if (fh == FILEH_INVALID) {
		goto svrd_err1;
	}
	size = (UINT)file_seek(fh, 0, FSEEK_END);
	if ((size <= headlen) ||
		(file_seek(fh, headlen, FSEEK_SET)) != (long)headlen) {
		goto svrd_err2;
	}
	size -= headlen;
	ret = (SAVEDATA)_MALLOC(sizeof(_SAVEDATA) + size, path);
	if (ret == NULL) {
		goto svrd_err2;
	}
	if (file_read(fh, ret + 1, size) != size) {
		goto svrd_err3;
	}
	ret->ptr = (BYTE *)(ret + 1);
	ret->size = size;
	if (pack) {
		tmp = saveunpack(ret->ptr, ret->size);
		if (tmp == NULL) {
			goto svrd_err3;
		}
		_MFREE(ret);
		ret = tmp;
	}
	savesolve(ret->ptr, ret->size);
	file_close(fh);
	return(ret);

svrd_err3:
	_MFREE(ret);

svrd_err2:
	file_close(fh);

svrd_err1:
	return(NULL);
}

static void savecode(BYTE *ptr, int size) {

const char	*p;
	BYTE	c;
	UINT	pos;
	UINT	keylen;

	pos = 0;
	p = gamecore.suf.title;
	keylen = strlen(p);
	if (keylen == 0) {
		keylen = 1;
	}
	while(size-- > 0) {
		c = *ptr;
		c = (c >> 2) | (c << 6);
		c ^= 0xff;
		c ^= p[pos];
		*ptr++ = c;
		pos = (pos + 1) % keylen;
	}
}

static SAVEDATA savepack(BYTE *ptr, int size) {

	SAVEDATA	ret;
const BYTE		*p;
	BYTE		*q;
	int			rem;
	UINT		datasize;
	BYTE		c;

	datasize = size;
	p = ptr;
	rem = size;
	while(rem--) {
		c = *p++;
		if (c == 0) {
			datasize++;
		}
	}

	ret = (SAVEDATA)_MALLOC(sizeof(_SAVEDATA) + datasize, "save buffer");
	if (ret == NULL) {
		goto spk_err;
	}
	ret->ptr = (BYTE *)(ret + 1);
	ret->size = datasize;

	p = ptr;
	rem = size;
	q = (BYTE *)(ret + 1);
	while(rem--) {
		c = *p++;
		*q++ = c;
		if (c == 0) {
			*q++ = 0;
		}
	}
	return(ret);

spk_err:
	return(NULL);
}

static BOOL savewrite(int num, BOOL pack, BYTE *ptr, int size) {

	SAVEDATA	spk;
	UINT		headlen;
	char		path[MAX_PATH];
	FILEH		fh;

	spk = NULL;
	savecode(ptr, size);
	if (pack) {
		spk = savepack(ptr, size);
		if (spk == NULL) {
			goto svwr_err1;
		}
		ptr = spk->ptr;
		size = spk->size;
	}

	headlen = strlen(header);

	savegetname(num, path, sizeof(path));
	fh = file_create(path);
	if (fh == FILEH_INVALID) {
		goto svwr_err2;
	}
	if ((file_write(fh, header, headlen) != headlen) ||
		(file_write(fh, ptr, size) != (UINT)size)) {
		goto svwr_err3;
	}
	file_close(fh);
	if (spk) {
		_MFREE(spk);
	}
	return(SUCCESS);

svwr_err3:
	file_close(fh);
	file_delete(path);

svwr_err2:
	 if (spk) {
		_MFREE(spk);
	}

svwr_err1:
	return(FAILURE);
}


// ----

static BOOL skipsavesize(SAVEDATA sav, int size) {

	if (sav->size >= size) {
		sav->size -= size;
		sav->ptr += size;
		return(SUCCESS);
	}
	else {
		return(FAILURE);
	}
}

static BOOL skipsavestr(SAVEDATA sav) {

	if (sav->size > 0) {
		return(skipsavesize(sav, sav->ptr[0] + 1));
	}
	else {
		return(FAILURE);
	}
}

static BOOL getsavebyte(SAVEDATA sav, BYTE *ret) {

	if (sav->size <= 0) {
		return(FAILURE);
	}
	*ret = *sav->ptr++;
	sav->size -= 1;
	return(SUCCESS);
}

static BOOL getsaveword(SAVEDATA sav, UINT16 *ret) {

	if (sav->size < 2) {
		return(FAILURE);
	}
	*ret = LOADINTELWORD(sav->ptr);
	sav->ptr += 2;
	sav->size -= 2;
	return(SUCCESS);
}

static BOOL getsaveval(SAVEDATA sav, SINT32 *ret) {

	if (sav->size < 4) {
		return(FAILURE);
	}
	*ret = (SINT32)LOADINTELDWORD(sav->ptr);
	sav->ptr += 4;
	sav->size -= 4;
	return(SUCCESS);
}

static BOOL getsavestr(SAVEDATA sav, char *buf, int size) {

	int		rem;
	int		leng;

	rem = sav->size - 1;
	if (sav->size < 0) {
		goto gss_err;
	}
	leng = sav->ptr[0];
	rem -= leng;
	if (rem < 0) {
		goto gss_err;
	}
	if (size) {
		size = min(size - 1, leng);
		if (size) {
			CopyMemory(buf, sav->ptr + 1, size);
		}
		buf[size] = '\0';
	}
	leng += 1;
	sav->ptr += leng;
	sav->size -= leng;
	return(SUCCESS);

gss_err:
	return(FAILURE);
}

static int getgamesize(SAVEDATA sav) {

	_SAVEDATA	p;
	UINT16		stacks;
	BYTE		cnt;
	SINT32		val;

	p = *sav;
	if ((skipsavesize(&p, 2 + 1 + 1) != SUCCESS) ||
		(skipsavestr(&p) != SUCCESS) ||			// topic
		(skipsavestr(&p) != SUCCESS)) {			// script
		goto ggsz_err;
	}

	if ((getsaveword(&p, &stacks) != SUCCESS) ||
		(stacks != 0)) {
		goto ggsz_err;
	}

	if ((getsaveword(&p, &stacks) != SUCCESS) ||
		(stacks != 0)) {
		goto ggsz_err;
	}

	if (getsavebyte(&p, &cnt) != SUCCESS) {
		goto ggsz_err;
	}
	while(cnt--) {
		if ((skipsavesize(&p, 2 + 4 + 16 + 4 + 16) != SUCCESS) ||
			(skipsavestr(&p) != SUCCESS)) {
			goto ggsz_err;
		}
	}

	if (skipsavesize(&p, 2) != SUCCESS) {
		goto ggsz_err;
	}

	if (getsaveword(&p, &stacks) != SUCCESS) {
		goto ggsz_err;
	}
	while(stacks--) {
		if ((skipsavesize(&p, 1 + 4 + 16 + 4 + 16) != SUCCESS) ||
			(skipsavestr(&p) != SUCCESS)) {
			goto ggsz_err;
		}
	}

	if (skipsavesize(&p, 1 + 2) != SUCCESS) {
		goto ggsz_err;
	}

	if (getsavebyte(&p, &cnt) != SUCCESS) {
		goto ggsz_err;
	}
	while(cnt--) {
		if ((skipsavesize(&p, 3) != SUCCESS) ||
			(skipsavestr(&p) != SUCCESS)) {
			goto ggsz_err;
		}
	}

	if (skipsavesize(&p, 1 + 5 + 1) != SUCCESS) {
		goto ggsz_err;
	}

	if (skipsavesize(&p, 2) != SUCCESS) {
		goto ggsz_err;
	}


	if (getsaveval(&p, &val) != SUCCESS) {
		goto ggsz_err;
	}
	if (val > 0) {
		if (skipsavesize(&p, (val + 7) >> 3) != SUCCESS) {
			goto ggsz_err;
		}
	}

	if ((getsaveword(&p, &stacks) != SUCCESS) ||
		(skipsavesize(&p, stacks << 1) != SUCCESS)) {
		goto ggsz_err;
	}
	return(sav->size - p.size);

ggsz_err:
	return(0);
}


// ----

static BOOL saveexist(SAVEHDL hdl, int num) {

	int		saves;
	char	path[MAX_PATH];

	saves = savefile_getsaves();
	if ((num >= 0) && (num < saves)) {
		savegetname(num + 1, path, sizeof(path));
		if (file_attr(path) != -1) {
			return(TRUE);
		}
	}
	(void)hdl;
	return(FALSE);
}

static int getnewdate(SAVEHDL hdl) {

	int			ret;
	_SAVEDATE	base;
	int			saves;
	int			i;
	SAVEDATA	sav;
	int			pos;
	_SAVEDATE	cur;

	ret = -1;
	ZeroMemory(&base, sizeof(base));
	saves = savefile_getsaves();
	for (i=0; i<saves; i++) {
		sav = saveread(i + 1, TRUE);
		if (sav) {
			pos = 1;
			if (sav->size != 0) {
				pos += sav->ptr[0];
			}
			if (sav->size >= (int)(pos + sizeof(_SYSTIME))) {
				savefile_cnvdate(&cur, (_SYSTIME *)(sav->ptr + pos));
				if (savefile_cmpdate(&base, &cur) > 0) {
					ret = i;
					base = cur;
				}
			}
			_MFREE(sav);
		}
	}
	(void)hdl;
	return(ret);
}

static BOOL readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height) {

	int			saves;
	SAVEDATA	sav;

	if (inf == NULL) {
		goto rdif_err1;
	}
	ZeroMemory(inf, sizeof(_SAVEINF));
	saves = savefile_getsaves();
	if ((num < 0) || (num >= saves)) {
		goto rdif_err1;
	}
	sav = saveread(num + 1, TRUE);
	if (sav == NULL) {
		goto rdif_err1;
	}
	if (getsavestr(sav, inf->comment, sizeof(inf->comment)) != SUCCESS) {
		goto rdif_err2;
	}
	if (sav->size < (int)sizeof(_SYSTIME)) {
		goto rdif_err2;
	}
	savefile_cnvdate(&inf->date, (_SYSTIME *)sav->ptr);

	_MFREE(sav);
	return(SUCCESS);

rdif_err2:
	_MFREE(sav);

rdif_err1:
	(void)hdl;
	(void)width;
	(void)height;
	return(FAILURE);
}

static BOOL readgame(SAVEHDL hdl, int num) {

	int			saves;
	SAVEDATA	sav;
	int			savecnt;
	int			size;
	UINT16		snum;
	char		script[16];
	UINT16		stacks;
	BYTE		cnt;
	BYTE		pnum;
	char		label[16];
	BYTE		playing;
	UINT16		track;

	saves = savefile_getsaves();
	if ((num < 0) || (num >= saves)) {
		goto rdgm_err1;
	}
	sav = saveread(num + 1, TRUE);
	if (sav == NULL) {
		goto rdgm_err1;
	}
	if ((skipsavestr(sav) != SUCCESS) ||
		(skipsavesize(sav, sizeof(_SYSTIME)) != SUCCESS)) {
		goto rdgm_err2;
	}
	sav->size -= 4;
	if (sav->size < 0) {
		goto rdgm_err2;
	}
	savecnt = LOADINTELWORD(sav->ptr + 2);
	sav->ptr += 4;
	if (!savecnt) {
		goto rdgm_err2;
	}
	size = 0;
	while(savecnt-- > 0) {
		sav->ptr += size;
		sav->size -= size;
		size = getgamesize(sav);
		if (size == 0) {
			goto rdgm_err2;
		}
	}
	TRACEOUT(("pre load success %d %d", sav->size, size));

	getsaveword(sav, &snum);
	skipsavesize(sav, 2);
	getsavestr(sav, gamecore.comment, sizeof(gamecore.comment));
	getsavestr(sav, script, sizeof(script));

	getsaveword(sav, &stacks);
	getsaveword(sav, &stacks);

	getsavebyte(sav, &cnt);
	while(cnt) {
		cnt--;
		getsavebyte(sav, &pnum);
		skipsavesize(sav, 1 + 4 + 16 + 4 + 16);
		getsavestr(sav, label, sizeof(label));
		TRACEOUT(("%d: %s", pnum, label));
	}

	skipsavesize(sav, 2);
	getsaveword(sav, &stacks);
	while(stacks--) {
		skipsavesize(sav, 1 + 4 + 16 + 4 + 16);
		skipsavestr(sav);
	}

	getsavebyte(sav, &playing);
	getsaveword(sav, &track);

	getsavebyte(sav, &cnt);
	while(cnt--) {
		getsavebyte(sav, &pnum);
		skipsavesize(sav, 2);
		getsavestr(sav, label, sizeof(label));
		TRACEOUT(("%d: %s", pnum, label));
	}

	skipsavesize(sav, 1 + 5 + 1);

#if 0
	if (skipsavesize(&p, 2) != SUCCESS) {
		goto ggsz_err;
	}

	p.size -= 4;
	if (p.size < 0) {
		goto ggsz_err;
	}
	cnt = LOADINTELDWORD(p.ptr);
	p.ptr += 4;
	if (skipsavesize(&p, (cnt + 7) >> 3) != SUCCESS) {
		goto ggsz_err;
	}

	p.size -= 2;
	if (p.size < 0) {
		goto ggsz_err;
	}
	cnt = LOADINTELWORD(p.ptr);
	p.ptr += 2;
	if (skipsavesize(&p, cnt << 1) != SUCCESS) {
		goto ggsz_err;
	}
#endif


	_MFREE(sav);
	return(SUCCESS);

rdgm_err2:
	_MFREE(sav);

rdgm_err1:
	(void)hdl;
	return(FAILURE);
}

static BOOL readsysflag(SAVEHDL hdl, UINT pos, UINT size) {

const BYTE	*ptr;
	int		cnt;
	BYTE	bit;

	ptr = gamecore.drssys.flag;
	if (ptr == NULL) {
		goto rsfg_err1;
	}
	size = min(size, LOADINTELDWORD(ptr));
	TRACEOUT(("load flag :%d", size));
	ptr += 4;
	bit = 0;
	cnt = 0;
	while(size--) {
		if (!cnt) {
			bit = *ptr++;
			cnt = 8;
		}
		scr_flagop(pos++, (BYTE)(bit & 1));
		bit >>= 1;
		cnt--;
	}
	return(SUCCESS);

rsfg_err1:
	(void)hdl;
	return(FAILURE);
}

static BOOL readsysflagex(SAVEHDL hdl, UINT pos, UINT size) {

const BYTE	*ptr;
	int		cnt;
	BYTE	bit;

	ptr = gamecore.drssys.flag;
	if (ptr == NULL) {
		goto rsfg_err1;
	}
	size = min(size, LOADINTELDWORD(ptr));
	TRACEOUT(("load flag :%d", size));
	ptr += 4;
	bit = 0;
	cnt = 0;
	while(size--) {
		if (!cnt) {
			bit = *ptr++;
			cnt = 8;
		}
		if (bit & 1) {
			scr_flagop(pos++, 1);
		}
		bit >>= 1;
		cnt--;
	}
	return(SUCCESS);

rsfg_err1:
	(void)hdl;
	return(FAILURE);
}


// ----

static BOOL writesysflag(SAVEHDL hdl, UINT pos, UINT size) {

	UINT	flags;
	BYTE	*ptr;
	BYTE	bit;
	BYTE	cnt;
	BYTE	flag;
	BYTE	renewal;

	renewal = 0;
	ptr = gamecore.drssys.flag;
	if (ptr) {
		flags = LOADINTELDWORD(ptr);
		if (flags < size) {
			_MFREE(ptr);
			ptr = NULL;
		}
	}
	flags = (size + 7) >> 3;
	if (ptr == NULL) {
		ptr = (BYTE *)_MALLOC(4 + flags, "drs sys flag");
		gamecore.drssys.flag = ptr;
		if (ptr == NULL) {
			goto wsfg_err1;
		}
		STOREINTELDWORD(ptr, size);
		ZeroMemory(ptr + 4, flags);
		renewal = 1;
	}
	ptr += 4;

	bit = 0;
	cnt = 0;
	while(size--) {
		if ((scr_flagget(pos++, &flag) == SUCCESS) && (flag)) {
			bit |= (1 << cnt);
		}
		cnt = (cnt + 1) & 7;
		if (!cnt) {
			if (*ptr != bit) {
				*ptr = bit;
				renewal = 1;
			}
			ptr++;
			bit = 0;
		}
	}
	if (*ptr != bit) {
		*ptr = bit;
		renewal = 1;
	}
	gamecore.drssys.saverenewal |= renewal;
	return(SUCCESS);

wsfg_err1:
	(void)hdl;
	return(FAILURE);
}


// ----

#if 1			// test
static const _SAVEHDL savedrs = {
			savenone_close,
			saveexist, getnewdate, readinf,
			readgame, savenone_readflags,
			readsysflag, readsysflagex,
			savenone_exa, savenone_exa, savenone_exa,
			savenone_writegame, writesysflag,
			savenone_exa, savenone_exa, savenone_exa};
#else
static const _SAVEHDL savedrs = {
			savenone_close,
			saveexist, getnewdate, readinf,
			savenone_readgame, savenone_readflags,
			readsysflag, readsysflagex,
			savenone_exa, savenone_exa, savenone_exa,
			savenone_writegame, writesysflag,
			savenone_exa, savenone_exa, savenone_exa};
#endif

SAVEHDL savedrs_open(BOOL create) {

	(void)create;
	return((SAVEHDL)&savedrs);
}


// ---- drs global

typedef struct {
	char	script[16];
	UINT16	num;
} _DRSSCR, *DRSSCR;

static BOOL drssea(void *vpItem, void *vpArg) {

	if (!milstr_cmp(((DRSSCR)vpItem)->script, (char *)vpArg)) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

static UINT16 drs2num(const char *name) {

	LISTARRAY	la;
	DRSSCR		ds;
	_DRSSCR		dsnew;

	la = gamecore.drssys.scr;
	if (la == NULL) {
		la = listarray_new(sizeof(_DRSSCR), 64);
		gamecore.drssys.scr = la;
	}
	else {
		ds = (DRSSCR)listarray_enum(la, drssea, (void *)name);
		if (ds) {
			return(ds->num);
		}
	}
	milstr_ncpy(dsnew.script, name, sizeof(dsnew.script));
	dsnew.num = (UINT16)listarray_getitems(la);
	listarray_append(la, &dsnew);
	return(dsnew.num);
}

static BOOL kidsea(LISTARRAY la, UINT32 kid) {

	int			rem;
	UINT32		*p;

	while(la) {
		p = (UINT32 *)(la + 1);
		rem = la->items;
		while(rem > 0) {
			if (*p == kid) {
				return(TRUE);
			}
			p++;
			rem--;
		}
		la = la->laNext;
	}
	return(FALSE);
}

BOOL savedrs_getkid(UINT16 num, const char *name) {

	UINT32		kid;
	LISTARRAY	la;

	la = gamecore.drssys.kid;
	if (la == NULL) {
		return(FALSE);
	}
	kid = drs2num(name);
	kid <<= 16;
	kid |= num;
	return(kidsea(la, kid));
}

void savedrs_setkid(UINT16 num, const char *name) {

	UINT32		kid;
	LISTARRAY	la;

	kid = drs2num(name);
	kid <<= 16;
	kid |= num;
	la = gamecore.drssys.kid;
	if (la == NULL) {
		la = listarray_new(sizeof(UINT32),
							(4096 - sizeof(_LISTARRAY)) / sizeof(UINT32));
		gamecore.drssys.kid = la;
	}
	else {
		if (kidsea(la, kid)) {
			return;
		}
	}
	gamecore.drssys.saverenewal = 1;
	listarray_append(la, &kid);
}

void savedrs_sysread(void) {

	SAVEDATA	sav;
const BYTE		*ptr;
	int			rem;
	int			cnt;
	int			leng;
	int			size;
	LISTARRAY	la;
	UINT32		kid;
	UINT32		curdrs;
	BYTE		*curp;
	char		name[16];

	gamecore.drssys.saverenewal = 0;
	sav = saveread(0, FALSE);
	if (sav == NULL) {
		goto sdsr_err1;
	}
	ptr = sav->ptr;
	rem = sav->size;
	rem -= sizeof(_SYSTIME);
	if (rem < 0) {
		goto sdsr_err2;
	}
	ptr += sizeof(_SYSTIME);

	rem -= 4;
	if (rem < 0) {
		goto sdsr_err2;
	}
	cnt = LOADINTELWORD(ptr + 2);
	ptr += 4;

	listarray_destroy(gamecore.drssys.kid);
	la = listarray_new(sizeof(UINT32),
							(4096 - sizeof(_LISTARRAY)) / sizeof(UINT32));
	gamecore.drssys.kid = la;
	curp = NULL;
	curdrs = 0;				// for gcc
	while(cnt-- > 0) {
		rem -= 3;
		if (rem < 0) {
			goto sdsr_err2;
		}
		leng = ptr[2];
		rem -= leng;
		if (rem < 0) {
			goto sdsr_err2;
		}
		if ((curp) && (memcmp(ptr + 2, curp, leng + 1))) {
			curp = NULL;
		}
		if (curp == NULL) {
			size = min(leng, (int)(sizeof(name) - 1));
			CopyMemory(name, ptr + 3, size);
			name[size] = 0;
			curdrs = drs2num(name);
			curdrs <<= 16;
		}
		kid = LOADINTELWORD(ptr);
		ptr += 3 + leng;
		kid |= curdrs;
		listarray_append(la, &kid);
	}

	rem -= 4;
	if (rem < 0) {
		goto sdsr_err2;
	}
	cnt = LOADINTELDWORD(ptr);
	cnt = min(cnt, rem * 8);
	if (cnt) {
		_MFREE(gamecore.drssys.flag);
		leng = (cnt + 7) >> 3;
		curp = (BYTE *)_MALLOC(4 + leng, "drs sys");
		gamecore.drssys.flag = curp;
		if (curp) {
			STOREINTELDWORD(curp, cnt);
			CopyMemory(curp + 4, ptr + 4, leng);
		}
	}

sdsr_err2:
	_MFREE(sav);

sdsr_err1:
	return;
}

static BOOL numsea(void *vpItem, void *vpArg) {

	if (((DRSSCR)vpItem)->num == (UINT16)(long)vpArg) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

void savedrs_syswrite(void) {

	UINT		savesize;
	UINT		kidcnt;
	UINT32		curkid;
	UINT		curkidlen;
	UINT32		kid;
	UINT32		num;
	LISTARRAY	scr;
	LISTARRAY	la;
const UINT32	*v;
	int			rem;
	DRSSCR		ds;
const BYTE		*flag;
	UINT		flagsize;
	BYTE		*sav;
	BYTE		*q;

	if (!gamecore.drssys.saverenewal) {
		return;
	}

	savesize = sizeof(_SYSTIME) + 4;
	kidcnt = 0;
	curkid = 0x10000;
	curkidlen = 0;			// for gcc
	ds = NULL;				// for gcc
	scr = gamecore.drssys.scr;
	la = gamecore.drssys.kid;
	while(la) {
		v = (UINT32 *)(la + 1);
		rem = la->items;
		while(rem > 0) {
			kid = *v++;
			rem--;
			num = kid >> 16;
			if (curkid != num) {
				ds = (DRSSCR)listarray_enum(scr, numsea, (void *)(long)num);
				if (ds == NULL) {
					continue;
				}
				curkid = num;
				curkidlen = strlen(ds->script);
			}
			savesize += curkidlen + 3;
			kidcnt++;
		}
		la = la->laNext;
	}

	flag = gamecore.drssys.flag;
	flagsize = 0;
	if (flag) {
		flagsize = LOADINTELDWORD(flag);
		flagsize += 7;
		flagsize >>= 3;
	}
	savesize += flagsize + 4;

	sav = (BYTE *)_MALLOC(savesize, "save tmp");
	if (sav == NULL) {
		goto sdsw_err1;
	}
	q = sav;
	timemng_gettime((_SYSTIME *)q);
	q += sizeof(_SYSTIME);

	STOREINTELWORD(q + 2, kidcnt);
	if (!kidcnt) {
		kidcnt++;
	}
	kidcnt += 127;
	kidcnt &= ~127;
	STOREINTELWORD(q + 0, kidcnt);
	q += 4;

	la = gamecore.drssys.kid;
	while(la) {
		v = (UINT32 *)(la + 1);
		rem = la->items;
		while(rem > 0) {
			kid = *v++;
			rem--;
			num = kid >> 16;
			if (curkid != num) {
				ds = (DRSSCR)listarray_enum(scr, numsea, (void *)(long)num);
				if (ds == NULL) {
					continue;
				}
				curkid = num;
				curkidlen = strlen(ds->script);
			}
			STOREINTELWORD(q, kid);
			q[2] = (BYTE)curkidlen;
			q += 3;
			if (curkidlen) {
				CopyMemory(q, ds->script, curkidlen);
			}
			q += curkidlen;
		}
		la = la->laNext;
	}
	if (flag) {
		CopyMemory(q, flag, flagsize + 4);
	}
	else {
		ZeroMemory(q, 4);
	}
	savewrite(0, FALSE, sav, savesize);
	_MFREE(sav);
	gamecore.drssys.saverenewal = 0;

sdsw_err1:
	return;
}

