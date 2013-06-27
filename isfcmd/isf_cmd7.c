// -------------------------------------------------------------
//
//	ISF script commands
//
//		(c)2001-02, Nonaka.K / aw9k-nnk@asahi-net.or.jp
//		(c)2002,    T.Yui    / myu@yui.ne.jp
//
// -------------------------------------------------------------

#include	"compiler.h"
#include	"gamecore.h"
#include	"sound.h"
#include	"arcfile.h"
#include	"sstream.h"
#include	"isf_cmd.h"


// ML : 音楽データのロード・再生 (T.Yui)
int isfcmd_70(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN+1];
	BYTE	cmd;

	if ((scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getbyte(op, &cmd) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("BGM: %-12s", label));

	sndplay_waveset(label, cmd);
	return(GAMEEV_SUCCESS);
}


// MP : 音楽の再生 (T.Yui)
int isfcmd_71(SCR_OPE *op) {

	BYTE	flag;
	SINT32	tick;
	int		exever;

	exever = gamecore.sys.version;

	flag = 0;
	if (exever != EXEVER_TEA2) {
		if (scr_getbyte(op, &flag) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	tick = 0;
	if (exever >= EXEVER_KAZOKU) {
		if (scr_getval(op, &tick) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	sndplay_waveplay(flag, tick);
	return(GAMEEV_SUCCESS);
}


// MF : 音楽のフェードアウト (T.Yui)
int isfcmd_72(SCR_OPE *op) {

	SINT32	tick;

	if (scr_getval(op, &tick) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_wavestop(tick);
	return(GAMEEV_SUCCESS);
}


// MS : 音楽のストップ (T.Yui)
int isfcmd_73(SCR_OPE *op) {

	sndplay_wavestop(0);
	(void)op;
	return(GAMEEV_SUCCESS);
}


// SER : 効果音のロード (T.Yui)		DRS cmd:76
int isfcmd_74(SCR_OPE *op) {

	char			label[ARCFILENAME_LEN+1];
	SINT32			num;
	ARCSTREAMARG	asa;

	if ((scr_getlabel(op, label, sizeof(label)) != SUCCESS) ||
		(scr_getval(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("SE: %-12s (%x)", label, num));

	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		asa.type = ARCTYPE_SE;
		asa.fname = label;
		soundmix_load(SOUNDTRK_SE + num, arcse_ssopen, &asa);
	}
	return(GAMEEV_SUCCESS);
}


// SEP : 効果音の再生 (T.Yui)
int isfcmd_75(SCR_OPE *op) {

	SINT32	cmd;
	SINT32	tick = 0;

	if (scr_getval(op, &cmd) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	if (gamecore.sys.version >= EXEVER_KAZOKU) {
		if (scr_getval(op, &tick) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}

	TRACEOUT(("SE play: %x (%d)", cmd, tick));
	sndplay_seplay(cmd, tick);
	return(GAMEEV_SUCCESS);
}


// SED : 効果音の削除 (T.Yui)										DRS cmd:78
int isfcmd_76(SCR_OPE *op) {

	SINT32	num;

	if (scr_getval(op, &num) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
//	TRACEOUT(("SE stop: %x (%d)", num, 0));
	sndplay_seplay(num & 0x1f, 0);								// akiba
	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		soundmix_unload(SOUNDTRK_SE + num);
	}

	return(GAMEEV_SUCCESS);
}


// PCMON : PCM 音声の再生 Nonaka.K T.Yui
int isfcmd_77(SCR_OPE *op) {

	GAMECFG		gamecfg;
	SNDPLAY		sndplay;
	char		label[ARCFILENAME_LEN+1];
	BYTE		cmd;

	gamecfg = &gamecore.gamecfg;
	sndplay = &gamecore.sndplay;
	if ((gamecfg->skip) || (!gamecfg->voice)) {
		goto cmd77_exit;
	}
	if (!(sndplay->playing & SNDPLAY_PCMLOAD)) {
		if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		sndplay_voiceset(label);
	}
	scr_getbyte(op, &cmd);
	sndplay_voiceplay();

cmd77_exit:
	sndplay_voicereset();
	return(GAMEEV_SUCCESS);
}


// PCML : PCMのロード Nonaka.K T.Yui
int isfcmd_78(SCR_OPE *op) {

	char	label[ARCFILENAME_LEN+1];

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("VOICE(78): %-12s", label));
	sndplay_voiceset(label);
	return(GAMEEV_SUCCESS);
}


// PCMS : PCMの停止 Nonaka.K T.Yui
int isfcmd_79(SCR_OPE *op) {

	soundmix_stop(SOUNDTRK_VOICE, 0);
	soundmix_unload(SOUNDTRK_VOICE);
	gamecore.sndplay.playing &= ~SNDPLAY_PCMLOAD;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// PCMEND : PCM 音声の停止待機 (Nonaka.K)
int isfcmd_7a(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_WAITPCMEND);
}


// SES : SES 効果音の停止 (T.Yui)
int isfcmd_7b(SCR_OPE *op) {

	SINT32	num;
	SINT32	tick;

	if (gamecore.sys.version == EXEVER_TSUKU) {			// テスト
		return(isfcmd_90(op));
	}

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_seplay(num, tick);
	return(GAMEEV_SUCCESS);
}


// BGMGETPOS : 音楽の再生位置取得 (T.Yui)
static void setplaypos(int ch, BYTE kind, UINT16 num) {

	if (!kind) {
		scr_flagop(num, (BYTE)((soundmix_isplaying(ch))?1:0));
	}
	else {
		scr_valset(num, soundmix_getpos(ch));
	}
}

int isfcmd_7c(SCR_OPE *op) {

	BYTE	kind;
	UINT16	num;

	if (gamecore.sys.version == EXEVER_TSUKU) {			// テスト
		return(isfcmd_91(op));
	}

	if ((scr_getbyte(op, &kind) != SUCCESS) ||
		(scr_getword(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	setplaypos(SOUNDTRK_SOUND, kind, num);
	return(GAMEEV_SUCCESS);
}


// SEGETPOS : 効果音の再生位置取得 (T.Yui)
int isfcmd_7d(SCR_OPE *op) {

	SINT32	senum;
	BYTE	kind;
	UINT16	num;

	if (gamecore.sys.version == EXEVER_TSUKU) {			// テスト
		return(isfcmd_92(op));
	}

	if ((scr_getval(op, &senum) != SUCCESS) ||
		(scr_getbyte(op, &kind) != SUCCESS) ||
		(scr_getword(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	if ((senum >= 0) && (senum < SOUNDTRK_MAXSE)) {
		setplaypos(SOUNDTRK_SE + senum, kind, num);
	}
	else {
		if (!kind) {
			scr_flagop(num, 0);
		}
		else {
			scr_valset(num, 0);
		}
	}
	return(GAMEEV_SUCCESS);
}


// PCMGETPOS : PCMの再生位置取得 (T.Yui)
int isfcmd_7e(SCR_OPE *op) {

	BYTE	kind;
	UINT16	num;

	if ((scr_getbyte(op, &kind) != SUCCESS) ||
		(scr_getword(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	setplaypos(SOUNDTRK_VOICE, kind, num);
	return(GAMEEV_SUCCESS);
}


// PCMCN : 音声ファイル名のバックアップ (T.Yui)
int isfcmd_7f(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}

