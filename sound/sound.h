
// ケモ様のサウンドミキサーっぽい奴


#define	SND_MAXTRACK	34
#define	SND_MAXSAMPLE	0x800
#define	SND_VOLSHIFT	8


// ---- supports

// #define	SOUND_MOREINFO				// chimeが情報取りにくるので…

#define	WAVE_MSADPCM
#define	WAVE_IMA
// #define	KSYS_KPV0

#define	SUPPORT_VBR		(1728 + 16)


// ---- stream

typedef void *(*PCMSTREAM_OPEN)(const void *arg, int num);
typedef UINT (*PCMSTREAM_READ)(void *stream, void *buf, UINT size);
typedef long (*PCMSTREAM_SEEK)(void *stream, long pos, int method);
typedef void (*PCMSTREAM_CLOSE)(void *stream);

typedef struct {
	PCMSTREAM_OPEN		stream_open;
	PCMSTREAM_READ		stream_read;
	PCMSTREAM_SEEK		stream_seek;
	PCMSTREAM_CLOSE		stream_close;
} PCMSTREAM;


// ---- wave file formats

enum {
	WAVEFMT_PCM			= 0x01,
	WAVEFMT_MSADPCM		= 0x02,
	WAVEFMT_IMA			= 0x11,
	WAVEFMT_MP2			= 0x52,
	WAVEFMT_MP3			= 0x55,
	WAVEFMT_OGG			= 0x4f4747,
	WAVEFMT_KPV			= 0x4b5056
};


// ----

enum {
	SNDTRK_LOADED		= 0x01,
	SNDTRK_PLAYING		= 0x02,
	SNDTRK_PAUSED		= 0x04,
	SNDTRK_STREAMEND	= 0x08,
	SNDTRK_FADE			= 0x10
};


struct _smixtrack;
typedef	struct _smixtrack	_SMIXTRACK;
typedef	struct _smixtrack	*SMIXTRACK;

typedef UINT (*DECFN)(SMIXTRACK self, void *buf);
typedef void (*DECENDFN)(SMIXTRACK self);
typedef void (*MIXFN)(SMIXTRACK self, void *buf, UINT size);
typedef void *(*MIXFN2)(SMIXTRACK self, void *buf, void *bufterm);
typedef void (*DECGAINFN)(SMIXTRACK self, int gain);


struct _smixtrack {
volatile UINT	flag;

	BYTE		*data;					// data load用バッファ
	UINT		indatas;
	UINT		maxdatas;

	void		*buf;
	UINT		remain;
	SINT32		mrate;
	SINT32		rem;
	SINT32		pcml;
	SINT32		pcmr;

	void			*stream;
	PCMSTREAM_READ	stream_read;
	PCMSTREAM_SEEK	stream_seek;
	PCMSTREAM_CLOSE	stream_close;

	void		*snd;
	DECFN		dec;
	DECENDFN	decend;
	DECGAINFN	decgain;

	MIXFN		cpy16;
	MIXFN2		cpy16cnv;
	MIXFN		cpy32;
	MIXFN2		cpy32cnv;
	MIXFN		add32;
	MIXFN2		add32cnv;

	UINT		samprate;
	UINT		channels;
	UINT		bit;
	UINT		block;
	UINT		blksamp;

	int			loop;
	long		loopfpos;
	UINT		loopsize;
	UINT		fremain;

	int			worksize;
	BYTE		*buffer;

	int			basevol;
	int			vol;
	int			dir;
	UINT		fadestep;
	UINT		fadesamp;

#ifdef SOUND_MOREINFO
	UINT		bps;
	UINT		fmt;
	char		info[64];
#endif
};

typedef struct {
	_SMIXTRACK	trk[SND_MAXTRACK];
	UINT		basehz;
	int			gain[SND_MAXTRACK];
} SOUND_T;


enum {
	SNDMIX_NEXT = -1,
	SNDMIX_SUCCESS = 0,
	SNDMIX_FAILURE = 1
};


#ifdef __cplusplus
extern "C" {
#endif

int sndwave_open(SMIXTRACK trk);
int sndmp3_open(SMIXTRACK trk);
int sndexse_open(SMIXTRACK trk);
int sndogg_open(SMIXTRACK trk);
int sndkpv_open(SMIXTRACK trk);

BOOL sndmix_setmixproc(SMIXTRACK trk, UINT basehz);

UINT sndmix_dataload(SMIXTRACK trk, UINT size);
UINT sndmix_dataread(SMIXTRACK trk, void *buf, UINT size);
void sndmix_datatrash(SMIXTRACK trk, UINT size);


void soundmix_create(UINT basehz);
void soundmix_destory(void);
void soundmix_setgain(int num, int gain);

BOOL soundmix_load(int num, PCMSTREAM *stream, void *arg);
void soundmix_unload(int num);
void soundmix_play(int num, int loop, int fadeintick);
void soundmix_stop(int num, int fadeouttick);
void soundmix_rew(int num);
void soundmix_continue(int num);

BOOL soundmix_isplaying(int num);
#ifdef SOUND_MOREINFO
SMIXTRACK soundmix_getinfo(int num);
#endif

UINT soundmix_getpcm(SINT16 *pcm, UINT samples);


#if defined(AMETHYST_OVL)
void amethsy_init(void);
void amethsy_term(void);
#endif

#ifdef __cplusplus
}
#endif

