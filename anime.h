
typedef struct {
	int		size;
	int		pat;
	VRAMHDL	vram;
} _GADFILE, *GADFILE;

typedef struct {
	int		enable;
	VRAMHDL	gad;
	VRAMHDL	bak;
	SINT32	reg;
	int		loop;

	int		frame;
	UINT32	tick;
	int		frames;
	POINT_T	pt;
	int		width;
	int		height;
} _GADHDL, *GADHDL;

typedef struct {
	UINT	size;
	UINT	scrmax;
	UINT	picmax;

	UINT	vrampos;
	VRAMHDL	vram[2];

	UINT	scrpos;
	UINT	reg;
	UINT32	tick;
} _GANHDL, *GANHDL;

typedef struct {
	GADFILE	gadfile;
	int		gadenable;

	GANHDL	ganenable;
	VRAMHDL	ganvram;

	VRAMHDL	gadvram[GAMECORE_MAXGADFILE];
	GADHDL	gadhdl[GAMECORE_MAXGADHDL];
	GANHDL	ganhdl[GAMECORE_MAXGANHDL];
} ANIME_T, *ANIME;


#ifdef __cplusplus
extern "C" {
#endif

void anime_open(int gad, const char *fname);
void anime_close(void);
void anime_trush(int gad);
void anime_setloop(int num, int loop);
void anime_setdata(int num, int major, int minor, SINT32 reg);
void anime_setloc(int num, int x, int y);
void anime_start(int cmd, int param1, int param2);
void anime_end(int cmd, int param1, int param2);

void anime_exec(void);
void anime_alltrush();

#ifdef __cplusplus
}
#endif

