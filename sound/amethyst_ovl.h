
enum {
	MPEG_SUCCESS		= 0,
	MPEG_FAILURE		= 1
};

enum {								// 可変長対応？
	MPEGHEAD_SUCCESS	= 0,
	MPEGHEAD_RENEWAL	= 1,
	MPEGHEAD_FAILURE	= 2
};

enum {
	SMODE_ISPROC	= 0x01,
	SMODE_MSPROC	= 0x02,

	SMODE_MASK		= 0x0c,
	SMODE_STEREO	= 0x00,
	SMODE_JOINT		= 0x04,
	SMODE_DUAL		= 0x08,
	SMODE_MONORAL	= 0x0c
};


typedef struct {
		UINT	lbandindex[22];
		UINT	sbandindex[13];
		BYTE	lbands[24];
		BYTE	sbands[16];
} BAND_TBL;

typedef struct {
		BYTE		id;
		BYTE		__mode;
		BYTE		head1;
		BYTE		head2;
		UINT		protection;
		UINT		outmode;

		UINT		channels;
		UINT		sidesize;

const	BAND_TBL	*bandtbl;
		UINT		bandlimit;
		UINT		lbandlimit;
		UINT		sbandlimit;
		UINT		limit;
		UINT		mixbands;

		UINT		framesize;
		UINT		insize;
		UINT		outsamples;
		UINT		outsize;

		UINT		kbitrate;
		UINT		samplingrate;
} HEAD_INF;

typedef struct {
		UINT		crc;
		UINT		datastart;
		UINT		scfsi[2];
} SIDE_INF;


typedef struct {
		HEAD_INF	c;
		SIDE_INF	s;
		UINT		frame;
} MPEGL3;


#ifdef __cplusplus
extern "C" {
#endif

typedef UINT (WINAPI *MP3_GETDECVER)(char *string, int leng);
typedef MPEGL3 * (WINAPI *MP3_CREATE)(BYTE *head);
typedef void (WINAPI *MP3_DESTROY)(MPEGL3 *mpegl3);
typedef UINT (WINAPI *MP3_PREDECODE)(MPEGL3 *mpegl3, BYTE *head);
typedef UINT (WINAPI *MP3_DECODE)(MPEGL3 *mpegl3, short *pcm,
						BYTE *samp, UINT size);

typedef MPEGL3 * (WINAPI *MP3_CREATEEX)(BYTE *samp1, UINT size1,
						BYTE *samp2, UINT size2);
typedef UINT (WINAPI *MP3_PREDECODEEX)(MPEGL3 *mpegl3,
						BYTE *samp1, UINT size1, BYTE *samp2, UINT size2);
typedef UINT (WINAPI *MP3_DECODEEX)(MPEGL3 *mpegl3, short *pcm,
						BYTE *samp1, UINT size1, BYTE *samp2, UINT size2);

typedef void (WINAPI *MP3_ADJUSTGAIN)(MPEGL3 *mpegl3, int gain);

#ifdef __cplusplus
}
#endif

