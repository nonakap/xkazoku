#include	"compiler.h"
#include	"gamecore.h"
#include	"arcfile.h"
#include	"cddamng.h"
#include	"sound.h"
#include	"sstream.h"
#include	"dosio.h"


void sndplay_init(void) {

	ZeroMemory(&gamecore.sndplay, sizeof(gamecore.sndplay));
	sndplay_cddaset(1, 99);
}


// ---- cdda

static void cddaplay(int tick) {

	SNDPLAY		sndplay;
	char		path[MAX_PATH];
	char		fname[32];
	BOOL		r;

	sndplay = &gamecore.sndplay;
	if (!(sndplay->playing & SNDPLAY_CDDAPLAY)) {
		sndplay->playing |= SNDPLAY_CDDAPLAY;
		r = cddamng_play(sndplay->cddatrack, sndplay->cddaloop, tick);
		if (r != SUCCESS) {
			milstr_ncpy(path, gamecore.suf.scriptpath, sizeof(path));
			plusyen(path, sizeof(path));
			milstr_ncat(path, "cdda", sizeof(path));
			plusyen(path, sizeof(path));

			sprintf(fname, "track%02d.wav", sndplay->cddatrack);
			milstr_ncat(path, fname, sizeof(path));
			r = soundmix_load(SOUNDTRK_CDDA, &mp3_stream, path);
#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
			if (r != SUCCESS) {
				cutFileName(path);
				sprintf(fname, "track%02d.mp3", sndplay->cddatrack);
				milstr_ncat(path, fname, sizeof(path));
				r = soundmix_load(SOUNDTRK_CDDA, &mp3_stream, path);
			}
#endif
#if defined(OGGVORBIS_LIB)
			if (r != SUCCESS) {
				cutFileName(path);
				sprintf(fname, "track%02d.ogg", sndplay->cddatrack);
				milstr_ncat(path, fname, sizeof(path));
				r = soundmix_load(SOUNDTRK_CDDA, &mp3_stream, path);
			}
#endif
		}
		if (r == SUCCESS) {
			soundmix_play(SOUNDTRK_CDDA, sndplay->cddaloop, tick);
		}
	}
}

void sndplay_cddaset(int cddastart, int cddaend) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	sndplay->cddastart = min(max(cddastart, 1), 99);
	sndplay->cddaend = min(max(cddaend, 1), 99);
}

void sndplay_cddaplay(int track, int cmd, int tick) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_CDDA) {
		sndplay_cddastop(0);
	}
	if ((sndplay->cddastart <= track) && (sndplay->cddaend >= track)) {
		sndplay->playing |= SNDPLAY_CDDA;
		sndplay->cddatrack = track;
		sndplay->cddaloop = cmd;
		if (gamecore.gamecfg.bgm) {
			cddaplay(tick);
		}
	}
}

void sndplay_cddastop(int tick) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_CDDAPLAY) {
		cddamng_stop(tick);
		soundmix_stop(SOUNDTRK_CDDA, tick);
	}
	sndplay->playing &= ~(SNDPLAY_CDDA | SNDPLAY_CDDAPLAY);
}

void sndplay_cddaenable(int enable, int fade) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_CDDA) {
		if (enable) {
			cddaplay(fade);
		}
		else if (sndplay->playing & SNDPLAY_CDDAPLAY) {
			sndplay->playing &= ~SNDPLAY_CDDAPLAY;
			cddamng_stop(fade);
			soundmix_stop(SOUNDTRK_CDDA, fade);
		}
	}
}


// ---- wave

static void waveplay(int tick) {

	SNDPLAY			sndplay;
	ARCSTREAMARG	asa;

	sndplay = &gamecore.sndplay;
	if (!(sndplay->playing & SNDPLAY_WAVEPLAY)) {
		sndplay->playing |= SNDPLAY_WAVEPLAY;
		asa.type = ARCTYPE_SOUND;
		asa.fname = sndplay->sound;
		soundmix_load(SOUNDTRK_SOUND, &arc_stream, &asa);
		soundmix_play(SOUNDTRK_SOUND, sndplay->waveloop, tick);
	}
}

void sndplay_waveset(const char *fname, int cmd) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	sndplay->playing &= ~(SNDPLAY_WAVE | SNDPLAY_WAVEPLAY);
	milstr_ncpy(sndplay->sound, fname, sizeof(sndplay->sound));
	soundmix_unload(SOUNDTRK_SOUND);
	if (!cmd) {
		sndplay_waveplay(0, 0);
	}
}

void sndplay_waveplay(BYTE flag, int tick) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (!(sndplay->playing & SNDPLAY_WAVE)) {
		sndplay->playing |= SNDPLAY_WAVE;
		sndplay->waveloop = TRUE;
		if (gamecore.gamecfg.bgm) {
			waveplay(tick);
		}
	}
	(void)flag;
}

void sndplay_wavestop(int tick) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_WAVE) {
		sndplay->playing &= ~SNDPLAY_WAVE;
	}
	if (sndplay->playing & SNDPLAY_WAVEPLAY) {
		sndplay->playing &= ~SNDPLAY_WAVEPLAY;
		soundmix_stop(SOUNDTRK_SOUND, tick);
	}
}

void sndplay_waveenable(int enable, int fade) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_WAVE) {
		if (enable) {
			waveplay(fade);
		}
		else if (sndplay->playing & SNDPLAY_WAVEPLAY) {
			sndplay->playing &= ~SNDPLAY_WAVEPLAY;
			soundmix_stop(SOUNDTRK_SOUND, fade);
		}
	}
}


// ---- s.e.

void sndplay_seplay(int cmd, int tick) {

	SNDPLAY		sndplay;
	int			num;

	sndplay = &gamecore.sndplay;
	num = cmd & 0x1f;
	cmd &= ~0x1f;
	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		if (gamecore.gamecfg.se) {
			if (cmd & 0x40) {
				cmd |= 1;
				soundmix_play(SOUNDTRK_SE + num, 1, tick);
			}
			else if (cmd & 0x20) {
				cmd |= 1;
				soundmix_rew(SOUNDTRK_SE + num);
				soundmix_play(SOUNDTRK_SE + num, 0, tick);
			}
			else if (sndplay->se[num] & 1) {
				soundmix_stop(SOUNDTRK_SE + num, tick);
			}
		}
		sndplay->se[num] = cmd;
	}
}

void sndplay_seenable(int enable, int fade) {

	SNDPLAY		sndplay;
	int			i;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_WAVE) {
		if (enable) {
			for (i=0; i<SOUNDTRK_MAXSE; i++) {
				if (sndplay->se[i] == 0x40) {
					sndplay->se[i] |= 1;
					soundmix_play(SOUNDTRK_SE + i, 1, fade);
				}
			}
		}
		else {
			for (i=0; i<SOUNDTRK_MAXSE; i++) {
				if (sndplay->se[i] & 1) {
					sndplay->se[i] &= ~1;
					soundmix_stop(SOUNDTRK_SE + i, fade);
				}
			}
		}
	}
}

