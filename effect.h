
typedef struct {
	BYTE	pat8[8];
} EFPAT;

typedef struct {
	int		leng;
	int		step;
	int		cnt;
	int		total;
} EFLINE;

typedef struct {
	int		leng;
	int		dir;
	int		step;
	int		cnt;
	int		base;
} EFBLIND;

typedef struct {
	UINT32	col;
} EFCOL;

typedef struct {
	int		phase;
	int		type;
	int		dir;
	int		leng;
} EFGRAY;

typedef struct {
	POINT_T	start;
	POINT_T	step;
	POINT_T	last;
} EFSCRL;

typedef struct {
	int		cnt;
	POINT_T	last;
} EFQUAKE;

typedef struct {
	BYTE	cmd;
	BYTE	upper;
	int		param;
	VRAMHDL	src;
	POINT_T	pt;
	RECT_T	r;
	RECT_T	r2;

	VRAMHDL	bmp;

	UINT32	tick;
	int		progress;
	union {
		EFPAT	ep;
		EFLINE	el;
		EFBLIND	eb;
		EFCOL	ec;
		EFGRAY	eg;
		EFSCRL	es;
		EFQUAKE	eq;
	} ex;
} EFFECT_T, *EFFECT;


#ifdef __cplusplus
extern "C" {
#endif

void effect_trush(void);

void effect_vramdraw(int num, const RECT_T *rect);

int effect_set(SCR_OPE *op);								// cmd:57
int effect_exec(void);

int effect_fadeinset(UINT32 tick, int num, UINT32 col);		// cmd:54
int effect_fadeinexec(void);

int effect_fadeoutset(UINT32 tick, int num, UINT32 col);	// cmd:55
int effect_fadeoutexec(void);

int effect_grayscale(void);									// cmd:60
int effect_scroll(void);									// cmd:62
int effect_quake(void);										// cmd:63

#ifdef __cplusplus
}
#endif

