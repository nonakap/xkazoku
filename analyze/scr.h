
enum {
	SCRIPT_ERR	= 0,
	SCRIPT_ISF	= 1,
	SCRIPT_DRS	= 2					// digital romance system
};

typedef struct {
	BYTE	*scr;					// script
	UINT	scrsize;

	BYTE	*jmptbl;				// jump table
	UINT	jmptbls;

	BYTE	*calltbl;				// DRS call table
	UINT	calltbls;

	BYTE	*unknown2;
	UINT	unknown2size;

	UINT	type;
	UINT	rev;

	ARCHDL	ah;
} _SCRHDL, *SCRHDL;

typedef struct {
	UINT	cmd;
	UINT	pos;
	BYTE	*ptr;
	int		remain;
} _SCROPE, *SCROPE;


#ifdef __cplusplus
extern "C" {
#endif

SCRHDL scr_create(ARCFH afh, const char *filename);
void scr_destroy(SCRHDL hdl);

void scr_enum(SCRHDL hdl, void *arg,
						int (*proc)(SCRHDL hdl, SCROPE ope, void *arg));

#ifdef __cplusplus
}
#endif

