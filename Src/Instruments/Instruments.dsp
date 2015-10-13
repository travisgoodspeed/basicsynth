# Microsoft Developer Studio Project File - Name="Instruments" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Instruments - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Instruments.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Instruments.mak" CFG="Instruments - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Instruments - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Instruments - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Instruments - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release6"
# PROP Intermediate_Dir "Release6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "../../Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_TINYXML" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\Instruments6.lib"

!ELSEIF  "$(CFG)" == "Instruments - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug6"
# PROP Intermediate_Dir "Debug6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_TINYXML" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\Instruments6D.lib"

!ENDIF 

# Begin Target

# Name "Instruments - Win32 Release"
# Name "Instruments - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\BuzzSynth.h
# End Source File
# Begin Source File

SOURCE=.\Chuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\FMSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\GMPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\LFO.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadInstrLib.cpp
# End Source File
# Begin Source File

SOURCE=.\MatrixSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\MixerControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ModSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\PitchBend.cpp
# End Source File
# Begin Source File

SOURCE=.\SFPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\SubSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\Tone.cpp
# End Source File
# Begin Source File

SOURCE=.\WFSynth.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddSynth.h
# End Source File
# Begin Source File

SOURCE=.\BuzzSynth.cpp
# End Source File
# Begin Source File

SOURCE=.\Chuffer.h
# End Source File
# Begin Source File

SOURCE=.\FMSynth.h
# End Source File
# Begin Source File

SOURCE=.\GMPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Includes.h
# End Source File
# Begin Source File

SOURCE=.\Instruments.h
# End Source File
# Begin Source File

SOURCE=.\LFO.h
# End Source File
# Begin Source File

SOURCE=.\LoadInstrLib.h
# End Source File
# Begin Source File

SOURCE=.\MatrixSynth.h
# End Source File
# Begin Source File

SOURCE=.\MixerControl.h
# End Source File
# Begin Source File

SOURCE=.\ModSynth.h
# End Source File
# Begin Source File

SOURCE=.\ModSynthUG.h
# End Source File
# Begin Source File

SOURCE=.\PitchBend.h
# End Source File
# Begin Source File

SOURCE=.\SFPlayer.h
# End Source File
# Begin Source File

SOURCE=.\SubSynth.h
# End Source File
# Begin Source File

SOURCE=.\Tone.h
# End Source File
# Begin Source File

SOURCE=.\UGCalc.h
# End Source File
# Begin Source File

SOURCE=.\UGDelay.h
# End Source File
# Begin Source File

SOURCE=.\UGEnvGen.h
# End Source File
# Begin Source File

SOURCE=.\UGFilter.h
# End Source File
# Begin Source File

SOURCE=.\UGOscil.h
# End Source File
# Begin Source File

SOURCE=.\WFSynth.h
# End Source File
# End Group
# End Target
# End Project
