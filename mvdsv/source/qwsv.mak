# Microsoft Developer Studio Generated NMAKE File, Based on qwsv.dsp
!IF "$(CFG)" == ""
CFG=qwsv - Win32 Release
!MESSAGE No configuration specified. Defaulting to qwsv - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "qwsv - Win32 Release" && "$(CFG)" != "qwsv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qwsv.mak" CFG="qwsv - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qwsv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qwsv - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "qwsv - Win32 Release"

OUTDIR=.\SRelease
INTDIR=.\SRelease
# Begin Custom Macros
OutDir=.\.\SRelease
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\quake\mvdsv.exe" "$(OUTDIR)\mvdsv.bsc"

!ELSE 

ALL : "..\..\..\quake\mvdsv.exe" "$(OUTDIR)\mvdsv.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\mdfour.obj"
	-@erase "$(INTDIR)\mdfour.sbr"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_chan.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmove.sbr"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\pmovetst.sbr"
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_cmds.sbr"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_edict.sbr"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\pr_exec.sbr"
	-@erase "$(INTDIR)\sv_ccmds.obj"
	-@erase "$(INTDIR)\sv_ccmds.sbr"
	-@erase "$(INTDIR)\sv_demo.obj"
	-@erase "$(INTDIR)\sv_demo.sbr"
	-@erase "$(INTDIR)\sv_ents.obj"
	-@erase "$(INTDIR)\sv_ents.sbr"
	-@erase "$(INTDIR)\sv_init.obj"
	-@erase "$(INTDIR)\sv_init.sbr"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_main.sbr"
	-@erase "$(INTDIR)\sv_model.obj"
	-@erase "$(INTDIR)\sv_model.sbr"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_move.sbr"
	-@erase "$(INTDIR)\sv_nchan.obj"
	-@erase "$(INTDIR)\sv_nchan.sbr"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_phys.sbr"
	-@erase "$(INTDIR)\sv_send.obj"
	-@erase "$(INTDIR)\sv_send.sbr"
	-@erase "$(INTDIR)\sv_sys_win.obj"
	-@erase "$(INTDIR)\sv_sys_win.sbr"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sv_user.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\version.obj"
	-@erase "$(INTDIR)\version.sbr"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\world.sbr"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\mvdsv.bsc"
	-@erase "..\..\..\quake\mvdsv.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /GX /Od /D "DEBUG" /D "SERVERONLY" /D "WIN32" /D\
 "_CONSOLE" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\mvdsv.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\SRelease/
CPP_SBRS=.\SRelease/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mvdsv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\mdfour.sbr" \
	"$(INTDIR)\net_chan.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\pmove.sbr" \
	"$(INTDIR)\pmovetst.sbr" \
	"$(INTDIR)\pr_cmds.sbr" \
	"$(INTDIR)\pr_edict.sbr" \
	"$(INTDIR)\pr_exec.sbr" \
	"$(INTDIR)\sv_ccmds.sbr" \
	"$(INTDIR)\sv_demo.sbr" \
	"$(INTDIR)\sv_ents.sbr" \
	"$(INTDIR)\sv_init.sbr" \
	"$(INTDIR)\sv_main.sbr" \
	"$(INTDIR)\sv_model.sbr" \
	"$(INTDIR)\sv_move.sbr" \
	"$(INTDIR)\sv_nchan.sbr" \
	"$(INTDIR)\sv_phys.sbr" \
	"$(INTDIR)\sv_send.sbr" \
	"$(INTDIR)\sv_sys_win.sbr" \
	"$(INTDIR)\sv_user.sbr" \
	"$(INTDIR)\version.sbr" \
	"$(INTDIR)\world.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\mvdsv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\mvdsv.pdb" /machine:I386 /out:"d:/quake/mvdsv.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mdfour.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\sv_ccmds.obj" \
	"$(INTDIR)\sv_demo.obj" \
	"$(INTDIR)\sv_ents.obj" \
	"$(INTDIR)\sv_init.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_model.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_nchan.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_send.obj" \
	"$(INTDIR)\sv_sys_win.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\version.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\zone.obj"

"..\..\..\quake\mvdsv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

OUTDIR=.\SDebug
INTDIR=.\SDebug
# Begin Custom Macros
OutDir=.\.\SDebug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\mvdsv.exe" "$(OUTDIR)\mvdsv.bsc"

!ELSE 

ALL : "$(OUTDIR)\mvdsv.exe" "$(OUTDIR)\mvdsv.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\mdfour.obj"
	-@erase "$(INTDIR)\mdfour.sbr"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_chan.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmove.sbr"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\pmovetst.sbr"
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_cmds.sbr"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_edict.sbr"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\pr_exec.sbr"
	-@erase "$(INTDIR)\sv_ccmds.obj"
	-@erase "$(INTDIR)\sv_ccmds.sbr"
	-@erase "$(INTDIR)\sv_demo.obj"
	-@erase "$(INTDIR)\sv_demo.sbr"
	-@erase "$(INTDIR)\sv_ents.obj"
	-@erase "$(INTDIR)\sv_ents.sbr"
	-@erase "$(INTDIR)\sv_init.obj"
	-@erase "$(INTDIR)\sv_init.sbr"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_main.sbr"
	-@erase "$(INTDIR)\sv_model.obj"
	-@erase "$(INTDIR)\sv_model.sbr"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_move.sbr"
	-@erase "$(INTDIR)\sv_nchan.obj"
	-@erase "$(INTDIR)\sv_nchan.sbr"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_phys.sbr"
	-@erase "$(INTDIR)\sv_send.obj"
	-@erase "$(INTDIR)\sv_send.sbr"
	-@erase "$(INTDIR)\sv_sys_win.obj"
	-@erase "$(INTDIR)\sv_sys_win.sbr"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sv_user.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\version.obj"
	-@erase "$(INTDIR)\version.sbr"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\world.sbr"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\mvdsv.bsc"
	-@erase "$(OUTDIR)\mvdsv.exe"
	-@erase "$(OUTDIR)\mvdsv.ilk"
	-@erase "$(OUTDIR)\mvdsv.map"
	-@erase "$(OUTDIR)\mvdsv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /GX /Od /D "_DEBUG" /D "SERVERONLY" /D "WIN32" /D\
 "_CONSOLE" /D "id386" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\mvdsv.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /ZI /c 
CPP_OBJS=.\SDebug/
CPP_SBRS=.\SDebug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mvdsv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\mdfour.sbr" \
	"$(INTDIR)\net_chan.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\pmove.sbr" \
	"$(INTDIR)\pmovetst.sbr" \
	"$(INTDIR)\pr_cmds.sbr" \
	"$(INTDIR)\pr_edict.sbr" \
	"$(INTDIR)\pr_exec.sbr" \
	"$(INTDIR)\sv_ccmds.sbr" \
	"$(INTDIR)\sv_demo.sbr" \
	"$(INTDIR)\sv_ents.sbr" \
	"$(INTDIR)\sv_init.sbr" \
	"$(INTDIR)\sv_main.sbr" \
	"$(INTDIR)\sv_model.sbr" \
	"$(INTDIR)\sv_move.sbr" \
	"$(INTDIR)\sv_nchan.sbr" \
	"$(INTDIR)\sv_phys.sbr" \
	"$(INTDIR)\sv_send.sbr" \
	"$(INTDIR)\sv_sys_win.sbr" \
	"$(INTDIR)\sv_user.sbr" \
	"$(INTDIR)\version.sbr" \
	"$(INTDIR)\world.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\mvdsv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\mvdsv.pdb" /map:"$(INTDIR)\mvdsv.map" /debug /machine:I386\
 /out:"$(OUTDIR)\mvdsv.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mdfour.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\sv_ccmds.obj" \
	"$(INTDIR)\sv_demo.obj" \
	"$(INTDIR)\sv_ents.obj" \
	"$(INTDIR)\sv_init.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_model.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_nchan.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_send.obj" \
	"$(INTDIR)\sv_sys_win.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\version.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\worlda.obj" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\mvdsv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "qwsv - Win32 Release" || "$(CFG)" == "qwsv - Win32 Debug"
SOURCE=.\cmd.c
DEP_CPP_CMD_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\cmd.obj"	"$(INTDIR)\cmd.sbr" : $(SOURCE) $(DEP_CPP_CMD_C)\
 "$(INTDIR)"


SOURCE=.\common.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_COMMO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\common.obj"	"$(INTDIR)\common.sbr" : $(SOURCE) $(DEP_CPP_COMMO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_COMMO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\common.obj"	"$(INTDIR)\common.sbr" : $(SOURCE) $(DEP_CPP_COMMO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\crc.c
DEP_CPP_CRC_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) $(DEP_CPP_CRC_C)\
 "$(INTDIR)"


SOURCE=.\cvar.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_CVAR_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\cvar.obj"	"$(INTDIR)\cvar.sbr" : $(SOURCE) $(DEP_CPP_CVAR_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_CVAR_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\cvar.obj"	"$(INTDIR)\cvar.sbr" : $(SOURCE) $(DEP_CPP_CVAR_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\mathlib.c
DEP_CPP_MATHL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\mathlib.obj"	"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL)\
 "$(INTDIR)"


SOURCE=.\mdfour.c
DEP_CPP_MDFOU=\
	".\mdfour.h"\
	

"$(INTDIR)\mdfour.obj"	"$(INTDIR)\mdfour.sbr" : $(SOURCE) $(DEP_CPP_MDFOU)\
 "$(INTDIR)"


SOURCE=.\net_chan.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_NET_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\net_chan.obj"	"$(INTDIR)\net_chan.sbr" : $(SOURCE) $(DEP_CPP_NET_C)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_NET_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\net_chan.obj"	"$(INTDIR)\net_chan.sbr" : $(SOURCE) $(DEP_CPP_NET_C)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\net_wins.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_NET_W=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\net_wins.obj"	"$(INTDIR)\net_wins.sbr" : $(SOURCE) $(DEP_CPP_NET_W)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_NET_W=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\net_wins.obj"	"$(INTDIR)\net_wins.sbr" : $(SOURCE) $(DEP_CPP_NET_W)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pmove.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PMOVE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\pmove.obj"	"$(INTDIR)\pmove.sbr" : $(SOURCE) $(DEP_CPP_PMOVE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PMOVE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\pmove.obj"	"$(INTDIR)\pmove.sbr" : $(SOURCE) $(DEP_CPP_PMOVE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pmovetst.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PMOVET=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\pmovetst.obj"	"$(INTDIR)\pmovetst.sbr" : $(SOURCE) $(DEP_CPP_PMOVET)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PMOVET=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\pmovetst.obj"	"$(INTDIR)\pmovetst.sbr" : $(SOURCE) $(DEP_CPP_PMOVET)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pr_cmds.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_CM=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_cmds.obj"	"$(INTDIR)\pr_cmds.sbr" : $(SOURCE) $(DEP_CPP_PR_CM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_CM=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_cmds.obj"	"$(INTDIR)\pr_cmds.sbr" : $(SOURCE) $(DEP_CPP_PR_CM)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pr_edict.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_ED=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_edict.obj"	"$(INTDIR)\pr_edict.sbr" : $(SOURCE) $(DEP_CPP_PR_ED)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_ED=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_edict.obj"	"$(INTDIR)\pr_edict.sbr" : $(SOURCE) $(DEP_CPP_PR_ED)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pr_exec.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_EX=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_exec.obj"	"$(INTDIR)\pr_exec.sbr" : $(SOURCE) $(DEP_CPP_PR_EX)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_EX=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\pr_exec.obj"	"$(INTDIR)\pr_exec.sbr" : $(SOURCE) $(DEP_CPP_PR_EX)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_ccmds.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_CC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_ccmds.obj"	"$(INTDIR)\sv_ccmds.sbr" : $(SOURCE) $(DEP_CPP_SV_CC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_CC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_ccmds.obj"	"$(INTDIR)\sv_ccmds.sbr" : $(SOURCE) $(DEP_CPP_SV_CC)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_demo.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_DE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_demo.obj"	"$(INTDIR)\sv_demo.sbr" : $(SOURCE) $(DEP_CPP_SV_DE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_DE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\winquake.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_demo.obj"	"$(INTDIR)\sv_demo.sbr" : $(SOURCE) $(DEP_CPP_SV_DE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_ents.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_ents.obj"	"$(INTDIR)\sv_ents.sbr" : $(SOURCE) $(DEP_CPP_SV_EN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_ents.obj"	"$(INTDIR)\sv_ents.sbr" : $(SOURCE) $(DEP_CPP_SV_EN)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_init.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_IN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_init.obj"	"$(INTDIR)\sv_init.sbr" : $(SOURCE) $(DEP_CPP_SV_IN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_IN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_init.obj"	"$(INTDIR)\sv_init.sbr" : $(SOURCE) $(DEP_CPP_SV_IN)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_main.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_MA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_main.obj"	"$(INTDIR)\sv_main.sbr" : $(SOURCE) $(DEP_CPP_SV_MA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_MA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_main.obj"	"$(INTDIR)\sv_main.sbr" : $(SOURCE) $(DEP_CPP_SV_MA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_model.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_model.obj"	"$(INTDIR)\sv_model.sbr" : $(SOURCE) $(DEP_CPP_SV_MO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_model.obj"	"$(INTDIR)\sv_model.sbr" : $(SOURCE) $(DEP_CPP_SV_MO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_move.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_MOV=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_move.obj"	"$(INTDIR)\sv_move.sbr" : $(SOURCE) $(DEP_CPP_SV_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_MOV=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_move.obj"	"$(INTDIR)\sv_move.sbr" : $(SOURCE) $(DEP_CPP_SV_MOV)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_nchan.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_NC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_nchan.obj"	"$(INTDIR)\sv_nchan.sbr" : $(SOURCE) $(DEP_CPP_SV_NC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_NC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_nchan.obj"	"$(INTDIR)\sv_nchan.sbr" : $(SOURCE) $(DEP_CPP_SV_NC)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_phys.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_PH=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_phys.obj"	"$(INTDIR)\sv_phys.sbr" : $(SOURCE) $(DEP_CPP_SV_PH)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_PH=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_phys.obj"	"$(INTDIR)\sv_phys.sbr" : $(SOURCE) $(DEP_CPP_SV_PH)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_send.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_SE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_send.obj"	"$(INTDIR)\sv_send.sbr" : $(SOURCE) $(DEP_CPP_SV_SE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_SE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_send.obj"	"$(INTDIR)\sv_send.sbr" : $(SOURCE) $(DEP_CPP_SV_SE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_sys_win.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_SY=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_sys_win.obj"	"$(INTDIR)\sv_sys_win.sbr" : $(SOURCE)\
 $(DEP_CPP_SV_SY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_SY=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_sys_win.obj"	"$(INTDIR)\sv_sys_win.sbr" : $(SOURCE)\
 $(DEP_CPP_SV_SY) "$(INTDIR)"


!ENDIF 

SOURCE=.\sv_user.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_US=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_user.obj"	"$(INTDIR)\sv_user.sbr" : $(SOURCE) $(DEP_CPP_SV_US)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_US=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\sv_user.obj"	"$(INTDIR)\sv_user.sbr" : $(SOURCE) $(DEP_CPP_SV_US)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\version.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_VERSI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\version.obj"	"$(INTDIR)\version.sbr" : $(SOURCE) $(DEP_CPP_VERSI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_VERSI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\version.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\version.obj"	"$(INTDIR)\version.sbr" : $(SOURCE) $(DEP_CPP_VERSI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\world.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_WORLD=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\world.obj"	"$(INTDIR)\world.sbr" : $(SOURCE) $(DEP_CPP_WORLD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_WORLD=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cmd.h"\
	".\common.h"\
	".\crc.h"\
	".\cvar.h"\
	".\mathlib.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\protocol.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\world.h"\
	".\zone.h"\
	

"$(INTDIR)\world.obj"	"$(INTDIR)\world.sbr" : $(SOURCE) $(DEP_CPP_WORLD)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\zone.c
DEP_CPP_ZONE_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\input.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\screen.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\zone.obj"	"$(INTDIR)\zone.sbr" : $(SOURCE) $(DEP_CPP_ZONE_)\
 "$(INTDIR)"


SOURCE=.\math.s

!IF  "$(CFG)" == "qwsv - Win32 Release"

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

OutDir=.\.\SDebug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
            $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	

!ENDIF 

SOURCE=.\worlda.s

!IF  "$(CFG)" == "qwsv - Win32 Release"

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

OutDir=.\.\SDebug
InputPath=.\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
            $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	

!ENDIF 


!ENDIF 

