/*	$Id: moviemng.c,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

/*
 * movie.c  movie を再生する
 *
 * Copyright (C)   2001-     Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
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

#ifndef	MPLAYER
#define	MPLAYER	""
#endif
char mplayer_cmd[MAX_PATH] = MPLAYER;
int mplayer_flag = 0;

static char movie_file[MAX_PATH] = "";
static char config_file[MAX_PATH] = "";

static pid_t run_mplayer(const char *player, const char *filepath, int window_id);

/*
 * 問答無用で mplayer を起動する
 */
void
moviemng_play(const char *fname, SCRN_T *scrn)
{
	static const char ext[][5] = { "", ".AVI", ".MPG", ".avi", ".mpg" };
	SDL_SysWMinfo info;
	char path[MAX_PATH];
	struct stat st;
	ARCFILEH arch;
	char *player = NULL;
	pid_t movie_pid;
	int i;

	UNUSED(scrn);

	if (!mplayer_flag)
		player = getenv("MPLAYER_CMD");
	if (player == NULL) {
		player = mplayer_cmd;
		if (player == NULL || player[0] == '\0')
			return;
	}

	bzero(&info, sizeof(info));
	if (!SDL_GetWMInfo(&info))
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
		break;
	}
	if (i < NELEMENTS(ext))
		goto run;

	/*
	 * 2. アーカイブ内に存在するか確認
	 *    (こんなあったら嫌だなぁ…)
	 */
	for (arch = 0, i = 0; i < ARCTYPES; ++i) {
		arch = arcfile_open(i, fname);
		if (arch)
			break;
	}
	if (arch) {
		int fd;
		char *buf;

		sprintf(path, "/tmp/." APP_NAME "-%s-XXXXXXX", fname);
#if defined(HAVE_MKSTEMP)
		fd = mkstemp(path);
#else
		mktemp(path);
		fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0600);
#endif
		buf = (char *)_MALLOC(arch->size, "movie temp file");
		arcfile_seek(arch, arch->pos, SEEK_SET);
		if (arcfile_read(arch, buf, arch->size) == arch->size) {
			write(fd, buf, arch->size);
			_MFREE(buf);

			milstr_ncpy(movie_file, path, sizeof(movie_file));
			goto run;
		}
		_MFREE(buf);
	}

	printf("movie filename: %s\n", fname);
	return;

run:
	inputmng_resetmouse(0);

	movie_pid = run_mplayer(player, path, info.info.x11.window);

	/* 待っていても問題無い様なので… */
	waitpid(movie_pid, 0, 0);
	if (movie_file[0] != '\0') {
		unlink(movie_file);
		movie_file[0] = '\0';
	}

	sound_init(AUDIO_RATE);
	sound_play();
}

static void
make_config_file(void)
{
	static const char conf[] =
		"ENTER quit\n"
		"SPACE quit\n"
		"q quit\n"
		"MOUSE_BTN0 quit\n"
		"MOUSE_BTN1 quit\n"
		"MOUSE_BTN2 quit\n";
	int fd;

	milstr_ncpy(config_file, "/tmp/." APP_NAME "-mplayer-input-XXXXXXX", sizeof(config_file));
#if defined(HAVE_MKSTEMP)
	fd = mkstemp(config_file);
#else
	mktemp(config_file);
	fd = open(config_file, O_RDWR | O_CREAT | O_TRUNC, 0600);
#endif
	if (fd < 0)
		return;
	write(fd, conf, strlen(conf));
}

static void
movie_signal_handler(int sig)
{

	UNUSED(sig);

	if (config_file[0] != '\0') {
		unlink(config_file);
		config_file[0] = '\0';
	}
}

static pid_t
run_mplayer(const char *player, const char *filepath, int window_id)
{
	int pipes[2];
	pid_t child_pid;

	if (pipe(pipes) < 0)
		return -1;

	sound_term();

	child_pid = fork();
	if (child_pid < 0)
		return child_pid;

	if (child_pid == 0) {	/* child process */
		pid_t pid;

		/* new process group */
		if (setpgid(getpid(), getpid()) < 0)
			perror("setpgid : ");

		signal(SIGCHLD, SIG_DFL);

		make_config_file();

		pid = fork();
		if (pid < 0)
			exit(0);
		if (pid == 0) {
			int fd;
			char opt1[20], opt2[10], opt3[10], opt4[256];

			close(pipes[1]);
			close(0);
			dup(pipes[0]);
			close(pipes[0]);

#if 1
			fd = open("/dev/null", O_RDWR, 0755);
#else
			fd = open("/tmp/mlog", O_RDWR|O_CREAT, 0600);
#endif
			if (fd < 0) {
				perror("open: ");
			} else {
				close(1);
				dup(fd);
				close(2);
				dup(fd);
				close(fd);
			}

			sprintf(opt1, "%d", window_id);
			sprintf(opt2, "%d", SCREEN_WIDTH);
			sprintf(opt3, "%d", SCREEN_HEIGHT);
			sprintf(opt4, "conf=%s", config_file);

			execl(player, player, "-vo", "x11", "-wid", opt1,
			    "-x", opt2, "-y", opt3, "-zoom",
			    "-input", opt4, "-framedrop", filepath, 0);
			exit(0);
		}
		signal(SIGINT, movie_signal_handler);
		signal(SIGTERM, movie_signal_handler);
		waitpid(pid, 0, 0);
		if (config_file[0] != '\0') {
			unlink(config_file);
			config_file[0] = '\0';
		}
		exit(0);
	}
	close(pipes[0]);
	close(pipes[1]);
	return child_pid;
}
