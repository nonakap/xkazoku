
#ifdef __cplusplus
extern "C" {
#endif

extern char str_ok[];
extern BYTE str_cancel[];
extern BYTE str_apply[];

BOOL sysmenu_create(void);
void sysmenu_destroy(void);

BOOL sysmenu_menuopen(void);

#ifdef __cplusplus
}
#endif

