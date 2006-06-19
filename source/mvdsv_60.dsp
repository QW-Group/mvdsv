# Microsoft Developer Studio Project File - Name="qwsv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=qwsv - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mvdsv_60.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mvdsv_60.mak" CFG="qwsv - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qwsv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qwsv - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qwsv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SRelease_60"
# PROP BASE Intermediate_Dir "SRelease_60"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SRelease_60"
# PROP Intermediate_Dir "SRelease_60"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /YX /c
# ADD CPP /nologo /G6 /W3 /O2 /Op /Ob2 /D "SERVERONLY" /D "NDEBUG" /D "USE_PR2" /D "__LITTLE_ENDIAN__Q__" /Fr /Fp"SRelease_60/mvdsv.pch" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"SRelease_60/mvdsv.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"../mvdsv-current_60.exe"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /map /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SDebug_60"
# PROP BASE Intermediate_Dir "SDebug_60"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SDebug_60"
# PROP Intermediate_Dir "SDebug_60"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /YX /c
# ADD CPP /nologo /G6 /ML /W3 /GX /ZI /Od /D "SERVERONLY" /D "_DEBUG" /D "USE_PR2" /D "__LITTLE_ENDIAN__Q__" /FAcs /FR /Fp"SDebug_60/mvdsv.pch" /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"SDebug_60/mvdsv.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"../mvdsv-debug_60.exe"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "qwsv - Win32 Release"
# Name "qwsv - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "pcre.c"

# PROP Default_Filter ""
# Begin Source File

SOURCE=pcre\get.c
# End Source File
# Begin Source File

SOURCE=pcre\pcre.c
# End Source File
# End Group
# Begin Source File

SOURCE=bothtools.c
# End Source File
# Begin Source File

SOURCE=cmd.c
# End Source File
# Begin Source File

SOURCE=common.c
# End Source File
# Begin Source File

SOURCE=crc.c
# End Source File
# Begin Source File

SOURCE=cvar.c
# End Source File
# Begin Source File

SOURCE=fs.c
# End Source File
# Begin Source File

SOURCE=mathlib.c
# End Source File
# Begin Source File

SOURCE=mdfour.c
# End Source File
# Begin Source File

SOURCE=net.c
# End Source File
# Begin Source File

SOURCE=net_chan.c
# End Source File
# Begin Source File

SOURCE=pmove.c
# End Source File
# Begin Source File

SOURCE=pmovetst.c
# End Source File
# Begin Source File

SOURCE=pr2_cmds.c
# End Source File
# Begin Source File

SOURCE=pr2_edict.c
# End Source File
# Begin Source File

SOURCE=pr2_exec.c
# End Source File
# Begin Source File

SOURCE=pr2_vm.c
# End Source File
# Begin Source File

SOURCE=pr_cmds.c
# End Source File
# Begin Source File

SOURCE=pr_edict.c
# End Source File
# Begin Source File

SOURCE=pr_exec.c
# End Source File
# Begin Source File

SOURCE=sha1.c
# End Source File
# Begin Source File

SOURCE=sv_ccmds.c
# End Source File
# Begin Source File

SOURCE=sv_demo.c
# End Source File
# Begin Source File

SOURCE=sv_ents.c
# End Source File
# Begin Source File

SOURCE=sv_init.c
# End Source File
# Begin Source File

SOURCE=sv_login.c
# End Source File
# Begin Source File

SOURCE=sv_main.c
# End Source File
# Begin Source File

SOURCE=sv_master.c
# End Source File
# Begin Source File

SOURCE=sv_mod_frags.c
# End Source File
# Begin Source File

SOURCE=sv_model.c
# End Source File
# Begin Source File

SOURCE=sv_move.c
# End Source File
# Begin Source File

SOURCE=sv_nchan.c
# End Source File
# Begin Source File

SOURCE=sv_phys.c
# End Source File
# Begin Source File

SOURCE=sv_send.c
# End Source File
# Begin Source File

SOURCE=sv_sys_win.c
# End Source File
# Begin Source File

SOURCE=sv_user.c
# End Source File
# Begin Source File

SOURCE=sv_windows.c
# End Source File
# Begin Source File

SOURCE=version.c
# End Source File
# Begin Source File

SOURCE=winquake.rc
# End Source File
# Begin Source File

SOURCE=world.c
# End Source File
# Begin Source File

SOURCE=zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "pcre.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=pcre\chartables.c
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=pcre\config.h
# End Source File
# Begin Source File

SOURCE=pcre\internal.h
# End Source File
# Begin Source File

SOURCE=pcre\pcre.h
# End Source File
# End Group
# Begin Source File

SOURCE=bothdefs.h
# End Source File
# Begin Source File

SOURCE=bspfile.h
# End Source File
# Begin Source File

SOURCE=cmd.h
# End Source File
# Begin Source File

SOURCE=common.h
# End Source File
# Begin Source File

SOURCE=crc.h
# End Source File
# Begin Source File

SOURCE=cvar.h
# End Source File
# Begin Source File

SOURCE=fs.h
# End Source File
# Begin Source File

SOURCE=g_public.h
# End Source File
# Begin Source File

SOURCE=log.h
# End Source File
# Begin Source File

SOURCE=mathlib.h
# End Source File
# Begin Source File

SOURCE=mdfour.h
# End Source File
# Begin Source File

SOURCE=model.h
# End Source File
# Begin Source File

SOURCE=modelgen.h
# End Source File
# Begin Source File

SOURCE=net.h
# End Source File
# Begin Source File

SOURCE=pmove.h
# End Source File
# Begin Source File

SOURCE=pr2.h
# End Source File
# Begin Source File

SOURCE=pr2_vm.h
# End Source File
# Begin Source File

SOURCE=pr_comp.h
# End Source File
# Begin Source File

SOURCE=progdefs.h
# End Source File
# Begin Source File

SOURCE=progs.h
# End Source File
# Begin Source File

SOURCE=protocol.h
# End Source File
# Begin Source File

SOURCE=quakeasm.h
# End Source File
# Begin Source File

SOURCE=qwsvdef.h
# End Source File
# Begin Source File

SOURCE=resource.h
# End Source File
# Begin Source File

SOURCE=sbar.h
# End Source File
# Begin Source File

SOURCE=server.h
# End Source File
# Begin Source File

SOURCE=sha1.h
# End Source File
# Begin Source File

SOURCE=spritegn.h
# End Source File
# Begin Source File

SOURCE=sv_mod_frags.h
# End Source File
# Begin Source File

SOURCE=sv_windows.h
# End Source File
# Begin Source File

SOURCE=sys.h
# End Source File
# Begin Source File

SOURCE=version.h
# End Source File
# Begin Source File

SOURCE=winquake.h
# End Source File
# Begin Source File

SOURCE=world.h
# End Source File
# Begin Source File

SOURCE=zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=qwcl2.ico
# End Source File
# End Group
# Begin Group "Asm Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=math.s

!IF  "$(CFG)" == "qwsv - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\SRelease_60
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

# Begin Custom Build
OutDir=.\SDebug_60
InputPath=math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=worlda.s

!IF  "$(CFG)" == "qwsv - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\SRelease_60
InputPath=worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

# Begin Custom Build
OutDir=.\SDebug_60
InputPath=worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm < $(OUTDIR)\$(InputName).spp >$(OUTDIR)\$(InputName).asm 
	ml /nologo /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
