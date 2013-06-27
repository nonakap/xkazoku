
typedef struct {
	BYTE	*ptr;
	int		xalign;
	int		yalign;
	int		width;
	int		height;
	int		bit;
const void	*arg;
} DEST_SURFACE;

#ifdef	__cplusplus
extern "C" {
#endif

BOOL xdraws_init(int width, int height);
void xdraws_term(void);
BOOL xdraws_draws(void (*cb)(DEST_SURFACE *), const void *arg);

#ifdef	__cplusplus
}
#endif

