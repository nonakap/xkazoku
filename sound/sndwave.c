#include	"compiler.h"
#include	"sound.h"


#if defined(__GNUC__)

typedef struct {
	char	head[4];
	BYTE	size[4];
	char	fmt[4];
} __attribute__ ((packed)) RIFF_HEADER;

typedef struct {
	char	head[4];
	BYTE	size[4];
} __attribute__ ((packed)) WAVE_HEADER;

typedef struct {
	BYTE	format[2];
	BYTE	channel[2];
	BYTE	rate[4];
	BYTE	rps[4];
	BYTE	block[2];
	BYTE	bit[2];
} __attribute__ ((packed)) WAVE_INFOS;

typedef struct {
	BYTE	exsize[2];
	BYTE	spb[2];
	BYTE	numcoef[2];
} __attribute__ ((packed)) WAVE_MSA_INFO;

#else /* __GNUC__ */

#pragma pack(push, 1)
typedef struct {
	char	head[4];
	BYTE	size[4];
	char	fmt[4];
} RIFF_HEADER;

typedef struct {
	char	head[4];
	BYTE	size[4];
} WAVE_HEADER;

typedef struct {
	BYTE	format[2];
	BYTE	channel[2];
	BYTE	rate[4];
	BYTE	rps[4];
	BYTE	block[2];
	BYTE	bit[2];
} WAVE_INFOS;

typedef struct {
	BYTE	exsize[2];
	BYTE	spb[2];
	BYTE	numcoef[2];
} WAVE_MSA_INFO;
#pragma pack(pop)

#endif /* __GNUC__ */

#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
BOOL __mp3_open(SMIXTRACK trk);
#endif

// ---- PCM

static const int alignbit[4] = {0, 1, 0, 2};

static UINT pcm_dec(SMIXTRACK trk, SINT16 *dst) {

	UINT	size;
	int		bit;

	size = sndmix_dataload(trk, trk->block);
	if (size != (UINT)-1) {
		bit = (int)trk->snd;
		size >>= bit;
		if (size) {
			CopyMemory(dst, trk->data, size << bit);
			sndmix_datatrash(trk, size << bit);
		}
	}
	return(size);
}


#ifdef BYTESEX_BIG
static UINT pcm_dec_big(SMIXTRACK trk, SINT16 *dst) {

	UINT	size;
	UINT	cnt;
	int		bit;
	BYTE	*src;

	size = sndmix_dataload(trk, trk->block);
	if (size != (UINT)-1) {
		bit = (int)trk->snd;
		size >>= bit;
		if (size) {
			cnt = size * trk->channels;
			src = trk->data;
			do {
				*dst++ = LOADINTELWORD(src);
				src += 2;
			} while(--cnt);
			sndmix_datatrash(trk, size << bit);
		}
	}
	return(size);
}
#endif


static BOOL pcm_open(SMIXTRACK trk) {

	UINT	align;

	if ((trk->bit != 16) && (trk->bit != 8)) {
		return(SNDMIX_FAILURE);
	}
	align = trk->channels * (trk->bit / 8);
	if ((!align) || (align > 4)) {
		return(SNDMIX_FAILURE);
	}
	if (trk->block != align) {
		return(SNDMIX_FAILURE);
	}
	trk->blksamp = 0x800;
	trk->block *= trk->blksamp;
	trk->snd = (void *)alignbit[align-1];
#ifndef BYTESEX_BIG
	trk->dec = (DECFN)pcm_dec;
#else
	trk->dec = (DECFN)((trk->bit == 16)?pcm_dec_big:pcm_dec);
#endif
	return(SNDMIX_SUCCESS);
}


// ---- IMA

#ifdef WAVE_IMA

#define IMA_MAXSTEP		89

static	BOOL	ima_init = FALSE;
static	BYTE	ima_statetbl[IMA_MAXSTEP][8];

static const int ima_stateadj[8] = {-1, -1, -1, -1, 2, 4, 6, 8};

static const int ima_steptable[IMA_MAXSTEP] = {
	    7,    8,    9,   10,   11,   12,   13,   14,   16,   17,   19,
	   21,   23,   25,   28,   31,   34,   37,   41,   45,   50,   55,
	   60,   66,   73,   80,   88,   97,  107,  118,  130,  143,  157,
	  173,  190,  209,  230,  253,  279,  307,  337,  371,  408,  449,
	  494,  544,  598,  658,  724,  796,  876,  963, 1060, 1166, 1282,
	 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660,
	 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493,10442,
	11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};


static void ima_inittable(void) {

	int		i, j, k;

	for (i=0; i<IMA_MAXSTEP; i++) {
		for (j=0; j<8; j++) {
			k = i + ima_stateadj[j];
			if (k < 0) {
				k = 0;
			}
			else if (k >= IMA_MAXSTEP) {
				k = IMA_MAXSTEP - 1;
			}
			ima_statetbl[i][j] = (BYTE)k;
		}
	}
}


static UINT ima_dec(SMIXTRACK trk, SINT16 *dst) {

	UINT	size;
	UINT	c;
	int		val[2], state[2];
	BYTE	*src;
	int		blk;
	UINT	samp;

	size = sndmix_dataload(trk, trk->block);
	if (size != trk->block) {
		if (size == (UINT)-1) {
			return((UINT)-1);
		}
		else {
			return(0);
		}
	}
	src = trk->data;
	blk = trk->blksamp;

	for (c=0; c<trk->channels; c++) {
		SINT16 tmp;
		tmp = LOADINTELWORD(src);
		val[c] = tmp;
		*dst++ = (SINT16)val[c];
		state[c] = src[2];
		if (state[c] >= IMA_MAXSTEP) {
			state[c] = 0;
		}
		src += 4;
	}
	blk--;

	samp = 0;
	while(blk >= 8) {
		blk -= 8;
		for (c=0; c<trk->channels; c++) {
			UINT r;
			r = 8;
			do {
				int step;
				int dp;
				if (!(r & 1)) {
					samp = *src++;
				}
				else {
					samp >>= 4;
				}
				step = ima_steptable[state[c]];
				state[c] = ima_statetbl[state[c]][samp & 7];
				dp = ((((samp & 7) << 1) + 1) * step) >> 3;
				if (!(samp & 8)) {
					val[c] += dp;
					if (val[c] > 32767) {
						val[c] = 32767;
					}
				}
				else {
					val[c] -= dp;
					if (val[c] < -32768) {
						val[c] = -32768;
					}
				}
				*dst = (SINT16)val[c];
				dst += trk->channels;
			} while(--r);
			dst -= (8 * trk->channels);
			dst++;
		}
		dst += (7 * trk->channels);
	}
	sndmix_datatrash(trk, trk->block);
	return(trk->blksamp);
}


static BOOL ima_open(SMIXTRACK trk) {

	int		blk;

	if (trk->bit != 4) {
		return(SNDMIX_FAILURE);
	}
	blk = trk->block;
	blk /= trk->channels;
	if (blk & 3) {
		return(SNDMIX_FAILURE);
	}
	blk -= 4;				// first block;
	blk *= 2;
	blk += 1;
	trk->blksamp = blk;
	trk->bit = 16;
	trk->dec = (DECFN)ima_dec;
	if (!ima_init) {
		ima_init = TRUE;
		ima_inittable();
	}
	return(SNDMIX_SUCCESS);
}

#endif


// ---- MS-ADPCM

#ifdef WAVE_MSADPCM

typedef struct {
	SINT16 Coef1;
	SINT16 Coef2;
} __COEFPAIR;

static const int MSADPCMTable[16] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230 
};


static UINT msa_dec(SMIXTRACK trk, SINT16 *dst) {

	UINT		size;
	BYTE		*buf;
	int			pred[2];
	int			delta[2];
	int			in;
	int			nibble;
	UINT		i;
	__COEFPAIR	*icoef;

	size = sndmix_dataload(trk, trk->block);
	if (size != trk->block) {
		if (size == (UINT)-1) {
			return((UINT)-1);
		}
		else {
			return(0);
		}
	}
	buf = trk->data;
	icoef = (__COEFPAIR *)trk->snd;

	if (trk->channels == 1) {
		pred[0] = buf[0];
		pred[1] = 0;
		delta[0] = LOADINTELWORD(buf+1);
		delta[1] = 0;
		*dst++ = (SINT16)LOADINTELWORD(buf+5);
		*dst++ = (SINT16)LOADINTELWORD(buf+3);
		buf += 7;
	}
	else {
		pred[0] = buf[0];
		pred[1] = buf[1];
		delta[0] = LOADINTELWORD(buf+2);
		delta[1] = LOADINTELWORD(buf+4);
		*dst++ = (SINT16)LOADINTELWORD(buf+10);
		*dst++ = (SINT16)LOADINTELWORD(buf+12);
		*dst++ = (SINT16)LOADINTELWORD(buf+6);
		*dst++ = (SINT16)LOADINTELWORD(buf+8);
		buf += 14;
	}
	nibble = 0;
	in = 0;
	for (i=2; i<trk->blksamp; i++) {
		UINT ch;
		for (ch=0; ch<trk->channels; ch++) {
			int d, p, out, data;
			d = delta[ch];
			if (nibble) {
				data = in & 15;
			}
			else {
				in = *buf++;
				data = in >> 4;
			}
			nibble ^= 1;
			delta[ch] = (MSADPCMTable[data] * d) >> 8;
			if (delta[ch] < 16) {
				delta[ch] = 16;
			}
			p = (((*(dst-trk->channels)) * icoef[pred[ch]].Coef1) + 
				((*(dst-trk->channels*2)) * icoef[pred[ch]].Coef2));
			p >>= 8;
			out = (((data>=8)?(data-16):data)*d) + p;
			if (out > 32767) {
				out = 32767;
			}
			else if (out < -32768) {
				out = -32768;
			}
			*dst++ = (SINT16)out;
		}
	}
	sndmix_datatrash(trk, trk->block);
	return(trk->blksamp);
}


static void msa_decend(SMIXTRACK trk) {

	_MFREE(trk->snd);
}


static BOOL msa_open(SMIXTRACK trk) {

	WAVE_MSA_INFO	*info;
	BYTE			*coef;
	UINT			exsize;
	UINT			spb;
	UINT			blk;
	UINT			i;
	UINT			numcoef;
	__COEFPAIR		*icoef;

	if (trk->bit != 4) {
		return(SNDMIX_FAILURE);
	}
	if (sndmix_dataload(trk, sizeof(WAVE_MSA_INFO))
												!= sizeof(WAVE_MSA_INFO)) {
		TRACEOUT(("wav: msa: failure read[1]"));
		return(SNDMIX_FAILURE);
	}
	info = (WAVE_MSA_INFO *)trk->data;
	exsize = LOADINTELWORD(info->exsize);
	spb = LOADINTELWORD(info->spb);
	numcoef = LOADINTELWORD(info->numcoef);
	sndmix_dataload(trk, sizeof(WAVE_MSA_INFO));

	blk = trk->block;
	blk /= trk->channels;
	blk -= 6;
	blk *= 2;
	TRACEOUT(("wav: msa: ExtraSize %d / SPB=%d NumOfCoefs=%d",
														exsize, spb, numcoef));

	if (blk != spb) {					// SamplesInBlock size check
		TRACEOUT(("wav: msa: block size error"));
		goto msaopen0;
	}

	if (exsize != (numcoef*4+4)) {		// NumOfCoefs * sizeof(short) * 2 + sizeof(SPB) + sizeof(NumOfCoefs)
		TRACEOUT(("wav: msa: extra info size error"));
		goto msaopen0;
	}

	icoef = (__COEFPAIR*)_MALLOC(numcoef*sizeof(__COEFPAIR), "msadpcm coefs");
	if ( !icoef ) {
		TRACEOUT(("wav: msa: failed to malloc coef table"));
		goto msaopen0;
	}

	if (sndmix_dataload(trk, numcoef << 2) != (numcoef << 2)) {
		TRACEOUT(("wav: msa: failure read[2]"));
		goto msaopen1;
	}
	coef = trk->data;
	for (i=0; i<numcoef; i++) {
		icoef[i].Coef1 = LOADINTELWORD(coef+0);
		icoef[i].Coef2 = LOADINTELWORD(coef+2);
		coef += 4;
	}
	sndmix_datatrash(trk, numcoef << 2);
	trk->snd = (void *)icoef;
	trk->blksamp = spb;
	trk->bit = 16;
	trk->dec = (DECFN)msa_dec;
	trk->decend = msa_decend;

	return(SNDMIX_SUCCESS);

msaopen1:
	_MFREE(icoef);

msaopen0:
	return(SNDMIX_FAILURE);
}

#endif


// ----

int sndwave_open(SMIXTRACK trk) {

	RIFF_HEADER		*riff;
	WAVE_INFOS		*info;
	UINT			fmt;
	long			fpos;
	BOOL			r;
	UINT			datasize;

	if (sndmix_dataload(trk, sizeof(RIFF_HEADER)) != sizeof(RIFF_HEADER)) {
		TRACEOUT(("wav: failure read[0]"));
		goto wavopn_next;
	}
	riff = (RIFF_HEADER *)trk->data;
	if (memcmp(riff->head, "RIFF", 4)) {
		TRACEOUT(("wav:error RIFF header"));
		goto wavopn_next;
	}
	fpos = sizeof(RIFF_HEADER);
	if (!memcmp(riff->fmt, "WAVE", 4)) {
		sndmix_datatrash(trk, sizeof(RIFF_HEADER));
		if (sndmix_dataload(trk, sizeof(WAVE_HEADER))
													!= sizeof(WAVE_HEADER)) {
			TRACEOUT(("wav: failure read[1]"));
			goto wavopn_err;
		}
		if (memcmp(trk->data, "fmt ", 4)) {
			TRACEOUT(("wav: error fmt header"));
			goto wavopn_err;
		}
		fpos += sizeof(WAVE_HEADER);
		fpos += LOADINTELDWORD(trk->data + 4);
		sndmix_datatrash(trk, sizeof(WAVE_HEADER));
		if (sndmix_dataload(trk, sizeof(WAVE_INFOS))
													!= sizeof(WAVE_INFOS)) {
			TRACEOUT(("wav: failure read[2]"));
			goto wavopn_err;
		}
		info = (WAVE_INFOS *)trk->data;
		fmt = LOADINTELWORD(info->format);
		trk->channels = LOADINTELWORD(info->channel);
		trk->samprate = LOADINTELDWORD(info->rate);
		trk->block = LOADINTELWORD(info->block);
		trk->bit = LOADINTELWORD(info->bit);
		TRACEOUT(("wav: fmt: %x / %dch %dHz %dbit",
							fmt, trk->channels, trk->samprate, trk->bit));

		if ((trk->channels != 1) && (trk->channels != 2)) {
			goto wavopn_err;
		}
#ifdef SOUND_MOREINFO
		trk->bps = LOADINTELDWORD(info->rps) * 8;
		trk->fmt = fmt;
		strcpy(trk->info, "SNDWAVE");
#endif
		sndmix_datatrash(trk, sizeof(WAVE_INFOS));
	}
	else if (!memcmp(riff->fmt, "RMP3", 4)) {
		sndmix_datatrash(trk, sizeof(RIFF_HEADER));
		fmt = WAVEFMT_MP3;
	}
	else {
		TRACEOUT(("wav: error WAVE header"));
		goto wavopn_next;
	}

	switch(fmt) {
		case WAVEFMT_PCM:
			r = pcm_open(trk);
			break;

#ifdef WAVE_MSADPCM
		case WAVEFMT_MSADPCM:
			r = msa_open(trk);
			break;
#endif

#ifdef WAVE_IMA
		case WAVEFMT_IMA:
			r = ima_open(trk);
			break;
#endif

#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
		case WAVEFMT_MP3:
			r = SNDMIX_SUCCESS;				// とりあえず成功を返す
			break;
#endif

		default:
			TRACEOUT(("wav: unknown format"));
			r = SNDMIX_FAILURE;
			break;
	}
	if (r) {
		TRACEOUT(("wav: decode open error"));
		goto wavopn_err;
	}

	while(1) {
		sndmix_datatrash(trk, (UINT)-1);
		if (trk->stream_seek(trk->stream, fpos, 0) != fpos) {
			TRACEOUT(("wav: failure seek[0]"));
			goto wavopn_err;
		}
		if (sndmix_dataload(trk, sizeof(WAVE_HEADER))
													!= sizeof(WAVE_HEADER)) {
			TRACEOUT(("wav: failure read[3]"));
			goto wavopn_err;
		}
		fpos += sizeof(WAVE_HEADER);
		datasize = LOADINTELDWORD(trk->data + 4);
		if (!memcmp(trk->data, "data", 4)) {
			break;
		}
		fpos += datasize;
	}
	sndmix_datatrash(trk, sizeof(WAVE_HEADER));

#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)
	if (fmt == WAVEFMT_MP3) {				// MP3はここで確保
		if (__mp3_open(trk) != SNDMIX_SUCCESS) {
			goto wavopn_err;
		}
	}
#endif

	trk->loopfpos = fpos;
	trk->loopsize = datasize;
	trk->fremain = datasize;
	return(SNDMIX_SUCCESS);

wavopn_err:
	return(SNDMIX_FAILURE);

wavopn_next:
	return(SNDMIX_NEXT);
}

