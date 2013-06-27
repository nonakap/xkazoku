
typedef struct {
const BYTE	*ptr;
	int		remain;

	UINT	cmd;			// どーせスタックにあるだろうから
	UINT	len;			// いちおー引き連れておく
	UINT	pos;
	UINT	size;
} SCR_OPE;


BOOL scr_valset(UINT num, SINT32 val);
BOOL scr_valget(UINT num, SINT32 *val);
BOOL scr_flagop(UINT num, BYTE method);
BOOL scr_flagget(UINT num, BYTE *flag);
BOOL scr_flagget8(UINT num, BYTE *flag);

SINT32 scr_cmdval(UINT32 val);

BOOL scr_getbyte(SCR_OPE *op, BYTE *val);
BOOL scr_getword(SCR_OPE *op, UINT16 *val);
BOOL scr_getdword(SCR_OPE *op, UINT32 *val);
BOOL scr_getval(SCR_OPE *op, SINT32 *val);
BOOL scr_getpt(SCR_OPE *op, POINT_T *pt);
BOOL scr_getrect(SCR_OPE *op, RECT_U *rect);
BOOL scr_getlabel(SCR_OPE *op, char *label, int size);

BOOL scr_getcurscr(char *label, int leng);
BOOL scr_scriptcall(const char *label);
BOOL src_scriptret(void);
BOOL scr_jump(int ptr);
BOOL scr_call(int ptr);
BOOL scr_ret(void);

BOOL scr_setdrspos(UINT16 pnum);

void scr_exaclear(void);
void scr_kidclear(void);

BOOL scr_init(UINT bits, UINT values);
void scr_term(void);
BOOL scr_restart(const char *label);

