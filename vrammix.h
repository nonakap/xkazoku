
#ifdef __cplusplus
extern "C" {
#endif

void vramcpy_cpy(VRAMHDL dst, VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpypat(VRAMHDL dst, VRAMHDL src, const BYTE *pat8,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyex(VRAMHDL dst, const VRAMHDL src,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_cpyalpha(VRAMHDL dst, const VRAMHDL src, UINT alpha,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mix(VRAMHDL dst, const VRAMHDL src1,
									const VRAMHDL src2, UINT alpha,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mixcol(VRAMHDL dst, const VRAMHDL src,
									UINT32 color, UINT alpha,
									const POINT_T *pt, const RECT_T *rct);
void vramcpy_mosaic(VRAMHDL dst, const VRAMHDL src, int dot,
									const POINT_T *pt, const RECT_T *rct);

void vrammix_cpy(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpyall(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpy2(VRAMHDL dst, const VRAMHDL src, BYTE alpha,
													const RECT_T *rct);
void vrammix_cpypat(VRAMHDL dst, const VRAMHDL src, const BYTE *pat8,
													const RECT_T *rct);
void vrammix_cpyex(VRAMHDL dst, const VRAMHDL src, const RECT_T *rct);
void vrammix_cpyalpha(VRAMHDL dst, const VRAMHDL src,
										UINT alpha, const RECT_T *rct);
void vrammix_mix(VRAMHDL dst, const VRAMHDL src1, const VRAMHDL src2,
										UINT alpha, const RECT_T *rct);
void vrammix_mixcol(VRAMHDL dst, const VRAMHDL src, UINT32 color,
										UINT alpha, const RECT_T *rct);
void vrammix_graybmp(VRAMHDL dst, const VRAMHDL src1, const VRAMHDL src2,
										const VRAMHDL bmp,
										int delta, const RECT_T *rct);

void vrammix_text(VRAMHDL dst, void *fhdl, const char *str,
							UINT32 color, POINT_T *pt, const RECT_T *rct);

#ifdef __cplusplus
}
#endif

