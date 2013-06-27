
typedef struct {
	BYTE	*ptr;
	int		size;
} _SAVEDATA, *SAVEDATA;

typedef struct {
	void		*fh;
	UINT		fsize;
	int			files;						// トータル
	int			gamesaves;					// ゲームセーブ数
	int			sysflag;					// システムフラグの場所
	int			exaflag;					// EXAフラグの格納場所
	int			exavalue;					// EXA変数の格納場所
	int			kid;						// 既読フラグの格納場所
	BYTE		header[33][4];				// <- これ可変にしたい。
} SAVE_T, *SAVEHDL;

typedef struct {
	int			year;
	int			month;
	int			day;
	int			hour;
	int			min;
	int			sec;
} SAVEDATE_T, *SAVEDATE;

typedef struct {
	SAVEDATE_T	date;
	void		*preview;
	char		comment[104];
} SAVEINF_T, *SAVEINF;


#ifdef __cplusplus
extern "C" {
#endif

SAVEHDL savefile_open(BOOL create);
SAVEHDL savefile_create(void);
void savefile_close(SAVEHDL hdl);

BOOL savefile_exist(SAVEHDL hdl, int num);
int savefile_getnewdate(SAVEHDL hdl);
BOOL savefile_readinf(SAVEHDL hdl, int num, SAVEINF inf,
													int width, int height);
BOOL savefile_readgame(SAVEHDL hdl, int num);

BOOL savefile_readsysflag(SAVEHDL hdl, UINT pos, UINT size);
BOOL savefile_readexaflag(SAVEHDL hdl, void *val);
BOOL savefile_readexaval(SAVEHDL hdl, void *val);
BOOL savefile_readkid(SAVEHDL hdl, void *val);

BOOL savefile_writegame(SAVEHDL hdl, int num, int vnum);

BOOL savefile_writesysflag(SAVEHDL hdl, UINT pos, UINT size);
BOOL savefile_writeexaflag(SAVEHDL hdl, void *val);
BOOL savefile_writeexaval(SAVEHDL hdl, void *val);
BOOL savefile_writekid(SAVEHDL hdl, void *val);

#ifdef __cplusplus
}
#endif

