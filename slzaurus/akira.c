#include "compiler.h"
// #include <sys/time.h>
// #include <signal.h>
// #include <unistd.h>
#include	"akira.h"
#include	"dosio.h"
#include	"gamecore.h"
#include	"xdraws.h"
#include	"xsound.h"
#include	"fontmng.h"
#include	"inputmng.h"
#include	"scrnmng.h"
#include	"taskmng.h"
#include	"sound.h"
#include	"sstream.h"
#include	"arcfile.h"
#include	"sysmenu.h"


static void usage(const char *progname) {

	printf("Usage: %s [options]\n", progname);
	printf("\t--help   [-h]       : print this message\n");
	printf("\t--suf    [-s] <file>: specify .SUF file\n");
	printf("\t--ttfont [-t] <file>: specify TrueType font file\n");
}

int SDL_main(int argc, char **argv) {

	int		rv;
	int		pos;
	char	*p;
	char	suffile[MAX_PATH] = "system.suf";
	BOOL	r;
	UINT	arcnametype;

	rv = FAILURE;
	arcnametype = 0;

	pos = 1;
	while(pos < argc) {
		p = argv[pos++];
		if ((!milstr_cmp(p, "-h")) || (!milstr_cmp(p, "--help"))) {
			usage(argv[0]);
			goto cmdline_out;
		}
		else if ((!milstr_cmp(p, "-s")) || (!milstr_cmp(p, "--suf"))) {
			if (pos < argc) {
				milstr_ncpy(suffile, argv[pos++], sizeof(suffile));
			}
		}
		else if ((!milstr_cmp(p, "-t")) || (!milstr_cmp(p, "--ttfont"))) {
			if (pos < argc) {
				fontmng_setdeffontname(argv[pos++]);
			}
		}
		else if (!milstr_cmp(p, "--arc-toupper")) {
			arcnametype |= ARCNAME_TOUPPER;
		}
		else if (!milstr_cmp(p, "--arc-tolower")) {
			arcnametype |= ARCNAME_TOLOWER;
		}
		else if (!milstr_cmp(p, "--arc-capitalize")) {
			arcnametype |= ARCNAME_CAPITALIZE;
		}
		else {
			printf("error command: %s\n", p);
			goto cmdline_out;
		}
	}

	dosio_init();

	file_setcd(suffile);

	TRACEINIT();

	if (fontmng_init() != SUCCESS) {
		goto fontmng_out;
	}
	inputmng_init();

	if (!arcnametype) {
		arcnametype = ARCNAME_TOUPPER;
	}
	archive_namingconv(arcnametype);
	if (gamecore_init(suffile) != SUCCESS) {
		fprintf(stderr, "Error: gamecore_init: %s\n", gamecore_geterror());
		goto gamecore_out;
	}
	if (sysmenu_create() != SUCCESS) {
		goto sysmenu_out;
	}
#ifndef SIZE_QVGA
	r = xdraws_init(gamecore.sys.width, gamecore.sys.height);
#else
	r = xdraws_init(320, 240);
#endif
	if (r != SUCCESS) {
		goto draw_out;
	}
	if (xsound_init(22050) == SUCCESS) {
		soundmix_create(22050);
	}
#if defined(VERMOUTH_LIB)
	vermouth_init();
#endif
	xsound_play();

	taskmng_init();

	gamecore_exec();
	rv = SUCCESS;

	taskmng_term();

	xsound_stop();
	xsound_term();
#if defined(VERMOUTH_LIB)
	vermouth_term();
#endif

draw_out:
	xdraws_term();

sysmenu_out:
	sysmenu_destroy();

gamecore_out:
	gamecore_destroy();

fontmng_out:
	TRACETERM();
	SDL_Quit();
	dosio_term();

cmdline_out:
	return(rv);
}

