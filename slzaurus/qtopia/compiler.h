#include	<sys/param.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
#include	<unistd.h>
#include	<assert.h>
#include	<pthread.h>
#include	<SDL.h>

#define	SLZAURUS

typedef	signed int		INT;

typedef	unsigned char	UCHAR;
typedef	unsigned short	USHORT;
typedef	unsigned int	UINT;
typedef	unsigned long	ULONG;

typedef	signed char		SINT8;
typedef	unsigned char	UINT8;
typedef	signed short	SINT16;
typedef	unsigned short	UINT16;
typedef	signed int		SINT32;
typedef	unsigned int	UINT32;

typedef	signed char		CHAR;
typedef	unsigned char	TCHAR;
typedef	unsigned char	BYTE;
typedef	int				BOOL;

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	MAX_PATH
#define	MAX_PATH	MAXPATHLEN
#endif

#ifndef	max
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef	min
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef	wsprintf
#define	wsprintf		sprintf
#endif

#ifndef	ZeroMemory
#define	ZeroMemory(d,n)		bzero((d),(n))
#endif
#ifndef	CopyMemory
#define	CopyMemory(d,s,n)	memcpy((d),(s),(n))
#endif
#ifndef	FillMemory
#define	FillMemory(a, b, c)	memset((a),(c),(b))
#endif

#ifndef	roundup
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define	BYTESEX_BIG
#else /* SDL_BYTEORDER == SDL_LIL_ENDIAN */
#define	BYTESEX_LITTLE
#endif	/* SDL_BYTEORDER == SDL_BIG_ENDIAN */

#define	__ASSERT(s)	assert(s)

#define	UNUSED(v)	((void)(v))

#ifndef	NELEMENTS
#define	NELEMENTS(a)	((int)(sizeof(a) / sizeof(a[0])))
#endif

#include	"common.h"
#include	"milstr.h"
#include	"_memory.h"
#include	"profile.h"
#include	"variant.h"
#include	"rect.h"
#include	"lstarray.h"
#include	"trace.h"

#define	GETTICK()	SDL_GetTicks()
#define	GETRAND()	random()

#define	SUPPORT_16BPP
#define	SCREEN_BPP			16
#define	SIZE_QVGA
#define	SUPPORT_PPCARC
// #define	SOUND_CRITICAL
#define	AMETHYST_LIB
#define	VERMOUTH_LIB
#define DAMEDEC_LIB
#define	SUPPORT_DRS


// You could specify a complete path, e.g. "/etc/timidity.cfg", and
// then specify the library directory in the configuration file.
// #define TIMIDITY_CFGFILE	"timidity.cfg"

