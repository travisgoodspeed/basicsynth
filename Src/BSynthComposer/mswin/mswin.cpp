/////////////////////////////////////////////////////////////////////////////
// Startup code for BSynthComposer running on MS-Windows. We read the project
// options, create a main frame window, and run the message loop. Various global
// housekeeping chores are done here as well.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <locale.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>

#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

SynthAppModule _Module;

const GUID DSDEVID_DefaultPlayback = {0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03};

int Run(LPTSTR cmd = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
// The C-runtime functions atof, sprintf, etc., use the locale
// to set the character for a decimal point. In order
// for XML and Notelist files to be consistent on different 
// locales, a call to setlocale(LC_NUMERIC, "C") needs to be made
// at some point. This is not the ideal solution since it also
// affects the UI. There are versions of atof and sprintf that take a
// locale parameter (atof_l), but these are not consistent across
// various compilers.
	setlocale(LC_NUMERIC, "C");

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	LoadString(_Module.GetResourceInstance(), IDS_PRODUCT, _Module.ProductName, 80);

	theProject = NULL;

	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	if (sw < 1024)
	{
		prjOptions.frmLeft = 0;
		prjOptions.frmWidth = sw;
	}
	else
	{
		prjOptions.frmLeft = (sw - 1024) / 2;
		prjOptions.frmWidth = 1024;
	}
	if (sh < 768)
	{
		prjOptions.frmTop = 0;
		prjOptions.frmHeight = sh;
	}
	else
	{
		prjOptions.frmTop = (sh - 768) / 2;
		prjOptions.frmHeight = 768;
	}

	prjOptions.Load();

	SetCurrentDirectory(prjOptions.defPrjDir);

	RECT rc;
	rc.left = prjOptions.frmLeft;
	rc.top = prjOptions.frmTop;
	rc.right = prjOptions.frmWidth + rc.left;
	rc.bottom = prjOptions.frmHeight + rc.top;

	MainFrame wndMain;
	if(wndMain.CreateEx(HWND_DESKTOP, rc) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	prjOptions.dsoundHWND = wndMain;

	if (cmd && *cmd)
	{
		while (*cmd == ' ')
			cmd++;
		if (*cmd== '"')
		{
			char *quo = strchr(++cmd, '"');
			if (quo)
				*quo = 0;
		}
		if (wndMain.OpenProject(cmd))
			wndMain.AfterOpenProject();
	}

	if (prjOptions.frmMax)
		nCmdShow = SW_MAXIMIZE;
	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	prjOptions.Save();
	return nRet;
}

static void bad_parameter(
   const wchar_t * expression,
   const wchar_t * function, 
   const wchar_t * file, 
   unsigned int line,
   uintptr_t pReserved)
{
#ifdef _DEBUG
	OutputDebugStringW(expression);
#endif
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	// this prevents the CRT library from kicking us out if there are bad parameters
	_set_invalid_parameter_handler(bad_parameter);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
	LoadEditorDLL();

	_Module.mainWnd = 0;
	hRes = _Module.Init(NULL, hInstance);

	knobCache = new WidgetImageCache;
	switchCache = new WidgetImageCache;

	int nRet = Run(lpstrCmdLine, nCmdShow);

	delete switchCache;
	delete knobCache;

	GdiplusShutdown(gdiplusToken);
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
