#include	"compiler.h"
#include	"cddamng.h"


BOOL cddamng_play(int track, int loop, int fadeintick) {

	(void)track;
	(void)loop;
	(void)fadeintick;
	return(FAILURE);			// FAILUREで次のデバイス(.mp3)を再生する
}

void cddamng_stop(int fadeouttick) {

	(void)fadeouttick;
}

