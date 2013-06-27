#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cgload.h"


#if defined(__GNUC__)
typedef struct {
	char	sig[4];
	BYTE	size[4];
	BYTE	unknown[4];
	BYTE	frames[2];
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	width[2];
	BYTE	height[2];
} __attribute__ ((packed)) GAD_HEAD;
typedef struct {
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	tick[4];
} __attribute__ ((packed)) GAD_DATA;
#else /* !__GNUC__ */
#pragma pack(push, 1)
typedef struct {
	char	sig[4];
	BYTE	size[4];
	BYTE	unknown[4];
	BYTE	frames[2];
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	width[2];
	BYTE	height[2];
} GAD_HEAD;
typedef struct {
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	tick[4];
} GAD_DATA;
#pragma pack(pop)
#endif /* __GNUC__ */


GADHDL gad_create(UINT type, const char *fname) {

	GADHDL		ret;
	GAD_HEAD	head;
	ARCFILEH	hdl;
	UINT		size;
	UINT		cnt;

	hdl = arcfile_open(type, fname);
	if (hdl == NULL) {
		TRACEOUT(("GAD: notfound"));
		goto gcre_err1;
	}
	if (arcfile_read(hdl, &head, sizeof(head)) != sizeof(head)) {
		TRACEOUT(("GAD: head read err"));
		goto gcre_err2;
	}
	if (memcmp(head.sig, "GAD ", 4)) {
		TRACEOUT(("GAD: sig err [%s]", head.sig));
		goto gcre_err2;
	}

	size = LOADINTELDWORD(head.size);
	cnt = LOADINTELWORD(head.frames);
	size -= sizeof(head);
	size /= sizeof(GAD_DATA);
	cnt = min(cnt, size);

	size = sizeof(_GADHDL);
	size += cnt * sizeof(GAD_DATA);
	ret = (GADHDL)_MALLOC(size, fname);
	if (ret == NULL) {
		goto gcre_err2;
	}
	ZeroMemory(ret, size);
	ret->frames = cnt;
	ret->pt.x = LOADINTELWORD(head.posx);
	ret->pt.y = LOADINTELWORD(head.posy);
	ret->width = LOADINTELWORD(head.width);
	ret->height = LOADINTELWORD(head.height);
	cnt *= sizeof(GAD_DATA);
	if (arcfile_read(hdl, ret + 1, cnt) != cnt) {
		goto gcre_err3;
	}
	arcfile_close(hdl);
	return(ret);

gcre_err3:
	_MFREE(ret);

gcre_err2:
	arcfile_close(hdl);

gcre_err1:
	return(NULL);
}

void gad_destroy(GADHDL hdl) {

	if (hdl) {
		vram_destroy(hdl->vram);
		_MFREE(hdl);
	}
}


// ----

void anime_exec(void) {

	ANIME		anime;
	GADHDL		hdl;
	GAD_DATA	*dat;
	UINT32		tick;
	RECT_U		drct;
	RECT_T		srct;
	DISPWIN		dispwin;
	VRAMHDL		dst;

	anime = &gamecore.anime;
	if (anime->enable == FALSE) {
		goto aexe_exit;
	}
	tick = GETTICK() - anime->basetick;
	if (tick < anime->nexttick) {
		goto aexe_exit;
	}
	hdl = anime->hdl;
	if ((hdl == NULL) ||
		(anime->frame >= hdl->frames)) {
		anime->enable = FALSE;
		TRACEOUT(("anime error"));
		goto aexe_exit;
	}
	TRACEOUT(("anime disp: %d", anime->frame));

	dat = (GAD_DATA *)(hdl + 1);
	dat += anime->frame;
	scr_valset(0, anime->frame);
	anime->frame++;
	anime->nexttick += LOADINTELDWORD(dat->tick);

	dispwin = &gamecore.dispwin;
	if (!(dispwin->flag & DISPWIN_VRAM)) {
		goto aexe_exit;
	}
	dst = gamecore.vram[dispwin->vramnum];

	srct.left = LOADINTELWORD(dat->posx);
	srct.top = LOADINTELWORD(dat->posy);
	srct.right = srct.left + hdl->width;
	srct.bottom = srct.top + hdl->height;
	drct.r.left = hdl->pt.x;
	drct.r.top = hdl->pt.y;
	drct.r.right = drct.r.left + hdl->width;
	drct.r.bottom = drct.r.top + hdl->height;
#ifdef SIZE_QVGA
	vramdraw_halfrect(&srct);
	vramdraw_halfrect(&drct.r);
#endif
	vramcpy_cpy(dst, hdl->vram, &drct.p, &srct);
	vramdraw_setrect(dst, &drct.r);
	vramdraw_draw();

aexe_exit:
	return;
}

