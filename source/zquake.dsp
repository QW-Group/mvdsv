# Microsoft Developer Studio Project File - Name="zquake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=zquake - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zquake.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zquake.mak" CFG="zquake - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zquake - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "zquake - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "zquake - Win32 GLDebug" (based on "Win32 (x86) Application")
!MESSAGE "zquake - Win32 GLRelease" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\ZRelease"
# PROP Intermediate_Dir ".\ZRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /GX /O2 /I "..\dxsdk\sdk\inc" /I "..\scitech\include" /I "..\client" /I "..\server" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "id386" /D "QW_BOTH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 mglfx.lib dxguid.lib opengl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcmt"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\ZDebug"
# PROP Intermediate_Dir ".\ZDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /ML /Gm /GX /ZI /Od /I "..\dxsdk\sdk\inc" /I "..\scitech\include" /I "..\client" /I "..\server" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "id386" /D "QW_BOTH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 mglfx.lib dxguid.lib opengl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\zquake___0"
# PROP BASE Intermediate_Dir ".\zquake___0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\ZGLDebug"
# PROP Intermediate_Dir ".\ZGLDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /ML /Gm /GX /Zi /Od /I "e:\msdev\projects\dxsdk\sdk\inc" /I "e:\msdev\projects\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /G5 /ML /GX /ZI /Od /I "..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /D "id386" /D "QW_BOTH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 opengl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib e:\msdev\projects\scitech\lib\win32\vc\mgllt.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 comctl32.lib glu32.lib dxguid.lib opengl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:".\ZGLDebug\zquake-gl.exe"
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\zquake___W"
# PROP BASE Intermediate_Dir ".\zquake___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\ZGLRelease"
# PROP Intermediate_Dir ".\ZGLRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /ML /GX /Zi /Od /I "e:\msdev\projects\dxsdk\sdk\inc" /I "e:\msdev\projects\scitech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /FR /YX /c
# ADD CPP /nologo /G5 /ML /GX /Ot /Ow /I "..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "GLQUAKE" /D "id386" /D "QW_BOTH" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 e:\msdev\projects\winquake\dxsdk\sdk\lib\dxguid.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GLDebug/glzquake.exe"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 comctl32.lib glu32.lib dxguid.lib opengl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:".\ZGLRelease\zquake-gl.exe"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "zquake - Win32 Release"
# Name "zquake - Win32 Debug"
# Name "zquake - Win32 GLDebug"
# Name "zquake - Win32 GLRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\cd_win.c
# End Source File
# Begin Source File

SOURCE=.\cl_cam.c
# End Source File
# Begin Source File

SOURCE=.\cl_demo.c
# End Source File
# Begin Source File

SOURCE=.\cl_ents.c
# End Source File
# Begin Source File

SOURCE=.\cl_input.c
# End Source File
# Begin Source File

SOURCE=.\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse.c
# End Source File
# Begin Source File

SOURCE=.\cl_pred.c
# End Source File
# Begin Source File

SOURCE=.\cl_slist.c
# End Source File
# Begin Source File

SOURCE=.\cl_tent.c
# End Source File
# Begin Source File

SOURCE=.\cmd.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\cvar.c
# End Source File
# Begin Source File

SOURCE=.\d_edge.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_fill.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_init.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_modech.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_part.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_polyse.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_scan.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_sky.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_sprite.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_surf.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_vars.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_zpoint.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_draw.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_mesh.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_model.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_ngraph.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_refrag.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rlight.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmain.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmisc.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rsurf.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_screen.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_test.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_warp.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\in_win.c
# End Source File
# Begin Source File

SOURCE=.\keys.c
# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\mdfour.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\model.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\net_chan.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\nonintel.c
# End Source File
# Begin Source File

SOURCE=.\pmove.c
# End Source File
# Begin Source File

SOURCE=.\pmovetst.c
# End Source File
# Begin Source File

SOURCE=.\pr_cmds.c
# End Source File
# Begin Source File

SOURCE=.\pr_edict.c
# End Source File
# Begin Source File

SOURCE=.\pr_exec.c
# End Source File
# Begin Source File

SOURCE=.\r_aclip.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_alias.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_bsp.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_edge.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_efrag.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_light.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_main.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_misc.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_part.c
# End Source File
# Begin Source File

SOURCE=.\r_sky.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_sprite.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_surf.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_vars.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\savegame.c
# End Source File
# Begin Source File

SOURCE=.\sbar.c
# End Source File
# Begin Source File

SOURCE=.\screen.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\skin.c
# End Source File
# Begin Source File

SOURCE=.\snd_dma.c
# End Source File
# Begin Source File

SOURCE=.\snd_mem.c
# End Source File
# Begin Source File

SOURCE=.\snd_mix.c
# End Source File
# Begin Source File

SOURCE=.\snd_win.c
# End Source File
# Begin Source File

SOURCE=.\sv_ccmds.c
# End Source File
# Begin Source File

SOURCE=.\sv_ents.c
# End Source File
# Begin Source File

SOURCE=.\sv_init.c
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_move.c
# End Source File
# Begin Source File

SOURCE=.\sv_nchan.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys.c
# End Source File
# Begin Source File

SOURCE=.\sv_send.c
# End Source File
# Begin Source File

SOURCE=.\sv_user.c
# End Source File
# Begin Source File

SOURCE=.\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\teamplay.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# Begin Source File

SOURCE=.\vid_wgl.c

!IF  "$(CFG)" == "zquake - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vid_win.c

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\view.c
# End Source File
# Begin Source File

SOURCE=.\wad.c
# End Source File
# Begin Source File

SOURCE=.\winquake.rc
# End Source File
# Begin Source File

SOURCE=.\world.c
# End Source File
# Begin Source File

SOURCE=.\zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\adivtab.h
# End Source File
# Begin Source File

SOURCE=.\anorm_dots.h
# End Source File
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\bothdefs.h
# End Source File
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\cdaudio.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\d_iface.h

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_ifacea.h

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_local.h

!IF  "$(CFG)" == "zquake - Win32 Release"

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\gl_model.h
# End Source File
# Begin Source File

SOURCE=.\gl_warp_sin.h
# End Source File
# Begin Source File

SOURCE=.\glquake.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\quake\v2\master\masterpr.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\modelgen.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\pmove.h
# End Source File
# Begin Source File

SOURCE=.\protocol.h
# End Source File
# Begin Source File

SOURCE=.\quakedef.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\r_shared.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\sbar.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\spritegn.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\wad.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# Begin Source File

SOURCE=.\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\qe3.ico
# End Source File
# Begin Source File

SOURCE=.\quakeworld.bmp
# End Source File
# Begin Source File

SOURCE=.\zquake2.ico
# End Source File
# End Group
# Begin Group "Asm Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_math.S

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\cl_math.S
InputName=cl_math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build
OutDir=.\ZDebug
InputPath=.\cl_math.S
InputName=cl_math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# Begin Custom Build
OutDir=.\ZGLDebug
InputPath=.\cl_math.S
InputName=cl_math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D "id386" /D "GLQUAKE" > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# Begin Custom Build
OutDir=.\ZGLRelease
InputPath=.\cl_math.S
InputName=cl_math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D id386 /D GLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_draw.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_draw16.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_parta.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_polysa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_scana.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_spr8.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d_varsa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# Begin Custom Build
OutDir=.\ZGLDebug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D "id386" /D "GLQUAKE" > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# Begin Custom Build
OutDir=.\ZGLRelease
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D id386 /D GLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_aclipa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_aliasa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_drawa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_edgea.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_varsa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\snd_mixa.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# Begin Custom Build
OutDir=.\ZGLDebug
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D "id386" /D "GLQUAKE" > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# Begin Custom Build
OutDir=.\ZGLRelease
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D id386 /D GLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\surf16.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\surf8.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build - GAS2MASM Conversion
OutDir=.\ZDebug
InputPath=.\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sys_x86.s

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build
OutDir=.\ZDebug
InputPath=.\sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# Begin Custom Build
OutDir=.\ZGLDebug
InputPath=.\sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D "id386" /D "GLQUAKE" > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# Begin Custom Build
OutDir=.\ZGLRelease
InputPath=.\sys_x86.s
InputName=sys_x86

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D id386 /D GLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\worlda.S

!IF  "$(CFG)" == "zquake - Win32 Release"

# Begin Custom Build
OutDir=.\ZRelease
InputPath=.\worlda.S
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 Debug"

# Begin Custom Build
OutDir=.\ZDebug
InputPath=.\worlda.S
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /D "id386" /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm.exe < $(OUTDIR)\$(InputName).spp          >$(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLDebug"

# Begin Custom Build
OutDir=.\ZGLDebug
InputPath=.\worlda.S
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D "id386" /D "GLQUAKE" > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "zquake - Win32 GLRelease"

# Begin Custom Build
OutDir=.\ZGLRelease
InputPath=.\worlda.S
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP /D id386 /D GLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath) 
	..\gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >          $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                                           $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
