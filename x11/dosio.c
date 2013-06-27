/*	$Id: dosio.c,v 1.6 2003/06/26 17:58:31 yui Exp $	*/

#include "compiler.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "dosio.h"
#include "milstr.h"

static char curpath[MAX_PATH+32] = "";
static char *curfilep = curpath;

void
dosio_init(void)
{

	/* Nothing to do. */
}

void
dosio_term(void)
{

	/* Nothing to do. */
}

/* ファイル操作 */
FILEH
file_open(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r+");
	return (FILEH)fp;
}

FILEH
file_open_rb(const char *filename)
{

	return file_open(filename);
}

FILEH
file_create(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w+");
	return (FILEH)fp;
}

long
file_seek(FILEH handle, long pointer, int method)
{

	fseek(handle, pointer, method);
	return ftell(handle);
}

UINT
file_read(FILEH handle, void *data, UINT length)
{
	size_t readsize;

	readsize = fread(data, 1, length, handle);
	return (UINT)readsize;
}

UINT
file_write(FILEH handle, const void *data, UINT length)
{
	size_t writesize;

	writesize = fwrite(data, 1, length, handle);
	return (UINT)writesize;
}

short
file_close(FILEH handle)
{

	fclose(handle);
	return 0;
}

UINT
file_getsize(FILEH handle)
{
	struct stat sb;

	if (fstat(fileno(handle), &sb) == 0)
		return sb.st_size;
	return 0;
}

short
file_attr(const char *filename)
{
	struct stat sb;
	short attr = FILE_ATTRIBUTE_NORMAL;

	if (stat(filename, &sb) == 0) {
		if (S_ISDIR(sb.st_mode))
			return FILE_ATTRIBUTE_DIRECTORY;
		if (!(sb.st_mode & S_IWUSR))
			attr |= FILE_ATTRIBUTE_READONLY;
		return attr;
	}
	return -1;
}

short
file_getdatetime(FILEH handle, DOSDATE *dosdate, DOSTIME *dostime)
{
	struct stat sb;
	struct tm tm;

	if (fstat(fileno(handle), &sb) == 0) {
		localtime_r(&sb.st_mtime, &tm);
		if (dosdate) {
			dosdate->year = tm.tm_year; /* XXX: origin 1900 */
			dosdate->month = tm.tm_mon; /* XXX: 0-11 */
			dosdate->day = tm.tm_mday;
		}
		if (dostime) {
			dostime->hour = tm.tm_hour;
			dostime->minute = tm.tm_min;
			dostime->second = tm.tm_sec;
		}
		return 0;
	}
	return -1;
}

short
file_delete(const char *filename)
{

	if (filename)
		return unlink(filename);
	return -1;
}

/* カレントファイル操作 */
void
file_setcd(const char *exename)
{

	milstr_ncpy(curpath, exename, sizeof(curpath));
	curfilep = getFileName(curpath);
	*curfilep = '\0';
}

char *
file_getcd(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return curpath;
}

FILEH
file_open_c(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return file_open(curpath);
}

FILEH
file_open_rb_c(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return file_open_rb(curpath);
}

FILEH
file_create_c(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return file_create(curpath);
}

short
file_delete_c(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return file_delete(curpath);
}

short
file_attr_c(const char *filename)
{

	milstr_ncpy(curfilep, filename, sizeof(curpath) - (curfilep - curpath));
	return file_attr_c(curpath);
}

char *
getFileName(char *filename)
{
	char *p, *q;

	for (p = q = filename; *p != '\0'; p++)
		if (*p == '/')
			q = p + 1;
	return q;
}

char *
cutFileName(char *filename)
{
	char *p, *q;

	for (p = q = filename; *p != '\0'; p++)
		if (*p == '/')
			q = p + 1;
	*q = '\0';
}

char *
getExtName(char * filename)
{
	char *p, *q;

	p = getFileName(filename);
	q = NULL;

	while (*p != '\0') {
		if (*p == '.')
			q = p + 1;
		p++;
	}
	if (q == NULL)
		q = p;
	return q;
}

void
cutExtName(char *filename)
{
	char *p, *q;

	p = getFileName(filename);
	q = NULL;

	while (*p != '\0') {
		if (*p == '.')
			q = p;
		p++;
	}
	if (q != NULL)
		*q = '\0';
}

void
cutyen(char *str)
{
	int pos = strlen(str) - 1;

	if ((pos > 0) && (str[pos] == '/'))
		str[pos] = '\0';
}

void
plusyen(char *str, int len)
{
	int pos = strlen(str);

	if (pos) {
		if (str[pos - 1] == '/')
			return;

		if ((pos + 2) >= len)
			return;
		str[pos++] = '/';
		str[pos] = '\0';
	}
}

