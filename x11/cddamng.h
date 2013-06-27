#ifdef	__cplusplus
extern "C" {
#endif

extern BOOL nocdrom_flag;

BOOL cddamng_play(int track, int loop, int fadeintick);
void cddamng_stop(int fadeouttick);

#ifdef	__cplusplus
};
#endif
