/*	$Id: cddamng.c,v 1.1.1.1 2003/04/14 12:48:03 yui Exp $	*/

#include "compiler.h"

#include "cddamng.h"

#include <SDL_cdrom.h>

static int cdda_inited = 0;
static int cddrive = -1;
static SDL_CD *cdrom = NULL;
static SDL_TimerID sdlcdda_timer_id;

static int
sdlcdda_init(void)
{
	char *env;
	int num;
	int i;

	if (SDL_InitSubSystem(SDL_INIT_CDROM|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Error: initialize SDL: %s\n", SDL_GetError());
		return FAILURE;
	}

	num = SDL_CDNumDrives();
	if (num == 0) {
		printf("No CD-ROM devices detected\n");
		return FAILURE;
	}

	for (i = 0; i < num; i++) {
		printf("CD-ROM %d name: %s\n", i, SDL_CDName(i));
	}

	env = getenv("CDROM_DRIVE");
	if (env != NULL) {
		cddrive = atoi(env);
		if (cddrive >= num) {
			fprintf(stderr, "Error: CDROM_DRIVE: %s\n", env);
			cddrive = 0;
		}
	} else {
		cddrive = 0;
	}
	cdrom = SDL_CDOpen(cddrive);
	if (cdrom == NULL) {
		fprintf(stderr, "Couldn't open drive %d: %s\n", cddrive, SDL_GetError());
		cddrive = -1;
		return FAILURE;
	}

	return SUCCESS;
}

static Uint32
sdlcdda_cb(Uint32 interval, void *param)
{
	Uint32 track = (Uint32)param;
	Uint32 len;
	int s;

	s = SDL_CDStatus(cdrom);
	if (CD_INDRIVE(s)) {
		if (s == CD_STOPPED) {
			SDL_CDPlayTracks(cdrom, track - 1, 0, 1, 0);
		}
		len = interval;
	} else {
		SDL_RemoveTimer(sdlcdda_timer_id);
		sdlcdda_timer_id = NULL;
		len = 0;
	}
	return len;
}

BOOL
cddamng_play(int track, int loop, int fadeintick)
{
	int s;

	UNUSED(fadeintick);

	if (getenv("NOCDROM")) {
		printf("NOCDROM = %s\n", getenv("NOCDROM"));
		return FAILURE;
	}

	if (!cdda_inited) {
		cdda_inited = 1;
		if (sdlcdda_init() != SUCCESS)
			return FAILURE;
	}

	if (cdrom == NULL)
		return FAILURE;

	s = SDL_CDStatus(cdrom);
	if (CD_INDRIVE(s)) {
		if (s != CD_STOPPED) {
			SDL_CDStop(cdrom);
		}
		SDL_CDPlayTracks(cdrom, track - 1, 0, 1, 0);
		if (loop) {
			sdlcdda_timer_id = SDL_AddTimer((cdrom->track[track-1].length / 75) * 1000, sdlcdda_cb, (void *)track);
		}
		return SUCCESS;
	}
	return FAILURE;
}

void
cddamng_stop(int fadeouttick)
{
	int s;

	UNUSED(fadeouttick);

	if (!cdda_inited)
		return;

	if (cdrom == NULL)
		return;

	s = SDL_CDStatus(cdrom);
	if (CD_INDRIVE(s)) {
		if (s != CD_STOPPED) {
			SDL_CDStop(cdrom);
		}
		if (sdlcdda_timer_id != NULL) {
			SDL_RemoveTimer(sdlcdda_timer_id);
			sdlcdda_timer_id = NULL;
		}
	}
}
