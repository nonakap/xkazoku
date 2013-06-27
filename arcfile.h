
#define	ARCHIVE_MAXFILES	32
#define	ARCFILENAME_LEN		12


enum {
	ARCTYPE_SCRIPT		= 0,
	ARCTYPE_GRAPHICS	= 1,
	ARCTYPE_SOUND		= 2,
	ARCTYPE_SE			= 3,
	ARCTYPE_VOICE		= 4,
	ARCTYPE_DATA		= 5,
	ARCTYPES			= 6
};

typedef struct {
	char	*filename;					// archive filename
	void	*fh;						// archive file handle
	long	base;						// archive file position
	long	pos;						// arcfile file position
	UINT	size;						// arcfile size
	char	*ext;						// arcfile ext name (NULL = no ext)
	char	name[ARCFILENAME_LEN+1];	// arcfile name / NULL terminal
} _ARCFILEH, *ARCFILEH;


#ifdef __cplusplus
extern "C" {
#endif

BOOL archive_create(void);
BOOL archive_set(const char *dir, const char *name, UINT type, UINT num);
void archive_destory(void);

int arcfile_gettestfiles(UINT type);
BOOL arcfile_gettestname(UINT type, int num, char *fname, int size);

ARCFILEH arcfile_open(UINT type, const char *fname);
void arcfile_close(ARCFILEH hdl);
UINT arcfile_read(ARCFILEH hdl, void *buf, UINT size);
long arcfile_seek(ARCFILEH hdl, long pos, int method);

// ���ƤΥ��������֤��Ȼפ���ʪ���ꤲ�롣
void archive_throwall(const char *path);

#ifdef __cplusplus
}
#endif

