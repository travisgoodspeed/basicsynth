//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#pragma once

//#define _CRT_SECURE_NO_WARNINGS

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

#include <atlbase.h>
// This is the base WTL include, by default in BasicSynth\OpenSource\WTL
// download from http://sourceforge.net/projects/wtl/ if needed
#include <atlapp.h>

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlsplit.h>
#define _WTL_NO_CSTRING 1
#include <atlmisc.h>

#include <ShlObj.h>
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#include <Mmreg.h>
//#include <dsound.h>
//#pragma comment(lib, "dsound.lib")
// dsound.h and dsound.lib are now only included with the DirectX SDK.
// We only need two functions from dsound.lib, so we will dynamically load them.
class SynthAppModule : public CAppModule
{
public:
	HWND mainWnd;
	char ProductName[80];
	GUID *waveID;
};

extern SynthAppModule _Module;

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#define PROJECT_PANE  0x01
#define MIXER_PANE    0x02
#define PLAYER_PANE   0x04
#define EDITOR_PANE   0x08
#define KEYBOARD_PANE 0x10
#define TOOLBAR_PANE  0x20
#define STATUS_PANE   0x40

#define ID_PROJECT_LIST 101
#define ID_PLAYER_WND   102
#define ID_MIXER_WND    103
#define ID_KEYBOARD_WND 104
#define ID_TAB_WND      105
#define ID_EDIT_WND     106
#define ID_FORM_WND     107
#define ID_SCROLL_WND   108

#define IN_UPDATEUI     1000

#define VKBD_KEYDN 1
#define VKBD_KEYUP 2
#define VKBD_CHANGE 3

#define WM_GENMSG (WM_USER+1)
#define WM_GENEND (WM_USER+2)
#define WM_GENUPDTM (WM_USER+3)
#define WM_GENUPDPK (WM_USER+4)
#define WM_VKBD   (WM_USER+5)

#define WFI_MAXPART 16

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <BasicSynth.h>
#include <SynthThread.h>
#include <Player.h>
#include <WaveOutDirect.h>
#include <Instruments.h>
#include <NLConvert.h>

#include <ComposerCore.h>

#include "WinUTF8.h"

extern void LoadEditorDLL();

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
