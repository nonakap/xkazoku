#include	"compiler.h"
#include	"dosio.h"
#include	"timemng.h"
#include	"analyze.h"
#include	"savcheck.h"


enum {
	SAVE_SYS		= 0x01,
	SAVE_EXA		= 0x02,
	SAVE_KID		= 0x04,
	SAVE_EF			= 0x08,

	SAVE_BFLAG		= 0x10,
	SAVE_VFLAG		= 0x20
};

typedef struct {
	UINT	flag;
	UINT	bits;
	UINT	bitssize;
	UINT	values;
	UINT	valsize;
	UINT	sysflags;
	UINT	exaflags;
	UINT	exavals;
	UINT	kids;
	UINT	type;
} _SAVPRM, *SAVPRM;

typedef struct {
	BYTE	*ptr;
	UINT	size;
	UINT	files;
	UINT	gamesaves;
	UINT	ver;
	UINT	base;
} _SAVINF, *SAVINF;

#if defined(__GNUC__)
typedef struct {
	BYTE	biSize[4];
	BYTE	biWidth[4];
	BYTE	biHeight[4];
	BYTE	biPlanes[2];
	BYTE	biBitCount[2];
	BYTE	biCompression[4];
	BYTE	biSizeImage[4];
	BYTE	biXPelsPerMeter[4];
	BYTE	biYPelsPerMeter[4];
	BYTE	biClrUsed[4];
	BYTE	biClrImportant[4];
} __attribute__ ((packed)) BMPINFO;
#else
#pragma pack(push, 1)
typedef struct {
	BYTE	biSize[4];
	BYTE	biWidth[4];
	BYTE	biHeight[4];
	BYTE	biPlanes[2];
	BYTE	biBitCount[2];
	BYTE	biCompression[4];
	BYTE	biSizeImage[4];
	BYTE	biXPelsPerMeter[4];
	BYTE	biYPelsPerMeter[4];
	BYTE	biClrUsed[4];
	BYTE	biClrImportant[4];
} BMPINFO;
#pragma pack(pop)
#endif


static int scmdcheck(SCRHDL hdl, SCROPE ope, void *arg) {

	SAVPRM	sv;
	UINT	prm1;
	UINT	prm2;

	sv = (SAVPRM)arg;
	switch(ope->cmd) {
		case 0x30:
			if (ope->remain < 2) {
				log_disp("FLN: wrong parametor");
				break;
			}
			prm1 = LOADINTELWORD(ope->ptr);
			if (!(sv->flag & SAVE_BFLAG)) {
				sv->flag |= SAVE_BFLAG;
				sv->bits = prm1;
				sv->bitssize = (prm1 + 8) / 8;
				log_disp("FLN: bit flag %d (%dbytes)", prm1, sv->bitssize);
			}
			else {
				if (sv->bits != prm1) {
					log_disp("FLN: disagree bit flag (%d)", prm1);
				}
			}
			break;

		case 0x40:
			if (ope->remain < 2) {
				log_disp("HLN: wrong parametor");
				break;
			}
			prm1 = LOADINTELWORD(ope->ptr);
			if (!(sv->flag & SAVE_VFLAG)) {
				sv->flag |= SAVE_VFLAG;
				sv->values = prm1;
				sv->valsize = (prm1 + 1) * 4;
				log_disp("HLN: value flag %d (%dbytes)",
											prm1, (prm1 + 1) * 4);
			}
			else {
				if (sv->values != prm1) {
					log_disp("HLN: disagree value flag (%d)", prm1);
				}
			}
			break;

		case 0x38:
		case 0x39:
			if (ope->remain < 4) {
				log_disp("ES/EC: wrong parametor");
				break;
			}
			prm1 = LOADINTELWORD(ope->ptr + 0);
			prm2 = LOADINTELWORD(ope->ptr + 2);
			if (prm1 > prm2) {
				log_disp("ES/EC: wrong parametor (empty data)");
				break;
			}
			prm1 = prm2 - prm1 + 1;
			if (!(sv->flag & SAVE_SYS)) {
				sv->flag |= SAVE_SYS;
				sv->sysflags = prm1;
				log_disp("ES/EC: bit flag %d (%dbytes)",
											prm1, (prm1 + 7) / 8);
			}
			else {
				if (sv->sysflags != prm1) {
					log_disp("ES/EC: disagree bit flag (%d)", prm1);
				}
			}
			break;

		case 0x48:
			if (ope->remain < 4) {
				log_disp("EXA: wrong parametor");
				break;
			}
			prm1 = LOADINTELWORD(ope->ptr + 0);
			prm2 = LOADINTELWORD(ope->ptr + 2);
			if (!(sv->flag & SAVE_EXA)) {
				sv->flag |= SAVE_EXA;
				sv->exaflags = prm1;
				sv->exavals = prm2;
				log_disp("EXA: bit flag %d (%dbytes)",
											prm1, ((prm1 >> 5) + 1) << 2);
				log_disp("EXA: value flag %d (%dbytes)",
											prm2, (prm2 + 1) << 2);
			}
			else {
				if (sv->exaflags != prm1) {
					log_disp("EXA: disagree bit flag (%d)", prm1);
				}
				if (sv->exavals != prm2) {
					log_disp("EXA: disagree value flag (%d)", prm2);
				}
			}
			break;

		case 0xa5:
			if (ope->remain < 4) {
				log_disp("KIDFN: wrong parametor");
				break;
			}
			prm1 = LOADINTELDWORD(ope->ptr);
			if (!(sv->flag & SAVE_KID)) {
				sv->flag |= SAVE_KID;
				sv->kids = prm1;
				log_disp("KIDFN: kid flag %d", prm1);
			}
			else {
				if (sv->kids != prm1) {
					log_disp("KIDFN: disagree kid flag (%d)", prm1);
				}
			}
			break;

		case 0xef:
			if (!(sv->flag & SAVE_EF)) {
				sv->flag |= SAVE_EF;
				log_disp("use extend save commands");
			}
			break;
	}
	return(0);
}

static UINT savegetpos(SAVINF si, UINT num) {

const BYTE	*p;

	p = si->ptr;
	if (si->ver <= 1) {
		p += num * 4;
	}
	else {
		p += si->base;
		p += num * 0x40;
	}
	return(LOADINTELDWORD(p));
}

static UINT savegetsize(SAVINF si, UINT num) {

	UINT	base;
	UINT	next;
	UINT	i;
	UINT	pos;

	if (num >= si->files) {
		goto svgs_err;
	}
	base = savegetpos(si, num);
	if (base == 0) {
		goto svgs_err;
	}
	next = si->size;
	for (i=0; i<si->files; i++) {
		pos = savegetpos(si, i);
		if ((pos > base) && (pos < next)) {
			next = pos;
		}
	}
	return(next - base);

svgs_err:
	return(0);
}

static void saveana(SAVPRM sv, SAVINF si, UINT num) {

const BYTE		*ptr;
	UINT		pos;
	int			size;
	int			width;
	int			height;
const _SYSTIME	*st;
const BMPINFO	*bi;
	UINT		year, month, day, hour, minute, second;
	int			syssize;

	pos = savegetpos(si, num);
	size = savegetsize(si, num);
	log_disp("analyze game save %d / filepos %d / size %dbytes",
														num + 1, pos, size);
	ptr = si->ptr + pos;
	size -= sizeof(_SYSTIME);
	if (size < 0) {
		log_disp("unknown game save");
		goto sana_exit;
	}
	st = (_SYSTIME *)ptr;
	ptr += sizeof(_SYSTIME);
	year = LOADINTELWORD(st->year);
	month = LOADINTELWORD(st->month);
	day = LOADINTELWORD(st->day);
	hour = LOADINTELWORD(st->hour);
	minute = LOADINTELWORD(st->minute);
	second = LOADINTELWORD(st->second);
	log_disp("date: %4u/%2u/%2u %02u:%02u:%02u",
									year, month, day, hour, minute, second);

	if (size >= sizeof(BMPINFO)) {
		bi = (BMPINFO *)ptr;
		if ((LOADINTELDWORD(bi->biSize) == sizeof(BMPINFO)) &&
			(LOADINTELWORD(bi->biPlanes) == 1) &&
			(LOADINTELWORD(bi->biBitCount) == 24) &&
			(LOADINTELDWORD(bi->biCompression) == 0)) {
			width = LOADINTELDWORD(bi->biWidth);
			height = LOADINTELDWORD(bi->biHeight);
			if (height < 0) {
				height = 0 - height;
			}
			log_disp("use bmp: set configure GAME_SAVEGRPH");
			if (si->ver <= 1) {
				if (sv->flag & SAVE_EF) {
					if ((width == 240) && (height == 180)) {
						log_disp("bmp size 240x180 - extention save");
					}
					else {
						log_disp("warning! bmp size must 240x180");
					}
				}
				else {
					if ((width == 240) && (height == 180)) {
						log_disp("bmp size 240x180 - type:new "
												"(later EXEVER_KONYA2?)");
					}
					else if ((width == 160) && (height == 120)) {
						log_disp("bmp size 160x120 - type:old");
					}
					else {
						log_disp("warning! unknown bmp size %dx%d",
															width, height);
					}
				}
			}
			else {
				log_disp("bmp size %dx%d - free format", width, height);
			}
			ptr += sizeof(BMPINFO);
			size -= sizeof(BMPINFO);
			width *= height;
			width *= 3;
			ptr += width;
			size -= width;
			if (size < 0) {
				log_disp("error: broken save file.");
				goto sana_exit;
			}
		}
	}

	syssize = sv->valsize + sv->bitssize + 0x30;
	size -= syssize;
	if (size < 0) {
		log_disp("error: broken save file.");
		goto sana_exit;
	}
	if (size >= 100) {
		size -= 100;
		log_disp("use comment: set configure GAME_SAVECOM"
										" - type:new (later EXEVER_NURSE)");
	}
	else if (size >= 80) {
		size -= 80;
		log_disp("use comment: set configure GAME_SAVECOM"
										" - type:old or save ver2");
	}

	if (!size) {
		log_disp("ok: can make compatible savedatas.");
	}
	else {
		log_disp("error: remain data: %dbytes", size);
	}

sana_exit:
	return;
}

static int svchk(const char *sname, SAVPRM sv) {

	FILEH	fh;
	_SAVINF	si;
	UINT	prm1;
	UINT	prm2;
	UINT	tmp;
	UINT	pos;
	UINT	i;

	si.ptr = NULL;
	fh = file_open_rb(sname);
	if (fh != FILEH_INVALID) {
		si.size = file_getsize(fh);
		if (si.size) {
			si.ptr = (BYTE *)_MALLOC(si.size, "save");
			if (si.ptr != NULL) {
				if (file_read(fh, si.ptr, si.size) != si.size) {
					_MFREE(si.ptr);
					si.ptr = NULL;
				}
			}
		}
		file_close(fh);
	}
	if (si.ptr == NULL) {
		return(-1);
	}

	log_disp("load %s", sname);

	if (si.size < 8) {
		goto sc_exit;
	}
	prm1 = LOADINTELDWORD(si.ptr + 0);
	prm2 = LOADINTELDWORD(si.ptr + 4);
	if (prm1 == (0xc10 + (prm2 * 0x40))) {
		log_disp("savefile version.2 (later EXEVER_HIMITSU)");
		if (prm1 > si.size) {
			log_disp("savefile wrong size");
			goto sc_exit;
		}
		si.files = prm2;
		si.ver = 2;
		si.base = 0xc10;
		log_disp("header files %d / size %dbytes", prm2, prm1);
	}
	else if (prm1 == (0xd10 + (prm2 * 0x40))) {
		log_disp("savefile version.2 (later EXEVER_AKIBA)");
		if (prm1 > si.size) {
			log_disp("savefile wrong size");
			goto sc_exit;
		}
		si.files = prm2;
		si.ver = 2;
		si.base = 0xd10;
		log_disp("header files %d / size %dbytes", prm2, prm1);
	}
	else {
		prm1 = 0;
		prm2 = si.size;
		pos = 0;
		while((pos + 4) <= prm2) {
			tmp = LOADINTELDWORD(si.ptr + pos);
			if (tmp == 0xffffffff) {
				log_disp("error: savefile version.2 header");
				goto sc_exit;
			}
			else if (tmp != 0) {
				if (prm2 > tmp) {
					prm1 = tmp;
					prm2 = tmp;
				}
			}
			pos += 4;
		}
		if (prm1 & 3) {
			log_disp("error: savefile version.1 header");
			goto sc_exit;
		}
		si.files = prm1 >> 2;
		si.ver = 1;
		log_disp("savefile version.1");
		log_disp("header files %d / size %dbytes", prm1 >> 2, prm1);
	}

	prm1 = si.files - sv->type;
	if ((prm1 < 9) || (prm1 > 53)) {
		log_disp("game saves %u, unknown save type, or not save game", prm1);
		goto sc_exit;
	}
	else if (prm1 == 9) {
		log_disp("game saves %u, set configure GAME_SAVEMAX9", prm1);
		si.gamesaves = 9;
	}
	else if (prm1 < 27) {
		log_disp("game saves %u, force by GAME_SAVEMAX9", prm1);
		si.gamesaves = 9;
	}
	else if (prm1 == 27) {
		log_disp("game saves %u, set configure GAME_SAVEMAX27", prm1);
		si.gamesaves = 27;
	}
	else if (prm1 < 30) {
		log_disp("game saves %u, force by GAME_SAVEMAX27", prm1);
		si.gamesaves = 27;
	}
	else if (prm1 == 30) {
		log_disp("game saves %u, set configure GAME_SAVEMAX30", prm1);
		si.gamesaves = 30;
	}
	else if (prm1 < 50) {
		log_disp("game saves %u, force by GAME_SAVEMAX30", prm1);
		si.gamesaves = 30;
	}
	else if (prm1 == 50) {
		log_disp("game saves %u, set configure GAME_SAVEMAX50", prm1);
		si.gamesaves = 50;
	}
	else {
		log_disp("game saves %u, force by GAME_SAVEMAX50", prm1);
		si.gamesaves = 50;
	}

	prm1 = si.size;
	prm2 = 0;
	for (i=0; i<si.gamesaves; i++) {
		tmp = savegetsize(&si, i);
		if ((tmp) && (tmp < prm1)) {
			prm1 = tmp;
			prm2 = i;
		}
	}
	saveana(sv, &si, prm2);

sc_exit:
	_MFREE(si.ptr);
	return(0);
}

void savcheck(const char *path, const char *arc, const char *key,
												int defbits, int defflags) {

	ARCFH		afh;
	int			i;
	char		name[ARCFILENAME_LEN + 1];
	char		sname[MAX_PATH];
	SCRHDL		sh;
	_SAVPRM		sv;
	UINT		type;

	afh = arcread_open(path);
	if (afh == NULL) {
		log_disp("%s: couldn't open.", path);
		goto sv_err;
	}

	ZeroMemory(&sv, sizeof(sv));
	sv.bits = defbits;
	sv.bitssize = (defbits + 7) / 8;
	sv.values = defflags;
	sv.valsize = defflags * 4;
	for (i=0; i<afh->files; i++) {
		if (arcread_getname(afh, i, name, sizeof(name))) {
			break;
		}
		sh = scr_create(afh, name);
		if (sh == NULL) {
			continue;
		}
		scr_enum(sh, &sv, scmdcheck);
		scr_destroy(sh);
	}
	arcread_close(afh);

	if (!(sv.flag & SAVE_BFLAG)) {
		log_disp("undefined FLN ... use gamedef default: %d", defbits);
	}
	if (!(sv.flag & SAVE_VFLAG)) {
		log_disp("undefined HLN ... use gamedef default: %d", defflags);
	}

	sv.type = 3;
	if (!(sv.flag & (SAVE_SYS | SAVE_EXA | SAVE_KID))) {
		log_disp("Non save (Demo version?)");
		goto sv_exit;
	}
	type = sv.flag & (SAVE_SYS | SAVE_EXA);
	if (type == (SAVE_SYS | SAVE_EXA)) {
		log_disp("unknown save.... used both SYS and EXA");
		goto sv_exit;
	}
	else if (!type) {
		log_disp("KID only: (Demo version?)");
	}
	else if (sv.flag & SAVE_SYS) {
		if (sv.flag & SAVE_KID) {
			log_disp("unknown save.... used both SYS and KID");
			goto sv_exit;
		}
		log_disp("used SYS: set configure GAME_SAVESYS");
		sv.type = 1;
	}
	else if (!(sv.flag & SAVE_KID)) {
		log_disp("unused KID: set configure GAME_NOKID");
		sv.type = 2;
	}

	milstr_ncpy(sname, arc, sizeof(sname));
	cutFileName(sname);
	milstr_ncat(sname, key, sizeof(sname));
	milstr_ncat(sname, ".sav", sizeof(sname));
	if (!svchk(sname, &sv)) {
		goto sv_exit;
	}

	milstr_ncpy(sname, arc, sizeof(sname));
	cutFileName(sname);
	milstr_ncat(sname, "game.sav", sizeof(sname));
	if (!svchk(sname, &sv)) {
		goto sv_exit;
	}

	milstr_ncpy(sname, arc, sizeof(sname));
	cutExtName(sname);
	milstr_ncat(sname, ".sav", sizeof(sname));
	if (!svchk(sname, &sv)) {
		goto sv_exit;
	}

sv_exit:
	log_disp("");

sv_err:
	return;
}

