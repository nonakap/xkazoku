/*	$Id: dosio.c,v 1.3 2003/04/22 15:23:27 yui Exp $	*/

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
	int len = strlen(exename);

	strncpy(curpath, exename, sizeof(curpath));
	if (len > 0 && curpath[0] == '/')
		plusyen(curpath, sizeof(curpath));
	curfilep = curpath + strlen(curpath) + 1;
	*curfilep = '\0';
}

char *
file_getcd(const char *filename)
{

	strncpy(curfilep, filename, curfilep - curpath);
	return curpath;
}

FILEH
file_open_c(const char *filename)
{

	strncpy(curfilep, filename, curfilep - curpath);
	return file_open(curpath);
}

FILEH
file_open_rb_c(const char *filename)
{

	strncpy(curfilep, filename, curfilep - curpath);
	return file_open_rb(curpath);
}

FILEH
file_create_c(const char *filename)
{

	strncpy(curfilep, filename, curfilep - curpath);
	return file_create(curpath);
}

short
file_delete_c(const char *filename)
{

	strncpy(curfilep, filename, curfilep - curpath);
	return file_delete(curpath);
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

int
kanji1st(char *str, int pos)
{
	int ret = 0;
	BYTE c;

	for (; pos > 0; pos--) {
		c = (BYTE)str[pos];
		if (!((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xfc)))
			break;
		ret ^= 1;
	}
	return ret;
}

int
kanji2nd(char *str, int pos)
{
	int ret = 0;
	BYTE c;

	while (pos-- > 0) {
		c = (BYTE)str[pos];
		if (!((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xfc)))
			break;
		ret ^= 1;
	}
	return ret;
}


int
ex_a2i(const char *str, int min, int max)
{
	int	ret = 0;
	char	c;

	if (str == NULL)
		return(min);

	for (;;) {
		c = *str++;
		if (c == ' ')
			continue;
		if ((c < '0') || (c > '9'))
			break;
		ret = ret * 10 + (c - '0');
	}

	if (ret < min)
		return min;
	else if (ret > max)
		return max;
	return ret;
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


void
fname_mix(char *str, char *mix, int size)
{
	char *p;
	int len;
	char c;
	char check;

	cutFileName(str);
	if (mix[0] == '/')
		str[0] = '\0';

	len = strlen(str);
	p = str + len;
	check = '.';
	while (len < size) {
		c = *mix++;
		if (c == '\0')
			break;

		if (c == check) {
			/* current dir */
			if (mix[0] == '/') {
				mix++;
				continue;
			}
			/* parent dir */
			if (mix[0] == '.' && mix[1] == '/') {
				mix += 2;
				cutyen(str);
				cutFileName(str);
				len = strlen(str);
				p = str + len;
				continue;
			}
		}
		if (c == '/')
			check = '.';
		else
			check = 0;
		*p++ = c;
		len++;
	}
	if (p < str + len)
		*p = '\0';
	else
		str[len - 1] = '\0';
}
