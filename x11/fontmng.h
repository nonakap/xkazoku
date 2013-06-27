/*	$Id: fontmng.h,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#ifdef __cplusplus
extern "C" {
#endif

extern char fontname[MAX_PATH];

enum {
	FDAT_BOLD		= 0x01,
	FDAT_PROPORTIONAL	= 0x02
};

typedef struct {
	int	width;
	int	height;
	int	pitch;
} _FNTDAT, *FNTDAT;

void *fontmng_create(int size, UINT type, const TCHAR *fontface);
void fontmng_destroy(void *hdl);

BOOL fontmng_getsize(void *hdl, const char *string, POINT_T *pt);
BOOL fontmng_getdrawsize(void *hdl, const char *string, POINT_T *pt);
FNTDAT fontmng_get(void *hdl, const char *string);

#ifdef __cplusplus
}
#endif
