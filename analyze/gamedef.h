
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

