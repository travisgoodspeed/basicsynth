# Microsoft Developer Studio Project File - Name="Common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Common - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Common.mak" CFG="Common - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Common - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_TINYXML" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\Common6.lib"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_TINYXML" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\Common6D.lib"

!ENDIF 

# Begin Target

# Name "Common - Win32 Release"
# Name "Common - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DLSFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Global.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\InstrManager.cpp
# End Source File
# Begin Source File

SOURCE=.\MIDIControl.cpp
# End Source File
# Begin Source File

SOURCE=.\MIDIInput.cpp
# End Source File
# Begin Source File

SOURCE=.\MIDISequencer.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Player.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SequenceFile.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sequencer.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SFFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SMFFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundBank.cpp
# End Source File
# Begin Source File

SOURCE=.\SynthFileW.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SynthMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\SynthString.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SynthThread.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinystr.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxml.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlerror.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlparser.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WaveFile.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WaveOutDirect.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XmlWrapN.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XmlWrapT.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XmlWrapW.cpp

!IF  "$(CFG)" == "Common - Win32 Release"

!ELSEIF  "$(CFG)" == "Common - Win32 Debug"

# PROP Intermediate_Dir "Debug6"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Include\AllPass.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\BiQuad.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\DelayLine.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Docpages.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\DynFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\EnvGen.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\EnvGenSeg.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Flanger.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\GenNoise.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\GenWave.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\GenWave64.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\GenWaveWT.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\GenWaveX.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Instrument.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\MIDIControl.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\MIDIDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\MIDIInput.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\MIDISequencer.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Mixer.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Player.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Reverb.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SeqEvent.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SequenceFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Sequencer.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SFDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SFFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SFGen.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SFSoundBank.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SMFFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthList.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthString.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\SynthThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\tinystr.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\tinyxml.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WaveFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WaveOutDirect.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WaveTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\XmlWrap.h
# End Source File
# End Group
# End Target
# End Project
