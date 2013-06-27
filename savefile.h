
typedef struct {
	int		year;
	int		month;
	int		day;
	int		hour;
	int		min;
	int		sec;
} _SAVEDATE, *SAVEDATE;

typedef struct {
	_SAVEDATE	date;
	void		*preview;
	char		comment[104];
} _SAVEINF, *SAVEINF;

struct _savehdl;
typedef	struct _savehdl		_SAVEHDL;
typedef	struct _savehdl		*SAVEHDL;

struct _savehdl {
	void	(*close)(SAVEHDL hdl);
	BOOL	(*exist)(SAVEHDL hdl, int num);
	int		(*getnewdate)(SAVEHDL hdl);
	BOOL	(*readinf)(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height);
	BOOL	(*readgame)(SAVEHDL hdl, int num);
	BOOL	(*readflags)(SAVEHDL hdl, int num, UINT reg, UINT pos, UINT cnt);
	BOOL	(*readsysflag)(SAVEHDL hdl, UINT pos, UINT size);
	BOOL	(*readsysflagex)(SAVEHDL hdl, UINT pos, UINT size);
	BOOL	(*readexaflag)(SAVEHDL hdl, void *val);
	BOOL	(*readexaval)(SAVEHDL hdl, void *val);
	BOOL	(*readkid)(SAVEHDL hdl, void *val);

	BOOL	(*writegame)(SAVEHDL hdl, int num, int vnum);
	BOOL	(*writesysflag)(SAVEHDL hdl, UINT pos, UINT size);
	BOOL	(*writeexaflag)(SAVEHDL hdl, void *val);
	BOOL	(*writeexaval)(SAVEHDL hdl, void *val);
	BOOL	(*writekid)(SAVEHDL hdl, void *val);
};


#ifdef __cplusplus
extern "C" {
#endif

int savefile_getsaves(void);
void savefile_cnvdate(SAVEDATE sd, const void *st);
int savefile_cmpdate(const SAVEDATE sd1, const SAVEDATE sd2);

SAVEHDL savefile_open(BOOL create);

void savenone_close(SAVEHDL hdl);
BOOL savenone_exist(SAVEHDL hdl, int num);
int savenone_getnewdate(SAVEHDL hdl);
BOOL savenone_readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height);
BOOL savenone_readgame(SAVEHDL hdl, int num);
BOOL savenone_readflags(SAVEHDL hdl, int num, UINT reg, UINT pos, UINT cnt);
BOOL savenone_writegame(SAVEHDL hdl, int num, int vnum);
BOOL savenone_sys(SAVEHDL hdl, UINT pos, UINT size);
BOOL savenone_exa(SAVEHDL hdl, void *val);

#ifdef __cplusplus
}
#endif

