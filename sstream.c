#include	"compiler.h"
#include	"arcfile.h"
#include	"sound.h"
#include	"sstream.h"
#include	"dosio.h"


#if defined(STREAM_BUFFERING)


#define	SBUFCNTBIT	2
#define	SBUFFERBIT	15

#define	SBUFCNT		(1 << SBUFCNTBIT)

#define	BUFFLAG_REQ	((1 << SBUFCNT) - 1)

enum {
	BUFFLAG_EOF		= 0x01,
	BUFFLAG_LOADING	= 0x02,
	BUFFLAG_SEEK	= 0x04
};

typedef struct {
	void		*hdl;

	PCMSTREAM_READ	sread;
	PCMSTREAM_SEEK	sseek;
	PCMSTREAM_CLOSE	sclose;

	UINT		readpos;
	UINT		loadpos;
	UINT		flag;
	UINT		avail;
	UINT		len[SBUFCNT];
	UINT		pos[SBUFCNT];
	long		fpos;
	long		seekpos;
	int			soundnum;
} STREAMBUF;

static	STREAMBUF	*shdl_reg[4] = {NULL, NULL, NULL, NULL};

static void stream_load(STREAMBUF *stream) {

	BOOL	stream_continue = FALSE;

	if (!stream) {
		return;
	}
	if (stream->flag & BUFFLAG_LOADING) {
		return;
	}
	stream->flag |= BUFFLAG_LOADING;

	if (stream->flag & BUFFLAG_SEEK) {
		stream->flag &= ~(BUFFLAG_SEEK | BUFFLAG_EOF);
		stream->sseek(stream->hdl, stream->seekpos, 0);
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
			rsize = stream->sread(stream->hdl,
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

static void regist_hdl(STREAMBUF *stream) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF *))); i++) {
		if (shdl_reg[i] == NULL) {
			shdl_reg[i] = stream;
			break;
		}
	}
}

static void unregist_hdl(STREAMBUF *stream) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF *))); i++) {
		if (shdl_reg[i] == stream) {
			shdl_reg[i] = NULL;
			break;
		}
	}
}


// ----

static void *arc_open(const void *arg, int num) {

	STREAMBUF	*ret;
	ARCFILEH	afh;

	if (arg == NULL) {
		return(NULL);
	}
	afh = arcfile_open(((const ARCSTREAMARG *)arg)->type,
							((const ARCSTREAMARG *)arg)->fname);
	if (afh == NULL) {
		return(NULL);
	}
	ret = (STREAMBUF *)_MALLOC(sizeof(STREAMBUF)
								+ (1 << (SBUFCNTBIT + SBUFFERBIT)),
															"sound stream");
	if (ret == NULL) {
		arcfile_close(afh);
		return(NULL);
	}
	ZeroMemory(ret, sizeof(STREAMBUF) + (1 << (SBUFCNTBIT + SBUFFERBIT)));
	ret->hdl = (void *)afh;
	ret->sread = (PCMSTREAM_READ)arcfile_read;
	ret->sseek = (PCMSTREAM_SEEK)arcfile_seek;
	ret->sclose = (PCMSTREAM_CLOSE)arcfile_close;
	ret->soundnum = num;

	stream_load(ret);
	regist_hdl(ret);
	return((void *)ret);
}


static void *mp3_open(const void *arg, int num) {

	STREAMBUF	*ret;
	FILEH		fh;

	if (arg == NULL) {
		return(NULL);
	}
	fh = file_open_rb((const char *)arg);
	if (fh == FILEH_INVALID) {
		return(NULL);
	}
	ret = (STREAMBUF *)_MALLOC(sizeof(STREAMBUF)
								+ (1 << (SBUFCNTBIT + SBUFFERBIT)),
															"sound stream");
	if (ret == NULL) {
		file_close(fh);
		return(NULL);
	}
	ZeroMemory(ret, sizeof(STREAMBUF) + (1 << (SBUFCNTBIT + SBUFFERBIT)));
	ret->hdl = (void *)fh;
	ret->flag = 0;
	ret->sread = (PCMSTREAM_READ)file_read;
	ret->sseek = (PCMSTREAM_SEEK)file_seek;
	ret->sclose = (PCMSTREAM_CLOSE)file_close;
	ret->soundnum = num;

	stream_load(ret);
	regist_hdl(ret);
	return((void *)ret);
}


static UINT arc_read(void *stream, void *buf, UINT size) {

	STREAMBUF	*s;
	BYTE		*b;
	UINT		pos;
	UINT		ret;

	s = (STREAMBUF *)stream;
	if (s == NULL) {
		return(0);
	}

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


static long arc_seek(void *stream, long pos, int method) {

	STREAMBUF	*s;
	UINT		bpos;
	long		chead;

	s = (STREAMBUF *)stream;
	if (s == NULL) {
		return(-1);
	}

	bpos = (s->readpos >> SBUFFERBIT) & (SBUFCNT - 1);
	chead = s->pos[bpos];

	switch(method) {
		case 1:
			pos += chead;
			pos += (s->readpos & ((1 << SBUFFERBIT) - 1));
			break;

		case 2:				// unsupported
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

static void arc_close(void *stream) {

	if (stream) {
		unregist_hdl((STREAMBUF *)stream);
		((STREAMBUF *)stream)->sclose(((STREAMBUF *)stream)->hdl);
		_MFREE(stream);
	}
}

void stream_prepart_task(void) {

	int		i;

	for (i=0; i<(int)((sizeof(shdl_reg)/sizeof(STREAMBUF *))); i++) {
		if (shdl_reg[i]) {
			stream_load(shdl_reg[i]);
		}
	}
}

PCMSTREAM	arc_stream = {arc_open, arc_read, arc_seek, arc_close};
PCMSTREAM	mp3_stream = {mp3_open, arc_read, arc_seek, arc_close};

#else

static void *arc_open(const void *arg, int num) {

	if (arg) {
		return(arcfile_open(((const ARCSTREAMARG *)arg)->type,
							((const ARCSTREAMARG *)arg)->fname));
	}
	return(NULL);
}

static UINT arc_read(void *stream, void *buf, UINT size) {

	return(arcfile_read((ARCFILEH)stream, buf, size));
}

static long arc_seek(void *stream, long pos, int method) {

	return(arcfile_seek((ARCFILEH)stream, pos, method));
}

static void arc_close(void *stream) {

	arcfile_close((ARCFILEH)stream);
}

static void *mp3_open(const void *arg) {

	if (arg) {
		return(file_open_rb((const char *)arg));
	}
	return((void *)FILEH_INVALID);
}

static UINT mp3_read(void *stream, void *buf, UINT size) {

	if ((FILEH)stream != FILEH_INVALID) {
		return(file_read((FILEH)stream, buf, size));
	}
	return(0);
}

static long mp3_seek(void *stream, long pos, int method) {

	if ((FILEH)stream != FILEH_INVALID) {
		return(file_seek((FILEH)stream, pos, method));
	}
	return(-1);
}

static void arc_close(void *stream) {

	if ((FILEH)stream != FILEH_INVALID) {
		file_close((FILEH)stream);
	}
}


PCMSTREAM	arc_stream = {arc_open, arc_read, arc_seek, arc_close};
PCMSTREAM	mp3_stream = {mp3_open, mp3_read, mp3_seek, mp3_close};

#endif


// ---- se

typedef struct {
	UINT	size;
	UINT	readpos;
} SESTREAM;

static void *se_open(const void *arg, int num) {

	SESTREAM	*ret;
	ARCFILEH	afh;

	if (arg == NULL) {
		return(NULL);
	}
	afh = arcfile_open(((const ARCSTREAMARG *)arg)->type,
							((const ARCSTREAMARG *)arg)->fname);
	if (afh == NULL) {
		return(NULL);
	}
	TRACEOUT(("S.E. buffer: %dbyte(s)", afh->size));
	ret = (SESTREAM *)_MALLOC(sizeof(SESTREAM) + afh->size, "se stream");
	if (ret) {
		ret->size = afh->size;
		ret->readpos = 0;
		arcfile_read(afh, ret+1, afh->size);
	}
	arcfile_close(afh);
	(void)num;
	return((void *)ret);
}


static UINT se_read(void *stream, void *buf, UINT size) {

	SESTREAM	*s;
	UINT		ret;

	s = (SESTREAM *)stream;
	if (s == NULL) {
		return(0);
	}

	ret = min(s->size - s->readpos, size);
	if (ret) {
		CopyMemory(buf, (BYTE *)(s + 1) + s->readpos, ret);
		s->readpos += ret;
	}
	return(ret);
}


static long se_seek(void *stream, long pos, int method) {

	SESTREAM	*s;

	s = (SESTREAM *)stream;
	if (s == NULL) {
		return(-1);
	}

	switch(method) {
		case 1:
			pos += s->readpos;
			break;

		case 2:
			pos += s->size;
			break;
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

static void se_close(void *stream) {

	if (stream) {
		_MFREE(stream);
	}
}

PCMSTREAM	se_stream = {se_open, se_read, se_seek, se_close};


// ----

typedef struct {
const	BYTE	*head;
		UINT	size;
		UINT	readpos;
} ONMEMSTREAM;

static void *onmem_open(const void *arg, int num) {

	ONMEMSTREAM	*ret;

	if (arg == NULL) {
		return(NULL);
	}
	ret = (ONMEMSTREAM *)_MALLOC(sizeof(ONMEMSTREAM), "on memory stream");
	if (ret) {
		ret->head = ((ONMEMSTMARG *)arg)->ptr;
		ret->size = ((ONMEMSTMARG *)arg)->size;
		ret->readpos = 0;
	}
	(void)num;
	return((void *)ret);
}


static UINT onmem_read(void *stream, void *buf, UINT size) {

	ONMEMSTREAM	*s;
	UINT		ret;

	s = (ONMEMSTREAM *)stream;
	if (s == NULL) {
		return(0);
	}
	ret = min(s->size - s->readpos, size);
	if (ret) {
		CopyMemory(buf, s->head + s->readpos, ret);
		s->readpos += ret;
	}
	return(ret);
}


static long onmem_seek(void *stream, long pos, int method) {

	ONMEMSTREAM	*s;

	s = (ONMEMSTREAM *)stream;
	if (s == NULL) {
		return(-1);
	}

	switch(method) {
		case 1:
			pos += s->readpos;
			break;

		case 2:
			pos += s->size;
			break;
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

static void onmem_close(void *stream) {

	if (stream) {
		_MFREE(stream);
	}
}

PCMSTREAM onmem_stream = {onmem_open, onmem_read, onmem_seek, onmem_close};

