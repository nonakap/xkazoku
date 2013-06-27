#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"sysmenu.h"
#include	"menubase.h"
#include	"sysmenu.res"
#include	"dlgcfg.h"
#include	"sound.h"
#include	"sstream.h"


// move(11,27)
static const char str_msgmd[] = "Message Mode";
static const char str_msgspd[] = "Message Speed";
static const char str_sndmd[] = "Sound Setting";
static const char str_test[] = "Test";
static const char str_min[] = "MIN";
static const char str_max[] = "MAX";

static const BYTE str_syscfg[] = {		// [  システム設定  ]
		0x20,0x20,0xbc,0xbd,0xc3,0xd1,0x90,0xdd,0x92,0xe8,0x20,0x20,0x00};

static const BYTE str_voice[] = {		// 音声
		0x89,0xb9,0x90,0xba,0x00};
static const BYTE str_text[] = {		// テキスト
		0x83,0x65,0x83,0x4c,0x83,0x58,0x83,0x67,0x00};
static const BYTE str_textvoice[] = {	// テキスト＋音声
		0x83,0x65,0x83,0x4c,0x83,0x58,0x83,0x67,0x81,0x7b,0x89,0xb9,0x90,0xba,
		0x00};
static const BYTE str_se[] = {			// 効果音
		0x8c,0xf8,0x89,0xca,0x89,0xb9,0x00};
static const BYTE str_bgm[] = {			// ＢＧＭ
		0x82,0x61,0x82,0x66,0x82,0x6c,0x00};


static const BYTE str_window[] = {		// ウィンドウ
		0xb3,0xa8,0xdd,0xc4,0xde,0xb3,0x00};

static const BYTE str_wincol[] = {		// ウィンドウ色
		0x83,0x45,0x83,0x42,0x83,0x93,0x83,0x68,0x83,0x45,0x90,0x46,0x00};
static const BYTE str_wincolr[] = {		// 赤(R)
		0x90,0xd4,0x28,0x52,0x29,0x00};
static const BYTE str_wincolg[] = {		// 緑(G)
		0x97,0xce,0x28,0x47,0x29,0x00};
static const BYTE str_wincolb[] = {		// 青(B)
		0x90,0xc2,0x28,0x42,0x29,0x00};
static const BYTE str_wincole[] = {		// 透過度(%)
		0x93,0xa7,0x89,0xdf,0x93,0x78,0x28,0x25,0x29,0x00};
static const BYTE str_wincold[] = {		// 標準設定
		0x95,0x57,0x8f,0x80,0x90,0xdd,0x92,0xe8,0x00};


#ifndef SIZE_QVGA

static const MENUPRM res_cfg[] = {
			{DLGTYPE_TABLIST,	DID_TAB,		0,
				NULL,									  7,   6, 419, 289},
			{DLGTYPE_BUTTON,	DID_OK,			0,
				str_ok,									150, 301,  88,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		0,
				str_cancel,								245, 301,  88,  21},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_GRAY,
				str_apply,								339, 301,  88,  21}};


// ヴォイス無しシステムメニュー
static const MENUPRM res_108[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 23,  44, 387,  80},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142,  60,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375,  60,  32,  13},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				147,  79, 247,  21},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 23, 134, 387, 134},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142, 155,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375, 155,  32,  13},
			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 37, 183,  64,  14},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								102, 180,  37,  18},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 176, 247,  21},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 37, 222,  64,  14},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 215, 247,  21},
};


// ヴォイスのみ有りシステムメニュー
static const MENUPRM res_109[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgmd,								 23,  36, 387,  41},
			{DLGTYPE_RADIO,		DID_TEXTMD0,	MENU_TABSTOP,
				str_voice,								 61,  54, 102,  13},
			{DLGTYPE_RADIO,		DID_TEXTMD1,	0,
				str_text,								164,  54, 104,  13},
			{DLGTYPE_RADIO,		DID_TEXTMD2,	0,
				str_textvoice,							269,  54, 128,  13},

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 23,  78, 387,  68},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142,  95,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375,  95,  32,  13},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				147, 110, 247,  21},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 23, 146, 387, 134},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142, 159,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375, 159,  32,  13},
			{DLGTYPE_CHECK,		DID_VOICE,		MENU_TABSTOP,
				str_voice,								 37, 181,  64,  14},
			{DLGTYPE_BUTTON,	DID_VOICETEST,	MENU_TABSTOP,
				str_test,								102, 179,  37,  18},
			{DLGTYPE_SLIDER,	DID_VOICEVOL,	MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 175, 247,  21},
			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 37, 213,  64,  14},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 206, 247,  21},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								102, 210,  37,  18},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 37, 244,  64,  14},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 238, 247,  21},
};


// ヴォイス有りシステムメニュー
static const MENUPRM res_191[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgmd,								 23,  36, 387,  41},
			{DLGTYPE_RADIO,		DID_TEXTMD1,	MENU_TABSTOP,
				str_text,								 44,  54,  80,  13},
			{DLGTYPE_RADIO,		DID_TEXTMD2,	0,
				str_textvoice,							198,  54, 112,  13},

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 23,  78, 387,  68},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142,  95,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375,  95,  32,  13},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				147, 110, 247,  21},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 23, 146, 387, 134},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								142, 159,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								375, 159,  32,  13},
			{DLGTYPE_CHECK,		DID_VOICE,		MENU_TABSTOP,
				str_voice,								 37, 181,  64,  14},
			{DLGTYPE_SLIDER,	DID_VOICEVOL,	MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 175, 247,  21},
			{DLGTYPE_BUTTON,	DID_VOICETEST,	MENU_TABSTOP,
				str_test,								102, 179,  37,  18},
			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 37, 213,  64,  14},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 206, 247,  21},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								102, 210,  37,  18},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 37, 244,  64,  14},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				147, 238, 247,  21},
};


// ウィンドウ色
static const MENUPRM res_199[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_wincol,								 20,  35, 394, 245},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 92,  62,  32,  13},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								300,  62,  32,  13},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolr,							 28,  84,  56,  13},
			{DLGTYPE_SLIDER,	DID_WINCOLR,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 98,  77, 220,  21},
			{DLGTYPE_EDIT,		DID_WINCOLR2,	0,
				NULL,									335,  75,  63,  21},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolg,							 28, 126,  56,  13},
			{DLGTYPE_SLIDER,	DID_WINCOLG,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 98, 119, 220,  21},
			{DLGTYPE_EDIT,		DID_WINCOLG2,	0,
				NULL,									335, 117,  63,  21},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolb,							 28, 168,  56,  13},
			{DLGTYPE_SLIDER,	DID_WINCOLB,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 98, 161, 220,  21},
			{DLGTYPE_EDIT,		DID_WINCOLB2,	0,
				NULL,									335, 159,  63,  21},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincole,							 28, 210,  56,  13},
			{DLGTYPE_SLIDER,	DID_WINCOLE,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 100),				 98, 203, 220,  21},
			{DLGTYPE_EDIT,		DID_WINCOLE2,	0,
				NULL,									335, 201,  63,  21},
			{DLGTYPE_BUTTON,	DID_WINCOLDEF,	MENU_TABSTOP,
				str_wincold,							177, 245,  88,  23},
};

#else

static const MENUPRM res_cfg[] = {
			{DLGTYPE_TABLIST,	DID_TAB,		0,
				NULL,									  5,   3, 279, 194},
			{DLGTYPE_BUTTON,	DID_OK,			0,
				str_ok,									102, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_CANCEL,		0,
				str_cancel,								164, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_GRAY,
				str_apply,								226, 200,  58,  15}};


// ヴォイス無しシステムメニュー
static const MENUPRM res_108[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 15,  29, 258,  54},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95,  38,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247,  38,  24,  11},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				 98,  52, 164,  15},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 15,  89, 258,  89},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95, 101,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247, 101,  24,  11},

			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 24, 121,  42,  11},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								 67, 120,  28,  12},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 118, 164,  15},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 24, 147,  42,  11},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 145, 164,  15},
};


// ヴォイスのみ有りシステムメニュー
static const MENUPRM res_109[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgmd,								 15,  24, 258,  28},
			{DLGTYPE_RADIO,		DID_TEXTMD0,	MENU_TABSTOP,
				str_voice,								 39,  35,  68,  11},
			{DLGTYPE_RADIO,		DID_TEXTMD1,	0,
				str_text,								109,  35,  68,  11},
			{DLGTYPE_RADIO,		DID_TEXTMD2,	0,
				str_textvoice,							179,  35,  88,  11},

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 15,  52, 258,  45},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95,  62,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247,  62,  24,  11},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				 98,  73, 164,  15},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 15,  97, 258,  85},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95, 105,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247, 105,  24,  11},
			{DLGTYPE_CHECK,		DID_VOICE,		MENU_TABSTOP,
				str_voice,								 24, 119,  42,  11},
			{DLGTYPE_SLIDER,	DID_VOICEVOL,	MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 116, 164,  15},
			{DLGTYPE_BUTTON,	DID_VOICETEST,	MENU_TABSTOP,
				str_test,								 67, 118,  26,  12},
			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 24, 140,  42,  11},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 137, 164,  15},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								 67, 139,  26,  12},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 24, 161,  42,  11},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 158, 164,  15},
};


// ヴォイス有りシステムメニュー
static const MENUPRM res_191[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgmd,								 15,  24, 258,  28},
			{DLGTYPE_RADIO,		DID_TEXTMD1,	MENU_TABSTOP,
				str_text,								 29,  35, 100,  11},
			{DLGTYPE_RADIO,		DID_TEXTMD2,	0,
				str_textvoice,							131,  35, 100,  11},

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_msgspd,								 15,  52, 258,  45},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95,  61,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247,  61,  24,  11},
			{DLGTYPE_SLIDER,	DID_TEXTSPD,	MENU_TABSTOP,
				(void *)SLIDERPOS(20, 0),				 98,  73, 164,  15},
			    // ↑ 逆、修正済 NONAKA.K

			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_sndmd,								 15,  97, 258,  85},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 95, 104,  24,  11},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_max,								247, 104,  24,  11},
			{DLGTYPE_CHECK,		DID_VOICE,		MENU_TABSTOP,
				str_voice,								 24, 119,  42,  11},
			{DLGTYPE_SLIDER,	DID_VOICEVOL,	MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 116, 164,  15},
			{DLGTYPE_BUTTON,	DID_VOICETEST,	MENU_TABSTOP,
				str_test,								 67, 118,  26,  12},
			{DLGTYPE_CHECK,		DID_SE,			MENU_TABSTOP,
				str_se,									 24, 140,  42,  11},
			{DLGTYPE_SLIDER,	DID_SEVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 137, 164,  15},
			{DLGTYPE_BUTTON,	DID_SETEST,		MENU_TABSTOP,
				str_test,								 67, 139,  26,  12},
			{DLGTYPE_CHECK,		DID_BGM,		MENU_TABSTOP,
				str_bgm,								 24, 161,  42,  11},
			{DLGTYPE_SLIDER,	DID_BGMVOL,		MENU_TABSTOP,
				(void *)SLIDERPOS(-16, 8),				 98, 158, 164,  15},
};


// ウィンドウ色
static const MENUPRM res_199[] = {
			{DLGTYPE_FRAME,		DID_STATIC,		0,
				str_wincol,								 13,  24, 262, 163},
			{DLGTYPE_LTEXT,		DID_STATIC,		0,
				str_min,								 62,  38,  24,  11},
			{DLGTYPE_RTEXT,		DID_STATIC,		0,
				str_max,								195,  38,  24,  11},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolr,							 16,  54,  44,  11},
			{DLGTYPE_SLIDER,	DID_WINCOLR,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 65,  51, 147,  15},
			{DLGTYPE_EDIT,		DID_WINCOLR2,	0,
				NULL,									223,  51,  42,  14},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolg,							 16,  82,  44,  11},
			{DLGTYPE_SLIDER,	DID_WINCOLG,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 65,  79, 147,  15},
			{DLGTYPE_EDIT,		DID_WINCOLG2,	0,
				NULL,									223,  79,  42,  14},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincolb,							 16, 110,  44,  11},
			{DLGTYPE_SLIDER,	DID_WINCOLB,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 255),				 65, 107, 147,  15},
			{DLGTYPE_EDIT,		DID_WINCOLB2,	0,
				NULL,									223, 107,  42,  14},
			{DLGTYPE_CTEXT,		DID_STATIC,		0,
				str_wincole,							 16, 138,  44,  11},
			{DLGTYPE_SLIDER,	DID_WINCOLE,	MENU_TABSTOP,
				(void *)SLIDERPOS(0, 100),				 65, 135, 147,  15},
			{DLGTYPE_EDIT,		DID_WINCOLE2,	0,
				NULL,									223, 135,  42,  14},
			{DLGTYPE_BUTTON,	DID_WINCOLDEF,	MENU_TABSTOP,
				str_wincold,							118, 163,  58,  15},
};

#endif


static	int		maxpage = 0;

static void resetcfg(void) {

	GAMECFG	gc;
	int		i;

	gc = &gamecore.gamecfg;
	sndplay_cddaenable(gc->bgm, 0);
	sndplay_waveenable(gc->bgm, 0);
	sndplay_seenable(gc->se, 0);
	soundmix_setgain(SOUNDTRK_SOUND, gc->bgmvol);
	soundmix_setgain(SOUNDTRK_VOICE, gc->voicevol);
	for (i=0; i<SOUNDTRK_MAXSE; i++) {
		soundmix_setgain(SOUNDTRK_SE + i, gc->sevol);
	}
}

static void setwincol(UINT32 winrgb, UINT winalpha) {

	int		val;
	char	work[16];

	val = (winrgb >> 16) & 0xff;
	menudlg_setval(DID_WINCOLR, val);
	sprintf(work, "%u", val);
	menudlg_settext(DID_WINCOLR2, work);
	val = (winrgb >> 8) & 0xff;
	menudlg_setval(DID_WINCOLG, val);
	sprintf(work, "%u", val);
	menudlg_settext(DID_WINCOLG2, work);
	val = winrgb & 0xff;
	menudlg_setval(DID_WINCOLB, val);
	sprintf(work, "%u", val);
	menudlg_settext(DID_WINCOLB2, work);
	val = (winalpha * 100) / 64;
	menudlg_setval(DID_WINCOLE, val);
	sprintf(work, "%u", val);
	menudlg_settext(DID_WINCOLE2, work);
}


// ----

typedef struct {
	LISTARRAY	self;
	int			curpage;
	MENUID		id;
	BYTE		page;
	BYTE		group;
	int			y;
	int			sy;
	int			cr;
	int			height;
	int			cmds;
	BYTE		lastcmd;
} _GCMS, *GCMS;

#ifndef SIZE_QVGA

enum {
	GCFG_SXCFG		= 23,
	GCFG_SYCFG		= 39,
	GCFG_CXCFG		= 387,

	GCFG_SYFRAME	= 14,
	GCFG_LYFRAME	= 5,
	GCFG_PYITEM		= 3,

	GCFG_CYRADIO	= 17,
	GCFG_CYCHECK	= 19,
	GCFG_CYSLIDER	= 47,

	GCFG_SXRITEM	= 33,
	GCFG_SYRITEM	= 4,
	GCFG_CXRITEM	= 123,
	GCFG_CYRITEM	= 13
};

#else

enum {
	GCFG_SXCFG		= 16,
	GCFG_SYCFG		= 26,
	GCFG_CXCFG		= 258,

	GCFG_SYFRAME	= 11,
	GCFG_LYFRAME	= 3,
	GCFG_PYITEM		= 2,

	GCFG_CYRADIO	= 12,
	GCFG_CYCHECK	= 12,
	GCFG_CYSLIDER	= 31,

	GCFG_SXRITEM	= 22,
	GCFG_SYRITEM	= 1,
	GCFG_CXRITEM	= 82,
	GCFG_CYRITEM	= 11
};

#endif

static BOOL gcms4(void *vpItem, void *vpArg) {

	GCDLG	gcdlg = (GCDLG)vpItem;
	GCMS	gcms = (GCMS)vpArg;
	MENUFLG	flag;
	int		val;

	if ((gcdlg->page != gcms->page) || (gcms->group != gcdlg->group)) {
		goto gcms4_exit;
	}
	switch(gcdlg->cmd) {
		case CFGTYPE_RADIO:
			if (gcms->lastcmd != gcdlg->cmd) {
				gcms->lastcmd = gcdlg->cmd;
				gcms->y += gcms->cr;
				gcms->cr = GCFG_CYRADIO + GCFG_PYITEM;
				gcms->cmds = 0;
				flag = MENU_TABSTOP;
			}
			else {
				gcms->cmds++;
				if (gcms->cmds >= 3) {
					gcms->y += gcms->cr;
					gcms->cmds = 0;
				}
				flag = 0;
			}
			gcdlg->id = gcms->id;
			gcms->id++;
			menudlg_append(DLGTYPE_RADIO, gcdlg->id, flag, gcdlg->c.r.str,
									gcms->cmds * GCFG_CXRITEM + GCFG_SXRITEM,
									gcms->y + GCFG_SYRITEM,
									GCFG_CXRITEM - 1, GCFG_CYRITEM);
			if ((scr_valget(gcdlg->c.r.val, &val) == SUCCESS) &&
				(val == gcdlg->num)) {
				menudlg_setval(gcdlg->id, 1);
			}
			break;

		case CFGTYPE_CHECK:
			if (gcms->lastcmd != gcdlg->cmd) {
				gcms->lastcmd = gcdlg->cmd;
				gcms->y += gcms->cr;
				gcms->cr = GCFG_CYCHECK + GCFG_PYITEM;
				gcms->cmds = 0;
			}
			else {
				gcms->cmds++;
				if (gcms->cmds >= 3) {
					gcms->y += gcms->cr;
					gcms->cmds = 0;
				}
			}
			gcdlg->id = gcms->id;
			gcms->id++;
			menudlg_append(DLGTYPE_CHECK, gcdlg->id, MENU_TABSTOP,
								gcdlg->c.c.str,
									gcms->cmds * GCFG_CXRITEM + GCFG_SXRITEM,
									gcms->y + GCFG_SYRITEM,
									GCFG_CXRITEM - 1, GCFG_CYRITEM);
			if (scr_valget(gcdlg->c.r.val, &val) == SUCCESS) {
				menudlg_setval(gcdlg->id, val);
			}
			break;

		case CFGTYPE_SLIDER:
			gcms->lastcmd = gcdlg->cmd;
			gcms->y += gcms->cr;
			gcms->cr = GCFG_CYSLIDER + GCFG_PYITEM;
			gcdlg->id = gcms->id;
			gcms->id++;
#ifndef SIZE_QVGA
			menudlg_append(DLGTYPE_LTEXT, DID_STATIC, 0,
								gcdlg->c.s.str, 32, gcms->y + 16, 104, 13);
			menudlg_append(DLGTYPE_LTEXT, DID_STATIC, 0,
								gcdlg->c.s.min, 137, gcms->y + 1, 130, 13);
			menudlg_append(DLGTYPE_RTEXT, DID_STATIC, 0,
								gcdlg->c.s.max, 271, gcms->y + 1, 130, 13);
			menudlg_append(DLGTYPE_SLIDER, gcdlg->id, MENU_TABSTOP,
					(void *)SLIDERPOS(gcdlg->c.s.minval, gcdlg->c.s.maxval),
												145, gcms->y + 17, 247, 21);
#else
			menudlg_append(DLGTYPE_LTEXT, DID_STATIC, 0,
								gcdlg->c.s.str, 20, gcms->y + 11, 69, 11);
			menudlg_append(DLGTYPE_LTEXT, DID_STATIC, 0,
								gcdlg->c.s.min, 91, gcms->y + 0,  86, 11);
			menudlg_append(DLGTYPE_RTEXT, DID_STATIC, 0,
								gcdlg->c.s.max, 180, gcms->y + 0,  86, 11);
			menudlg_append(DLGTYPE_SLIDER, gcdlg->id, MENU_TABSTOP,
					(void *)SLIDERPOS(gcdlg->c.s.minval, gcdlg->c.s.maxval),
												96, gcms->y + 12, 164, 15);
#endif
			if (scr_valget(gcdlg->c.r.val, &val) == SUCCESS) {
				menudlg_setval(gcdlg->id, val);
			}
			break;
	}

gcms4_exit:
	return(FALSE);
}

static BOOL gcms3(void *vpItem, void *vpArg) {

	GCDLG	gcdlg = (GCDLG)vpItem;
	GCMS	gcms = (GCMS)vpArg;

	if ((gcdlg->page != gcms->page) || (gcms->group != gcdlg->group)) {
		goto gcms3_exit;
	}
	switch(gcdlg->cmd) {
		case CFGTYPE_RADIO:
			if (gcms->lastcmd != gcdlg->cmd) {
				gcms->lastcmd = gcdlg->cmd;
				gcms->height += GCFG_CYRADIO + GCFG_PYITEM;
				gcms->cmds = 0;
			}
			else {
				gcms->cmds++;
				if (gcms->cmds >= 3) {
					gcms->height += GCFG_CYRADIO + GCFG_PYITEM;
					gcms->cmds = 0;
				}
			}
			break;

		case CFGTYPE_CHECK:
			if (gcms->lastcmd != gcdlg->cmd) {
				gcms->lastcmd = gcdlg->cmd;
				gcms->height += GCFG_CYCHECK + GCFG_PYITEM;
				gcms->cmds = 0;
			}
			else {
				gcms->cmds++;
				if (gcms->cmds >= 3) {
					gcms->height += GCFG_CYCHECK + GCFG_PYITEM;
					gcms->cmds = 0;
				}
			}
			break;

		case CFGTYPE_SLIDER:
			gcms->lastcmd = gcdlg->cmd;
			gcms->height += GCFG_CYSLIDER + GCFG_PYITEM;
			break;
	}

gcms3_exit:
	return(FALSE);
}

static BOOL gcms2(void *vpItem, void *vpArg) {

	GCDLG	gcdlg = (GCDLG)vpItem;
	GCMS	gcms = (GCMS)vpArg;

	if ((gcdlg->page == gcms->page) && (gcdlg->cmd == CFGTYPE_FRAME)) {
		gcms->group = gcdlg->group;
		gcms->height = GCFG_SYFRAME - GCFG_PYITEM + GCFG_LYFRAME;
		gcms->lastcmd = CFGTYPE_TAG;
		listarray_enum(gcms->self, gcms3, gcms);
		menudlg_append(DLGTYPE_FRAME, DID_STATIC, 0, gcdlg->c.f.str,
							GCFG_SXCFG, gcms->sy, GCFG_CXCFG, gcms->height);
		gcms->y = gcms->sy + GCFG_SYFRAME;
		gcms->cr = 0;
		gcms->lastcmd = CFGTYPE_TAG;
		listarray_enum(gcms->self, gcms4, gcms);
		gcms->sy += gcms->height + 1;
	}
	return(FALSE);
}

static BOOL gcms1(void *vpItem, void *vpArg) {

	GCDLG	gcdlg = (GCDLG)vpItem;
	GCMS	gcms = (GCMS)vpArg;

	if (gcdlg->cmd == CFGTYPE_TAG) {
		gcms->page = gcdlg->page;
		gcms->curpage++;
		menudlg_setpage((MENUID)gcms->curpage);
		menudlg_itemappend(DID_TAB, gcdlg->c.t.str);
		gcms->sy = GCFG_SYCFG;
		listarray_enum(gcms->self, gcms2, gcms);
	}
	return(FALSE);
}

static int gamecfg_menuset(int curpage) {

	_GCMS	gcms;

	gcms.self = gamecore.cfglist;
	gcms.curpage = curpage;
	gcms.id = DID_USERCFG;
	listarray_enum(gcms.self, gcms1, &gcms);
	return(gcms.curpage);
}


static BOOL gcmg(void *vpItem, void *vpArg) {

	GCDLG	gcdlg = (GCDLG)vpItem;
	int		val;

	switch(gcdlg->cmd) {
		case CFGTYPE_RADIO:
			val = menudlg_getval(gcdlg->id);
			if (val) {
				scr_valset(gcdlg->c.r.val, gcdlg->num);
			}
			break;

		case CFGTYPE_CHECK:
			val = menudlg_getval(gcdlg->id);
			scr_valset(gcdlg->c.r.val, val);
			break;

		case CFGTYPE_SLIDER:
			val = menudlg_getval(gcdlg->id);
			scr_valset(gcdlg->c.r.val, val);
			break;
	}
	(void)vpArg;
	return(FALSE);
}


// ----

int cfgdlg_cmd(int msg, MENUID id) {

	GAMECFG			gc;
	int				i;
	int				val;
	int				page;
	char			work[16];
const MENUPRM		*res;
	UINT			gametype;
	int				tfile;
	ARCSTREAMARG	asa;

	gc = &gamecore.gamecfg;
	switch(msg) {
		case DLGMSG_CREATE:
			page = 0;
			menudlg_appends(res_cfg, sizeof(res_cfg)/sizeof(MENUPRM));

			page++;
			menudlg_setpage((MENUID)page);
			menudlg_itemappend(DID_TAB, (char *)str_syscfg);
			gametype = gamecore.sys.type;
			if (gametype & GAME_VOICEONLY) {
				res = res_109;
				val = sizeof(res_109)/sizeof(MENUPRM);
			}
			else if (gametype & GAME_VOICE) {
				res = res_191;
				val = sizeof(res_191)/sizeof(MENUPRM);
			}
			else {
				res = res_108;
				val = sizeof(res_108)/sizeof(MENUPRM);
			}
			menudlg_appends(res, val);

			if (gametype & GAME_HAVEALPHA) {
				page++;
				menudlg_setpage((MENUID)page);
				menudlg_itemappend(DID_TAB, (char *)str_window);
				menudlg_appends(res_199, sizeof(res_199)/sizeof(MENUPRM));
			}

			page = gamecfg_menuset(page);

			maxpage = page;
			menudlg_setval(DID_TAB, 0);
			for (i=1; i<page; i++) {
				menudlg_disppagehidden((MENUID)(i + 1), TRUE);
			}

			val = gc->msgtype;
			if ((val >= 0) && (val < 3)) {
				menudlg_setval((MENUID)(DID_TEXTMD0 + val), 1);
			}
			menudlg_setval(DID_TEXTSPD, gc->textwaittick / 5);
			menudlg_setval(DID_BGM, gc->bgm);
			menudlg_setval(DID_SE, gc->se);
			menudlg_setval(DID_VOICE, gc->voice);
			menudlg_setval(DID_BGMVOL, gc->bgmvol);
			menudlg_setval(DID_SEVOL, gc->sevol);
			menudlg_setval(DID_VOICEVOL, gc->voicevol);

			setwincol(gc->winrgb, gc->winalpha);

			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_TAB:
					val = menudlg_getval(DID_TAB);
					page = maxpage;
					for (i=0; i<page; i++) {
						menudlg_disppagehidden((MENUID)(i + 1), (i != val));
					}
					break;

				case DID_VOICE:
					val = menudlg_getval(DID_VOICE);
					if (!val) {
						soundmix_stop(SOUNDTRK_VOICE, 0);
					}
					break;

				case DID_VOICETEST:
					if (menudlg_getval(DID_VOICE)) {
						tfile = arcfile_gettestfiles(ARCTYPE_VOICE);
						if (!tfile) {
							break;
						}
						tfile = rand() % tfile;
						if (arcfile_gettestname(ARCTYPE_VOICE, tfile,
														work, sizeof(work))) {
							break;
						}
						asa.type = ARCTYPE_VOICE;
						asa.fname = work;
						soundmix_load(SOUNDTRK_VOICE, &se_stream, &asa);
						soundmix_play(SOUNDTRK_VOICE, 0, 0);
					}
					break;

				case DID_VOICEVOL:
					val = menudlg_getval(DID_VOICEVOL);
					soundmix_setgain(SOUNDTRK_VOICE, val);
					break;

				case DID_SE:
					val = menudlg_getval(DID_SE);
					sndplay_seenable(val, 500);
					break;

				case DID_SETEST:
					if (menudlg_getval(DID_SE)) {
						tfile = arcfile_gettestfiles(ARCTYPE_SE);
						if (!tfile) {
							break;
						}
						tfile = rand() % tfile;
						if (arcfile_gettestname(ARCTYPE_SE, tfile,
														work, sizeof(work))) {
							break;
						}
						asa.type = ARCTYPE_SE;
						asa.fname = work;
						soundmix_load(SOUNDTRK_SE, &se_stream, &asa);
						sndplay_seplay(0x20, 0);
					}
					break;

				case DID_SEVOL:
					val = menudlg_getval(DID_SEVOL);
					for (i=0; i<SOUNDTRK_MAXSE; i++) {
						soundmix_setgain(SOUNDTRK_SE + i, val);
					}
					break;

				case DID_BGM:
					val = menudlg_getval(DID_BGM);
					sndplay_cddaenable(val, 500);
					sndplay_waveenable(val, 500);
					break;

				case DID_BGMVOL:
					val = menudlg_getval(DID_BGMVOL);
					soundmix_setgain(SOUNDTRK_SOUND, val);
					break;

				case DID_WINCOLR:
					val = menudlg_getval(DID_WINCOLR);
					sprintf(work, "%u", val);
					menudlg_settext(DID_WINCOLR2, work);
					break;

				case DID_WINCOLG:
					val = menudlg_getval(DID_WINCOLG);
					sprintf(work, "%u", val);
					menudlg_settext(DID_WINCOLG2, work);
					break;

				case DID_WINCOLB:
					val = menudlg_getval(DID_WINCOLB);
					sprintf(work, "%u", val);
					menudlg_settext(DID_WINCOLB2, work);
					break;

				case DID_WINCOLE:
					val = menudlg_getval(DID_WINCOLE);
					sprintf(work, "%u", val);
					menudlg_settext(DID_WINCOLE2, work);
					break;

				case DID_WINCOLDEF:
					setwincol(0, 32);
					break;

				case DID_OK:
					for (i=0; i<3; i++) {
						if (menudlg_getval((MENUID)(DID_TEXTMD0 + i))) {
							gc->msgtype = i;
							break;
						}
					}
					gc->textwaittick = menudlg_getval(DID_TEXTSPD) * 5;
					gc->bgm = menudlg_getval(DID_BGM);
					gc->se = menudlg_getval(DID_SE);
					gc->voice = menudlg_getval(DID_VOICE);
					gc->bgmvol = menudlg_getval(DID_BGMVOL);
					gc->sevol = menudlg_getval(DID_SEVOL);
					gc->voicevol = menudlg_getval(DID_VOICEVOL);
					val = menudlg_getval(DID_WINCOLR) << 16;
					val |= (menudlg_getval(DID_WINCOLG) << 8);
					val |= (menudlg_getval(DID_WINCOLB));
					gc->winrgb = val;
					val = menudlg_getval(DID_WINCOLE);
					gc->winalpha = (val * 64) / 100;

					listarray_enum(gamecore.cfglist, gcmg, NULL);

					menubase_close();
					break;

				case DID_CANCEL:
					resetcfg();
					menubase_close();
					break;
			}
			break;

		case DLGMSG_CLOSE:
			resetcfg();
			menubase_close();
			break;
	}
	return(0);
}

