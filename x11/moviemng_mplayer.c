/*	$Id$	*/

/*
 * Copyright (C) 2001- Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
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

#if defined(SUPPORT_MOVIE_MPLAYER)

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "SDL_syswm.h"

#include "gamecore.h"
#include "x11.h"
#include "moviemng.h"


#ifndef	MPLAYER
#define	MPLAYER	""
#endif
char mplayer_cmd[MAX_PATH] = MPLAYER;
int mplayer_flag = 0;

static char config_file[MAX_PATH] = "";

static pid_t run_mplayer(const char *player, const char *filepath, int window_id);

void
movie_play(const char *moviefile)
{
	SDL_SysWMinfo info;
	char *player = NULL;
	pid_t movie_pid;

	if (!mplayer_flag)
		player = getenv("MPLAYER_CMD");
	if (player == NULL) {
		player = mplayer_cmd;
		if (player == NULL || player[0] == '\0')
			return;
	}

	memset(&info, 0, sizeof(info));
	if (SDL_GetWMInfo(&info) < 0)
		return;

	movie_pid = run_mplayer(player, moviefile, info.info.x11.window);
	if (movie_pid > 0)
		waitpid(movie_pid, 0, 0);
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
#if defined(HAVE_MKSTEMP) || defined(__NetBSD__)
	fd = mkstemp(config_file);
#else
	mktemp(config_file);
	fd = open(config_file, O_RDWR | O_CREAT | O_TRUNC, 0600);
#endif
	if (fd < 0)
		return;
	write(fd, conf, strlen(conf));
	close(fd);
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
			fd = open("/dev/null", O_RDWR, 0600);
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
#endif
