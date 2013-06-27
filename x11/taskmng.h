
#ifdef __cplusplus
extern "C" {
#endif

void taskmng_init(void);
void taskmng_term(void);

void taskmng_exit(void);

void taskmng_rol(void);
BOOL taskmng_isavail(void);
BOOL taskmng_sleep(UINT32 tick);

#ifdef __cplusplus
}
#endif

