#include	"compiler.h"
#include	"xsound.h"
#include	"sound.h"


#define	DEFAULT_SAMPLES		(512 * 4)
#if defined(SOUND_MONOOUT)
#define	DEFAULT_CHANNELS	1
#else
#define	DEFAULT_CHANNELS	2
#endif
#define	NSNDBUF				3

#define	SAMPLEALIGN			(DEFAULT_CHANNELS * 2)

static	BOOL	sound_opened = FALSE;
static	int		nsndbuf = 0;
static	char	sndbuf[NSNDBUF][DEFAULT_SAMPLES * SAMPLEALIGN];


static void sound_play_cb(void *userdata, BYTE *stream, int len) {

	int		length;

	length = (len > DEFAULT_SAMPLES * SAMPLEALIGN)
								?(DEFAULT_SAMPLES * SAMPLEALIGN):len;
	soundmix_getpcm((short *)sndbuf[nsndbuf], length / SAMPLEALIGN);
	SDL_MixAudio(stream, sndbuf[nsndbuf], length, SDL_MIX_MAXVOLUME);
	nsndbuf = (nsndbuf + 1) % NSNDBUF;
	(void)userdata;
}

BOOL xsound_init(UINT rate) {

	SDL_AudioSpec	fmt;

	if (sound_opened) {
		return(SUCCESS);
	}
	if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Error: SDL_Init: %s\n", SDL_GetError());
		return(FAILURE);
	}

	ZeroMemory(&fmt, sizeof(fmt));
	fmt.freq = rate;
	fmt.format = AUDIO_S16SYS;
	fmt.channels = DEFAULT_CHANNELS;
	fmt.samples = DEFAULT_SAMPLES;
	fmt.callback = sound_play_cb;
	if (SDL_OpenAudio(&fmt, NULL) < 0) {
		fprintf(stderr, "Error: SDL_OpenAudio: %s\n", SDL_GetError());
		return(FAILURE);
	}
	sound_opened = TRUE;
	return(SUCCESS);
}

BOOL xsound_term(void) {

	if (sound_opened) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		nsndbuf = 0;
		ZeroMemory(sndbuf, sizeof(sndbuf));
		sound_opened = FALSE;
	}
	return(SUCCESS);
}

void xsound_play(void) {

	if (sound_opened) {
		SDL_PauseAudio(0);
	}
}

void xsound_stop(void) {

	if (sound_opened) {
		SDL_PauseAudio(1);
	}
}

