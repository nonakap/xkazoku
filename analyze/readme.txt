
�� analyze�̎g����

�@analyze [-lcdw] SUF [filename]

�@�@-l �g�p�R�}���h�ꗗ���o���܂��B
�@�@-c �R�}���h���`�F�b�N���܂�
�@�@-d �X�N���v�g���_���v���܂�
    -s �Z�[�u���`�F�b�N���܂��B(�Q�ӏ��ȏ�ɃZ�[�u���K�v)
�@�@-w �x�����x����ݒ肵�܂�
�@�@�@-w0 ���g�p�R�}���h�E���g�p�p�����[�^�����o���܂��B
�@�@�@-w1 ��ɉ����� xkazoku�Ŗ��g�p�̃p�����[�^�����o���܂�
�@�@�@-w2 ��ɉ����� xkazoku�ňꕔ���g�p�̃p�����[�^�����o���܂�

�@�@�I�v�V�������w�肳��ĂȂ��ꍇ�� -c �ƌ��Ȃ��܂��B
�@�@filename���w�肳��Ă��Ȃ��ꍇ�� ���ׂẴX�N���v�g���������܂��B
�@�@-d�w��� filename���K�v�ł��B


�@��: analyze -l "c:\program files\do\kazoku\kazoku.suf"
�@�@�@�@�Ƒ��v��̂��ׂẴX�N���v�g�̎g�p�R�}���h�ꗗ��\�����܂�

�@�@�@analyze -c "c:\program files\do\kazoku\kazoku.suf"
�@�@�@�@�Ƒ��v��̂��ׂẴX�N���v�g�̃R�}���h���`�F�b�N���܂�

�@�@�@analyze -c -w2 "c:\program files\do\kazoku\kazoku.suf" start
�@�@�@�@�Ƒ��v��� START.ISF�̃R�}���h���x�����x��2�Ń`�F�b�N���܂�

�@�@�@analyze -d "c:\program files\do\kazoku\kazoku.suf" start
�@�@�@�@�Ƒ��v��� START.ISF�X�N���v�g���_���v���܂�

�@�@�@analyze -s "c:\program files\do\kazoku\kazoku.suf"
�@�@�@�@�Ƒ��v��� �Z�[�u�t�@�C�����`�F�b�N���܂�

�@win32�ŃR�}���h�v�����v�g������Ȃ����Đl�� analyze.exe�� suf�t�@�C����
�@�h���b�v���ĉ������B





�� analyze�̊Ԉ�����g����

�@analyze packfile filename
�@�@packfile���� filename�����o���܂�

�@��: analyze -d "c:\program files\do\kazoku\GGD" title.gg0
�@�@�@�@�Ƒ��v���GGD�t�@�C������ TITLE.GG0�����o���܂��B

�@�ł����Awin32���Ȃ� susie���g�����ق��������ł��A�����B





�� �Q�[���̑Ή����@

�@�������B
�@�@1. ����
�@�@2. gamedef.txt�ɃG���g�����쐬���܂��B
�@�@3. analyze�����s���܂��B
�@�@4. �Z�[�u�f�[�^�̌݊��������܂��B
�@�@5. ���s�ƍׂ�������
�@�@6. �t�^:�ݒ�L�[





�@1. ����
�@�@�Q�[�����C���X�g�[�����A�Q�ӏ��ȏ�ɃZ�[�u���܂��B
�@�@(���̕��������̓I���W�i���ł����삷��Windows���K�v�ł��B)





�@2. gamedef.txt�ɃG���g�����쐬���܂��B

�@�@�Q�[�����C���X�g�[�������t�H���_�� *.suf���J���A
�@�@���[�J�[���ƃQ�[���L�[���`�F�b�N���܂��B


�@�@Windows�̏ꍇ analyze.exe�Ɠ����t�H���_��
�@�@X11�̏ꍇ �ړI��*.suf�Ɠ����f�B���N�g����
�@�@gamedef.txt�Ƃ����t�@�C�����̃e�L�X�g�t�@�C�����쐬���A
�@�@�ȉ��̏����ŉ��̐ݒ�������܂��B
�@�@(�L�[�ɂ� ASCII�����g��Ȃ��̂� �Q�[������ShiftJIS��EUC�̂ǂ�����ʂ锤)


�@�@gamedef.txt ---------------------------------------------------

�@�@[(�Q�[����)]
�@�@company = (���[�J�[��)
�@�@key = (�Q�[���L�[��)
�@�@ver = 
�@�@type = 

    ---------------------------------------------------------------

�@��: MOEKKO.SUF
�@�@�@> COMPANY=ZACKZACK
�@�@�@> KEY=MOEKKO

�@�@�@�������ꍇ

�@�@�@�@[�G���b���i�[�X]
�@�@�@�@company = ZACKZACK
�@�@�@�@key = MOEKKO
�@�@�@�@ver = 
�@�@�@�@type = 

�@�@�@�ƁA�Ȃ�܂��B





�@3. analyze�����s���܂��B

�@�@�ړI��*.suf�t�@�C�����w�肵�� analyze�����s���܂��B

�@�@USAGE: analyze [complete SUF path]

�@�@�G���[���\�����ꂽ�@�ȉ��̃��[���ɏ]�� gamedef.txt�̐ݒ��ύX����
�@�@�ēx���s���܂��B
�@�@�o�[�W�������ɂ��Ă� �{�h�L�������g�̍Ō�̏͂��Q�l�ɂ��ĉ������B
�@�@�ʏ�A�������̋߂�����I�����Ă����Ɨǂ��ł��傤�B


�@�@�G���[����
�@�@�@warning error : xkazoku�ŋ��s���܂��B
�@�@�@�@�@�@�@�@�@�@�@�e�L�X�g�A�t�F�[�h�A�E�g�̎��Ԏw�蓙�̌��ʂ������܂��B
�@�@�@�@�@�@�@�@�@�@�@�ꕔ�R�}���h�œ��͂Ő���������Ԃ��Ȃ��� �p���s�\��
�@�@�@�@�@�@�@�@�@�@�@�Ȃ�ꍇ������܂��B

�@�@�@severe error  : xkazoku�ŏ����o���܂���B
�@�@�@�@�@�@�@�@�@�@�@�������_�ŃG���[���o�ďI�����Ă��܂��܂��B

�@�@�΍�
�@�@�@error: wrong parameter
�@�@�@�@�w��o�[�W������ �R�}���h�̃p�����[�^��������܂���B
�@�@�@�@EXEVER�̎w��������ĉ������B

�@�@�@error: unsupport command xx
�@�@�@�@�w��o�[�W������ �Ή����Ă��Ȃ��R�}���h���g�p���܂����B
�@�@�@�@EXEVER�̎w����グ�ĉ������B
�@�@�@�@����ł��������Ȃ��ꍇ�͕񍐂����肢���܂��B

�@�@�@warning: wrong delimiter
�@�@�@�@�w��o�[�W������ �R�}���h�ɏ������Ȃ��p�����[�^�����݂��܂��B
�@�@�@�@EXEVER�̎w����グ�ĉ������B
�@�@�@�@�� �P�ɃS�~�������Ă邾���̏ꍇ������܂��B

�@�@�@warning: unsupport textcmd xx
�@�@�@�@�e�L�X�g�ɔ��p���b�Z�[�W���g�p�̃Q�[���̏ꍇ�ɔ������܂��B
�@�@�@�@GAME_TEXTASCII��t���ĉ������B


�@�@��: 
�@�@�@ver = EXEVER_PLANET
�@�@�@type = 

�@�@�@> D:\xkazoku>analyze e:\moekko\moekko.suf
�@�@�@> COMPANY: ZACKZACK
�@�@�@�@�@�@�F
�@�@�@> TURNCHCK.ISF
�@�@�@> warning: wrong delimiter
�@�@�@> 0014e: 55 - d0 07 00 00 00 00 00 01    (�����Ղ�ƃp�����[�^�����Ⴄ
�@�@�@>
�@�@�@>  3114 warning errors
�@�@�@>     1  severe errors


�@�@�@ver = EXEVER_DM
�@�@�@type = 

�@�@�@> D:\xkazoku>analyze e:\moekko\moekko.suf
�@�@�@> COMPANY: ZACKZACK
�@�@�@> TITLE: uGeaabu?aiu[aX
�@�@�@> KEY: MOEKKO
�@�@�@> EV001.ISF
�@�@�@> warning: unsupport textcmd 83          (�����p���b�Z�[�W���g�p)
�@�@�@> 01e91: 2b - 00 ff 83 76 83 89 5c 00 83 68 3f 1a 54 3d 90 6c
�@�@�@>           - 8a d4 36 3e 39 16 37 16 17 22 37 24 03 00 00
�@�@�@�@�@�@�F
�@�@�@> TURNCHCK.ISF
�@�@�@>    29 warning errors
�@�@�@>     1  severe errors


�@�@�@ver = EXEVER_DM
�@�@�@type = GAME_TEXTASCII

�@�@�@> D:\xkazoku>analyze e:\moekko\moekko.suf
�@�@�@> COMPANY: ZACKZACK
�@�@�@�@�@�F
�@�@�@> LOADSYS.ISF
�@�@�@> warning: unsupport ef 6                (�G���b���i�[�X�g���R�}���h)
�@�@�@> 00089: ef - 06 10 00 00 00 00 00 00 00 00 00 00 00 10 00 00
�@�@�@>           - 00 00 00 00 00
�@�@�@�@�@�F
�@�@�@> TURNCHCK.ISF
�@�@�@>     1 warning errors
�@�@�@>     0  severe errors


�@�@�@ver = EXEVER_MOEKKO
�@�@�@type = GAME_TEXTASCII

�@�@�@> D:\xkazoku>analyze e:\moekko\moekko.suf
�@�@�@> COMPANY: ZACKZACK
�@�@�@�@�@�F
�@�@�@> TURNCHCK.ISF
�@�@�@>     0 warning errors
�@�@�@>     0  severe errors





�@4. �Z�[�u�f�[�^�̌݊��������܂��B

�@�@-s �I�v�V�����t���� analyze�����s���A�t���O��ݒ肵�܂��B
�@�@EXEVER�̐ݒ�͎g�p���Ă��܂���̂� 3.������ɐݒ肵�Ă��ǂ��ł��傤�B

�@�@��:
�@�@�@> D:\xkazoku>analyze -s e:\moekko\moekko.suf
�@�@�@> COMPANY: ZACKZACK
�@�@�@> TITLE: uGeaabu?aiu[aX
�@�@�@> KEY: MOEKKO
�@�@�@> use extend save commands
�@�@�@> HLN: value flag 600 (2404bytes)
�@�@�@> FLN: bit flag 701 (88bytes)
�@�@�@> EXA: bit flag 501 (64bytes)
�@�@�@> EXA: value flag 501 (2008bytes)
�@�@�@> KIDFN: kid flag 30000
�@�@�@> load e:/moekko/MOEKKO.sav
�@�@�@> savefile version.2 (later EXEVER_DM)                             (��)
�@�@�@> header files 53 / size 6480bytes
�@�@�@> game saves 50, set configure GAME_SAVEMAX50                      (��)
�@�@�@> analyze game save 1 / filepos 8560 / size 23412bytes
�@�@�@> date: 2003/ 4/26 02:11:12
�@�@�@> use bmp: set configure GAME_SAVEGRPH                             (��)
�@�@�@> bmp size 96x72 - free format
�@�@�@> use comment: set configure GAME_SAVECOM - type:old or save ver2  (��)

�@�@�@����ǉ������ (EXEVER_DM < EXEVER_MOEKKO �Ȃ̂� EXEVER�͕ύX�Ȃ�)

�@�@�@ver = EXEVER_MOEKKO
�@�@�@type = GAME_TEXTASCII + GAME_SAVEMAX50 + GAME_TEXTASCII +
�@�@�@�@�@�@ GAME_SAVEGRPH + GAME_SAVECOM

�@�@�@�� �Ȃ�܂��B





�@5. ���s�ƍׂ�������

�@�@�I���W�i���Ɣ�r���� �ȉ��̐ݒ���������܂��B

�@�@�@GAME_VOICE      - �I�v�V������ �u�����v�̑I������

�@�@�@GAME_VOICEONLY  - �I�v�V������ �u�����̂݁v�̑I������ (GAME_VOICE���ݒ�)

�@�@�@GAME_SELECTEX   - �I�𒆂� �w��͈͈ȊO�Ƀ}�E�X���ړ������ꍇ�ɂ�
�@�@�@�@�@�@�@�@�@�@�@�@�t�H�[�J�X��\�������܂��B
�@�@�@�@�@�@�@�@�@�@�@�@(�Ƒ��v��^�C�v)

�@�@�@GAME_TEXTLAYER  - �G�t�F�N�g���Ƀe�L�X�g���C���[���`�悵�܂��B
                        (���p�[�v��, ���܃^�C�v)

�@�@�@GAME_CMDWINNOBG - �I�����̃o�b�N�O���E���h��\�������Ȃ��B
                        (���܃^�C�v)

�@�@�@������ �P�ɕ\�����ꕔ�ύX�����邽�߂̕��� �ݒ肵�Ȃ������Ƃ��Ă�
�@�@�@�Q�[����{�����ɂ͊֌W����܂���B

�@�@�@GAME_VOICE �� GAME_VOICEONLY�� �P�� EXE�̃f�t�H���g�l�̎w��ŁA
�@�@�@���ۂɂ� �X�N���v�g��ŕύX�ł��锤�Ȃ̂ł����A�Ӑ}�I�ɕύX���Ă�
�@�@�@�Q�[���͌������Ƃ͂���܂���c�B(�S�ăf�t�H���g�w��̂܂��)


�@�@�Ō�ɍ쐬���� gamedef.txt��
�@�@�@WinCE�̏ꍇ akira.exe�Ɠ����t�H���_��
�@�@�@MacOS�̏ꍇ xkazoku�Ɠ����t�H���_��
�@�@�@X11�̏ꍇ �ړI��*.suf�Ɠ����f�B���N�g����
�@�@�R�s�[����� �p�b�`���L���ɂȂ�܂��B





�@6. �t�^:�ݒ�L�[

�@�@ver = EXEVER_KANA         99-06-16 ����
          EXEVER_MYU          99-07-16 Purple
          EXEVER_TSUKU        00-02-15 �s�����Ă������Ⴄ
          EXEVER_AMEIRO       00-04-18 ���߂���̋G��
          EXEVER_TEA2         00-08-10 ���E��E���E���Q
          EXEVER_PLANET       00-11-27 ����Ղ�˂���
          EXEVER_PLANDVD      01-01-31 ����Ղ�˂���DVD
          EXEVER_NURSE        01-06-01 �v���C�x�[�g�i�[�X
          EXEVER_KONYA        01-07-15 ���ɍ��遙��ɗ���
          EXEVER_VECHO        01-08-29 ���F���x�b�g�G�R�[
          EXEVER_CRES         01-09-11 �N���V�F���h
          EXEVER_KAZOKU       01-10-24 �Ƒ��v��
          EXEVER_BLEED        02-05-10 �o����
          EXEVER_OSHIETE      02-08-19 �����Ă������Ⴄ
          EXEVER_SISKON       02-09-02 ��������
          EXEVER_KONYA2       02-10-03 ���ɍ��遙��ɗ��ĂQ
          EXEVER_KAZOKUK      02-11-27 �Ƒ��v�� �J��
          EXEVER_HEART        03-01-13 �͂���de���[�����C�g
          EXEVER_SHISYO       03-01-16 �i������Ƃ������偙
          EXEVER_DM           03-03-11 �h�[�^�[���[�J�[
          EXEVER_MOEKKO       03-04-04 �G���b���i�[�X
          EXEVER_AOI          03-05-07 ����
          EXEVER_KAMOOK       03-05-30 �Ƒ��v��`�J�{�`
          EXEVER_RESTORE      03-06-11 �ꂷ�Ƃ�


�@�@type = (+ ��؂�ŕ����w��\)
          GAME_VOICE          ���H�C�X�̃T�|�[�g
          GAME_VOICEONLY      ���H�C�X�݂̂̃T�|�[�g
          GAME_HAVEALPHA      �e�L�X�g���ϒʉߏ����T�|�[�g
          GAME_TEXTASCII      ���p�e�L�X�g������
          GAME_DRS            DRS system
          GAME_SVGA           800x600
          GAME_NOKID          ���ǃt���O�Ȃ�
          GAME_SAVEMAX9       �Z�[�u�� 9
          GAME_SAVEMAX27      �Z�[�u�� 27
          GAME_SAVEMAX30      �Z�[�u�� 30
          GAME_SAVEMAX50      �Z�[�u�� 50
          GAME_SAVEGRPH       �O���t�B�b�N���܂�
          GAME_SAVECOM        �R�����g���܂�
          GAME_SAVESYS        �V�X�e���̈�L��
          GAME_SELECTEX       �I���t�H�[�J�X���E���Ȃ�
          GAME_TEXTLAYER      �e�L�X�g���C���[����
          GAME_CMDWINNOBG     �I�����o�b�N�O���E���h�Ȃ�

�@�@type�w��ɂ��Ă� �����s�ōs�Ȃ��܂��B
�@�@�Q�s�ڈȍ~�� + �ŏ��̃I�v�V�����̑O�ɂ�t���ĉ������B

�@�@�@type = GAME_TEXTASCII + GAME_SAVEMAX50 + GAME_TEXTASCII
�@�@�@type = + GAME_SAVEGRPH + GAME_SAVECOM

