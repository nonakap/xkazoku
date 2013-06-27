#include	"compiler.h"
#include	"dosio.h"
#include	"sound.h"
#include	"arcfile.h"
#include	"sstream.h"


// ---- sound stream prepare

#if defined(STREAM_BUFFERING)

enum {
	SBUFCNTBIT		= 2,
	SBUFFERBIT		= 15,
	SBUFCNT			= (1 << SBUFCNTBIT),
	BUFFLAG_REQ		= ((1 << SBUFCNT) - 1),

	BUFFLAG_EOF		= 0x01,
	BUFFLAG_LOADING	= 0x02,
	BUFFLAG_SEEK	= 0x04
};

typedef struct {
	_SNDSTREAM	ss;
	UINT		readpos;
	UINT		loadpos;
	UINT		flag;
	UINT		avail;
	UINT		len[SBUFCNT];
	UINT		pos[SBUFCNT];
	long		fpos;
	long		seekpos;
	int			soundnum;
} _STREAMBUF, *STREAMBUF;

static	STREAMBUF	shdl_reg[4] = {NULL, NULL, NULL, NULL};

static void streambuf_load(STREAMBUF stream) {

	BOOL	stream_continue;

	stream_continue = FALSE;
	if (stream->flag & BUFFLAG_LOADING) {
		return;
	}
	stream->flag |= BUFFLAG_LOADING;

	if (stream->flag & BUFFLAG_SEEK) {
		stream->flag &= ~(BUFFLAG_SEEK | BUFFLAG_EOF);
		stream->fpos = stream->ss.ssseek(&stream->ss,
												stream->seekpos, SSSEEK_SET);
		stream->avail = 0;
		stream->loadpos = 0;
		stream->readpos = 0;
		stream_continue = TRUE;
	}
	if (!(stream->flag & BUFFLAG_EOF)) {
		while((stream->avail & BUFFLAG_REQ) != BUFFLAG_REQ) {
			UINT rsize;
			UINT bit = (1 << stream->loadpos);
			if (stream->avail & bit) {
				break;
			}
			rsize = stream->ss.ssread(&stream->ss,
							(BYTE *)(stream + 1) +
									(stream->loadpos << SBUFFERBIT),
									(1 << SBUFFERBIT));
			if (rsize != (1 << SBUFFERBIT)) {
				stream->flag |= BUFFLAG_EOF;
				if (!rsize) {
					break;
				}
			}
			stream->pos[stream->loadpos] = stream->fpos;
			stream->len[stream->loadpos] = rsize;
			stream->fpos += rsize;
			stream->loadpos++;
			stream->loadpos &= (SBUFCNT - 1);
			stream->avail |= bit;
			if (stream_continue) {
				stream_continue = FALSE;
				soundmix_continue(stream->soundnum);
			}
		}
	}
	stream->flag &= ~BUFFLAG_LOADING;
}

static BOOL streambuf_regist(STREAMBUF stream) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF))); i++) {
		if (shdl_reg[i] == NULL) {
			shdl_reg[i] = stream;
			return(SUCCESS);
		}
	}
	return(FAILURE);
}

static void streambuf_unregist(STREAMBUF stream) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF))); i++) {
		if (shdl_reg[i] == stream) {
			shdl_reg[i] = NULL;
			break;
		}
	}
}

static UINT streambuf_ssread(SNDSTREAM stream, void *buf, UINT size) {

	STREAMBUF	s;
	BYTE		*b;
	UINT		pos;
	UINT		ret;

	s = (STREAMBUF)stream->hdl;
	b = (BYTE *)buf;
	pos = (s->readpos >> SBUFFERBIT) & (SBUFCNT - 1);
	ret = 0;

	if (s->avail & (1 << pos)) {
		UINT remain;
		remain = s->len[pos] - (s->readpos & ((1 << SBUFFERBIT) - 1));
		ret = min(remain, size);
		if (ret) {
			CopyMemory(b, (BYTE *)(s + 1) + s->readpos, ret);
			if (ret == remain) {
				s->avail ^= (1 << pos);
			}
			size -= ret;
			s->readpos += ret;
			s->readpos &= ((1 << (SBUFCNTBIT + SBUFFERBIT)) - 1);

			if (size) {
				UINT rsize;
				pos++;
				pos &= (SBUFCNT - 1);
				if (s->avail & (1 << pos)) {
					rsize = min(s->len[pos], size);
					if (rsize) {
						CopyMemory(b + ret,
									(BYTE *)(s + 1) + (pos << SBUFFERBIT),
									rsize);
					}
					ret += rsize;
					if (s->len[pos] == rsize) {
						s->avail ^= (1 << pos);
					}
					s->readpos += rsize;
					s->readpos &= ((1 << (SBUFCNTBIT + SBUFFERBIT)) - 1);
				}
			}
		}
	}
	return(ret);
}

static long streambuf_ssseek(SNDSTREAM stream, long pos, int method) {

	STREAMBUF	s;
	UINT		bpos;
	long		chead;

	s = (STREAMBUF)stream->hdl;
	bpos = (s->readpos >> SBUFFERBIT) & (SBUFCNT - 1);
	chead = s->pos[bpos];

	switch(method) {
		case SSSEEK_SET:
			break;

		case SSSEEK_CUR:
			pos += chead;
			pos += (s->readpos & ((1 << SBUFFERBIT) - 1));
			break;

		case SSSEEK_END:			// unsupported
		default:
			return(-1);
	}
	if ((!(s->avail & (1 << bpos))) ||
		(pos < chead) || (pos >= (int)(chead + s->len[bpos]))) {
		s->flag |= BUFFLAG_SEEK;
		s->seekpos = pos;
		return(-1);
	}
	s->readpos = (pos - chead) + (bpos << SBUFFERBIT);
	return(pos);
}

static void streambuf_ssclose(SNDSTREAM stream) {

	STREAMBUF	s;

	s = (STREAMBUF)stream->hdl;
	streambuf_unregist(s);
	if (s->ss.ssclose != NULL) {
		s->ss.ssclose(&s->ss);
	}
	_MFREE(s);
	stream->hdl = NULL;
	stream->ssread = NULL;
	stream->ssseek = NULL;
	stream->ssclose = NULL;
}

int streambuf_attach(SNDSTREAM stream, int num) {

	STREAMBUF	s;

	if ((stream->ssseek == NULL) || (stream->ssread == NULL)) {
		goto sba_err1;
	}
	s = (STREAMBUF)_MALLOC(sizeof(_STREAMBUF)
										+ (1 << (SBUFCNTBIT + SBUFFERBIT)),
															"sound stream");
	if (s == NULL) {
		goto sba_err1;
	}
	ZeroMemory(s, sizeof(_STREAMBUF) + (1 << (SBUFCNTBIT + SBUFFERBIT)));
	s->ss = *stream;
	s->soundnum = num;
	s->fpos = s->ss.ssseek(&s->ss, 0, SSSEEK_CUR);
	if (streambuf_regist(s) != SUCCESS) {
		goto sba_err2;
	}
	stream->hdl = (void *)s;
	stream->ssread = streambuf_ssread;
	stream->ssseek = streambuf_ssseek;
	stream->ssclose = streambuf_ssclose;
	streambuf_load(s);
	return(SNDMIX_SUCCESS);

sba_err2:
	_MFREE(s);

sba_err1:
	return(SNDMIX_FAILURE);
}

void stream_prepart_task(void) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF))); i++) {
		if (shdl_reg[i]) {
			streambuf_load(shdl_reg[i]);
		}
	}
}
#endif


// ---- file stream

static UINT mp3_ssread(SNDSTREAM stream, void *buf, UINT size) {

	return(file_read((FILEH)stream->hdl, buf, size));
}

static long mp3_ssseek(SNDSTREAM stream, long pos, int method) {

	int		fmethod;

	switch(method) {
		case SSSEEK_SET:
			fmethod = FSEEK_SET;
			break;

		case SSSEEK_CUR:
			fmethod = FSEEK_CUR;
			break;

		case SSSEEK_END:
			fmethod = FSEEK_END;
			break;

		default:
			return(-1);
	}
	return(file_seek((FILEH)stream->hdl, pos, fmethod));
}

static void mp3_ssclose(SNDSTREAM stream) {

	file_close((FILEH)stream->hdl);
	stream->hdl = NULL;
	stream->ssread = NULL;
	stream->ssseek = NULL;
	stream->ssclose = NULL;
}

int mp3_ssopen(SNDSTREAM stream, void *arg, int num) {

	FILEH	fh;

	if (arg == NULL) {
		goto m3o_err1;
	}
	fh = file_open_rb((char *)arg);
	if (fh == FILEH_INVALID) {
		goto m3o_err1;
	}
	stream->hdl = (void *)fh;
	stream->ssread = mp3_ssread;
	stream->ssseek = mp3_ssseek;
	stream->ssclose = mp3_ssclose;
#if defined(STREAM_BUFFERING)
	return(streambuf_attach(stream, num));
#else
	return(SNDMIX_SUCCESS);
#endif

m3o_err1:
	(void)num;
	return(SNDMIX_FAILURE);
}


// ---- arc

static UINT arc_ssread(SNDSTREAM stream, void *buf, UINT size) {

	return(arcfile_read((ARCFILEH)stream->hdl, buf, size));
}

static long arc_ssseek(SNDSTREAM stream, long pos, int method) {

	int		fmethod;

	switch(method) {
		case SSSEEK_SET:
			fmethod = 0;
			break;

		case SSSEEK_CUR:
			fmethod = 1;
			break;

		case SSSEEK_END:
			fmethod = 2;
			break;

		default:
			return(-1);
	}
	return(arcfile_seek((ARCFILEH)stream->hdl, pos, fmethod));
}

static void arc_ssclose(SNDSTREAM stream) {

	arcfile_close((ARCFILEH)stream->hdl);
	stream->hdl = NULL;
	stream->ssread = NULL;
	stream->ssseek = NULL;
	stream->ssclose = NULL;
}

int arcraw_ssopen(SNDSTREAM stream, void *arg, int num) {

	ARCFILEH	afh;

	if (arg == NULL) {
		goto ao_err1;
	}
	afh = arcfile_open(((ARCSTREAMARG *)arg)->type,
							((ARCSTREAMARG *)arg)->fname);
	if (afh == NULL) {
		goto ao_err1;
	}
	stream->hdl = (void *)afh;
	stream->ssread = arc_ssread;
	stream->ssseek = arc_ssseek;
	stream->ssclose = arc_ssclose;
	return(SNDMIX_SUCCESS);

ao_err1:
	(void)num;
	return(SNDMIX_FAILURE);
}

int arc_ssopen(SNDSTREAM stream, void *arg, int num) {

	int		r;

	r = arcraw_ssopen(stream, arg, num);
#if defined(STREAM_BUFFERING)
	if (r == SNDMIX_SUCCESS) {
		r = streambuf_attach(stream, num);
	}
#endif
	return(r);
}


// ---- on memory stream

typedef struct {
const BYTE	*ptr;
	UINT	size;
	UINT	readpos;
} _MEMSTREAM, *MEMSTREAM;

static UINT onmem_ssread(SNDSTREAM stream, void *buf, UINT size) {

	MEMSTREAM	s;
	UINT		ret;

	s = (MEMSTREAM)stream->hdl;
	ret = min(s->size - s->readpos, size);
	if (ret) {
		CopyMemory(buf, s->ptr + s->readpos, ret);
		s->readpos += ret;
	}
	return(ret);
}

static long onmem_ssseek(SNDSTREAM stream, long pos, int method) {

	MEMSTREAM	s;

	s = (MEMSTREAM)stream->hdl;
	switch(method) {
		case SSSEEK_SET:
			break;

		case SSSEEK_CUR:
			pos += s->readpos;
			break;

		case SSSEEK_END:
			pos += s->size;
			break;

		default:
			return(-1);
	}
	if (pos < 0) {
		pos = 0;
	}
	else if (pos > (long)s->size) {
		pos = s->size;
	}
	s->readpos = pos;
	return(pos);
}

static void onmem_ssclose(SNDSTREAM stream) {

	_MFREE(stream->hdl);
	stream->hdl = NULL;
	stream->ssread = NULL;
	stream->ssseek = NULL;
	stream->ssclose = NULL;
}

int onmem_ssopen(SNDSTREAM stream, void *arg, int num) {

	MEMSTREAM	s;

	if (arg == NULL) {
		goto oms_err1;
	}
	s = (MEMSTREAM)_MALLOC(sizeof(_MEMSTREAM), "on memory stream");
	if (s == NULL) {
		goto oms_err1;
	}
	s->ptr = ((ONMEMSTMARG *)arg)->ptr;
	s->size = ((ONMEMSTMARG *)arg)->size;
	s->readpos = 0;
	stream->hdl = (void *)s;
	stream->ssread = onmem_ssread;
	stream->ssseek = onmem_ssseek;
	stream->ssclose = onmem_ssclose;
	return(SNDMIX_SUCCESS);

oms_err1:
	(void)num;
	return(SNDMIX_FAILURE);
}

int arcse_ssopen(SNDSTREAM stream, void *arg, int num) {

	ARCFILEH	afh;
	MEMSTREAM	s;

	if (arg == NULL) {
		goto aso_err1;
	}
	afh = arcfile_open(((ARCSTREAMARG *)arg)->type,
							((ARCSTREAMARG *)arg)->fname);
	if (afh == NULL) {
		goto aso_err1;
	}
	TRACEOUT(("S.E. buffer: %dbyte(s)", afh->size));
	s = (MEMSTREAM)_MALLOC(sizeof(_MEMSTREAM) + afh->size, "se stream");
	if (s == NULL) {
		goto aso_err2;
	}
	s->ptr = (BYTE *)(s + 1);
	s->size = afh->size;
	s->readpos = 0;
	arcfile_read(afh, s + 1, afh->size);
	arcfile_close(afh);
	stream->hdl = (void *)s;
	stream->ssread = onmem_ssread;
	stream->ssseek = onmem_ssseek;
	stream->ssclose = onmem_ssclose;
	return(SNDMIX_SUCCESS);

aso_err2:
	arcfile_close(afh);

aso_err1:
	(void)num;
	return(SNDMIX_FAILURE);
}

