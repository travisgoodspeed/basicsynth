//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file globinc.h Global includes
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#if _WIN32
#include <windows.h>
#pragma comment(lib, "Winmm.lib")
#include <Mmreg.h>
//#pragma comment(lib, "dsound.lib")
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

////////////////////////////////////
// FLTK Includes
///////////////////////////////////
#define USE_POLL 0
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Choice.H>
//#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>
//#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
//#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/x.H>
// If using fltk 1.3.x use tree widget.
// If using fltk 1.1.x use list box widget.
// Or, set -DUSE_TREE_CTL=?
#ifndef USE_TREE_CTL
# if FL_MINOR_VERSION > 1
#  define USE_TREE_CTL 1
#include <FL/Fl_Tree.H>
# else
#  define USE_TREE_CTL 0
# endif
#endif


////////////////////////////////////
// BasicSynth Includes
///////////////////////////////////
#include <BasicSynth.h>
#include <SynthThread.h>
#if _WIN32
#include <WaveOutDirect.h>
#endif
#if UNIX
#include <WaveOutALSA.h>
#endif
#include <Instruments.h>
#include <NLConvert.h>
#include <ComposerCore.h>

extern void LogDebug(const char *fmt, ...);
