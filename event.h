
enum {
	CUR_SYSTEM	= 14,
	CUR_USER	= 32
};

typedef struct {
	int		x;
	int		y;
	UINT	btn;
	UINT	key;
} EVTHDL_T, *EVTHDL;

typedef struct {
	int			num;
	UINT16		val;
} CWEVT_T, *CWEVT;

typedef struct {
	UINT16		cur;
	UINT16		btn;
	BYTE		cnt;
	BYTE		btnflg;
} MOUSEPRM;

typedef struct {
	RECT_T		r;
	UINT16		num;
	BYTE		kind;
	BYTE		c2;
	BYTE		c3;
	BYTE		c4;
} REGIONPRM;

typedef struct {
	BOOL	enable;
	SINT32	move[8];
} REGIONKEY;

enum {
	MEVT_RENEWAL	= 0x01,
	MEVT_CLIP		= 0x02,
	MEVT_RELMODE	= 0x04
};

typedef struct {
	int			curnum;
	int			pos;
	int			lastpos;
	int			keypos;
	int			defpos;
	UINT		flag;
	VRAMHDL		map;
	RECT_T		clip;
	MOUSEPRM	prm;
	REGIONPRM	rgn[GAMECORE_MAXRGN];
	REGIONKEY	key[GAMECORE_MAXRGN];
} MOUSEEVT_T, *MOUSEEVT;

typedef struct {
	SINT32		val;
	UINT32		base;
} TIMEEVT_T, *TIMEEVT;


#ifdef __cplusplus
extern "C" {
#endif

void event_setmouse(int x, int y);
UINT event_getmouse(int *x, int *y);
void event_resetmouse(UINT mask);
UINT event_getkey(void);
void event_resetkey(UINT mask);
void event_resetall(void);

void event_cmdwindraw(TEXTWIN textwin, int num, int method);
void event_choicewindraw(TEXTWIN textwin, int num, int method);

int event_getcmdwin(TEXTWIN textwin);
int event_cmdwin(void);

int event_waitpcmend(void);

void event_mousereset(void);
void event_mouserel(int rel);
int event_mouse(void);

int event_msgclk(void);
int event_irclk(void);
int event_wait(void);

#ifdef __cplusplus
}
#endif

