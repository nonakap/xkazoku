/*	$Id: moviemng.h,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#ifdef __cplusplus
extern "C" {
#endif

void moviemng_play(const char *fname, SCRN_T *scrn);

#if defined(SUPPORT_MOVIE_MPLAYER)
extern char mplayer_cmd[MAX_PATH];
extern int mplayer_flag;
#endif

void movie_play(const char *moviefile);

#ifdef __cplusplus
}
#endif

