#include	"compiler.h"
#include	"dosio.h"
#include	"textfile.h"


static int prof_spacedel(char *string, char **sep) {

	BYTE	c;
	char	quot;
	int		ret;
	int		spos;
	int		dpos;
	int		flag;
	char	*sepa;

	quot = 0;
	ret = 0;
	spos = 0;
	dpos = 0;
	flag = 0;
	sepa = NULL;

	do {
		c = string[spos++];
		if (c == '\0') {
			break;
		}
		if (c == '\"') {
			if (string[spos] != '\"') {
				quot ^= 1;
				continue;
			}
			spos++;
			flag &= ~1;
		}
		if (!quot) {
			if (c <= ' ') {
				continue;
			}
			else if (c == ';') {
				break;
			}
			else if ((c == '=') && (!(ret & 4))) {
				ret = 2;
				sepa = string + dpos;
			}
			else if ((c == '[') && (!dpos)) {
				flag |= 2;
			}
			else if (c == ']') {
				flag |= 1;
			}
			else {
				flag &= ~1;
			}
		}
		string[dpos++] = c;
	} while(1);

	if (flag == 3) {
		ret = 1;
		dpos--;
		sepa = string + 1;
	}
	string[dpos] = '\0';
	if (sep) {
		*sep = sepa;
	}
	return(ret);
}


static BOOL prof_sea(TEXTFILEH tfh, const char *appname, const char *key,
													char *work, int size) {

	BOOL	apphit;
	char	*p;
	char	buf[0x200];
	int		r;

	apphit = FALSE;
	while(!textfile_read(tfh, buf, sizeof(buf))) {
		r = prof_spacedel(buf, &p);
		if (r == 1) {
			apphit = !milstr_cmp(p, appname);
		}
		else if ((r == 2) && (apphit)) {
			*p++ = '\0';
			if (!milstr_cmp(buf, key)) {
				milstr_ncpy(work, p, size);
				return(SUCCESS);
			}
		}
	}
	return(FAILURE);
}


static BOOL prof_get(const char *fname, const char *appname, const char *key,
													char *work, int size) {

	BOOL		ret;
	TEXTFILEH	tfh;

	ret = FALSE;
	tfh = textfile_open(fname, 0x2000);
	if (tfh != NULL) {
		if (!prof_sea(tfh, appname, key, work, size)) {
			ret = TRUE;
		}
		textfile_close(tfh);
	}
	return(ret);
}


// ----

BOOL profile_getstr(const char *appname, const char *key,
										const char *def, char *out,
										int size, const char *fname) {

	BOOL	ret;
	char	work[512];

	ret = prof_get(fname, appname, key, work, sizeof(work));
	if (out) {
		if (ret) {
			def = work;
		}
		else if (def == NULL) {
			def = (const char *)"";
		}
		milstr_ncpy(out, def, size);
	}
	return(ret);
}


BOOL profile_enum(const char *filename, void *arg,
							BOOL (*proc)(void *arg, const char *para,
									const char *key, const char *data)) {
	TEXTFILEH	fh;
	BOOL		r;
	char		buf[0x200];
	char		para[0x100];
	char		*p;
	int			ret;

	r = FALSE;
	if (proc == NULL) {
		goto gden_err0;
	}
	fh = textfile_open(filename, 0x800);
	if (fh == NULL) {
		goto gden_err0;
	}
	para[0] = '\0';
	while(1) {
		if (textfile_read(fh, buf, sizeof(buf)) != SUCCESS) {
			break;
		}
		ret = prof_spacedel(buf, &p);
		if (ret == 1) {
			milstr_ncpy(para, p, sizeof(para));
		}
		else if (ret == 2) {
			*p++ = '\0';
			r = proc(arg, para, buf, p);
			if (r) {
				break;
			}
		}
	}
	textfile_close(fh);

gden_err0:
	return(r);
}

