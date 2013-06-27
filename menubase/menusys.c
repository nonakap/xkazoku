#include	"compiler.h"
#include	"vram.h"
#include	"vrammix.h"
#include	"menudeco.inc"
#include	"menubase.h"
#include	"fontmng.h"


typedef struct _mhdl {
struct _mhdl	*chain;
struct _mhdl	*next;
struct _mhdl	*child;
		MENUID	id;
		MENUFLG	flag;
		RECT_T	rct;
		char	string[32];
} _MENUHDL, *MENUHDL;


typedef struct {
	VRAMHDL		vram;
	MENUHDL		menu;
	int			items;
	int			focus;
} _MSYSWND, *MSYSWND;

typedef struct {
	_MSYSWND	wnd[MENUSYS_MAX];
	LISTARRAY	res;
	MENUHDL		lastres;
	MENUHDL		root;
	VRAMHDL		icon;
	void		(*cmd)(MENUID id);
	int			depth;
	int			opened;
	int			lastdepth;
	int			lastpos;
	char		title[128];
} MENUSYS;


static MENUSYS	menusys;


static const BYTE str_sysr[] = {		// ���Υ��������᤹
		0x8c,0xb3,0x82,0xcc,0x83,0x54,0x83,0x43,0x83,0x59,0x82,0xc9,0x96,
		0xdf,0x82,0xb7,0x00};
static const BYTE str_sysm[] = {		// ��ư
		0x88,0xda,0x93,0xae,0x00};
static const BYTE str_syss[] = {		// �������ѹ�
		0x83,0x54,0x83,0x43,0x83,0x59,0x95,0xcf,0x8d,0x58,0x00};
static const BYTE str_sysn[] = {		// �Ǿ���
		0x8d,0xc5,0x8f,0xac,0x89,0xbb,0x00};
static const BYTE str_sysx[] = {		// ���粽
		0x8d,0xc5,0x91,0xe5,0x89,0xbb,0x00};
static const BYTE str_sysc[] = {		// �Ĥ���
		0x95,0xc2,0x82,0xb6,0x82,0xe9,0x00};


static const MSYSITEM s_exit[] = {
			{(char *)str_sysr,	NULL,		0,			MENU_GRAY},
			{(char *)str_sysm,	NULL,		0,			MENU_GRAY},
			{(char *)str_syss,	NULL,		0,			MENU_GRAY},
			{(char *)str_sysn,	NULL,		0,			MENU_GRAY},
			{(char *)str_sysx,	NULL,		0,			MENU_GRAY},
			{NULL,				NULL,		0,			MENU_SEPARATOR},
			{(char *)str_sysc,	NULL,		SID_CLOSE,	MENU_DELETED}};

static const MSYSITEM s_root[2] = {
			{NULL,				s_exit,		0,			MENUS_SYSTEM},
			{NULL,				NULL,		SID_CLOSE,	MENUS_CLOSE
														| MENU_DELETED}};


// ---- regist

static BOOL seaempty(void *vpItem, void *vpArg) {

	if (((MENUHDL)vpItem)->flag & MENU_DELETED) {
		return(TRUE);
	}
	(void)vpArg;
	return(FALSE);
}


static MENUHDL append1(MENUSYS *sys, const MSYSITEM *item) {

	MENUHDL		ret;
	_MENUHDL	hdl;

	ZeroMemory(&hdl, sizeof(hdl));
	hdl.id = item->id;
	hdl.flag = item->flag & (~MENU_DELETED);
	if (item->string) {
		milstr_ncpy(hdl.string, item->string, sizeof(hdl.string));
	}
	ret = (MENUHDL)listarray_enum(sys->res, seaempty, NULL);
	if (ret) {
		*ret = hdl;
	}
	else {
		ret = (MENUHDL)listarray_append(sys->res, &hdl);
	}
	if (ret) {
		if (sys->lastres) {
			sys->lastres->chain = ret;
		}
		sys->lastres = ret;
	}
	return(ret);
}


static MENUHDL appends(MENUSYS *sys, const MSYSITEM *item) {

	MENUHDL		ret;
	MENUHDL		cur;

	ret = append1(sys, item);
	cur = ret;
	while(1) {
		if (cur == NULL) {
			goto ap_err;
		}
		if (item->child) {
			cur->child = appends(sys, item->child);
		}
		if (item->flag & MENU_DELETED) {
			break;
		}
		item++;
		cur->next = append1(sys, item);
		cur = cur->next;
	}
	return(ret);

ap_err:
	return(NULL);
}


// ----

static void draw(VRAMHDL dst, const RECT_T *rect, void *arg) {

	MENUSYS		*sys;
	int			cnt;
	MSYSWND		wnd;

	sys = &menusys;
	wnd = sys->wnd;
	cnt = sys->depth;
	while(cnt--) {
		vrammix_cpy2(dst, wnd->vram, 2, rect);
		wnd++;
	}
	(void)arg;
}


static MENUHDL getitem(MENUSYS *sys, int depth, int pos) {

	MENUHDL	ret;

	if ((unsigned int)depth >= (unsigned int)sys->depth) {
		goto gi_err;
	}
	ret = sys->wnd[depth].menu;
	while(ret) {
		if (!pos) {
			if (!(ret->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
				return(ret);
			}
			else {
				break;
			}
		}
		pos--;
		ret = ret->next;
	}

gi_err:
	return(NULL);
}


static void wndclose(MENUSYS *sys, int depth) {

	MSYSWND		wnd;

	sys->depth = depth;
	wnd = sys->wnd + depth;
	while(depth < MENUSYS_MAX) {
		menubase_clrrect(wnd->vram);
		vram_destroy(wnd->vram);
		wnd->vram = NULL;
		wnd++;
		depth++;
	}
}


static void bitemdraw(VRAMHDL vram, MENUHDL menu, int flag) {

	void	*font;
	POINT_T	pt;
	UINT32	color;
	int		pos;
	int		menutype;

	font = menubase.font;
	menutype = menu->flag & MENUS_CTRLMASK;
	if (menutype == 0) {
		vram_filldat(vram, &menu->rct, menucolor[MVC_STATIC]);
		pos = 0;
		if (flag) {
			pos = 1;
		}
		if (!(menu->flag & MENU_GRAY)) {
			color = menucolor[MVC_TEXT];
		}
		else {
#if 0
			if (flag == 2) {
				flag = 0;
				pos = 0;
			}
#endif
			pt.x = menu->rct.left + pos + MENUSYS_SXSYS + MENU_DSTEXT;
			pt.y = menu->rct.top + pos + MENUSYS_SYSYS + MENU_DSTEXT;
			vrammix_text(vram, font, menu->string,
									menucolor[MVC_GRAYTEXT2], &pt, NULL);
			color = menucolor[MVC_GRAYTEXT1];
		}
		pt.x = menu->rct.left + pos + MENUSYS_SXSYS;
		pt.y = menu->rct.top + pos + MENUSYS_SYSYS;
		vrammix_text(vram, font, menu->string, color, &pt, NULL);
		if (flag) {
			menuvram_box(vram, &menu->rct,
								MVC2(MVC_SHADOW, MVC_HILIGHT), (flag==2));
		}
	}
}


static BOOL wndopenbase(MENUSYS *sys) {

	MENUHDL menu;
	RECT_T	mrect;
	int		items;
	int		posx;
	int		rootflg;
	int		menutype;
	int		height;
	POINT_T	pt;

	wndclose(sys, 0);

	rootflg = 0;
	menu = sys->root;
	while(menu) {					// ��˥塼���Ƥ�Ĵ�٤롣
		if (!(menu->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
			switch(menu->flag & MENUS_CTRLMASK) {
				case MENUS_POPUP:
					break;

				case MENUS_SYSTEM:
					rootflg |= 1;
					break;

				case MENUS_CLOSE:
					rootflg |= 2;
					break;

				default:
					rootflg |= 4;
					break;
			}
		}
		menu = menu->next;
	}

	mrect.left = MENU_FBORDER + MENU_BORDER;
	mrect.top = MENU_FBORDER + MENU_BORDER;
	mrect.right = menubase.width - (MENU_FBORDER + MENU_BORDER);
	mrect.bottom = (MENU_FBORDER + MENU_BORDER) + MENUSYS_CYCAPTION;
	height = ((MENU_FBORDER + MENU_BORDER) * 2) + MENUSYS_CYCAPTION;
	if (rootflg & 4) {
		height += (MENUSYS_BCAPTION * 3) + MENUSYS_CYSYS;
		mrect.left += MENUSYS_BCAPTION;
		mrect.top += MENUSYS_BCAPTION;
		mrect.right -= MENUSYS_BCAPTION;
		mrect.bottom += MENUSYS_BCAPTION;
	}
	sys->wnd[0].vram = menuvram_create(menubase.width, height);
	if (sys->wnd[0].vram == NULL) {
		goto wopn0_err;
	}
	menuvram_caption(sys->wnd[0].vram, &mrect, sys->icon, sys->title);
	menubase_setrect(sys->wnd[0].vram, NULL);
	menu = sys->root;
	sys->wnd[0].menu = menu;
	sys->wnd[0].focus = -1;
	sys->depth++;
	items = 0;
	posx = MENU_FBORDER + MENU_BORDER + MENUSYS_BCAPTION;
	while(menu) {
		if (!(menu->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
			menutype = menu->flag & MENUS_CTRLMASK;
			if (menutype == MENUS_POPUP) {
			}
			else if (menutype == MENUS_SYSTEM) {
				menu->rct.left = mrect.left + MENU_PXCAPTION;
				menu->rct.right = menu->rct.left;
				menu->rct.top = mrect.top + MENU_PYCAPTION;
				menu->rct.bottom = menu->rct.top;
				if (sys->icon) {
#ifndef SIZE_QVGA
					menu->rct.right += sys->icon->width;
					menu->rct.bottom += sys->icon->height;
#else
					menu->rct.right += sys->icon->width * 2;
					menu->rct.bottom += sys->icon->height * 2;
#endif
				}
			}
			else if (menutype == MENUS_CLOSE) {
				menu->rct.right = mrect.right - MENU_PXCAPTION;
				menu->rct.left = menu->rct.right - MENUSYS_CXCLOSE;
				menu->rct.top = mrect.top +
								((MENUSYS_CYCAPTION - MENUSYS_CYCLOSE) / 2);
				menu->rct.bottom = menu->rct.top + MENUSYS_CYCLOSE;
				menuvram_closebtn(sys->wnd[0].vram, &menu->rct, 0);
			}
			else {
				menu->rct.left = posx;
				menu->rct.top = mrect.bottom + MENUSYS_BCAPTION;
				menu->rct.bottom = menu->rct.top + MENUSYS_CYSYS;
				fontmng_getsize(menubase.font, menu->string, &pt);
				posx += MENUSYS_SXSYS + pt.x + MENUSYS_LXSYS;
				if (posx >= (menubase.width -
						(MENU_FBORDER + MENU_BORDER + MENUSYS_BCAPTION))) {
					break;
				}
				menu->rct.right = posx;
				bitemdraw(sys->wnd[0].vram, menu, 0);
			}
		}
		items++;
		menu = menu->next;
	}
	sys->wnd[0].items = items;
	return(SUCCESS);

wopn0_err:
	return(FAILURE);
}


// ----

static void citemdraw2(VRAMHDL vram, MENUHDL menu, UINT mvc, int pos) {

	MENURES2	*res;
	POINT_T		pt;

	res = menures_sys;
	if (menu->flag & MENU_CHECKED) {
		pt.x = menu->rct.left + MENUSYS_SXITEM + pos,
		pt.y = menu->rct.top + pos;
		menuvram_res3put(vram, res, &pt, mvc);
	}
	if (menu->child) {
		pt.x = menu->rct.right - MENUSYS_SXITEM - res[1].width + pos,
		pt.y = menu->rct.top + pos;
		menuvram_res3put(vram, res+1, &pt, mvc);
	}
}


static void citemdraw(VRAMHDL vram, MENUHDL menu, int flag) {

	POINT_T	pt;
	void	*font;
	int		left;
	int		right;
	int		top;
	UINT32	txtcol;

	vram_filldat(vram, &menu->rct, (flag != 0)?0x000080:0xc0c0c0);

	if (menu->flag & MENU_SEPARATOR) {
		left = menu->rct.left + MENUSYS_SXSEP;
		right = menu->rct.right - MENUSYS_LXSEP;
		top = menu->rct.top + MENUSYS_SYSEP;
		menuvram_linex(vram, left, top, right, MVC_SHADOW);
		menuvram_linex(vram, left, top + MENU_LINE, right, MVC_HILIGHT);
	}
	else {
		left = menu->rct.left + MENUSYS_SXITEM + MENUSYS_CXCHECK;
		top = menu->rct.top + MENUSYS_SYITEM;
		font = menubase.font;
		if (!(menu->flag & MENU_GRAY)) {
			txtcol = (flag != 0)?MVC_CURTEXT:MVC_TEXT;
		}
		else {
			if (flag == 0) {
				pt.x = left + MENU_DSTEXT;
				pt.y = top + MENU_DSTEXT;
				vrammix_text(vram, font, menu->string,
										menucolor[MVC_GRAYTEXT2], &pt, NULL);
				citemdraw2(vram, menu, MVC_GRAYTEXT2, 1);
			}
			txtcol = MVC_GRAYTEXT1;
		}
		pt.x = left;
		pt.y = top;
		vrammix_text(vram, font, menu->string, menucolor[txtcol], &pt, NULL);
		citemdraw2(vram, menu, txtcol, 0);
	}
}


static void childopn(MENUSYS *sys, int depth, int pos) {

	MENUHDL	menu;
	int		posx;
	int		posy;
	int		width;
	int		height;
	int		items;
	MSYSWND	wnd;
	int		drawitems;
	POINT_T	pt;

	menu = getitem(sys, depth, pos);
	if ((menu == NULL) || (menu->child == NULL)) {
		TRACEOUT(("child not found."));
		goto copn_end;
	}
	wnd = sys->wnd + depth;
	posx = wnd->vram->posx;
	posy = wnd->vram->posy;
	if (!depth) {
		if ((menu->flag & MENUS_CTRLMASK) == MENUS_POPUP) {
			posx = 0;
			posy = wnd->vram->height;
		}
		else {
			posx += menu->rct.left;
			posy += menu->rct.bottom;
		}
	}
	else {
		posx += menu->rct.right;
		posy += menu->rct.top;
	}
	if (depth >= (MENUSYS_MAX - 1)) {
		TRACEOUT(("menu max."));
		goto copn_end;
	}
	wnd++;
	width = 0;
	height = (MENU_FBORDER + MENU_BORDER);
	items = 0;
	drawitems = 0;
	menu = menu->child;
	wnd->menu = menu;
	while(menu) {
		if (!(menu->flag & MENU_DISABLE)) {
			menu->rct.left = (MENU_FBORDER + MENU_BORDER);
			menu->rct.top = height;
			if (menu->flag & MENU_SEPARATOR) {
				if (height > (menubase.height - MENUSYS_CYSEP -
											(MENU_FBORDER + MENU_BORDER))) {
					break;
				}
				height += MENUSYS_CYSEP;
				menu->rct.bottom = height;
			}
			else {
				if (height > (menubase.height - MENUSYS_CYITEM -
											(MENU_FBORDER + MENU_BORDER))) {
					break;
				}
				height += MENUSYS_CYITEM;
				menu->rct.bottom = height;
				fontmng_getsize(menubase.font, menu->string, &pt);
				if (width < pt.x) {
					width = pt.x;
				}
			}
		}
		items++;
		menu = menu->next;
	}
	width += ((MENU_FBORDER + MENU_BORDER + MENUSYS_SXITEM) * 2) +
										MENUSYS_CXCHECK + MENUSYS_CXNEXT;
	if (width >= menubase.width) {
		width = menubase.width;
	}
	height += (MENU_FBORDER + MENU_BORDER);
	wnd->vram = menuvram_create(width, height);
	if (wnd->vram == NULL) {
		TRACEOUT(("sub menu vram couldn't create"));
		goto copn_end;
	}
	wnd->vram->posx = min(posx, menubase.width - width);
	wnd->vram->posy = min(posy, menubase.height - height);
	wnd->items = items;
	wnd->focus = -1;
	sys->depth++;
	menu = wnd->menu;
	drawitems = items;
	while(drawitems--) {
		if (!(menu->flag & MENU_DISABLE)) {
			menu->rct.right = width - (MENU_FBORDER + MENU_BORDER);
			citemdraw(wnd->vram, menu, 0);
		}
		menu = menu->next;
	}
	menubase_setrect(wnd->vram, NULL);

copn_end:
	return;
}


static int openpopup(MENUSYS *sys) {

	MENUHDL menu;
	int		pos;

	if (sys->depth == 1) {
		pos = 0;
		menu = sys->root;
		while(menu) {
			if (!(menu->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
				if ((menu->flag & MENUS_CTRLMASK) == MENUS_POPUP) {
					childopn(sys, 0, pos);
					return(1);
				}
			}
			menu = menu->next;
			pos++;
		}
	}
	return(0);
}


static void itemdraw(MENUSYS *sys, int depth, int pos, int flag) {

	MENUHDL		menu;
	VRAMHDL	vram;
	void		(*drawfn)(VRAMHDL vram, MENUHDL menu, int flag);

	menu = getitem(sys, depth, pos);
	if (menu) {
		vram = sys->wnd[depth].vram;
		drawfn = (depth)?citemdraw:bitemdraw;
		drawfn(vram, menu, flag);
		menubase_setrect(vram, &menu->rct);
	}
}


// ----

typedef struct {
	int		depth;
	int		pos;
	MSYSWND	wnd;
	MENUHDL	menu;
} MENUPOS;

static void getposinfo(MENUSYS *sys, MENUPOS *pos, int x, int y) {

	RECT_T		rct;
	int			cnt;
	MSYSWND		wnd;
	MENUHDL		menu;

	cnt = sys->depth;
	wnd = sys->wnd + cnt;
	while(cnt--) {
		wnd--;
		if (wnd->vram) {
			vram_getrect(wnd->vram, &rct);
			if (rect_in(&rct, x, y)) {
				x -= wnd->vram->posx;
				y -= wnd->vram->posy;
				break;
			}
		}
	}
	if (cnt >= 0) {
		pos->depth = cnt;
		pos->wnd = wnd;
		menu = wnd->menu;
		cnt = 0;
		while((menu) && (cnt < wnd->items)) {
			if (!(menu->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
				if (rect_in(&menu->rct, x, y)) {
					pos->pos = cnt;
					pos->menu = menu;
					return;
				}
			}
			cnt++;
			menu = menu->next;
		}
	}
	else {
		pos->depth = -1;
		pos->wnd = NULL;
	}
	pos->pos = -1;
	pos->menu = NULL;
}


// ----

static void defcmd(MENUID id) {

	(void)id;
}


BOOL menusys_create(const MSYSITEM *item, VRAMHDL icon,
								void (*cmd)(MENUID id), const char *title) {

	MENUSYS		*ret;
	LISTARRAY	r;
	MENUHDL		hdl;

	ret = &menusys;
	ZeroMemory(ret, sizeof(MENUSYS));
	ret->icon = icon;
	if (cmd == NULL) {
		cmd = defcmd;
	}
	ret->cmd = cmd;
	if (title) {
		milstr_ncpy(ret->title, title, sizeof(ret->title));
	}
	r = listarray_new(sizeof(_MENUHDL), 32);
	if (r == NULL) {
		goto mscre_err;
	}
	ret->res = r;
	hdl = appends(ret, s_root);
	if (hdl == NULL) {
		goto mscre_err;
	}
	ret->root = hdl;
	if (item) {
		while(hdl->next) {
			hdl = hdl->next;
		}
		hdl->next = appends(ret, item);
	}
	return(SUCCESS);

mscre_err:
	return(FAILURE);
}


void menusys_destroy(void) {

	MENUSYS	*sys;

	sys = &menusys;
	wndclose(sys, 0);
	if (sys->res) {
		listarray_destroy(sys->res);
	}
}


BOOL menusys_open(void) {

	MENUSYS	*sys;

	sys = &menusys;

	if (menubase_open(1) != SUCCESS) {
		goto msopn_err;
	}
	sys->opened = 0;
	sys->lastdepth = -1;
	sys->lastpos = -1;
	if (wndopenbase(sys) != SUCCESS) {
		goto msopn_err;
	}
	sys->opened = openpopup(sys);
	menubase_draw(draw, sys);
	return(SUCCESS);

msopn_err:
	menubase_close();
	return(FAILURE);
}


void menusys_close(void) {

	MENUSYS	*sys;

	sys = &menusys;
	wndclose(sys, 0);
}


void menusys_moving(int x, int y, int btn) {

	MENUSYS	*sys;
	MENUPOS	cur;
	int		topwnd;

	sys = &menusys;
	getposinfo(sys, &cur, x, y);

	// ��˥塼���Ĥ����
	if (cur.depth < 0) {
		if (btn == 2) {
			menubase_close();
			return;
		}
	}
	topwnd = sys->depth - 1;
	if (cur.menu != NULL) {
		if (cur.wnd->focus != cur.pos) {
			if (sys->opened) {
				if (cur.depth != topwnd) {
					wndclose(sys, cur.depth + 1);
				}
				if ((!(cur.menu->flag & MENU_GRAY)) &&
					(cur.menu->child != NULL)) {
					childopn(sys, cur.depth, cur.pos);
				}
			}
			itemdraw(sys, cur.depth, cur.wnd->focus, 0);
			itemdraw(sys, cur.depth, cur.pos, 2 - sys->opened);
			cur.wnd->focus = cur.pos;
		}
		if (!(cur.menu->flag & MENU_GRAY)) {
			if (btn == 1) {
				if ((!sys->opened) && (cur.depth == 0) &&
					(cur.menu->child != NULL)) {
					wndclose(sys, 1);
					itemdraw(sys, 0, cur.pos, 1);
					childopn(sys, 0, cur.pos);
					sys->opened = 1;
				}
			}
			else if (btn == 2) {
				if (cur.menu->id) {
					menubase_close();
					sys->cmd(cur.menu->id);
					return;
				}
			}
		}
	}
	else {
		if ((btn == 1) && (cur.depth == 0)) {
			wndclose(sys, 1);
			itemdraw(sys, 0, cur.wnd->focus, 0);
			sys->opened = openpopup(sys);
		}
		else if (cur.depth != topwnd) {
			cur.depth = topwnd;
			cur.pos = -1;
			cur.wnd = sys->wnd + cur.depth;
			if (cur.wnd->focus != cur.pos) {
				itemdraw(sys, cur.depth, cur.wnd->focus, 0);
				cur.wnd->focus = cur.pos;
			}
		}
	}
	menubase_draw(draw, sys);
	return;
}


// ----

typedef struct {
	MENUHDL		ret;
	MENUID		id;
} ITEMSEA;

static BOOL _itemsea(void *vpItem, void *vpArg) {

	if (((MENUHDL)vpItem)->id == ((ITEMSEA *)vpArg)->id) {
		((ITEMSEA *)vpArg)->ret = (MENUHDL)vpItem;
		return(TRUE);
	}
	return(FALSE);
}


static MENUHDL itemsea(MENUSYS *sys, MENUID id) {

	ITEMSEA		sea;

	sea.ret = NULL;
	sea.id = id;
	listarray_enum(sys->res, _itemsea, &sea);
	return(sea.ret);
}


static void menusys_setflag(MENUID id, MENUFLG flag, MENUFLG mask) {

	MENUSYS	*sys;
	MENUHDL	itm;
	int		depth;
	int		pos;
	int		focus;

	sys = &menusys;
	itm = itemsea(sys, id);
	if (itm == NULL) {
		goto mssf_end;
	}
	flag ^= itm->flag;
	flag &= mask;
	if (!flag) {
		goto mssf_end;
	}
	itm->flag ^= flag;

	// ��ɥ�����ɬ�ס�
	depth = 0;
	while(depth < sys->depth) {
		itm = sys->wnd[depth].menu;
		pos = 0;
		while(itm) {
			if (itm->id == id) {
				if (!(itm->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
					focus = 0;
					if (sys->wnd[depth].focus == pos) {
						focus = 2 - sys->opened;
					}
					itemdraw(sys, depth, pos, focus);
					menubase_draw(draw, sys);
					goto mssf_end;
				}
			}
			pos++;
			itm = itm->next;
		}
		depth++;
	}

mssf_end:
	return;
}


static void menusys_settxt(MENUID id, void *arg) {

	MENUSYS	*sys;
	MENUHDL	itm;
	int		depth;
	int		pos;
	int		focus;

	sys = &menusys;
	itm = itemsea(sys, id);
	if (itm == NULL) {
		goto msst_end;
	}

	if (arg) {
		milstr_ncpy(itm->string, (char *)arg, sizeof(itm->string));
	}
	else {
		itm->string[0] = '\0';
	}

	// ��ɥ�����ɬ�ס� (ToDo: �ƥ����ץ󤹤٤�)
	depth = 0;
	while(depth < sys->depth) {
		itm = sys->wnd[depth].menu;
		pos = 0;
		while(itm) {
			if (itm->id == id) {
				if (!(itm->flag & (MENU_DISABLE | MENU_SEPARATOR))) {
					focus = 0;
					if (sys->wnd[depth].focus == pos) {
						focus = 2 - sys->opened;
					}
					itemdraw(sys, depth, pos, focus);
					menubase_draw(draw, sys);
					goto msst_end;
				}
			}
			pos++;
			itm = itm->next;
		}
		depth++;
	}

msst_end:
	return;
}


void *menusys_msg(int ctrl, MENUID id, void *arg) {

	void	*ret;
	MENUSYS	*sys;
	MENUHDL	itm;

	ret = NULL;
	sys = &menusys;
	itm = itemsea(sys, id);
	if (itm == NULL) {
		goto msmsg_exit;
	}

	switch(ctrl) {
		case SMSG_SETHIDE:
			ret = (void *)((itm->flag & MENU_DISABLE)?1:0);
			menusys_setflag(id,
							(MENUFLG)((arg)?MENU_DISABLE:0), MENU_DISABLE);
			break;

		case SMSG_GETHIDE:
			ret = (void *)((itm->flag & MENU_DISABLE)?1:0);
			break;

		case SMSG_SETENABLE:
			ret = (void *)((itm->flag & MENU_GRAY)?0:1);
			menusys_setflag(id, (MENUFLG)((arg)?0:MENU_GRAY), MENU_GRAY);
			break;

		case SMSG_GETENABLE:
			ret = (void *)((itm->flag & MENU_GRAY)?0:1);
			break;

		case SMSG_SETCHECK:
			ret = (void *)((itm->flag & MENU_CHECKED)?1:0);
			menusys_setflag(id,
							(MENUFLG)((arg)?MENU_CHECKED:0), MENU_CHECKED);
			break;

		case SMSG_GETCHECK:
			ret = (void *)((itm->flag & MENU_CHECKED)?1:0);
			break;

		case SMSG_SETTEXT:
			menusys_settxt(id, arg);
			break;
	}

msmsg_exit:
	return(ret);
}
