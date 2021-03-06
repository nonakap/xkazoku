/*	$Id: compiler.h,v 1.7 2003/08/13 05:01:54 yui Exp $	*/

/*
 * Copyright (C) 2002-2004 NONAKA Kimihiro
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

typedef	signed char	CHAR;
typedef	signed short	SHORT;
typedef	signed int	INT;
typedef	signed long	LONG;

typedef	unsigned char	UCHAR;
typedef	unsigned short	USHORT;
typedef	unsigned int	UINT;
typedef	unsigned long	ULONG;

typedef	signed char	SINT8;
typedef	unsigned char	UINT8;
typedef	short		SINT16;
typedef	unsigned short	UINT16;
typedef	int		SINT32;
typedef	unsigned int	UINT32;

typedef	unsigned char	BYTE;
typedef	unsigned char	TCHAR;

typedef	int		BOOL;

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
#define	ZeroMemory(d,n)		memset((d), 0, (n))
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

#include <SDL.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define	BYTESEX_BIG
#else /* SDL_BYTEORDER == SDL_LIL_ENDIAN */
#define	BYTESEX_LITTLE
#endif	/* SDL_BYTEORDER == SDL_BIG_ENDIAN */

#define	GETTICK()	SDL_GetTicks()

#define	GETRAND()	random()

#define	__ASSERT(s)	assert(s)

#define	UNUSED(v)	((void)(v))

#ifndef	NELEMENTS
#define	NELEMENTS(a)	((int)(sizeof(a) / sizeof(a[0])))
#endif

#include "common.h"
#include "milstr.h"
#include "_memory.h"
#include "profile.h"
#include "variant.h"
#include "rect.h"
#include "lstarray.h"
#include "trace.h"

#ifndef	SIZE_QVGA
#define	SUPPORT_24BPP
#define	SCREEN_BPP	24
#else
#define	SUPPORT_16BPP
#define	SCREEN_BPP	16
#endif

#define	VERMOUTH_LIB
#define	SUPPORT_DRS

/*
 * You could specify a complete path, e.g. "/etc/timidity.cfg", and
 * then specify the library directory in the configuration file.
 */
#ifndef	TIMIDITY_CFGFILE
#define	TIMIDITY_CFGFILE	"timidity.cfg"
#endif	/* TIMIDITY_CFGFILE */
