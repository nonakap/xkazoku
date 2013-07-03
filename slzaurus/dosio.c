#include	"compiler.h"
#include	<sys/stat.h>
#include	<time.h>
#include	"dosio.h"

#if 0
#include <sys/param.h>
#include <unistd.h>
#endif

static char curpath[MAX_PATH+32] = "";
static char *curfilep = curpath;


void dosio_init(void) {
}

void dosio_term(void) {
}

/* ファイル操作 */
FILEH file_open(const char *filename) {

	return(fopen(filename, "rb+"));
}

FILEH file_open_rb(const char *filename) {

	return(fopen(filename, "rb+"));
}

FILEH file_create(const char *filename) {

	return(fopen(filename, "wb+"));
}

long file_seek(FILEH handle, long pointer, int method) {

	fseek(handle, pointer, method);
	return(ftell(handle));
}

UINT file_read(FILEH handle, void *data, UINT length) {

	return((UINT)fread(data, 1, length, handle));
}

UINT file_write(FILEH handle, const void *data, UINT length) {

	return((UINT)fwrite(data, 1, length, handle));
}

short file_close(FILEH handle) {

	fclose(handle);
	return(0);
}

UINT file_getsize(FILEH handle) {

	struct stat sb;

	if (fstat(fileno(handle), &sb) == 0) {
		return(sb.st_size);
	}
	return(0);
}

short file_attr(const char *filename) {

struct stat	sb;
	short	attr;

	if (stat(filename, &sb) == 0) {
#ifndef WIN32
		if (S_ISDIR(sb.st_mode)) {
			return(FILEATTR_DIRECTORY);
		}
		attr = 0;
		if (!(sb.st_mode & S_IWUSR)) {
			attr |= FILEATTR_READONLY;
		}
#else
		if (sb.st_mode & _S_IFDIR) {
			attr = FILEATTR_DIRECTORY;
		}
		else {
			attr = 0;
		}
		if (!(sb.st_mode & S_IWRITE)) {
			attr |= FILEATTR_READONLY;
		}
#endif
		return(attr);
	}
	return(-1);
}

short file_getdatetime(FILEH handle, DOSDATE *dosdate, DOSTIME *dostime) {

struct stat sb;
struct tm	*ftime;

	if (fstat(fileno(handle), &sb) == 0) {
		ftime = localtime(&sb.st_mtime);
		if (ftime) {
			if (dosdate) {
				dosdate->year = ftime->tm_year + 1900;
				dosdate->month = ftime->tm_mon + 1;
				dosdate->day = ftime->tm_mday;
			}
			if (dostime) {
				dostime->hour = ftime->tm_hour;
				dostime->minute = ftime->tm_min;
				dostime->second = ftime->tm_sec;
			}
			return(0);
		}
	}
	return(-1);
}

short file_delete(const char *filename) {

	if (filename) {
		return(unlink(filename));
	}
	return(-1);
}

/* カレントファイル操作 */
void file_setcd(const char *exename) {

	milstr_ncpy(curpath, exename, sizeof(curpath));
	curfilep = getFileName(curpath);
	*curfilep = '\0';
}

char *file_getcd(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(curpath);
}

FILEH file_open_c(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(file_open(curpath));
}

FILEH file_open_rb_c(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(file_open_rb(curpath));
}

FILEH file_create_c(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(file_create(curpath));
}

short file_delete_c(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(file_delete(curpath));
}

short file_attr_c(const char *filename) {

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return(file_attr_c(curpath));
}

char *getFileName(char *filename) {

	char	*p;
	char	*q;

	p = filename;
	q = filename;
	while(*p) {
		if (*p++ == '/') {
			q = p;
		}
	}
	return(q);
}

void cutFileName(char *filename) {

	char	*p;

	p = getFileName(filename);
	*p = '\0';
}

char *getExtName(char *filename) {

	char	*p;
	char	*q;

	p = getFileName(filename);
	q = NULL;
	while(*p != '\0') {
		if (*p == '.') {
			q = p + 1;
		}
		p++;
	}
	if (q == NULL) {
		q = p;
	}
	return(q);
}

void cutExtName(char *filename) {

	char	*p;
	char	*q;

	p = getFileName(filename);
	q = NULL;
	while(*p != '\0') {
		if (*p == '.') {
			q = p;
		}
		p++;
	}
	if (q != NULL) {
		*q = '\0';
	}
}

void cutyen(char *str) {

	int		pos;

	pos = strlen(str) - 1;
	if ((pos > 0) && (str[pos] == '/')) {
		str[pos] = '\0';
	}
}

void plusyen(char *str, int len) {

	int		pos;

	pos = strlen(str);
	if ((pos) && (str[pos - 1] != '/') && ((pos + 2) < len)) {
		str[pos++] = '/';
		str[pos] = '\0';
	}
}

