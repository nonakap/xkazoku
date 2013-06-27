
#if defined(__GNUC__)
typedef struct {
	BYTE	bfType[2];
	BYTE	bfSize[4];
	BYTE	bfReserved1[2];
	BYTE	bfReserved2[2];
	BYTE	bfOffBits[4];
} __attribute__ ((packed)) BMPFILE;
typedef struct {
	BYTE	biSize[4];
	BYTE	biWidth[4];
	BYTE	biHeight[4];
	BYTE	biPlanes[2];
	BYTE	biBitCount[2];
	BYTE	biCompression[4];
	BYTE	biSizeImage[4];
	BYTE	biXPelsPerMeter[4];
	BYTE	biYPelsPerMeter[4];
	BYTE	biClrUsed[4];
	BYTE	biClrImportant[4];
} __attribute__ ((packed)) BMPINFO;
#else
#pragma pack(push, 1)
typedef struct {
	BYTE	bfType[2];
	BYTE	bfSize[4];
	BYTE	bfReserved1[2];
	BYTE	bfReserved2[2];
	BYTE	bfOffBits[4];
} BMPFILE;
typedef struct {
	BYTE	biSize[4];
	BYTE	biWidth[4];
	BYTE	biHeight[4];
	BYTE	biPlanes[2];
	BYTE	biBitCount[2];
	BYTE	biCompression[4];
	BYTE	biSizeImage[4];
	BYTE	biXPelsPerMeter[4];
	BYTE	biYPelsPerMeter[4];
	BYTE	biClrUsed[4];
	BYTE	biClrImportant[4];
} BMPINFO;
#pragma pack(pop)
#endif

typedef struct {
	int		width;
	int		height;
	int		bpp;
} BMPDATA;


#ifdef __cplusplus
extern "C" {
#endif

UINT bmpdata_getalignex(BMPINFO *bi, BOOL pad);
UINT bmpdata_getdatasizeex(BMPINFO *bi, BOOL pad);

void bmpdata_setinfo(BMPINFO *bi, const BMPDATA *inf, BOOL setsize);
BOOL bmpdata_getinfo(const BMPINFO *bi, BMPDATA *inf);

VRAMHDL bmpdata_load8(const char *name);

VRAMHDL bmpdata_vram24cnvex(int width, int height,
							BMPINFO *bi, const BYTE *data, BOOL pad);
BYTE *bmpdata_bmp24cnvex(BMPINFO *bi, VRAMHDL vram, BOOL pad);

#ifdef __cplusplus
}
#endif


// ---- Macro

#define	bmpdata_getalign(a)				bmpdata_getalignex(a, TRUE)
#define	bmpdata_getdatasize(a)			bmpdata_getdatasizeex(a, TRUE)
#define	bmpdata_vram24cnv(a, b, c, d)	bmpdata_vram24cnvex(a, b, c, d, TRUE)
#define	bmpdata_bmp24cnv(a, b)			bmpdata_bmp24cnvex(a, b, TRUE)

