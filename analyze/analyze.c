#include	"compiler.h"
#include	"dosio.h"
#include	"analyze.h"
#include	"arccheck.h"
#include	"isfcheck.h"
#include	"savcheck.h"


// ----

enum {
	MODE_LIST	= 0x01,
	MODE_CHECK	= 0x02,
	MODE_DUMP	= 0x04,
	MODE_SAVE	= 0x08
};

static const char usage[] =									\
			"Usage: analyze [option] archive [filename]\n";

static const char *isfname[] = {"DRSSNR", "TRSNR", "ISF"};




// ---- set default dir

static void setcd(const char *arc) {

#if defined(WIN32)

	char	module[MAX_PATH];

#if !defined(UNICODE)
	GetModuleFileName(NULL, module, sizeof(module));
#else
	TCHAR	module2[MAX_PATH];
	GetModuleFileName(NULL, module2, sizeof(module2)/sizeof(TCHAR));
	WideCharToMultiByte(CP_ACP, 0, module2, -1,
										module, sizeof(module), NULL, NULL);
#endif
	file_setcd(module);
	(void)arc;

#elif defined(MACOS)

#ifdef AKIRAGCC
	file_setcd("::::");
#endif
	(void)arc;

#elif defined(X11)

	file_setcd(arc);

#else

	(void)arc;

#endif
}


// ----

static BOOL getisfname(const char *suf, const char *key,
											char *path, int size) {

	int		i;
	BOOL	subdir;

	milstr_ncpy(path, suf, size);
	subdir = FALSE;
	while(1) {
		for (i=0; i<(int)(sizeof(isfname)/sizeof(char *)); i++) {
			cutFileName(path);
			milstr_ncat(path, isfname[i], size);
			if (file_attr(path) != -1) {
				return(SUCCESS);
			}
		}
		if ((subdir) || (!key[0])) {
			break;
		}
		subdir = TRUE;
		cutFileName(path);
		milstr_ncat(path, key, size);
		plusyen(path, size);
	}
	return(FAILURE);
}


int main(int argc, char **argv) {

	int			i;
	char		*ptr;
	char		*arc;
	char		*file;
	char		isf[MAX_PATH];
	int			warnlevel = 0;
	int			mode = 0;
	char		c;
	SUF_T		suf;
	SYS_T		sys;

	arc = NULL;
	file = NULL;
	for (i=1; i<argc; i++) {
		ptr = argv[i];
#if defined(_WIN32)
		if ((ptr[0] == '/') || (ptr[0] == '-'))
#else
		if (ptr[0] == '-')
#endif
		{
			ptr++;
			c = *ptr++;
			if ((c == 'w') || (c == 'W')) {
				warnlevel = ptr[0] - 0x30;
			}
			else {
				while(1) {
					if (c == '\0') {
						break;
					}
					if ((c == 'l') || (c == 'L')) {
						mode |= MODE_LIST;
					}
					else if ((c == 'c') || (c == 'C')) {
						mode |= MODE_CHECK;
					}
					else if ((c == 'd') || (c == 'D')) {
						mode |= MODE_DUMP;
					}
					else if ((c == 's') || (c == 'S')) {
						mode |= MODE_SAVE;
					}
					c = *ptr++;
				}
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

	setcd(arc);

	gamedef_create();
	log_open();
	ptr = getExtName(arc);
	if (!milstr_cmp(ptr, "SUF")) {
		gamedef_loadsuf(arc, &suf, &sys);
		log_disp("COMPANY: %s", suf.company);
		log_disp("TITLE: %s", suf.title);
		log_disp("KEY: %s", suf.key);
		if (getisfname(arc, suf.key, isf, sizeof(isf)) == SUCCESS) {
			if (!mode) {
				mode = MODE_CHECK;
			}
			if (mode & MODE_LIST) {
				isfcheck_cmdlist(isf, file);
			}
			if (mode & MODE_CHECK) {
				isfcheck_cmdcheck(isf, file,
										sys.version, sys.type, warnlevel);
			}
			if (mode & MODE_DUMP) {
				isfcheck_dump(isf, file);
			}
			if (mode & MODE_SAVE) {
				savcheck(isf, arc, suf.key, sys.defbits, sys.defflags);
			}
		}
	}
	else {
		if (file == NULL) {
			arccheck_list(arc, mode & MODE_CHECK);
		}
		else {
			arccheck_get(arc, file);
		}
	}
	log_close();
	gamedef_destroy();

	return(0);
}

