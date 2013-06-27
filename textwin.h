
enum {
	TEXTCTRL_BOLD		= 0x0001,
	TEXTCTRL_ITALIC		= 0x0002,
	TEXTCTRL_FONTMASK	= 0x0003,

	TEXTCTRL_ASCII		= 0x0100,
//	TEXTCTRL_SHADOW		= 0x0200,
	TEXTCTRL_CLIP		= 0x0400,
	TEXTCTRL_READY		= 0x0800
};

typedef struct {
	VRAMHDL		vram;
	UINT		fonttype;
	void		*font;
	int			fontsize;
	UINT32		fontcolor[3];
	int			intext;
	int			tx;
	int			ty;
	RECT_T		clip;
	UNIRECT		drawrect;
} TEXTCTRL_T, *TEXTCTRL;


enum {
	TEXTWIN_CMD			= 0x0001,
	TEXTWIN_CMDFRAME	= 0x0002,
	TEXTWIN_TEXT		= 0x0004,
	TEXTWIN_TEXTHIDE	= 0x0008,

	TEXTWIN_CMDRECT		= 0x0010,
	TEXTWIN_CMDCAP		= 0x0020,
	TEXTWIN_CMDCAPEX	= 0x0040,

	TEXTWIN_WINBYCMD	= 0x0100,
	TEXTWIN_WINBYTEXT	= 0x0200,

	TEXTWIN_WINBYALL	= TEXTWIN_WINBYCMD | TEXTWIN_WINBYTEXT
};

typedef struct {
	SINT32	num;
	POINT_T	pt;
} CMD_T, *CMDITEM;

typedef struct {
	RECT_T	rct;
	int		type;
	int		x;
	int		y;
	char	str[GAMECORE_CHOICELEN];
} CHO_T, *CHOICE;

typedef struct {
	UINT		flag;
	VRAMHDL		frame;

	TEXTCTRL_T	textctrl;
	SCRN_T		scrn;
	SCRN_T		txtscrn;
	SCRN_T		hisscrn;

	VRAMHDL		cmdvram;
	VRAMHDL		cmdframe;
	VRAMHDL		cmdicon;
	int			iconwidth;
	SCRN_T		cmdscrn;
	int			cmdtype;
	int			cmdmax;
	int			cmdfocus;
	int			cmdret;
	UINT32		chocolor[6];

	char		chrname[GAMECORE_MAXNAME][GAMECORE_NAMELEN];

	CMD_T		cmd[GAMECORE_MAXCMDS];
	CHO_T		cho[GAMECORE_MAXCMDS];
} TEXTWIN_T, *TEXTWIN;

#ifdef __cplusplus
extern "C" {
#endif

void textctrl_init(TEXTCTRL textctrl, BOOL ascii);
void textctrl_setsize(TEXTCTRL textctrl, int size);
void textctrl_settype(TEXTCTRL textctrl, BYTE type);
void textctrl_trash(TEXTCTRL textctrl);
void textctrl_renewal(TEXTCTRL textctrl);


TEXTWIN textwin_getwin(int num);
void textwin_create(void);
void textwin_destroy(void);
void textwin_setpos(int num, const SCRN_T *scrn, const SCRN_T *clip);
void textwin_open(int num);
void textwin_clear(int num);
void textwin_close(int num);
void textwin_setframe(int num, const char *label);

void textwin_cmdopen(int num, int cmds, int type);
void textwin_cmdclose(int num);

void textwin_setcmdframe(int num, const char *label);
void textwin_setcmdicon(int num, int width, const char *label);

#ifdef __cplusplus
}
#endif

