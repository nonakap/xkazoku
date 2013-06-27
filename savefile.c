#include	"compiler.h"
#include	"gamecore.h"
#include	"dosio.h"
#include	"savefile.h"
#include	"timemng.h"
#include	"bmpdata.h"


static void savefile_getname(char *path, int leng) {

	milstr_ncpy(path, gamecore.suf.scriptpath, leng);

	switch(gamecore.sys.type & GAME_SAVEMASK) {
		case GAME_SAVEMYU:
			milstr_ncat(path, "GAME.SAV", leng);
			break;

		default:
			milstr_ncat(path, gamecore.suf.key, leng);
			milstr_ncat(path, ".SAV", leng);
			break;
	}
}

static BOOL savefile_zerofill(FILEH fh, UINT size) {

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


static UINT savefile_getsize(SAVEHDL hdl, int num) {

	UINT	base;
	UINT	next;
	int		i;

	if ((num < 0) || (num >= hdl->files)) {
		return(0);
	}
	base = LOADINTELDWORD(hdl->header[num]);
	if (base == 0) {
		return(0);
	}
	next = hdl->fsize;
	for (i=0; i<hdl->files; i++) {
		UINT pos;
		pos = LOADINTELDWORD(hdl->header[i]);
		if ((pos > base) && (pos < next)) {
			next = pos;
		}
	}
	return(next - base);
}


static BOOL savefile_writeseek(SAVEHDL hdl, int num, UINT newsize) {

	int		i;
	UINT	basepos;
	UINT	orgsize;
	UINT	extsize;
	UINT	size;
	char	work[1024];
	UINT	pos;

	if ((num < 0) || (num >= hdl->files)) {
		return(FAILURE);
	}
	orgsize = savefile_getsize(hdl, num);

	if (orgsize < newsize) {

		// 書込み番地を指定
		basepos = LOADINTELDWORD(hdl->header[num]);
		switch(gamecore.sys.type & GAME_SAVEMASK) {
			case GAME_SAVEMYU:
				if (basepos == 0) {
					basepos = 10*4 + (num * newsize);
				}
				else if (num != hdl->sysflag) {
					return(FAILURE);
				}
				break;

			default:
				if (basepos == 0) {
					for (i=num; i--;) {
						basepos = LOADINTELDWORD(hdl->header[i]);
						if (basepos) {
							basepos += savefile_getsize(hdl, i);
							break;
						}
					}
					if (basepos == 0) {
						basepos = hdl->files * 4;
					}
				}

				// 先に増幅サイズ分書込み
				extsize = newsize - orgsize;
				if (file_seek((FILEH)hdl->fh, hdl->fsize, FSEEK_SET)
														!= (long)hdl->fsize) {
					return(FAILURE);
				}
				if (savefile_zerofill((FILEH)hdl->fh, extsize) != SUCCESS) {
					return(FAILURE);
				}

				// で移動。 エラーだとファイル破壊なのでチェキ無意味
				pos = hdl->fsize;
				while(pos > basepos) {
					size = min(pos - basepos, sizeof(work));
					pos -= size;
					file_seek((FILEH)hdl->fh, pos, FSEEK_SET);
					file_read((FILEH)hdl->fh, work, size);
					file_seek((FILEH)hdl->fh, pos + extsize, FSEEK_SET);
					file_write((FILEH)hdl->fh, work, size);
				}

				hdl->fsize += extsize;

				for (i=0; i<hdl->files; i++) {
					pos = LOADINTELDWORD(hdl->header[i]);
					if ((i != num) && (pos) && (pos >= basepos)) {
						pos += extsize;
						STOREINTELDWORD(hdl->header[i], pos);
					}
				}
				break;
		}

		STOREINTELDWORD(hdl->header[num], basepos);

		// TABLE更新　エラーだとファイル破壊なのでチェキ無意味
		file_seek((FILEH)hdl->fh, 0, FSEEK_SET);
		file_write((FILEH)hdl->fh, hdl->header, hdl->files * 4);
	}

	pos = LOADINTELDWORD(hdl->header[num]);
	if (file_seek((FILEH)hdl->fh, pos, FSEEK_SET) != (long)pos) {
		return(FAILURE);
	}
	return(SUCCESS);
}


// ----

SAVEHDL savefile_open(BOOL create) {

	char	path[MAX_PATH];
	FILEH	fh;
	SAVEHDL	ret;
	UINT	rsize;
	BOOL	r = SUCCESS;
	BYTE	purpleflagpos[4];

	savefile_getname(path, sizeof(path));
	TRACEOUT(("savefile: %s", path));

	do {
		fh = file_open(path);
		if (fh != FILEH_INVALID) {
			break;
		}
		if (!create) {
			break;
		}

		TRACEOUT(("create savefile"));
		fh = file_create(path);
		if (fh == FILEH_INVALID) {
			break;
		}
		switch(gamecore.sys.type & (GAME_SAVEMASK | GAME_SAVESYS)) {
			case GAME_SAVEMYU:
				STOREINTELDWORD(purpleflagpos, 0x80000);
				r = savefile_zerofill(fh, 0x80000);
				if (r == SUCCESS) {
					if ((file_seek(fh, 9*4, FSEEK_SET) != 9*4) ||
						(file_write(fh, purpleflagpos, 4) != 4)) {
						r = FAILURE;
					}
				}
				break;

			case GAME_SAVEMAX27:
				r = savefile_zerofill(fh, 30*4);
				break;

			case GAME_SAVEMAX30:
			default:
				r = savefile_zerofill(fh, 33*4);
				break;

			case GAME_SAVEMAX30 | GAME_SAVESYS:
				r = savefile_zerofill(fh, 31*4);
				break;

			case GAME_SAVEMAX50:
				r = savefile_zerofill(fh, 53*4);
				break;
		}
		if (r != SUCCESS) {
			file_close(fh);
			fh = FILEH_INVALID;
			file_delete(path);
		}
	} while(0);

	if (fh == FILEH_INVALID) {
		TRACEOUT(("savefile not found."));
		return(NULL);
	}

	ret = (SAVEHDL)_MALLOC(sizeof(SAVE_T), "save handle");
	do {
		if (ret == NULL) {
			break;
		}
		ZeroMemory(ret, sizeof(SAVE_T));
		ret->fh = (void *)fh;

		switch(gamecore.sys.type & (GAME_SAVEMASK | GAME_SAVESYS)) {
			case GAME_SAVEMYU:
				ret->files = 10;
				rsize = 10 * 4;
				ret->sysflag = 9;
				ret->gamesaves = 9;
				ret->exaflag = -1;
				ret->exavalue = -1;
				ret->kid = -1;
				break;

			case GAME_SAVEMAX27:
				ret->files = 30;
				rsize = 30 * 4;
				ret->gamesaves = 27;
				ret->sysflag = -1;
				ret->exaflag = 27;
				ret->exavalue = 28;
				ret->kid = 29;
				break;

			case GAME_SAVEMAX30:
			default:
				ret->files = 33;
				rsize = 33 * 4;
				ret->gamesaves = 30;
				ret->sysflag = -1;
				ret->exaflag = 30;
				ret->exavalue = 31;
				ret->kid = 32;
				break;

			case GAME_SAVEMAX30 | GAME_SAVESYS:
				ret->files = 31;
				rsize = 31 * 4;
				ret->gamesaves = 30;
				ret->sysflag = 30;
				ret->exaflag = -1;
				ret->exavalue = -1;
				ret->kid = -1;
				break;

			case GAME_SAVEMAX50:
				ret->files = 53;
				rsize = 53 * 4;
				ret->gamesaves = 50;
				ret->sysflag = -1;
				ret->exaflag = 50;
				ret->exavalue = 51;
				ret->kid = 52;
				break;
		}
		ret->fsize = file_seek(fh, 0, FSEEK_END);
		if (file_seek(fh, 0, FSEEK_SET) != 0) {
			break;
		}
		if (file_read(fh, ret->header, rsize) != rsize) {
			break;
		}
		return(ret);
	} while(0);

	if (ret) {
		_MFREE(ret);
	}
	file_close(fh);
	return(NULL);
}


void savefile_close(SAVEHDL hdl) {

	if (hdl) {
		file_close((FILEH)hdl->fh);
		_MFREE(hdl);
	}
}


static SAVEDATA savefile_read(SAVEHDL hdl, int num) {

	UINT		size;
	SAVEDATA	ret = NULL;
	long		pos;

	do {
		size = savefile_getsize(hdl, num);
		if (size == 0) {
			break;
		}
		ret = (SAVEDATA)_MALLOC(sizeof(_SAVEDATA) + size, "save data");
		if (ret == NULL) {
			break;
		}
		ret->ptr = (BYTE *)(ret + 1);
		ret->size = size;
		ZeroMemory((ret + 1), size);
		pos = LOADINTELDWORD(hdl->header[num]);
		if (file_seek((FILEH)hdl->fh, pos, FSEEK_SET) != pos) {
			break;
		}
		if (file_read((FILEH)hdl->fh, (ret + 1), size) != size) {
			break;
		}
		return(ret);
	} while(0);
	if (ret) {
		_MFREE(ret);
	}
	return(NULL);
}


// ---- read系。

static void cnvdate(SAVEDATE sd, const _SYSTIME *st) {

	sd->year = LOADINTELWORD(st->year);
	sd->month = LOADINTELWORD(st->month);
	sd->day = LOADINTELWORD(st->day);
	sd->hour = LOADINTELWORD(st->hour);
	sd->min = LOADINTELWORD(st->minute);
	sd->sec = LOADINTELWORD(st->second);
}

static int cmpdate(const SAVEDATE sd1, const SAVEDATE sd2) {

	int		r;

	r = sd2->year - sd1->year;
	if (r) {
		goto cd_exit;
	}
	r = sd2->month - sd1->month;
	if (r) {
		goto cd_exit;
	}
	r = sd2->day - sd1->day;
	if (r) {
		goto cd_exit;
	}
	r = sd2->hour - sd1->hour;
	if (r) {
		goto cd_exit;
	}
	r = sd2->min - sd1->min;
	if (r) {
		goto cd_exit;
	}
	r = sd2->sec - sd1->sec;

cd_exit:
	return(r);
}

BOOL savefile_exist(SAVEHDL hdl, int num) {

	if ((hdl != NULL) &&
		(num >= 0) && (hdl->gamesaves > num) &&
		(savefile_getsize(hdl, num) != 0)) {
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}

int savefile_getnewdate(SAVEHDL hdl) {

	int			ret;
	SAVEDATE_T	base;
	int			i;
	UINT		size;
	long		pos;
	_SYSTIME	st;
	SAVEDATE_T	cur;

	ret = -1;
	if (hdl == NULL) {
		goto sfgnd_exit;
	}
	ZeroMemory(&base, sizeof(base));
	for (i=0; i<hdl->gamesaves; i++) {
		size = savefile_getsize(hdl, i);
		if (size < sizeof(_SYSTIME)) {
			continue;
		}
		pos = LOADINTELDWORD(hdl->header[i]);
		if (file_seek((FILEH)hdl->fh, pos, FSEEK_SET) != pos) {
			continue;
		}

		if (file_read((FILEH)hdl->fh, &st, sizeof(_SYSTIME))
													!= sizeof(_SYSTIME)) {
			continue;
		}
		cnvdate(&cur, &st);
		if (cmpdate(&base, &cur) > 0) {
			ret = i;
			base = cur;
		}
	}

sfgnd_exit:
	return(ret);
}


BOOL savefile_readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height) {

	SAVEDATA	dat;
	int			remain;
	BYTE		*ptr;
	BMPINFO		*bi;
	UINT		bmpsize;

	if ((hdl == NULL) || (num < 0) || (hdl->gamesaves <= num) ||
		(inf == NULL)) {
		goto sfrt_err1;
	}

	dat = savefile_read(hdl, num);
	if (dat == NULL) {
		goto sfrt_err1;
	}

	inf->preview = NULL;

	ptr = dat->ptr;
	remain = dat->size;

	remain -= sizeof(_SYSTIME);
	if (remain < 0) {
		goto sfrt_err2;
	}
	cnvdate(&inf->date, (_SYSTIME *)ptr);
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
		bmpsize = bmpdata_getdatasize(bi);
		remain -= bmpsize;
		if (remain < 0) {
			goto sfrt_err2;
		}
		inf->preview = bmpdata_vram24cnv(width, height, bi, ptr);
		ptr += bmpsize;
	}

	inf->comment[0] = '\0';
	if (gamecore.sys.type & GAME_SAVECOM) {
		remain -= 100;
		if (remain < 0) {
			goto sfrt_err2;
		}
		CopyMemory(inf->comment, ptr, 100);
		inf->comment[100] = '\0';
		ptr += 100;
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


BOOL savefile_readgame(SAVEHDL hdl, int num) {

	FLAGS		flags;
	SAVEDATA	dat;
	BYTE		*ptr;
	int			remain;
	int			size;
	int			i;
	BMPINFO		*bi;
	UINT		bmpsize;

	flags = &gamecore.flags;

	if ((hdl == NULL) || (hdl->gamesaves <= num)) {
		goto sfrg_err1;
	}
	dat = savefile_read(hdl, num);
	if (dat == NULL) {
		goto sfrg_err1;
	}

	ptr = dat->ptr;
	remain = dat->size;

	ptr += sizeof(_SYSTIME);
	remain -= sizeof(_SYSTIME);
	if (remain < 0) {
		goto sfrg_err2;
	}

	if (gamecore.sys.type & GAME_SAVEGRPH) {
		remain -= sizeof(BMPINFO);
		if (remain < 0) {
			goto sfrg_err2;
		}
		bi = (BMPINFO *)ptr;
		ptr += sizeof(BMPINFO);
		bmpsize = bmpdata_getdatasize(bi);
		remain -= bmpsize;
		if (remain < 0) {
			goto sfrg_err2;
		}
		ptr += bmpsize;
	}

	size = 0;
	if (gamecore.sys.type & GAME_SAVECOM) {
		size += 100;
	}
	size += flags->flagsize;
	size += flags->maxval * 4;

	if (remain < size) {
		goto sfrg_err2;
	}

	if (gamecore.sys.type & GAME_SAVECOM) {
		CopyMemory(gamecore.comment, ptr, 100);
		ptr += 100;
		remain -= 100;
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


BOOL savefile_readsysflag(SAVEHDL hdl, UINT pos, UINT size) {

	SAVEDATA	dat;
	BOOL		r = FAILURE;

	if (hdl == NULL) {
		return(FAILURE);
	}

	dat = savefile_read(hdl, hdl->sysflag);
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


static BOOL savefile_readexa(SAVEHDL hdl, int num, _VARIANT *val) {

	SAVEDATA	dat;
	BOOL		r = FAILURE;
	int			size;

	if (val == NULL) {
		return(FAILURE);
	}
	dat = savefile_read(hdl, num);
	if (dat == NULL) {
		return(FAILURE);
	}
	if (dat->size > 4) {
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
			r = SUCCESS;
		}
	}
	_MFREE(dat);
	return(r);
}

BOOL savefile_readexaflag(SAVEHDL hdl, void *val) {

	if (hdl == NULL) {
		return(FAILURE);
	}
	return(savefile_readexa(hdl, hdl->exaflag, (_VARIANT *)val));
}

BOOL savefile_readexaval(SAVEHDL hdl, void *val) {

	if (hdl == NULL) {
		return(FAILURE);
	}
	return(savefile_readexa(hdl, hdl->exavalue, (_VARIANT *)val));
}


BOOL savefile_readkid(SAVEHDL hdl, void *val) {

	SAVEDATA	dat;
	BYTE		*ptr;
	int			rem;
	int			size;
	UINT		cmd;
	int			pos;

	if ((val == NULL) || (hdl == NULL)) {
		goto sfrk_err;
	}
	dat = savefile_read(hdl, hdl->kid);
	if (dat == NULL) {
		goto sfrk_err;
	}
	ptr = dat->ptr;
	rem = dat->size;
	rem -= 4;
	if (rem < 0) {
		goto sfrk_exit;
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
				goto sfrk_exit;
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
	TRACEOUT(("kid %d %d", pos, ((_VARIANT *)val)->max));

sfrk_exit:
	_MFREE(dat);
	return(SUCCESS);

sfrk_err:
	return(FAILURE);
}


// ---- write系

static const BMPDATA preview = {160, -120, 24};

BOOL savefile_writegame(SAVEHDL hdl, int num, int vnum) {

	FLAGS		flags;
	UINT		size;
	int			i;
	_SYSTIME	gdate;
	BYTE		val[4];
	BMPINFO		bi;
	UINT		bmpsize;
	BYTE		*dat;

	flags = &gamecore.flags;

	if ((hdl == NULL) || (hdl->gamesaves <= num)) {
		goto sfwg_err1;
	}

	ZeroMemory(&gdate, sizeof(gdate));
	timemng_gettime(&gdate);

	bmpdata_setinfo(&bi, &preview);
	bmpsize = bmpdata_getdatasize(&bi);

	// セーブのサイズを求める〜
	size = sizeof(gdate);

	if (gamecore.sys.type & GAME_SAVEGRPH) {
		size += sizeof(BMPINFO);
		size += bmpsize;
	}

	if (gamecore.sys.type & GAME_SAVECOM) {
		size += 100;
	}
	size += flags->flagsize;
	size += flags->maxval * 4;
	size += sizeof(SINT32) * 12;

	if (savefile_writeseek(hdl, num, size) != SUCCESS) {
		return(FAILURE);
	}

	file_write((FILEH)hdl->fh, &gdate, sizeof(gdate));

	if (gamecore.sys.type & GAME_SAVEGRPH) {
		file_write((FILEH)hdl->fh, &bi, sizeof(bi));
		if ((vnum < 0) || (vnum >= GAMECORE_MAXVRAM)) {
			vnum = gamecore.dispwin.vramnum;
		}
		dat = bmpdata_bmp24cnv(&bi, gamecore.vram[vnum]);
		if (dat) {
			file_write((FILEH)hdl->fh, dat, bmpsize);
			_MFREE(dat);
		}
		else {
			savefile_zerofill((FILEH)hdl->fh, bmpsize);
		}
	}

	if (gamecore.sys.type & GAME_SAVECOM) {
		file_write((FILEH)hdl->fh, gamecore.comment, 100);
	}

	file_write((FILEH)hdl->fh, flags->flag, flags->flagsize);
	for (i=0; i<flags->maxval; i++) {
		STOREINTELDWORD(val, flags->val[i]);
		file_write((FILEH)hdl->fh, val, 4);
	}

	return(SUCCESS);

sfwg_err1:
	return(FAILURE);
}


BOOL savefile_writesysflag(SAVEHDL hdl, UINT pos, UINT size) {

	BYTE	*dat = NULL;
	BYTE	*p;
	BYTE	bit;
	BYTE	flag;
	UINT	datsize;

	if (hdl == NULL) {
		goto sfws_err1;
	}
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

	if (savefile_writeseek(hdl, hdl->sysflag, datsize) != SUCCESS) {
		goto sfws_err2;
	}
	file_write((FILEH)hdl->fh, dat, datsize);
	_MFREE(dat);
	return(SUCCESS);

sfws_err2:
	_MFREE(dat);

sfws_err1:
	return(FAILURE);
}


static BOOL savefile_writeexa(SAVEHDL hdl, int num, _VARIANT *val) {

	BYTE	size[4];

	if (val == NULL) {
		return(FAILURE);
	}
	if (savefile_writeseek(hdl, num, 4 + val->size) != SUCCESS) {
		return(FAILURE);
	}

	STOREINTELDWORD(size, val->max);
	file_write((FILEH)hdl->fh, size, 4);
	file_write((FILEH)hdl->fh, val + 1, val->size);
	return(SUCCESS);
}

BOOL savefile_writeexaflag(SAVEHDL hdl, void *val) {

	if (hdl == NULL) {
		return(FAILURE);
	}
	return(savefile_writeexa(hdl, hdl->exaflag, (_VARIANT *)val));
}

BOOL savefile_writeexaval(SAVEHDL hdl, void *val) {

	if (hdl == NULL) {
		return(FAILURE);
	}
	return(savefile_writeexa(hdl, hdl->exavalue, (_VARIANT *)val));
}


BOOL savefile_writekid(SAVEHDL hdl, void *val) {

	int		pos;
	int		cnt;
	UINT	wsize;
	BYTE	bit;
	BYTE	cur;
	UINT	bitsize;
	BYTE	*dat;
	BYTE	*ptr;

	if ((hdl == NULL) || (val == NULL)) {
		goto sfwk_err;
	}
	cnt = ((_VARIANT *)val)->max;
	if (cnt <= 0) {
		goto sfwk_err;
	}
	wsize = 0;

	pos = cnt;
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
		goto sfwk_err;
	}
	ptr = dat;
	STOREINTELDWORD(ptr, wsize);
	ptr = dat + wsize + 4;

	pos = cnt;
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

	if (savefile_writeseek(hdl, hdl->kid, wsize + 4) != SUCCESS) {
		goto sfwk_exit;
	}
	file_write((FILEH)hdl->fh, dat, wsize + 4);

sfwk_exit:
	_MFREE(dat);
	return(SUCCESS);

sfwk_err:
	return(FAILURE);
}

