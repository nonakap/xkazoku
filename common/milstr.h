
#ifdef __cplusplus
extern "C" {
#endif

// ��ʸ����ʸ����Ʊ��뤷�����
// ret 0:����
BOOL milstr_cmp(const char *str, const char *cmp);

// ��ʸ����ʸ���� Ʊ��뤷��cmp�Υ̥�ޤ����
// ret 0:����
BOOL milstr_memcmp(const char *str, const char *cmp);

// 0~9, A~Z �Τߤ���ʸ����ʸ����Ʊ��뤷�����
// ret 0:����
BOOL milstr_extendcmp(const char *str, const char *cmp);

// str[pos]���������Х����ܤ��ɤ�����
int milstr_kanji1st(const char *str, int pos);

// str[pos]���������Х����ܤ��ɤ�����
int milstr_kanji2nd(const char *str, int pos);

// maxlenʬ����ʸ����򥳥ԡ�
void milstr_ncpy(char *dst, const char *src, int maxlen);

// maxlenʬ����ʸ����򥭥�å�
void milstr_ncat(char *dst, const char *src, int maxlen);

// ʸ���󤫤�ARG�μ���
int milstr_getarg(char *str, char *arg[], int maxarg);

// HEX2INT
long milstr_solveHEX(const char *str);

// STR2INT
long milstr_solveINT(const char *str);

// �ѥ�����ʸ�������
void milstr_mkstr255(BYTE *dst, const char *src);

#ifdef __cplusplus
}
#endif

