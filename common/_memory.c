#include	"compiler.h"
#include	"dosio.h"


#define	MEMTBLMAX	256
#define	HDLTBLMAX	256

#if defined(MEMTRACE)

typedef struct {
	void	*hdl;
	UINT	size;
	char	name[24];
} _MEMTBL;

typedef struct {
	void	*hdl;
	char	name[28];
} _HDLTBL;

static _MEMTBL	memtbl[MEMTBLMAX];
static _HDLTBL	hdltbl[HDLTBLMAX];

const static char memstr[] =										\
				"Handle   Size       Name\r\n"						\
				"--------------------------------------------\r\n";

const static char hdlstr[] =										\
				"Handle   Name\r\n"									\
				"-------------------------------------\r\n";

static void _scpy(char *dst, const char *src, int size) {

	if ((dst) && (size)) {
		int len = 0;
		size--;
		if (src) {
			len = strlen(src);
			if (len > size) {
				len = size;
			}
			if (len) {
				CopyMemory(dst, src, len);
			}
		}
		dst[len] = '\0';
	}
}


void _meminit(void) {

	ZeroMemory(memtbl, sizeof(memtbl));
	ZeroMemory(hdltbl, sizeof(hdltbl));
}


void *_memalloc(int size, const char *name) {

	void	*ret;

	ret = malloc(size);
	if (ret) {
		int		i;
		for (i=0; i<MEMTBLMAX; i++) {
			if (memtbl[i].hdl == NULL) {
				memtbl[i].hdl = ret;
				memtbl[i].size = size;
				_scpy(memtbl[i].name, name, 24);
				break;
			}
		}
	}
	return(ret);
}


void _memfree(void *hdl) {

	if (hdl) {
		int		i;
		for (i=0; i<MEMTBLMAX; i++) {
			if (memtbl[i].hdl == hdl) {
				memtbl[i].hdl = NULL;
				break;
			}
		}
		free(hdl);
	}
}


void _handle_append(void *hdl, const char *name) {

	if (hdl) {
		int		i;
		for (i=0; i<HDLTBLMAX; i++) {
			if (hdltbl[i].hdl == NULL) {
				hdltbl[i].hdl = hdl;
				_scpy(hdltbl[i].name, name, 28);
				break;
			}
		}
	}
}


void _handle_remove(void *hdl) {

	if (hdl) {
		int		i;
		for (i=0; i<HDLTBLMAX; i++) {
			if (hdltbl[i].hdl == hdl) {
				hdltbl[i].hdl = NULL;
				break;
			}
		}
	}
}


void _memused(const char *filename) {

	int		i;
	FILEH	fh;
	int		memuses = 0;
	int		hdluses = 0;
	BYTE	memusebit[(MEMTBLMAX+7)/8];
	BYTE	hdlusebit[(HDLTBLMAX+7)/8];
	char	work[256];

	ZeroMemory(memusebit, sizeof(memusebit));
	ZeroMemory(hdlusebit, sizeof(hdlusebit));
	for (i=0; i<MEMTBLMAX; i++) {
		if (memtbl[i].hdl) {
			memusebit[i>>3] |= (BYTE)0x80 >> (i & 7);
			memuses++;
		}
	}
	for (i=0; i<HDLTBLMAX; i++) {
		if (hdltbl[i].hdl) {
			hdlusebit[i>>3] |= (BYTE)0x80 >> (i & 7);
			hdluses++;
		}
	}
	fh = file_create_c(filename);
	if (fh != (FILEH)-1) {
		wsprintf(work, "memused: %d\r\n", memuses);
		file_write(fh, work, strlen(work));
		if (memuses) {
			file_write(fh, memstr, strlen(memstr));
			for (i=0; i<MEMTBLMAX; i++) {
				if ((memusebit[i>>3] << (i & 7)) & 0x80) {
					wsprintf(work, "%08lx %10u %s\r\n",
						(long)memtbl[i].hdl, memtbl[i].size, memtbl[i].name);
					file_write(fh, work, strlen(work));
				}
			}
			file_write(fh, "\r\n", 2);
		}
		wsprintf(work, "hdlused: %d\r\n", hdluses);
		file_write(fh, work, strlen(work));
		if (hdluses) {
			file_write(fh, hdlstr, strlen(hdlstr));
			for (i=0; i<HDLTBLMAX; i++) {
				if ((hdlusebit[i>>3] << (i & 7)) & 0x80) {
					wsprintf(work, "%08lx %s\r\n",
									(long)hdltbl[i].hdl, hdltbl[i].name);
					file_write(fh, work, strlen(work));
				}
			}
			file_write(fh, "\r\n", 2);
		}
		file_close(fh);
	}
}

#elif defined(MEMCHECK)

typedef struct {
	void	*hdl;
	DWORD	size;
} _MEMTBL;

		DWORD	usedmemory;
		BOOL	chgmemory;
static	_MEMTBL	memtbl[MEMTBLMAX];


void _meminit(void) {

	usedmemory = 0;
	chgmemory = FALSE;
	ZeroMemory(memtbl, sizeof(memtbl));
}


void *_memalloc(int size) {

	void	*ret;

	ret = malloc(size);
	if (ret) {
		int		i;
		for (i=0; i<MEMTBLMAX; i++) {
			if (memtbl[i].hdl == NULL) {
				memtbl[i].hdl = ret;
				memtbl[i].size = size;
				usedmemory += size;
				chgmemory = TRUE;
				break;
			}
		}
	}
	return(ret);
}


void _memfree(void *hdl) {

	if (hdl) {
		int		i;
		for (i=0; i<MEMTBLMAX; i++) {
			if (memtbl[i].hdl == hdl) {
				memtbl[i].hdl = NULL;
				usedmemory -= memtbl[i].size;
				chgmemory = TRUE;
				break;
			}
		}
		free(hdl);
	}
}

#endif

