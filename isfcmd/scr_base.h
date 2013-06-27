
typedef struct {
	BYTE	*ptr;
	int		remain;

	UINT	cmd;			// どーせスタックにあるだろうから
	UINT	len;			// いちおー引き連れておく
	UINT	pos;
	UINT	size;
} SCR_OPE;


extern BYTE textsinglebyte[];


BOOL scr_valset(int num, SINT32 val);
BOOL scr_valget(int num, SINT32 *val);
BOOL scr_flagop(int num, BYTE method);
BOOL scr_flagget(int num, BYTE *flag);
BOOL scr_flagget8(int num, BYTE *flag);

SINT32 scr_cmdval(UINT32 val);

BOOL scr_getbyte(SCR_OPE *op, BYTE *val);
BOOL scr_getword(SCR_OPE *op, UINT16 *val);
BOOL scr_getdword(SCR_OPE *op, UINT32 *val);
BOOL scr_getval(SCR_OPE *op, SINT32 *val);
BOOL scr_getpt(SCR_OPE *op, POINT_T *pt);
BOOL scr_getrect(SCR_OPE *op, RECT_U *rect);
BOOL scr_getlabel(SCR_OPE *op, char *label, int size);
int scr_getmsg(SCR_OPE *op, char *string, int len);

BOOL scr_scriptcall(const char *label);
BOOL src_scriptret(void);
BOOL scr_jump(int ptr);
BOOL scr_call(int ptr);
BOOL scr_ret(void);

void scr_exaclear(void);
void scr_kidclear(void);

BOOL scr_init(void);
void scr_term(void);
BOOL scr_restart(const char *label);

