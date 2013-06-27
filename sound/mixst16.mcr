
#if (MIX_INPUTBIT == 8)
#define	_SMP_IN		unsigned char
#define	_SAMP(s)	(((SINT32)(s) - 0x80) << 8)
#elif (MIX_INPUTBIT == 16)
#define	_SMP_IN		SINT16
#define	_SAMP(s)	(SINT32)(s)
#endif


#if (MIX_OUTPUTBIT == 16)
#define	_SMP_OUT		SINT16
#define	_SAMPMULVOL(s)	(((s) * trk->vol) >> SND_VOLSHIFT)
#define	_SAMPLIMIT(s)	if ((s) > 32767) {				\
							(s) = 32767;				\
						}								\
						else if ((s) < -32768) {		\
							(s) = -32768;				\
						}
#elif (MIX_OUTPUTBIT == 32)
#define	_SMP_OUT		SINT32
#define	_SAMPMULVOL(s)	((s) * trk->vol)
#define	_SAMPLIMIT(s)
#endif


#ifdef SAMPADD
#define	_OUTSAMP(d, s)	(d) += (_SMP_OUT)(s)
#else
#define	_OUTSAMP(d, s)	(d) = (_SMP_OUT)(s)
#endif


#ifdef NOMATTER_VOL
#undef _SAMPMULVOL
#if (MIX_OUTPUTBIT == 16)
#define	_SAMPMULVOL(s)	(s)
#elif (MIX_OUTPUTBIT == 32)
#define	_SAMPMULVOL(s)	((s) << SND_VOLSHIFT)
#endif
#endif


static _SMP_OUT *FUNC_NOR(SMIXTRACK trk, _SMP_OUT *pcm, _SMP_OUT *pcmterm) {

	_SMP_IN	*samp;
	UINT	size;

	size = min(trk->remain, (UINT)((pcmterm - pcm) / 2));
	trk->remain -= size;
	samp = (_SMP_IN *)trk->buf;
	do {
		SINT32 out;
		out = _SAMPMULVOL(_SAMP(*samp++));
		_SAMPLIMIT(out);
		_OUTSAMP(pcm[0], out);
#if (MIX_CHANNELS == 2)
		out = _SAMPMULVOL(_SAMP(*samp++));
		_SAMPLIMIT(out);
#endif
		_OUTSAMP(pcm[1], out);
		pcm += 2;
	} while(--size);
	trk->buf = samp;
	return(pcm);
}

static _SMP_OUT *FUNC_DOWN(SMIXTRACK trk, _SMP_OUT *pcm, _SMP_OUT *pcmterm) {

	SINT32	mrate;
	_SMP_IN	*samp;

	samp = (_SMP_IN *)trk->buf;
	mrate = trk->mrate;
	do {
		if (trk->rem > mrate) {
			trk->rem -= mrate;
			trk->pcml += _SAMP(*samp++) * mrate;
#if (MIX_CHANNELS == 2)
			trk->pcmr += _SAMP(*samp++) * mrate;
#endif
		}
		else {
			SINT32 out;
			SINT32 tmp;
			out = trk->pcml;
			out += _SAMP(samp[0]) * trk->rem;
			out >>= DNBASEBITS;
			out = _SAMPMULVOL(out);
			_SAMPLIMIT(out);
			_OUTSAMP(pcm[0], out);
#if (MIX_CHANNELS == 2)
			out = trk->pcmr;
			out += _SAMP(samp[1]) * trk->rem;
			out >>= DNBASEBITS;
			out = _SAMPMULVOL(out);
			_SAMPLIMIT(out);
#endif
			_OUTSAMP(pcm[1], out);
			tmp = mrate - trk->rem;
			trk->pcml = _SAMP(*samp++) * tmp;
#if (MIX_CHANNELS == 2)
			trk->pcmr = _SAMP(*samp++) * tmp;
#endif
			trk->rem = DNMIXBASE - tmp;
			pcm += 2;
			if (pcm >= pcmterm) {
				trk->remain--;
				break;
			}
		}
	} while(--trk->remain);
	trk->buf = samp;
	return(pcm);
}

static _SMP_OUT *FUNC_UP(SMIXTRACK trk, _SMP_OUT *pcm, _SMP_OUT *pcmterm) {

	_SMP_IN	*samp;
	SINT32	mrate;

	samp = (_SMP_IN *)trk->buf;
	mrate = trk->mrate;
	do {
		SINT32 tmp;
		tmp = UPMIXBASE - trk->rem;
		if (tmp >= 0) {
			SINT32 dat;
			SINT32 next;
			dat = (trk->pcml * trk->rem);
			next = _SAMP(*samp++);
			dat += (next * tmp);
			dat >>= UPBASEBITS;
			dat = _SAMPMULVOL(dat);
			_SAMPLIMIT(dat);
			trk->pcml = next;
			_OUTSAMP(pcm[0], dat);
#if (MIX_CHANNELS == 2)
			dat = (trk->pcmr * trk->rem);
			next = _SAMP(*samp++);
			dat += (next * tmp);
			dat >>= UPBASEBITS;
			dat = _SAMPMULVOL(dat);
			_SAMPLIMIT(dat);
			trk->pcmr = next;
#endif
			_OUTSAMP(pcm[1], dat);
			trk->remain--;
			trk->rem = trk->mrate - tmp;
			pcm += 2;
			if (pcm >= pcmterm) {
				goto upsampterm;
			}
		}
		while(trk->rem >= UPMIXBASE) {
			SINT32 out;
			trk->rem -= UPMIXBASE;
			out = trk->pcml;
			out = _SAMPMULVOL(out);
			_SAMPLIMIT(out);
			_OUTSAMP(pcm[0], out);
#if (MIX_CHANNELS == 2)
			out = trk->pcmr;
			out = _SAMPMULVOL(out);
			_SAMPLIMIT(out);
#endif
			_OUTSAMP(pcm[1], out);
			pcm += 2;
			if (pcm >= pcmterm) {
				goto upsampterm;
			}
		}
	} while(trk->remain);
upsampterm:
	trk->buf = samp;
	return(pcm);
}

#undef _SAMP
#undef _SMP_IN
#undef _SMP_OUT
#undef _SAMPMULVOL
#undef _SAMPLIMIT
#undef _OUTSAMP

#undef MIX_INPUTBIT
#undef MIX_OUTPUTBIT
#undef MIX_CHANNELS
#undef FUNC_NOR
#undef FUNC_DOWN
#undef FUNC_UP
#undef SAMPADD
#undef NOMATTER_VOL

