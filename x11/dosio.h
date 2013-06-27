/*	$Id: dosio.h,v 1.4 2003/05/18 02:17:52 yui Exp $	*/

#ifdef	__cplusplus
extern "C" {
#endif

typedef FILE *		FILEH;
#define	FILEH_INVALID	NULL

#define	FSEEK_SET	SEEK_SET
#define	FSEEK_CUR	SEEK_CUR
#define	FSEEK_END	SEEK_END

#define	FILE_ATTRIBUTE_READONLY		0x01
#define	FILE_ATTRIBUTE_HIDDEN		0x02
#define	FILE_ATTRIBUTE_SYSTEM		0x04
#define	FILE_ATTRIBUTE_VOLUME		0x08
#define	FILE_ATTRIBUTE_DIRECTORY	0x10
#define	FILE_ATTRIBUTE_ARCHIVE		0x20
#define	FILE_ATTRIBUTE_NORMAL		0x40

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
char *cutFileName(char *filename);
char *getExtName(char *filename);
void cutExtName(char *filename);
void cutyen(char *str);
void plusyen(char *str, int len);
void fname_mix(char *str, char *mix, int size);

#ifdef	__cplusplus
};
#endif
