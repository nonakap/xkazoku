
// #define	GAMEMSG_MSGWAIT		0x083a		// ��å�����������������

// #define	GAMEMSG_SAVE		0x00e5		// ��������
// #define	GAMEMSG_LOAD		0x01e5		// ������

// #define	GAMEMSG_MOVIE		0x00f3		// �ࡼ�ӡ�����
// #define	GAMEMSG_ENABLEPOPUP	0x00f4		// �ݥåץ��åץ�˥塼����
// #define	GAMEMSG_ENABLESAVE	0x00f5		// �����ֲ���

// #define	GAMEMSG_EFFECT		0x0100		// ���ե������� CB


#ifdef __cplusplus
extern "C" {
#endif

int gamemsg_send(UINT msg, long param);

#ifdef __cplusplus
}
#endif

