
#ifdef __cplusplus
extern "C" {
#endif

SAVEHDL savedrs_open(BOOL create);

BOOL savedrs_getkid(UINT16 num, const char *name);
void savedrs_setkid(UINT16 num, const char *name);
void savedrs_sysread(void);
void savedrs_syswrite(void);

#ifdef __cplusplus
}
#endif

