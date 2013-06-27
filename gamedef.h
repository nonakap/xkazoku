
enum {
	EXE_VER0		= 0,
	EXEVER_KANA,					// 99-06-16 加奈
	EXEVER_MYU,						// 99-07-16 Purple
	EXEVER_KANAUS,					// KANA - little sister -

	EXE_VER1		= 100,
	EXEVER_TSUKU,					// 00-02-15 尽くしてあげちゃう
	EXEVER_AMEIRO,					// 00-04-18 あめいろの季節
	EXEVER_TEA2,					// 00-08-10 せ・ん・せ・い２
	EXEVER_PLANET,					// 00-11-27 星空ぷらねっと
	EXEVER_PLANDVD,					// 01-01-31 星空ぷらねっとDVD
	EXEVER_NURSE,					// 01-06-01 プライベートナース
	EXEVER_KONYA,					// 01-07-15 私に今夜☆会いに来て
	EXEVER_VECHO,					// 01-08-29 ヴェルベットエコー
	EXEVER_CRES,					// 01-09-11 クレシェンド
	EXEVER_CRESD,					// 03-07-02 クレシェンドFV
	EXEVER_KAZOKU,					// 01-10-24 家族計画
	EXEVER_BLEED,					// 02-05-10 出血簿
	EXEVER_ROSYU2,					// 02-05-10 露出調教同好会２
	EXEVER_OSHIETE,					// 02-08-19 教えてあげちゃう
	EXEVER_SISKON,					// 02-09-02 しすこん
	EXEVER_KONYA2,					// 02-10-03 私に今夜☆会いに来て２
	EXEVER_KAZOKUK,					// 02-11-27 家族計画 絆箱
	EXEVER_HEART,					// 03-01-13 はぁとdeルームメイト
	EXEVER_SHISYO,					// 03-01-16 司書さんといっしょ☆

	EXE_VER2		= 200,
	EXEVER_HIMITSU,					// 03-02-23 女の子のヒミツ
	EXEVER_DM,						// 03-03-11 ドーターメーカー
	EXEVER_MOEKKO,					// 03-04-04 萌えッ娘ナース
	EXEVER_AOI,						// 03-05-07 青い涙
	EXEVER_KAMOOK,					// 03-05-30 家族計画～絆本～
//	EXE_VER210		= 210,
	EXEVER_RESTORE,					// 03-06-11 れすとあ
	EXEVER_AKIBA,					// 03-06-20 アキバでお茶しよっ！
	EXEVER_OSHI_TAI,				// 03-07-12 おしかけハーレム体験版
	EXEVER_KT_TRY					// 03-08-07 黒の図書館体験版
};

enum {
	GAME_VOICE			= 0x00000001,	// ヴォイスのサポート
	GAME_VOICEONLY		= 0x00000002,	// ヴォイスのみのサポート
	GAME_HAVEALPHA		= 0x00000004,	// テキスト窓可変通過処理サポート
	GAME_TEXTASCII		= 0x00000008,	// 半角テキストを許す
	GAME_DRS			= 0x00000010,	// DRS system
	GAME_SVGA			= 0x00000020,	// 800x600
	GAME_NOKID			= 0x00000040,	// 既読フラグなし
	GAME_ENGSTYLE		= 0x00000080,	// ダミー

	GAME_SAVEMAX9		= 0x00000100,	// セーブ数 9
	GAME_SAVEMAX27		= 0x00000200,	// セーブ数 27
	GAME_SAVEMAX30		= 0x00000300,	// セーブ数 30
	GAME_SAVEMAX50		= 0x00000400,	// セーブ数 50
	GAME_SAVEMASK		= 0x00000700,
	GAME_SAVEGRPH		= 0x00000800,	// グラフィックを含む
	GAME_SAVECOM		= 0x00001000,	// コメントを含む
	GAME_SAVESYS		= 0x00002000,	// システム領域有り
	GAME_SAVEAUTOCOM	= 0x00004000,	// 表示テキストをコメントにする
	GAME_SAVEALIAS		= 0x00008000,	// xkazoku.sav でセーブ

	// 以下 細かい挙動用 (別に無くても良い)
	GAME_SELECTEX		= 0x00010000,	// 選択フォーカスを殺さない
	GAME_TEXTLAYER		= 0x00020000,	// テキストレイヤー分離
	GAME_CMDWINNOBG		= 0x00040000	// 選択肢バックグラウンドなし
};


typedef struct {
	char	company[64];
	char	title[64];
	char	key[64];
	char	scriptpath[MAX_PATH];
} SUF_T;

typedef struct {
	UINT	version;
	UINT	type;
	int		width;
	int		height;

	int		bmpwidth;
	int		bmpheight;
	int		defbits;
	int		defflags;
} SYS_T;

#ifdef __cplusplus
extern "C" {
#endif

void gamedef_create(void);
void gamedef_destroy(void);
void gamedef_loadsuf(const char *path, SUF_T *suf, SYS_T *sys);

#ifdef __cplusplus
}
#endif

