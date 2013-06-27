#include	"compiler.h"
#include	"sound.h"


#if defined(OGGVORBIS_LIB)
#include	<math.h>
#include	"vorbis/codec.h"


typedef struct {
	int					phase;
	ogg_sync_state		oy;
	ogg_stream_state	os;
	ogg_page			og;
	ogg_packet			op;

	vorbis_info			vi;
	vorbis_comment		vc;
	vorbis_dsp_state	vd;
	vorbis_block		vb;
} __OV;


// ----

enum {
	OVPHASE_HEAD		= 0,
	OVPHASE_STREAMIN,
	OVPHASE_GETPCM,
	OVPHASE_NEXT,
	OVPHASE_CLOSE
};


static UINT ogg_dec(SMIXTRACK trk, SINT16 *dst) {

	__OV	*ov;
	int		result;
	char	*buffer;
	UINT	r;
	float	**pcm;
	int		samples;
	int		i;
	int		j;
	float	*mono;
	SINT16	*ptr;
	SINT32	val;

	ov = (__OV *)trk->snd;

	do {
		switch(ov->phase) {
			case OVPHASE_HEAD:
				result = ogg_sync_pageout(&ov->oy, &ov->og);
				if (result > 0) {
					ogg_stream_pagein(&ov->os, &ov->og);
					ov->phase = OVPHASE_STREAMIN;
				}
				else if (result == 0) {
					ov->phase = OVPHASE_NEXT;
				}
				else {
					TRACEOUT(("Corrupt or missing data in bitstream"));
				}
				break;

			case OVPHASE_STREAMIN:
				result = ogg_stream_packetout(&ov->os, &ov->op);
				if (result > 0) {
					if (vorbis_synthesis(&ov->vb, &ov->op) == 0) {
						vorbis_synthesis_blockin(&ov->vd, &ov->vb);
					}
					ov->phase = OVPHASE_GETPCM;
				}
				else if (result == 0) {
					if (!ogg_page_eos(&ov->og)) {
						ov->phase = OVPHASE_NEXT;
					}
					else {
						ov->phase = OVPHASE_CLOSE;
					}
				}
				break;

			case OVPHASE_GETPCM:
				samples = vorbis_synthesis_pcmout(&ov->vd, &pcm);
				if (samples > 0) {
					if (samples > (int)trk->blksamp) {
						samples = (int)trk->blksamp;
					}
					for (i=0; i<ov->vi.channels; i++) {
						ptr = dst + i;
						mono = pcm[i];
						for (j=0; j<samples; j++) {
							val = (long)(mono[j] * 32767.f);
							if (val > 32767) {
								val = 32767;
							}
							if (val < -32768) {
								val = -32768;
							}
							*ptr = (SINT16)val;
							ptr += ov->vi.channels;
						}
					}
					vorbis_synthesis_read(&ov->vd, samples);
					return((UINT)samples);
				}
				ov->phase = OVPHASE_STREAMIN;
				break;

			case OVPHASE_NEXT:
				buffer = ogg_sync_buffer(&ov->oy, trk->block);
				r = sndmix_dataread(trk, buffer, trk->block);
				ogg_sync_wrote(&ov->oy, (int)r);
				if (r) {
					ov->phase = OVPHASE_HEAD;
				}
				else {
					ov->phase = OVPHASE_CLOSE;
				}
				break;

			case OVPHASE_CLOSE:
				ov->phase = OVPHASE_HEAD;			// ¤ë¡¼¤×
				return(0);
		}
	} while(1);
}


static void ogg_decend(SMIXTRACK trk) {

	__OV	*ov;

	ov = (__OV *)trk->snd;
	if (ov) {
		ogg_stream_clear(&ov->os);
		vorbis_block_clear(&ov->vb);
		vorbis_dsp_clear(&ov->vd);
		vorbis_comment_clear(&ov->vc);
		vorbis_info_clear(&ov->vi);
		ogg_sync_clear(&ov->oy);
		_MFREE(ov);
	}
}


int sndogg_open(SMIXTRACK trk) {

	__OV	*ov;
	char	*buffer;
	UINT	r;
	int		bytes;
	int 	i;
	int		result;

	ov = (__OV *)_MALLOC(sizeof(__OV), "__OV");
	if (ov == NULL) {
		goto ovopn_next;
	}
	ZeroMemory(ov, sizeof(__OV));

	r = sndmix_dataload(trk, trk->maxdatas);
	if ((r == (UINT)-1) || (r == 0)) {
		goto ovopn_next2;
	}
	buffer = ogg_sync_buffer(&ov->oy, trk->maxdatas);
	CopyMemory(buffer, trk->data, r);
	ogg_sync_wrote(&ov->oy, (int)r);

	if (ogg_sync_pageout(&ov->oy, &ov->og) != 1) {
		TRACEOUT(("Input does not appear to be an Ogg bitstream."));
		goto ovopn_next2;
	}
	ogg_stream_init(&ov->os, ogg_page_serialno(&ov->og));
	sndmix_datatrash(trk, (UINT)-1);

	vorbis_info_init(&ov->vi);
	vorbis_comment_init(&ov->vc);
	if (ogg_stream_pagein(&ov->os, &ov->og) < 0) {
		TRACEOUT(("Error reading first page of Ogg bitstream data."));
		goto ovopn_err1;
	}

	if (ogg_stream_packetout(&ov->os, &ov->op) != 1) {
		TRACEOUT(("Error reading initial header packet."));
		goto ovopn_err1;
	}

	if (vorbis_synthesis_headerin(&ov->vi, &ov->vc, &ov->op) < 0) {
		TRACEOUT(("This Ogg bitstream does not contain Vorbis audio data."));
		goto ovopn_err1;
	}

	i = 0;
	while(i < 2) {
		while(i < 2) {
			result = ogg_sync_pageout(&ov->oy, &ov->og);
			if (result == 0) {
				break;
			}
			if (result == 1) {
				ogg_stream_pagein(&ov->os, &ov->og);
				while(i < 2) {
					result = ogg_stream_packetout(&ov->os, &ov->op);
					if (result == 0) {
						break;
					}
					if (result < 0) {
						TRACEOUT(("Corrupt secondary header. Exiting."));
						goto ovopn_err1;
					}
					vorbis_synthesis_headerin(&ov->vi, &ov->vc, &ov->op);
					i++;
				}
			}
		}
		buffer = ogg_sync_buffer(&ov->oy, 4096);
		bytes = sndmix_dataread(trk, buffer, 4096);
		if ((bytes == 0) && (i < 2)) {
			TRACEOUT(("End of file before finding all Vorbis headers!"));
			return(SNDMIX_FAILURE);
		}
		ogg_sync_wrote(&ov->oy, bytes);
	}

	trk->snd = ov;
	trk->dec = (DECFN)ogg_dec;
	trk->decend = ogg_decend;
	trk->samprate = ov->vi.rate;
	trk->channels = ov->vi.channels;
	trk->block = 4096;
	trk->blksamp = 4096 / ov->vi.channels;
	trk->bit = 16;
#if defined(SOUND_MOREINFO)
	trk->bps = 0;
	trk->fmt = WAVEFMT_OGG;
	CopyMemory(trk->info, "Ogg vorbis", 11);
#endif

	vorbis_synthesis_init(&ov->vd, &ov->vi);
	vorbis_block_init(&ov->vd, &ov->vb);
	return(SNDMIX_SUCCESS);

ovopn_err1:
	ogg_sync_clear(&ov->oy);
	_MFREE(ov);
	return(SNDMIX_FAILURE);

ovopn_next2:
	_MFREE(ov);

ovopn_next:
	return(SNDMIX_NEXT);
}

#endif

