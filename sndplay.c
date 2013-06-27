#include	"compiler.h"
#include	"dosio.h"
#include	"cddamng.h"
#include	"gamecore.h"
#include	"sound.h"
#include	"arcfile.h"
#include	"sstream.h"


void sndplay_init(void) {

	ZeroMemory(&gamecore.sndplay, sizeof(gamecore.sndplay));
	sndplay_cddaset(1, 99);
}


// ---- cdda

static BOOL cddafileplay(int track, const char *subdir) {

	BOOL	r;
	char	path[MAX_PATH];
	char	fname[32];

	milstr_ncpy(path, gamecore.suf.scriptpath, sizeof(path));
	plusyen(path, sizeof(path));
	milstr_ncat(path, subdir, sizeof(path));
	plusyen(path, sizeof(path));

	sprintf(fname, "track%02d.wav", track);
	milstr_ncat(path, fname, sizeof(path));
	r = soundmix_load(SOUNDTRK_CDDA, mp3_ssopen, path);
#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
	if (r != SUCCESS) {
		cutFileName(path);
		sprintf(fname, "track%02d.mp3", track);
		milstr_ncat(path, fname, sizeof(path));
		r = soundmix_load(SOUNDTRK_CDDA, mp3_ssopen, path);
	}
#endif
#if defined(OGGVORBIS_LIB)
	if (r != SUCCESS) {
		cutFileName(path);
		sprintf(fname, "track%02d.ogg", track);
		milstr_ncat(path, fname, sizeof(path));
		r = soundmix_load(SOUNDTRK_CDDA, mp3_ssopen, path);
	}
#endif
	return(r);
}

static void cddaplay(int tick) {

	SNDPLAY	sndplay;
	BOOL	r;

	sndplay = &gamecore.sndplay;
	if (!(sndplay->playing & SNDPLAY_CDDAPLAY)) {
		sndplay->playing |= SNDPLAY_CDDAPLAY;
		r = cddamng_play(sndplay->cddatrack, sndplay->cddaloop, tick);
		if (r != SUCCESS) {
			r = cddafileplay(sndplay->cddatrack, "cdda");
#if defined(MACOS)
			if (r != SUCCESS) {
				r = cddafileplay(sndplay->cddatrack, "bgm");
			}
#endif
			if (r == SUCCESS) {
				soundmix_play(SOUNDTRK_CDDA, sndplay->cddaloop, tick);
			}
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
		if (soundmix_load(SOUNDTRK_SOUND, arc_ssopen, &asa) != SUCCESS) {
			asa.type = ARCTYPE_MIDI;
			asa.fname = sndplay->sound;
			soundmix_load(SOUNDTRK_SOUND, arcraw_ssopen, &asa);
		}
		soundmix_play(SOUNDTRK_SOUND, sndplay->waveloop, tick);
	}
}

void sndplay_waveset(const char *fname, int cmd) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	sndplay->playing &= ~(SNDPLAY_WAVE | SNDPLAY_WAVEPLAY);
	milstr_ncpy(sndplay->sound, fname, sizeof(sndplay->sound));
	soundmix_unload(SOUNDTRK_SOUND);
	if (cmd == 0) {
		sndplay_waveplay(0, 0);
	}
	else if (cmd == 2) {
		sndplay_waveplay(1, 0);
	}
}

void sndplay_waveplay(BYTE flag, int tick) {

	SNDPLAY		sndplay;

	sndplay = &gamecore.sndplay;
	if (!(sndplay->playing & SNDPLAY_WAVE)) {
		sndplay->playing |= SNDPLAY_WAVE;
		sndplay->waveloop = (flag == 0)?TRUE:FALSE;
		if (gamecore.gamecfg.bgm) {
			waveplay(tick);
		}
	}
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


// ---- voice

void sndplay_voiceset(const char *fname) {

	SNDPLAY	sndplay;

	sndplay = &gamecore.sndplay;
	sndplay->playing |= SNDPLAY_PCMLOAD;
	milstr_ncpy(sndplay->pcm, fname, sizeof(sndplay->pcm));
}

void sndplay_voicereset(void) {

	SNDPLAY	sndplay;

	sndplay = &gamecore.sndplay;
	sndplay->playing &= ~SNDPLAY_PCMLOAD;
}

void sndplay_voiceplay(void) {

	SNDPLAY			sndplay;
	ARCSTREAMARG	asa;

	sndplay = &gamecore.sndplay;
	if (sndplay->playing & SNDPLAY_PCMLOAD) {
		asa.type = ARCTYPE_VOICE;
		asa.fname = sndplay->pcm;
		soundmix_load(SOUNDTRK_VOICE, arcse_ssopen, &asa);
		soundmix_play(SOUNDTRK_VOICE, 0, 0);
	}
}

BOOL sndplay_voicecondition(const char *fname) {

	SNDPLAY	sndplay;

	sndplay = &gamecore.sndplay;
	if ((sndplay->playing & SNDPLAY_PCMLOAD) &&
		(fname) && (!milstr_cmp(fname, sndplay->pcm))) {
		return(SUCCESS);
	}
	else {
		return(FAILURE);
	}
}

