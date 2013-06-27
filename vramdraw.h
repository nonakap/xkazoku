
#ifdef __cplusplus
extern "C" {
#endif

extern	VRAMHDL		mainvram;

BOOL vramdraw_create(void);
void vramdraw_destroy(void);

int vramdraw_orgsize(int size);
int vramdraw_half(int size);
void vramdraw_halfpoint(POINT_T *pt);
void vramdraw_halfsize(POINT_T *pt);
void vramdraw_halfscrn(SCRN_T *rct);
void vramdraw_halfrect(RECT_T *rct);

void vramdraw_scrn2rect(SCRN_T *scs, RECT_T *rct);

void vramdraw_setrect(VRAMHDL vram, const RECT_T *rect);
void vramdraw_draw(void);
void vramdraw_drawupper(VRAMHDL vram, const RECT_T *rect);

void vramdraw_savebmp(int num, const char *dir, const char *name,
						int pos, const char *credit0, const char *credit1);

#ifdef __cplusplus
}
#endif

