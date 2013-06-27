#include	"compiler.h"
#include	"gamecore.h"
#include	"inputmng.h"
#include	"taskmng.h"
#include	"sound.h"


static void cmdwin_exec(TEXTWIN textwin, int x, int y, UINT btn, UINT key) {

	VRAMHDL		vram;
	int			i;
	int			curval;
	CMD_T		*cmd;
	int			size;
	CHO_T		*cho;

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
				vramdraw_draw();
				textwin->cmdfocus = curval;
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
		}
		if ((btn & LBUTTON_UPBIT) || (key & KEY_ENTER)) {
			textwin->cmdret = curval;
		}
	}

cwexec_exit:
	return;
}

static void event_exec(void) {

	EVTHDL	evthdl;
	int		x;
	int		y;
	UINT	btn;
	UINT	key;
	int		i;
	TEXTWIN	textwin;

	evthdl = &gamecore.evthdl;
	btn = inputmng_getmouse(&x, &y);
	inputmng_resetmouse(LBUTTON_BIT | RBUTTON_BIT);
	key = inputmng_getkey();

	evthdl->x = x;
	evthdl->y = y;
	evthdl->btn &= ~(LBUTTON_BIT | RBUTTON_BIT);
	evthdl->btn |= btn;
	evthdl->key = key;

	// ������߽�����
	if (btn & LBUTTON_DOWNBIT) {
		if (soundmix_isplaying(SOUNDTRK_VOICE)) {
			soundmix_stop(SOUNDTRK_VOICE, 0);
		}
	}

	// ���ޥ�ɥ�����ɥ�������
	for (i=0; i<GAMECORE_MAXTXTWIN; i++) {
		textwin = gamecore.textwin[i];
		if ((textwin) &&
			(textwin->flag & (TEXTWIN_CMDCAP | TEXTWIN_CMDCAPEX))) {
			cmdwin_exec(textwin, x, y, btn, key);
		}
	}
	vramdraw_draw();
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
	inputmng_resetmouse(mask | (~(LBUTTON_BIT | RBUTTON_BIT)));
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
	POINT_T		pt;
	TEXTCTRL	textctrl;

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
	if (gamecore.sys.version >= EXE_VER1) {
		if (method) {
			method = 3;
		}
		if (textwin->chocolor[method+2] == 0) {
			vram_zerofill(dst, &cho->rct);
		}
		else {
			vram_fill(dst, &cho->rct, textwin->chocolor[method+2], 0xff);
		}
		pt.x = cho->x + 1;
		pt.y = cho->y + 1;
		vrammix_text(dst, textctrl->font, cho->str,
								textwin->chocolor[method+1], &pt, &cho->rct);
		pt.x = cho->x;
		pt.y = cho->y;
		vrammix_text(dst, textctrl->font, cho->str,
								textwin->chocolor[method+0], &pt, &cho->rct);
	}
	else {
		vram_zerofill(dst, &cho->rct);
		pt.x = cho->x;
		pt.y = cho->y;
		vrammix_text(dst, textctrl->font, cho->str,
								textwin->chocolor[method], &pt, &cho->rct);
	}
	vramdraw_setrect(dst, &cho->rct);
}


// ----

int event_getcmdwin(TEXTWIN textwin) {

	int		r;

	event_exec();					// �����Ͼ�˲󤹤٤�
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

	mouseevt = &gamecore.mouseevt;

	btn = event_getmouse(&x, &y);
	key = event_getkey();

	cnt = min(mouseevt->prm.cnt, GAMECORE_MAXRGN);

	// �ޥ���
	if ((btn & MOUSE_MOVEBIT) || (mouseevt->flag & MEVT_RENEWAL)) {
		pos = -1;

		// �ޥ����ϰϻ���
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

	// �����ܡ���
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
	if ((mouseevt->pos != pos) && (pos != -1)) {
		mouseevt->pos = pos;
	}
	if ((mouseevt->pos >= 0) && (rgnenable(mouseevt->rgn + mouseevt->pos))) {
		pos = mouseevt->pos;
	}

	flag = 0;
	if (mouseevt->prm.btnflg & 1) {
		if (btn & RBUTTON_BIT) {
			flag = -1;
		}
	}

	if (key & KEY_ENTER) {
		flag = 2;
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
		// �����ʤ��Ȥ� �ץ饤�١��ȥʡ����� 2���֤��������
		if (gamecore.sys.version >= EXEVER_KONYA) {
			if (flag == 1) {
				if (pos == mouseevt->pos) {
					flag++;
				}
			}
		}
	}

	if (mouseevt->prm.btnflg & 2) {
		if (!flag) {			// �ܥ����Ԥ�
			taskmng_sleep(10);
			return(GAMEEV_WAITMOUSE);
		}
		else {
			event_resetmouse(0);
		}
	}

	if (flag) {
		event_resetmouse(LBUTTON_BIT | RBUTTON_BIT);		// ��˥塼�к�
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
	if ((gamecfg->lastread) && (gamecfg->readskip)) {
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
