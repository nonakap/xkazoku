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

extern	PCMSTREAM	arc_stream;
extern	PCMSTREAM	mp3_stream;
extern	PCMSTREAM	se_stream;
extern	PCMSTREAM	onmem_stream;

#ifdef STREAM_BUFFERING
void stream_prepart_task(void);
#endif

#ifdef __cplusplus
}
#endif

