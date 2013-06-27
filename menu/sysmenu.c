#include	"compiler.h"
#include	"fontmng.h"
#include	"scrnmng.h"
#include	"taskmng.h"
#include	"gamecore.h"
#include	"menubase.h"
#include	"sysmenu.res"
#include	"sysmenu.h"
#include	"dlgcfg.h"
#include	"dlgsave.h"


	char	str_ok[] = "OK";
	BYTE	str_cancel[] = {		// キャンセル
					0x83,0x4c,0x83,0x83,0x83,0x93,0x83,0x5a,0x83,0x8b,0x00};
	BYTE	str_apply[] = {			// 適用
					0x93,0x4b,0x97,0x70,0x00};
	BYTE	str_prop[] = {			// プロパティ
		0x83,0x76,0x83,0x8d,0x83,0x70,0x83,0x65,0x83,0x42,0x00};


// ----

static const BYTE str_winmd0[] = {	// ウィンドウモード
		0x83,0x45,0x83,0x42,0x83,0x93,0x83,0x68,0x83,0x45,0x83,0x82,0x81,0x5b,
		0x83,0x68,0x00};
static const BYTE str_winmd1[] = {	// フルスクリーンモード
		0x83,0x74,0x83,0x8b,0x83,0x58,0x83,0x4e,0x83,0x8a,0x81,0x5b,0x83,0x93,
		0x83,0x82,0x81,0x5b,0x83,0x68,0x00};
static const BYTE str_windel[] = {	// ウィンドウの消去
		0x83,0x45,0x83,0x42,0x83,0x93,0x83,0x68,0x83,0x45,0x82,0xcc,0x8f,0xc1,
		0x8b,0x8e,0x00};
static const BYTE str_skip[] = {	// メッセージスキップ
		0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,0x83,0x58,0x83,0x4c,
		0x83,0x62,0x83,0x76,0x00};
static const BYTE str_read[] = {	// 既読メッセージスキップ
		0x8a,0xf9,0x93,0xc7,0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,
		0x83,0x58,0x83,0x4c,0x83,0x62,0x83,0x76,0x00};
static const BYTE str_auto[] = {	// オートクリック
		0x83,0x49,0x81,0x5b,0x83,0x67,0x83,0x4e,0x83,0x8a,0x83,0x62,0x83,0x4e,
		0x00};
static const BYTE str_auto2[] = {	// 自動送りモード解除
		0x83,0x49,0x81,0x5b,0x83,0x67,0x83,0x4e,0x83,0x8a,0x83,0x62,0x83,0x4e,
		0x89,0xf0,0x8f,0x9c,0x00};

static const BYTE str_log[] = {		// 既読メッセージの再表示
		0x8a,0xf9,0x93,0xc7,0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,
		0x82,0xcc,0x8d,0xc4,0x95,0x5c,0x8e,0xa6,0x00};
static const BYTE str_title[] = {	// タイトルに戻る
		0x83,0x5e,0x83,0x43,0x83,0x67,0x83,0x8b,0x82,0xc9,0x96,0xdf,0x82,0xe9,
		0x00};
static const BYTE str_exit[] = {	// ゲーム終了
		0x83,0x51,0x81,0x5b,0x83,0x80,0x8f,0x49,0x97,0xb9,0x00};

static const BYTE str_titler[] = {	// タイトルに戻ります。よろしいですか？
		0x83,0x5e,0x83,0x43,0x83,0x67,0x83,0x8b,0x82,0xc9,0x96,0xdf,0x82,0xe8,
		0x82,0xdc,0x82,0xb7,0x81,0x42,0x82,0xe6,0x82,0xeb,0x82,0xb5,0x82,0xa2,
		0x82,0xc5,0x82,0xb7,0x82,0xa9,0x81,0x48,0x00};

static const BYTE str_exitr[] = {	// ゲームを終了します。よろしいですか？
		0x83,0x51,0x81,0x5b,0x83,0x80,0x82,0xf0,0x8f,0x49,0x97,0xb9,0x82,0xb5,
		0x82,0xdc,0x82,0xb7,0x81,0x42,0x82,0xe6,0x82,0xeb,0x82,0xb5,0x82,0xa2,
		0x82,0xc5,0x82,0xb7,0x82,0xa9,0x81,0x48,0x00};


static const MSYSITEM s_popup[] = {
		{(char *)str_winmd1,	NULL,		MID_SCREEN,		MENU_GRAY},
		{(char *)str_windel,	NULL,		MID_TEXT,		MENU_GRAY},
//		{"ウィンドウ初期配置",	NULL,		MID_TXTPOS,		MENU_GRAY},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_prop,		NULL,		MID_CONFIG,		0},
		{"SAVE",				NULL,		MID_SAVE,		0},
		{"LOAD",				NULL,		MID_LOAD,		0},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},

		{(char *)str_skip,		NULL,		MID_SKIP,		0},
		{(char *)str_read,		NULL,		MID_READSKIP,	0},
		{NULL,					NULL,		MID_SKIPSEP,	MENU_SEPARATOR},
		{(char *)str_auto,		NULL,		MID_AUTO,		0},
		{NULL,					NULL,		MID_AUTOSEP,	MENU_SEPARATOR},
		{(char *)str_log,		NULL,		MID_LOG,		MENU_GRAY},
		{NULL,					NULL,		MID_LOGSEP,		MENU_SEPARATOR},
		{"HELP",				NULL,		MID_HELP,		MENU_GRAY},
		{NULL,					NULL,		MID_HELPSEP,	MENU_SEPARATOR},
		{(char *)str_title,		NULL,		MID_TITLE,		0},
		{(char *)str_exit,		NULL,		MID_EXIT,		MENU_DELETED}};

static const MSYSITEM s_main[] = {
		{"POPUP",				s_popup,	MID_POPUP,		MENUS_POPUP |
															MENU_DELETED}};


static void sys_cmd(MENUID id);


// ----

BOOL sysmenu_create(void) {

	if (menubase_create() != SUCCESS) {
		goto smcre_err;
	}
	if (menusys_create(s_main, NULL, sys_cmd, gamecore.suf.title)) {
		goto smcre_err;
	}
	return(SUCCESS);

smcre_err:
	return(FAILURE);
}


void sysmenu_destroy(void) {

	menubase_close();
	menubase_destroy();
	menusys_destroy();
}


BOOL sysmenu_menuopen(BOOL menu, int x, int y) {

	GAMECFG	gc;
	int		version;
	int		i;

	gc = &gamecore.gamecfg;
	if (menu) {
		menusys_sethide(MID_POPUP, FALSE);
		version = gamecore.sys.version;
		if (version == EXEVER_MYU) {

			for (i=MID_SKIP; i<=MID_EXIT; i++) {
				menusys_sethide((MENUID)i, TRUE);
			}
			menusys_sethide(MID_HELP, FALSE);
		}

#ifdef SUPPORT_FULLSCREEN
		menusys_setenable(MID_SCREEN, TRUE);
		menusys_settext(MID_SCREEN,
							(char *)(gc->fullscreen?str_winmd0:str_winmd1));
#endif
		menusys_setenable(MID_SAVE, gc->enablesave);
		menusys_setcheck(MID_SKIP, gc->skip);
		menusys_setcheck(MID_READSKIP, gc->readskip);
		menusys_settext(MID_AUTO, (char *)(gc->autoclick?str_auto2:str_auto));
	}
	else {
		menusys_sethide(MID_POPUP, TRUE);
	}
	return(menusys_open(x, y));
}


// ----

static void sys_cmd(MENUID id) {

	int		val;
	BOOL	r;

	switch(id) {
#ifdef SUPPORT_FULLSCREEN
		case MID_SCREEN:
			scrnmng_fullscreen(!gamecore.gamecfg.fullscreen);
			break;
#endif

		case MID_CONFIG:
			cfgdlg_open();
			break;

		case MID_SKIP:
			val = !menusys_getcheck(MID_SKIP);
			gamecfg_setskip(val);
			break;

		case MID_READSKIP:
			val = !menusys_getcheck(MID_READSKIP);
			gamecfg_setreadskip(val);
			break;

		case MID_AUTO:
			gamecore.gamecfg.autoclick = !gamecore.gamecfg.autoclick;
			break;

		case MID_LOAD:
			r = scr_scriptcall("LOADSYS");
			if (r == SUCCESS) {
				gamecore.event = GAMEEV_SUCCESS;		// 要調整
			}
			else {
				dlgsave_load();
			}
			break;

		case MID_SAVE:
			r = scr_scriptcall("SAVESYS");
			if (r == SUCCESS) {
				gamecore.event = GAMEEV_SUCCESS;		// 要調整
			}
			else {
				dlgsave_save();
			}
			break;

		case MID_TITLE:
			if (menumbox((char *)str_titler, gamecore.suf.key,
								MBOX_YESNO | MBOX_ICONQUESTION) == DID_YES) {
				if (scr_restart("TITLE") != SUCCESS) {
					scr_restart("MAIN");			// プライベートナース
				}
			}
			break;

		case MID_EXIT:
			TRACEOUT(("title: %s", gamecore.suf.key));
			if (menumbox((char *)str_exitr, gamecore.suf.key,
								MBOX_YESNO | MBOX_ICONQUESTION) == DID_YES) {
				taskmng_exit();
			}
			break;

#if defined(MENU_TASKMINIMIZE)
		case SID_MINIMIZE:
			taskmng_minimize();
			break;
#endif

		case SID_CLOSE:
			taskmng_exit();
			break;
	}
}

