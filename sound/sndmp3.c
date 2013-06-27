#include	"compiler.h"
#include	"sound.h"


#if defined(AMETHYST_LIB) || defined(AMETHYST_OVL)

BOOL __mp3_open(SMIXTRACK trk);

#if defined(AMETHYST_LIB)

#include	"amethyst.h"

#define mp3_getdecver			__mp3_getdecver
#define mp3_create				__mp3_create
#define	mp3_destroy				__mp3_destroy
#define	mp3_predecode			__mp3_predecode
#define	mp3_decode				__mp3_decode
#define	mp3_adjustgain			__mp3_adjustgain

#elif defined(AMETHYST_OVL)

#include	"amethyst_ovl.h"

static	HMODULE			amethyst = NULL;
static	MP3_GETDECVER	mp3_getdecver;
static	MP3_CREATE		mp3_create;
static	MP3_DESTROY		mp3_destroy;
static	MP3_PREDECODE	mp3_predecode;
static	MP3_DECODE		mp3_decode;
static	MP3_ADJUSTGAIN	mp3_adjustgain;

#define	MP3GETDECVER	STRLITERAL("__mp3_getdecver")
#define	MP3CREATE		STRLITERAL("__mp3_create")
#define	MP3DESTROY		STRLITERAL("__mp3_destroy")
#define	MP3PREDECODE	STRLITERAL("__mp3_predecode")
#define	MP3DECODE		STRLITERAL("__mp3_decode")
#define	MP3ADJUSTGAIN	STRLITERAL("__mp3_adjustgain")


void amethsy_init(void) {

	amethyst = LoadLibrary(STRLITERAL("Amethyst.ovl"));
	if (amethyst) {
		mp3_getdecver = (MP3_GETDECVER)GetProcAddress(amethyst, MP3GETDECVER);
		mp3_create = (MP3_CREATE)GetProcAddress(amethyst, MP3CREATE);
		mp3_destroy = (MP3_DESTROY)GetProcAddress(amethyst, MP3DESTROY);
		mp3_predecode = (MP3_PREDECODE)GetProcAddress(
												amethyst, MP3PREDECODE);
		mp3_decode = (MP3_DECODE)GetProcAddress(amethyst, MP3DECODE);
		mp3_adjustgain = (MP3_ADJUSTGAIN)GetProcAddress(
												amethyst, MP3ADJUSTGAIN);
		if (mp3_adjustgain) {
			TRACEOUT(("Amethyst: mp3_adjustgain supported"));
		}
#if 1	// for Amethyst1.05
		if ((!mp3_getdecver) || (!mp3_create) || (!mp3_destroy) ||
			(!mp3_predecode) || (!mp3_decode) || (!mp3_adjustgain))
#else
		if ((!mp3_getdecver) || (!mp3_create) || (!mp3_destroy) ||
			(!mp3_predecode) || (!mp3_decode))
#endif
		{
			FreeLibrary(amethyst);
			amethyst = NULL;
		}
	}
}

void amethsy_term(void) {

	if (amethyst) {
		FreeLibrary(amethyst);
		amethyst = NULL;
	}
}

#endif


static UINT mp3_dec(SMIXTRACK trk, SINT16 *dst) {

	MPEGL3	*mp3;
	int		r;
	UINT	size;

	size = sndmix_dataload(trk, 4);
	if (size != 4) {
		if (size == (UINT)-1) {
			return((UINT)-1);
		}
		else {
			return(0);
		}
	}
	mp3 = (MPEGL3 *)trk->snd;
	r = mp3_predecode(mp3, trk->data);
	if (r) {
#if defined(SUPPORT_VBR)
		if ((r != MPEGHEAD_RENEWAL) ||
			(mp3->c.insize > SUPPORT_VBR) ||
			(mp3->c.samplingrate != (UINT)trk->samprate) ||
			(mp3->c.channels != (UINT)trk->channels)) {
			TRACEOUT(("mp3: decord err"));
			return(0);
		}
#else
		TRACEOUT(("mp3: decord err"));
		return(0);
#endif
	}

	size = sndmix_dataload(trk, mp3->c.insize);
	if (mp3->c.insize != (UINT)size) {
		if (size == (UINT)-1) {
			return((UINT)-1);
		}
		else {
			return(0);
		}
	}
	mp3_decode(mp3, dst, trk->data, mp3->c.insize);
	sndmix_datatrash(trk, mp3->c.insize);
	return(mp3->c.outsamples);
}


static void mp3_decend(SMIXTRACK trk) {

	mp3_destroy((MPEGL3 *)trk->snd);
}


static void mp3_setgain(SMIXTRACK trk, int gain) {

	mp3_adjustgain((MPEGL3 *)trk->snd, gain);
}


BOOL __mp3_open(SMIXTRACK trk) {

	MPEGL3	*mp3;

#if defined(AMETHYST_OVL)
	if (amethyst == NULL) {
		TRACEOUT(("mp3: Amethyst not ready."));
		goto mp3opn_err;
	}
#endif
	if (sndmix_dataload(trk, 4) != 4) {
		goto mp3opn_err;
	}
	mp3 = mp3_create(trk->data);
	if (mp3 == NULL) {
		goto mp3opn_err;
	}

	trk->snd = mp3;
	trk->dec = (DECFN)mp3_dec;
	trk->decend = mp3_decend;

#if 1	// for Amethyst1.05
	trk->decgain = mp3_setgain;
#else
	if (((buf[1] & 6) != 4) && (mp3_adjustgain)) {
		trk->decgain = mp3_setgain;
	}
#endif

	trk->samprate = mp3->c.samplingrate;
	trk->channels = mp3->c.channels;
#if defined(SUPPORT_VBR)
	trk->block = SUPPORT_VBR;
#else
	trk->block = mp3->c.insize + 1;
#endif
	trk->blksamp = mp3->c.outsamples;
	trk->bit = 16;

#if defined(SOUND_MOREINFO)
	trk->bps = mp3->c.kbitrate * 1000;
	if ((buf[1] & 6) == 4) {
		trk->fmt = WAVEFMT_MP2;
	}
	else {
		trk->fmt = WAVEFMT_MP3;
	}
	mp3_getdecver(trk->info, sizeof(trk->info));
#endif

	TRACEOUT(("mp3: %dHz %dkbps", mp3->c.samplingrate, mp3->c.kbitrate));
	return(SNDMIX_SUCCESS);

mp3opn_err:
	return(SNDMIX_FAILURE);
}


int sndmp3_open(SMIXTRACK trk) {

	BYTE	*buf;
	long	headpos;
	UINT	loopsize = 0xffffffff;				// loopsize unknown
	int		r;

	r = SNDMIX_NOTSUPPORT;
	if (sndmix_dataload(trk, 10) != 10) {
		TRACEOUT(("mp3: failure head [0]"));
		goto mp3opn_err;
	}

	buf = trk->data;
	headpos = 0;
	if (!memcmp(buf, "ID3", 3)) {
		headpos = (buf[6] & 0x7f);
		headpos <<= 7;
		headpos |= (buf[7] & 0x7f);
		headpos <<= 7;
		headpos |= (buf[8] & 0x7f);
		headpos <<= 7;
		headpos |= (buf[9] & 0x7f);
		headpos += 10;
		TRACEOUT(("mp3: ID3 Tag - size:%dbyte(s)", headpos));
		sndmix_datatrash(trk, (UINT)-1);
		if ((trk->stream.ssseek == NULL) ||
			(trk->stream.ssseek(&trk->stream, headpos, SSSEEK_SET)
															!= headpos)) {
			TRACEOUT(("mp3: failure seek head"));
			r = SNDMIX_STREAMERROR;
			goto mp3opn_err;
		}
		r = SNDMIX_DATAERROR;
	}

	if (__mp3_open(trk) != SNDMIX_SUCCESS) {
		goto mp3opn_err;
	}

	trk->loopfpos = headpos;
	trk->loopsize = loopsize;
	trk->fremain = trk->loopsize;
	return(SNDMIX_SUCCESS);

mp3opn_err:
	return(r);
}

#endif

