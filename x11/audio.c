/*	$Id: audio.c,v 1.3 2003/08/13 05:01:53 yui Exp $	*/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by NONAKA Kimihiro.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "compiler.h"

#include "audio.h"
#include "sound.h"

#include <SDL_audio.h>

#define	DEFAULT_SAMPLES	512
#define	NSNDBUF		3

static BOOL sound_opened = FALSE;

static int nsndbuf = 0;
static char sndbuf[NSNDBUF][DEFAULT_SAMPLES * 4];

static void
sound_play_cb(void *userdata, Uint8 *stream, int len)
{
	int length = len > DEFAULT_SAMPLES * 4 ? DEFAULT_SAMPLES * 4 : len;

	UNUSED(userdata);

	soundmix_getpcm((short *)sndbuf[nsndbuf], length / 4);
	SDL_MixAudio(stream, sndbuf[nsndbuf], length, SDL_MIX_MAXVOLUME);

	nsndbuf = (nsndbuf + 1) % NSNDBUF;
}

BOOL
sound_init(UINT rate)
{
	SDL_AudioSpec fmt;
	UINT opna_rate;

	if (getenv("NOSOUND"))
		return FAILURE;

	if (sound_opened)
		return SUCCESS;

	if (rate == 0)
		return SUCCESS;

	if (rate < 11025)
		opna_rate = 11025;
	else if (rate < 44100)
		opna_rate = 22050;
	else
		opna_rate = 44100;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Error: SDL_Init: %s\n", SDL_GetError());
		return FAILURE;
	}

	memset(&fmt, 0, sizeof(fmt));
	fmt.freq = opna_rate;
	fmt.format = AUDIO_S16SYS;
	fmt.channels = 2;
	fmt.samples = DEFAULT_SAMPLES;
	fmt.callback = sound_play_cb;
	if (SDL_OpenAudio(&fmt, NULL) < 0) {
		fprintf(stderr, "Error: SDL_OpenAudio: %s\n", SDL_GetError());
		return FAILURE;
	}

	sound_opened = TRUE;
	return SUCCESS;
}

BOOL
sound_term(void)
{

	if (sound_opened) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		nsndbuf = 0;
		memset(sndbuf, 0, sizeof(sndbuf));
		sound_opened = FALSE;
	}
	return SUCCESS;
}

void
sound_play(void)
{

	if (sound_opened)
		SDL_PauseAudio(0);
}

void
sound_stop(void)
{

	if (sound_opened)
		SDL_PauseAudio(1);
}
