
typedef struct {
	int			msgtype;				// 1041:1043:1047
	int			textwaittick;			// 1040

	int			bgm;					// 1038
	int			bgmvol;					// 1039
	int			se;						// 1035
	int			sevol;					// 1037
	int			voice;					// 1034
	int			voicevol;				// 1036

	UINT32		winrgb;					// 1100:1102:1101
	int			winalpha;				// 1099

	int			fullscreen;

	int			skip;
	int			readskip;
	int			autoclick;

	int			lastread;

	BYTE		enablepopup;
	BYTE		enablesave;
} GAMECFG_T, *GAMECFG;

enum {
	CFGTYPE_TAG,
	CFGTYPE_FRAME,
	CFGTYPE_RADIO,
	CFGTYPE_CHECK,
	CFGTYPE_SLIDER
};

typedef struct {
	char	str[64];
} GCD_TAG;

typedef struct {
	char	str[64];
} GCD_FRAME;

typedef struct {
	UINT16	val;
	char	str[64];
} GCD_RADIO;

typedef struct {
	UINT16	val;
	char	str[64];
} GCD_CHECK;

typedef struct {
	UINT16	val;
	SINT32	minval;
	SINT32	maxval;
	SINT32	mul;
	char	str[32];
	char	min[16];
	char	max[16];
} GCD_SLIDER;

typedef struct {
	BYTE	cmd;
	BYTE	page;
	BYTE	group;
	BYTE	num;
	UINT16	id;
	union {
		GCD_TAG		t;
		GCD_FRAME	f;
		GCD_RADIO	r;
		GCD_CHECK	c;
		GCD_SLIDER	s;
	} c;
} _GCDLG, *GCDLG;


#ifdef __cplusplus
extern "C" {
#endif

void gamecfg_init(void);
void gamecfg_reset(void);

void gamecfg_load(void);
void gamecfg_save(void);

void gamecfg_setskip(int skip);
void gamecfg_setreadskip(int readskip);

#ifdef __cplusplus
}
#endif

