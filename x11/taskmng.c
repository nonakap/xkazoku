#include "compiler.h"
#include "taskmng.h"
#include "inputmng.h"
#include "sound.h"
#include "sstream.h"
#include "x11.h"
#include "audio.h"

#include <signal.h>

static BOOL is_proc = FALSE;

void
sighandler(int signo)
{

        UNUSED(signo);
	is_proc = FALSE;
}

void
taskmng_init(void)
{

	is_proc = TRUE;
}

void
taskmng_term(void)
{
}

void
taskmng_exit(void)
{

	is_proc = FALSE;
}

void
taskmng_rol(void)
{
	SDL_Event e;

	if (!is_proc) {
		return;
	}

	while (SDL_PollEvent(&e) > 0) {
		switch (e.type) {
		case SDL_MOUSEBUTTONUP:
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				inputmng_buttonup(LBUTTON_BIT);
				break;

			case SDL_BUTTON_RIGHT:
				inputmng_buttonup(RBUTTON_BIT);
				break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				inputmng_buttondown(LBUTTON_BIT);
				break;

			case SDL_BUTTON_RIGHT:
				inputmng_buttondown(RBUTTON_BIT);
				break;
			}
			break;

		case SDL_KEYDOWN:
			inputmng_keyset(e.key.keysym.sym);
			break;

		case SDL_KEYUP:
			inputmng_keyreset(e.key.keysym.sym);
			break;

		case SDL_QUIT:
			taskmng_exit();
			break;
		}
	}
	stream_prepart_task();
}

BOOL
taskmng_isavail(void)
{

	return is_proc;
}

BOOL
taskmng_sleep(UINT32 tick)
{
	UINT32 base = GETTICK();
	while (is_proc && ((GETTICK() - base) < tick)) {
		taskmng_rol();
		usleep(1);
	}
	return is_proc;
}
