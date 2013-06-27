#include	"compiler.h"
#include	"dosio.h"
#include	"taskmng.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cgload.h"


#if defined(__GNUC__)
typedef struct {
	char	sig[4];
	BYTE	size[4];
	BYTE	pat[2];
} __attribute__ ((packed)) GADHEAD;
typedef struct {
	BYTE	major;
	BYTE	minor;
	BYTE	frames[2];
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	width[2];
	BYTE	height[2];
} __attribute__ ((packed)) GADTBL;
typedef struct {
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	tick[4];
} __attribute__ ((packed)) GADDATA;
typedef struct {
	char	sig[8];
	BYTE	scr[4];
	BYTE	pic[4];
} __attribute__ ((packed)) GANHEAD;
typedef struct {
	BYTE	cmd[4];
	BYTE	prm[4];
} __attribute__ ((packed)) GANSCR;
typedef struct {
	BYTE	id[4];
	BYTE	ref[4];
	BYTE	pos[4];
	BYTE	size[4];
} __attribute__ ((packed)) GANFILE;
#else /* !__GNUC__ */
#pragma pack(push, 1)
typedef struct {
	char	sig[4];
	BYTE	size[4];
	BYTE	pat[2];
} GADHEAD;
typedef struct {
	BYTE	major;
	BYTE	minor;
	BYTE	frames[2];
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	width[2];
	BYTE	height[2];
} GADTBL;
typedef struct {
	BYTE	posx[2];
	BYTE	posy[2];
	BYTE	tick[4];
} GADDATA;
typedef struct {
	char	sig[8];
	BYTE	scr[4];
	BYTE	pic[4];
} GANHEAD;
typedef struct {
	BYTE	cmd[4];
	BYTE	prm[4];
} GANSCR;
typedef struct {
	BYTE	id[4];
	BYTE	ref[4];
	BYTE	pos[4];
	BYTE	size[4];
} GANFILE;
#pragma pack(pop)
#endif /* __GNUC__ */


static GADFILE gadfile_create(const char *fname) {

	GADFILE		ret;
	GADHEAD		head;
	ARCFILEH	hdl;
	int			size;

	hdl = arcfile_open(ARCTYPE_GRAPHICS, fname);
	if (hdl == NULL) {
		TRACEOUT(("GAD: %s not found", fname));
		goto gcre_err1;
	}
	if (arcfile_read(hdl, &head, sizeof(head)) != sizeof(head)) {
		TRACEOUT(("GAD: head read err"));
		goto gcre_err2;
	}
	if (memcmp(head.sig, "GAD ", 4)) {
		goto gcre_err2;
	}
	size = LOADINTELDWORD(head.size);
	size -= sizeof(head);
	if (size <= 0) {
		goto gcre_err2;
	}
	ret = (GADFILE)_MALLOC(sizeof(_GADFILE) + size, fname);
	if (ret == NULL) {
		goto gcre_err2;
	}
	ret->size = size;
	ret->pat = LOADINTELWORD(head.pat);
	ret->vram = NULL;
	if (arcfile_read(hdl, ret + 1, size) != (UINT)size) {
		goto gcre_err3;
	}
	arcfile_close(hdl);
	cgload_data(&ret->vram, ARCTYPE_GRAPHICS, fname);
	if (ret->vram == NULL) {
		goto gcre_err3;
	}
	return(ret);

gcre_err3:
	_MFREE(ret);

gcre_err2:
	arcfile_close(hdl);

gcre_err1:
	return(NULL);
}

static void gadfile_destroy(GADFILE hdl) {

	if (hdl) {
		_MFREE(hdl);
	}
}


// ---- GAD

static void gad_open(ANIME anime, int gad, const char *fname) {

	GADFILE	hdl;
	char	label[ARCFILENAME_LEN+1];

	if ((gad < 0) || (gad >= GAMECORE_MAXGADFILE)) {
		goto ago_exit;
	}
	milstr_ncpy(label, fname, sizeof(label));
	milstr_ncat(label, ".GAD", sizeof(label));
	hdl = gadfile_create(label);
	if (hdl == NULL) {
		milstr_ncpy(label, fname, sizeof(label));
		milstr_ncat(label, ".DGA", sizeof(label));
		hdl = gadfile_create(label);
	}
	if (hdl == NULL) {
		hdl = gadfile_create(fname);
	}
	if (hdl) {
		anime->gadfile = hdl;
		anime->gadvram[gad] = hdl->vram;
	}

ago_exit:
	return;
}

static void gad_close(ANIME anime) {

	GADFILE	gadfile;

	gadfile = anime->gadfile;
	anime->gadfile = NULL;
	gadfile_destroy(gadfile);
}

static void gad_trush(ANIME anime, int gad) {

	VRAMHDL	gadvram;
	GADFILE	gadfile;
	int		i;
	GADHDL	gadhdl;

	if ((gad < 0) || (gad >= GAMECORE_MAXGADFILE)) {
		goto agt_exit;
	}
	gadvram = anime->gadvram[gad];
	if (gadvram == NULL) {
		goto agt_exit;
	}
	gadfile = anime->gadfile;
	if ((gadfile) && (gadfile->vram == gadvram)) {
		anime->gadfile = NULL;
		gadfile_destroy(gadfile);
	}

	anime->gadvram[gad] = NULL;
	for (i=0; i<GAMECORE_MAXGADHDL; i++) {
		gadhdl = anime->gadhdl[i];
		if ((gadhdl) && (gadhdl->gad == gadvram)) {
			anime->gadhdl[i] = NULL;
			if (gadhdl->enable) {
				anime->gadenable--;
			}
			vram_destroy(gadhdl->bak);
			_MFREE(gadhdl);
		}
	}
	vram_destroy(gadvram);

agt_exit:
	return;
}

static void gad_start(ANIME anime, int cmd, int from, int to) {

	GADHDL	gadhdl;

	from = max(from, 0);
	to = min(to, GAMECORE_MAXGADHDL);
	while(from <= to) {
		gadhdl = anime->gadhdl[from];
		if ((gadhdl) && (!gadhdl->enable)) {
			gadhdl->frame = 0;
			gadhdl->tick = GETTICK();
			gadhdl->enable = TRUE;
			anime->gadenable++;
			TRACEOUT(("start gad anime: %d", from));
		}
		from++;
	}
	(void)cmd;
}

static void gad_end(ANIME anime, int cmd, int from, int to) {

	GADHDL	gadhdl;

	from = max(from, 0);
	to = min(to, GAMECORE_MAXGADHDL);
	while(from <= to) {
		gadhdl = anime->gadhdl[from];
		if ((gadhdl) && (gadhdl->enable)) {
			gadhdl->enable = FALSE;
			anime->gadenable--;
		}
		from++;
	}
	(void)cmd;
}

static BOOL gad_exec(GADHDL gadhdl, UINT32 tick) {

	GADDATA		*dat;
	RECT_U		drct;
	RECT_T		srct;
	DISPWIN		dispwin;
	VRAMHDL		dst;
	int			past;

	past = tick - gadhdl->tick;
	if (past < 0) {
		goto ae1_exit;
	}
	if (gadhdl->frame >= gadhdl->frames) {
		if (!gadhdl->loop) {
			return(FALSE);
		}
		gadhdl->frame = 0;
	}

	dat = (GADDATA *)(gadhdl + 1);
	dat += gadhdl->frame;
	scr_valset(gadhdl->reg, gadhdl->frame);
	gadhdl->frame++;
	gadhdl->tick = tick + LOADINTELDWORD(dat->tick);

	dispwin = &gamecore.dispwin;
	if (!(dispwin->flag & DISPWIN_VRAM)) {
		goto ae1_exit;
	}
	dst = gamecore.vram[dispwin->vramnum];

	srct.left = LOADINTELWORD(dat->posx);
	srct.top = LOADINTELWORD(dat->posy);
	srct.right = srct.left + gadhdl->width;
	srct.bottom = srct.top + gadhdl->height;
	drct.r.left = gadhdl->pt.x;
	drct.r.top = gadhdl->pt.y;
	drct.r.right = drct.r.left + gadhdl->width;
	drct.r.bottom = drct.r.top + gadhdl->height;
#ifdef SIZE_QVGA
	vramdraw_halfrect(&srct);
	vramdraw_halfrect(&drct.r);
#endif
	vramcpy_cpy(dst, gadhdl->bak, &drct.p, NULL);
	vramcpy_cpyex(dst, gadhdl->gad, &drct.p, &srct);
	vramdraw_setrect(dst, &drct.r);
	vramdraw_draw();

ae1_exit:
	return(TRUE);
}


// ---- GAN

static void ganstop(ANIME anime, GANHDL ganhdl) {

	if ((ganhdl) && (anime->ganenable == ganhdl)) {
		anime->ganenable = NULL;
		anime->ganvram = NULL;
		vram_destroy(ganhdl->vram[0]);
		vram_destroy(ganhdl->vram[1]);
		ganhdl->vram[0] = NULL;
		ganhdl->vram[1] = NULL;
	}
}

static void gan_open(ANIME anime, int num, const char *fname) {

	ARCFILEH	hdl;
	GANHEAD		head;
	UINT		size;
	UINT		scr;
	UINT		pic;
	GANHDL		ganhdl;
	UINT		rsize;
	BYTE		*ptr;
	GANFILE		*file;
	UINT		pos;

	if ((num < 0) || (num >= GAMECORE_MAXGANHDL)) {
		goto gope_err1;
	}

	hdl = arcfile_open(ARCTYPE_GRAPHICS, fname);
	if (hdl == NULL) {
		goto gope_err1;
	}
	size = hdl->size;
	if ((size <= sizeof(head) + 0x2800) ||
		(arcfile_read(hdl, &head, sizeof(head)) != sizeof(head)) ||
		(memcmp(head.sig, "GANM0100", 8))) {
		goto gope_err2;
	}
	scr = LOADINTELDWORD(head.scr);
	pic = LOADINTELDWORD(head.pic);
	if ((scr == 0) || (scr > (0x2000 / 8)) ||
		(pic == 0) || (pic > (0x800 / 16))) {
		goto gope_err2;
	}
	TRACEOUT(("GAN: size %d", size));
	ganhdl = (GANHDL)_MALLOC(sizeof(_GANHDL) + size, "GANHDL");
	if (ganhdl == NULL) {
		goto gope_err2;
	}
	ZeroMemory(ganhdl, sizeof(_GANHDL));
	ganhdl->size = size;
	ganhdl->scrmax = scr;
	ganhdl->picmax = pic;
	ptr = (BYTE *)(ganhdl + 1);
	CopyMemory(ptr, &head, sizeof(head));
	ptr += sizeof(head);
	size -= sizeof(head);
	while(size) {
		rsize = min(size, 0x1000);
		size -= rsize;
		if (arcfile_read(hdl, ptr, rsize) != rsize) {
			goto gope_err3;
		}
		ptr += rsize;
		taskmng_rol();
	}
	file = (GANFILE *)(((BYTE *)(ganhdl + 1)) + 0x2010);
	while(pic) {
		pos = LOADINTELDWORD(file->pos);
		size = LOADINTELDWORD(file->size);
		if (pos >= hdl->size) {
			STOREINTELDWORD(file->size, 0);
		}
		else if ((pos + size) > hdl->size) {
			size = hdl->size - pos;
			STOREINTELDWORD(file->size, size);
		}
		file++;
		pic--;
	}

	anime->ganhdl[num] = ganhdl;
	arcfile_close(hdl);
	TRACEOUT(("GAN load:success"));
	return;

gope_err3:
	_MFREE(ganhdl);

gope_err2:
	arcfile_close(hdl);

gope_err1:
	return;
}

static void gan_trush(ANIME anime, int num) {

	GANHDL		ganhdl;

	if ((num >= 0) && (num < GAMECORE_MAXGANHDL)) {
		ganhdl = anime->ganhdl[num];
		if (ganhdl) {
			ganstop(anime, ganhdl);
			anime->ganhdl[num] = NULL;
			_MFREE(ganhdl);
		}
	}
}

static void gan_start(ANIME anime, int cmd, int num, int val) {

	GANHDL	ganhdl;

	if ((num < 0) || (num >= GAMECORE_MAXGANHDL)) {
		goto gst_exit;
	}
	ganhdl = anime->ganhdl[num];
	if (ganhdl == NULL) {
		goto gst_exit;
	}
	ganhdl->reg = val;
	if (anime->ganenable != ganhdl) {
		ganstop(anime, anime->ganenable);
#ifndef SIZE_QVGA
		ganhdl->vram[0] = vram_create(640, 480, FALSE, DEFAULT_BPP);
		ganhdl->vram[1] = vram_create(640, 480, FALSE, DEFAULT_BPP);
#else
		ganhdl->vram[0] = vram_create(320, 240, FALSE, DEFAULT_BPP);
		ganhdl->vram[1] = vram_create(320, 240, FALSE, DEFAULT_BPP);
#endif
		ganhdl->scrpos = 0;
		ganhdl->tick = GETTICK();
		anime->ganenable = ganhdl;
	}
	TRACEOUT(("start gan anime: %d", num));

gst_exit:
	(void)cmd;
	return;
}

static void gan_end(ANIME anime, int cmd, int num, int val) {

	GANHDL	ganhdl;

	if ((num >= 0) && (num < GAMECORE_MAXGANHDL)) {
		ganhdl = anime->ganhdl[num];
		ganstop(anime, ganhdl);
	}
	(void)cmd;
	(void)val;
}

static BOOL gan_exec(ANIME anime, GANHDL ganhdl, UINT32 tick) {

const GANSCR	*scr;
const GANFILE	*file;
	UINT		cmd;
	UINT		prm;
	VRAMHDL		vram;

	if (ganhdl->scrpos >= ganhdl->scrmax) {
		return(FALSE);
	}
	scr = (GANSCR *)(((BYTE *)(ganhdl + 1)) + 0x10);
	scr += ganhdl->scrpos;
	ganhdl->scrpos += 1;
	cmd = LOADINTELDWORD(scr->cmd);
	prm = LOADINTELDWORD(scr->prm);
	switch(cmd) {
		case 1:		// disp screen;
			prm--;
			if (prm >= ganhdl->picmax) {
				break;
			}
			file = (GANFILE *)(((BYTE *)(ganhdl + 1)) + 0x2010);
			file += prm;
			vram = ganhdl->vram[ganhdl->vrampos & 1];
			ganhdl->vrampos ^= 1;
			if (cgload_gan(vram, LOADINTELDWORD(file->ref),
					((BYTE *)(ganhdl + 1)) + LOADINTELDWORD(file->pos),
					LOADINTELDWORD(file->size)) == SUCCESS) {
				anime->ganvram = vram;
				vramdraw_setrect(vram, NULL);
				vramdraw_draw();
			}
			break;

		case 2:		// wait
			prm += 1;
			prm *= 1000;
			prm /= 24;
			if ((tick - ganhdl->tick) >= prm) {
				ganhdl->tick = tick;
			}
			else {
				ganhdl->scrpos -= 1;
			}
			break;

		case 3:		// end
			return(FALSE);

		case 4:		// se?
			sndplay_seplay((prm & 0x1f) | 0x20, 0);
			break;

		case 5:		// jump
			ganhdl->scrpos = prm;
			break;

		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			if (ganhdl->reg >= (cmd - 5)) {
				ganhdl->scrpos = prm;
			}
			break;
	}
	return(TRUE);
}


// ----

void anime_open(int gad, const char *fname) {

	ANIME	anime;

	anime = &gamecore.anime;
	gad_close(anime);
	gad_trush(anime, gad);
	gan_trush(anime, gad);
	gad_open(anime, gad, fname);
	gan_open(anime, gad, fname);
}

void anime_close(void) {

	ANIME	anime;

	anime = &gamecore.anime;
	gad_close(anime);
}

void anime_trush(int gad) {

	ANIME	anime;

	anime = &gamecore.anime;
	gad_trush(anime, gad);
	gan_trush(anime, gad);
}

void anime_setloop(int num, int loop) {

	ANIME	anime;
	GADHDL	gadhdl;

	if ((num >= 0) && (num < GAMECORE_MAXGADHDL)) {
		anime = &gamecore.anime;
		gadhdl = anime->gadhdl[num];
		if (gadhdl) {
			gadhdl->loop = loop;
		}
	}
}

void anime_setdata(int num, int major, int minor, SINT32 reg) {

	ANIME	anime;
	GADFILE	gadfile;
	BYTE	*ptr;
	BYTE	*ptrterm;
	GADTBL	*tbl;
	int		pat;
	int		frames;
	int		size;
	GADHDL	gadhdl;
	RECT_T	rct;

	if ((num < 0) || (num >= GAMECORE_MAXGADHDL)) {
		goto ads_exit;
	}
	anime = &gamecore.anime;
	gadfile = anime->gadfile;
	if (gadfile == NULL) {
		goto ads_exit;
	}
	ptr = (BYTE *)(gadfile + 1);
	ptrterm = ptr + gadfile->size;
	pat = gadfile->pat;
	while(pat) {
		pat--;
		tbl = (GADTBL *)ptr;
		ptr += sizeof(GADTBL);
		if (ptr > ptrterm) {
			break;
		}
		frames = LOADINTELWORD(tbl->frames);
		size = frames * sizeof(GADDATA);
		ptr += size;
		if (ptr > ptrterm) {
			break;
		}
		if (((int)tbl->major != major) || ((int)tbl->minor != minor)) {
			continue;
		}
		// hit‚µ‚Ü‚·‚½B
		TRACEOUT(("hit anime pattern %d %d %d", num, major, minor));
		gadhdl = anime->gadhdl[num];
		if (gadhdl) {
			anime->gadhdl[num] = NULL;
			if (gadhdl->enable) {
				anime->gadenable--;
			}
			vram_destroy(gadhdl->bak);
			_MFREE(gadhdl);
		}
		if (frames == 0) {
			break;
		}
		gadhdl = (GADHDL)_MALLOC(sizeof(_GADHDL) + size, "GADHDL");
		if (gadhdl == NULL) {
			break;
		}
		anime->gadhdl[num] = gadhdl;
		ZeroMemory(gadhdl, sizeof(_GADHDL));
		gadhdl->gad = gadfile->vram;
		gadhdl->reg = reg;
		gadhdl->frames = frames;
		gadhdl->pt.x = LOADINTELWORD(tbl->posx);
		gadhdl->pt.y = LOADINTELWORD(tbl->posy);
		gadhdl->width = LOADINTELWORD(tbl->width);
		gadhdl->height = LOADINTELWORD(tbl->height);
		CopyMemory(gadhdl + 1, tbl + 1, size);
		rct.left = gadhdl->pt.x;
		rct.top = gadhdl->pt.y;
		rct.right = rct.left + gadhdl->width;
		rct.bottom = rct.top + gadhdl->height;
#ifdef SIZE_QVGA
		vramdraw_halfrect(&rct);
#endif
		gadhdl->bak = vram_create(rct.right - rct.left,
								rct.bottom - rct.top, FALSE, DEFAULT_BPP);
		vramcpy_cpy(gadhdl->bak, gamecore.vram[gamecore.dispwin.vramnum],
																NULL, &rct);
		break;
	}

ads_exit:
	return;
}

void anime_setloc(int num, int x, int y) {

	ANIME	anime;
	GADHDL	gadhdl;
	VRAMHDL	dst;
	POINT_T	pt;
	RECT_T	rct;

	if ((num >= 0) && (num < GAMECORE_MAXGADHDL)) {
		anime = &gamecore.anime;
		gadhdl = anime->gadhdl[num];
		if (gadhdl) {
			dst = gamecore.vram[gamecore.dispwin.vramnum];
			if (gadhdl->enable) {
				pt = gadhdl->pt;
#ifdef SIZE_QVGA
				vramdraw_halfpoint(&pt);
#endif
				vramcpy_cpy(dst, gadhdl->bak, &pt, NULL);
			}
			gadhdl->pt.x = x;
			gadhdl->pt.y = y;
			rct.left = x;
			rct.top = y;
			rct.right = x + gadhdl->width;
			rct.bottom = y + gadhdl->height;
#ifdef SIZE_QVGA
			vramdraw_halfrect(&rct);
#endif
			vramcpy_cpy(gadhdl->bak, dst, NULL, &rct);
		}
	}
}

void anime_start(int cmd, int param1, int param2) {

	ANIME	anime;

	anime = &gamecore.anime;
	gad_start(anime, cmd, param1, param2);
	gan_start(anime, cmd, param1, param2);
}

void anime_end(int cmd, int param1, int param2) {

	ANIME	anime;

	anime = &gamecore.anime;
	gad_end(anime, cmd, param1, param2);
	gan_end(anime, cmd, param1, param2);
}


// ----

void anime_exec(void) {

	ANIME	anime;
	UINT32	tick;
	int		i;
	GADHDL	gadhdl;
	GANHDL	ganhdl;
	BOOL	r;

	anime = &gamecore.anime;
	if (anime->gadenable) {
		tick = GETTICK();
		for (i=0; i<GAMECORE_MAXGADHDL; i++) {
			gadhdl = anime->gadhdl[i];
			if ((gadhdl) && (gadhdl->enable)) {
				r = gad_exec(gadhdl, tick);
				if (!r) {
					gadhdl->enable = FALSE;
					anime->gadenable--;
				}
			}
		}
	}
	ganhdl = anime->ganenable;
	if (ganhdl) {
		r = gan_exec(anime, ganhdl, GETTICK());
		if (!r) {
			ganstop(anime, ganhdl);
		}
	}
}

void anime_alltrush(void) {

	ANIME	anime;
	int		i;

	anime = &gamecore.anime;
	for (i=0; i<GAMECORE_MAXGADFILE; i++) {
		gad_trush(anime, i);
	}
	for (i=0; i<GAMECORE_MAXGANHDL; i++) {
		gan_trush(anime, i);
	}
}

