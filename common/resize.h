
typedef struct {
	BYTE	*ptr;
	int		width;
	int		height;
	int		align;
	UINT	bpp;
} RSDIMG;

typedef struct {
const BYTE	*ptr;
	int		width;
	int		height;
	int		align;
	UINT	bpp;
} RSSIMG;


#ifdef __cplusplus
extern "C" {
#endif

void resize_init(void);
void resize_term(void);
void resize(const RSDIMG *dst, const RSSIMG *src);

#ifdef __cplusplus
}
#endif

