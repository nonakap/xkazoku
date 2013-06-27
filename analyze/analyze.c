#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"
#include	"arccheck.h"
#include	"isfcheck.h"
#include	"gamedef.tbl"


typedef struct {
	char	company[64];
	char	title[64];
	char	key[64];
} _SUF, *SUF;

static BOOL sufload(void *arg, const char *para,
									const char *key, const char *data) {

	SUF		suf;

	suf = (SUF)arg;
	if (!milstr_cmp(para, "StartUpInfo")) {
		if (!milstr_cmp(key, "COMPANY")) {
			milstr_ncpy(suf->company, data, sizeof(suf->company));
		}
		else if (!milstr_cmp(key, "TITLE")) {
			milstr_ncpy(suf->title, data, sizeof(suf->title));
		}
		else if (!milstr_cmp(key, "KEY")) {
			milstr_ncpy(suf->key, data, sizeof(suf->key));
		}
	}
	return(FALSE);
}

static const GAMEDEF *readsuf(const char *path) {

	_SUF		suf;
	UINT		cnt;
const GAMEDEF	*r;

	if (path == NULL) {
		log_disp("%s: couldn't open.", path);
		goto rs_err;
	}

	profile_enum(path, &suf, sufload);
	log_disp("COMPANY: %s", suf.company);
	log_disp("TITLE: %s", suf.title);
	log_disp("KEY: %s", suf.key);

	r = gamedef;
	cnt = sizeof(gamedef) / sizeof(GAMEDEF);
	do {
		if (((r->company == NULL) ||
				(!milstr_cmp(r->company, suf.company))) &&
			((r->key == NULL) || (!milstr_cmp(r->key, suf.key)))) {
			return(r);
		}
		r++;
	} while(--cnt);

rs_err:
	return(NULL);
}


// ----

static const char usage[] =									\
			"Usage: analyze [option] archive [filename]\n";

int main(int argc, char **argv) {

	int			i;
	char		*ptr;
	char		*arc;
	char		*file;
	char		isf[MAX_PATH];
const GAMEDEF	*def;
	int			warnlevel = 0;

	arc = NULL;
	file = NULL;
	for (i=1; i<argc; i++) {
		ptr = argv[i];
		if ((ptr[0] == '/') || (ptr[0] == '-')) {
			if ((ptr[1] == 'w') || (ptr[1] == 'W')) {
				warnlevel = ptr[2] - 0x30;
			}
		}
		else if (arc == NULL) {
			arc = ptr;
		}
		else if (file == NULL) {
			file = ptr;
		}
	}

	if (arc == NULL) {
		printf(usage);
		return(1);
	}

	log_open();
	ptr = getExtName(arc);
	if (!milstr_cmp(ptr, "SUF")) {
		def = readsuf(arc);
		if (def != NULL) {
			milstr_ncpy(isf, arc, sizeof(isf));
			cutFileName(isf);
			milstr_ncat(isf, "ISF", sizeof(isf));
			if (file == NULL) {
//				isfcheck_cmdlist(isf);
				isfcheck_cmdcheck(isf, def->exever, def->gametype, warnlevel);
			}
			else {
				isfcheck_dump(isf, file);
			}
		}
	}
	else {
		if (file == NULL) {
			arccheck_list(arc);
		}
		else {
			arccheck_get(arc, file);
		}
	}
	log_close();

	return(0);
}

