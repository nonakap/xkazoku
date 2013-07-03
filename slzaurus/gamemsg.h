
// #define	GAMEMSG_MSGWAIT		0x083a		// メッセージウェイト設定

// #define	GAMEMSG_SAVE		0x00e5		// セーブ窓
// #define	GAMEMSG_LOAD		0x01e5		// ロード窓

// #define	GAMEMSG_MOVIE		0x00f3		// ムービー再生
// #define	GAMEMSG_ENABLEPOPUP	0x00f4		// ポップアップメニュー可否
// #define	GAMEMSG_ENABLESAVE	0x00f5		// セーブ可否

// #define	GAMEMSG_EFFECT		0x0100		// エフェクト中 CB


#ifdef __cplusplus
extern "C" {
#endif

int gamemsg_send(UINT msg, long param);

#ifdef __cplusplus
}
#endif

