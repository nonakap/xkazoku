// ----------------------------------------------------------------
//	Sound Mix
//
//			(c) 2002, K.Takagi(Kenjo) [waffle@mac.com]
//			(c) 2002, K.Nonaka        [aw9k-nnk@asahi-net.or.jp]
//			(c) 2002, T.Yui           [myu@yui.ne.jp]
//
//		sound.c
//			サウンド合成メイン
// ----------------------------------------------------------------

#include	"compiler.h"
#include	"sound.h"
#include	"sndcsec.h"


static	SNDHDL	_snd = NULL;

// FOR I=-16 TO 8:PRINT INT((2^(I/4)) * 256);",";:NEXT
static const int gaintbl[24+1] =
				{ 16,  19,  22,  26,  32,  38,  45,  53,
				  64,  76,  90, 107, 128, 152, 181, 215,
				 256, 304, 362, 430, 512, 608, 724, 861, 1024};


static int defdecrew(SMIXTRACK trk) {

	trk->fremain = trk->loopsize;
	sndmix_datatrash(trk, (UINT)-1);
	if (trk->stream.ssseek == NULL) {
		return(SNDMIX_FAILURE);
	}
	else if (trk->stream.ssseek(&trk->stream, trk->loopfpos, SSSEEK_SET)
														== trk->loopfpos) {
		return(SNDMIX_SUCCESS);
	}
	else {
		return(SNDMIX_STREAMERROR);
	}
}

UINT sndmix_dataload(SMIXTRACK trk, UINT size) {

	if (size <= trk->indatas) {
		return(size);
	}
	else {
		size -= trk->indatas;
		size = min(size, trk->fremain);
		if (trk->stream.ssread == NULL) {
			return(0);
		}
		size = trk->stream.ssread(&trk->stream,
											trk->data + trk->indatas, size);
		if (size == (UINT)-1) {
			return(size);
		}
		else {
			trk->fremain -= size;
			trk->indatas += size;
			return(trk->indatas);
		}
	}
}

UINT sndmix_dataread(SMIXTRACK trk, void *buf, UINT size) {

	UINT	ret;

	ret = min(size, trk->indatas);
	if (ret) {
		CopyMemory(buf, trk->data, ret);
		buf = (void *)(((BYTE *)buf) + ret);
		sndmix_datatrash(trk, ret);
		size -= ret;
	}
	if ((size) && (trk->stream.ssread != NULL)) {
		size = trk->stream.ssread(&trk->stream, buf, size);
		if (size != (UINT)-1) {
			ret += size;
		}
	}
	return(ret);
}

void sndmix_datatrash(SMIXTRACK trk, UINT size) {

	if (trk->indatas <= size) {
		trk->indatas = 0;
	}
	else {
		BYTE *p;
		BYTE *q;
		q = trk->data;
		p = q + size;
		trk->indatas -= size;
		size = trk->indatas;
		do {
			*q++ = *p++;
		} while(--size);
	}
}


// ----

void soundmix_create(UINT basehz) {

	int		size;
	SNDHDL	snd;

	soundmix_destory();

	if ((basehz < 8000) || (basehz > 48000)) {
		return;
	}

	size = sizeof(int) * 2 * SND_MAXSAMPLE;
	size += sizeof(_SNDHDL);
	snd = (SNDHDL)_MALLOC(size, "SOUND");
	if (snd) {
		ZeroMemory(snd, size);
		snd->basehz = basehz;
		SNDCSEC_INIT;
	}
	_snd = snd;
}

void soundmix_destory(void) {

	SNDHDL	snd;
	int		i;

	snd = _snd;
	if (snd) {
		for (i=0; i<SND_MAXTRACK; i++) {
			soundmix_unload(i);
		}
		_snd = NULL;
		_MFREE(snd);
		SNDCSEC_TERM;
	}
}

static void sndsetvol(SNDHDL snd, SMIXTRACK trk, UINT samp) {

	if (!(trk->flag & SNDTRK_FADE)) {
		trk->vol = trk->basevol;
	}
	else {
		trk->fadestep += samp;
		if (trk->fadestep >= trk->fadesamp) {
			if (!trk->dir) {
				trk->flag &= ~SNDTRK_FADE;
				trk->vol = trk->basevol;
			}
			else {
				trk->flag &= ~(SNDTRK_FADE | SNDTRK_PLAYING);
				trk->vol = 0;
			}
		}
		else {
			int vol;
			vol = (trk->basevol * trk->fadestep) / trk->fadesamp;
			if (!trk->dir) {
				trk->vol = vol;
			}
			else {
				trk->vol = trk->basevol - vol;
			}
		}
	}
	sndmix_setmixproc(trk, snd->basehz);
}

static void sndsetgain(SNDHDL snd, SMIXTRACK trk, int gain) {

	if (gain < -16) {
		gain = -16;
	}
	else if (gain > 8) {
		gain = 8;
	}
	if (trk->decgain) {
		trk->decgain(trk, gain);
		trk->basevol = 1 << SND_VOLSHIFT;
	}
	else {
		trk->basevol = gaintbl[gain+16];
	}
	SNDCSEC_ENTER;
	sndsetvol(snd, trk, 0);
	SNDCSEC_LEAVE;
}

static void setfade(SMIXTRACK trk, UINT basehz, int tick, int dir) {

	if (tick < 0) {
		tick = 0;
	}
	basehz *= tick;
	basehz /= 1000;

	SNDCSEC_ENTER;
	if (basehz == 0) {
		if (dir) {
			trk->flag &= ~SNDTRK_PLAYING;
		}
	}
	else if ((!(trk->flag & SNDTRK_FADE)) || (trk->dir < dir)) {
		trk->flag &= ~SNDTRK_FADE;
		trk->fadesamp = basehz;
		trk->fadestep = 0;
		trk->dir = dir;
		trk->flag |= SNDTRK_FADE;
	}
	SNDCSEC_LEAVE;
}

int soundmix_load(int num, SNDSTREAM_OPEN ssopen, void *arg) {

	SNDHDL		snd;
	int			r;
	SMIXTRACK	trk;
	int			outsize;
	int			size;
	int			ptr;
	BYTE		data[512];

	snd = _snd;
	r = SNDMIX_SUCCESS;
	if (snd == NULL) {
		r = SNDMIX_FAILURE;
		goto smld_err0;
	}
	if ((unsigned int)num >= SND_MAXTRACK) {
		r = SNDMIX_TRACKERROR;
		goto smld_err0;
	}
	soundmix_unload(num);
	trk = snd->trk + num;
	ZeroMemory(trk, sizeof(_SMIXTRACK));
	if (ssopen == NULL) {
		r = SNDMIX_PARAMERROR;
		goto smld_err0;
	}
	if (ssopen(&trk->stream, arg, num) != SNDMIX_SUCCESS) {
		r = SNDMIX_STREAMERROR;
		goto smld_err0;
	}
	trk->data = data;
	trk->maxdatas = sizeof(data);
	trk->fremain = (UINT)-1;

	r = sndwave_open(trk);
#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
	if (r == SNDMIX_NOTSUPPORT) {
		r = sndmp3_open(trk);
	}
#ifdef USE_EXSE
	if (r == SNDMIX_NOTSUPPORT) {
		r = sndexse_open(trk);
	}
#endif
#endif
#if defined(VERMOUTH_LIB)
	if (r == SNDMIX_NOTSUPPORT) {
		r = sndmidi_open(trk);
	}
#endif
#if defined(OGGVORBIS_LIB)
	if (r == SNDMIX_NOTSUPPORT) {
		r = sndogg_open(trk);
	}
#endif
#ifdef KSYS_KPV0
	if (r == SNDMIX_NOTSUPPORT) {
		r = sndkpv_open(trk);
	}
#endif
	if (r != SNDMIX_SUCCESS) {
		goto smld_err1;
	}
	if (sndmix_setmixproc(trk, snd->basehz) != SUCCESS) {
		r = SNDMIX_PARAMERROR;
		goto smld_err1;
	}
	if (trk->decrew == NULL) {
		trk->decrew = defdecrew;
	}

	trk->maxdatas = max(trk->indatas, trk->block);
	outsize = (trk->bit / 8);
	outsize *= trk->channels;
	outsize *= trk->blksamp;
	size = outsize + trk->maxdatas + 64;
	trk->buffer = (BYTE *)_MALLOC(size, "SOUND track");
	if (trk->buffer == NULL) {
		r = SNDMIX_MEMORYERROR;
		goto smld_err1;
	}
	ptr = (outsize + 16 + 15) & (~(0x0f));
	trk->data = trk->buffer + ptr;
	if (trk->indatas) {
		CopyMemory(trk->data, data, trk->indatas);
	}
	sndsetgain(snd, trk, snd->gain[num]);
	trk->flag = SNDTRK_LOADED;
	TRACEOUT(("sound load success"));
	return(SNDMIX_SUCCESS);

smld_err1:
	if (trk->decend) {
		trk->decend(trk);         // ←020618 メモリリークゆいたん修正
	}
	if (trk->stream.ssclose) {
		trk->stream.ssclose(&trk->stream);
	}

smld_err0:
	return(SNDMIX_FAILURE);
}

void soundmix_unload(int num) {

	SNDHDL		snd;
	SMIXTRACK	trk;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	SNDCSEC_ENTER;
	if (trk->flag & SNDTRK_PLAYING) {
		trk->flag &= ~SNDTRK_PLAYING;
	}
	SNDCSEC_LEAVE;
	if (trk->flag & SNDTRK_LOADED) {
		trk->flag = 0;
		if (trk->decend) {
			trk->decend(trk);         // ←020618 メモリリークゆいたん修正
		}
		if (trk->stream.ssclose) {
			trk->stream.ssclose(&trk->stream);
		}
		if (trk->buffer) {
			_MFREE(trk->buffer);
			trk->buffer = NULL;
		}
	}
}

void soundmix_play(int num, int loop, int fadeintick) {

	SNDHDL		snd;
	SMIXTRACK	trk;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	if (trk->flag & SNDTRK_LOADED) {
		SNDCSEC_ENTER;
		trk->loop = loop;
		if (!(trk->flag & SNDTRK_PLAYING)) {
			trk->flag |= SNDTRK_PLAYING;
			TRACEOUT(("sound play track:%d", num));
			setfade(trk, snd->basehz, fadeintick, 0);
		}
		SNDCSEC_LEAVE;
	}
}

void soundmix_stop(int num, int fadeouttick) {

	SNDHDL	snd;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	setfade(snd->trk + num, snd->basehz, fadeouttick, 1);
}

void soundmix_rew(int num) {

	SNDHDL		snd;
	SMIXTRACK	trk;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	SNDCSEC_ENTER;
	if ((trk->flag & (SNDTRK_LOADED | SNDTRK_PLAYING)) == SNDTRK_LOADED) {
		trk->samplepos = 0;
		trk->decrew(trk);
	}
	SNDCSEC_LEAVE;
}

void soundmix_continue(int num) {

	SNDHDL		snd;
	SMIXTRACK	trk;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	SNDCSEC_ENTER;
	if (trk->flag & SNDTRK_PAUSED) {
		trk->flag &= ~SNDTRK_PAUSED;
		trk->flag |= SNDTRK_PLAYING;
	}
	SNDCSEC_LEAVE;
}

BOOL soundmix_isplaying(int num) {

	SNDHDL	snd;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK) ||
		(!(snd->trk[num].flag & SNDTRK_PLAYING))) {
		return(FALSE);
	}
	return(TRUE);
}

UINT soundmix_getpos(int num) {

	SNDHDL		snd;
	SMIXTRACK	trk;
	UINT		samp;
	UINT		sec;
	UINT		mill;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return(0);
	}
	trk = snd->trk + num;
	if (!(trk->flag & SNDTRK_PLAYING)) {
		return(0);
	}
	samp = trk->samplepos - trk->remain;
	sec = samp / trk->samprate;
	mill = samp - (sec * trk->samprate);
	mill *= 1000;
	mill /= trk->samprate;
	return((sec * 1000) + mill);
}

void soundmix_setgain(int num, int gain) {

	SNDHDL	snd;

	snd = _snd;
	if ((snd != NULL) && ((unsigned int)num < SND_MAXTRACK)) {
		snd->gain[num] = gain;
		if (snd->trk[num].flag & SNDTRK_LOADED) {	// これー 2003/05/04
			sndsetgain(snd, snd->trk + num, gain);
		}
	}
}

#ifdef SOUND_MOREINFO
SMIXTRACK soundmix_getinfo(int num) {

	SNDHDL	snd;

	snd = _snd;
	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK) ||
		(!(snd->trk[num].flag & SNDTRK_LOADED))) {
		return(NULL);
	}
	return(snd->trk + num);
}
#endif


UINT soundmix_getpcm(SINT16 *pcm, UINT samples) {

	SNDHDL		snd;
	UINT		cnt;
	int			fade;
	SMIXTRACK	trk;
	UINT		r;
	UINT		smpcnt;
	SINT32		*ptr;

	snd = _snd;
	if (snd) {
		SNDCSEC_ENTER;
		while(samples) {
			trk = snd->trk;
			cnt = 0;
			fade = 0;
			r = SND_MAXTRACK;
			do {
				if (trk->flag & SNDTRK_PLAYING) {
					cnt++;
					if (trk->flag & SNDTRK_FADE) {
						fade++;
					}
				}
				trk++;
			} while(--r);

			if (cnt == 0) {
				break;
			}

			smpcnt = samples;
			if (cnt >= 2) {
				smpcnt = min(smpcnt, SND_MAXSAMPLE);
			}
			if (fade) {
				smpcnt = min(smpcnt, 144);
				trk = snd->trk;
				r = SND_MAXTRACK;
				do {
					if (trk->flag & SNDTRK_FADE) {
						sndsetvol(snd, trk, smpcnt);
					}
					trk++;
				} while(--r);
			}

			trk = snd->trk;
			r = SND_MAXTRACK;

			do {
				if (trk->flag & SNDTRK_PLAYING) {
					break;
				}
				trk++;
			} while(--r);
			if (!r) {
				break;
			}

			samples -= smpcnt;

			if (cnt == 1) {							// １トラックのみ再生中～
				(*trk->cpy16)(trk, pcm, smpcnt);
#if defined(SOUND_MONOOUT)
				pcm += smpcnt;
#else
				pcm += (smpcnt << 1);
#endif
			}
			else {									// 複数トラック再生中～
				ptr = (SINT32 *)(snd + 1);
				(*trk->cpy32)(trk, ptr, smpcnt);

				while(--r) {
					trk++;
					if (trk->flag & SNDTRK_PLAYING) {
						(*trk->add32)(trk, ptr, smpcnt);
					}
				}
#if !defined(SOUND_MONOOUT)
				smpcnt <<= 1;
#endif
				do {
					if (*ptr >= (32768 << SND_VOLSHIFT)) {
						*pcm = 32767;
					}
					else if (*ptr < (-32768 << SND_VOLSHIFT)) {
						*pcm = -32768;
					}
					else {
						*pcm = (SINT16)((*ptr) >> SND_VOLSHIFT);
					}
					ptr++;
					pcm++;
				} while(--smpcnt);
			}
		}
		SNDCSEC_LEAVE;
	}
	if (samples) {
#if defined(SOUND_MONOOUT)
		ZeroMemory(pcm, samples * sizeof(SINT16));
#else
		ZeroMemory(pcm, samples * sizeof(SINT16) * 2);
#endif
	}
	return(0);
}

