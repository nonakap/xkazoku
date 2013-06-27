
enum {
	TDFLAG_MULTI	= 1,
	TDFLAG_RENUM	= 2
};

typedef struct {
const BYTE		*ptr;
	int			remain;
} STRING_T;

typedef struct {
	STRING_T	cmd;
	STRING_T	txt;
	int			x;
	int			y;
	UINT		flag;
	BYTE		txtwork[36];
} TEXT_T;

typedef struct {
	int			txtnum;
	int			count;
	UINT		flag;
	int			skip;
	TEXT_T		txt[GAMECORE_MAXTEXT];
	STRING_T	*ext[GAMECORE_MAXTEXT];
} TEXTDISP_T, *TEXTDISP;


#ifdef __cplusplus
extern "C" {
#endif

void textdisp_draw(int num, POINT_T *pt, SCR_OPE *op);

int textdisp_set(int num, SCR_OPE *op);
int textdisp_multiset(int num, int txtnum, SCR_OPE *op);
void textdisp_multireset(void);
int textdisp_exec(void);

#ifdef __cplusplus
}
#endif

