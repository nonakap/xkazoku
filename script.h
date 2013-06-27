
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
	char	name[16];
} SCRIPT_T, *SCRIPT_HDL;

typedef struct {
	SCRIPT_HDL	script;
const BYTE		*ptr;
	UINT		pos;
} SCENE_T, *SCENE_HDL;


#ifdef __cplusplus
extern "C" {
#endif

SCRIPT_HDL script_create(const char *fname);
void script_destroy(SCRIPT_HDL hdl);

int script_dispatch(void);

#ifdef __cplusplus
}
#endif

