
enum {
	EXE_VER0		= 0,
	EXEVER_KANA,					// 99-06-16 ����
	EXEVER_MYU,						// 99-07-16 Purple
	EXE_VER1		= 100,
	EXEVER_TEA2DEMO,				// 00-08-08 ���󤻤����ǥ�
	EXEVER_PLANET,					// 00-11-27 �����פ�ͤä�
	EXEVER_PLANDVD,					// 01-01-31 �����פ�ͤä�DVD
	EXEVER_NURSE,					// 01-06-01 �ץ饤�١��ȥʡ���
	EXEVER_KONYA,					// 01-07-15 ��˺�����񤤤����
	EXEVER_CRES,					// 01-09-11 ���쥷�����
	EXEVER_KAZOKU,					// 01-10-24 ��²�ײ�
	EXEVER_OSHIETE,					// 02-08-19 �����Ƥ������㤦
	EXEVER_SISKON,					// 02-09-02 ��������
	EXEVER_KONYA2,					// 02-10-03 ��˺�����񤤤���ƣ�
	EXEVER_KAZOKUK,					// 02-11-27 ��²�ײ� �Ȣ
	EXEVER_HEART,					// 03-01-13 �Ϥ���de�롼��ᥤ��
	EXEVER_DM,						// 03-03-11 �ɡ������᡼����
	EXEVER_MOEKKO					// 03-04-04 ˨����̼�ʡ���
};

enum {
	GAME_VOICE			= 0x0001,		// ���������Υ��ݡ���
	GAME_VOICEONLY		= 0x0002,		// ���������ΤߤΥ��ݡ���
	GAME_HAVEALPHA		= 0x0004,		// �ƥ�����������̲�������ݡ���
	GAME_TEXTASCII		= 0x0008,		// Ⱦ�ѥƥ����Ȥ����
	GAME_SVGA			= 0x0010,		// 800x600

	GAME_SAVEMYU		= 0x0100,		// MYU-.EXE ������
	GAME_SAVEMAX27		= 0x0200,		// �����ֿ� 27
	GAME_SAVEMAX30		= 0x0300,		// �����ֿ� 30
	GAME_SAVEMAX50		= 0x0400,		// �����ֿ� 50
	GAME_SAVEMASK		= 0x0700,
	GAME_SAVEGRPH		= 0x0800,		// ����ե��å���ޤ�
	GAME_SAVECOM		= 0x1000,		// �����Ȥ�ޤ�
	GAME_SAVESYS		= 0x2000		// �����ƥ��ΰ�ͭ��
};

