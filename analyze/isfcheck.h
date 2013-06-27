
#ifdef __cplusplus
extern "C" {
#endif

void isfcheck_cmdlist(const char *path, const char *filename);
void isfcheck_dump(const char *path, const char *filename);
void isfcheck_cmdcheck(const char *path, const char *filename,
								int exever, UINT gametype, int warnlevel);

void isfcheck_cmddisp(const char *path, const char *filename, BYTE cmd);

#ifdef __cplusplus
}
#endif

