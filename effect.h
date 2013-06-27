
typedef struct {
	BYTE		cmd;
	int			param;
	VRAMHDL		src;
	POINT_T		pt;
	RECT_T		r;
	RECT_T		r2;

	int			param2;
	int			param3;
	int			param4;
	int			param5;

	int			cnt;
	int			basetick;

	VRAMHDL		bmp;
	VRAMHDL		tmp1;
	VRAMHDL		tmp2;

	UINT32		tick;
	UINT32		col;
	int			lastalpha;
	BYTE		pat8[8];
} EFFECT_T, *EFFECT;


#ifdef __cplusplus
extern "C" {
#endif

void effect_trush(void);

int effect_set(SCR_OPE *op);
int effect_exec(void);

int effect_fadeout(UINT32 tick, BYTE *col);
int effect_fadeoutexec(void);

int effect_grayscale(void);
int effect_scroll(void);

#ifdef __cplusplus
}
#endif

