
// #define	GAMEMSG_MSGWAIT		0x083a		// ���b�Z�[�W�E�F�C�g�ݒ�

// #define	GAMEMSG_SAVE		0x00e5		// �Z�[�u��
// #define	GAMEMSG_LOAD		0x01e5		// ���[�h��

// #define	GAMEMSG_MOVIE		0x00f3		// ���[�r�[�Đ�
// #define	GAMEMSG_ENABLEPOPUP	0x00f4		// �|�b�v�A�b�v���j���[��
// #define	GAMEMSG_ENABLESAVE	0x00f5		// �Z�[�u��

// #define	GAMEMSG_EFFECT		0x0100		// �G�t�F�N�g�� CB


#ifdef __cplusplus
extern "C" {
#endif

int gamemsg_send(UINT msg, long param);

#ifdef __cplusplus
}
#endif

