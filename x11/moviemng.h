/*	$Id: moviemng.h,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#ifdef __cplusplus
extern "C" {
#endif

extern char mplayer_cmd[MAX_PATH];
extern int mplayer_flag;

void moviemng_play(const char *fname, SCRN_T *scrn);

#ifdef __cplusplus
}
#endif

