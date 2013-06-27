
#define	ARCFILENAME_LEN		12

typedef struct {
	void	*fh;
	int		files;
} _ARCFH, *ARCFH;

typedef struct {
	char	name[ARCFILENAME_LEN];
	long	ptr;
	UINT	size;
} _AFTBL, *AFTBL;

typedef struct {
	BYTE	*ptr;
	UINT	size;
	char	name[ARCFILENAME_LEN + 1];
} _ARCHDL, *ARCHDL;


#ifdef __cplusplus
extern "C" {
#endif

ARCFH arcread_open(const char *path);
void arcread_close(ARCFH afh);
ARCHDL arcread_get(ARCFH afh, const char *filename);
BOOL arcread_getname(ARCFH afh, int num, char *buffer, int size);
AFTBL arcread_getinf(ARCFH afh, int num);

#ifdef __cplusplus
}
#endif

