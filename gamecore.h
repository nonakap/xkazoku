
enum {
	GAMECORE_MAXSCENE	= 16,				// 実は12個？
	GAMECORE_MAXRGN		= 32,
	GAMECORE_MAXCMDS	= 16,
	GAMECORE_MAXVRAM	= 128,				// VRAMNUM_MAX
	GAMECORE_MAXSTACK	= 0x400,
	GAMECORE_MAXNAME	= 160,
	GAMECORE_MAXTXTWIN	= 8,
	GAMECORE_NAMELEN	= 32,
	GAMECORE_CHOICELEN	= (128 - 28),
	GAMECORE_MAXTEXT	= 30,
	GAMECORE_PATTBLS	= 100,

	SOUNDTRK_SOUND		= 0,
	SOUNDTRK_CDDA		= 0,
	SOUNDTRK_VOICE		= 1,
	SOUNDTRK_SE			= 2,
	SOUNDTRK_MAXSE		= 32
};

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
	SCRN_T		txtclip;
	SCRN_T		hisclip;
} DISPWIN_T, *DISPWIN;


typedef struct {
	int		size;
} PATTBL_T, *PATTBL;

typedef struct {
	int		*val;
	int		maxval;

	BYTE	*flag;
	int		maxflag;
	UINT	flagsize;			// セーブで必要。

	void	*exaval;
	void	*exaflag;
	void	*kid;

	PATTBL	pattbl[GAMECORE_PATTBLS];
} FLAGS_T, *FLAGS;


typedef struct {
	char		company[64];
	char		title[64];
	char		key[64];

	char		scriptpath[MAX_PATH];
} SUF_T;


typedef struct {
	int			version;
	int			type;
	int			width;
	int			height;
} SYS_T;


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

	LISTARRAY	cfglist;
	char		comment[100];
} GAMECORE;


enum {
	EXE_VER0		= 0,
	EXEVER_KANA,					// 99-06-16 加奈
	EXEVER_MYU,						// 99-07-16 Purple
	EXE_VER1		= 100,
	EXEVER_TEA2DEMO,				// 00-08-08 せんせい２デモ
	EXEVER_PLANET,					// 00-11-27 星空ぷらねっと
	EXEVER_PLANDVD,					// 01-01-31 星空ぷらねっとDVD
	EXEVER_NURSE,					// 01-06-01 プライベートナース
	EXEVER_KONYA,					// 01-07-15 私に今夜☆会いに来て
	EXEVER_CRES,					// 01-09-11 クレシェンド
	EXEVER_KAZOKU,					// 01-10-24 家族計画
	EXEVER_OSHIETE,					// 02-08-19 教えてあげちゃう
	EXEVER_SISKON,					// 02-09-02 しすこん
	EXEVER_KONYA2,					// 02-10-03 私に今夜☆会いに来て２
	EXEVER_KAZOKUK,					// 02-11-27 家族計画 絆箱
	EXEVER_HEART,					// 03-01-13 はぁとdeルームメイト
	EXEVER_DM,						// 03-03-11 ドーターメーカー
	EXEVER_MOEKKO					// 03-04-04 萌えッ娘ナース
};


enum {
	GAME_VOICE			= 0x0001,		// ヴォイスのサポート
	GAME_VOICEONLY		= 0x0002,		// ヴォイスのみのサポート
	GAME_HAVEALPHA		= 0x0004,		// テキスト窓可変通過処理サポート
	GAME_TEXTASCII		= 0x0008,		// 半角テキストを許す
	GAME_SVGA			= 0x0010,		// 800x600

	GAME_SAVEMYU		= 0x0100,		// MYU-.EXE セーブ
	GAME_SAVEMAX27		= 0x0200,		// セーブ数 27
	GAME_SAVEMAX30		= 0x0300,		// セーブ数 30
	GAME_SAVEMAX50		= 0x0400,		// セーブ数 50
	GAME_SAVEMASK		= 0x0700,
	GAME_SAVEGRPH		= 0x0800,		// グラフィックを含む
	GAME_SAVECOM		= 0x1000,		// コメントを含む
	GAME_SAVESYS		= 0x2000		// システム領域有り
};


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
	GAMEEV_EFFECT		= 3,			// エフェクト中
	GAMEEV_FADEOUT		= 4,			// フェードアウト中
	GAMEEV_FADEIN		= 5,			// フェードイン中
	GAMEEV_CMDWIN		= 6,			// コマンドウィンドウ実行中
	GAMEEV_GRAYSCALE	= 7,			// グレースケール(cmd:60)
	GAMEEV_SCROLL		= 8,			// スクロール処理(cmd:62)
	GAMEEV_QUAKE		= 9,			// 画面揺らし処理(cmd:63)
	GAMEEV_WAITPCMEND	= 10,			// PCM終了待ち
	GAMEEV_MSGCLK		= 11,			// メッセージクリック待ち
	GAMEEV_IRCLK		= 12,			// IRCLK クリック待ち
	GAMEEV_WAIT			= 13,			// ウェイト

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

BOOL gamecore_proc(void);
BOOL gamecore_exec(void);

#ifdef __cplusplus
}
#endif

