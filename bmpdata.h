
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

UINT bmpdata_getalign(BMPINFO *bi);
UINT bmpdata_getdatasize(BMPINFO *bi);

void bmpdata_setinfo(BMPINFO *bi, const BMPDATA *inf);
BOOL bmpdata_getinfo(const BMPINFO *bi, BMPDATA *inf);

VRAMHDL bmpdata_load8(const char *name);

VRAMHDL bmpdata_vram24cnv(int width, int height,
											BMPINFO *bi, const BYTE *data);
BYTE *bmpdata_bmp24cnv(BMPINFO *bi, VRAMHDL vram);

#ifdef __cplusplus
}
#endif
