
typedef FILE *			FILEH;
#define	FILEH_INVALID	NULL

#define	FSEEK_SET	SEEK_SET
#define	FSEEK_CUR	SEEK_CUR
#define	FSEEK_END	SEEK_END

enum {
	FILEATTR_READONLY	= 0x01,
	FILEATTR_HIDDEN		= 0x02,
	FILEATTR_SYSTEM		= 0x04,
	FILEATTR_VOLUME		= 0x08,
	FILEATTR_DIRECTORY	= 0x10,
	FILEATTR_ARCHIVE	= 0x20
};

typedef struct {
	UINT16	year;		/* cx */
	BYTE	month;		/* dh */
	BYTE	day;		/* dl */
} DOSDATE;

typedef struct {
	BYTE	hour;		/* ch */
	BYTE	minute;		/* cl */
	BYTE	second;		/* dh */
} DOSTIME;


#ifdef	__cplusplus
extern "C" {
#endif

/* DOSIO:関数の準備 */
void dosio_init(void);
void dosio_term(void);

/* ファイル操作 */
FILEH file_open(const char *filename);
FILEH file_open_rb(const char *filename);
FILEH file_create(const char *filename);
long file_seek(FILEH handle, long pointer, int method);
UINT file_read(FILEH handle, void *data, UINT length);
UINT file_write(FILEH handle, const void *data, UINT length);
short file_close(FILEH handle);
UINT file_getsize(FILEH handle);
short file_getdatetime(FILEH handle, DOSDATE *dosdate, DOSTIME *dostime);
short file_delete(const char *filename);
short file_attr(const char *filename);

/* カレントファイル操作 */
void file_setcd(const char *exename);
char *file_getcd(const char *filename);
FILEH file_open_c(const char *filename);
FILEH file_open_rb_c(const char *filename);
FILEH file_create_c(const char *filename);
short file_delete_c(const char *filename);
short file_attr_c(const char *filename);

char *getFileName(char *filename);
void cutFileName(char *filename);
char *getExtName(char *filename);
void cutExtName(char *filename);
void cutyen(char *str);
void plusyen(char *str, int len);
void fname_mix(char *str, char *mix, int size);

#ifdef	__cplusplus
};
#endif
