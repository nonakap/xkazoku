#include	"compiler.h"
#include	"gamecore.h"
#include	"sysmenu.h"
#include	"menubase.h"
#include	"sysmenu.res"
#include	"dlgsave.h"
#include	"dosio.h"
#include	"savefile.h"


static const char str_load[] = "  Load  ";
static const char str_save[] = "  Save  ";
static const BYTE str_data[] = {		// データ
		0x83,0x66,0x81,0x5b,0x83,0x5e,0};
static const BYTE str_date[] = {		// 作成日 %d年%d月%d日 %d時%d分
		0x8d,0xec,0x90,0xac,0x93,0xfa,0x20,0x25,0x64,0x94,0x4e,0x25,0x64,0x8c,
		0x8e,0x25,0x64,0x93,0xfa,0x20,0x25,0x64,0x8e,0x9e,0x25,0x64,0x95,0xaa,
		0};
static const BYTE str_date2[] = {		// %d年%d月%d日 %d時%d分
		0x25,0x64,0x94,0x4e,0x25,0x64,0x8c,0x8e,0x25,0x64,0x93,0xfa,0x20,0x25,
		0x64,0x8e,0x9e,0x25,0x64,0x95,0xaa,0};
static const BYTE str_datedef[] = {		// 作成日 ----年--月--日 --時--分
		0x8d,0xec,0x90,0xac,0x93,0xfa,0x20,0x2d,0x2d,0x2d,0x2d,0x94,0x4e,0x2d,
		0x2d,0x8c,0x8e,0x2d,0x2d,0x93,0xfa,0x20,0x2d,0x2d,0x8e,0x9e,0x2d,0x2d,
		0x95,0xaa,0};
static const char str_nodata[] = "--- No Data ---";
static const BYTE str_nodata2[] = {		// セーブデータがありません。
		0x83,0x5a,0x81,0x5b,0x83,0x75,0x83,0x66,0x81,0x5b,0x83,0x5e,0x82,0xaa,
		0x82,0xa0,0x82,0xe8,0x82,0xdc,0x82,0xb9,0x82,0xf1,0x81,0x42,0};


#ifndef SIZE_QVGA

enum {
	RES180CX	= 437,
	RES180CY	= 303,

	RES185CX	= 435,
	RES185CY	= 339,

	RES193CX	= 565,
	RES193CY	= 324,

	RES195CX	= 565,
	RES195CY	= 303
};


// セーブ数9
static const MENUPRM res_180[] = {			// 437,303
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  7,   6, 423, 264},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	 11,  36, 415, 218},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  60, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  84, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 108, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 132, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 156, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 180, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 204, 412,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 228, 412,   2},
			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 30,  43,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE1T,	0,	NULL,	112,  43, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 30,  67,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE2T,	0,	NULL,	112,  67, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 30,  91,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE3T,	0,	NULL,	112,  91, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 30, 115,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE4T,	0,	NULL,	112, 115, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 30, 139,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE5T,	0,	NULL,	112, 139, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 30, 163,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE6T,	0,	NULL,	112, 163, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 30, 187,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE7T,	0,	NULL,	112, 187, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 30, 211,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE8T,	0,	NULL,	112, 211, 304,  13},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 30, 235,  81,  13},
			{DLGTYPE_CTEXT,		DID_SAVE9T,	0,	NULL,	112, 235, 304,  13},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									140, 276,  88,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								235, 276,  88,  21},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								329, 276,  88,  21},
};


// セーブ数10, プレビュー, コメント有り
static const MENUPRM res_185[] = {			// 435,339
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  7,   6, 421, 300},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 28,  43,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE1B,	0,	NULL, 	 22,  38,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 28,  64,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE2B,	0,	NULL, 	 22,  59,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 28,  85,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE3B,	0,	NULL, 	 22,  80,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 28, 106,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE4B,	0,	NULL, 	 22, 101,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 28, 129,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE5B,	0,	NULL, 	 22, 123,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 28, 150,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE6B,	0,	NULL, 	 22, 144,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 28, 171,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE7B,	0,	NULL, 	 22, 165,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 28, 193,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE8B,	0,	NULL, 	 22, 188,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 28, 216,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE9B,	0,	NULL, 	 22, 210,  89,  24},
			{DLGTYPE_RADIO,		DID_SAVE0,	0,	NULL,	 28, 238,  80,  13},
			{DLGTYPE_BOX,		DID_SAVE0B,	0,	NULL, 	 22, 233,  89,  24},
			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	109,  38, 303, 219},
			{DLGTYPE_VRAM,		DID_VIEW,	0,	NULL, 	120,  42, 280, 210},
			{DLGTYPE_CTEXT,		DID_NODATA,	0,
										str_nodata2,	160, 141, 200,  13},
			{DLGTYPE_LTEXT,		DID_DATE,	0,	NULL, 	286, 238, 112,  12},
			{DLGTYPE_CTEXT,		DID_COMM,	0,	NULL, 	 90, 272, 256,  12},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									152, 312,  88,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								247, 312,  88,  21},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								341, 312,  88,  21},
};


// セーブ数10, コメント有り
static const MENUPRM res_193[] = {			// 565,324
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  7,   6, 551, 285},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	 11,  36, 541, 242},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  60, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  84, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 108, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 132, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 156, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 180, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 204, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 228, 538,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 252, 538,   2},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 30,  43,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE1T,	0,	NULL,	130,  43, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE1C,	0,	NULL,	283,  43, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 30,  67,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE2T,	0,	NULL,	130,  67, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE2C,	0,	NULL,	283,  67, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 30,  91,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE3T,	0,	NULL,	130,  91, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE3C,	0,	NULL,	283,  91, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 30, 115,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE4T,	0,	NULL,	130, 115, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE4C,	0,	NULL,	283, 115, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 30, 139,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE5T,	0,	NULL,	130, 139, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE5C,	0,	NULL,	283, 139, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 30, 163,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE6T,	0,	NULL,	130, 163, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE6C,	0,	NULL,	283, 163, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 30, 187,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE7T,	0,	NULL,	130, 187, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE7C,	0,	NULL,	283, 187, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 30, 211,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE8T,	0,	NULL,	130, 211, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE8C,	0,	NULL,	283, 211, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 30, 235,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE9T,	0,	NULL,	130, 235, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE9C,	0,	NULL,	283, 235, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE0,	0,	NULL,	 30, 259,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE0T,	0,	NULL,	130, 259, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE0C,	0,	NULL,	283, 259, 268,  13},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									282, 297,  88,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								377, 297,  88,  21},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								471, 297,  88,  21},
};


// セーブ数9, コメント有り
static const MENUPRM res_195[] = {			// 565,303
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  7,   6, 551, 264},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	 11,  33, 543, 221},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  60, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12,  84, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 108, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 132, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 156, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 180, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 204, 540,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	 12, 228, 540,   2},
			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 30,  43,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE1T,	0,	NULL,	130,  43, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE1C,	0,	NULL,	283,  43, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 30,  67,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE2T,	0,	NULL,	130,  67, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE2C,	0,	NULL,	283,  67, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 30,  91,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE3T,	0,	NULL,	130,  91, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE3C,	0,	NULL,	283,  91, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 30, 115,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE4T,	0,	NULL,	130, 115, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE4C,	0,	NULL,	283, 115, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 30, 139,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE5T,	0,	NULL,	130, 139, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE5C,	0,	NULL,	283, 139, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 30, 163,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE6T,	0,	NULL,	130, 163, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE6C,	0,	NULL,	283, 163, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 30, 187,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE7T,	0,	NULL,	130, 187, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE7C,	0,	NULL,	283, 187, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 30, 211,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE8T,	0,	NULL,	130, 211, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE8C,	0,	NULL,	283, 211, 268,  13},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 30, 235,  99,  13},
			{DLGTYPE_LTEXT,		DID_SAVE9T,	0,	NULL,	130, 235, 152,  13},
			{DLGTYPE_LTEXT,		DID_SAVE9C,	0,	NULL,	283, 235, 268,  13},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									282, 276,  88,  21},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								377, 276,  88,  21},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								471, 276,  88,  21},
};

#else

enum {
	RES180CX	= 291,
	RES180CY	= 202,

	RES185CX	= 290,
	RES185CY	= 218,

	RES193CX	= 308,
	RES193CY	= 218,

	RES195CX	= 308,
	RES195CY	= 202
};


// セーブ数9
static const MENUPRM res_180[] = {			// 291,202
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  5,   3, 282, 177},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	  8,  24, 276, 146},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9,  40, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9,  56, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9,  72, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9,  88, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9, 104, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9, 120, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9, 136, 273,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  9, 152, 273,   2},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 20,  28,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE1T,	0,	NULL,	 85,  28, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 20,  44,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE2T,	0,	NULL,	 85,  44, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 20,  60,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE3T,	0,	NULL,	 85,  60, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 20,  76,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE4T,	0,	NULL,	 85,  76, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 20,  92,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE5T,	0,	NULL,	 85,  92, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 20, 108,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE6T,	0,	NULL,	 85, 108, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 20, 124,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE7T,	0,	NULL,	 85, 124, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 20, 140,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE8T,	0,	NULL,	 85, 140, 182,  11},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 20, 156,  64,  11},
			{DLGTYPE_CTEXT,		DID_SAVE9T,	0,	NULL,	 85, 156, 182,  11},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									 96, 184,  58,  15},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								158, 184,  58,  15},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								220, 184,  58,  15},
};


// セーブ数10, プレビュー, コメント有り
static const MENUPRM res_185[] = {			// 290,218
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  5,   3, 281, 193},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 18,  26,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE1B,	0,	NULL, 	 15,  23,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 18,  41,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE2B,	0,	NULL, 	 15,  38,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 18,  56,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE3B,	0,	NULL, 	 15,  53,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 18,  71,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE4B,	0,	NULL, 	 15,  68,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 18,  86,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE5B,	0,	NULL, 	 15,  83,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 18, 101,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE6B,	0,	NULL, 	 15,  98,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 18, 116,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE7B,	0,	NULL, 	 15, 113,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 18, 131,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE8B,	0,	NULL, 	 15, 128,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 18, 146,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE9B,	0,	NULL, 	 15, 143,  60,  17},
			{DLGTYPE_RADIO,		DID_SAVE0,	0,	NULL,	 18, 161,  54,  11},
			{DLGTYPE_BOX,		DID_SAVE0B,	0,	NULL, 	 15, 158,  60,  17},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	 73,  23, 202, 152},
			{DLGTYPE_VRAM,		DID_VIEW,	0,	NULL, 	 80,  29, 186, 140},
			{DLGTYPE_CTEXT,		DID_NODATA,	0,
										str_nodata2,	106,  94, 134,  11},
			{DLGTYPE_LTEXT,		DID_DATE,	0,	NULL, 	154, 158, 112,  10},
			{DLGTYPE_CTEXT,		DID_COMM,	0,	NULL, 	  8, 177, 275,  11},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									104, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								166, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								228, 200,  58,  15},
};


// セーブ数10, コメント有り
static const MENUPRM res_193[] = {			// 308,218
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  4,   3, 300, 193},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	  7,  23, 294, 162},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  39, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  55, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  71, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  87, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 103, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 119, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 135, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 151, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 167, 291,   2},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 15,  27,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE1T,	0,	NULL,	 69,  27,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE1C,	0,	NULL,	148,  27, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 15,  43,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE2T,	0,	NULL,	 69,  43,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE2C,	0,	NULL,	148,  43, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 15,  59,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE3T,	0,	NULL,	 69,  59,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE3C,	0,	NULL,	148,  59, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 15,  75,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE4T,	0,	NULL,	 69,  75,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE4C,	0,	NULL,	148,  75, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 15,  91,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE5T,	0,	NULL,	 69,  91,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE5C,	0,	NULL,	148,  91, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 15, 107,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE6T,	0,	NULL,	 69, 107,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE6C,	0,	NULL,	148, 107, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 15, 123,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE7T,	0,	NULL,	 69, 123,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE7C,	0,	NULL,	148, 123, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 15, 139,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE8T,	0,	NULL,	 69, 139,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE8C,	0,	NULL,	148, 139, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 15, 155,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE9T,	0,	NULL,	 69, 155,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE9C,	0,	NULL,	148, 155, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE0,	0,	NULL,	 15, 171,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE0T,	0,	NULL,	 69, 171,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE0C,	0,	NULL,	148, 171, 150,  11},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									121, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								183, 200,  58,  15},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								245, 200,  58,  15},
};


// セーブ数9, コメント有り
static const MENUPRM res_195[] = {			// 308,202
			{DLGTYPE_TABLIST,	DID_TAB,	0,	NULL,	  4,   3, 300, 177},

			{DLGTYPE_BOX,		DID_STATIC,	0,	NULL, 	  7,  23, 294, 146},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  39, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  55, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  71, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8,  87, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 103, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 119, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 135, 291,   2},
			{DLGTYPE_LINE,		DID_STATIC,	0,	NULL,	  8, 151, 291,   2},

			{DLGTYPE_RADIO,		DID_SAVE1,	0,	NULL,	 15,  27,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE1T,	0,	NULL,	 69,  27,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE1C,	0,	NULL,	148,  27, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE2,	0,	NULL,	 15,  43,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE2T,	0,	NULL,	 69,  43,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE2C,	0,	NULL,	148,  43, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE3,	0,	NULL,	 15,  59,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE3T,	0,	NULL,	 69,  59,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE3C,	0,	NULL,	148,  59, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE4,	0,	NULL,	 15,  75,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE4T,	0,	NULL,	 69,  75,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE4C,	0,	NULL,	148,  75, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE5,	0,	NULL,	 15,  91,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE5T,	0,	NULL,	 69,  91,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE5C,	0,	NULL,	148,  91, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE6,	0,	NULL,	 15, 107,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE6T,	0,	NULL,	 69, 107,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE6C,	0,	NULL,	148, 107, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE7,	0,	NULL,	 15, 123,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE7T,	0,	NULL,	 69, 123,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE7C,	0,	NULL,	148, 123, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE8,	0,	NULL,	 15, 139,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE8T,	0,	NULL,	 69, 139,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE8C,	0,	NULL,	148, 139, 150,  11},
			{DLGTYPE_RADIO,		DID_SAVE9,	0,	NULL,	 15, 155,  53,  11},
			{DLGTYPE_LTEXT,		DID_SAVE9T,	0,	NULL,	 69, 155,  78,  11},
			{DLGTYPE_LTEXT,		DID_SAVE9C,	0,	NULL,	148, 155, 150,  11},

			{DLGTYPE_BUTTON,	DID_OK,			MENU_TABSTOP,
				str_ok,									121, 184,  58,  15},
			{DLGTYPE_BUTTON,	DID_CANCEL,		MENU_TABSTOP,
				str_cancel,								183, 184,  58,  15},
			{DLGTYPE_BUTTON,	DID_APPLY,		MENU_TABSTOP | MENU_GRAY,
				str_apply,								245, 184,  58,  15},
};

#endif

// ----

typedef struct {
	int		width;
	int		height;
	int		(*load)(int msg, MENUID id);
	int		(*save)(int msg, MENUID id);
} SAVEDLG;


static int sysdlg_getpos(int max) {

	int		i;

	for (i=0; i<max; i++) {
		if (menudlg_getval((MENUID)(DID_SAVE1 + i))) {
			return(i + 1);
		}
	}
	return(0);
}


// ---- 通常

static void dlgnor_setpage(BOOL save, int pagemax, int page) {

	int			num;
	int			i;
	char		work[64];
	SAVEHDL		fh;
	BOOL		r;
	int			check;
	SAVEINF_T	inf;

	num = page * pagemax;
	check = 0;
	fh = savefile_open(FALSE);
	for (i=0; i<pagemax; i++) {
		r = savefile_readinf(fh, num, &inf, 0, 0);
		milstr_ncpy(work, (char *)str_data, sizeof(work));
		num++;
		if (num < 10) {
			work[6] = (char)0x82;
			work[7] = (char)(0x4f + num);
		}
		else {
			work[6] = (char)((num / 10) + 0x30);
			work[7] = (char)((num % 10) + 0x30);
		}
		work[8] = '\0';
		menudlg_settext((MENUID)(DID_SAVE1 + i), work);
		if (r == SUCCESS) {
			sprintf(work, (char *)str_date,
								inf.date.year, inf.date.month, inf.date.day,
								inf.date.hour, inf.date.min);
			menudlg_settext((MENUID)(DID_SAVE1T + i), work);
			menudlg_setenable((MENUID)(DID_SAVE1 + i), TRUE);
			menudlg_setenable((MENUID)(DID_SAVE1T + i), TRUE);
			if (!check) {
				check = i + 1;
			}
			vram_destroy((VRAMHDL)inf.preview);
		}
		else {
			menudlg_settext((MENUID)(DID_SAVE1T + i), (char *)str_datedef);
			menudlg_setenable((MENUID)(DID_SAVE1 + i), save);
			menudlg_setenable((MENUID)(DID_SAVE1T + i), FALSE);
		}
	}
	if (check) {
		check--;
	}
	menudlg_setval((MENUID)(DID_SAVE1 + check), 1);
	savefile_close(fh);
}


static int dlgmyu_cmd(int msg, MENUID id, BOOL save, int pagemax) {

	int		val;
const char	*tab;
	SAVEHDL	sh;
	BOOL	sproc;

	switch(msg) {
		case DLGMSG_CREATE:
			tab = (gamecore.gamecfg.enablesave)?str_save:str_load;
			menudlg_appends(res_180, sizeof(res_180)/sizeof(MENUPRM));
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, str_load);
			menudlg_setval(DID_TAB, save?0:1);
			dlgnor_setpage(save, pagemax, 0);
			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_OK:
					val = menudlg_getval(DID_TAB);
					sproc = FALSE;
					if (!val) {
						sproc = gamecore.gamecfg.enablesave?TRUE:FALSE;
					}
					val = sysdlg_getpos(pagemax);
					if (val) {
						val--;
						sh = savefile_open(sproc);
						if (sproc) {
							savefile_writegame(sh, val, -1);
						}
						else {
							savefile_readgame(sh, val);
						}
						savefile_close(sh);
					}
					menubase_close();
					break;

				case DID_CANCEL:
					menubase_close();
					break;

				case DID_TAB:
					val = menudlg_getval(DID_TAB);
					sproc = FALSE;
					if (!val) {
						sproc = gamecore.gamecfg.enablesave?TRUE:FALSE;
					}
					dlgnor_setpage(sproc, pagemax, 0);
					break;
			}
			break;

		case DLGMSG_CLOSE:
			menubase_close();
			break;
	}
	return(0);
}


static int dlgmyu_load(int msg, MENUID id) {

	return(dlgmyu_cmd(msg, id, FALSE, 9));
}

static int dlgmyu_save(int msg, MENUID id) {

	return(dlgmyu_cmd(msg, id, TRUE, 9));
}

static const SAVEDLG dlgmyu = {RES180CX, RES180CY, dlgmyu_load, dlgmyu_save};


static int dlgnor_cmd(int msg, MENUID id, BOOL save, int pagemax) {

	int		val;
const char	*tab;
	SAVEHDL	sh;

	switch(msg) {
		case DLGMSG_CREATE:
			tab = (save)?str_save:str_load;
			menudlg_appends(res_180, sizeof(res_180)/sizeof(MENUPRM));
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_setval(DID_TAB, 0);
			dlgnor_setpage(save, pagemax, 0);
			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_OK:
					val = sysdlg_getpos(pagemax);
					if (val) {
						val--;
						val += menudlg_getval(DID_TAB) * pagemax;
						sh = savefile_open(save);
						if (save) {
							savefile_writegame(sh, val, -1);
						}
						else {
							savefile_readgame(sh, val);
						}
						savefile_close(sh);
					}
					menubase_close();
					break;

				case DID_CANCEL:
					menubase_close();
					break;

				case DID_TAB:
					val = menudlg_getval(DID_TAB);
					dlgnor_setpage(save, pagemax, val);
					break;
			}
			break;

		case DLGMSG_CLOSE:
			menubase_close();
			break;
	}
	return(0);
}


static int dlg180_load(int msg, MENUID id) {

	return(dlgnor_cmd(msg, id, FALSE, 9));
}

static int dlg180_save(int msg, MENUID id) {

	return(dlgnor_cmd(msg, id, TRUE, 9));
}

static const SAVEDLG dlg180 = {RES180CX, RES180CY, dlg180_load, dlg180_save};


// ---- プレピュー, コメント有り

static void dlgpc_setitem(int pagemax, int page, int item) {

	int			i;
	SAVEHDL		fh;
	SAVEINF_T	inf;
	BOOL		r;
	char		work[64];

	for (i=0; i<pagemax; i++) {
		menudlg_sethide((MENUID)(DID_SAVE1B + i), TRUE);
	}
	menudlg_setval((MENUID)(DID_SAVE1 + item), 1);
	menudlg_sethide((MENUID)(DID_SAVE1B + item), FALSE);

	item += (pagemax * page);
	fh = savefile_open(FALSE);
#ifndef SIZE_QVGA
	r = savefile_readinf(fh, item, &inf, 280, 210);
#else
	r = savefile_readinf(fh, item, &inf, 186, 140);
#endif
	if (r == SUCCESS) {
		vram_destroy(menudlg_setvram(DID_VIEW, (VRAMHDL)inf.preview));
		menudlg_sethide(DID_NODATA, TRUE);
		sprintf(work, (char *)str_date2,
							inf.date.year, inf.date.month, inf.date.day,
							inf.date.hour, inf.date.min);
		menudlg_settext(DID_DATE, work);
		menudlg_settext(DID_COMM, inf.comment);
	}
	else {
		vram_destroy(menudlg_setvram(DID_VIEW, NULL));
		menudlg_sethide(DID_NODATA, FALSE);
		menudlg_settext(DID_DATE, NULL);
		menudlg_settext(DID_COMM, NULL);
	}
	savefile_close(fh);
}

static void dlgpc_setpage(BOOL save, int pagemax, int page) {

	int			num;
	int			i;
	char		work[64];
	SAVEHDL		fh;
	BOOL		r;
	int			check;

	num = page * pagemax;
	check = 0;
	fh = savefile_open(FALSE);
	for (i=0; i<pagemax; i++) {
		r = savefile_exist(fh, num);
		milstr_ncpy(work, (char *)str_data, sizeof(work));
		num++;
		if (num < 10) {
			work[6] = (char)0x82;
			work[7] = (char)(0x4f + num);
		}
		else {
			work[6] = (char)((num / 10) + 0x30);
			work[7] = (char)((num % 10) + 0x30);
		}
		work[8] = '\0';
		menudlg_settext((MENUID)(DID_SAVE1 + i), work);
		if (r == TRUE) {
			if (!check) {
				check = i + 1;
			}
			menudlg_setenable((MENUID)(DID_SAVE1 + i), TRUE);
		}
		else {
			menudlg_setenable((MENUID)(DID_SAVE1 + i), save);
		}
	}
	savefile_close(fh);
	if (check) {
		check--;
	}
	dlgpc_setitem(pagemax, page, check);
}

static int dlgpc_cmd(int msg, MENUID id, BOOL save, int pagemax) {

	int		val;
const char	*tab;
	SAVEHDL	sh;

	switch(msg) {
		case DLGMSG_CREATE:
			tab = (save)?str_save:str_load;
			menudlg_appends(res_185, sizeof(res_185)/sizeof(MENUPRM));
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_setval(DID_TAB, 0);
			dlgpc_setpage(save, pagemax, 0);
			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_OK:
					val = sysdlg_getpos(pagemax);
					if (val) {
						val--;
						val += menudlg_getval(DID_TAB) * pagemax;
						sh = savefile_open(save);
						if (save) {
							savefile_writegame(sh, val, -1);
						}
						else {
							savefile_readgame(sh, val);
						}
						savefile_close(sh);
					}
					vram_destroy(menudlg_setvram(DID_VIEW, NULL));
					menubase_close();
					break;

				case DID_CANCEL:
					vram_destroy(menudlg_setvram(DID_VIEW, NULL));
					menubase_close();
					break;

				case DID_TAB:
					val = menudlg_getval(DID_TAB);
					dlgpc_setpage(save, pagemax, val);
					break;

				case DID_SAVE1:
				case DID_SAVE2:
				case DID_SAVE3:
				case DID_SAVE4:
				case DID_SAVE5:
				case DID_SAVE6:
				case DID_SAVE7:
				case DID_SAVE8:
				case DID_SAVE9:
				case DID_SAVE0:
					val = menudlg_getval(DID_TAB);
					dlgpc_setitem(pagemax, val, id - DID_SAVE1);
					break;
			}
			break;

		case DLGMSG_CLOSE:
			vram_destroy(menudlg_setvram(DID_VIEW, NULL));
			menubase_close();
			break;
	}
	return(0);
}


static int dlg185_load(int msg, MENUID id) {

	return(dlgpc_cmd(msg, id, FALSE, 10));
}

static int dlg185_save(int msg, MENUID id) {

	return(dlgpc_cmd(msg, id, TRUE, 10));
}

static const SAVEDLG dlg185 = {RES185CX, RES185CY, dlg185_load, dlg185_save};


// ---- コメント有り

static void dlgcom_setpage(BOOL save, int pagemax, int page) {

	int			num;
	int			i;
	char		work[64];
	SAVEHDL		fh;
	BOOL		r;
	int			check;
	SAVEINF_T	inf;

	num = page * pagemax;
	check = 0;
	fh = savefile_open(FALSE);
	for (i=0; i<pagemax; i++) {
		r = savefile_readinf(fh, num, &inf, 0, 0);
		milstr_ncpy(work, (char *)str_data, sizeof(work));
		num++;
		if (num < 10) {
			work[6] = (char)0x82;
			work[7] = (char)(0x4f + num);
		}
		else {
			work[6] = (char)((num / 10) + 0x30);
			work[7] = (char)((num % 10) + 0x30);
		}
		work[8] = '\0';
		menudlg_settext((MENUID)(DID_SAVE1 + i), work);
		if (r == SUCCESS) {
			sprintf(work, "%4d/%2d/%2d %02d:%02d",
							inf.date.year, inf.date.month, inf.date.day,
							inf.date.hour, inf.date.min);
			menudlg_settext((MENUID)(DID_SAVE1T + i), work);
			menudlg_settext((MENUID)(DID_SAVE1C + i), inf.comment);
			menudlg_setenable((MENUID)(DID_SAVE1 + i), TRUE);
			menudlg_setenable((MENUID)(DID_SAVE1T + i), TRUE);
			menudlg_setenable((MENUID)(DID_SAVE1C + i), TRUE);
			if (!check) {
				check = i + 1;
			}
			vram_destroy((VRAMHDL)inf.preview);
		}
		else {
			menudlg_settext((MENUID)(DID_SAVE1T + i), "0000/00/00 00:00");
			menudlg_settext((MENUID)(DID_SAVE1C + i), str_nodata);
			menudlg_setenable((MENUID)(DID_SAVE1 + i), save);
			menudlg_setenable((MENUID)(DID_SAVE1T + i), FALSE);
			menudlg_setenable((MENUID)(DID_SAVE1C + i), FALSE);
		}
	}
	if (check) {
		check--;
	}
	menudlg_setval((MENUID)(DID_SAVE1 + check), 1);
	savefile_close(fh);
}

static int dlgcom_cmd(int msg, MENUID id, BOOL save, int pagemax) {

	int		val;
const char	*tab;
	SAVEHDL	sh;

	switch(msg) {
		case DLGMSG_CREATE:
			tab = (save)?str_save:str_load;
			if (pagemax == 9) {
				menudlg_appends(res_195, sizeof(res_195)/sizeof(MENUPRM));
			}
			else {
				menudlg_appends(res_193, sizeof(res_193)/sizeof(MENUPRM));
			}
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_itemappend(DID_TAB, tab);
			menudlg_setval(DID_TAB, 0);
			dlgcom_setpage(save, pagemax, 0);
			break;

		case DLGMSG_COMMAND:
			switch(id) {
				case DID_OK:
					val = sysdlg_getpos(pagemax);
					if (val) {
						val--;
						val += menudlg_getval(DID_TAB) * pagemax;
						sh = savefile_open(save);
						if (save) {
							savefile_writegame(sh, val, -1);
						}
						else {
							savefile_readgame(sh, val);
						}
						savefile_close(sh);
					}
					menubase_close();
					break;

				case DID_CANCEL:
					menubase_close();
					break;

				case DID_TAB:
					val = menudlg_getval(DID_TAB);
					dlgcom_setpage(save, pagemax, val);
					break;
			}
			break;

		case DLGMSG_CLOSE:
			menubase_close();
			break;
	}
	return(0);
}


static int dlg193_load(int msg, MENUID id) {

	return(dlgcom_cmd(msg, id, FALSE, 10));
}

static int dlg193_save(int msg, MENUID id) {

	return(dlgcom_cmd(msg, id, TRUE, 10));
}

static const SAVEDLG dlg193 = {RES193CX, RES193CY, dlg193_load, dlg193_save};


static int dlg195_load(int msg, MENUID id) {

	return(dlgcom_cmd(msg, id, FALSE, 9));
}

static int dlg195_save(int msg, MENUID id) {

	return(dlgcom_cmd(msg, id, TRUE, 9));
}

static const SAVEDLG dlg195 = {RES195CX, RES195CY, dlg195_load, dlg195_save};


// ----

static const SAVEDLG *getdlg(void) {

const SAVEDLG	*ret;
	UINT		gametype;

	gametype = gamecore.sys.type;
	switch(gametype & (GAME_SAVEMASK | GAME_SAVEGRPH | GAME_SAVECOM)) {
		case GAME_SAVEMYU:
			ret = &dlgmyu;
			break;

		case GAME_SAVEMAX27:
			ret = &dlg180;
			break;

		case GAME_SAVEMAX27 | GAME_SAVECOM:
			ret = &dlg195;
			break;

		case GAME_SAVEMAX30 | GAME_SAVECOM:
			ret = &dlg193;
			break;

		case GAME_SAVEMAX30 | GAME_SAVEGRPH | GAME_SAVECOM:
			ret = &dlg185;
			break;

		default:
			ret = &dlg195;
			break;
	}
	return(ret);
}


BOOL dlgsave_save(void) {

const SAVEDLG *dlg;

	dlg = getdlg();
	if (dlg) {
		return(menudlg_create(dlg->width, dlg->height,
									(char *)str_data, dlg->save));
	}
	return(FAILURE);
}

BOOL dlgsave_load(void) {

const SAVEDLG *dlg;

	dlg = getdlg();
	if (dlg) {
		return(menudlg_create(dlg->width, dlg->height,
									(char *)str_data, dlg->load));
	}
	return(FAILURE);
}

