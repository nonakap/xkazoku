
enum {
	LOG_SJIS	= 0x00,
	LOG_EUC		= 0x80,

	LOG_LF		= 1,
	LOG_CR		= 2,
	LOG_CRLF	= 3
};

#ifdef __cplusplus
extern "C" {
#endif

void log_open(void);
void log_ext(const char *filename);
void log_close(void);
void log_disp(const char *str, ...);

#ifdef __cplusplus
}
#endif

