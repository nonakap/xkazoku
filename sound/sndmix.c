#include	"compiler.h"
#include	"sound.h"


#define	DNBASEBITS	9
#define	DNMIXBASE	(1 << DNBASEBITS)

#define	UPBASEBITS	8
#define	UPMIXBASE	(1 << UPBASEBITS)

// 偽物てんぷれーと
// マイクロソフトはマクロ展開下手だから動作チェキするように。

#if defined(SOUND_MONOOUT)

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c16_mn8n
#define	FUNC_DOWN		c16_mn8dn
#define	FUNC_UP			c16_mn8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c32_mn8n
#define	FUNC_DOWN		c32_mn8dn
#define	FUNC_UP			c32_mn8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		a32_mn8n
#define	FUNC_DOWN		a32_mn8dn
#define	FUNC_UP			a32_mn8up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"


#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c16_st8n
#define	FUNC_DOWN		c16_st8dn
#define	FUNC_UP			c16_st8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c32_st8n
#define	FUNC_DOWN		c32_st8dn
#define	FUNC_UP			c32_st8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		a32_st8n
#define	FUNC_DOWN		a32_st8dn
#define	FUNC_UP			a32_st8up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c16_mn16n
#define	FUNC_DOWN		c16_mn16dn
#define	FUNC_UP			c16_mn16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c32_mn16n
#define	FUNC_DOWN		c32_mn16dn
#define	FUNC_UP			c32_mn16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		a32_mn16n
#define	FUNC_DOWN		a32_mn16dn
#define	FUNC_UP			a32_mn16up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c16_st16n
#define	FUNC_DOWN		c16_st16dn
#define	FUNC_UP			c16_st16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c32_st16n
#define	FUNC_DOWN		c32_st16dn
#define	FUNC_UP			c32_st16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		a32_st16n
#define	FUNC_DOWN		a32_st16dn
#define	FUNC_UP			a32_st16up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixmn16.mcr"


// ヴォリューム掛け算なしバージョン 単独再生のみ
#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		xc16_mn8n
#define	FUNC_DOWN		xc16_mn8dn
#define	FUNC_UP			xc16_mn8up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		xc16_st8n
#define	FUNC_DOWN		xc16_st8dn
#define	FUNC_UP			xc16_st8up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixmn16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		xc16_mn16n
#define	FUNC_DOWN		xc16_mn16dn
#define	FUNC_UP			xc16_mn16up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixmn16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		xc16_st16n
#define	FUNC_DOWN		xc16_st16dn
#define	FUNC_UP			xc16_st16up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixmn16.mcr"

#else

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c16_mn8n
#define	FUNC_DOWN		c16_mn8dn
#define	FUNC_UP			c16_mn8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c32_mn8n
#define	FUNC_DOWN		c32_mn8dn
#define	FUNC_UP			c32_mn8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		a32_mn8n
#define	FUNC_DOWN		a32_mn8dn
#define	FUNC_UP			a32_mn8up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"


#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c16_st8n
#define	FUNC_DOWN		c16_st8dn
#define	FUNC_UP			c16_st8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c32_st8n
#define	FUNC_DOWN		c32_st8dn
#define	FUNC_UP			c32_st8up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		a32_st8n
#define	FUNC_DOWN		a32_st8dn
#define	FUNC_UP			a32_st8up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c16_mn16n
#define	FUNC_DOWN		c16_mn16dn
#define	FUNC_UP			c16_mn16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		c32_mn16n
#define	FUNC_DOWN		c32_mn16dn
#define	FUNC_UP			c32_mn16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	1
#define	FUNC_NOR		a32_mn16n
#define	FUNC_DOWN		a32_mn16dn
#define	FUNC_UP			a32_mn16up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c16_st16n
#define	FUNC_DOWN		c16_st16dn
#define	FUNC_UP			c16_st16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		c32_st16n
#define	FUNC_DOWN		c32_st16dn
#define	FUNC_UP			c32_st16up
#undef	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	32
#define	MIX_CHANNELS	2
#define	FUNC_NOR		a32_st16n
#define	FUNC_DOWN		a32_st16dn
#define	FUNC_UP			a32_st16up
#define	SAMPADD
#undef	NOMATTER_VOL
#include "mixst16.mcr"


// ヴォリューム掛け算なしバージョン 単独再生のみ
#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		xc16_mn8n
#define	FUNC_DOWN		xc16_mn8dn
#define	FUNC_UP			xc16_mn8up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	8
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		xc16_st8n
#define	FUNC_DOWN		xc16_st8dn
#define	FUNC_UP			xc16_st8up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixst16.mcr"


#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	1
#define	FUNC_NOR		xc16_mn16n
#define	FUNC_DOWN		xc16_mn16dn
#define	FUNC_UP			xc16_mn16up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixst16.mcr"

#define	MIX_INPUTBIT	16
#define	MIX_OUTPUTBIT	16
#define	MIX_CHANNELS	2
#define	FUNC_NOR		xc16_st16n
#define	FUNC_DOWN		xc16_st16dn
#define	FUNC_UP			xc16_st16up
#undef	SAMPADD
#define	NOMATTER_VOL
#include "mixst16.mcr"

#endif


// ---- function...

static BOOL loopproc(SMIXTRACK trk) {

	int		r;

	if (trk->loop) {
		trk->samplepos = 0;
		r = trk->decrew(trk);
		if (r == SNDMIX_SUCCESS) {
			return(SNDMIX_SUCCESS);
		}
		trk->flag |= SNDTRK_PAUSED;
	}
	else {
		trk->flag |= SNDTRK_STREAMEND;
	}
	trk->flag &= ~SNDTRK_PLAYING;
	return(SNDMIX_FAILURE);
}


static BOOL pcmget(SMIXTRACK trk) {

	UINT	r;

	trk->buf = trk->buffer;
	r = trk->dec(trk, trk->buffer);
	if (r == (UINT)-1) {
		goto pcmget_paused;
	}
	if (!r) {
		if (loopproc(trk) != SNDMIX_SUCCESS) {
			goto pcmget_stop;
		}
		trk->samplepos = 0;
		r = trk->dec(trk, trk->buffer);
		if (r == (UINT)-1) {
			goto pcmget_paused;
		}
		if (!r) {
			goto pcmget_stop;
		}
	}
	trk->samplepos += r;
	trk->remain = r;
	return(SNDMIX_SUCCESS);

pcmget_stop:
	trk->remain = 0;
	trk->flag &= ~SNDTRK_PLAYING;
	trk->flag |= SNDTRK_STREAMEND;
	return(SNDMIX_FAILURE);

pcmget_paused:
	trk->flag |= SNDTRK_PAUSED;
	trk->flag &= ~SNDTRK_PLAYING;
	return(SNDMIX_FAILURE);
}


static void pcmfunc16(SMIXTRACK trk, SINT16 *pcm, UINT samples) {

	SINT16	*pcmterm;

#if defined(SOUND_MONOOUT)
	pcmterm = pcm + samples;
#else
	pcmterm = pcm + samples * 2;
#endif
	while(pcm < pcmterm) {
		if (trk->remain) {
			pcm = (SINT16 *)trk->cpy16cnv(trk, pcm, pcmterm);
		}
		if ((!trk->remain) && (pcmget(trk) != SNDMIX_SUCCESS)) {
			break;
		}
	}
	if (pcm < pcmterm) {
		ZeroMemory(pcm, (int)pcmterm - (int)pcm);
	}
}


static void pcmfunccpy32_16(SMIXTRACK trk, SINT32 *pcm, UINT samples) {

	SINT32	*pcmterm;

#if defined(SOUND_MONOOUT)
	pcmterm = pcm + samples;
#else
	pcmterm = pcm + samples * 2;
#endif
	while(pcm < pcmterm) {
		if (trk->remain) {
			pcm = (SINT32 *)trk->cpy32cnv(trk, pcm, pcmterm);
		}
		if ((!trk->remain) && (pcmget(trk) != SNDMIX_SUCCESS)) {
			break;
		}
	}
	if (pcm < pcmterm) {
		ZeroMemory(pcm, (int)pcmterm - (int)pcm);
	}
}


static void pcmfuncadd32_16(SMIXTRACK trk, SINT32 *pcm, UINT samples) {

	SINT32	*pcmterm;

#if defined(SOUND_MONOOUT)
	pcmterm = pcm + samples;
#else
	pcmterm = pcm + samples * 2;
#endif
	while(pcm < pcmterm) {
		if (trk->remain) {
			pcm = (SINT32 *)trk->add32cnv(trk, pcm, pcmterm);
		}
		if ((!trk->remain) && (pcmget(trk) != SNDMIX_SUCCESS)) {
			break;
		}
	}
}


static const MIXFN2 cnvfunc_16[] = {
				(MIXFN2)c16_mn16n,	(MIXFN2)c16_mn16dn,	(MIXFN2)c16_mn16up,
				(MIXFN2)c16_st16n,	(MIXFN2)c16_st16dn,	(MIXFN2)c16_st16up,
				(MIXFN2)c16_mn8n,	(MIXFN2)c16_mn8dn,	(MIXFN2)c16_mn8up,
				(MIXFN2)c16_st8n,	(MIXFN2)c16_st8dn,	(MIXFN2)c16_st8up,
				(MIXFN2)xc16_mn16n,	(MIXFN2)xc16_mn16dn,(MIXFN2)xc16_mn16up,
				(MIXFN2)xc16_st16n,	(MIXFN2)xc16_st16dn,(MIXFN2)xc16_st16up,
				(MIXFN2)xc16_mn8n,	(MIXFN2)xc16_mn8dn,	(MIXFN2)xc16_mn8up,
				(MIXFN2)xc16_st8n,	(MIXFN2)xc16_st8dn,	(MIXFN2)xc16_st8up};

static const MIXFN2 cnvfunc_c32_16[] = {
				(MIXFN2)c32_mn16n, (MIXFN2)c32_mn16dn, (MIXFN2)c32_mn16up,
				(MIXFN2)c32_st16n, (MIXFN2)c32_st16dn, (MIXFN2)c32_st16up,
				(MIXFN2)c32_mn8n,  (MIXFN2)c32_mn8dn,  (MIXFN2)c32_mn8up,
				(MIXFN2)c32_st8n,  (MIXFN2)c32_st8dn,  (MIXFN2)c32_st8up};

static const MIXFN2 cnvfunc_a32_16[] = {
				(MIXFN2)a32_mn16n, (MIXFN2)a32_mn16dn, (MIXFN2)a32_mn16up,
				(MIXFN2)a32_st16n, (MIXFN2)a32_st16dn, (MIXFN2)a32_st16up,
				(MIXFN2)a32_mn8n,  (MIXFN2)a32_mn8dn,  (MIXFN2)a32_mn8up,
				(MIXFN2)a32_st8n,  (MIXFN2)a32_st8dn,  (MIXFN2)a32_st8up};


BOOL sndmix_setmixproc(SMIXTRACK trk, UINT basehz) {

	int		funcnum;

	if ((trk->samprate < 8000) || (trk->samprate > 48000)) {
		return(SNDMIX_FAILURE);
	}
	trk->cpy16 = (MIXFN)pcmfunc16;
	trk->cpy32 = (MIXFN)pcmfunccpy32_16;
	trk->add32 = (MIXFN)pcmfuncadd32_16;
	funcnum = 0;
	if (trk->channels == 2) {
		funcnum += 3;
	}
	if (trk->bit == 8) {
		funcnum += 6;
	}

	if (trk->samprate > basehz) {
		trk->mrate = DNMIXBASE * basehz / trk->samprate;
		trk->rem = DNMIXBASE;
		funcnum += 1;
	}
	else if (trk->samprate < basehz) {
		trk->mrate = UPMIXBASE * basehz / trk->samprate;
		funcnum += 2;
	}
	trk->cpy32cnv = cnvfunc_c32_16[funcnum];
	trk->add32cnv = cnvfunc_a32_16[funcnum];

	if (trk->vol == (1 << SND_VOLSHIFT)) {
		funcnum += 12;
	}
	trk->cpy16cnv = cnvfunc_16[funcnum];
	return(SNDMIX_SUCCESS);
}

