
enum {
	EXE_VER0		= 0,
	EXEVER_KANA,					// 99-06-16 ����
	EXEVER_MYU,						// 99-07-16 Purple
	EXEVER_KANAUS,					// KANA - little sister -

	EXE_VER1		= 100,
	EXEVER_TSUKU,					// 00-02-15 �s�����Ă������Ⴄ
	EXEVER_AMEIRO,					// 00-04-18 ���߂���̋G��
	EXEVER_TEA2,					// 00-08-10 ���E��E���E���Q
	EXEVER_PLANET,					// 00-11-27 ����Ղ�˂���
	EXEVER_PLANDVD,					// 01-01-31 ����Ղ�˂���DVD
	EXEVER_NURSE,					// 01-06-01 �v���C�x�[�g�i�[�X
	EXEVER_KONYA,					// 01-07-15 ���ɍ��遙��ɗ���
	EXEVER_VECHO,					// 01-08-29 ���F���x�b�g�G�R�[
	EXEVER_CRES,					// 01-09-11 �N���V�F���h
	EXEVER_CRESD,					// 03-07-02 �N���V�F���hFV
	EXEVER_KAZOKU,					// 01-10-24 �Ƒ��v��
	EXEVER_BLEED,					// 02-05-10 �o����
	EXEVER_ROSYU2,					// 02-05-10 �I�o�������D��Q
	EXEVER_OSHIETE,					// 02-08-19 �����Ă������Ⴄ
	EXEVER_SISKON,					// 02-09-02 ��������
	EXEVER_KONYA2,					// 02-10-03 ���ɍ��遙��ɗ��ĂQ
	EXEVER_KAZOKUK,					// 02-11-27 �Ƒ��v�� �J��
	EXEVER_HEART,					// 03-01-13 �͂���de���[�����C�g
	EXEVER_SHISYO,					// 03-01-16 �i������Ƃ������偙

	EXE_VER2		= 200,
	EXEVER_HIMITSU,					// 03-02-23 ���̎q�̃q�~�c
	EXEVER_DM,						// 03-03-11 �h�[�^�[���[�J�[
	EXEVER_MOEKKO,					// 03-04-04 �G���b���i�[�X
	EXEVER_AOI,						// 03-05-07 ����
	EXEVER_KAMOOK,					// 03-05-30 �Ƒ��v��`�J�{�`
//	EXE_VER210		= 210,
	EXEVER_RESTORE,					// 03-06-11 �ꂷ�Ƃ�
	EXEVER_AKIBA,					// 03-06-20 �A�L�o�ł���������I
	EXEVER_OSHI_TAI,				// 03-07-12 ���������n�[�����̌���
	EXEVER_KT_TRY					// 03-08-07 ���̐}���ّ̌���
};

enum {
	GAME_VOICE			= 0x00000001,	// ���H�C�X�̃T�|�[�g
	GAME_VOICEONLY		= 0x00000002,	// ���H�C�X�݂̂̃T�|�[�g
	GAME_HAVEALPHA		= 0x00000004,	// �e�L�X�g���ϒʉߏ����T�|�[�g
	GAME_TEXTASCII		= 0x00000008,	// ���p�e�L�X�g������
	GAME_DRS			= 0x00000010,	// DRS system
	GAME_SVGA			= 0x00000020,	// 800x600
	GAME_NOKID			= 0x00000040,	// ���ǃt���O�Ȃ�
	GAME_ENGSTYLE		= 0x00000080,	// �_�~�[

	GAME_SAVEMAX9		= 0x00000100,	// �Z�[�u�� 9
	GAME_SAVEMAX27		= 0x00000200,	// �Z�[�u�� 27
	GAME_SAVEMAX30		= 0x00000300,	// �Z�[�u�� 30
	GAME_SAVEMAX50		= 0x00000400,	// �Z�[�u�� 50
	GAME_SAVEMASK		= 0x00000700,
	GAME_SAVEGRPH		= 0x00000800,	// �O���t�B�b�N���܂�
	GAME_SAVECOM		= 0x00001000,	// �R�����g���܂�
	GAME_SAVESYS		= 0x00002000,	// �V�X�e���̈�L��
	GAME_SAVEAUTOCOM	= 0x00004000,	// �\���e�L�X�g���R�����g�ɂ���
	GAME_SAVEALIAS		= 0x00008000,	// xkazoku.sav �ŃZ�[�u

	// �ȉ� �ׂ��������p (�ʂɖ����Ă��ǂ�)
	GAME_SELECTEX		= 0x00010000,	// �I���t�H�[�J�X���E���Ȃ�
	GAME_TEXTLAYER		= 0x00020000,	// �e�L�X�g���C���[����
	GAME_CMDWINNOBG		= 0x00040000	// �I�����o�b�N�O���E���h�Ȃ�
};


typedef struct {
	char	company[64];
	char	title[64];
	char	key[64];
	char	scriptpath[MAX_PATH];
} SUF_T;

typedef struct {
	UINT	version;
	UINT	type;
	int		width;
	int		height;

	int		bmpwidth;
	int		bmpheight;
	int		defbits;
	int		defflags;
} SYS_T;

#ifdef __cplusplus
extern "C" {
#endif

void gamedef_create(void);
void gamedef_destroy(void);
void gamedef_loadsuf(const char *path, SUF_T *suf, SYS_T *sys);

#ifdef __cplusplus
}
#endif

