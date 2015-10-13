//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file main.cpp Program entry function.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "MainFrm.h"

void LogDebug(const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, 1024, fmt, args);
#if _MSVC
	OutputDebugString(buffer);
#else
	fputs(buffer, stderr);
#endif
}

#ifndef FOREVER
#define FOREVER 1e20
#endif

int main(int argc, char *argv[])
{
#if _MSC_VER && _DEBUG
	// The FLTK Windows version of WinMain opens a console window in DEBUG mode. This is not good...
	// With VS2008 on XP, if the program crashes you get orphaned consoles .
	freopen("nul", "r", stdin);
	freopen("nul", "w", stdout);
	freopen("nul", "w", stderr);
	FreeConsole();
#endif

	// use '.' for decimal point
	setlocale(LC_NUMERIC, "C");

//	Fl::scheme("none");
//	Fl::scheme("plastic");
	Fl::scheme("gtk+");
	prjOptions.frmWidth = Fl::w();
	if (prjOptions.frmWidth > 1024)
		prjOptions.frmWidth = 1024;
	prjOptions.frmHeight = Fl::h();
	if (prjOptions.frmHeight > 768)
		prjOptions.frmHeight = 768;
	prjOptions.frmLeft = (Fl::w() - prjOptions.frmWidth) / 2;
	prjOptions.frmTop  = (Fl::h() - prjOptions.frmHeight) / 2;
	prjOptions.Load();
	MainFrame *mainwnd = new MainFrame(prjOptions.frmLeft, prjOptions.frmTop,
		prjOptions.frmWidth, prjOptions.frmHeight, prjOptions.programName);
	mainwnd->show(argc, argv);
#ifdef _WIN32
	prjOptions.dsoundHWND = fl_xid(mainwnd);
#endif
	Fl::lock();
	while (mainwnd->Running())
		Fl::wait(FOREVER);
	return 0;
}
