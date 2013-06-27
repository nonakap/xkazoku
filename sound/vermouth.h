
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	UINT	samprate;
} *MIDIMOD;

typedef struct {
	UINT	samprate;
	UINT	worksize;
} *MIDIHDL;

UINT midiout_getver(char *string, int leng);

MIDIMOD midimod_create(UINT samprate);
void midimod_destroy(MIDIMOD hdl);
void midimod_loadprogram(MIDIMOD hdl, UINT num);
void midimod_loadrhythm(MIDIMOD hdl, UINT num);

MIDIHDL midiout_create(MIDIMOD module, UINT worksize);
void midiout_destroy(MIDIHDL hdl);
void midiout_shortmsg(MIDIHDL hdl, UINT msg);
void midiout_longmsg(MIDIHDL hdl, const void *msg, UINT size);
UINT midiout_get(MIDIHDL hdl, void *pcm, UINT size);
void midiout_setgain(MIDIHDL hdl, int gain);

#ifdef __cplusplus
}
#endif

