#include	"compiler.h"
#include	<limits.h>
#include	"sound.h"


#if defined(VERMOUTH_LIB)

#include	"vermouth.h"

#define		MIDIBASERATE	22050
#define		MIDIBUFFERS		(4 * 576)


int __midi_open(SMIXTRACK trk);


static	MIDIMOD		module = NULL;

static const BYTE EXCV_GMRESET[] = {
			0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};


void vermouth_init(void) {

	int		i;
	MIDIMOD	hdl;

	if (module == NULL) {
		hdl = midimod_create(MIDIBASERATE);
		module = hdl;
		if (hdl) {
			for (i=0; i<128; i++) {
				midimod_loadprogram(hdl, i);
				midimod_loadrhythm(hdl, i);
			}
		}
	}
}

void vermouth_term(void) {

	if (module) {
		midimod_destroy(module);
		module = NULL;
	}
}


// ---- midifile

typedef struct {
const BYTE	*data;
	int		size;
	UINT	step;
	BYTE	status;
} _MIDITRK, *MIDITRK;

typedef struct {
	UINT	size;
	UINT	base;

	MIDIHDL	hdl;

	UINT	tracks;
	UINT	timebase;
const BYTE	*data;
	UINT	datasize;

	UINT	enable;

	UINT	waitcount;

	UINT	stepinc;
	UINT	stepincfix;
	UINT	stepincmask;
	UINT	stepfix;

	UINT	steprem;
	UINT	stepnext;

	UINT	tempo;
} _MIDIFILE, *MIDIFILE;

typedef struct {
	char	sig[4];
	BYTE	size[4];
} _MIDIBLOCK, *MIDIBLOCK;

typedef struct {
	BYTE	format[2];
	BYTE	track[2];
	BYTE	timebase[2];
} _MIDIHEAD;

#define	STEPDIV		2


static void calctempo(MIDIFILE mf) {

	int		exp;
	UINT	tempo;
	UINT	mulmax;

	tempo = mf->tempo;
	if (tempo < (15000000 / 500)) {
		tempo = 15000000 / 500;					// min: 30000
	}
	else if (tempo >= 15000000 / 5) {
		tempo = 15000000 / 5;					// max: 3000000
	}
	exp = -6;
	while(tempo < ((UINT_MAX >> (STEPDIV + 2)) + 1)) {
		tempo <<= 1;
		exp -= 1;
	}
	tempo /= 15625;
	mulmax = (UINT_MAX >> 1);
	mulmax /= mf->base;
	while(tempo > mulmax) {
		tempo >>= 1;
		exp += 1;
	}
	tempo *= mf->base;
	while(tempo < ((UINT_MAX >> (STEPDIV + 2)) + 1)) {
		tempo <<= 1;
		exp -= 1;
	}
	tempo /= mf->timebase;
	if (exp >= 0) {
		mf->stepinc = (UINT_MAX >> (STEPDIV + 1)) / mf->timebase;
		mf->stepincfix = 0;
		mf->stepincmask = 0;
		mf->stepfix = 0;
	}
	else {
		while(exp < -29) {
			tempo >>= 1;
			exp++;
		}
		if (tempo == 0) {
			tempo = 1;
		}
		while(!(tempo & 1)) {
			tempo >>= 1;
			exp++;
		}
		mf->stepinc = tempo;
		exp += mf->stepincfix;
		if (exp < 0) {
			mf->stepfix <<= (0 - exp);
		}
		else if (exp > 0) {
			mf->stepfix >>= exp;
		}
		mf->stepincfix -= exp;
		mf->stepincmask = (1 << mf->stepincfix) - 1;
	}
	// stepinc = mf->tempo(us) * mf->base(hz) / mf->timebase(step) / 1000000
	//         = mf->stepinc * 2^(-mf->stepincfix)
}

static UINT restart(MIDIFILE mf) {

	UINT		tracks;
	MIDITRK		trk;
const BYTE		*data;
	UINT		datasize;
	UINT		enable;
	MIDIBLOCK	mblk;
	UINT		size;
	UINT		delta;

	if (mf == NULL) {
		return(0);
	}
	trk = (MIDITRK)(mf + 1);
	tracks = mf->tracks;
	data = mf->data;
	datasize = mf->datasize;
	enable = 0;
	while(tracks--) {
		if (datasize < sizeof(_MIDIBLOCK)) {
			break;
		}
		mblk = (MIDIBLOCK)data;
		data += sizeof(_MIDIBLOCK);
		datasize -= sizeof(_MIDIBLOCK);
		size = (mblk->size[0] << 24) | (mblk->size[1] << 16) |
				(mblk->size[2] << 8) | mblk->size[3];
		size = min(size, datasize);
		if (memcmp(mblk->sig, "MTrk", 4)) {
			break;
		}
		trk->data = data;
		trk->size = size;
		trk->step = 0;
		trk->status = 0x80;
		data += size;

		do {
			trk->size--;
			if (trk->size < 0) {
				break;
			}
			delta = *trk->data++;
			trk->step <<= 7;
			trk->step += (delta & 0x7f);
		} while(delta & 0x80);
		if (trk->size < 0) {
			continue;
		}

		trk++;
		enable++;
	}
	mf->enable = enable;
	mf->waitcount = 0;
	mf->stepfix = 0;
	mf->steprem = 0;
	mf->stepnext = 0;
	mf->tempo = 500000;
	calctempo(mf);
	return(enable);
}

static BOOL trackskip(MIDITRK trk) {

	UINT	delta;
	BYTE	c;

	delta = 0;
	do {
		trk->size--;
		if (trk->size < 0) {
			return(FAILURE);
		}
		c = *trk->data++;
		delta <<= 7;
		delta += (c & 0x7f);
	} while(c & 0x80);
	if ((UINT)trk->size < delta) {
		return(FAILURE);
	}
	trk->size -= delta;
	trk->data += delta;
	return(SUCCESS);
}

static UINT midifile_trkstep(MIDIFILE mf, MIDITRK trk, UINT step) {

	UINT	forward;
	UINT	delta;
	BYTE	c1;
	BYTE	c2;
	BYTE	c3;
	BYTE	c;

	forward = min(trk->step, step);
	step -= forward;
	trk->step -= forward;
	while(!trk->step) {
		trk->size--;
		if (trk->size < 0) {
			goto trk_done;
		}
		c1 = *trk->data++;
		if (c1 < 0xf0) {
			if (c1 & 0x80) {
				trk->status = c1;
				trk->size--;
				if (trk->size < 0) {
					goto trk_done;
				}
				c2 = *trk->data++;
			}
			else {
				c2 = c1;
				c1 = trk->status;
			}
			if ((c1 >= 0xc0) && (c1 < 0xe0)) {
				midiout_shortmsg(mf->hdl, (c2 << 8) | c1);
//				TRACEOUT(("mid: msg %02x %02x", c1, c2));
			}
			else {
				trk->size--;
				if (trk->size < 0) {
					goto trk_done;
				}
				c3 = *trk->data++;
				midiout_shortmsg(mf->hdl, (c3 << 16) | (c2 << 8) | c1);
//				TRACEOUT(("mid: msg %02x %02x %02x", c1, c2, c3));
			}
		}
		else if ((c1 == 0xf0) || (c1 == 0xf7)) {
			if (trackskip(trk) != SUCCESS) {
				goto trk_done;
			}
		}
		else if (c1 == 0xff) {
			trk->size--;
			if (trk->size < 0) {
				goto trk_done;
			}
			c2 = *trk->data++;
			if (c2 == 0x2f) {
				return(0);
			}
			else if (c2 == 0x51) {
				if ((trk->size >= 4) || (*trk->data == 3)) {
					mf->tempo = (trk->data[1] << 16) |
								(trk->data[2] << 8) | trk->data[3];
					calctempo(mf);
				}
			}
			if (trackskip(trk) != SUCCESS) {
				goto trk_done;
			}
		}
		else {
			TRACEOUT(("mid: unknown data %02x", c1));
			goto trk_done;
		}

		delta = 0;
		do {
			trk->size--;
			if (trk->size < 0) {
				goto trk_done;
			}
			c = *trk->data++;
			delta <<= 7;
			delta += (c & 0x7f);
		} while(c & 0x80);
		trk->step = delta;
	}

trk_done:
	return(trk->step);
}


// ----

static UINT mid_dec(SMIXTRACK trk, SINT16 *dst) {

	MIDIFILE	mf;
	UINT		count;
	UINT		ret;
	MIDITRK		p;
	MIDITRK		q;
	UINT		enable;
	UINT		cnt;
	UINT		nextcnt;

	mf = (MIDIFILE)trk->snd;
	count = MIDIBUFFERS;
	ret = 0;
	if ((mf == NULL) || (mf->enable == 0)) {
		goto mf_stop;
	}
	while(mf->waitcount <= count) {
		ret += mf->waitcount;
		count -= mf->waitcount;
		midiout_get(mf->hdl, dst, mf->waitcount);
		dst += mf->waitcount * 2;
		if (mf->steprem == 0) {
			p = (MIDITRK)(mf + 1);
			q = p;
			enable = mf->enable;
			nextcnt = UINT_MAX;
			while(enable--) {
				cnt = midifile_trkstep(mf, p, mf->stepnext);
				if (cnt) {
					nextcnt = min(nextcnt, cnt);
					if (q != p) {
						*q = *p;
					}
					q++;
				}
				else {
					mf->enable--;
					TRACEOUT(("mid: enable %d", mf->enable));
					if (mf->enable == 0) {
						goto mf_stop;
					}
				}
				p++;
			}
			mf->stepnext = nextcnt;
			mf->steprem = nextcnt;
		}
		cnt = min(mf->steprem, mf->timebase << STEPDIV);
		mf->steprem -= cnt;
		cnt *= mf->stepinc;
		cnt += mf->stepfix;
		mf->stepfix = cnt & mf->stepincmask;
		mf->waitcount = cnt >> mf->stepincfix;
	}
	mf->waitcount -= count;
	ret += count;
	midiout_get(mf->hdl, dst, count);

mf_stop:
	return(ret);
}

static void mid_decend(SMIXTRACK trk) {

	MIDIFILE	mf;

	mf = (MIDIFILE)trk->snd;
	if (mf) {
		midiout_destroy(mf->hdl);
		_MFREE(mf);
	}
}

static int mid_rew(SMIXTRACK trk) {

	restart((MIDIFILE)trk->snd);
	return(SNDMIX_SUCCESS);
}

static void mid_setgain(SMIXTRACK trk, int gain) {

	midiout_setgain(((MIDIFILE)(trk->snd))->hdl, gain);
}


int __midi_open(SMIXTRACK trk) {

	MIDIHDL		hdl;
	MIDIFILE	mf;
	_MIDIBLOCK	mblk;
	_MIDIHEAD	mhead;
	UINT		headsize;
	UINT		format;
	UINT		tracks;
	UINT		timebase;
	UINT		filesize;
	UINT		size;
	BYTE		*dat;

	sndmix_datatrash(trk, (UINT)-1);

	hdl = midiout_create(module, MIDIBUFFERS);
	if (hdl == NULL) {
		goto mfop_err1;
	}
	if (trk->stream.ssseek == NULL) {
		goto mfop_err1;
	}
	filesize = trk->stream.ssseek(&trk->stream, 0, 2);
	if (filesize == (UINT)-1) {
		goto mfop_err2;
	}
	if ((trk->stream.ssseek == NULL) ||
		(trk->stream.ssseek(&trk->stream, 0, 0) != 0)) {
		goto mfop_err2;
	}
	if ((trk->stream.ssread == NULL) ||
		(trk->stream.ssread(&trk->stream, &mblk, sizeof(mblk))
														!= sizeof(mblk)) ||
		(memcmp(mblk.sig, "MThd", 4))) {
		TRACEOUT(("mid: not hit MThd"));
		goto mfop_err2;
	}
	headsize = (mblk.size[0] << 24) | (mblk.size[1] << 16) |
			(mblk.size[2] << 8) | mblk.size[3];
	if (headsize < sizeof(_MIDIHEAD)) {
		goto mfop_err2;
	}
	if ((trk->stream.ssread == NULL) ||
		(trk->stream.ssread(&trk->stream, &mhead, sizeof(mhead))
														!= sizeof(mhead))) {
		goto mfop_err2;
	}
	format = (mhead.format[0] << 8) + mhead.format[1];
	tracks = (mhead.track[0] << 8) + mhead.track[1];
	timebase = (mhead.timebase[0] << 8) + mhead.timebase[1];
	TRACEOUT(("mid: format %d", format));
	TRACEOUT(("mid: tracks %d", tracks));
	TRACEOUT(("mid: timebase %d", timebase));
	if (format == 0) {
		tracks = 1;
	}
	else if (format != 1) {
		tracks = 0;
	}
	if ((tracks == 0) || (timebase == 0)) {
		goto mfop_err2;
	}
	headsize += sizeof(_MIDIBLOCK);
	if (headsize >= filesize) {
		goto mfop_err2;
	}
	if ((trk->stream.ssseek == NULL) ||
		(trk->stream.ssseek(&trk->stream, headsize, 0) != (long)headsize)) {
		goto mfop_err2;
	}
	filesize -= headsize;

	size = sizeof(_MIDIFILE);
	size += tracks * sizeof(_MIDITRK);
	size += filesize;
	mf = (MIDIFILE)_MALLOC(size, "midi");
	if (mf == NULL) {
		goto mfop_err2;
	}
	dat = (BYTE *)mf;
	dat += sizeof(_MIDIFILE);
	dat += tracks * sizeof(_MIDITRK);
	if ((trk->stream.ssread == NULL) ||
		(trk->stream.ssread(&trk->stream, dat, filesize) != filesize)) {
		goto mfop_err3;
	}
	mf->size = size;
	mf->base = MIDIBASERATE;
	mf->hdl = hdl;

	mf->tracks = tracks;
	mf->timebase = timebase;
	mf->data = dat;
	mf->datasize = filesize;
	if (restart(mf) == 0) {
		TRACEOUT(("mid: none channel"));
		goto mfop_err3;
	}
	TRACEOUT(("mid: open success"));

	midiout_longmsg(hdl, EXCV_GMRESET, sizeof(EXCV_GMRESET));

	trk->snd = mf;
	trk->dec = (DECFN)mid_dec;
	trk->decend = mid_decend;
	trk->decrew = mid_rew;
	trk->decgain = mid_setgain;

	trk->samprate = MIDIBASERATE;
	trk->channels = 2;
	trk->block = 0;
	trk->blksamp = MIDIBUFFERS;
	trk->bit = 16;

#if defined(SOUND_MOREINFO)
	trk->bps = 0;
	trk->fmt = WAVEFMT_MIDI;
	midiout_getver(trk->info, sizeof(trk->info));
#endif
	return(SNDMIX_SUCCESS);

mfop_err3:
	_MFREE(mf);

mfop_err2:
	midiout_destroy(hdl);
	TRACEOUT(("mid: open error!"));

mfop_err1:
	return(SNDMIX_FAILURE);
}


// ----

int sndmidi_open(SMIXTRACK trk) {

	BYTE	*buf;
	int		r;

	if (sndmix_dataload(trk, 4) != 4) {
		TRACEOUT(("mid: failed read header."));
		r = SNDMIX_NOTSUPPORT;
		goto midopn_err;
	}
	buf = trk->data;
	if (memcmp(buf, "MThd", 4)) {
		r = SNDMIX_NOTSUPPORT;
		goto midopn_err;
	}

	if (module == NULL) {
		TRACEOUT(("mid: vermouth not ready."));
		r = SNDMIX_DEVICEERROR;
		goto midopn_err;
	}
	r = __midi_open(trk);
	if (r != SNDMIX_SUCCESS) {
		goto midopn_err;
	}

	trk->loopfpos = 0;
	trk->loopsize = (UINT)-1;
	trk->fremain = (UINT)-1;
	return(SNDMIX_SUCCESS);

midopn_err:
	return(r);
}

#endif

