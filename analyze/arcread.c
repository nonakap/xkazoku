#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"


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


static BOOL fnamecmp(const char *string1, const char *string2) {

	int		i;
	BYTE	c1, c2;

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


// ----

static ARCFH arcfhnew(FILEH fh, int files) {

	ARCFH	r;
	UINT	size;

	size = files * sizeof(_AFTBL);
	size += sizeof(_ARCFH);
	r = (ARCFH)_MALLOC(size, path);
	if (r) {
		ZeroMemory(r, size);
		r->fh = (void *)fh;
		r->files = files;
	}
	return(r);
}

static ARCFH afhopen1(const char *path) {

	ARCFH	r;
	FILEH	fh;
	SM2MPX	sm2mpx;
	FTBL1	tbl1;
	int		files;
	long	ptr;
	AFTBL	ftbl;

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

	r = arcfhnew(fh, files);
	if (r == NULL) {
		goto ar1_err1;
	}
	ftbl = (AFTBL)(r + 1);
	do {
		if (file_read(fh, &tbl1, sizeof(tbl1)) != sizeof(tbl1)) {
			goto ar1_err2;
		}
		CopyMemory(ftbl->name, tbl1.name, ARCFILENAME_LEN);
		ftbl->ptr = LOADINTELDWORD(tbl1.ptr);
		ftbl->size = LOADINTELDWORD(tbl1.size);
		ftbl++;
	} while(--files);
	return(r);

ar1_err2:
	_MFREE(r);

ar1_err1:
	file_close(fh);

ar1_err0:
	return(NULL);
}

static ARCFH afhopen2(const char *path) {

	ARCFH	r;
	FILEH	fh;
	BYTE	buf[4];
	int		files;
	UINT	tmp;
	FTBL2	tbl2;
	AFTBL	ftbl;

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

	r = arcfhnew(fh, files);
	if (r == NULL) {
		goto ar2_err1;
	}
	ftbl = (AFTBL)(r + 1);

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
	return(r);

ar2_err2:
	_MFREE(r);

ar2_err1:
	file_close(fh);

ar2_err0:
	return(NULL);
}


// ----

ARCFH arcread_open(const char *path) {

	ARCFH	r;

	r = afhopen1(path);
	if (r == NULL) {
		r = afhopen2(path);
	}
	return(r);
}

void arcread_close(ARCFH afh) {

	if (afh) {
		file_close((FILEH)afh->fh);
		_MFREE(afh);
	}
}

ARCHDL arcread_get(ARCFH afh, const char *filename) {

	int		i;
	AFTBL	ftbl;
	int		hit;
	UINT	size;
	ARCHDL	r;

	if ((afh == NULL) || (filename == NULL)) {
		goto arget_err1;
	}
	ftbl = (AFTBL)(afh + 1);
	hit = -1;
	for (i=0; i<afh->files; i++, ftbl++) {
		if (fnamecmp(ftbl->name, filename) == SUCCESS) {
			hit = i;
		}
	}
	if (hit < 0) {
		goto arget_err1;
	}

	ftbl = (AFTBL)(afh + 1);
	ftbl += hit;
	if (ftbl->size <= 0) {
		goto arget_err1;
	}
	if (file_seek((FILEH)afh->fh, ftbl->ptr, FSEEK_SET) != ftbl->ptr) {
		goto arget_err1;
	}

	size = sizeof(_ARCHDL) + ftbl->size;
	r = (ARCHDL)_MALLOC(size, filename);
	if (r == NULL) {
		goto arget_err1;
	}
	r->ptr = (BYTE *)(r + 1);
	r->size = ftbl->size;
	milstr_ncpy(r->name, ftbl->name, sizeof(r->name));
	if (file_read((FILEH)afh->fh, r + 1, ftbl->size) != ftbl->size) {
		goto arget_err2;
	}
	return(r);

arget_err2:
	_MFREE(r);

arget_err1:
	return(NULL);
}

BOOL arcread_getname(ARCFH afh, int num, char *buffer, int size) {

	AFTBL	ftbl;
	char	name[ARCFILENAME_LEN + 1];

	if ((afh == NULL) ||
		(num < 0) || (num >= afh->files)) {
		goto agn_err;
	}
	ftbl = (AFTBL)(afh + 1);
	CopyMemory(name, ftbl[num].name, ARCFILENAME_LEN);
	name[ARCFILENAME_LEN] = '\0';
	milstr_ncpy(buffer, name, size);
	return(SUCCESS);

agn_err:
	return(FAILURE);
}

AFTBL arcread_getinf(ARCFH afh, int num) {

	AFTBL	r;

	if ((afh == NULL) ||
		(num < 0) || (num >= afh->files)) {
		goto agi_err;
	}
	r = (AFTBL)(afh + 1);
	r += num;
	return(r);

agi_err:
	return(NULL);
}

