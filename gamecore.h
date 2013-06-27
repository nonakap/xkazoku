
enum {
	GAMECORE_MAXSCENE	= 16,
	GAMECORE_MAXRGN		= 64,
	GAMECORE_MAXCMDS	= 16,
	GAMECORE_MAXVRAM	= 128,
	GAMECORE_MAXSTACK	= 0x400,
	GAMECORE_MAXNAME	= 160,
	GAMECORE_MAXTXTWIN	= 8,
	GAMECORE_NAMELEN	= 32,
	GAMECORE_CMDTEXTLEN	= 128,
	GAMECORE_CHOICELEN	= (128 - 20),
	GAMECORE_MAXTEXT	= 30,
	GAMECORE_PATTBLS	= 100,
	GAMECORE_MAXGADFILE	= 32,
	GAMECORE_MAXGADHDL	= 32,
	GAMECORE_MAXGANHDL	= 32,

	SOUNDTRK_SOUND		= 0,
	SOUNDTRK_CDDA		= 0,
	SOUNDTRK_VOICE		= 1,
	SOUNDTRK_SE			= 2,
	SOUNDTRK_MAXSE		= 32
};

#include	"gamedef.h"
#include	"vram.h"
#include	"vramdraw.h"
#include	"vrammix.h"
#include	"script.h"
#include	"scr_base.h"
#include	"textwin.h"
#include	"textdisp.h"
#include	"event.h"
#include	"effect.h"
#include	"gamecfg.h"
#include	"anime.h"
#include	"sndplay.h"


enum {
	DISPWIN_VRAM		= 0x01,
//	DISPWIN_TEXT		= 0x02,
	DISPWIN_CLIPTEXT	= 0x04,
	DISPWIN_CLIPHIS		= 0x08
};

typedef struct {
	UINT		flag;
	int			vramnum;
//	int			textnum;
	int			posx;
	int			posy;
	int			fontsize;
	UINT		fonttype;
	SCRN_T		txtclip;
	SCRN_T		hisclip;
} DISPWIN_T, *DISPWIN;


typedef struct {
	int		size;
} PATTBL_T, *PATTBL;

typedef struct {
	int		*val;
	UINT	maxval;

	BYTE	*flag;
	UINT	maxflag;
	UINT	flagsize;			// セーブで必要。

	void	*exaval;
	void	*exaflag;
	void	*kid;

	PATTBL	pattbl[GAMECORE_PATTBLS];
} FLAGS_T, *FLAGS;

typedef struct {
	SINT32	num;
	SINT32	bit;
	char	name[16];
} CGFLAG_T, *CGFLAG;

typedef struct {
	SINT32	width;
	SINT32	indenty;
	SINT32	indentx;
	SINT32	length;
	BOOL	flag;					// centering?
} SCOMCFG_T, *SCOMCFG;

typedef struct {
	BYTE		type;
	BYTE		pos;
} SCRNSHOT_T;

typedef struct {
	UINT16		curpage;
	BYTE		savepre;
	BYTE		saverenewal;
	BYTE		*flag;
	LISTARRAY	scr;
	LISTARRAY	kid;
	LISTARRAY	cgflag;
} DRSSYS_T, *DRSSYS;

typedef struct {
	SYS_T		sys;
	int			event;
//	void		*cgload;
	UNIRECT		vramrect;
	SCENE_T		*curscene;
	int			scenes;
	int			stacks;

	VRAMHDL		vram[GAMECORE_MAXVRAM];
	UINT		stack[GAMECORE_MAXSTACK];
	SCENE_T		scene[GAMECORE_MAXSCENE];

	EVTHDL_T	evthdl;
	DISPWIN_T	dispwin;
	GAMECFG_T	gamecfg;
	ANIME_T		anime;
	MOUSEEVT_T	mouseevt;
	TEXTWIN		textwin[GAMECORE_MAXTXTWIN];
	TEXTDISP_T	textdisp;
	FLAGS_T		flags;
	EFFECT_T	ef;
	SNDPLAY_T	sndplay;
	SUF_T		suf;
	TIMEEVT_T	timer;
	TIMEEVT_T	wait;
	TEXTCTRL_T	textdraw;
	CWEVT_T		cwevt;
	SCOMCFG_T	scomcfg;

	SCRNSHOT_T	scrnshot;
	DRSSYS_T	drssys;

	LISTARRAY	cfglist;
	char		comment[100];
	char		ggdname[GAMECORE_MAXVRAM][16];
	char		sename[SOUNDTRK_MAXSE][16];

	BOOL		initialized;
const char		*err;
	char		errstr[256];
} GAMECORE;


enum {
	GAMEEV_EOS			= -100,			// end of script
	GAMEEV_WRONGCMDLENG	= -101,			// wrong length
	GAMEEV_WRONGCMD		= -102,			// wrong command
	GAMEEV_WRONGLENG	= -103,			// wrong length
	GAMEEV_WRONGPARAM	= -104,			// wrong parameter

	GAMEEV_MEMORYERROR	= -110,			// memory allocation error
	GAMEEV_ALREADYVAL	= -111,			// value table existed
	GAMEEV_FILENOTFOUND	= -112,			// script file not found

	GAMEEV_FAILURE		= -200,			// todo

	GAMEEV_ERROR		= -2,			// エラー・強制終了
	GAMEEV_EXIT			= -1,			// ゲーム終了
	GAMEEV_SUCCESS		= 0,			// 次のステップへ
	GAMEEV_TEXTOUT		= 1,			// テキスト表示
	GAMEEV_WAITMOUSE	= 2,			// マウスムーブ / プッシュ待ち
	GAMEEV_CMDWIN		= 3,			// コマンドウィンドウ実行中
	GAMEEV_WAITPCMEND	= 4,			// PCM終了待ち
	GAMEEV_MSGCLK		= 5,			// メッセージクリック待ち
	GAMEEV_IRCLK		= 6,			// IRCLK クリック待ち
	GAMEEV_WAIT			= 7,			// ウェイト
	GAMEEV_FADEIN		= 10,			// フェードイン中(cmd:54)
	GAMEEV_FADEOUT		= 11,			// フェードアウト中(cmd:55)
	GAMEEV_EFFECT		= 12,			// エフェクト中(cmd:57)
	GAMEEV_GRAYSCALE	= 13,			// グレースケール(cmd:60)
	GAMEEV_SCROLL		= 14,			// スクロール処理(cmd:62)
	GAMEEV_QUAKE		= 15,			// 画面揺らし処理(cmd:63)

	GAMEEV_FORCE		= 100			// 強行
};


#ifdef __cplusplus
extern "C" {
#endif

extern	GAMECORE	gamecore;

BOOL gamecore_init(const char *path);
void gamecore_destroy(void);

void gamecore_resetvolume(void);
void gamecore_reset(void);

const char *gamecore_seterrorevent(int event);
const char *gamecore_geterror(void);

BOOL gamecore_proc(void);
BOOL gamecore_exec(void);

#ifdef __cplusplus
}
#endif

