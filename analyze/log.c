#include	"compiler.h"
#include	<stdarg.h>
#include	"codecnv.h"
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
		codecnv_sjis2euc(euc, sizeof(euc), sjis, sizeof(sjis));
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

