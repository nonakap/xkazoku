/*	$Id: trace.c,v 1.1.1.1 2003/04/14 12:47:46 yui Exp $	*/

#include "compiler.h"

#include <stdarg.h>

#include "dosio.h"

#ifdef TRACE

static unsigned short
sjis2jis(unsigned short sjis)
{
	unsigned short h, l;
    
	h = (sjis >> 8) & 0xff;
	l = sjis & 0xff;

	h -= (h <= 0x9f) ? 0x71 : 0xb1;
	h = (h << 1) + 1;
	if (l > 0x7f)
		l--;
	if (l >= 0x9e) {
		l -= 0x7d;
		h++;
	} else
		l -= 0x1f;

	return (h << 8) | l;
}

static unsigned short
jis2euc(unsigned short jis)
{

	return ((jis + 0x80) & 0xff) + ((jis + 0x8000) & 0xff00);
}

unsigned short
sjis2euc(unsigned short sjis)
{

	return jis2euc(sjis2jis(sjis));
}

unsigned char *
sjis2euc_str(unsigned char *str)
{
	unsigned char *p, *q;
	int iskanji2nd;
	unsigned short sjis;
	unsigned short euc;
	unsigned char w;

	iskanji2nd = 0;
	sjis = 0;
	for (p = q = str; *p != '\0'; p++) {
		if (!iskanji2nd) {
			if ((*p & 0x80) == 0) {
				q++;
				continue;
			}

			w = *p ^ 0x40;
			if ((w >= 0xa0) && (w <= 0xdf)) {
				sjis = *p << 8;
				iskanji2nd = 1;
			} else
				q++;
		} else {
			w = *p & 0x40;
			if ((*p >= 0x40 && *p <= 0x7e)
			    || (*p >= 0x80 && *p <= 0xfc)) {
				sjis |= *p;
				iskanji2nd = 0;

				euc = sjis2euc(sjis);
				*q++ = (euc >> 8) & 0xff;
				*q++ = euc & 0xff;
			} else {
				q++;
			}
		}
	}
	return str;
}

void
trace_init(void)
{

	/* Nothing to do */
}

void
trace_term(void)
{

	/* Nothing to do */
}

void
trace_fmt(const char *fmt, ...)
{
	static unsigned char buf[0x1000];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	printf("%s\n", sjis2euc_str(buf));
}

#endif
