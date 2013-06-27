#include	<windows.h>
#include	<stdio.h>
#include	<SDL.h>

#define	SLZAURUS
#define	BYTESEX_LITTLE

#ifndef __GNUC__
typedef	signed char		SINT8;
typedef	unsigned char	UINT8;
typedef	signed short	SINT16;
typedef	unsigned short	UINT16;
typedef	signed int		SINT32;
typedef	unsigned int	UINT32;
#else
#include	<stdlib.h>
typedef	signed char		SINT8;
typedef	unsigned char	UINT8;
typedef	signed short	SINT16;
typedef	unsigned short	UINT16;
typedef	signed int		SINT32;
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
#define	GETRAND()	rand()

#define	SUPPORT_16BPP
#define	SCREEN_BPP			16
#define	SIZE_QVGA
#define	SUPPORT_PPCARC
#define	AMETHYST_LIB
#define	VERMOUTH_LIB
#define DAMEDEC_LIB
#define	SUPPORT_DRS

