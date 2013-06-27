
typedef struct {
	BYTE	*ptr;
	int		size;
} _SAVEDATA, *SAVEDATA;

typedef struct {
	void		*fh;
	UINT		fsize;
	int			files;						// �ȡ�����
	int			gamesaves;					// �����ॻ���ֿ�
	int			sysflag;					// �����ƥ�ե饰�ξ��
	int			exaflag;					// EXA�ե饰�γ�Ǽ���
	int			exavalue;					// EXA�ѿ��γ�Ǽ���
	int			kid;						// ���ɥե饰�γ�Ǽ���
	BYTE		header[33][4];				// <- ������Ѥˤ�������
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

