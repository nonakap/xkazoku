/*	$Id: moviemng.c,v 1.3 2003/08/13 05:01:54 yui Exp $	*/

/*
 * Copyright (C) 2004 NONAKA Kimihiro
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
#include "arcfile.h"
#include "gamecore.h"

#include "x11.h"
#include "moviemng.h"
#include "audio.h"
#include "dosio.h"
#include "inputmng.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>

#include <SDL_syswm.h>

#if !defined(SUPPORT_MOVIE_MPLAYER) && !defined(SUPPORT_MOVIE_XINE)
void
moviemng_play(const char *fname, SCRN_T *scrn)
{

	(void)fname;
	(void)scrn;
}
#else
void
moviemng_play(const char *fname, SCRN_T *scrn)
{
	static const char ext[][5] = { "", ".AVI", ".MPG", ".avi", ".mpg" };
	char movie_file[MAX_PATH] = "";
	char path[MAX_PATH];
	struct stat st;
	SDL_SysWMinfo info;
	ARCFILEH arch;
	int i;

	UNUSED(scrn);

	memset(&info, 0, sizeof(info));
	info.version.major = SDL_MAJOR_VERSION;
	info.version.minor = SDL_MINOR_VERSION;
	if (SDL_GetWMInfo(&info) <= 0)
		return;

	/*
	 * 1. とりあえずデータディレクトリから検索
	 */
	for (i = 0; i < NELEMENTS(ext); ++i) {
		milstr_ncpy(path, gamecore.suf.scriptpath, sizeof(path));
		milstr_ncat(path, fname, sizeof(path));
		cutExtName(path);
		milstr_ncat(path, ext[i], sizeof(path));

		if (stat(path, &st) < 0)
			continue;
		if (st.st_mode & S_IFDIR)
			continue;
		if (access(path, R_OK) < 0)
			continue;

		/* OK */
		goto run;
	}

	/*
	 * 2. アーカイブ内に存在するか確認
	 *    (こんなあったら嫌だなぁ…)
	 */
	for (arch = NULL, i = 0; i < ARCTYPES; ++i) {
		arch = arcfile_open(i, fname);
		if (arch)
			break;
	}
	if (arch) {
		int fd;
		char *buf;

		sprintf(path, "/tmp/." APP_NAME "-%s-XXXXXXX", fname);
#if defined(HAVE_MKSTEMP) || defined(__NetBSD__)
		fd = mkstemp(path);
#else
		mktemp(path);
		fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0600);
#endif
		if (fd >= 0) {
			buf = (char *)_MALLOC(arch->size, "movie temp file");
			arcfile_seek(arch, arch->pos, SEEK_SET);
			if (arcfile_read(arch, buf, arch->size) == arch->size) {
				write(fd, buf, arch->size);
				close(fd);
				_MFREE(buf);
				milstr_ncpy(movie_file,path,sizeof(movie_file));
				goto run;
			}
			close(fd);
			_MFREE(buf);
		}
	}
	return;

run:
	inputmng_resetmouse(0);
	sound_term();

	info.info.x11.lock_func();
	movie_play(path);
	info.info.x11.unlock_func();

	if (movie_file[0] != '\0') {
		unlink(movie_file);
	}

	sound_init(audio_rate);
	sound_play();
}
#endif
