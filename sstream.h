
#define STREAM_BUFFERING

typedef struct {
	UINT	type;
const char	*fname;
} ARCSTREAMARG;

typedef struct {
const BYTE	*ptr;
	UINT	size;
} ONMEMSTMARG;


#ifdef __cplusplus
extern "C" {
#endif

#ifdef STREAM_BUFFERING
int streambuf_attach(SNDSTREAM stream, int num);
void stream_prepart_task(void);
#endif

int mp3_ssopen(SNDSTREAM stream, void *arg, int num);
int arc_ssopen(SNDSTREAM stream, void *arg, int num);
int arcraw_ssopen(SNDSTREAM stream, void *arg, int num);
int onmem_ssopen(SNDSTREAM stream, void *arg, int num);
int arcse_ssopen(SNDSTREAM stream, void *arg, int num);

#ifdef __cplusplus
}
#endif

