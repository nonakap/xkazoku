#include	"compiler.h"
#include	"fontmng.h"
#include	"inputmng.h"
#include	"taskmng.h"
#include	"gamecore.h"
#include	"sound.h"


static BOOL cmdwin_exec(TEXTWIN textwin, int x, int y, UINT btn, UINT key) {

	VRAMHDL	vram;
	int		i;
	int		curval;
	CMD_T	*cmd;
	int		size;
	CHO_T	*cho;
	BOOL	r;
	DISPWIN	dispwin;

	r = FALSE;
	curval = textwin->cmdfocus;
	vram = textwin->cmdvram;
	if (textwin->cmdtype == 1) {
		if (vram == NULL) {
			goto cwexec_exit;
		}
		if (btn & MOUSE_MOVEBIT) {
			x -= vram->posx;
			y -= vram->posy;
			cmd = textwin->cmd;
			size = vram->height;
			curval = -1;
			for (i=0; i<textwin->cmdmax; i++, cmd++) {
				if ((cmd->pt.x <= x) && ((cmd->pt.x + size) >= x) &&
					(cmd->pt.y <= y) && ((cmd->pt.y + size) >= y)) {
					curval = i;
					break;
				}
			}
			if (textwin->cmdfocus != curval) {
				event_cmdwindraw(textwin, textwin->cmdfocus, 2);
				event_cmdwindraw(textwin, curval, 0);
				textwin->cmdfocus = curval;
				r = TRUE;
			}
		}
		if (btn & LBUTTON_DOWNBIT) {
			textwin->cmdret = curval;
		}
	}
	else if (textwin->cmdtype == 2) {
		if (vram == NULL) {
			vram = textwin->textctrl.vram;
		}
		if (vram == NULL) {
			goto cwexec_exit;
		}

		if (btn & MOUSE_MOVEBIT) {
			x -= vram->posx;
			y -= vram->posy;
			if (gamecore.sys.type & GAME_CMDWINNOBG) {
				dispwin = &gamecore.dispwin;
				if (dispwin->flag & DISPWIN_CLIPTEXT) {
#ifndef SIZE_QVGA
					x -= dispwin->txtclip.left;
					y -= dispwin->txtclip.top;
#else
					x -= vramdraw_half(dispwin->txtclip.left);
					y -= vramdraw_half(dispwin->txtclip.top);
#endif
				}
			}
			cho = textwin->cho;
			curval = -1;
			for (i=0; i<textwin->cmdmax; i++, cho++) {
				if (rect_in(&cho->rct, x, y)) {
					curval = i;
					break;
				}
			}
		}
		if (key & KEY_UP) {
			curval--;
			if (curval < 0) {
				curval = textwin->cmdmax - 1;
			}
		}
		if (key & KEY_DOWN) {
			curval++;
			if (curval >= textwin->cmdmax) {
				curval = 0;
			}
		}
		if (textwin->cmdfocus != curval) {
			event_choicewindraw(textwin, textwin->cmdfocus, 0);
			event_choicewindraw(textwin, curval, 1);
			textwin->cmdfocus = curval;
			r = TRUE;
		}
		if ((btn & LBUTTON_UPBIT) || (key & KEY_ENTER)) {
			textwin->cmdret = curval;
		}
	}

cwexec_exit:
	return(r);
}

static void event_exec(void) {

	EVTHDL	evthdl;
	int		x;
	int		y;
	UINT	btn;
	UINT	key;
	UINT	keydown;
	int		i;
	TEXTWIN	textwin;
	BOOL	r;

	evthdl = &gamecore.evthdl;
	btn = inputmng_getmouse(&x, &y);
	inputmng_resetmouse(LBUTTON_BIT | RBUTTON_BIT);
	key = inputmng_getkey();

	if (btn & MOUSE_MOVEBIT) {
		evthdl->x = x;
		evthdl->y = y;
	}
	evthdl->btn &= ~(LBUTTON_BIT | RBUTTON_BIT);
	evthdl->btn |= btn;
	keydown = (~evthdl->key) & key;
	evthdl->key = key;

	// 音声停止処理～
	if ((btn & LBUTTON_DOWNBIT) || (key & (KEY_SKIP | KEY_ENTER))) {
		if (soundmix_isplaying(SOUNDTRK_VOICE)) {
			soundmix_stop(SOUNDTRK_VOICE, 0);
		}
	}

	// コマンドウィンドウ処理～
	r = FALSE;
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if ((textwin) &&
			(textwin->flag & (TEXTWIN_CMDCAP | TEXTWIN_CMDCAPEX))) {
			r |= cmdwin_exec(textwin, x, y, btn, keydown);
		}
	}
	if (r) {
		vramdraw_draw();
	}
}

void event_setmouse(int x, int y) {

	EVTHDL	evthdl;

	evthdl = &gamecore.evthdl;
	evthdl->x = x;
	evthdl->y = y;
	evthdl->btn |= MOUSE_MOVEBIT;
}

UINT event_getmouse(int *x, int *y) {

	EVTHDL	evthdl;

	event_exec();
	evthdl = &gamecore.evthdl;
	if (x) {
		*x = evthdl->x;
	}
	if (y) {
		*y = evthdl->y;
	}
	return(evthdl->btn);
}

void event_resetmouse(UINT mask) {

	EVTHDL	evthdl;

	evthdl = &gamecore.evthdl;
	evthdl->btn &= mask;
	inputmng_resetmouse(mask);
}

UINT event_getkey(void) {

	EVTHDL	evthdl;

	event_exec();
	evthdl = &gamecore.evthdl;
	return(evthdl->key);
}

void event_resetkey(UINT mask) {

	EVTHDL	evthdl;

	evthdl = &gamecore.evthdl;
	evthdl->key &= mask;
	inputmng_resetkey(mask);
}

void event_resetall(void) {

	event_resetmouse(LBUTTON_BIT | RBUTTON_BIT);
	event_resetkey(KEY_SKIP);
}


// ----

void event_cmdwindraw(TEXTWIN textwin, int num, int method) {

	CMD_T		*cmd;
	POINT_T		pt;
	RECT_T		rct;
	int			size;

	if ((num < 0) || (num >= textwin->cmdmax)) {
		return;
	}
	size = textwin->cmdvram->height;
	cmd = textwin->cmd + num;
	rct.left = method * size;
	rct.top = cmd->num * size;
	rct.right = rct.left + size;
	rct.bottom = rct.top + size;
	pt.x = cmd->pt.x;
	pt.y = cmd->pt.y;
	vramcpy_cpy(textwin->cmdvram, textwin->cmdicon, &pt, &rct);
	vramdraw_setrect(textwin->cmdvram, &rct);
}


void event_choicewindraw(TEXTWIN textwin, int num, int method) {

	CHO_T		*cho;
	VRAMHDL		dst;
	RECT_T		rct;
	POINT_T		pt;
	POINT_T		pfnt;
	TEXTCTRL	textctrl;
	DISPWIN		dispwin;
	BOOL		fntgetsize;
	int			pos;

	if ((num < 0) || (num >= textwin->cmdmax)) {
		return;
	}
	cho = textwin->cho + num;
	textctrl = &textwin->textctrl;
	textctrl_renewal(textctrl);
	dst = textwin->cmdvram;
	if (dst == NULL) {
		dst = textwin->textctrl.vram;
	}
	rct = cho->rct;
	fntgetsize = fontmng_getsize(textctrl->font, cho->str, &pfnt);
	if ((fntgetsize == SUCCESS) && (gamecore.sys.type & GAME_ENGSTYLE)) {
		// フォントの差を吸収
		pos = pfnt.x - (rct.right - rct.left);
		if (pos > 0) {
			rct.right += pos;
		}
#if 0
		pos = pfnt.y - (rct.bottom - rct.top);
		if (pos > 0) {
			rct.bottom += pos;
		}
#endif
	}
	if (gamecore.sys.version >= EXE_VER1) {
		if (method) {
			method = 3;
		}
		if (gamecore.sys.type & GAME_CMDWINNOBG) {
			dispwin = &gamecore.dispwin;
			if (dispwin->flag & DISPWIN_CLIPTEXT) {
#ifndef SIZE_QVGA
				rct.left += dispwin->txtclip.left;
				rct.top += dispwin->txtclip.top;
				rct.right += dispwin->txtclip.left;
				rct.bottom += dispwin->txtclip.top;
#else
				pt.x = dispwin->txtclip.left;
				pt.y = dispwin->txtclip.top;
				vramdraw_halfpoint(&pt);
				rct.left += pt.x;
				rct.top += pt.y;
				rct.right += pt.x;
				rct.bottom += pt.y;
#endif
			}
			vram_zerofill(dst, &rct);
		}
		else if (textwin->chocolor[method+2] == 0) {
			vram_zerofill(dst, &rct);
		}
		else {
			vram_fill(dst, &rct, textwin->chocolor[method+2], 0xff);
		}
		if (textctrl->fonttype & TEXTCTRL_SHADOW) {
			pt.x = rct.left + 1;
			pt.y = rct.top + 1;
			vrammix_textex(dst, textctrl->font, cho->str,
								textwin->chocolor[method+1], &pt, &rct);
		}
		pt.x = rct.left;
		pt.y = rct.top;
		vrammix_textex(dst, textctrl->font, cho->str,
								textwin->chocolor[method+0], &pt, &rct);
	}
	else {
		vram_zerofill(dst, &rct);
		pt.x = rct.left;
		pt.y = rct.top;
		if ((fntgetsize == SUCCESS) && (!(gamecore.sys.type & GAME_DRS))) {
			pt.x += (rct.right - rct.left - pfnt.x) / 2;
			pt.y += (rct.bottom - rct.top - pfnt.y) / 2;
		}
		vrammix_textex(dst, textctrl->font, cho->str,
								textwin->chocolor[method], &pt, &rct);
	}
	vramdraw_setrect(dst, &rct);
}


// ----

int event_getcmdwin(TEXTWIN textwin) {

	int		r;

	event_exec();					// 本当は常に回すべし
	r = textwin->cmdret;
	textwin->cmdret = -1;
	return(r);
}

int event_cmdwin(void) {

	CWEVT	cwevt;
	TEXTWIN textwin;
	int		r;

	event_exec();
	cwevt = &gamecore.cwevt;
	textwin = textwin_getwin(cwevt->num);
	if (textwin == NULL) {
		return(GAMEEV_FAILURE);
	}
	r = textwin->cmdret;
	if (r >= 0) {
		textwin->cmdret = -1;
		scr_valset(cwevt->val, r);
		return(GAMEEV_SUCCESS);
	}
	taskmng_sleep(10);
	return(GAMEEV_CMDWIN);
}


// ---- voice event

int event_waitpcmend(void) {

	event_exec();
	if (soundmix_isplaying(SOUNDTRK_VOICE)) {
		taskmng_sleep(10);
		return(GAMEEV_WAITPCMEND);
	}
	return(GAMEEV_SUCCESS);
}


// ---- mouse event

static const UINT keymov[] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT};


void event_mousereset(void) {

	MOUSEEVT	mouseevt;

	mouseevt = &gamecore.mouseevt;
	mouseevt->pos = -1;
	mouseevt->lastpos = -1;
	mouseevt->keypos = -1;
	mouseevt->flag |= MEVT_RENEWAL;
}


void event_mouserel(int rel) {

	MOUSEEVT	mouseevt;

	mouseevt = &gamecore.mouseevt;
	if (rel) {
		mouseevt->flag |= MEVT_RELMODE;
	}
	else {
		mouseevt->flag &= ~MEVT_RELMODE;
	}
	event_resetmouse(LBUTTON_BIT | RBUTTON_BIT);
	event_resetkey(~KEY_ENTER);
}


static BOOL rgnenable(REGIONPRM *rgn) {

	BYTE	bit;
	int		val;

	if (!rgn->kind) {
		if ((scr_flagget(rgn->num, &bit) != SUCCESS) || (!bit)) {
			return(FALSE);
		}
	}
	else {
		if ((scr_valget(rgn->num, &val) != SUCCESS) || (!val)) {
			return(FALSE);
		}
	}
	return(TRUE);
}


int event_mouse(void) {

	int			x, y;
	UINT		btn, key;
	MOUSEEVT	mouseevt;
	REGIONPRM	*rgn;
	REGIONKEY	*rkey;
	int			cnt;
	int			i, r;
	int			pos;
	int			flag;
	UINT		keybit;
	VRAMHDL		map;
	int			version;
	BOOL		enable2;

	mouseevt = &gamecore.mouseevt;

	btn = event_getmouse(&x, &y);
	key = event_getkey();

	cnt = min(mouseevt->prm.cnt, GAMECORE_MAXRGN);

	// マウス
	if ((btn & MOUSE_MOVEBIT) || (mouseevt->flag & MEVT_RENEWAL)) {
		pos = -1;

		// マウス範囲指定
		if ((!(mouseevt->flag & MEVT_CLIP)) ||
			(rect_in(&mouseevt->clip, x, y))) {
			map = mouseevt->map;
			if (map) {
				if ((x >= 0) && (x < map->width) &&
					(y >= 0) && (y < map->height)) {
					pos = map->ptr[x + (y * map->width)];
					if ((pos >= cnt) || (!rgnenable(mouseevt->rgn + pos))) {
						pos = -1;
					}
				}
			}
			if (pos == -1) {
				rgn = mouseevt->rgn;
				for (i=0; i<cnt; i++, rgn++) {
					if (!rgnenable(rgn)) {
						continue;
					}
					if (rect_in(&rgn->r, x, y)) {
						pos = i;
						break;
					}
				}
			}
		}
		mouseevt->lastpos = pos;
		if (mouseevt->flag & MEVT_RENEWAL) {
			mouseevt->flag &= ~MEVT_RENEWAL;
			mouseevt->pos = pos;
		}
		if (btn & MOUSE_MOVEBIT) {
			event_resetmouse(~MOUSE_MOVEBIT);
			mouseevt->keypos = pos;
		}
	}

	// キーボード
	if (key & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
		pos = mouseevt->keypos;
		if ((pos >= 0) && (pos < cnt)) {
			for (i=0; i<(int)(sizeof(keymov)/sizeof(UINT)); i++) {
				keybit = keymov[i];
				if (key & keybit) {
					event_resetkey(~keybit);
					r = cnt;
					do {
						rkey = mouseevt->key + pos;
						if (!rkey->enable) {
							pos = mouseevt->defpos;
							break;
						}
						pos = rkey->move[i];
						if ((pos < 0) || (pos >= cnt)) {
							pos = mouseevt->defpos;
							break;
						}
						if (rgnenable(mouseevt->rgn + pos)) {
							break;
						}
					} while(--r);
					if (r == 0) {
						pos = mouseevt->defpos;
					}
					break;
				}
			}
		}
		else {
			event_resetkey(~(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT));
			pos = mouseevt->defpos;
		}
		mouseevt->lastpos = pos;
		mouseevt->keypos = pos;
	}

	pos = mouseevt->lastpos;
	if (gamecore.sys.type & GAME_SELECTEX) {
		if ((mouseevt->pos != pos) && (pos != -1)) {
			mouseevt->pos = pos;
		}
		if ((mouseevt->pos >= 0) &&
			(rgnenable(mouseevt->rgn + mouseevt->pos))) {
			pos = mouseevt->pos;
		}
	}

	flag = 0;
	if (mouseevt->prm.btnflg & 1) {
		if ((btn & RBUTTON_BIT) || (key & KEY_MENU)) {
			flag = -1;
		}
	}

	version = gamecore.sys.version;
	enable2 = ((version >= EXEVER_KONYA) && (version != EXEVER_VECHO));
	if (key & KEY_ENTER) {
		flag = (enable2)?2:1;
	}
	else {
		if (!(mouseevt->prm.btnflg & 4)) {
			if (btn & LBUTTON_BIT) {
				flag = 1;
			}
		}
		else {
			if (btn & LBUTTON_UPBIT) {
				event_resetmouse(~LBUTTON_UPBIT);
				flag = 1;
			}
		}
		if (enable2) {
			if (flag == 1) {
				if ((pos != -1) && (pos == mouseevt->lastpos)) {
					flag++;
				}
			}
		}
	}

	if (mouseevt->prm.btnflg & 2) {
		if (!flag) {			// ボタン待ち
			taskmng_sleep(10);
			return(GAMEEV_WAITMOUSE);
		}
		else {
			event_resetmouse(0);
		}
	}

	if (flag) {
		event_resetmouse(LBUTTON_BIT | RBUTTON_BIT);		// メニュー対策
		event_resetkey(~KEY_ENTER);
	}

	scr_valset(mouseevt->prm.cur, pos);
	scr_valset(mouseevt->prm.btn, flag);
	return(GAMEEV_SUCCESS);
}


// ---- msgclk

int event_msgclk(void) {

	UINT	btn, key;
	GAMECFG	gamecfg;

	btn = event_getmouse(NULL, NULL);
	key = event_getkey();
	if ((btn & LBUTTON_UPBIT) || (key & (KEY_SKIP | KEY_ENTER))) {
		event_resetmouse(0);
		event_resetkey(~KEY_ENTER);
		return(GAMEEV_SUCCESS);
	}
	gamecfg = &gamecore.gamecfg;
	if ((gamecfg->skip) ||
		((gamecfg->lastread) && (gamecfg->readskip))) {
		return(GAMEEV_SUCCESS);
	}

	taskmng_sleep(10);
	return(GAMEEV_MSGCLK);
}

// ---- irclk

int event_irclk(void) {

	UINT	btn, key;

	btn = event_getmouse(NULL, NULL);
	key = event_getkey();
	if ((btn & LBUTTON_UPBIT) || (key & (KEY_SKIP | KEY_ENTER))) {
		event_resetmouse(0);
		event_resetkey(~KEY_ENTER);
		return(GAMEEV_SUCCESS);
	}
	taskmng_sleep(10);
	return(GAMEEV_IRCLK);
}


// ---- wait

int event_wait(void) {

	TIMEEVT	tevt;
	int		tick;

	tevt = &gamecore.wait;
	tick = (int)(GETTICK() - tevt->base);
	tick = tevt->val - tick;
	if (tick > 0) {
		tick = min(tick, 20);
		taskmng_sleep(tick);
		return(GAMEEV_WAIT);
	}
	else {
		return(GAMEEV_SUCCESS);
	}
}

