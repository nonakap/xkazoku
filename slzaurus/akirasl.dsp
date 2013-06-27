# Microsoft Developer Studio Project File - Name="akirasl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=akirasl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "akirasl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "akirasl.mak" CFG="akirasl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "akirasl - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "akirasl - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "akirasl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../bin"
# PROP Intermediate_Dir "../obj/slrel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".\\" /I ".\win32" /I "..\\" /I "..\common" /I "..\isfcmd" /I "..\sound" /I "..\menu" /I "..\menubase" /I "..\sound\vermouth" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "MEMTRACE" /D "TRACE" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amethyst.lib damedec.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "akirasl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../bin"
# PROP Intermediate_Dir "../obj/sldbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".\\" /I ".\win32" /I "..\\" /I "..\common" /I "..\isfcmd" /I "..\sound" /I "..\menu" /I "..\menubase" /I "..\sound\vermouth" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "MEMTRACE" /D "TRACE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amethyst.lib damedec.lib /nologo /subsystem:console /debug /machine:I386 /out:"../bin/akirasld.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "akirasl - Win32 Release"
# Name "akirasl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\COMMON\_MEMORY.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\CODECNV.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\LSTARRAY.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\MILSTR.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\PROFILE.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\RECT.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\RESIZE.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\TEXTFILE.C
# End Source File
# Begin Source File

SOURCE=..\COMMON\VARIANT.C
# End Source File
# End Group
# Begin Group "zaurus"

# PROP Default_Filter ""
# Begin Group "SDL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32\SDL.c
# End Source File
# Begin Source File

SOURCE=.\win32\SDL_ttf.c
# End Source File
# Begin Source File

SOURCE=.\win32\SDLaudio.c
# End Source File
# Begin Source File

SOURCE=.\win32\SDLevent.c
# End Source File
# Begin Source File

SOURCE=.\win32\SDLmpw.c
# End Source File
# Begin Source File

SOURCE=.\win32\SDLvideo.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\akira.c
# End Source File
# Begin Source File

SOURCE=.\cddamng.c
# End Source File
# Begin Source File

SOURCE=.\dosio.c
# End Source File
# Begin Source File

SOURCE=.\fontmng.c
# End Source File
# Begin Source File

SOURCE=.\gamemsg.c
# End Source File
# Begin Source File

SOURCE=.\inputmng.c
# End Source File
# Begin Source File

SOURCE=.\moviemng.c
# End Source File
# Begin Source File

SOURCE=.\scrnmng.c
# End Source File
# Begin Source File

SOURCE=.\taskmng.c
# End Source File
# Begin Source File

SOURCE=.\timemng.c
# End Source File
# Begin Source File

SOURCE=.\trace.c
# End Source File
# Begin Source File

SOURCE=.\xdraws.c
# End Source File
# Begin Source File

SOURCE=.\xsound.c
# End Source File
# End Group
# Begin Group "Script"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD0.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD1.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD2.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD3.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD4.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD5.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMD7.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\DRS_CMDF.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD0.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD1.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD2.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD3.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD4.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD5.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD6.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD7.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD8.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMD9.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMDA.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMDB.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMDD.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMDE.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_CMDF.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\ISF_TODO.C
# End Source File
# Begin Source File

SOURCE=..\ISFCMD\SCR_BASE.C
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Group "vermouth"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SOUND\VERMOUTH\MIDIMOD.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\VERMOUTH\MIDINST.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\VERMOUTH\MIDIOUT.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\VERMOUTH\MIDTABLE.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\VERMOUTH\MIDVOICE.C
# End Source File
# End Group
# Begin Source File

SOURCE=..\SOUND\SNDCSEC.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SNDMIDI.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SNDMIX.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SNDMP3.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SNDOGG.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SNDWAVE.C
# End Source File
# Begin Source File

SOURCE=..\SOUND\SOUND.C
# End Source File
# End Group
# Begin Group "Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MENU\DLGCFG.C
# End Source File
# Begin Source File

SOURCE=..\MENU\DLGSAVE.C
# End Source File
# Begin Source File

SOURCE=..\MENU\SYSMENU.C
# End Source File
# End Group
# Begin Group "MENUBASE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MENUBASE\MENUBASE.C
# End Source File
# Begin Source File

SOURCE=..\MENUBASE\MENUDLG.C
# End Source File
# Begin Source File

SOURCE=..\MENUBASE\MENUMBOX.C
# End Source File
# Begin Source File

SOURCE=..\MENUBASE\MENURES.C
# End Source File
# Begin Source File

SOURCE=..\MENUBASE\MENUSYS.C
# End Source File
# Begin Source File

SOURCE=..\MENUBASE\MENUVRAM.C
# End Source File
# End Group
# Begin Source File

SOURCE=..\ANIME.C
# End Source File
# Begin Source File

SOURCE=..\ARCFILE.C
# End Source File
# Begin Source File

SOURCE=..\BMPDATA.C
# End Source File
# Begin Source File

SOURCE=.\patch\cgload.c
# End Source File
# Begin Source File

SOURCE=..\EFFECT.C
# End Source File
# Begin Source File

SOURCE=..\EVENT.C
# End Source File
# Begin Source File

SOURCE=..\GAMECFG.C
# End Source File
# Begin Source File

SOURCE=..\GAMECORE.C
# End Source File
# Begin Source File

SOURCE=..\GAMEDEF.C
# End Source File
# Begin Source File

SOURCE=..\SAVEDRS.C
# End Source File
# Begin Source File

SOURCE=..\SAVEFILE.C
# End Source File
# Begin Source File

SOURCE=..\SAVEISF.C
# End Source File
# Begin Source File

SOURCE=..\SCRIPT.C
# End Source File
# Begin Source File

SOURCE=..\SNDPLAY.C
# End Source File
# Begin Source File

SOURCE=..\SSTREAM.C
# End Source File
# Begin Source File

SOURCE=..\TEXTDISP.C
# End Source File
# Begin Source File

SOURCE=..\TEXTWIN.C
# End Source File
# Begin Source File

SOURCE=..\VRAM.C
# End Source File
# Begin Source File

SOURCE=..\VRAMDRAW.C
# End Source File
# Begin Source File

SOURCE=..\VRAMMIX.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
