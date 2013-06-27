#include	"compiler.h"
#include	"dosio.h"
#include	"timemng.h"
#include	"gamecore.h"
#include	"savefile.h"
#include	"saveisf.h"
#include	"bmpdata.h"


typedef struct {
	BYTE	*ptr;
	int		size;
} _SAVEDATA, *SAVEDATA;

typedef struct {
	FILEH	fh;
	UINT	fsize;
	UINT	headsize;
	int		gamesaves;					// ゲームセーブ数
	int		files;						// トータル
	int		commsize;
	int		savever;
} _SAVEISF, *SAVEISF;


static void getpreviewsize(SAVEISF isf, BMPDATA *pv, int vnum, VRAMHDL vram) {

	if (vnum != -1) {
		if ((isf->savever <= 1) || (vram == NULL)) {
			pv->width = 240;
			pv->height = -180;
		}
		else {
#ifndef SIZE_QVGA
			pv->width = vram->width;
			pv->height = vram->height;
#else
			pv->width = vramdraw_orgsize(vram->width);
			pv->height = vramdraw_orgsize(vram->height);
#endif
			pv->width = min(pv->width, 240);
			pv->height = min(pv->height, 180);
			pv->height = 0 - pv->height;
		}
	}
	else {
		if (gamecore.sys.version < EXEVER_KONYA2) {
			pv->width = 160;
			pv->height = -120;
		}
		else {
			pv->width = gamecore.sys.bmpwidth;
			pv->height = 0 - gamecore.sys.bmpheight;
		}
	}
	pv->bpp = 24;
}


// ----

static BOOL savezerofill(FILEH fh, UINT size) {

	UINT	wsize;
	char	work[1024];

	ZeroMemory(work, sizeof(work));
	while(size) {
		wsize = min(size, sizeof(work));
		if (file_write(fh, work, wsize) != wsize) {
			return(FAILURE);
		}
		size -= wsize;
	}
	return(SUCCESS);
}

static UINT savegetinitsize(SAVEISF isf) {

	UINT	size;
	UINT	type;
	BMPINFO	bi;
	FLAGS	flags;
	BMPDATA	pv;

	if (gamecore.sys.version == EXEVER_MYU) {
		return(0x80000);
	}
	flags = &gamecore.flags;
	size = sizeof(_SYSTIME);
	type = gamecore.sys.type;
	if (type & GAME_SAVEGRPH) {
		size += sizeof(BMPINFO);
		getpreviewsize(isf, &pv, -1, NULL);
		bmpdata_setinfo(&bi, &pv, FALSE);
		size += bmpdata_getdatasizeex(&bi, FALSE);
	}
	if (type & GAME_SAVECOM) {
		size += isf->commsize;
	}
	size += flags->flagsize;
	size += flags->maxval * 4;
	size += sizeof(SINT32) * 12;
	return(isf->headsize + (isf->gamesaves * size));
}

static BOOL savever2header(FILEH fh, SAVEISF isf) {

	BYTE	*ptr;
	UINT	wsize;

	ptr = (BYTE *)_MALLOC(isf->headsize, "SAVE ISF ver2");
	if (ptr == NULL) {
		goto sv2h_err1;
	}
	STOREINTELDWORD(ptr + 0, isf->headsize);
	STOREINTELDWORD(ptr + 4, isf->files);
	FillMemory(ptr + 0x008, 0xb40, 0xff);
	ZeroMemory(ptr + 0xb48, isf->headsize - 0xb48);

//	STOREINTELDWORD(ptr + 8 + 0x060, 6);
//	STOREINTELDWORD(ptr + 8 + 0x240, 1);
//	STOREINTELDWORD(ptr + 8 + 0x244, 1);
//	STOREINTELDWORD(ptr + 8 + 0x248, 1);

	wsize = file_write(fh, ptr, isf->headsize);
	_MFREE(ptr);

	if (wsize == isf->headsize) {
		return(SUCCESS);
	}

sv2h_err1:
	return(FAILURE);
}

static FILEH saveopen(SAVEISF isf, BOOL create) {

	FILEH	fh;
	char	path[MAX_PATH];
	BOOL	r;

	milstr_ncpy(path, gamecore.suf.scriptpath, sizeof(path));

	if (!(gamecore.sys.type & GAME_SAVEALIAS)) {
		switch(gamecore.sys.version) {
			case EXEVER_MYU:
			case EXEVER_AMEIRO:
			case EXEVER_NURSE:
			case EXEVER_VECHO:
				milstr_ncat(path, "GAME.SAV", sizeof(path));
				break;

			case EXEVER_TSUKU:
				milstr_ncat(path, "TSUKU.SAV", sizeof(path));
				break;

			default:
				milstr_ncat(path, gamecore.suf.key, sizeof(path));
				milstr_ncat(path, ".SAV", sizeof(path));
				break;
		}
	}
	else {
		milstr_ncat(path, "xkazoku.sav", sizeof(path));
	}
	TRACEOUT(("savefile: %s", path));
	if (!create) {
		fh = file_open_rb(path);
	}
	else {
		fh = file_open(path);
	}
	if (fh != FILEH_INVALID) {
		if (isf->savever == 2) {
			BYTE work[8];
			if (file_read(fh, work, 8) == 8) {
				isf->headsize = LOADINTELDWORD(work+0);
				isf->files = LOADINTELDWORD(work+4);
			}
			file_seek(fh, 0, FSEEK_SET);
		}
	}
	else if (create) {
		fh = file_create(path);
		if (fh != FILEH_INVALID) {
			if (isf->savever == 0) {
				r = savezerofill(fh, savegetinitsize(isf));
			}
			else if (isf->savever == 1) {
				r = savezerofill(fh, isf->headsize);
			}
			else {
				r = savever2header(fh, isf);
			}
			if (r != SUCCESS) {
				file_close(fh);
				fh = FILEH_INVALID;
				file_delete(path);
			}
		}
	}
	return(fh);
}


// ----

static UINT savegetpos(SAVEISF isf, int num) {

const BYTE	*p;

	p = (BYTE *)(isf + 1);
	if (isf->savever <= 1) {
		p += num * 4;
	}
	else {
		p += (isf->headsize - ((isf->files - num) * 0x40));
	}
	return(LOADINTELDWORD(p));
}

static void savesetpos(SAVEISF isf, int num, UINT pos) {

	BYTE	*p;

	p = (BYTE *)(isf + 1);
	if (isf->savever <= 1) {
		p += num * 4;
	}
	else {
		p += (isf->headsize - ((isf->files - num) * 0x40));
	}
	STOREINTELDWORD(p, pos);
}


// ----

static UINT savegetsize(SAVEISF isf, int num) {

	UINT	base;
	UINT	next;
	int		i;
	UINT	pos;

	if ((num < 0) || (num >= isf->files)) {
		goto svgs_err;
	}
	base = savegetpos(isf, num);
	if (base == 0) {
		goto svgs_err;
	}
	next = isf->fsize;
	for (i=0; i<isf->files; i++) {
		pos = savegetpos(isf, i);
		if ((pos > base) && (pos < next)) {
			next = pos;
		}
	}
	return(next - base);

svgs_err:
	return(0);
}

static SAVEDATA saveread(SAVEISF isf, int num) {

	UINT		size;
	SAVEDATA	ret;
	long		pos;

	size = savegetsize(isf, num);
	if (size == 0) {
		goto svrd_err1;
	}
	ret = (SAVEDATA)_MALLOC(sizeof(_SAVEDATA) + size, "save data");
	if (ret == NULL) {
		goto svrd_err1;
	}
	ret->ptr = (BYTE *)(ret + 1);
	ret->size = size;
	pos = savegetpos(isf, num);
	if ((file_seek(isf->fh, pos, FSEEK_SET) != pos) ||
		(file_read(isf->fh, (ret + 1), size) != size)) {
		goto svrd_err2;
	}
	return(ret);

svrd_err2:
	_MFREE(ret);

svrd_err1:
	return(NULL);
}

static BOOL savewriteseek(SAVEISF isf, int num, UINT newsize) {

	int		i;
	UINT	basepos;
	UINT	orgsize;
	UINT	extsize;
	UINT	size;
	char	work[1024];
	UINT	pos;

	if ((num < 0) || (num >= isf->files)) {
		return(FAILURE);
	}
	orgsize = savegetsize(isf, num);

	if (orgsize < newsize) {
		// 書込み番地を指定
		basepos = savegetpos(isf, num);
		if ((isf->savever == 0) && (num < isf->gamesaves)) {
			if (basepos == 0) {
				basepos = isf->headsize;
				basepos += num * newsize;
			}
			else {
				return(FAILURE);
			}
		}
		else {
			if (basepos == 0) {
				for (i=num; i--;) {
					basepos = savegetpos(isf, i);
					if (basepos) {
						basepos += savegetsize(isf, i);
						break;
					}
				}
				if (basepos == 0) {
					basepos = isf->headsize;
				}
				if (isf->savever == 0) {
					size = savegetinitsize(isf);
					basepos = max(basepos, size);
				}
			}

			// 先に増幅サイズ分書込み
			extsize = newsize - orgsize;
			if ((file_seek(isf->fh, isf->fsize, FSEEK_SET)
													!= (long)isf->fsize) ||
				(savezerofill(isf->fh, extsize) != SUCCESS)) {
				return(FAILURE);
			}

			// で移動。 エラーだとファイル破壊なのでチェキ無意味
			pos = isf->fsize;
			while(pos > basepos) {
				size = min(pos - basepos, sizeof(work));
				pos -= size;
				file_seek(isf->fh, pos, FSEEK_SET);
				file_read(isf->fh, work, size);
				file_seek(isf->fh, pos + extsize, FSEEK_SET);
				file_write(isf->fh, work, size);
			}
			isf->fsize += extsize;

			for (i=0; i<isf->files; i++) {
				pos = savegetpos(isf, i);
				if ((i != num) && (pos) && (pos >= basepos)) {
					pos += extsize;
					savesetpos(isf, i, pos);
				}
			}
		}
		savesetpos(isf, num, basepos);

		// TABLE更新　エラーだとファイル破壊なのでチェキ無意味
		file_seek(isf->fh, 0, FSEEK_SET);
		file_write(isf->fh, isf + 1, isf->headsize);
	}

	pos = savegetpos(isf, num);
	if (file_seek(isf->fh, pos, FSEEK_SET) != (long)pos) {
		return(FAILURE);
	}
	return(SUCCESS);
}


// ----

static void saveclose(SAVEHDL hdl) {

	SAVEISF	isf;

	isf = (SAVEISF)(hdl + 1);
	file_close(isf->fh);
	_MFREE(hdl);
}


// ---- read系。

static BOOL saveexist(SAVEHDL hdl, int num) {

	SAVEISF	isf;

	isf = (SAVEISF)(hdl + 1);
	if ((num >= 0) && (num < isf->gamesaves) &&
		(savegetsize(isf, num) != 0)) {
		return(TRUE);
	}
	return(FALSE);
}

static int getnewdate(SAVEHDL hdl) {

	int			ret;
	SAVEISF		isf;
	_SAVEDATE	base;
	int			i;
	UINT		size;
	long		pos;
	_SYSTIME	st;
	_SAVEDATE	cur;

	ret = -1;
	isf = (SAVEISF)(hdl + 1);
	ZeroMemory(&base, sizeof(base));
	for (i=0; i<isf->gamesaves; i++) {
		size = savegetsize(isf, i);
		if (size < sizeof(_SYSTIME)) {
			continue;
		}
		pos = savegetpos(isf, i);
		if (file_seek(isf->fh, pos, FSEEK_SET) != pos) {
			continue;
		}

		if (file_read(isf->fh, &st, sizeof(_SYSTIME)) != sizeof(_SYSTIME)) {
			continue;
		}
		savefile_cnvdate(&cur, &st);
		if (savefile_cmpdate(&base, &cur) > 0) {
			ret = i;
			base = cur;
		}
	}
	return(ret);
}

static BOOL readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height) {

	SAVEISF		isf;
	SAVEDATA	dat;
	int			remain;
	BYTE		*ptr;
	BMPINFO		*bi;
	UINT		bmpsize;

	if (inf == NULL) {
		goto sfrt_err1;
	}
	ZeroMemory(inf, sizeof(_SAVEINF));
	isf = (SAVEISF)(hdl + 1);
	if ((num < 0) || (num >= isf->gamesaves)) {
		goto sfrt_err1;
	}

	dat = saveread(isf, num);
	if (dat == NULL) {
		goto sfrt_err1;
	}

	ptr = dat->ptr;
	remain = dat->size;

	remain -= sizeof(_SYSTIME);
	if (remain < 0) {
		goto sfrt_err2;
	}
	savefile_cnvdate(&inf->date, ptr);
	ptr += sizeof(_SYSTIME);

	if (gamecore.sys.type & GAME_SAVEGRPH) {
		remain -= sizeof(BMPINFO);
		if (remain < 0) {
			goto sfrt_err2;
		}
		bi = (BMPINFO *)ptr;
		if (LOADINTELDWORD(bi->biSize) != sizeof(BMPINFO)) {
			goto sfrt_err2;
		}
		ptr += sizeof(BMPINFO);
		bmpsize = bmpdata_getdatasizeex(bi, FALSE);
		remain -= bmpsize;
		if (remain < 0) {
			goto sfrt_err2;
		}
		if (width < 0) {
			width = LOADINTELDWORD(bi->biWidth);
#ifdef SIZE_QVGA
			width = vramdraw_half(width);
#endif
		}
		if (height < 0) {
			height = (SINT32)LOADINTELDWORD(bi->biHeight);
			if (height < 0) {
				height *= -1;
			}
#ifdef SIZE_QVGA
			height = vramdraw_half(height);
#endif
		}
		inf->preview = bmpdata_vram24cnvex(width, height, bi, ptr, FALSE);
		ptr += bmpsize;
	}

	inf->comment[0] = '\0';
	if (gamecore.sys.type & GAME_SAVECOM) {
		remain -= isf->commsize;
		if (remain < 0) {
			goto sfrt_err2;
		}
		CopyMemory(inf->comment, ptr, isf->commsize);
		inf->comment[isf->commsize] = '\0';
		ptr += isf->commsize;
	}

	_MFREE(dat);
	return(SUCCESS);

sfrt_err2:
	vram_destroy((VRAMHDL)inf->preview);
	inf->preview = NULL;
	_MFREE(dat);

sfrt_err1:
	return(FAILURE);
}

static BOOL readgame(SAVEHDL hdl, int num) {

	SAVEISF		isf;
	FLAGS		flags;
	SAVEDATA	dat;
	BYTE		*ptr;
	int			remain;
	int			size;
	UINT		i;
	BMPINFO		*bi;
	UINT		bmpsize;
	UINT		type;

	isf = (SAVEISF)(hdl + 1);
	if ((num < 0) || (num >= isf->gamesaves)) {
		goto sfrg_err1;
	}

	flags = &gamecore.flags;
	type = gamecore.sys.type;

	dat = saveread(isf, num);
	if (dat == NULL) {
		goto sfrg_err1;
	}

	ptr = dat->ptr + sizeof(_SYSTIME);
	remain = dat->size - sizeof(_SYSTIME);
	if (remain < 0) {
		goto sfrg_err2;
	}

	if (type & GAME_SAVEGRPH) {
		remain -= sizeof(BMPINFO);
		if (remain < 0) {
			goto sfrg_err2;
		}
		bi = (BMPINFO *)ptr;
		ptr += sizeof(BMPINFO);
		bmpsize = bmpdata_getdatasizeex(bi, FALSE);
		remain -= bmpsize;
		if (remain < 0) {
			goto sfrg_err2;
		}
		ptr += bmpsize;
	}

	size = 0;
	if (type & GAME_SAVECOM) {
		size += isf->commsize;
	}
	size += flags->flagsize;
	size += flags->maxval * 4;

	if (remain < size) {
		goto sfrg_err2;
	}

	if (type & GAME_SAVECOM) {
		if (!(type & GAME_SAVEAUTOCOM)) {
			CopyMemory(gamecore.comment, ptr, isf->commsize);
		}
		ptr += isf->commsize;
		remain -= isf->commsize;
	}

	size = flags->flagsize;
	if (size) {
		CopyMemory(flags->flag, ptr, size);
		ptr += size;
		remain -= size;
	}

	for (i=0; i<flags->maxval; i++) {
		flags->val[i] = LOADINTELDWORD(ptr);
		ptr += 4;
		remain -= 4;
	}

	_MFREE(dat);

	scr_restart("LOAD");
	return(SUCCESS);

sfrg_err2:
	_MFREE(dat);

sfrg_err1:
	return(FAILURE);
}

static BOOL readflags(SAVEHDL hdl, int num, UINT reg, UINT pos, UINT cnt) {

	SAVEISF		isf;
	FLAGS		flags;
	SAVEDATA	dat;
	BYTE		*ptr;
	int			remain;
	int			size;
	BMPINFO		*bi;
	UINT		bmpsize;
	UINT		type;

	isf = (SAVEISF)(hdl + 1);
	if ((num < 0) || (num >= isf->gamesaves)) {
		goto sfrg_err1;
	}

	flags = &gamecore.flags;
	type = gamecore.sys.type;

	dat = saveread(isf, num);
	if (dat == NULL) {
		goto sfrg_err1;
	}

	ptr = dat->ptr + sizeof(_SYSTIME);
	remain = dat->size - sizeof(_SYSTIME);
	if (remain < 0) {
		goto sfrg_err2;
	}

	if (type & GAME_SAVEGRPH) {
		remain -= sizeof(BMPINFO);
		if (remain < 0) {
			goto sfrg_err2;
		}
		bi = (BMPINFO *)ptr;
		ptr += sizeof(BMPINFO);
		bmpsize = bmpdata_getdatasizeex(bi, FALSE);
		remain -= bmpsize;
		if (remain < 0) {
			goto sfrg_err2;
		}
		ptr += bmpsize;
	}

	size = 0;
	if (type & GAME_SAVECOM) {
		size += isf->commsize;
	}
	size += flags->flagsize;
	if (remain < (size + (int)(flags->maxval * 4))) {
		goto sfrg_err2;
	}
	ptr += size;
	remain -= size;

	while(cnt) {
		if ((reg < flags->maxval) && (pos < flags->maxval)) {
			flags->val[reg] = LOADINTELDWORD(ptr + (pos * 4));
		}
		reg++;
		pos++;
		cnt--;
	}

	_MFREE(dat);
	return(SUCCESS);

sfrg_err2:
	_MFREE(dat);

sfrg_err1:
	return(FAILURE);
}

static BOOL readsysflag(SAVEHDL hdl, UINT pos, UINT size) {

	SAVEISF		isf;
	SAVEDATA	dat;
	BOOL		r = FAILURE;

	isf = (SAVEISF)(hdl + 1);
	dat = saveread(isf, isf->gamesaves);
	if (dat == NULL) {
		return(FAILURE);
	}
	if (dat->size > 4) {
		BYTE	*ptr;
		BYTE	bit;
		BYTE	cnt;
		size = min(size, LOADINTELDWORD(dat->ptr));
		size = min(size, (UINT)(dat->size - 4) * 8);
		ptr = dat->ptr + 4;
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
		r = SUCCESS;
	}
	_MFREE(dat);
	return(r);
}

static BOOL savereadexa(SAVEHDL hdl, int num, _VARIANT *val) {

	SAVEISF		isf;
	SAVEDATA	dat;
	int			size;

	if (val == NULL) {
		goto svre_err;
	}
	isf = (SAVEISF)(hdl + 1);
	dat = saveread(isf, isf->gamesaves + num);
	if (dat == NULL) {
		goto svre_err;
	}
	if (dat->size >= 4) {
		size = LOADINTELDWORD(dat->ptr);
		size *= val->type;
		size /= 32;
		size++;
		size *= 4;

		size = min(size, val->size);
		size = min(size, dat->size - 4);
		if (size) {
			TRACEOUT(("read flag %dbyte(s)", size));
			CopyMemory(val + 1, dat->ptr + 4, size);
		}
	}
	_MFREE(dat);
	return(SUCCESS);

svre_err:
	return(FAILURE);
}

static BOOL readexaflag(SAVEHDL hdl, void *val) {

	return(savereadexa(hdl, 0, (_VARIANT *)val));
}

static BOOL readexaval(SAVEHDL hdl, void *val) {

	return(savereadexa(hdl, 1, (_VARIANT *)val));
}

static BOOL savereadkid0(SAVEISF isf, _VARIANT *val) {

	SAVEDATA	dat;
	BYTE		*ptr;
	int			rem;
	int			size;
	UINT		cmd;
	int			pos;

	dat = saveread(isf, isf->gamesaves + 2);
	if (dat == NULL) {
		goto sfrk_err;
	}
	ptr = dat->ptr;
	rem = dat->size;
	rem -= 4;
	if (rem < 0) {
		goto sfrk_done;
	}
	size = LOADINTELDWORD(ptr);
	ptr += 4;
	rem = min(rem, size);
	pos = 0;
	while(1) {
		rem--;
		if (rem < 0) {
			break;
		}
		cmd = *ptr++;
		size = cmd & 0x1f;
		while(cmd & 0x60) {
			cmd -= 0x20;
			rem--;
			if (rem < 0) {
				goto sfrk_done;
			}
			size <<= 8;
			size |= (*ptr++);
		}
		cmd >>= 7;
		while(size--) {
			variant_set(val, pos, cmd);
			pos++;
		}
	}
	TRACEOUT(("read kid %d %d", pos, val->max));

sfrk_done:
	_MFREE(dat);
	return(SUCCESS);

sfrk_err:
	return(FAILURE);
}

static BOOL savereadkid2(SAVEISF isf, _VARIANT *val) {

	SAVEDATA	dat;
	int			size;

	dat = saveread(isf, isf->gamesaves + 2);
	if (dat == NULL) {
		return(FAILURE);
	}
	size = min(dat->size, val->size);
	if (size) {
		TRACEOUT(("read kid %d", size * 8));
		CopyMemory(val + 1, dat->ptr, size);
	}
	_MFREE(dat);
	return(SUCCESS);
}

static BOOL readkid(SAVEHDL hdl, void *val) {

	SAVEISF		isf;

	if (val == NULL) {
		return(FAILURE);
	}
	isf = (SAVEISF)(hdl + 1);
	if (isf->savever <= 1) {
		return(savereadkid0(isf, (_VARIANT *)val));
	}
	else {
		return(savereadkid2(isf, (_VARIANT *)val));
	}
}


// ---- write系

static BOOL writegame(SAVEHDL hdl, int num, int vnum) {

	SAVEISF		isf;
	VRAMHDL		vram;
	FLAGS		flags;
	UINT		size;
	UINT		i;
	_SYSTIME	gdate;
	BYTE		val[4];
	BMPINFO		bi;
	UINT		bmpsize;
	BYTE		*dat;
	BMPDATA		pv;
	UINT		type;

	flags = &gamecore.flags;
	type = gamecore.sys.type;

	isf = (SAVEISF)(hdl + 1);
	if ((num < 0) || (num >= isf->gamesaves)) {
		goto sfwg_err1;
	}

	ZeroMemory(&gdate, sizeof(gdate));
	timemng_gettime(&gdate);

	if ((vnum < 0) || (vnum >= GAMECORE_MAXVRAM)) {
		vram = gamecore.vram[gamecore.dispwin.vramnum];
	}
	else {
		vram = gamecore.vram[vnum];
	}

	getpreviewsize(isf, &pv, vnum, vram);
	bmpdata_setinfo(&bi, &pv, FALSE);
	bmpsize = bmpdata_getdatasizeex(&bi, FALSE);

	// セーブのサイズを求める～
	size = sizeof(gdate);

	if (type & GAME_SAVEGRPH) {
		size += sizeof(BMPINFO);
		size += bmpsize;
	}

	if (type & GAME_SAVECOM) {
		size += isf->commsize;
	}
	size += flags->flagsize;
	size += flags->maxval * 4;
	size += sizeof(SINT32) * 12;

	if (savewriteseek(isf, num, size) != SUCCESS) {
		return(FAILURE);
	}

	file_write(isf->fh, &gdate, sizeof(gdate));

	if (type & GAME_SAVEGRPH) {
		file_write(isf->fh, &bi, sizeof(bi));
		dat = bmpdata_bmp24cnvex(&bi, vram, FALSE);
		if (dat) {
			file_write(isf->fh, dat, bmpsize);
			_MFREE(dat);
		}
		else {
			savezerofill(isf->fh, bmpsize);
		}
	}

	if (type & GAME_SAVECOM) {
		file_write(isf->fh, gamecore.comment, isf->commsize);
	}

	file_write(isf->fh, flags->flag, flags->flagsize);
	for (i=0; i<flags->maxval; i++) {
		STOREINTELDWORD(val, flags->val[i]);
		file_write(isf->fh, val, 4);
	}

	return(SUCCESS);

sfwg_err1:
	return(FAILURE);
}

static BOOL writesysflag(SAVEHDL hdl, UINT pos, UINT size) {

	SAVEISF	isf;
	BYTE	*dat;
	BYTE	*p;
	BYTE	bit;
	BYTE	flag;
	UINT	datsize;

	datsize = 4;
	datsize += (size + 7) / 8;
	dat = (BYTE *)_MALLOC(datsize, "flagwrite tmp");
	if (dat == NULL) {
		goto sfws_err1;
	}

	ZeroMemory(dat, datsize);
	STOREINTELDWORD(dat, size);
	p = dat + 4;
	bit = 1;
	while(size--) {
		if ((scr_flagget(pos++, &flag) == SUCCESS) && (flag)) {
			*p |= bit;
		}
		bit <<= 1;
		if (bit == 0) {
			bit++;
			p++;
		}
	}

	isf = (SAVEISF)(hdl + 1);
	if (savewriteseek(isf, isf->gamesaves, datsize) != SUCCESS) {
		goto sfws_err2;
	}
	file_write(isf->fh, dat, datsize);
	_MFREE(dat);
	return(SUCCESS);

sfws_err2:
	_MFREE(dat);

sfws_err1:
	return(FAILURE);
}

static BOOL savewriteexa(SAVEHDL hdl, int num, _VARIANT *val) {

	SAVEISF	isf;
	BYTE	size[4];

	if (val == NULL) {
		return(FAILURE);
	}
	isf = (SAVEISF)(hdl + 1);
	if (savewriteseek(isf, isf->gamesaves + num, 4 + val->size) != SUCCESS) {
		return(FAILURE);
	}

	STOREINTELDWORD(size, val->max);
	file_write(isf->fh, size, 4);
	file_write(isf->fh, val + 1, val->size);
	return(SUCCESS);
}

static BOOL writeexaflag(SAVEHDL hdl, void *val) {

	return(savewriteexa(hdl, 0, (_VARIANT *)val));
}

static BOOL writeexaval(SAVEHDL hdl, void *val) {

	return(savewriteexa(hdl, 1, (_VARIANT *)val));
}

static BOOL savewritekid0(SAVEISF isf, _VARIANT *val) {

	int		pos;
	UINT	wsize;
	BYTE	bit;
	BYTE	cur;
	UINT	bitsize;
	BYTE	*dat;
	BYTE	*ptr;

	wsize = 0;

	pos = ((_VARIANT *)val)->max;
	pos--;
	variant_get(val, pos, &bit);
	do {
		bitsize = 1;
		while(1) {
			pos--;
			if (pos < 0) {
				break;
			}
			variant_get(val, pos, &cur);
			if (cur != bit) {
				break;
			}
			bitsize++;
		}
		wsize++;
		while(bitsize & (~0x1f)) {
			bitsize >>= 8;
			wsize++;
		}
		bit = cur;
	} while(pos >= 0);

	dat = (BYTE *)_MALLOC(wsize + 4, "work");
	if (dat == NULL) {
		goto sfwk0_err;
	}
	ptr = dat;
	STOREINTELDWORD(ptr, wsize);
	ptr = dat + wsize + 4;

	pos = ((_VARIANT *)val)->max;
	pos--;
	variant_get(val, pos, &bit);
	do {
		bitsize = 1;
		while(1) {
			pos--;
			if (pos < 0) {
				break;
			}
			variant_get(val, pos, &cur);
			if (cur != bit) {
				break;
			}
			bitsize++;
		}
		bit <<= 7;
		while(bitsize & (~0x1f)) {
			ptr--;
			*ptr = (BYTE)bitsize;
			bitsize >>= 8;
			bit += 0x20;
		}
		ptr--;
		*ptr = (BYTE)(bitsize | bit);
		bit = cur;
	} while(pos >= 0);

	if (savewriteseek(isf, isf->gamesaves + 2, wsize + 4) == SUCCESS) {
		file_write(isf->fh, dat, wsize + 4);
	}
	_MFREE(dat);
	return(SUCCESS);

sfwk0_err:
	return(FAILURE);
}

static BOOL savewritekid2(SAVEISF isf, _VARIANT *val) {

	if (savewriteseek(isf, isf->gamesaves + 2, val->size) == SUCCESS) {
		file_write(isf->fh, val + 1, val->size);
	}
	return(SUCCESS);
}

static BOOL writekid(SAVEHDL hdl, void *val) {

	SAVEISF	isf;

	if ((val == NULL) || (((_VARIANT *)val)->max <= 0)) {
		return(FAILURE);
	}
	isf = (SAVEISF)(hdl + 1);
	if (isf->savever <= 1) {
		return(savewritekid0(isf, (_VARIANT *)val));
	}
	else {
		return(savewritekid2(isf, (_VARIANT *)val));
	}
}


// ----

static const _SAVEHDL saveisf = {
			saveclose,
			saveexist, getnewdate, readinf,
			readgame, readflags,
			readsysflag, savenone_sys,
			readexaflag, readexaval, readkid,
			writegame, writesysflag,
			writeexaflag, writeexaval, writekid};

static void setinf(SAVEHDL hdl, SAVEISF isf) {

	int		version;
	UINT	type;
	int		saves;

	*hdl = saveisf;
	version = gamecore.sys.version;
	type = gamecore.sys.type;
	saves = savefile_getsaves();
	isf->gamesaves = saves;
	if (type & GAME_SAVESYS) {
		isf->files = saves + 1;
		hdl->readexaflag = savenone_exa;
		hdl->readexaval = savenone_exa;
		hdl->readkid = savenone_exa;
		hdl->writeexaflag = savenone_exa;
		hdl->writeexaval = savenone_exa;
		hdl->writekid = savenone_exa;
	}
	else {
		if (type & GAME_NOKID) {
			isf->files = saves + 2;
			hdl->readkid = savenone_exa;
			hdl->writekid = savenone_exa;
		}
		else {
			isf->files = saves + 3;
		}
		hdl->readsysflag = savenone_sys;
		hdl->writesysflag = savenone_sys;
	}
	if (version == EXEVER_NURSE) {
		isf->files++;
	}
	isf->commsize = (version >= EXEVER_NURSE)?100:80;
	if ((type & GAME_SAVESYS) || (version == EXEVER_VECHO)) {
		isf->savever = 0;
		isf->headsize = isf->files * 4;
	}
	else if ((version >= EXEVER_AKIBA) || (version == EXEVER_CRESD)) {
		isf->savever = 2;
		isf->headsize = 0x0d10 + (isf->files * 0x40);
		isf->commsize = 80;
	}
	else if (version < EXE_VER2) {
		isf->savever = 1;
		isf->headsize = isf->files * 4;
	}
	else {
		isf->savever = 2;
		isf->headsize = 0x0c10 + (isf->files * 0x40);
		isf->commsize = 80;
	}
}

SAVEHDL saveisf_open(BOOL create) {

	_SAVEHDL	hdlbase;
	_SAVEISF	isfbase;
	SAVEHDL		hdl;
	SAVEISF		isf;
	int			size;

	setinf(&hdlbase, &isfbase);
	isfbase.fh = saveopen(&isfbase, create);
	if (isfbase.fh == FILEH_INVALID) {
		TRACEOUT(("savefile not found."));
		goto siop_err1;
	}
	size = sizeof(_SAVEHDL) + sizeof(_SAVEISF) + isfbase.headsize;
	hdl = (SAVEHDL)_MALLOC(size, "saveisf handle");
	if (hdl == NULL) {
		goto siop_err2;
	}
	isf = (SAVEISF)(hdl + 1);
	*hdl = hdlbase;
	*isf = isfbase;
	isf->fsize = file_seek(isfbase.fh, 0, FSEEK_END);
	if ((file_seek(isfbase.fh, 0, FSEEK_SET) != 0) ||
		(file_read(isfbase.fh, isf + 1, isf->headsize) != isf->headsize)) {
		goto siop_err3;
	}
	return(hdl);

siop_err3:
	_MFREE(hdl);

siop_err2:
	file_close(isfbase.fh);

siop_err1:
	return(NULL);
}

