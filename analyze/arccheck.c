#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"
#include	"arccheck.h"


static void getinfo(const char *name, const BYTE *ptr, int size,
												char *info, int infosize) {

	UINT32	r;
	BYTE	tmp[4];
	char	*ext;
	char	work[32];

	ext = getExtName((char *)name);
	if ((!milstr_cmp(ext, "ISF")) && (size >= 8)) {
		sprintf(work, " ISF rev:%02x%02x", ptr[4], ptr[5]);
		milstr_ncat(info, work, infosize);
		return;
	}

	if ((size >= 8) && (!memcmp(ptr, "GANM", 4))) {
		milstr_ncat(info, " GANM rev:", infosize);
		CopyMemory(work, ptr + 4, 4);
		work[4] = '\0';
		milstr_ncat(info, work, infosize);
		return;
	}
	if ((size >= 8) && (!memcmp(ptr, "GAD ", 4))) {
		milstr_ncat(info, " GAD", infosize);
		r = LOADINTELDWORD(ptr + 4);
		ptr += r;
		size -= r;
	}
	if ((size >= 8) && (!memcmp(ptr, "GGA00000", 8))) {
		milstr_ncat(info, " GG0", infosize);
		return;
	}
	if (size >= 4) {
		if (!memcmp(ptr, "PGG0", 4)) {
			milstr_ncat(info, " GG0(PPC)", infosize);
			return;
		}
		if (!memcmp(ptr, "PLZ0", 4)) {
			milstr_ncat(info, " LZ(PPC)", infosize);
			return;
		}

		if (!memcmp(ptr, "MThd", 4)) {
			milstr_ncat(info, " MThd", infosize);
			return;
		}

		if (!memcmp(ptr, "RIFF", 4)) {
			milstr_ncat(info, " RIFF", infosize);
			return;
		}
		tmp[0] = ptr[0] ^ 0xff;
		tmp[1] = ptr[1] ^ 0xff;
		tmp[2] = ptr[2] ^ 0xff;
		tmp[3] = ptr[3] ^ 0xff;
		if (!memcmp(tmp, "256G", 4)) {
			milstr_ncat(info, " 256G", infosize);
			return;
		}
		if (!memcmp(tmp, "HIGH", 4)) {
			milstr_ncat(info, " HIGH", infosize);
			return;
		}
		if (!memcmp(tmp, "FULL", 4)) {
			milstr_ncat(info, " FULL", infosize);
			return;
		}
	}
}

void arccheck_list(const char *path, int mode) {

	ARCFH	afh;
	ARCHDL	ah;
	int		i;
	AFTBL	ftbl;
	char	info[64];
	char	name[ARCFILENAME_LEN + 1];

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto acl_err;
	}
	for (i=0; i<afh->files; i++) {
		ftbl = arcread_getinf(afh, i);
		if (!ftbl) {
			continue;
		}
		CopyMemory(name, ftbl->name, ARCFILENAME_LEN);
		name[ARCFILENAME_LEN] = '\0';
		info[0] = '\0';
		if (mode) {
			ah = arcread_get(afh, name);
			if (ah) {
				getinfo(name, ah->ptr, ah->size, info, sizeof(info));
				_MFREE(ah);
			}
		}
		log_disp("%-12s  %8d  [%08lx]%s", name, ftbl->size, ftbl->ptr, info);
	}
	arcread_close(afh);

acl_err:
	return;
}

void arccheck_get(const char *path, const char *filename) {

	ARCFH	afh;
	ARCHDL	ah;
	FILEH	fh;

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto acg_err1;
	}
	ah = arcread_get(afh, filename);
	arcread_close(afh);
	if (ah == NULL) {
		log_disp("%s: couldn't open.", filename);
		goto acg_err2;
	}
	fh = file_create(ah->name);
	if (fh == FILEH_INVALID) {
		log_disp("%s: couldn't create.", ah->name);
		goto acg_err3;
	}
	if (file_write(fh, ah->ptr, ah->size) != ah->size) {
		log_disp("%s: write error.", ah->name);
	}

acg_err3:
	file_close(fh);

acg_err2:
	_MFREE(ah);

acg_err1:
	return;
}

