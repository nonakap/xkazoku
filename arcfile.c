#include	"compiler.h"
#include	"dosio.h"
#include	"arcfile.h"


#define	DATACACHES		4
#define	DATACACHESIZE	8192
#define	DATAINCACHE		0


#if defined(__GNUC__)

typedef struct {
	BYTE	sig[8];
	BYTE	files[4];
	BYTE	headsize[4];
	BYTE	fmt[12];
	BYTE	ptr[4];
} __attribute__ ((packed)) SM2MPX;

typedef struct {
	char	name[ARCFILENAME_LEN];
	BYTE	ptr[4];
	BYTE	size[4];
} __attribute__ ((packed)) FTBL1;

typedef struct {
	char	name[ARCFILENAME_LEN];
	BYTE	ptr[4];
} __attribute__ ((packed)) FTBL2;

#else /* !__GNUC__ */

#pragma pack(push, 1)
typedef struct {
	BYTE	sig[8];
	BYTE	files[4];
	BYTE	headsize[4];
	BYTE	fmt[12];
	BYTE	ptr[4];
} SM2MPX;

typedef struct {
	char	name[ARCFILENAME_LEN];
	BYTE	ptr[4];
	BYTE	size[4];
} FTBL1;

typedef struct {
	char	name[ARCFILENAME_LEN];
	BYTE	ptr[4];
} FTBL2;
#pragma pack(pop)

#endif /* __GNUC__ */


typedef struct {
	UINT		type;
	int			files;
	char		name[ARCFILENAME_LEN];
	int			testfiles;
	char		path[MAX_PATH];
} _ARCHIVE, *ARCHIVE;

typedef struct {
	char		name[ARCFILENAME_LEN];
	long		ptr;
	UINT		size;
} _ARCFTBL, *ARCFTBL;


static	ARCHIVE		arcptr[ARCTYPES][2];

static	int			arcs = 0;
static	ARCHIVE		arc[ARCHIVE_MAXFILES];
static	UINT		arcname_type = 0;


// ----

#if defined(DATACACHES)
typedef struct {
	UINT	type;
	char	name[ARCFILENAME_LEN];
	int		pry;
	int		used;
	void	*ptr;
	UINT	size;
} _CACHE, *CACHE;

static	_CACHE	caches[DATACACHES];


static void *getarcfile(ARCFILEH af) {

	FILEH	fh;
	void	*ret;

	ret = _MALLOC(af->size, af->name);
	if (ret == NULL) {
		goto gaf_err1;
	}
	fh = file_open_rb(af->filename);
	if (fh == FILEH_INVALID) {
		goto gaf_err2;
	}
	if ((file_seek(fh, af->base, FSEEK_SET) != (long)af->base) ||
		(file_read(fh, ret, af->size) != af->size)) {
		goto gaf_err3;
	}
	file_close(fh);
	return(ret);

gaf_err3:
	file_close(fh);

gaf_err2:
	_MFREE(ret);

gaf_err1:
	return(NULL);
}

static CACHE cache_sea(ARCFILEH af) {

	int		i;
	CACHE	ret;

	ret = caches;
	for (i=0; i<DATACACHES; i++) {
		if (ret->ptr == af->fh) {
			return(ret);
		}
		ret++;
	}
	return(NULL);
}

static CACHE cache_open(UINT type, ARCFILEH af) {

	int		i;
	CACHE	cc;
	CACHE	ret;
	CACHE	stc;
	int		pry;

	ret = NULL;
	stc = NULL;
	cc = caches;
	pry = DATACACHES + 1;
	for (i=0; i<DATACACHES; i++) {
		if (cc->pry) {
			if ((cc->type == type) &&
				(!memcmp(cc->name, af->name, ARCFILENAME_LEN))) {
				ret = cc;
				break;
			}
		}
		if (!cc->used) {
			if (pry > cc->pry) {
				pry = cc->pry;
				stc = cc;
			}
		}
		cc++;
	}
	if ((ret == NULL) && (stc != NULL)) {
		if (stc->ptr) {
			_MFREE(stc->ptr);
		}
		stc->pry = 0;
		stc->ptr = getarcfile(af);
		if (stc->ptr) {
			stc->type = type;
			CopyMemory(stc->name, af->name, ARCFILENAME_LEN);
			stc->size = af->size;
			ret = stc;
			TRACEOUT(("set cache"));
		}
	}
	if (ret) {
		cc = caches;
		for (i=0; i<DATACACHES; i++) {
			if (cc->pry > ret->pry) {
				cc->pry--;
			}
			cc++;
		}
		ret->pry = DATACACHES;
	}
	return(ret);
}

static void cache_reset(UINT type) {

	int		i;
	CACHE	cc;

	cc = caches;
	for (i=0; i<DATACACHES; i++) {
		if (cc->type == type) {
			cc->pry = 0;
		}
		cc++;
	}
}

static void cache_destroy(void) {

	int		i;
	CACHE	cc;

	cc = caches;
	for (i=0; i<DATACACHES; i++) {
		if (cc->ptr) {
			_MFREE(cc->ptr);
		}
		cc++;
	}
	ZeroMemory(caches, sizeof(caches));
}
#endif


// ----

static BOOL fnamecmp(const char *string1, const char *string2) {

	int		i;
	BYTE	c1 = 0;		// for gcc
	BYTE	c2 = 0;		// for gcc

	for (i=0; i<ARCFILENAME_LEN; i++) {
		c1 = *string1++;
		c2 = *string2++;
		if (((c1 - 'a') & 0xff) < 26) {
			c1 -= 0x20;
		}
		if (((c2 - 'a') & 0xff) < 26) {
			c2 -= 0x20;
		}
		if (c1 != c2) {
			break;
		}
		if (!c2) {
			break;
		}
	}
	if ((i < ARCFILENAME_LEN) &&
		((c2) || ((c1 != '.') && (c1 != '\0')))) {
		return(FAILURE);
	}
	return(SUCCESS);
}

static ARCHIVE arcnew(const char *path, const char *name,
												UINT type, UINT files) {

	ARCHIVE	r;
	UINT	size;

	size = files * sizeof(_ARCFTBL);
	size += sizeof(_ARCHIVE);
	r = (ARCHIVE)_MALLOC(size, path);
	if (r) {
		ZeroMemory(r, size);
		r->type = type;
		r->files = files;
		milstr_ncpy(r->name, name, sizeof(r->name));
		milstr_ncpy(r->path, path, sizeof(r->path));
	}
	return(r);
}

static ARCHIVE arcreg1(const char *path, const char *name, UINT type) {

	ARCHIVE		r;
	FILEH		fh;
	SM2MPX		sm2mpx;
	FTBL1		tbl1;
	UINT		files;
	long		ptr;
	ARCFTBL		ftbl;

	fh = file_open_rb(path);
	if (fh == FILEH_INVALID) {
		goto ar1_err0;
	}
	if (file_read(fh, &sm2mpx, sizeof(sm2mpx)) != sizeof(sm2mpx)) {
		goto ar1_err1;
	}
	if (memcmp(sm2mpx.sig, "SM2MPX10", 8)) {
		goto ar1_err1;
	}
	files = LOADINTELDWORD(sm2mpx.files);
	if (files == 0) {
		goto ar1_err1;
	}
	ptr = LOADINTELDWORD(sm2mpx.ptr);
	if (file_seek(fh, ptr, FSEEK_SET) != ptr) {
		goto ar1_err1;
	}

	r = arcnew(path, name, type, files);
	if (r == NULL) {
		goto ar1_err1;
	}
	ftbl = (ARCFTBL)(r + 1);
	do {
		if (file_read(fh, &tbl1, sizeof(tbl1)) != sizeof(tbl1)) {
			goto ar1_err2;
		}
		CopyMemory(ftbl->name, tbl1.name, ARCFILENAME_LEN);
		ftbl->ptr = LOADINTELDWORD(tbl1.ptr);
		ftbl->size = LOADINTELDWORD(tbl1.size);
		ftbl++;
	} while(--files);
	file_close(fh);
	TRACEOUT(("regist: %s %dfile(s) type:SM2MPX", r->name, r->files));
	return(r);

ar1_err2:
	_MFREE(r);

ar1_err1:
	file_close(fh);

ar1_err0:
	return(NULL);
}

static ARCHIVE arcreg2(const char *path, const char *name, UINT type) {

	ARCHIVE		r;
	FILEH		fh;
	BYTE		buf[4];
	UINT		files;
	UINT		tmp;
	FTBL2		tbl2;
	ARCFTBL		ftbl;

	fh = file_open_rb(path);
	if (fh == FILEH_INVALID) {
		goto ar2_err0;
	}
	if (file_read(fh, buf, 2) != 2) {
		goto ar2_err1;
	}
	files = LOADINTELWORD(buf);
	files /= sizeof(FTBL2);
	if (files < 2) {
		goto ar2_err1;
	}
	files--;

	r = arcnew(path, name, type, files);
	if (r == NULL) {
		goto ar2_err1;
	}
	ftbl = (ARCFTBL)(r + 1);

	if (file_read(fh, &tbl2, sizeof(tbl2)) != sizeof(tbl2)) {
		goto ar2_err2;
	}
	tmp = LOADINTELDWORD(tbl2.ptr);
	do {
		CopyMemory(ftbl->name, tbl2.name, ARCFILENAME_LEN);
		ftbl->ptr = tmp;
		if (file_read(fh, &tbl2, sizeof(tbl2)) != sizeof(tbl2)) {
			goto ar2_err2;
		}
		tmp = LOADINTELDWORD(tbl2.ptr);
		ftbl->size = tmp - ftbl->ptr;
		ftbl++;
	} while(--files);

	file_close(fh);
	TRACEOUT(("regist: %s %dfile(s) type:old", r->name, r->files));
	return(r);

ar2_err2:
	_MFREE(r);

ar2_err1:
	file_close(fh);

ar2_err0:
	return(NULL);
}

static ARCHIVE arcreg(const char *path, const char *name, UINT type) {

	ARCHIVE		r;

	if (arcs >= ARCHIVE_MAXFILES) {
		return(NULL);
	}
	r = arcreg1(path, name, type);
	if (r == NULL) {
		r = arcreg2(path, name, type);
	}
	if (r == NULL) {
		goto ar_exit;
	}
	arc[arcs++] = r;

ar_exit:
	return(r);
}

static char *arctestname(UINT type) {

	if (type == ARCTYPE_VOICE) {
		return("VTEST");
	}
	else if (type == ARCTYPE_SE) {
		return("SETEST");
	}
	return(NULL);
}

static int arctestfiles(ARCHIVE hdl, UINT type) {

	int			ret;
	int			i;
	ARCFTBL		ftbl;
const char		*test;

	ret = 0;
	test = arctestname(type);
	if (test == NULL) {
		goto atf_exit;
	}
	ftbl = (ARCFTBL)(hdl + 1);
	for (i=0; i<hdl->files; i++) {
		if (!milstr_memcmp(ftbl->name, test)) {
			ret++;
		}
		ftbl++;
	}

atf_exit:
	return(ret);
}

static ARCHIVE archive_sea(const char *dir, const char *name, UINT type) {

	ARCHIVE	r;
	int		i;
	int		j;
	char	arcname[MAX_PATH];
	char	path[MAX_PATH];

	for (i=0; i<arcs; i++) {
		r = arc[i];
		if (fnamecmp(name, r->name) == SUCCESS) {
			return(r);
		}
	}
	for (i=0; i<4; i++) {
		if (i == 0) {
			if (arcname_type != 0) {
				continue;
			}
		}
		else {
			if (!(arcname_type & (1 << (i - 1)))) {
				continue;
			}
		}
		milstr_ncpy(arcname, name, sizeof(arcname));
		if (i) {
			for (j=0; arcname[j]; j++) {
				if ((((arcname[j] ^ 0x20) - 0xa1) & 0xff) < 0x3c) {
					if (arcname[j] == '\0') {
						break;
					}
					j++;
				}
				else if (((arcname[j] - 0x41) & 0xdf) < 26) {
					if ((i == 1) || ((i == 3) && (j == 0))) {
						arcname[j] &= 0xdf;
					}
					else {
						arcname[j] |= 0x20;
					}
				}
			}
		}
		milstr_ncpy(path, dir, sizeof(path));
		plusyen(path, sizeof(path));
		milstr_ncat(path, arcname, sizeof(path));
		r = arcreg(path, arcname, type);
#ifdef SUPPORT_PPCARC
		if (r == NULL) {
			milstr_ncpy(path, dir, sizeof(path));
			plusyen(path, sizeof(path));
			milstr_ncat(path, "_", sizeof(path));
			milstr_ncat(path, arcname, sizeof(path));
			r = arcreg(path, arcname, type);
		}
#endif
		if (r) {
			r->testfiles = arctestfiles(r, type);
			return(r);
		}
	}
	return(NULL);
}


// ----

BOOL archive_create(void) {

	archive_destory();
	return(SUCCESS);
}

void archive_destory(void) {

	int		i;

	for (i=0; i<arcs; i++) {
		if (arc[i]) {
			_MFREE(arc[i]);
		}
	}
	arcs = 0;
	ZeroMemory(arcptr, sizeof(arcptr));
#if defined(DATACACHES)
	cache_destroy();
#endif
}

BOOL archive_set(const char *dir, const char *name, UINT type, UINT num) {

	ARCHIVE		r;

	if ((type >= ARCTYPES) || (num >= 2)) {
		goto arcset_err;
	}
#if defined(DATACACHES)
	cache_reset(type);
#endif
	r = archive_sea(dir, name, type);
	if (r == NULL) {
		goto arcset_err;
	}
	arcptr[type][num] = r;
	return(SUCCESS);

arcset_err:
	return(FAILURE);
}


// ---- test files

int arcfile_gettestfiles(UINT type) {

	int			ret;
	int			i;
	ARCHIVE		tbl;

	ret = 0;
	if (type >= ARCTYPES) {
		goto afgtf_exit;
	}
	for (i=0; i<2; i++) {
		tbl = arcptr[type][i];
		if (tbl) {
			ret += tbl->testfiles;
		}
	}

afgtf_exit:
	return(ret);
}

BOOL arcfile_gettestname(UINT type, int num, char *fname, int size) {

	int			i, j;
	ARCHIVE		tbl;
	ARCFTBL		ftbl;
	char		name[ARCFILENAME_LEN + 1];
const char		*test;

	test = arctestname(type);
	if (test == NULL) {
		goto afgtn_err;
	}
	for (i=0; i<2; i++) {
		tbl = arcptr[type][i];
		if (tbl == NULL) {
			continue;
		}
		ftbl = (ARCFTBL)(tbl + 1);
		for (j=0; j<tbl->files; j++) {
			if (!milstr_memcmp(ftbl->name, test)) {
				if (!num) {
					CopyMemory(name, ftbl[num].name, ARCFILENAME_LEN);
					name[ARCFILENAME_LEN] = '\0';
					milstr_ncpy(fname, name, size);
					return(SUCCESS);
				}
				num--;
			}
			ftbl++;
		}
	}

afgtn_err:
	return(FAILURE);
}


// ----

static ARCFILEH arcfile_get(UINT type, const char *fname) {

	int			i, j;
	ARCHIVE		tbl;
	ARCFTBL		ftbl;
	int			hit;
	ARCFILEH	ret;
	int			ext;

	if ((fname == NULL) || (type >= ARCTYPES)) {
		goto arcget_err;
	}

	for (i=0; i<2; i++) {
		tbl = arcptr[type][i];
		if (tbl == NULL) {
			continue;
		}
		ftbl = (ARCFTBL)(tbl + 1);
		hit = -1;
		for (j=0; j<tbl->files; j++, ftbl++) {
			// â∆ë∞åvâÊÇ¡Çƒ TITLE.GG0Ç∆ TITLE.GGDÇ™Ç†ÇÈÇÃÇÀÅc(ó‹
			if (fnamecmp(ftbl->name, fname) == SUCCESS) {
				hit = j;
			}
		}
		if (hit < 0) {
			continue;
		}

		ftbl = (ARCFTBL)(tbl + 1);
		ftbl += hit;
		if ((ftbl->ptr == 0) || (ftbl->size <= 0)) {
			break;
		}
		ret = (ARCFILEH)_MALLOC(sizeof(_ARCFILEH), "ARC");
		if (ret == NULL) {
			break;
		}
		ZeroMemory(ret, sizeof(_ARCFILEH));
		ret->filename = tbl->path;
		ret->fh = (void *)FILEH_INVALID;
		ret->base = ftbl->ptr;
		ret->size = ftbl->size;
		CopyMemory(ret->name, ftbl->name, ARCFILENAME_LEN);
		ext = strlen(ret->name);
		while(ext) {
			if (ret->name[--ext] == '.') {
				ret->ext = ret->name + ext + 1;
				break;
			}
		}
		return(ret);
	}

arcget_err:
	return(NULL);
}


// ----

ARCFILEH arcfile_open(UINT type, const char *fname) {

	ARCFILEH	ret;

	ret = arcfile_get(type, fname);
	if (ret == NULL) {
		goto afopn_err1;
	}

#if defined(DATACACHES)
	if (ret->size <= DATACACHESIZE) {
		CACHE cc;
		cc = cache_open(type, ret);
		if (cc) {
			TRACEOUT(("cache hit"));
			ret->base = DATAINCACHE;
			ret->fh = cc->ptr;
			cc->used++;
			return(ret);
		}
	}
#endif

	ret->fh = (void *)file_open_rb(ret->filename);
	if ((FILEH)ret->fh == FILEH_INVALID) {
		goto afopn_err2;
	}
	file_seek((FILEH)ret->fh, ret->base, FSEEK_SET);
	return(ret);

afopn_err2:
	arcfile_close(ret);

afopn_err1:
	return(NULL);
}

void arcfile_close(ARCFILEH hdl) {

	if (hdl == NULL) {
		goto afcls_exit;
	}
#if defined(DATACACHES)
	if (hdl->base == DATAINCACHE) {
		CACHE cc;
		cc = cache_sea(hdl);
		if (cc) {
			cc->used--;
		}
	}
	else
#endif
	{
		if ((FILEH)hdl->fh != FILEH_INVALID) {
			file_close((FILEH)hdl->fh);
		}
	}
	_MFREE(hdl);

afcls_exit:
	return;
}

UINT arcfile_read(ARCFILEH hdl, void *buf, UINT size) {

	UINT	ret;

	ret = 0;
	if (hdl == NULL) {
		goto afrd_exit;
	}
	ret = min(size, hdl->size - hdl->pos);
	if (ret == 0) {
		goto afrd_exit;
	}

#if defined(DATACACHES)
	if (hdl->base == DATAINCACHE) {
		CopyMemory(buf, ((BYTE *)hdl->fh) + hdl->pos, ret);
	}
	else
#endif
	{
		ret = file_read((FILEH)hdl->fh, buf, ret);
	}
	hdl->pos += ret;

afrd_exit:
	return(ret);
}

long arcfile_seek(ARCFILEH hdl, long pos, int method) {

	long	ret;

	ret = 0;
	if (hdl == NULL) {
		goto afsk_exit;
	}

	switch(method) {
		case 1:
			ret = hdl->pos;
			break;
		case 2:
			ret = hdl->size;
			break;
	}
	ret += pos;
	if (ret < 0) {
		ret = 0;
	}
	else if (ret > (int)hdl->size) {
		ret = hdl->size;
	}
	hdl->pos = ret;
#if defined(DATACACHES)
	if (hdl->base != DATAINCACHE)
#endif
	{
		file_seek((FILEH)hdl->fh, hdl->base + ret, FSEEK_SET);
	}

afsk_exit:
	return(ret);
}


// ----

void archive_namingconv(UINT type) {

	arcname_type = type;
}

BOOL archive_throwall(const char *dir) {

	BOOL	ret;

	ret = FAILURE;

	// â∆ë∞åvâÊ, êØÇ’ÇÁ(D.O.)
	// Ç©Ç∑Ç›óVãY, Lien(Purple)
	ret &= archive_set(dir, "ISF", ARCTYPE_SCRIPT, 0);
	ret &= archive_set(dir, "GGD", ARCTYPE_GRAPHICS, 0);
	ret &= archive_set(dir, "WMSC", ARCTYPE_SOUND, 0);
	ret &= archive_set(dir, "MIDI", ARCTYPE_MIDI, 0);		// Ç©Ç∑Ç›óVãY
	ret &= archive_set(dir, "SE", ARCTYPE_SE, 0);
	ret &= archive_set(dir, "VOICE", ARCTYPE_VOICE, 0);
	ret &= archive_set(dir, "DATA", ARCTYPE_DATA, 0);

	// Ç∆ÇÁÇ‘Å[
	ret &= archive_set(dir, "TRSNR", ARCTYPE_SCRIPT, 0);
	ret &= archive_set(dir, "TRGRP", ARCTYPE_GRAPHICS, 0);
	ret &= archive_set(dir, "TRSE", ARCTYPE_SE, 0);
	ret &= archive_set(dir, "TRMSC", ARCTYPE_SOUND, 0);

	// WILL
	ret &= archive_set(dir, "RIO", ARCTYPE_SCRIPT, 0);
	ret &= archive_set(dir, "CHIP", ARCTYPE_GRAPHICS, 0);
	ret &= archive_set(dir, "WAV", ARCTYPE_SOUND, 0);

	// â¡ìﬁ(D.O.)
	ret &= archive_set(dir, "DRSSNR", ARCTYPE_SCRIPT, 0);
	ret &= archive_set(dir, "DRSGRP", ARCTYPE_GRAPHICS, 0);
	ret &= archive_set(dir, "DRSSEF", ARCTYPE_SE, 0);
	ret &= archive_set(dir, "DRSDAT", ARCTYPE_DATA, 0);

	return(ret);
}

