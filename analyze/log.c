#include	"compiler.h"
#include	<stdarg.h>
#include	"dosio.h"
#include	"analyze.h"


#if defined(_WIN32)
#define	DEFAULT_CODE	(LOG_SJIS | LOG_CRLF)
#elif defined(__MACOS__)
#define	DEFAULT_CODE	(LOG_SJIS | LOG_CR)
#else
#define	DEFAULT_CODE	(LOG_EUC | LOG_LF)
#endif

typedef struct {
	UINT16	leng;
	BYTE	str[2];
} CRCODE;

static const CRCODE crcode[4] = {
				{0, {0x00, 0x00}}, {1, {0x0a, 0x00}},
				{1, {0x0d, 0x00}}, {2, {0x0d, 0x0a}}};
static const char logfile[] = "analyze.txt";

static	int		_codetype = DEFAULT_CODE;
static	FILEH	logfh = FILEH_INVALID;
static	FILEH	extfh = FILEH_INVALID;


void log_open(void) {

	FILEH	fh;

	if (logfh == FILEH_INVALID) {
		fh = file_open(logfile);
		if (fh != FILEH_INVALID) {
			file_seek(fh, 0, FSEEK_END);
			file_write(fh, "\r\n", 2);
		}
		else {
			fh = file_create(logfile);
		}
		logfh = fh;
	}
}

void log_ext(const char *filename) {

	FILEH	fh;

	fh = extfh;
	if (fh != FILEH_INVALID) {
		extfh = FILEH_INVALID;
		file_close(fh);
	}
	if (filename) {
		extfh = file_create(filename);
	}
}

void log_close(void) {

	FILEH	fh;

	fh = logfh;
	if (fh != FILEH_INVALID) {
		logfh = FILEH_INVALID;
		file_close(fh);
	}
	log_ext(NULL);
}


static const BYTE euctbl[0x80] = {
	0xa1, 0xa1, 0xa1, 0xa3, 0xa1, 0xd6, 0xa1, 0xd7, 0xa1, 0xa2, 0xa1, 0xa6,
	0xa5, 0xf2, 0xa5, 0xa1, 0xa5, 0xa3, 0xa5, 0xa5, 0xa5, 0xa7, 0xa5, 0xa9,
	0xa5, 0xe3, 0xa5, 0xe5, 0xa5, 0xe7, 0xa5, 0xc3, 0xa1, 0xbc, 0xa5, 0xa2,
	0xa5, 0xa4, 0xa5, 0xa6, 0xa5, 0xa8, 0xa5, 0xaa, 0xa5, 0xab, 0xa5, 0xad,
	0xa5, 0xaf, 0xa5, 0xb1, 0xa5, 0xb3, 0xa5, 0xb5, 0xa5, 0xb7, 0xa5, 0xb9,
	0xa5, 0xbb, 0xa5, 0xbd, 0xa5, 0xbf, 0xa5, 0xc1, 0xa5, 0xc4, 0xa5, 0xc6,
	0xa5, 0xc8, 0xa5, 0xca, 0xa5, 0xcb, 0xa5, 0xcc, 0xa5, 0xcd, 0xa5, 0xce,
	0xa5, 0xcf, 0xa5, 0xd2, 0xa5, 0xd5, 0xa5, 0xd8, 0xa5, 0xdb, 0xa5, 0xde,
	0xa5, 0xdf, 0xa5, 0xe0, 0xa5, 0xe1, 0xa5, 0xe2, 0xa5, 0xe4, 0xa5, 0xe6,
	0xa5, 0xe8, 0xa5, 0xe9, 0xa5, 0xea, 0xa5, 0xeb, 0xa5, 0xec, 0xa5, 0xed,
	0xa5, 0xef, 0xa5, 0xf3, 0xa1, 0xab, 0xa1, 0xac};

static void sjis2euc(BYTE *euc, const BYTE *sjis, int eucsize) {

	int		s;
	int		c;

	if (eucsize <= 0) {
		return;
	}
	eucsize--;
	while(1) {
		s = *sjis++;
		if (s < 0x80) {				// ascii
			if (!s) {
				break;
			}
			eucsize--;
			if (eucsize < 0) {
				break;
			}
			*euc++ = (BYTE)s;
		}
		else if ((((s ^ 0x20) - 0xa1) & 0xff) < 0x2f) {
			c = *sjis++;
			if (!c) {
				break;
			}
			eucsize -= 2;
			if (eucsize < 0) {
				break;
			}
			c += 0x62 - ((c & 0x80) >> 7);
			if (c < 256) {
				c = (c - 0xa2) & 0x1ff;
			}
			c += 0x9fa1;
			*euc++ = (BYTE)(((s & 0x3f) << 1) + (c >> 8));
			*euc++ = (BYTE)c;
		}
		else {
			s = (s - 0xa0) & 0xff;
			if (s < 0x40) {
				eucsize -= 2;
				if (eucsize < 0) {
					break;
				}
				s <<= 1;
				*euc++ = euctbl[s+0];
				*euc++ = euctbl[s+1];
			}
		}
	}
	*euc = '\0';
}


void log_disp(const char *fmt, ...) {

	char		*ptr;
	char		sjis[2048];
	char		euc[2048];
	va_list		ap;
	FILEH		fh;
	int			codetype;
const CRCODE	*cr;

	ptr = sjis;
	va_start(ap, fmt);
	vsprintf(ptr, fmt, ap);
	va_end(ap);

	codetype = _codetype;
	if (codetype & LOG_EUC) {
		sjis2euc((BYTE *)euc, (const BYTE *)sjis, sizeof(euc));
		ptr = euc;
	}
	puts(ptr);

	fh = extfh;
	if (fh == FILEH_INVALID) {
		fh = logfh;
	}
	if (fh != FILEH_INVALID) {
		file_write(fh, ptr, strlen(ptr));
		cr = crcode + (codetype & 3);
		if (cr->leng) {
			file_write(fh, cr->str, cr->leng);
		}
	}
}

