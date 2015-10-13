// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if _MSC_VER
#pragma once
#endif

#ifdef _WIN32
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOGDI                           // don't need graphics
#define NOCRYPT
// Windows Header Files:
#include <windows.h>
#define MMNODRV
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#ifdef _MSC_VER
#define snprintf sprintf_s
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <BasicSynth.h>
#include <SynthThread.h>
#include <Instruments.h>
#include <SFFile.h>
#include <DLSFile.h>
#include <SMFFile.h>
#include <GMPlayer.h>
#ifdef _WIN32
#include <WaveOutDirect.h>
#endif
#ifdef UNIX
#include <pthread.h>
#include <WaveOutALSA.h>
#endif

#include "GMInstrManager.h"
#include "GMSynthDLL.h"

