#include "compiler.h"
#include "taskmng.h"
#include "inputmng.h"
#include "sound.h"
#include "sstream.h"
#include "x11.h"
#include "audio.h"

#include <signal.h>

static BOOL is_proc = 1;

void
sighandler(int signo)
{

        UNUSED(signo);
	is_proc = 0;
}

void
taskmng_init(void)
{

	is_proc = 1;
}

void
taskmng_term(void)
{
}

void
taskmng_exit(void)
{

	is_proc = 0;
}

void
taskmng_rol(void)
{
	SDL_Event e;

	if (is_proc) {
		if (SDL_PollEvent(&e)) {
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
				is_proc = 0;
				break;

			default:
				break;
			}
		}
		stream_prepart_task();
	}
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
