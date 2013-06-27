#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"
#include	"arccheck.h"


void arccheck_list(const char *path) {

	ARCFH	afh;
	int		i;
	AFTBL	ftbl;
	char	name[ARCFILENAME_LEN + 1];

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto acl_err;
	}
	for (i=0; i<afh->files; i++) {
		ftbl = arcread_getinf(afh, i);
		if (ftbl) {
			CopyMemory(name, ftbl->name, ARCFILENAME_LEN);
			name[ARCFILENAME_LEN] = '\0';
			log_disp("%-12s  %8d  [%08lx]", name, ftbl->size, ftbl->ptr);
		}
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
		goto acg_err1;
	}
	fh = file_create(ah->name);
	if (fh == FILEH_INVALID) {
		log_disp("%s: couldn't create.", ah->name);
		goto acg_err2;
	}
	if (file_write(fh, ah->ptr, ah->size) != ah->size) {
		log_disp("%s: write error.", ah->name);
	}
	file_close(fh);

acg_err2:
	_MFREE(fh);

acg_err1:
	return;
}

