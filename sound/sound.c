#include	"compiler.h"
#include	"sound.h"


static	SOUND_T		*snd = NULL;

// FOR I=-16 TO 8:PRINT INT((2^(I/4)) * 256);",";:NEXT
static const int gaintbl[24+1] =
				{ 16,  19,  22,  26,  32,  38,  45,  53,
				  64,  76,  90, 107, 128, 152, 181, 215,
				 256, 304, 362, 430, 512, 608, 724, 861, 1024};


UINT sndmix_dataload(SMIXTRACK trk, UINT size) {

	if (size <= trk->indatas) {
		return(size);
	}
	else {
		size -= trk->indatas;
		size = min(size, trk->fremain);
		size = trk->stream_read(trk->stream, trk->data + trk->indatas, size);
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
		sndmix_datatrash(trk, ret);
		size -= ret;
	}
	if (size) {
		size = trk->stream_read(trk->stream, ((BYTE *)buf) + ret, size);
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

	soundmix_destory();

	if ((basehz < 8000) || (basehz > 48000)) {
		return;
	}

	size = sizeof(int) * 2 * SND_MAXSAMPLE;
	size += sizeof(SOUND_T);
	snd = (SOUND_T *)_MALLOC(size, "SOUND");
	if (snd) {
		ZeroMemory(snd, size);
		snd->basehz = basehz;
	}
}


void soundmix_destory(void) {

	if (snd) {
		SOUND_T *tmp;
		int		i;

		for (i=0; i<SND_MAXTRACK; i++) {
			soundmix_unload(i);
		}
		tmp = snd;
		snd = NULL;
		_MFREE(tmp);
	}
}


static void sndsetvol(SMIXTRACK trk, UINT samp) {

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


static void sndsetgain(SMIXTRACK trk, int gain) {

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
	sndsetvol(trk, 0);
}


static void setfade(SMIXTRACK trk, UINT basehz, int tick, int dir) {

	if (tick < 0) {
		tick = 0;
	}
	basehz *= tick;
	basehz /= 1000;

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
}


BOOL soundmix_load(int num, PCMSTREAM *stream, void *arg) {

	SMIXTRACK	trk;
	int			r;
	int			outsize;
	int			size;
	int			ptr;
	BYTE		data[512];

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		goto smld_err0;
	}
	soundmix_unload(num);
	if (stream == NULL) {
		goto smld_err0;
	}
	if ((stream->stream_read == NULL) ||
		(stream->stream_seek == NULL)) {
		goto smld_err0;
	}
	trk = snd->trk + num;
	ZeroMemory(trk, sizeof(_SMIXTRACK));
	trk->stream_read = stream->stream_read;
	trk->stream_seek = stream->stream_seek;
	trk->stream_close = stream->stream_close;
	if (stream->stream_open) {
		trk->stream = stream->stream_open(arg, num);
	}
	trk->data = data;
	trk->maxdatas = sizeof(data);
	trk->fremain = (UINT)-1;

	r = sndwave_open(trk);
#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
	if (r == SNDMIX_NEXT) {
		r = sndmp3_open(trk);
	}
#ifdef USE_EXSE
	if (r == SNDMIX_NEXT) {
		r = sndexse_open(trk);
	}
#endif
#endif
#if defined(OGGVORBIS_LIB)
	if (r == SNDMIX_NEXT) {
		r = sndogg_open(trk);
	}
#endif
#ifdef KSYS_KPV0
	if (r == SNDMIX_NEXT) {
		r = sndkpv_open(trk);
	}
#endif
	if (r != SNDMIX_SUCCESS) {
		goto smld_err1;
	}
	if (sndmix_setmixproc(trk, snd->basehz) != SUCCESS) {
		goto smld_err1;
	}

	trk->maxdatas = max(trk->indatas, trk->block);
	outsize = (trk->bit / 8);
	outsize *= trk->channels;
	outsize *= trk->blksamp;
	size = outsize + trk->maxdatas + 64;
	trk->buffer = (BYTE *)_MALLOC(size, "SOUND track");
	if (trk->buffer == NULL) {
		goto smld_err1;
	}
	ptr = (outsize + 16 + 15) & (~(0x0f));
	trk->data = trk->buffer + ptr;
	if (trk->indatas) {
		CopyMemory(trk->data, data, trk->indatas);
	}
	sndsetgain(trk, snd->gain[num]);
	trk->flag = SNDTRK_LOADED;
	TRACEOUT(("sound load success"));
	return(SUCCESS);

smld_err1:
	if (trk->decend) {
		trk->decend(trk);         // ←020618 メモリリークゆいたん修正
	}
	if (trk->stream_close) {
		trk->stream_close(trk->stream);
	}

smld_err0:
	return(FAILURE);
}


void soundmix_unload(int num) {

	SMIXTRACK	trk;

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	if (trk->flag & SNDTRK_LOADED) {
		trk->flag = 0;
		if (trk->decend) {
			trk->decend(trk);         // ←020618 メモリリークゆいたん修正
		}
		if (trk->stream_close) {
			trk->stream_close(trk->stream);
		}
		if (trk->buffer) {
			_MFREE(trk->buffer);
			trk->buffer = NULL;
		}
	}
}


void soundmix_play(int num, int loop, int fadeintick) {

	SMIXTRACK	trk;

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	if (trk->flag & SNDTRK_LOADED) {
		trk->loop = loop;
		if (!(trk->flag & SNDTRK_PLAYING)) {
			trk->flag |= SNDTRK_PLAYING;
			TRACEOUT(("sound play track:%d", num));
			setfade(trk, snd->basehz, fadeintick, 0);
		}
	}
}


void soundmix_stop(int num, int fadeouttick) {

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	setfade(snd->trk + num, snd->basehz, fadeouttick, 1);
}


void soundmix_rew(int num) {

	SMIXTRACK	trk;

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	trk = snd->trk + num;
	if ((trk->flag & (SNDTRK_LOADED | SNDTRK_PLAYING)) != SNDTRK_LOADED) {
		return;
	}
	trk->fremain = trk->loopsize;
	sndmix_datatrash(trk, (UINT)-1);
	trk->stream_seek(trk->stream, trk->loopfpos, 0);
}


void soundmix_continue(int num) {

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK)) {
		return;
	}
	if (snd->trk[num].flag & SNDTRK_PAUSED) {
		snd->trk[num].flag &= ~SNDTRK_PAUSED;
		snd->trk[num].flag |= SNDTRK_PLAYING;
	}
}


BOOL soundmix_isplaying(int num) {

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK) ||
		(!(snd->trk[num].flag & SNDTRK_PLAYING))) {
		return(FALSE);
	}
	return(TRUE);
}


#ifdef SOUND_MOREINFO
SMIXTRACK soundmix_getinfo(int num) {

	if ((snd == NULL) || ((unsigned int)num >= SND_MAXTRACK) ||
		(!(snd->trk[num].flag & SNDTRK_LOADED))) {
		return(NULL);
	}
	return(snd->trk + num);
}
#endif


void soundmix_setgain(int num, int gain) {

	if ((snd != NULL) && ((unsigned int)num < SND_MAXTRACK)) {
		snd->gain[num] = gain;
		sndsetgain(snd->trk + num, gain);
	}
}


UINT soundmix_getpcm(SINT16 *pcm, UINT samples) {

	UINT		cnt;
	int			fade;
	SMIXTRACK	trk;
	UINT		r;
	UINT		smpcnt;
	SINT32		*ptr;

	if (snd) {
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
						sndsetvol(trk, smpcnt);
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

			if (cnt == 1) {							// １トラックのみ再生中〜
				(*trk->cpy16)(trk, pcm, smpcnt);
#if defined(SOUND_MONOOUT)
				pcm += smpcnt;
#else
				pcm += (smpcnt << 1);
#endif
			}
			else {									// 複数トラック再生中〜
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

