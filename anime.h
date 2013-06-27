
typedef struct {
	int		frames;
	VRAMHDL	vram;
	POINT_T	pt;
	int		width;
	int		height;
} _GADHDL, *GADHDL;

typedef struct {
	GADHDL	hdl;
	int		frame;
	int		enable;
	UINT32	basetick;
	UINT32	nexttick;
} ANIME_T, *ANIME;


#ifdef __cplusplus
extern "C" {
#endif

GADHDL gad_create(UINT type, const char *fname);
void gad_destroy(GADHDL hdl);

void anime_exec(void);

#ifdef __cplusplus
}
#endif

