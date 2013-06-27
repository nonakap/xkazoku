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
#include	"arcfile.h"
#include	"isf_cmd.h"
#include	"sound.h"
#include	"sstream.h"


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
	if (exever != EXEVER_TEA2DEMO) {
		if (scr_getbyte(op, &flag) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	tick = 0;
	if (exever >= EXEVER_KAZOKUK) {
		if (scr_getval(op, &tick) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
	}
	sndplay_waveplay(flag, tick);
	return(GAMEEV_SUCCESS);
}


// MF : 音楽のフェードアウト
int isfcmd_72(SCR_OPE *op) {

	SINT32	tick;

	if (scr_getval(op, &tick) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_wavestop(tick);
	return(GAMEEV_SUCCESS);
}


// MS : 音楽のストップ
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
		soundmix_load(SOUNDTRK_SE + num, &se_stream, &asa);
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
	sndplay_seplay(num, 0);
	if ((num >= 0) && (num < SOUNDTRK_MAXSE)) {
		soundmix_unload(SOUNDTRK_SE + num);
	}

	return(GAMEEV_SUCCESS);
}


// PCMON : PCM 音声の再生 (T.Yui)
int isfcmd_77(SCR_OPE *op) {

	GAMECFG			gamecfg;
	SNDPLAY			sndplay;
	char			label[ARCFILENAME_LEN+1];
	ARCSTREAMARG	asa;
	BYTE			cmd;

	gamecfg = &gamecore.gamecfg;
	sndplay = &gamecore.sndplay;
	if ((gamecfg->skip) || (!gamecfg->voice)) {
		goto cmd77_exit;
	}
	asa.type = ARCTYPE_VOICE;
	if (sndplay->playing & SNDPLAY_PCMLOAD) {
		asa.fname = sndplay->pcm;
	}
	else {
		if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
			return(GAMEEV_WRONGLENG);
		}
		TRACEOUT(("VOICE(77): %-12s", label));
		asa.fname = label;
	}
	scr_getbyte(op, &cmd);
	soundmix_load(SOUNDTRK_VOICE, &se_stream, &asa);
	soundmix_play(SOUNDTRK_VOICE, 0, 0);

cmd77_exit:
	sndplay->playing &= ~SNDPLAY_PCMLOAD;
	return(GAMEEV_SUCCESS);
}


// PCML : PCMのロード
int isfcmd_78(SCR_OPE *op) {

	char		label[ARCFILENAME_LEN+1];
	SNDPLAY		sndplay;

	if (scr_getlabel(op, label, sizeof(label)) != SUCCESS) {
		return(GAMEEV_WRONGLENG);
	}
	TRACEOUT(("VOICE(78): %-12s", label));
	sndplay = &gamecore.sndplay;
	sndplay->playing |= SNDPLAY_PCMLOAD;
	milstr_ncpy(sndplay->pcm, label, sizeof(sndplay->pcm));

	return(GAMEEV_SUCCESS);
}


// PCMS : PCMの停止
int isfcmd_79(SCR_OPE *op) {

	soundmix_stop(SOUNDTRK_VOICE, 0);
	soundmix_unload(SOUNDTRK_VOICE);
	gamecore.sndplay.playing &= ~SNDPLAY_PCMLOAD;
	(void)op;
	return(GAMEEV_SUCCESS);
}


// PCMEND : PCM 音声の停止待機 (T.Yui)
int isfcmd_7a(SCR_OPE *op) {

	(void)op;
//	soundmix_stop(SOUNDTRK_VOICE, 0);
	return(GAMEEV_WAITPCMEND);
}


// SES : SES 効果音の停止 (T.Yui)
int isfcmd_7b(SCR_OPE *op) {

	SINT32	num;
	SINT32	tick;

	if ((scr_getval(op, &num) != SUCCESS) ||
		(scr_getval(op, &tick) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	sndplay_seplay(num, tick);
	return(GAMEEV_SUCCESS);
}


// PCMGETPOS : PCMの再生位置取得
int isfcmd_7e(SCR_OPE *op) {

	BYTE	kind;
	BYTE	cmd;
	UINT16	num;

	if ((scr_getbyte(op, &kind) != SUCCESS) ||
		(scr_getword(op, &num) != SUCCESS)) {
		return(GAMEEV_WRONGLENG);
	}
	cmd = soundmix_isplaying(SOUNDTRK_VOICE)?1:0;
	if (!kind) {
		scr_flagop(num, cmd);
	}
	else {
		scr_valset(num, cmd);
	}
	return(GAMEEV_SUCCESS);
}


// PCMCN : 音声ファイル名のバックアップ
int isfcmd_7f(SCR_OPE *op) {

	(void)op;
	return(GAMEEV_SUCCESS);
}

