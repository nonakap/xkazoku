#include	"compiler.h"
#include	"gamecore.h"
#include	"taskmng.h"
#include	"fontmng.h"
#include	"sysmenu.h"
#include	"menubase.h"
#include	"sysmenu.res"
#include	"dlgcfg.h"
#include	"dlgsave.h"
#include	"scrnmng.h"

	char	str_ok[] = "OK";
	BYTE	str_cancel[] = {		// ����󥻥�
					0x83,0x4c,0x83,0x83,0x83,0x93,0x83,0x5a,0x83,0x8b,0x00};
	BYTE	str_apply[] = {			// Ŭ��
					0x93,0x4b,0x97,0x70,0x00};

// ----

static const BYTE str_winmd0[] = {	// ������ɥ��⡼��
		0x83,0x45,0x83,0x42,0x83,0x93,0x83,0x68,0x83,0x45,0x83,0x82,0x81,0x5b,
		0x83,0x68,0x00};
static const BYTE str_winmd1[] = {	// �ե륹���꡼��⡼��
		0x83,0x74,0x83,0x8b,0x83,0x58,0x83,0x4e,0x83,0x8a,0x81,0x5b,0x83,0x93,
		0x83,0x82,0x81,0x5b,0x83,0x68,0x00};
static const BYTE str_windel[] = {	// ������ɥ��ξõ�
		0x83,0x45,0x83,0x42,0x83,0x93,0x83,0x68,0x83,0x45,0x82,0xcc,0x8f,0xc1,
		0x8b,0x8e,0x00};
static const BYTE str_prop[] = {	// �ץ�ѥƥ�
		0x83,0x76,0x83,0x8d,0x83,0x70,0x83,0x65,0x83,0x42,0x00};
static const BYTE str_skip[] = {	// ��å����������å�
		0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,0x83,0x58,0x83,0x4c,
		0x83,0x62,0x83,0x76,0x00};
static const BYTE str_read[] = {	// ���ɥ�å����������å�
		0x8a,0xf9,0x93,0xc7,0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,
		0x83,0x58,0x83,0x4c,0x83,0x62,0x83,0x76,0x00};
static const BYTE str_auto[] = {	// �����ȥ���å�
		0x83,0x49,0x81,0x5b,0x83,0x67,0x83,0x4e,0x83,0x8a,0x83,0x62,0x83,0x4e,
		0x00};
static const BYTE str_auto2[] = {	// ��ư����⡼�ɲ��
		0x83,0x49,0x81,0x5b,0x83,0x67,0x83,0x4e,0x83,0x8a,0x83,0x62,0x83,0x4e,
		0x89,0xf0,0x8f,0x9c,0x00};

static const BYTE str_log[] = {		// ���ɥ�å������κ�ɽ��
		0x8a,0xf9,0x93,0xc7,0x83,0x81,0x83,0x62,0x83,0x5a,0x81,0x5b,0x83,0x57,
		0x82,0xcc,0x8d,0xc4,0x95,0x5c,0x8e,0xa6,0x00};
static const BYTE str_title[] = {	// �����ȥ�����
		0x83,0x5e,0x83,0x43,0x83,0x67,0x83,0x8b,0x82,0xc9,0x96,0xdf,0x82,0xe9,
		0x00};
static const BYTE str_exit[] = {	// �����ཪλ
		0x83,0x51,0x81,0x5b,0x83,0x80,0x8f,0x49,0x97,0xb9,0x00};

static const BYTE str_titler[] = {	// �����ȥ�����ޤ���������Ǥ�����
		0x83,0x5e,0x83,0x43,0x83,0x67,0x83,0x8b,0x82,0xc9,0x96,0xdf,0x82,0xe8,
		0x82,0xdc,0x82,0xb7,0x81,0x42,0x82,0xe6,0x82,0xeb,0x82,0xb5,0x82,0xa2,
		0x82,0xc5,0x82,0xb7,0x82,0xa9,0x81,0x48,0x00};

static const BYTE str_exitr[] = {	// �������λ���ޤ���������Ǥ�����
		0x83,0x51,0x81,0x5b,0x83,0x80,0x82,0xf0,0x8f,0x49,0x97,0xb9,0x82,0xb5,
		0x82,0xdc,0x82,0xb7,0x81,0x42,0x82,0xe6,0x82,0xeb,0x82,0xb5,0x82,0xa2,
		0x82,0xc5,0x82,0xb7,0x82,0xa9,0x81,0x48,0x00};


static const MSYSITEM s_popup[] = {
		{(char *)str_winmd1,	NULL,		MID_SCREEN,		MENU_GRAY},
		{(char *)str_windel,	NULL,		MID_TEXT,		MENU_GRAY},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_prop,		NULL,		MID_CONFIG,		0},
		{"SAVE",				NULL,		MID_SAVE,		0},
		{"LOAD",				NULL,		MID_LOAD,		0},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_skip,		NULL,		MID_SKIP,		0},
		{(char *)str_read,		NULL,		MID_READSKIP,	0},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_auto,		NULL,		MID_AUTO,		0},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_log,		NULL,		MID_LOG,		MENU_GRAY},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{"HELP",				NULL,		MID_HELP,		MENU_GRAY},
		{NULL,					NULL,		MID_STATIC,		MENU_SEPARATOR},
		{(char *)str_title,		NULL,		MID_TITLE,		0},
		{(char *)str_exit,		NULL,		MID_EXIT,		MENU_DELETED}};

static const MSYSITEM s_main[] = {
		{"POPUP",				s_popup,	MID_STATIC,		MENUS_POPUP
															| MENU_DELETED}};


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


BOOL sysmenu_menuopen(void) {

	GAMECFG	gc;

	gc = &gamecore.gamecfg;
#ifdef SUPPORT_FULLSCREEN
	menusys_setenable(MID_SCREEN, TRUE);
	menusys_settext(MID_SCREEN,
							(char *)(gc->fullscreen?str_winmd0:str_winmd1));
#endif
	menusys_setenable(MID_SAVE, gc->enablesave);
	menusys_setcheck(MID_SKIP, gc->skip);
	menusys_setcheck(MID_READSKIP, gc->readskip);
	menusys_settext(MID_AUTO, (char *)(gc->autoclick?str_auto2:str_auto));
	return(menusys_open());
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
#ifndef SIZE_QVGA
			menudlg_create(433, 328, (char *)str_prop, cfgdlg_cmd);
#else
			menudlg_create(288, 216, (char *)str_prop, cfgdlg_cmd);
#endif
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
			if (r != SUCCESS) {
				dlgsave_load();
			}
			break;

		case MID_SAVE:
			r = scr_scriptcall("SAVESYS");
			if (r != SUCCESS) {
				dlgsave_save();
			}
			break;

		case MID_TITLE:
			if (menumbox((char *)str_titler, gamecore.suf.key,
								MBOX_YESNO | MBOX_ICONQUESTION) == DID_YES) {
				if (scr_restart("TITLE") != SUCCESS) {
					scr_restart("MAIN");			// �ץ饤�١��ȥʡ���
				}
			}
			break;

		case MID_EXIT:
			if (menumbox((char *)str_exitr, gamecore.suf.key,
								MBOX_YESNO | MBOX_ICONQUESTION) == DID_YES) {
				taskmng_exit();
			}
			break;

		case SID_CLOSE:
			taskmng_exit();
			break;
	}
}

