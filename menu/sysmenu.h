
#ifdef __cplusplus
extern "C" {
#endif

extern char str_ok[];
extern BYTE str_cancel[];
extern BYTE str_apply[];
extern BYTE str_prop[];

BOOL sysmenu_create(void);
void sysmenu_destroy(void);

BOOL sysmenu_menuopen(BOOL menu, int x, int y);

#ifdef __cplusplus
}
#endif

