# makefile for SL-Zaurus
#   gcc version 2.95.2


# for arm-linux
# SDL_CONFIG = /opt/Embedix/tools/arm-linux/bin/sdl-config

SDL_CONFIG?= sdl-config

CC		= arm-linux-gcc
RM		= rm -f

OBJ		= ../obj
BIN		= ../bin

BASE		= ..
COMMON		= ../common
ISFCMD		= ../isfcmd
MENU		= ../menu
MENUBASE	= ../menubase
SOUND		= ../sound
VERMOUTH	= ../sound/vermouth
TARG		= .
TARG2		= ./qtopia
PATCH		= ./patch

HEADERS		=	$(TARG2)/compiler.h $(BASE)/common.h $(COMMON)/milstr.h \
				$(COMMON)/_memory.h $(COMMON)/profile.h $(COMMON)/variant.h \
				$(COMMON)/rect.h $(COMMON)/lstarray.h $(TARG)/trace.h

BASEINC		=	$(BASE)/gamedef.h $(BASE)/vram.h $(BASE)/vramdraw.h \
				$(BASE)/vrammix.h $(BASE)/script.h $(ISFCMD)/scr_base.h \
				$(BASE)/textwin.h $(BASE)/textdisp.h $(BASE)/event.h \
				$(BASE)/effect.h $(BASE)/gamecfg.h $(BASE)/anime.h \
				$(BASE)/sndplay.h

MENUINC		=	$(MENUBASE)/menudeco.inc $(MENUBASE)/menubase.h \
				$(MENUBASE)/menusys.h $(MENUBASE)/menuvram.h \
				$(MENUBASE)/menumbox.h $(MENUBASE)/menures.h

VERMOUTHINC	=	$(VERMOUTH)/midiout.h $(VERMOUTH)/midimod.h \
				$(VERMOUTH)/midinst.h $(VERMOUTH)/midvoice.h \
				$(VERMOUTH)/midtable.h

COPT	= -c -Wall -pipe -O3 -Wpointer-arith -fno-exceptions -fno-rtti \
			-fsigned-char \
			-I$(TARG) -I$(TARG2) -I$(COMMON) -I$(ISFCMD) -I$(MENU) \
			-I$(MENUBASE) -I$(SOUND) -I$(BASE) -I$(VERMOUTH) \
			`$(SDL_CONFIG) --cflags`
LOPT	=

XKZKOBJ	=	$(OBJ)/milstr.o		$(OBJ)/_memory.o	$(OBJ)/textfile.o	\
			$(OBJ)/profile.o	$(OBJ)/variant.o	$(OBJ)/rect.o		\
			$(OBJ)/lstarray.o	$(OBJ)/resize.o		$(OBJ)/codecnv.o	\
			$(OBJ)/scr_base.o	$(OBJ)/isf_cmd0.o	$(OBJ)/isf_cmd1.o	\
			$(OBJ)/isf_cmd2.o	$(OBJ)/isf_cmd3.o	$(OBJ)/isf_cmd4.o	\
			$(OBJ)/isf_cmd5.o	$(OBJ)/isf_cmd6.o	$(OBJ)/isf_cmd7.o	\
			$(OBJ)/isf_cmd8.o	$(OBJ)/isf_cmd9.o	$(OBJ)/isf_cmda.o	\
			$(OBJ)/isf_cmdb.o	$(OBJ)/isf_cmdd.o	$(OBJ)/isf_cmde.o	\
			$(OBJ)/isf_cmdf.o	$(OBJ)/isf_todo.o						\
			$(OBJ)/drs_cmd0.o	$(OBJ)/drs_cmd1.o	$(OBJ)/drs_cmd2.o	\
			$(OBJ)/drs_cmd3.o	$(OBJ)/drs_cmd4.o	$(OBJ)/drs_cmd5.o	\
			$(OBJ)/drs_cmd7.o	$(OBJ)/drs_cmdf.o						\
			$(OBJ)/sysmenu.o	$(OBJ)/dlgcfg.o		$(OBJ)/dlgsave.o	\
			$(OBJ)/menubase.o	$(OBJ)/menusys.o	$(OBJ)/menudlg.o	\
			$(OBJ)/menumbox.o	$(OBJ)/menuvram.o	$(OBJ)/menures.o	\
			$(OBJ)/sound.o		$(OBJ)/sndwave.o	$(OBJ)/sndmp3.o		\
			$(OBJ)/sndmidi.o	$(OBJ)/sndogg.o		$(OBJ)/sndmix.o		\
			$(OBJ)/sndcsec.o											\
			$(OBJ)/midiout.o	$(OBJ)/midimod.o	$(OBJ)/midinst.o	\
			$(OBJ)/midvoice.o	$(OBJ)/midtable.o						\
			$(OBJ)/akira.o		$(OBJ)/xdraws.o		$(OBJ)/xsound.o		\
			$(OBJ)/dosio.o		$(OBJ)/trace.o		$(OBJ)/cddamng.o	\
			$(OBJ)/fontmng.o	$(OBJ)/inputmng.o	$(OBJ)/moviemng.o	\
			$(OBJ)/scrnmng.o	$(OBJ)/taskmng.o	$(OBJ)/timemng.o	\
			$(OBJ)/gamemsg.o											\
			$(OBJ)/gamecore.o	$(OBJ)/gamedef.o	$(OBJ)/gamecfg.o	\
			$(OBJ)/arcfile.o	$(OBJ)/savefile.o	$(OBJ)/savedrs.o	\
			$(OBJ)/saveisf.o	$(OBJ)/script.o		$(OBJ)/event.o		\
			$(OBJ)/vram.o		$(OBJ)/vramdraw.o	$(OBJ)/vrammix.o	\
			$(OBJ)/textwin.o	$(OBJ)/effect.o		$(OBJ)/textdisp.o	\
			$(OBJ)/anime.o		$(OBJ)/cgload.o		$(OBJ)/bmpdata.o	\
			$(OBJ)/sndplay.o	$(OBJ)/sstream.o
XKZKLIB = -lamethyst -ldamedec \
			-lSDL -lSDL_ttf -lfreetype `$(SDL_CONFIG) --libs`
XKZKBIN	= $(BIN)/xkazoku


all: $(OBJ) $(BIN) $(XKZKBIN)


$(OBJ):
	mkdir $(OBJ)

$(BIN):
	mkdir $(BIN)


$(XKZKBIN): $(XKZKOBJ)
	$(CC) $(XKZKOBJ) $(LOPT) $(XKZKLIB) -o $@



$(OBJ)/milstr.o		:	$(COMMON)/milstr.c $(HEADERS)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/_memory.o	:	$(COMMON)/_memory.c $(HEADERS) \
						$(TARG)/dosio.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/textfile.o	:	$(COMMON)/textfile.c $(HEADERS) \
						$(TARG)/dosio.h \
						$(COMMON)/textfile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/profile.o	:	$(COMMON)/profile.c $(HEADERS) \
						$(TARG)/dosio.h \
						$(COMMON)/textfile.h $(COMMON)/profile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/variant.o	:	$(COMMON)/variant.c $(HEADERS)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/rect.o		:	$(COMMON)/rect.c $(HEADERS)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/lstarray.o	:	$(COMMON)/lstarray.c $(HEADERS)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/resize.o		:	$(COMMON)/resize.c $(HEADERS) $(COMMON)/resize.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/codecnv.o	:	$(COMMON)/codecnv.c $(HEADERS) $(COMMON)/codecnv.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/scr_base.o	:	$(ISFCMD)/scr_base.c $(HEADERS) $(BASEINC) \
						$(BASE)/savefile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd0.o	:	$(ISFCMD)/isf_cmd0.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd1.o	:	$(ISFCMD)/isf_cmd1.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd2.o	:	$(ISFCMD)/isf_cmd2.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd3.o	:	$(ISFCMD)/isf_cmd3.c $(HEADERS) $(BASEINC) \
						$(TARG)/inputmng.h $(BASE)/savefile.h \
						$(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd4.o	:	$(ISFCMD)/isf_cmd4.c $(HEADERS) $(BASEINC) \
						$(BASE)/savefile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd5.o	:	$(ISFCMD)/isf_cmd5.c $(HEADERS) $(BASEINC) \
						$(TARG)/cddamng.h $(SOUND)/sound.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h \
						$(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd6.o	:	$(ISFCMD)/isf_cmd6.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(BASE)/savefile.h \
						$(BASE)/bmpdata.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd7.o	:	$(ISFCMD)/isf_cmd7.c $(HEADERS) $(BASEINC) \
						$(SOUND)/sound.h \
						$(BASE)/arcfile.h $(BASE)/sstream.h \
						$(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd8.o	:	$(ISFCMD)/isf_cmd8.c $(HEADERS) $(BASEINC) \
						$(TARG)/inputmng.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h \
						$(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmd9.o	:	$(ISFCMD)/isf_cmd9.c $(HEADERS) $(BASEINC) \
						$(TARG)/cddamng.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmda.o	:	$(ISFCMD)/isf_cmda.c $(HEADERS) $(BASEINC) \
						$(BASE)/savefile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmdb.o	:	$(ISFCMD)/isf_cmdb.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/inputmng.h \
						$(BASE)/arcfile.h $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmdd.o	:	$(ISFCMD)/isf_cmdd.c $(HEADERS) $(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmde.o	:	$(ISFCMD)/isf_cmde.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/timemng.h $(TARG)/gamemsg.h \
						$(BASE)/arcfile.h $(BASE)/savefile.h \
						$(ISFCMD)/isf_cmd.h \
						$(MENU)/dlgcfg.h $(MENU)/dlgsave.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_cmdf.o	:	$(ISFCMD)/isf_cmdf.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/moviemng.h $(TARG)/gamemsg.h \
						$(BASE)/arcfile.h $(ISFCMD)/isf_cmd.h \
						$(MENU)/dlgcfg.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/isf_todo.o	:	$(ISFCMD)/isf_todo.c $(HEADERS) $(BASEINC) \
						$(ISFCMD)/isf_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd0.o	:	$(ISFCMD)/drs_cmd0.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(BASE)/savefile.h \
						$(BASE)/savedrs.h $(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd1.o	:	$(ISFCMD)/drs_cmd1.c $(HEADERS) $(BASEINC) \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd2.o	:	$(ISFCMD)/drs_cmd2.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd3.o	:	$(ISFCMD)/drs_cmd3.c $(HEADERS) $(BASEINC) \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd4.o	:	$(ISFCMD)/drs_cmd4.c $(HEADERS) $(BASEINC) \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd5.o	:	$(ISFCMD)/drs_cmd5.c $(HEADERS) $(BASEINC) \
						$(BASE)/arcfile.h $(BASE)/cgload.h \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmd7.o	:	$(ISFCMD)/drs_cmd7.c $(HEADERS) $(BASEINC) \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/drs_cmdf.o	:	$(ISFCMD)/drs_cmdf.c $(HEADERS) $(BASEINC) \
						$(TARG)/moviemng.h $(BASE)/arcfile.h \
						$(ISFCMD)/drs_cmd.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/sysmenu.o	:	$(MENU)/sysmenu.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/fontmng.h $(TARG)/scrnmng.h \
						$(TARG)/taskmng.h \
						$(MENU)/sysmenu.res $(MENU)/sysmenu.h \
						$(MENU)/dlgcfg.h $(MENU)/dlgsave.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/dlgcfg.o		:	$(MENU)/dlgcfg.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(SOUND)/sound.h $(BASE)/arcfile.h $(BASE)/sstream.h \
						$(MENU)/sysmenu.res $(MENU)/sysmenu.h $(MENU)/dlgcfg.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/dlgsave.o	:	$(MENU)/dlgsave.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/dosio.h $(TARG)/fontmng.h \
						$(BASE)/savefile.h $(MENU)/sysmenu.res \
						$(MENU)/sysmenu.h $(MENU)/dlgsave.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/menubase.o	:	$(MENUBASE)/menubase.c $(HEADERS) $(MENUINC) \
						$(TARG)/fontmng.h $(TARG)/inputmng.h \
						$(TARG)/scrnmng.h $(TARG)/taskmng.h \
						$(BASE)/vram.h $(BASE)/vramdraw.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/menusys.o	:	$(MENUBASE)/menusys.c $(HEADERS) $(MENUINC) \
						$(TARG)/fontmng.h $(TARG)/inputmng.h \
						$(BASE)/vram.h $(BASE)/vramdraw.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/menudlg.o	:	$(MENUBASE)/menudlg.c $(HEADERS) $(MENUINC) \
						$(TARG)/fontmng.h $(BASE)/vram.h $(BASE)/vramdraw.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/menumbox.o	:	$(MENUBASE)/menumbox.c $(HEADERS) $(MENUINC) \
						$(TARG)/fontmng.h $(BASE)/vram.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/menuvram.o	:	$(MENUBASE)/menuvram.c $(HEADERS) $(MENUINC) \
						$(BASE)/vram.h $(BASE)/vramdraw.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/menures.o	:	$(MENUBASE)/menures.c $(HEADERS) $(MENUINC) \
						$(BASE)/vram.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/sound.o		:	$(SOUND)/sound.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndwave.o	:	$(SOUND)/sndwave.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndmp3.o		:	$(SOUND)/sndmp3.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndmidi.o	:	$(SOUND)/sndmidi.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndogg.o		:	$(SOUND)/sndogg.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndmix.o		:	$(SOUND)/sndmix.c $(HEADERS) $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndcsec.o	:	$(SOUND)/sndcsec.c $(HEADERS) $(SOUND)/sound.h \
						$(SOUND)/sndcsec.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/midiout.o	:	$(VERMOUTH)/midiout.c $(HEADERS) $(VERMOUTHINC)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/midimod.o	:	$(VERMOUTH)/midimod.c $(HEADERS) $(VERMOUTHINC) \
						$(TARG)/dosio.h $(COMMON)/textfile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/midinst.o	:	$(VERMOUTH)/midinst.c $(HEADERS) $(VERMOUTHINC) \
						$(TARG)/dosio.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/midvoice.o	:	$(VERMOUTH)/midvoice.c $(HEADERS) $(VERMOUTHINC)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/midtable.o	:	$(VERMOUTH)/midtable.c $(HEADERS) $(VERMOUTHINC)
	$(CC) $(COPT) -o $@ $<



$(OBJ)/akira.o		:	$(TARG)/akira.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/dosio.h $(TARG)/xdraws.h $(TARG)/xsound.h \
						$(TARG)/fontmng.h $(TARG)/inputmng.h \
						$(TARG)/scrnmng.h $(TARG)/taskmng.h \
						$(SOUND)/sound.h $(BASE)/sstream.h \
						$(BASE)/arcfile.h $(MENU)/sysmenu.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/xdraws.o		:	$(TARG)/xdraws.c $(HEADERS) $(BASEINC) \
						$(TARG)/xdraws.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/xsound.o		:	$(TARG)/xsound.c $(HEADERS) \
						$(TARG)/xsound.h $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/dosio.o		:	$(TARG)/dosio.c $(HEADERS) $(TARG)/dosio.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/trace.o		:	$(TARG)/trace.c $(HEADERS) $(COMMON)/codecnv.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/cddamng.o	:	$(TARG)/cddamng.c $(HEADERS) $(TARG)/cddamng.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/fontmng.o	:	$(TARG)/fontmng.c $(HEADERS) $(TARG)/fontmng.h \
						$(COMMON)/codecnv.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/inputmng.o	:	$(TARG)/inputmng.c $(HEADERS) $(TARG)/inputmng.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/moviemng.o	:	$(TARG)/moviemng.c $(HEADERS)
	$(CC) $(COPT) -o $@ $<

$(OBJ)/scrnmng.o	:	$(TARG)/scrnmng.c $(HEADERS) $(MENUINC) \
						$(BASE)/vram.h $(BASE)/vramdraw.h \
						$(TARG)/xdraws.h $(TARG)/scrnmng.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/taskmng.o	:	$(TARG)/taskmng.c $(HEADERS) \
						$(TARG)/inputmng.h $(TARG)/taskmng.h \
						$(SOUND)/sound.h $(BASE)/sstream.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/timemng.o	:	$(TARG)/timemng.c $(HEADERS) $(TARG)/timemng.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/gamemsg.o	:	$(TARG)/gamemsg.c $(HEADERS) $(BASEINC) \
						$(TARG)/gamemsg.h
	$(CC) $(COPT) -o $@ $<



$(OBJ)/gamecore.o	:	$(BASE)/gamecore.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/dosio.h $(TARG)/cddamng.h \
						$(TARG)/fontmng.h $(TARG)/inputmng.h \
						$(TARG)/scrnmng.h $(TARG)/taskmng.h \
						$(TARG)/timemng.h $(TARG)/gamemsg.h \
						$(SOUND)/sound.h $(MENU)/sysmenu.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h \
						$(BASE)/savefile.h $(BASE)/savedrs.h \
						$(BASE)/sstream.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/gamedef.o	:	$(BASE)/gamedef.c $(HEADERS) \
						$(TARG)/dosio.h $(BASE)/gamedef.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/gamecfg.o	:	$(BASE)/gamecfg.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/gamemsg.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/arcfile.o	:	$(BASE)/arcfile.c $(HEADERS) \
						$(TARG)/dosio.h $(BASE)/arcfile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/savefile.o	:	$(BASE)/savefile.c $(HEADERS) $(BASEINC) \
						$(TARG)/timemng.h \
						$(BASE)/savefile.h $(BASE)/savedrs.h $(BASE)/saveisf.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/savedrs.o	:	$(BASE)/savedrs.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/timemng.h \
						$(BASE)/savefile.h $(BASE)/savedrs.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/saveisf.o	:	$(BASE)/saveisf.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/timemng.h \
						$(BASE)/savefile.h $(BASE)/saveisf.h $(BASE)/bmpdata.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/script.o		:	$(BASE)/script.c $(HEADERS) $(BASEINC) $(MENUINC) \
						$(TARG)/dosio.h \
						$(ISFCMD)/isf_cmd.h $(ISFCMD)/drs_cmd.h \
						$(BASE)/arcfile.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/event.o		:	$(BASE)/event.c $(HEADERS) $(BASEINC) \
						$(TARG)/fontmng.h $(TARG)/scrnmng.h \
						$(TARG)/taskmng.h $(SOUND)/sound.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/vram.o		:	$(BASE)/vram.c $(HEADERS) $(BASE)/vram.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/vramdraw.o	:	$(BASE)/vramdraw.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/scrnmng.h \
						$(TARG)/taskmng.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h $(BASE)/bmpdata.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/vrammix.o	:	$(BASE)/vrammix.c $(HEADERS) $(TARG)/fontmng.h \
						$(BASE)/vram.h $(BASE)/vrammix.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/textwin.o	:	$(BASE)/textwin.c $(HEADERS) $(BASEINC) \
						$(TARG)/fontmng.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/effect.o		:	$(BASE)/effect.c $(HEADERS) $(BASEINC) \
						$(TARG)/scrnmng.h $(TARG)/gamemsg.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/textdisp.o	:	$(BASE)/textdisp.c $(HEADERS) $(BASEINC) \
						$(TARG)/fontmng.h $(TARG)/inputmng.h \
						$(TARG)/taskmng.h \
						$(SOUND)/sound.h $(BASE)/arcfile.h $(BASE)/sstream.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/anime.o		:	$(BASE)/anime.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/taskmng.h \
						$(BASE)/arcfile.h $(BASE)/cgload.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/cgload.o		:	$(PATCH)/cgload.c $(HEADERS) $(TARG)/taskmng.h \
						$(BASE)/arcfile.h $(BASE)/vram.h $(BASE)/vramdraw.h \
						$(BASE)/cgload.h $(BASE)/bmpdata.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/bmpdata.o	:	$(BASE)/bmpdata.c $(HEADERS) \
						$(BASE)/arcfile.h $(BASE)/vram.h $(BASE)/bmpdata.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sndplay.o	:	$(BASE)/sndplay.c $(HEADERS) $(BASEINC) \
						$(TARG)/dosio.h $(TARG)/cddamng.h \
						$(SOUND)/sound.h $(BASE)/arcfile.h $(BASE)/sstream.h
	$(CC) $(COPT) -o $@ $<

$(OBJ)/sstream.o	:	$(BASE)/sstream.c $(HEADERS) \
						$(TARG)/dosio.h $(SOUND)/sound.h \
						$(BASE)/arcfile.h $(BASE)/sstream.h
	$(CC) $(COPT) -o $@ $<



clean:
	$(RM) $(XKZKOBJ)
	$(RM) $(XKZKBIN)

