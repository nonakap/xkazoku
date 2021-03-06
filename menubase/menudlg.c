#include	"compiler.h"
#include	"vram.h"
#include	"vrammix.h"
#include	"menudeco.inc"
#include	"menubase.h"
#include	"fontmng.h"


typedef struct _dprm {
struct _dprm	*next;
		int		val;
		char	str[128];
} _DLGPRM, *DLGPRM;

typedef struct {
	POINT_T		pt;
	void		*font;
} DLGTEXT;

typedef struct {
	void		*font;
	int			fontsize;
} DLGTAB;

typedef struct {
	void		*font;
	short		fontsize;
	short		scrollbar;
	short		dispmax;
	short		basepos;
} DLGLIST;

typedef struct {
	short		minval;
	short		maxval;
	int			pos;
	int			moving;
} DLGSLD;

typedef struct {
	VRAMHDL		icon;
} DLGICO;

typedef struct _ditem {
	int			type;
	MENUID		id;
	MENUFLG		flag;
	MENUID		page;
	MENUID		group;
	RECT_T		rect;
	DLGPRM		prm;
	int			prmcnt;
	int			val;
	VRAMHDL		vram;
	union {
		DLGTEXT		dt;
		DLGTAB		dtl;
		DLGLIST		dl;
		DLGSLD		ds;
		DLGICO		di;
	} c;
} _DLGHDL, *DLGHDL;

typedef struct {
	VRAMHDL		vram;
	LISTARRAY	dlg;
	LISTARRAY	res;
	int			locked;
	int			closing;
	int			sx;
	int			sy;
	void		*font;
	MENUID		page;
	MENUID		group;
	int			(*proc)(int msg, MENUID id);

	int			dragflg;
	int			btn;
	int			lastx;
	int			lasty;
	MENUID		lastid;
} _MENUDLG, *MENUDLG;


static	_MENUDLG	menudlg;

static void drawctrls(MENUDLG dlg, DLGHDL hdl);


// ----

static DLGPRM resappend(MENUDLG dlg, const char *str) {

	_DLGPRM		prm;

	prm.next = NULL;
	prm.val = 0;
	prm.str[0] = '\0';
	if (str) {
		milstr_ncpy(prm.str, str, sizeof(prm.str));
	}
	return((DLGPRM)listarray_append(dlg->res, &prm));
}


static DLGPRM ressea(DLGHDL hdl, int pos) {

	DLGPRM	prm;

	if (pos >= 0) {
		prm = hdl->prm;
		while(prm) {
			if (!pos) {
				return(prm);
			}
			pos--;
			prm = prm->next;
		}
	}
	return(NULL);
}


static BOOL dsbyid(void *vpItem, void *vpArg) {

	if (((DLGHDL)vpItem)->id == (MENUID)(unsigned long)vpArg) {
		return(TRUE);
	}
	return(FALSE);
}


static DLGHDL dlghdlsea(MENUDLG dlg, MENUID id) {

	return((DLGHDL)listarray_enum(dlg->dlg, dsbyid, (void *)(long)id));
}


static void getleft(POINT_T *pt, const RECT_T *rect, const POINT_T *sz) {

	pt->x = rect->left;
	pt->y = rect->top;
	(void)sz;
}


static void getcenter(POINT_T *pt, const RECT_T *rect, const POINT_T *sz) {

	pt->x = rect->left;
	pt->x += (rect->right - rect->left - sz->x) >> 1;
	pt->y = rect->top;
}

static void getright(POINT_T *pt, const RECT_T *rect, const POINT_T *sz) {

	pt->x = rect->right - sz->x - MENU_DSTEXT;
	pt->y = rect->top;
}


static void getmid(POINT_T *pt, const RECT_T *rect, const POINT_T *sz) {

	pt->x = rect->left;
	pt->x += (rect->right - rect->left - sz->x) >> 1;
	pt->y = rect->top;
	pt->y += (rect->bottom - rect->top - sz->y) >> 1;
}


static BOOL _cre_settext(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	hdl->prm = resappend(dlg, (const char *)arg);
	hdl->c.dt.font = dlg->font;
	fontmng_getsize(dlg->font, (char *)arg, &hdl->c.dt.pt);
	return(SUCCESS);
}


static void dlg_text(MENUDLG dlg, DLGHDL hdl,
									const POINT_T *pt, const RECT_T *rect) {

const char	*string;
	int		color;
	POINT_T	p;

	if (hdl->prm == NULL) {
		goto dgtx_exit;
	}
	string = hdl->prm->str;
	if (string == NULL) {
		goto dgtx_exit;
	}
	if (!(hdl->flag & MENU_GRAY)) {
		color = MVC_TEXT;
	}
	else {
		p.x = pt->x + MENU_DSTEXT;
		p.y = pt->y + MENU_DSTEXT;
		vrammix_text(dlg->vram, hdl->c.dt.font, string,
										menucolor[MVC_GRAYTEXT2], &p, rect);
		color = MVC_GRAYTEXT1;
	}
	p.x = pt->x;
	p.y = pt->y;
	vrammix_text(dlg->vram, hdl->c.dt.font, string,
										menucolor[color], &p, rect);

dgtx_exit:
	return;
}


// ---- base

static BOOL dlgbase_create(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	RECT_T		rct;

	rct.right = hdl->rect.right - hdl->rect.left -
										((MENU_FBORDER + MENU_BORDER) * 2);
	hdl->vram = vram_create(rct.right, MENUDLG_CYCAPTION, FALSE, DEFAULT_BPP);
	if (hdl->vram == NULL) {
		goto dbcre_err;
	}
	hdl->vram->posx = (MENU_FBORDER + MENU_BORDER);
	hdl->vram->posy = (MENU_FBORDER + MENU_BORDER);
	rct.left = 0;
	rct.top = 0;
	rct.bottom = MENUDLG_CYCAPTION;
	menuvram_caption(hdl->vram, &rct, NULL, (const char *)arg);
	return(SUCCESS);

dbcre_err:
	(void)dlg;
	return(FAILURE);
}


static void dlgbase_paint(MENUDLG dlg, DLGHDL hdl) {

	char	*title;

	title = NULL;
	if (hdl->prm) {
		title = hdl->prm->str;
	}
	menuvram_base(dlg->vram);
	vrammix_cpy(dlg->vram, hdl->vram, NULL);
	menubase_setrect(dlg->vram, NULL);
}


static void dlgbase_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	RECT_T	rct;

	vram_getrect(hdl->vram, &rct);
	dlg->dragflg = rect_in(&rct, x, y);
	dlg->lastx = x;
	dlg->lasty = y;
}


static void dlgbase_move(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (dlg->dragflg) {
		x -= dlg->lastx;
		y -= dlg->lasty;
		if ((x) || (y)) {
			menubase_clrrect(dlg->vram);
			dlg->vram->posx += x;
			dlg->vram->posy += y;
			menubase_setrect(dlg->vram, NULL);
		}
	}
	(void)hdl;
	(void)focus;
}


// ---- close

static void dlgclose_paint(MENUDLG dlg, DLGHDL hdl) {

	menuvram_closebtn(dlg->vram, &hdl->rect, hdl->val);
}


static void dlgclose_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	hdl->val = 1;
	drawctrls(dlg, hdl);
	(void)x;
	(void)y;
}


static void dlgclose_move(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (hdl->val != focus) {
		hdl->val = focus;
		drawctrls(dlg, hdl);
	}
	(void)x;
	(void)y;
}


static void dlgclose_rel(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (focus) {
		dlg->proc(DLGMSG_CLOSE, 0);
	}
	(void)hdl;
	(void)x;
	(void)y;
}


// ---- button

static void dlgbtn_paint(MENUDLG dlg, DLGHDL hdl) {

	POINT_T	pt;
	UINT	c;

	vram_filldat(dlg->vram, &hdl->rect, menucolor[MVC_BTNFACE]);
	if (!hdl->val) {
		c = MVC4(MVC_HILIGHT, MVC_DARK, MVC_LIGHT, MVC_SHADOW);
	}
	else {
		c = MVC4(MVC_DARK, MVC_DARK, MVC_SHADOW, MVC_SHADOW);
	}
	menuvram_box2(dlg->vram, &hdl->rect, c);

	if (hdl->prm) {
		getmid(&pt, &hdl->rect, &hdl->c.dt.pt);
		if (hdl->val) {
			pt.x += MENU_DSTEXT;
			pt.y += MENU_DSTEXT;
		}
		dlg_text(dlg, hdl, &pt, &hdl->rect);
	}
}


static void dlgbtn_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	hdl->val = 1;
	drawctrls(dlg, hdl);
	(void)x;
	(void)y;
}


static void dlgbtn_move(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (hdl->val != focus) {
		hdl->val = focus;
		drawctrls(dlg, hdl);
	}
	(void)x;
	(void)y;
}


static void dlgbtn_rel(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (focus) {
		hdl->val = 0;
		drawctrls(dlg, hdl);
		dlg->proc(DLGMSG_COMMAND, hdl->id);
	}
	(void)x;
	(void)y;
}


// ---- list

static void *dlglist_setfont(DLGHDL hdl, void *font) {
										// 後でスクロールバーの調整をすべし
	void	*ret;
	POINT_T	pt;

	ret = hdl->c.dl.font;
	hdl->c.dl.font = font;
	fontmng_getsize(font, " ", &pt);
	if ((pt.y <= 0) || (pt.y >= 65536)) {
		pt.y = 16;
	}
	hdl->c.dl.fontsize = (short)pt.y;
	hdl->c.dl.dispmax = (short)(hdl->vram->height / pt.y);
	return(ret);
}

static BOOL dlglist_create(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	int		width;
	int		height;

	width = hdl->rect.right - hdl->rect.left - (MENU_LINE * 4);
	height = hdl->rect.bottom - hdl->rect.top - (MENU_LINE * 4);
	hdl->vram = vram_create(width, height, FALSE, DEFAULT_BPP);
	if (hdl->vram == NULL) {
		goto dlcre_err;
	}
	hdl->vram->posx = hdl->rect.left + (MENU_LINE * 2);
	hdl->vram->posy = hdl->rect.top + (MENU_LINE * 2);
	vram_filldat(hdl->vram, NULL, 0xffffff);

	hdl->val = -1;
	hdl->c.dl.scrollbar = 0;
	hdl->c.dl.basepos = 0;
	dlglist_setfont(hdl, dlg->font);
	return(SUCCESS);

dlcre_err:
	(void)dlg;
	(void)arg;
	return(FAILURE);
}


static void dlglist_paint(MENUDLG dlg, DLGHDL hdl) {

	menuvram_box2(dlg->vram, &hdl->rect,
						MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_DARK, MVC_LIGHT));
	vrammix_cpy(dlg->vram, hdl->vram, NULL);
}


static void dlglist_drawitem(DLGHDL hdl, DLGPRM prm, int focus,
												POINT_T *pt, RECT_T *rct) {

	vram_filldat(hdl->vram, rct, menucolor[focus?MVC_CURBACK:MVC_HILIGHT]);
	vrammix_text(hdl->vram, hdl->c.dl.font, prm->str,
							menucolor[focus?MVC_CURTEXT:MVC_TEXT], pt, rct);
}


static void dlglist_drawsub(DLGHDL hdl, int pos, int focus) {

	DLGPRM	prm;
	POINT_T	pt;
	RECT_T	rct;

	prm = ressea(hdl, pos);
	if (prm == NULL) {
		goto dlds_end;
	}
	pos -= hdl->c.dl.basepos;
	if (pos < 0) {
		goto dlds_end;
	}
	pt.x = 0;
	pt.y = pos * hdl->c.dl.fontsize;
	if (pt.y >= hdl->vram->height) {
		goto dlds_end;
	}
	rct.left = 0;
	rct.top = pt.y;
	rct.right = hdl->vram->width;
	if (hdl->prmcnt > hdl->c.dl.dispmax) {
		rct.right -= MENUDLG_CXVSCR;
	}
	rct.bottom = rct.top + hdl->c.dl.fontsize;
	dlglist_drawitem(hdl, prm, focus, &pt, &rct);

dlds_end:
	return;
}


static void dlglist_setbtn(DLGHDL hdl, int flg) {

	RECT_T		rct;
	POINT_T		pt;
	UINT		mvc4;
const MENURES2	*res;

	res = menures_scrbtn;
	rct.right = hdl->vram->width;
	rct.left = rct.right - MENUDLG_CXVSCR;
	if (!(flg & 2)) {
		rct.top = 0;
	}
	else {
		rct.top = hdl->vram->height - MENUDLG_CYVSCR;
		if (rct.top < MENUDLG_CYVSCR) {
			rct.top = MENUDLG_CYVSCR;
		}
		res++;
	}
	rct.bottom = rct.top + MENUDLG_CYVSCR;

	vram_filldat(hdl->vram, &rct, menucolor[MVC_BTNFACE]);
	if (flg & 1) {
		mvc4 = MVC4(MVC_SHADOW, MVC_SHADOW, MVC_LIGHT, MVC_LIGHT);
	}
	else {
		mvc4 = MVC4(MVC_LIGHT, MVC_DARK, MVC_HILIGHT, MVC_SHADOW);
	}
	menuvram_box2(hdl->vram, &rct, mvc4);
	pt.x = rct.left + (MENU_LINE * 2);
	pt.y = rct.top + (MENU_LINE * 2);
	if (flg & 1) {
		pt.x += MENU_DSTEXT;
		pt.y += MENU_DSTEXT;
	}
	menuvram_res3put(hdl->vram, res, &pt, MVC_TEXT);
}


static void dlglist_drawall(DLGHDL hdl) {

	DLGPRM	prm;
	POINT_T	pt;
	RECT_T	rct;
	int		pos;

	rct.left = 0;
	rct.top = 0 - (hdl->c.dl.basepos * hdl->c.dl.fontsize);
	rct.right = hdl->vram->width;
	if (hdl->prmcnt > hdl->c.dl.dispmax) {
		rct.right -= MENUDLG_CXVSCR;
	}

	prm = hdl->prm;
	pos = 0;
	while(prm) {
		if (rct.top >= hdl->vram->height) {
			break;
		}
		if (rct.top >= 0) {
			rct.bottom = rct.top + hdl->c.dl.fontsize;
			pt.x = 0;
			pt.y = rct.top;
			dlglist_drawitem(hdl, prm, (pos == hdl->val), &pt, &rct);
		}
		prm = prm->next;
		pos++;
		rct.top += hdl->c.dl.fontsize;
	}
	rct.bottom = hdl->vram->height;
	vram_filldat(hdl->vram, &rct, menucolor[MVC_HILIGHT]);
}


static int dlglist_barpos(DLGHDL hdl) {

	int		ret;

	ret = hdl->vram->height - (MENUDLG_CYVSCR * 2);
	ret -= hdl->c.dl.scrollbar;
	ret *= hdl->c.dl.basepos;
	ret /= (hdl->prmcnt - hdl->c.dl.dispmax);
	return(ret);
}


static void dlglist_drawbar(DLGHDL hdl) {

	RECT_T	rct;

	rct.right = hdl->vram->width;
	rct.left = rct.right - MENUDLG_CXVSCR;
	rct.top = MENUDLG_CYVSCR;
	rct.bottom = hdl->vram->height - MENUDLG_CYVSCR;
	vram_filldat(hdl->vram, &rct, menucolor[MVC_SCROLLBAR]);

	rct.top += dlglist_barpos(hdl);
	rct.bottom = rct.top + hdl->c.dl.scrollbar;
	vram_filldat(hdl->vram, &rct, menucolor[MVC_BTNFACE]);
	menuvram_box2(hdl->vram, &rct,
						MVC4(MVC_LIGHT, MVC_DARK, MVC_HILIGHT, MVC_SHADOW));
}


static void dlglist_append(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	DLGPRM	*sto;
	int		barsize;

	sto = &hdl->prm;
	while(*sto) {
		sto = &((*sto)->next);
	}
	*sto = resappend(dlg, (const char *)arg);
	if (*sto) {
		dlglist_drawsub(hdl, hdl->prmcnt, FALSE);
		hdl->prmcnt++;
		if (hdl->prmcnt > hdl->c.dl.dispmax) {
			barsize = hdl->vram->height - (MENUDLG_CYVSCR * 2);
			if (barsize >= 8) {
				barsize *= hdl->c.dl.dispmax;
				barsize /= hdl->prmcnt;
				barsize = max(barsize, 6);
				if (!hdl->c.dl.scrollbar) {
					dlglist_drawall(hdl);
					dlglist_setbtn(hdl, 0);
					dlglist_setbtn(hdl, 2);
				}
				hdl->c.dl.scrollbar = barsize;
				dlglist_drawbar(hdl);
			}
		}
	}
}


static int dlglist_getpc(DLGHDL hdl, int x, int y) {

	x -= (MENU_LINE * 2);
	if ((unsigned int)x >= (unsigned int)hdl->vram->width) {
		goto dlgp_out;
	}
	y -= (MENU_LINE * 2);
	if ((unsigned int)y >= (unsigned int)hdl->vram->height) {
		goto dlgp_out;
	}
	if ((hdl->prmcnt < hdl->c.dl.dispmax) ||
		(x < (hdl->vram->width - MENUDLG_CXVSCR))) {
		return(0);
	}
	else if (y < MENUDLG_CYVSCR) {
		return(1);
	}
	else if (y >= (hdl->vram->height - MENUDLG_CYVSCR)) {
		return(3);
	}
	else if (hdl->c.dl.scrollbar) {
		return(2);
	}

dlgp_out:
	return(-1);
}


static void dlglist_setval(MENUDLG dlg, DLGHDL hdl, int val) {

	if ((unsigned int)val >= (unsigned int)hdl->prmcnt) {
		val = -1;
	}
	if (val != hdl->val) {
		dlglist_drawsub(hdl, hdl->val, FALSE);
		dlglist_drawsub(hdl, val, TRUE);
		hdl->val = val;
		drawctrls(dlg, hdl);
	}
}


static void dlglist_setbasepos(MENUDLG dlg, DLGHDL hdl, int pos) {

	int		displimit;

	if (pos < 0) {
		pos = 0;
	}
	else {
		displimit = hdl->prmcnt - hdl->c.dl.dispmax;
		if (displimit < 0) {
			displimit = 0;
		}
		if (pos > displimit) {
			pos = displimit;
		}
	}
	if (hdl->c.dl.basepos != pos) {
		hdl->c.dl.basepos = pos;
		dlglist_drawall(hdl);
		dlglist_drawbar(hdl);
	}
	(void)dlg;
}


static void dlglist_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	int		flg;

	flg = dlglist_getpc(hdl, x, y);
	y -= (MENU_LINE * 2);
	dlg->dragflg = flg;
	switch(flg) {
		case 0:
			dlglist_setval(dlg, hdl, (y / hdl->c.dl.fontsize) +
														hdl->c.dl.basepos);
			break;

		case 1:
		case 3:
			dlglist_setbtn(hdl, flg);
			dlglist_setbasepos(dlg, hdl, hdl->c.dl.basepos + flg - 2);
			drawctrls(dlg, hdl);
			break;

		case 2:
			y -= MENUDLG_CYVSCR;
			y -= dlglist_barpos(hdl);
			if ((unsigned int)y < (unsigned int)hdl->c.dl.scrollbar) {
				dlg->lasty = y;
			}
			else {
				dlg->lasty = -1;
			}
			break;
	}
}


static void dlglist_move(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	int		flg;
	int		height;

	flg = dlglist_getpc(hdl, x, y);
	y -= (MENU_LINE * 2);
	switch(dlg->dragflg) {
		case 0:
			if (flg == 0) {
				dlglist_setval(dlg, hdl, (y / hdl->c.dl.fontsize) +
														hdl->c.dl.basepos);
			}
			break;

		case 1:
		case 3:
			dlglist_setbtn(hdl, dlg->dragflg - ((dlg->dragflg == flg)?0:1));
			drawctrls(dlg, hdl);
			break;

		case 2:
			if (dlg->lasty >= 0) {
				y -= MENUDLG_CYVSCR;
				y -= dlg->lasty;
				height = hdl->vram->height - (MENUDLG_CYVSCR * 2);
				height -= hdl->c.dl.scrollbar;
				if (y < 0) {
					y = 0;
				}
				else if (y > height) {
					y = height;
				}
				y *= (hdl->prmcnt - hdl->c.dl.dispmax);
				y /= height;
				dlglist_setbasepos(dlg, hdl, y);
				drawctrls(dlg, hdl);
			}
			break;
	}
	(void)focus;
}


static void dlglist_rel(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	switch(dlg->dragflg) {
		case 1:
		case 3:
			dlglist_setbtn(hdl, dlg->dragflg - 1);
			drawctrls(dlg, hdl);
			break;
	}
	(void)x;
	(void)y;
	(void)focus;
}


// ---- slider

static int dlgslider_setpos(DLGHDL hdl, int val) {

	int		range;
	int		width;
	int		dir;

	range = hdl->c.ds.maxval - hdl->c.ds.minval;
	if (range) {
		dir = (range > 0)?1:-1;
		val -= hdl->c.ds.minval;
		val *= dir;
		range *= dir;
		if (val < 0) {
			val = 0;
		}
		else if (val >= range) {
			val = range;
		}
		hdl->val = hdl->c.ds.minval + (val * dir);
		if (!(hdl->flag & MSS_VERT)) {
			width = hdl->rect.right - hdl->rect.left;
		}
		else {
			width = hdl->rect.bottom - hdl->rect.top;
		}
		width -= (MENUDLG_SXSLD * 2) + MENU_LINE;
		if ((width > 0) || (range)) {
			val *= width;
			val /= range;
		}
		else {
			val = 0;
		}
	}
	else {
		val = 0;
	}
	return(val);
}


static BOOL dlgslider_create(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	hdl->c.ds.minval = (short)(long)arg;
	hdl->c.ds.maxval = (short)((long)arg >> 16);
	hdl->c.ds.moving = 0;
	hdl->c.ds.pos = dlgslider_setpos(hdl, 0);
	(void)dlg;
	return(SUCCESS);
}


static void dlgslider_paint(MENUDLG dlg, DLGHDL hdl) {

	RECT_U		rct;
	POINT_T		pt;
const MENURES2	*src;
	int			ptr;

	switch(hdl->flag & MSS_POSMASK) {
		case MSS_BOTH:
			ptr = 1;
			break;
		case MSS_TOP:
			ptr = 2;
			break;
		default:
			ptr = 0;
			break;
	}
	vram_filldat(dlg->vram, &hdl->rect, menucolor[MVC_STATIC]);
	src = menures_sld;
	if (!(hdl->flag & MSS_VERT)) {
		rct.r.left = hdl->rect.left;
		rct.r.right = hdl->rect.right;
		rct.r.top = hdl->rect.top + MENUDLG_SYSLD + ptr;
		rct.r.bottom = rct.r.top + (MENU_LINE * 4);
		menuvram_box2(dlg->vram, &rct.r,
						MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_DARK, MVC_LIGHT));
		pt.x = hdl->rect.left + hdl->c.ds.pos;
		pt.y = hdl->rect.top;
		if ((hdl->flag & MENU_GRAY) || (hdl->c.ds.moving)) {
			src += 3;
		}
		menuvram_res2put(dlg->vram, src + ptr, &pt);
	}
	else {
		rct.r.left = hdl->rect.left + MENUDLG_SYSLD + ptr;
		rct.r.right = rct.r.left + (MENU_LINE * 4);
		rct.r.top = hdl->rect.top;
		rct.r.bottom = hdl->rect.bottom;
		menuvram_box2(dlg->vram, &rct.r,
						MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_DARK, MVC_LIGHT));
		pt.x = hdl->rect.left;
		pt.y = hdl->rect.top + hdl->c.ds.pos;
		if ((hdl->flag & MENU_GRAY) || (hdl->c.ds.moving)) {
			src += 3;
		}
		menuvram_res2put(dlg->vram, src + 6 + ptr, &pt);
	}
}


static void dlgslider_setval(MENUDLG dlg, DLGHDL hdl, int val) {

	int		pos;

	pos = dlgslider_setpos(hdl, val);
	if (hdl->c.ds.pos != pos) {
		hdl->c.ds.pos = pos;
		drawctrls(dlg, hdl);
	}
}


static void dlgslider_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	int		range;
	int		dir;

	if (hdl->flag & MSS_VERT) {
		x = y;
	}
	x -= hdl->c.ds.pos;
	if ((x >= 0) && (x <= (MENUDLG_SXSLD * 2))) {
		dlg->dragflg = x;
		hdl->c.ds.moving = 1;
		drawctrls(dlg, hdl);
	}
	else {
		dlg->dragflg = -1;
		dir = (x > 0)?1:0;
		range = hdl->c.ds.maxval - hdl->c.ds.minval;
		if (range < 0) {
			range = 0 - range;
			dir ^= 1;
		}
		if (range < 16) {
			range = 16;
		}
		range >>= 4;
		if (!dir) {
			range = 0 - range;
		}
		dlgslider_setval(dlg, hdl, hdl->val + range);
		dlg->proc(DLGMSG_COMMAND, hdl->id);
	}
}


static void dlgslider_move(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	int		range;
	int		width;
	int		dir;

	if (hdl->c.ds.moving) {
		range = hdl->c.ds.maxval - hdl->c.ds.minval;
		if (range) {
			dir = (range > 0)?1:-1;
			range *= dir;
			if (!(hdl->flag & MSS_VERT)) {
				width = hdl->rect.right - hdl->rect.left;
			}
			else {
				width = hdl->rect.bottom - hdl->rect.top;
				x = y;
			}
			x -= dlg->dragflg;
			width -= (MENUDLG_SXSLD * 2) + 1;
			if ((x < 0) || (width <= 0)) {
				x = 0;
			}
			else if (x >= width) {
				x = range;
			}
			else {
				x *= range;
				x += (width >> 1);
				x /= width;
			}
			x = hdl->c.ds.minval + (x * dir);
			dlgslider_setval(dlg, hdl, x);
			dlg->proc(DLGMSG_COMMAND, hdl->id);
		}
	}
	(void)focus;
}


static void dlgslider_rel(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	if (hdl->c.ds.moving) {
		hdl->c.ds.moving = 0;
		drawctrls(dlg, hdl);
	}
	(void)x;
	(void)y;
	(void)focus;
}


// ---- tablist

static void *dlgtablist_setfont(DLGHDL hdl, void *font) {

	void	*ret;
	POINT_T	pt;
	DLGPRM	prm;

	ret = hdl->c.dtl.font;
	hdl->c.dtl.font = font;
	fontmng_getsize(font, " ", &pt);
	if ((pt.y <= 0) || (pt.y >= 65536)) {
		pt.y = 16;
	}
	hdl->c.dtl.fontsize = pt.y;
	prm = hdl->prm;
	while(prm) {
		fontmng_getsize(hdl->c.dtl.font, prm->str, &pt);
		prm->val = pt.x;
		prm = prm->next;
	}
	return(ret);
}

static BOOL dlgtablist_create(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	RECT_T	rct;

	rct.right = hdl->rect.right - hdl->rect.left;
	hdl->val = -1;
	dlgtablist_setfont(hdl, dlg->font);
	(void)arg;
	return(SUCCESS);
}


static void dlgtablist_paint(MENUDLG dlg, DLGHDL hdl) {

	VRAMHDL	dst;
	DLGPRM	prm;
	POINT_T	pt;
	RECT_T	rct;
	int		posx;
	int		lx;
	int		cnt;
	int		tabey;
	int		tabdy;

	dst = dlg->vram;
	rct = hdl->rect;
	vram_filldat(dst, &rct, menucolor[MVC_STATIC]);
	tabey = rct.top + hdl->c.dtl.fontsize +
							MENUDLG_SYTAB + MENUDLG_TYTAB + MENUDLG_EYTAB;
	rct.top = tabey;
	menuvram_box2(dst, &rct,
						MVC4(MVC_HILIGHT, MVC_DARK, MVC_LIGHT, MVC_SHADOW));

	posx = hdl->rect.left + (MENU_LINE * 2);
	prm = hdl->prm;
	cnt = hdl->val;
	while(prm) {
		if (cnt) {
			pt.x = posx;
			pt.y = hdl->rect.top + MENUDLG_SYTAB;
			menuvram_liney(dst, pt.x, pt.y + (MENU_LINE * 2),
														tabey, MVC_HILIGHT);
			pt.x += MENU_LINE;
			menuvram_liney(dst, pt.x, pt.y + MENU_LINE,
										pt.y + (MENU_LINE * 2), MVC_HILIGHT);
			menuvram_liney(dst, pt.x, pt.y + (MENU_LINE * 2),
														tabey, MVC_LIGHT);
			pt.x += MENU_LINE;
			lx = pt.x + prm->val + (MENUDLG_TXTAB * 2);
			menuvram_linex(dst, pt.x, pt.y, lx, MVC_HILIGHT);
			menuvram_linex(dst, pt.x, pt.y + MENU_LINE, lx, MVC_LIGHT);

			menuvram_liney(dst, lx, pt.y + MENU_LINE,
										pt.y + (MENU_LINE * 2), MVC_DARK);
			menuvram_liney(dst, lx, pt.y + (MENU_LINE * 2),
														tabey, MVC_SHADOW);
			lx++;
			menuvram_liney(dst, lx, pt.y + (MENU_LINE * 2),
														tabey, MVC_DARK);
			pt.x += MENUDLG_TXTAB;
			pt.y += MENUDLG_TYTAB;
			vrammix_text(dst, hdl->c.dtl.font, prm->str,
											menucolor[MVC_TEXT], &pt, NULL);
		}
		cnt--;
		posx += prm->val + (MENU_LINE * 4) + (MENUDLG_TXTAB) * 2;
		prm = prm->next;
	}

	posx = hdl->rect.left;
	prm = hdl->prm;
	cnt = hdl->val;
	while(prm) {
		if (!cnt) {
			pt.x = posx;
			pt.y = hdl->rect.top;
			if (posx == hdl->rect.left) {
				tabdy = tabey + 2;
			}
			else {
				tabdy = tabey + 1;
				menuvram_linex(dst, pt.x, tabdy,
										pt.x + (MENU_LINE * 2), MVC_STATIC);
			}
			menuvram_liney(dst, pt.x, pt.y + (MENU_LINE * 2),
														tabdy, MVC_HILIGHT);
			pt.x += MENU_LINE;
			menuvram_liney(dst, pt.x, pt.y + MENU_LINE,
										pt.y + (MENU_LINE * 2), MVC_HILIGHT);
			menuvram_liney(dst, pt.x, pt.y + (MENU_LINE * 2),
														tabdy, MVC_LIGHT);
			pt.x += MENU_LINE;
			lx = pt.x + prm->val + (MENU_LINE * 4) + (MENUDLG_TXTAB * 2);
			menuvram_linex(dst, pt.x, pt.y, lx, MVC_HILIGHT);
			menuvram_linex(dst, pt.x, pt.y + MENU_LINE, lx, MVC_LIGHT);
			menuvram_linex(dst, pt.x, tabey, lx, MVC_STATIC);
			menuvram_linex(dst, pt.x, tabey + MENU_LINE, lx, MVC_STATIC);
			tabdy = tabey + 1;
			menuvram_liney(dst, lx, pt.y + MENU_LINE,
										pt.y + (MENU_LINE * 2), MVC_DARK);
			menuvram_liney(dst, lx, pt.y + (MENU_LINE * 2),
														tabdy, MVC_SHADOW);
			lx++;
			menuvram_liney(dst, lx, pt.y + (MENU_LINE * 2),
														tabdy, MVC_DARK);
			pt.x += MENUDLG_TXTAB + (MENU_LINE * 2);
			pt.y += MENUDLG_TYTAB;
			vrammix_text(dst, hdl->c.dtl.font, prm->str,
											menucolor[MVC_TEXT], &pt, NULL);
			break;
		}
		cnt--;
		posx += prm->val + (MENU_LINE * 4) + (MENUDLG_TXTAB * 2);
		prm = prm->next;
	}
}


static void dlgtablist_setval(MENUDLG dlg, DLGHDL hdl, int val) {

	if (hdl->val != val) {
		hdl->val = val;
		drawctrls(dlg, hdl);
	}
}


static void dlgtablist_append(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	DLGPRM	res;
	DLGPRM	*sto;
	POINT_T	pt;

	sto = &hdl->prm;
	while(*sto) {
		sto = &((*sto)->next);
	}
	res = resappend(dlg, (const char *)arg);
	if (res) {
		*sto = res;
		fontmng_getsize(hdl->c.dtl.font, (char *)arg, &pt);
		res->val = pt.x;
		hdl->prmcnt++;
	}
}


static void dlgtablist_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	DLGPRM	prm;
	int		pos;

	if (y < (hdl->c.dtl.fontsize +
							MENUDLG_SYTAB + MENUDLG_TYTAB + MENUDLG_EYTAB)) {
		pos = 0;
		prm = hdl->prm;
		while(prm) {
			x -= (MENU_LINE * 4);
			if (x < 0) {
				break;
			}
			x -= prm->val + (MENUDLG_TXTAB * 2);
			if (x < 0) {
				dlgtablist_setval(dlg, hdl, pos);
				dlg->proc(DLGMSG_COMMAND, hdl->id);
				break;
			}
			pos++;
			prm = prm->next;
		}
	}
}


// ---- edit

static void dlgedit_paint(MENUDLG dlg, DLGHDL hdl) {

	RECT_T	rct;
	POINT_T	pt;
const char	*string;

	rct = hdl->rect;
	menuvram_box2(dlg->vram, &rct,
						MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_DARK, MVC_LIGHT));
	rct.left += (MENU_LINE * 2);
	rct.top += (MENU_LINE * 2);
	rct.right -= (MENU_LINE * 2);
	rct.bottom -= (MENU_LINE * 2);
	vram_filldat(dlg->vram, &rct, menucolor[
							(hdl->flag & MENU_GRAY)?MVC_STATIC:MVC_HILIGHT]);
	if (hdl->prm == NULL) {
		goto dged_exit;
	}
	string = hdl->prm->str;
	if (string == NULL) {
		goto dged_exit;
	}
	pt.x = rct.left + MENU_LINE;
	pt.y = rct.top + MENU_LINE;
	vrammix_text(dlg->vram, hdl->c.dt.font, string,
											menucolor[MVC_TEXT], &pt, &rct);

dged_exit:
	return;
}


// ---- frame

static void dlgframe_paint(MENUDLG dlg, DLGHDL hdl) {

	RECT_T		rct;
	POINT_T		pt;

	rct.left = hdl->rect.left;
	rct.top = hdl->rect.top + MENUDLG_SYFRAME;
	rct.right = hdl->rect.right;
	rct.bottom = hdl->rect.bottom;
	menuvram_box2(dlg->vram, &rct,
					MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_HILIGHT, MVC_SHADOW));
	rct.left += MENUDLG_SXFRAME;
	rct.top = hdl->rect.top;
	rct.right = rct.left + (MENUDLG_PXFRAME * 2) + hdl->c.dt.pt.x;
	rct.bottom = rct.top + hdl->c.dt.pt.y + MENU_DSTEXT;
	vram_filldat(dlg->vram, &rct, menucolor[MVC_STATIC]);
	if (hdl->prm) {
		pt.x = rct.left + MENUDLG_PXFRAME;
		pt.y = rct.top;
		dlg_text(dlg, hdl, &pt, &rct);
	}
}


// ---- radio

static void dlgradio_paint(MENUDLG dlg, DLGHDL hdl) {

	POINT_T		pt;
const MENURES2	*src;
	int			pat;

	vram_filldat(dlg->vram, &hdl->rect, menucolor[MVC_STATIC]);
	pt.x = hdl->rect.left;
	pt.y = hdl->rect.top;
	src = menures_radio;
	pat = (hdl->flag & MENU_GRAY)?1:0;
	menuvram_res2put(dlg->vram, src + pat, &pt);
	if (hdl->val) {
		menuvram_res3put(dlg->vram, src + 2, &pt,
					(hdl->flag & MENU_GRAY)?MVC_GRAYTEXT1:MVC_TEXT);
	}
	pt.x += MENUDLG_SXRADIO;
	dlg_text(dlg, hdl, &pt, &hdl->rect);
}


typedef struct {
	MENUDLG	dlg;
	MENUID	group;
} MDCB1;

static BOOL drsv_cb(void *vpItem, void *vpArg) {

	DLGHDL	item;

	item = (DLGHDL)vpItem;
	if ((item->type == DLGTYPE_RADIO) && (item->val) &&
		(item->group == ((MDCB1 *)vpArg)->group)) {
		item->val = 0;
		drawctrls(((MDCB1 *)vpArg)->dlg, item);
	}
	return(FALSE);
}

static void dlgradio_setval(MENUDLG dlg, DLGHDL hdl, int val) {

	MDCB1	mdcb;

	if (hdl->val != val) {
		if (val) {
			mdcb.dlg = dlg;
			mdcb.group = hdl->group;
			listarray_enum(dlg->dlg, drsv_cb, &mdcb);
		}
		hdl->val = val;
		drawctrls(dlg, hdl);
	}
}


static void dlgradio_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	if (x < (hdl->c.dt.pt.x + MENUDLG_SXRADIO)) {
		dlgradio_setval(dlg, hdl, 1);
		dlg->proc(DLGMSG_COMMAND, hdl->id);
	}
	(void)y;
}


// ---- check

static void dlgcheck_paint(MENUDLG dlg, DLGHDL hdl) {

	POINT_T	pt;
	RECT_T	rct;
	UINT32	basecol;
	UINT32	txtcol;

	vram_filldat(dlg->vram, &hdl->rect, menucolor[MVC_STATIC]);
	rct.left = hdl->rect.left;
	rct.top = hdl->rect.top;
	rct.right = rct.left + MENUDLG_CXCHECK;
	rct.bottom = rct.top + MENUDLG_CYCHECK;
	if (!(hdl->flag & MENU_GRAY)) {
		basecol = MVC_HILIGHT;
		txtcol = MVC_TEXT;
	}
	else {
		basecol = MVC_STATIC;
		txtcol = MVC_GRAYTEXT1;
	}
	vram_filldat(dlg->vram, &rct, menucolor[basecol]);
	menuvram_box2(dlg->vram, &rct,
						MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_DARK, MVC_LIGHT));
	if (hdl->val) {
		pt.x = rct.left + (MENU_LINE * 2);
		pt.y = rct.top + (MENU_LINE * 2);
		menuvram_res3put(dlg->vram, &menures_check, &pt, txtcol);
	}
	pt.x = rct.left + MENUDLG_SXCHECK;
	pt.y = rct.top;
	dlg_text(dlg, hdl, &pt, &hdl->rect);
}


static void dlgcheck_setval(MENUDLG dlg, DLGHDL hdl, int val) {

	if (hdl->val != val) {
		hdl->val = val;
		drawctrls(dlg, hdl);
	}
}


static void dlgcheck_onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	if (x < (hdl->c.dt.pt.x + MENUDLG_SXCHECK)) {
		dlgcheck_setval(dlg, hdl, !hdl->val);
		dlg->proc(DLGMSG_COMMAND, hdl->id);
	}
	(void)y;
}


// ---- text

static void dlgtext_paint(MENUDLG dlg, DLGHDL hdl) {

	POINT_T	pt;
	void	(*getpt)(POINT_T *pt, const RECT_T *rect, const POINT_T *sz);

	vram_filldat(dlg->vram, &hdl->rect, menucolor[MVC_STATIC]);
	if (hdl->prm) {
		switch(hdl->flag & MST_POSMASK) {
			case MST_LEFT:
			default:
				getpt = getleft;
				break;

			case MST_CENTER:
				getpt = getcenter;
				break;

			case MST_RIGHT:
				getpt = getright;
				break;
		}
		getpt(&pt, &hdl->rect, &hdl->c.dt.pt);
		dlg_text(dlg, hdl, &pt, &hdl->rect);
	}
}


static void dlgtext_itemset(MENUDLG dlg, DLGHDL hdl, const void *str) {

	if (hdl->prm) {
		if (str) {
			milstr_ncpy(hdl->prm->str, (char *)str, sizeof(hdl->prm->str));
			fontmng_getsize(hdl->c.dt.font, (char *)str, &hdl->c.dt.pt);
		}
		else {
			hdl->prm->str[0] = '\0';
			hdl->c.dt.pt.x = 0;
		}
	}
	(void)dlg;
}


// ---- icon

static BOOL dlgicon_create(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	hdl->c.di.icon = (VRAMHDL)arg;
	(void)dlg;
	return(SUCCESS);
}

static void dlgicon_paint(MENUDLG dlg, DLGHDL hdl) {

	VRAMHDL		src;
	RECT_U		r;
	UINT32		bgcol;

	r.p.x = hdl->rect.left;
	r.p.y = hdl->rect.top;
	bgcol = menucolor[MVC_STATIC];
	src = hdl->c.di.icon;
	if (src) {
		if (src->alpha) {
			r.r.right = r.r.left + src->width;
			r.r.bottom = r.r.top + src->height;
			vram_filldat(dlg->vram, &r.r, bgcol);
			vramcpy_cpyex(dlg->vram, src, &r.p, NULL);
		}
		else {
			vramcpy_cpy(dlg->vram, src, &r.p, NULL);
		}
	}
	else {
		vram_filldat(dlg->vram, &hdl->rect, bgcol);
	}
}


// ---- line

static void dlgline_paint(MENUDLG dlg, DLGHDL hdl) {

	if (!(hdl->flag & MSL_VERT)) {
		menuvram_linex(dlg->vram, hdl->rect.left, hdl->rect.top,
											hdl->rect.right, MVC_SHADOW);
		menuvram_linex(dlg->vram, hdl->rect.left, hdl->rect.top + MENU_LINE,
											hdl->rect.right, MVC_HILIGHT);
	}
	else {
		menuvram_liney(dlg->vram, hdl->rect.left, hdl->rect.top,
											hdl->rect.bottom, MVC_SHADOW);
		menuvram_liney(dlg->vram, hdl->rect.left+MENU_LINE, hdl->rect.top,
											hdl->rect.bottom, MVC_HILIGHT);
	}
}


// ---- box

static void dlgbox_paint(MENUDLG dlg, DLGHDL hdl) {

	menuvram_box2(dlg->vram, &hdl->rect,
					MVC4(MVC_SHADOW, MVC_HILIGHT, MVC_HILIGHT, MVC_SHADOW));
}


// ---- procs

static BOOL _cre(MENUDLG dlg, DLGHDL hdl, const void *arg) {

	(void)dlg;
	(void)hdl;
	(void)arg;
	return(0);
}

#if 0		// not used
static void _paint(MENUDLG dlg, DLGHDL hdl) {

	(void)dlg;
	(void)hdl;
}
#endif

#if 0		// not used
static void _onclick(MENUDLG dlg, DLGHDL hdl, int x, int y) {

	(void)dlg;
	(void)hdl;
	(void)x;
	(void)y;
}
#endif

static void _setval(MENUDLG dlg, DLGHDL hdl, int val) {

	(void)dlg;
	(void)hdl;
	(void)val;
}

static void _moverel(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus) {

	(void)dlg;
	(void)hdl;
	(void)x;
	(void)y;
	(void)focus;
}

typedef BOOL (*DLGCRE)(MENUDLG dlg, DLGHDL hdl, const void *arg);
typedef void (*DLGPAINT)(MENUDLG dlg, DLGHDL hdl);
typedef void (*DLGSETVAL)(MENUDLG dlg, DLGHDL hdl, int val);
typedef void (*DLGCLICK)(MENUDLG dlg, DLGHDL hdl, int x, int y);
typedef void (*DLGMOV)(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus);
typedef void (*DLGREL)(MENUDLG dlg, DLGHDL hdl, int x, int y, int focus);

static const DLGCRE dlgcre[] = {
		dlgbase_create,				// DLGTYPE_BASE
		_cre,						// DLGTYPE_CLOSE
		_cre_settext,				// DLGTYPE_BUTTON
		dlglist_create,				// DLGTYPE_LIST
		dlgslider_create,			// DLGTYPE_SLIDER
		dlgtablist_create,			// DLGTYPE_TABLIST
		_cre_settext,				// DLGTYPE_RADIO
		_cre_settext,				// DLGTYPE_CHECK
		_cre_settext,				// DLGTYPE_FRAME
		_cre_settext,				// DLGTYPE_EDIT
		_cre_settext,				// DLGTYPE_TEXT
		dlgicon_create,				// DLGTYPE_VRAM
		_cre,						// DLGTYPE_LINE
		_cre						// DLGTYPE_BOX
};

static const DLGPAINT dlgpaint[] = {
		dlgbase_paint,				// DLGTYPE_BASE
		dlgclose_paint,				// DLGTYPE_CLOSE
		dlgbtn_paint,				// DLGTYPE_BUTTON
		dlglist_paint,				// DLGTYPE_LIST
		dlgslider_paint,			// DLGTYPE_SLIDER
		dlgtablist_paint,			// DLGTYPE_TABLIST
		dlgradio_paint,				// DLGTYPE_RADIO
		dlgcheck_paint,				// DLGTYPE_CHECK
		dlgframe_paint,				// DLGTYPE_FRAME
		dlgedit_paint,				// DLGTYPE_EDIT
		dlgtext_paint,				// DLGTYPE_TEXT
		dlgicon_paint,				// DLGTYPE_VRAM
		dlgline_paint,				// DLGTYPE_LINE
		dlgbox_paint				// DLGTYPE_BOX
};

static const DLGSETVAL dlgsetval[] = {
		_setval,					// DLGTYPE_BASE
		_setval,					// DLGTYPE_CLOSE
		_setval,					// DLGTYPE_BUTTON
		dlglist_setval,				// DLGTYPE_LIST
		dlgslider_setval,			// DLGTYPE_SLIDER
		dlgtablist_setval,			// DLGTYPE_TABLIST
		dlgradio_setval,			// DLGTYPE_RADIO
		dlgcheck_setval				// DLGTYPE_CHECK
};

static const DLGCLICK dlgclick[] = {
		dlgbase_onclick,			// DLGTYPE_BASE
		dlgclose_onclick,			// DLGTYPE_CLOSE
		dlgbtn_onclick,				// DLGTYPE_BUTTON
		dlglist_onclick,			// DLGTYPE_LIST
		dlgslider_onclick,			// DLGTYPE_SLIDER
		dlgtablist_onclick,			// DLGTYPE_TABLIST
		dlgradio_onclick,			// DLGTYPE_RADIO
		dlgcheck_onclick			// DLGTYPE_CHECK
};

static const DLGMOV dlgmov[] = {
		dlgbase_move,				// DLGTYPE_BASE
		dlgclose_move,				// DLGTYPE_CLOSE
		dlgbtn_move,				// DLGTYPE_BUTTON
		dlglist_move,				// DLGTYPE_LIST
		dlgslider_move				// DLGTYPE_SLIDER
};

static const DLGREL dlgrel[] = {
		_moverel,					// DLGTYPE_BASE
		dlgclose_rel,				// DLGTYPE_CLOSE
		dlgbtn_rel,					// DLGTYPE_BUTTON
		dlglist_rel,				// DLGTYPE_LIST
		dlgslider_rel				// DLGTYPE_SLIDER
};


// ---- draw

static void draw(VRAMHDL dst, const RECT_T *rect, void *arg) {

	MENUDLG		dlg;

	dlg = (MENUDLG)arg;
	vrammix_cpy2(dst, dlg->vram, 2, rect);
}


typedef struct {
	MENUDLG	dlg;
	DLGHDL	hdl;
	RECT_T	rect;
} MDCB2;

static BOOL dc_cb(void *vpItem, void *vpArg) {

	DLGHDL	hdl;
	MDCB2	*mdcb;

	hdl = (DLGHDL)vpItem;
	mdcb = (MDCB2 *)vpArg;
	if (hdl == mdcb->hdl) {
		mdcb->hdl = NULL;
	}
	if ((mdcb->hdl != NULL) || (hdl->flag & MENU_DISABLE)) {
		goto dccb_exit;
	}
	if (rect_isoverlap(&mdcb->rect, &hdl->rect)) {
		hdl->flag |= MENU_REDRAW;
	}

dccb_exit:
	return(FALSE);
}


static BOOL dc_cb2(void *vpItem, void *vpArg) {

	MENUDLG	dlg;
	DLGHDL	hdl;

	hdl = (DLGHDL)vpItem;
	dlg = (MENUDLG)vpArg;
	if (hdl->flag & MENU_REDRAW) {
		hdl->flag &= ~MENU_REDRAW;
		if ((!(hdl->flag & MENU_DISABLE)) &&
			((unsigned int)hdl->type < (sizeof(dlgpaint)/sizeof(DLGPAINT)))) {
			dlgpaint[hdl->type](dlg, hdl);
			menubase_setrect(dlg->vram, &hdl->rect);
		}
	}
	return(FALSE);
}


static void drawctrls(MENUDLG dlg, DLGHDL hdl) {

	MDCB2	mdcb;

	if (hdl) {
		if (hdl->flag & MENU_DISABLE) {
			goto dcs_end;
		}
		mdcb.rect = hdl->rect;
	}
	else {
		mdcb.rect.left = 0;
		mdcb.rect.top = 0;
		mdcb.rect.right = dlg->vram->width;
		mdcb.rect.bottom = dlg->vram->height;
	}
	mdcb.dlg = dlg;
	mdcb.hdl = hdl;
	listarray_enum(dlg->dlg, dc_cb, &mdcb);
	if (!dlg->locked) {
		listarray_enum(dlg->dlg, dc_cb2, dlg);
		menubase_draw(draw, dlg);
	}

dcs_end:
	return;
}

static void drawlock(BOOL lock) {

	MENUDLG	dlg;

	dlg = &menudlg;
	if (lock) {
		dlg->locked++;
	}
	else {
		dlg->locked--;
		if (!dlg->locked) {
			listarray_enum(dlg->dlg, dc_cb2, dlg);
			menubase_draw(draw, dlg);
		}
	}
}


// ----

static int defproc(int msg, MENUID id) {

	if (msg == DLGMSG_CLOSE) {
		menubase_close();
	}
	(void)id;
	return(0);
}


BOOL menudlg_create(int width, int height, const char *str,
										int (*proc)(int msg, MENUID id)) {

	MENUBASE	*mb;
	MENUDLG		dlg;

	dlg = &menudlg;
	if (menubase_open(2) != SUCCESS) {
		goto mdcre_err;
	}
	ZeroMemory(dlg, sizeof(_MENUDLG));
	if ((width <= 0) || (height <= 0)) {
		goto mdcre_err;
	}
	width += (MENU_FBORDER + MENU_BORDER) * 2;
	height += ((MENU_FBORDER + MENU_BORDER) * 2) +
					MENUDLG_CYCAPTION + MENUDLG_BORDER;
	mb = &menubase;
	dlg->font = mb->font;
	dlg->vram = vram_create(width, height, FALSE, DEFAULT_BPP);
	if (dlg->vram == NULL) {
		goto mdcre_err;
	}
	dlg->vram->posx = (mb->width - width) >> 1;
	dlg->vram->posy = (mb->height - height) >> 1;
	dlg->dlg = listarray_new(sizeof(_DLGHDL), 32);
	if (dlg->dlg == NULL) {
		goto mdcre_err;
	}
	dlg->res = listarray_new(sizeof(_DLGPRM), 32);
	if (dlg->res == NULL) {
		goto mdcre_err;
	}
	if (menudlg_append(DLGTYPE_BASE, SID_CAPTION, 0, str,
										0, 0, width, height) != SUCCESS) {
		goto mdcre_err;
	}
	if (menudlg_append(DLGTYPE_CLOSE, SID_CLOSE, 0, NULL,
							width - (MENU_FBORDER + MENU_BORDER) -
									(MENUDLG_CXCLOSE + MENUDLG_PXCAPTION),
							(MENU_FBORDER + MENU_BORDER) +
								((MENUDLG_CYCAPTION - MENUDLG_CYCLOSE) / 2),
							MENUDLG_CXCLOSE, MENUDLG_CYCLOSE) != SUCCESS) {
		goto mdcre_err;
	}
	dlg->sx = (MENU_FBORDER + MENU_BORDER);
	dlg->sy = (MENU_FBORDER + MENU_BORDER) +
							(MENUDLG_CYCAPTION + MENUDLG_BORDER);
	if (proc == NULL) {
		proc = defproc;
	}
	dlg->proc = proc;
	dlg->locked = 0;
	drawlock(TRUE);
	proc(DLGMSG_CREATE, 0);
	drawctrls(dlg, NULL);
	drawlock(FALSE);

	return(SUCCESS);

mdcre_err:
	menubase_close();
	return(FAILURE);
}


static BOOL mdds_cb(void *vpItem, void *vpArg) {

	vram_destroy(((DLGHDL)vpItem)->vram);
	(void)vpArg;
	return(FALSE);
}


void menudlg_destroy(void) {

	MENUDLG		dlg;

	dlg = &menudlg;

	if (dlg->closing) {
		return;
	}
	dlg->closing = 1;
	dlg->proc(DLGMSG_DESTROY, 0);
	listarray_enum(dlg->dlg, mdds_cb, NULL);
	menubase_clrrect(dlg->vram);
	vram_destroy(dlg->vram);
	dlg->vram = NULL;
	listarray_destroy(dlg->dlg);
	dlg->dlg = NULL;
	listarray_destroy(dlg->res);
	dlg->res = NULL;
}


// ----

BOOL menudlg_appends(const MENUPRM *res, int count) {

	BOOL	r;

	r = SUCCESS;
	while(count--) {
		r |= menudlg_append(res->type, res->id, res->flg, res->arg,
							res->posx, res->posy, res->width, res->height);
		res++;
	}
	return(r);
}


BOOL menudlg_append(int type, MENUID id, MENUFLG flg, const void *arg,
								int posx, int posy, int width, int height) {

	MENUDLG		dlg;
	DLGHDL		hdl;
	_DLGHDL		dhdl;

	dlg = &menudlg;

	if (flg & MENU_TABSTOP) {
		dlg->group++;
	}
	switch(type) {
		case DLGTYPE_LTEXT:
			type = DLGTYPE_TEXT;
			flg &= ~MST_POSMASK;
			flg |= MST_LEFT;
			break;

		case DLGTYPE_CTEXT:
			type = DLGTYPE_TEXT;
			flg &= ~MST_POSMASK;
			flg |= MST_CENTER;
			break;

		case DLGTYPE_RTEXT:
			type = DLGTYPE_TEXT;
			flg &= ~MST_POSMASK;
			flg |= MST_RIGHT;
			break;
	}

	ZeroMemory(&dhdl, sizeof(dhdl));
	dhdl.type = type;
	dhdl.id = id;
	dhdl.flag = flg;
	dhdl.page = dlg->page;
	dhdl.group = dlg->group;
	dhdl.rect.left = dlg->sx + posx;
	dhdl.rect.top = dlg->sy + posy;
	dhdl.rect.right = dhdl.rect.left + width;
	dhdl.rect.bottom = dhdl.rect.top + height;
	dhdl.prm = NULL;
	dhdl.prmcnt = 0;
	dhdl.val = 0;
	if (((unsigned int)type >= (sizeof(dlgcre)/sizeof(DLGCRE))) ||
		(dlgcre[type](dlg, &dhdl, arg))) {
		goto mda_err;
	}
	drawlock(TRUE);
	hdl = (DLGHDL)listarray_append(dlg->dlg, &dhdl);
	drawctrls(dlg, hdl);
	drawlock(FALSE);
	return(SUCCESS);

mda_err:
	return(FAILURE);
}


// ---- moving

typedef struct {
	int		x;
	int		y;
	DLGHDL	ret;
} MDCB3;

static BOOL hps_cb(void *vpItem, void *vpArg) {

	DLGHDL	hdl;
	MDCB3	*mdcb;

	hdl = (DLGHDL)vpItem;
	mdcb = (MDCB3 *)vpArg;
	if ((!(hdl->flag & (MENU_DISABLE | MENU_GRAY))) &&
		(rect_in(&hdl->rect, mdcb->x, mdcb->y))) {
		mdcb->ret = hdl;
	}
	return(FALSE);
}

static DLGHDL hdlpossea(MENUDLG dlg, int x, int y) {

	MDCB3	mdcb;

	mdcb.x = x;
	mdcb.y = y;
	mdcb.ret = NULL;
	listarray_enum(dlg->dlg, hps_cb, &mdcb);
	return(mdcb.ret);
}

void menudlg_moving(int x, int y, int btn) {

	MENUDLG	dlg;
	DLGHDL	hdl;
	int		focus;

	drawlock(TRUE);
	dlg = &menudlg;
	x -= dlg->vram->posx;
	y -= dlg->vram->posy;
	if (!dlg->btn) {
		if (btn == 1) {
			hdl = hdlpossea(dlg, x, y);
			if (hdl) {
				x -= hdl->rect.left;
				y -= hdl->rect.top;
				dlg->btn = 1;
				dlg->lastid = hdl->id;
				if ((unsigned int)hdl->type <
									(sizeof(dlgclick)/sizeof(DLGCLICK))) {
					dlgclick[hdl->type](dlg, hdl, x, y);
				}
			}
		}
	}
	else {
		hdl = dlghdlsea(dlg, dlg->lastid);
		if (hdl) {
			focus = rect_in(&hdl->rect, x, y);
			x -= hdl->rect.left;
			y -= hdl->rect.top;
			if ((unsigned int)hdl->type < (sizeof(dlgmov)/sizeof(DLGMOV))) {
				dlgmov[hdl->type](dlg, hdl, x, y, focus);
			}
			if (btn == 2) {
				dlg->btn = 0;
				if ((unsigned int)hdl->type <
										(sizeof(dlgrel)/sizeof(DLGREL))) {
					dlgrel[hdl->type](dlg, hdl, x, y, focus);
				}
			}
		}
	}
	drawlock(FALSE);
}


// ---- ctrl

void *menudlg_msg(int ctrl, MENUID id, void *arg) {

	void	*ret;
	MENUDLG	dlg;
	DLGHDL	hdl;
	int		flg;

	ret = NULL;
	dlg = &menudlg;
	hdl = dlghdlsea(dlg, id);
	if (hdl == NULL) {
		goto mdm_exit;
	}
	drawlock(TRUE);
	switch(ctrl) {
		case DMSG_SETHIDE:
			ret = (void *)((hdl->flag & MENU_DISABLE)?1:0);
			flg = (arg)?MENU_DISABLE:0;
			if ((hdl->flag ^ flg) & MENU_DISABLE) {
				hdl->flag ^= MENU_DISABLE;
				if (flg) {
					drawctrls(dlg, NULL);
				}
				else {
					drawctrls(dlg, hdl);
				}
			}
			break;

		case DMSG_GETHIDE:
			ret = (void *)((hdl->flag & MENU_DISABLE)?1:0);
			break;

		case DMSG_SETENABLE:
			ret = (void *)((hdl->flag & MENU_GRAY)?0:1);
			flg = (arg)?0:MENU_GRAY;
			if ((hdl->flag ^ flg) & MENU_GRAY) {
				hdl->flag ^= MENU_GRAY;
				drawctrls(dlg, hdl);
			}
			break;

		case DMSG_GETENABLE:
			ret = (void *)((hdl->flag & MENU_GRAY)?0:1);
			break;

		case DMSG_SETVAL:
			ret = (void *)hdl->val;
			if ((unsigned int)hdl->type <
									(sizeof(dlgsetval)/sizeof(DLGSETVAL))) {
				dlgsetval[hdl->type](dlg, hdl, (int)arg);
			}
			break;

		case DMSG_GETVAL:
			ret = (void *)hdl->val;
			break;

		case DMSG_SETVRAM:
			if (hdl->type == DLGTYPE_VRAM) {
				ret = hdl->c.di.icon;
				hdl->c.di.icon = (VRAMHDL)arg;
				drawctrls(dlg, hdl);
			}
			break;

		case DMSG_SETTEXT:
			switch(hdl->type) {
				case DLGTYPE_RADIO:
				case DLGTYPE_CHECK:
				case DLGTYPE_EDIT:
				case DLGTYPE_TEXT:
					dlgtext_itemset(dlg, hdl, arg);
					drawctrls(dlg, hdl);
					break;
			}
			break;

		case DMSG_APPENDTEXT:
			switch(hdl->type) {
				case DLGTYPE_LIST:
					dlglist_append(dlg, hdl, arg);
					drawctrls(dlg, hdl);
					break;

				case DLGTYPE_TABLIST:
					dlgtablist_append(dlg, hdl, arg);
					drawctrls(dlg, hdl);
					break;
			}
			break;

		case DMSG_SETLISTPOS:
			if (hdl->type == DLGTYPE_LIST) {
				ret = (void *)(long)hdl->c.dl.basepos;
				dlglist_setbasepos(dlg, hdl, (int)arg);
				drawctrls(dlg, hdl);
			}
			break;

		case DMSG_GETRECT:
			ret = &hdl->rect;
			break;

		case DMSG_SETRECT:
			ret = &hdl->rect;
			if ((hdl->type == DLGTYPE_TEXT) && (arg)) {
				drawctrls(dlg, hdl);
				hdl->rect = *(RECT_T *)arg;
				drawctrls(dlg, hdl);
			}
			break;

		case DMSG_SETFONT:
			if (hdl->type == DLGTYPE_LIST) {
				ret = dlglist_setfont(hdl, arg);
				drawctrls(dlg, hdl);
			}
			else if (hdl->type == DLGTYPE_TABLIST) {
				ret = dlgtablist_setfont(hdl, arg);
				drawctrls(dlg, hdl);
			}
			else if (hdl->type == DLGTYPE_TEXT) {
				ret = hdl->c.dt.font;
				hdl->c.dt.font = arg;
				drawctrls(dlg, hdl);
			}
			break;

		case DMSG_GETFONT:
			if (hdl->type == DLGTYPE_LIST) {
				ret = hdl->c.dl.font;
			}
			else if (hdl->type == DLGTYPE_TABLIST) {
				ret = hdl->c.dtl.font;
			}
			else if (hdl->type == DLGTYPE_TEXT) {
				ret = hdl->c.dt.font;
			}
			break;

	}
	drawlock(FALSE);

mdm_exit:
	return(ret);
}


// --- page

void menudlg_setpage(MENUID page) {

	MENUDLG	dlg;

	dlg = &menudlg;
	dlg->page = page;
}


typedef struct {
	MENUID	page;
	MENUFLG	flag;
} MDCB4;

static BOOL mddph_cb(void *vpItem, void *vpArg) {

	DLGHDL	hdl;
	MDCB4	*mdcb;

	hdl = (DLGHDL)vpItem;
	mdcb = (MDCB4 *)vpArg;
	if ((hdl->page == mdcb->page) &&
		((hdl->flag ^ mdcb->flag) & MENU_DISABLE)) {
		hdl->flag ^= MENU_DISABLE;
	}
	return(FALSE);
}

void menudlg_disppagehidden(MENUID page, BOOL hidden) {

	MENUDLG	dlg;
	MDCB4	mdcb;

	dlg = &menudlg;
	mdcb.page = page;
	mdcb.flag = (hidden)?MENU_DISABLE:0;
	listarray_enum(dlg->dlg, mddph_cb, &mdcb);
	drawlock(TRUE);
	drawctrls(dlg, NULL);
	drawlock(FALSE);
}

