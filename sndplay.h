
enum {
	SNDPLAY_CDDA		= 0x01,
	SNDPLAY_CDDAPLAY	= 0x02,
	SNDPLAY_WAVE		= 0x04,
	SNDPLAY_WAVEPLAY	= 0x08,
	SNDPLAY_PCMLOAD		= 0x10
};

typedef struct {
	UINT	playing;
	int		waveloop;
	int		cddatrack;
	int		cddaloop;
	int		cddastart;
	int		cddaend;
	int		se[SOUNDTRK_MAXSE];
	char	sound[16];
	char	pcm[16];
} SNDPLAY_T, *SNDPLAY;


#ifdef __cplusplus
extern "C" {
#endif

void sndplay_init(void);

void sndplay_cddaset(int cddastart, int cddaend);
void sndplay_cddaplay(int track, int cmd, int tick);
void sndplay_cddastop(int tick);
void sndplay_cddaenable(int enable, int fade);

void sndplay_waveset(const char *fname, int cmd);
void sndplay_waveplay(BYTE flag, int tick);
void sndplay_wavestop(int tick);
void sndplay_waveenable(int enable, int fade);

void sndplay_seplay(int cmd, int tick);
void sndplay_seenable(int enable, int fade);

void sndplay_voiceset(const char *fname);
void sndplay_voicereset(void);
void sndplay_voiceplay(void);
BOOL sndplay_voicecondition(const char *fname);

#ifdef __cplusplus
}
#endif

