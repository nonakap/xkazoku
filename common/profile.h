
#ifdef __cplusplus
extern "C" {
#endif

int profile_getint(const char *appname, const char *key,
										int def, const char *fname);
BOOL profile_getstr(const char *appname, const char *key,
										const char *def, char *out,
										int size, const char *fname);
BOOL profile_putstr(const char *appname, const char *key,
										const char *str, const char *fname);

BOOL profile_enum(const char *filename, void *arg,
							BOOL (*proc)(void *arg, const char *para,
									const char *key, const char *data));

#ifdef __cplusplus
}
#endif

