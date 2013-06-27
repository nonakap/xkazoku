/*	$Id: akira.c,v 1.10 2003/08/13 06:26:08 yui Exp $	*/

#include "compiler.h"

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include "dosio.h"
#include "gamecore.h"
#include "inputmng.h"
#include "scrnmng.h"
#include "sysmenu.h"
#include "x11.h"
#include "audio.h"
#include "sound.h"
#include "sstream.h"
#include "arcfile.h"

#include "fontmng.h"
#include "inputmng.h"
#include "moviemng.h"
#include "taskmng.h"

#if defined(SUPPORT_MOVIE_XINE)
#include <X11/Xlib.h>
#endif


/*
 * signal handler
 */
sigfunc *
setup_signal(int signo, sigfunc *func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}

static char *progname;

static void
usage(void)
{
	printf("Usage: %s [options]\n", progname);
	printf("\t--help       [-h]                 : print this message\n");
	printf("\t--fullscreen [-f]                 : full screen mode\n");
#if defined(SUPPORT_MOVIE_MPLAYER)
	printf("\t--mplayer    [-m] <file>          : specify MPlayer execute file\n");
#endif
	printf("\t--suf        [-s] <file>          : specify .SUF file\n");
	printf("\t--ttfont     [-t] <file>          : specify TrueType font file\n");
	printf("\t--rate       [-r] <sampling rate> : specify sound sampling rate\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	static struct option longopts[] = {
		{ "fullscreen",	no_argument,		0,	'f' },
		{ "mplayer",	required_argument,	0,	'm' },
		{ "suf",	required_argument,	0,	's' },
		{ "ttfont",	required_argument,	0,	't' },
		{ "rate",	required_argument,	0,	'r' },
		{ "help",	no_argument,		0,	'h' },
		{ 0,		0,			0,	0   },
	};
	static char suffile[MAX_PATH] = "system.suf";
	long rate;
	int rv = 1;
	int ch;

	progname = argv[0];

	while ((ch = getopt_long(argc, argv, "hfm:s:t:r:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'f':
			fullscreen_flag = 1;
			break;

		case 'm':
#if defined(SUPPORT_MOVIE_MPLAYER)
			if (access(optarg, X_OK) < 0) {
				fprintf(stderr, "Can't access %s.\n", optarg);
				exit(1);
			}
			milstr_ncpy(mplayer_cmd, optarg, sizeof(mplayer_cmd));
			mplayer_flag = 1;
#else
			fprintf(stderr, "Need to defined SUPPORT_MOVIE_MPLAYER!!!\n");
#endif
			break;

		case 's': 
			if (access(optarg, R_OK) < 0) {
				fprintf(stderr, "Can't access %s.\n", optarg);
				exit(1);
			}
			milstr_ncpy(suffile, optarg, sizeof(suffile));
			break;

		case 't':
			if (access(optarg, R_OK) < 0) {
				fprintf(stderr, "Can't access %s.\n", optarg);
				exit(1);
			}
			milstr_ncpy(fontname, optarg, sizeof(fontname));
			break;

		case 'r':
			rate = milstr_solveINT(optarg);
			switch (rate) {
			case 0:
			case 11025:
			case 22050:
			case 44100:
			case 48000:
				audio_rate = rate;
				break;

			default:
				fprintf(stderr, "invalid sound sampling rate (%ld)\n", rate);
				exit(1);
			}
			break;

		case '?':
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	dosio_init();

	file_setcd(suffile);

	TRACEINIT();

#if defined(SUPPORT_MOVIE_XINE)
	XInitThreads();
#endif

	if (fontmng_init() != SUCCESS)
		goto gamecore_out;
	inputmng_init();

	archive_namingconv(ARCNAME_TOUPPER);
	if (gamecore_init(suffile) != SUCCESS)
		goto gamecore_out;

	if (sysmenu_create() != SUCCESS)
		goto sysmenu_out;

	if (xdraws_init(SCREEN_WIDTH, SCREEN_HEIGHT) != SUCCESS)
		goto draw_out;

	setup_signal(SIGINT, sighandler);
	setup_signal(SIGTERM, sighandler);

	sound_init(audio_rate);
	soundmix_create(audio_rate);
#if defined(VERMOUTH_LIB)
	vermouth_init();
#endif
	sound_play();

	taskmng_init();

	gamecore_exec();
	rv = 0;

	taskmng_term();

	sound_stop();
	sound_term();
#if defined(VERMOUTH_LIB)
	vermouth_term();
#endif

draw_out:
	xdraws_term();
sysmenu_out:
	sysmenu_destroy();
gamecore_out:
	gamecore_destroy();
	TRACETERM();
	SDL_Quit();
	dosio_term();

	return rv;
}
