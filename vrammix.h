
enum {
	VRAMALPHA		= 255,
	VRAMALPHABIT	= 8
};


#ifdef __cplusplus
extern "C" {
#endif

void vramcpy_cpy(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_move(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyall(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpypat(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyex(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyexa(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyalpha(VRAMHDL dst, const VRAMHDL src, UINT alpha256,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mix(VRAMHDL dst, const VRAMHDL org,
									const VRAMHDL src, UINT alpha64,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mixcol(VRAMHDL dst, const VRAMHDL src,
									UINT32 color, UINT alpha64,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_zoom(VRAMHDL dst, const VRAMHDL src, int dot,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mosaic(VRAMHDL dst, const VRAMHDL src, int dot,
									const POINT_T *pt, const RECT_T *rct);

void vrammix_cpy(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpyall(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpy2(VRAMHDL dst, const VRAMHDL src, UINT alpha,
													const RECT_T *rct);
void vrammix_cpypat(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
													const RECT_T *rct);
void vrammix_cpypat16w(VRAMHDL dst, const VRAMHDL src, const UINT pat16,
													const RECT_T *rct);
void vrammix_cpypat16h(VRAMHDL dst, const VRAMHDL src, const UINT pat16,
													const RECT_T *rct);
void vrammix_cpyex(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpyex2(VRAMHDL dst, const VRAMHDL src,
										UINT alpha64, const RECT_T *rct);
void vrammix_cpyexpat16w(VRAMHDL dst, const VRAMHDL src, const UINT pat16,
													const RECT_T *rct);
void vrammix_cpyexpat16h(VRAMHDL dst, const VRAMHDL src, const UINT pat16,
													const RECT_T *rct);
void vrammix_mix(VRAMHDL dst, const VRAMHDL org, const VRAMHDL src,
										UINT alpha64, const RECT_T *rct);
void vrammix_mixcol(VRAMHDL dst, const VRAMHDL src, UINT32 color,
										UINT alpha64, const RECT_T *rct);
void vrammix_mixalpha(VRAMHDL dst, const VRAMHDL src, UINT32 color,
													const RECT_T *rct);
void vrammix_graybmp(VRAMHDL dst, const VRAMHDL org, const VRAMHDL src,
										const VRAMHDL bmp,
										int delta, const RECT_T *rct);

void vrammix_colex(VRAMHDL dst, const VRAMHDL src, UINT32 color,
													const RECT_T *rct);

void vrammix_resize(VRAMHDL dst, const RECT_T *drct,
									const VRAMHDL src, const RECT_T *srct);

void vrammix_text(VRAMHDL dst, void *fhdl, const char *str,
							UINT32 color, POINT_T *pt, const RECT_T *rct);
void vrammix_textex(VRAMHDL dst, void *fhdl, const char *str,
							UINT32 color, POINT_T *pt, const RECT_T *rct);

#ifdef __cplusplus
}
#endif

