/////////////////////////////////////////////////////////////////////////////
// BSynthComposer - frame window implementation for MS-Windows
//
// Much of the GUI implementation is here. This class manages the main frame
// layout, tree view, creation of dialogs and editors, and menu/toolbar commands.
// Properties are handled by separate dialogs. The specific dialog is determined
// from the project item type.
//
// Most methods are short, primarily passing work off to the current project
// item and/or editor.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <Htmlhelp.h>

#include "AboutDlg.h"
#include "QueryValueDlg.h"
#include "MainFrm.h"
#include "FormEditor.h"
#include "TextEditor.h"
#include "OptionsDlg.h"
#include "ProjectItemDlg.h"
#include "ProjectPropertiesDlg.h"
#include "ItemPropertiesDlg.h"
#include "GenerateDlg.h"
#include "MixerSetupDlg.h"

static char mruRegKey[] = "Software\\BasicSynth";

// toolbar buttons are defined here. The images
// are in the IDR_MAINFRAME bitmap. 
static TBBUTTON mainButtons[] = {
	{ 0, ID_PROJECT_NEW, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 1, ID_PROJECT_OPEN, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 13, ID_PROJECT_SAVE, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 3, ID_EDIT_CUT, 0, BTNS_BUTTON },
	{ 4, ID_EDIT_COPY, 0, BTNS_BUTTON },
	{ 5, ID_EDIT_PASTE, 0, BTNS_BUTTON },
	{ 6, ID_EDIT_UNDO, 0, BTNS_BUTTON },
	{ 7, ID_EDIT_REDO, 0, BTNS_BUTTON },
	{ 8, ID_EDIT_FIND, 0, BTNS_BUTTON },
	{ 9, ID_EDIT_FINDNEXT, 0, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 15, ID_ITEM_EDIT, 0, BTNS_BUTTON },
	{ 2, ID_ITEM_SAVE, 0, BTNS_BUTTON },
	{ 18, ID_ITEM_CLOSE, 0, BTNS_BUTTON },
	{ 14, ID_ITEM_NEW, 0, BTNS_BUTTON },
	{ 16, ID_ITEM_ADD, 0, BTNS_BUTTON },
	{ 12, ID_ITEM_PROPERTIES, 0, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 11, ID_PROJECT_GENERATE, 0, BTNS_BUTTON },
	{ 17, ID_PROJECT_PLAY, 0, BTNS_BUTTON },
	{ 4, 0, 0, BTNS_SEP },
	{ 19, ID_HELP_CONTENTS, TBSTATE_ENABLED, BTNS_BUTTON },
};

static int idbToolbar[] =
{
	IDB_NewProject32,
	IDB_OpenProject32_1,
	IDB_SaveProject32,
	IDB_Cut32,
	IDB_Copy32,
	IDB_Paste32,
	IDB_Undo32,
	IDB_Redo32,
	IDB_Find32,
	IDB_FindNext32,
	IDB_EditItem32,
	IDB_Save32,
	IDB_CloseItem32,
	IDB_NewItem32,
	IDB_AddItem32_1,
	IDB_ItemProperties32,
	IDB_CD32,
	IDB_Keyboard32,
	IDB_ProjectProperties32,
	IDB_Help32,
	-1
};

static int idbToolbar16[] =
{
	IDB_NewProject16,
	IDB_OpenProject16_1,
	IDB_SaveProject16,
	IDB_Cut16,
	IDB_Copy16,
	IDB_Paste16,
	IDB_Undo16,
	IDB_Redo16,
	IDB_Find16,
	IDB_FindNext16,
	IDB_EditItem16,
	IDB_Save16,
	IDB_CloseItem16,
	IDB_NewItem16,
	IDB_AddItem16_1,
	IDB_ItemProperties16,
	IDB_CD16,
	IDB_Keyboard16,
	IDB_ProjectProperties16,
	IDB_Help16_1,
	-1
};


static TBBUTTON mainButtons32[] = {
	{ 0, ID_PROJECT_NEW, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 1, ID_PROJECT_OPEN, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 2, ID_PROJECT_SAVE, 0, BTNS_BUTTON },
	{ 0, 0, 0, BTNS_SEP },
	{ 3, ID_EDIT_CUT, 0, BTNS_BUTTON },
	{ 4, ID_EDIT_COPY, 0, BTNS_BUTTON },
	{ 5, ID_EDIT_PASTE, 0, BTNS_BUTTON },
	{ 6, ID_EDIT_UNDO, 0, BTNS_BUTTON },
	{ 7, ID_EDIT_REDO, 0, BTNS_BUTTON },
	{ 8, ID_EDIT_FIND, 0, BTNS_BUTTON },
	{ 9, ID_EDIT_FINDNEXT, 0, BTNS_BUTTON },
	{ 0, 0, 0, BTNS_SEP },
	{ 10, ID_ITEM_EDIT, 0, BTNS_BUTTON },
	{ 11, ID_ITEM_SAVE, 0, BTNS_BUTTON },
	{ 12, ID_ITEM_CLOSE, 0, BTNS_BUTTON },
	{ 13, ID_ITEM_NEW, 0, BTNS_BUTTON },
	{ 14, ID_ITEM_ADD, 0, BTNS_BUTTON },
	{ 15, ID_ITEM_PROPERTIES, 0, BTNS_BUTTON },
	{ 0, 0, 0, BTNS_SEP },
	{ 16, ID_PROJECT_GENERATE, 0, BTNS_BUTTON },
	{ 17, ID_PROJECT_PLAY, 0, BTNS_BUTTON },
	{ 18, ID_PROJECT_OPTIONS, TBSTATE_ENABLED, BTNS_BUTTON },
	{ 0, 0, 0, BTNS_SEP },
	{ 19, ID_HELP_CONTENTS, TBSTATE_ENABLED, BTNS_BUTTON },
};


BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return tabView.PreTranslateMessage(pMsg);
}

BOOL MainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

/// Create the main frame window and associated child windows.
// Frame
//   +-- Rebar
//        +-- Menu (command bar)
//        +-- Toolbar
//   +-- Splitter
//        +-- Project tree view
//        +-- Tab View
//             +-- Editor
//   +-- Keyboard
//   +-- status bar
LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_Module.mainWnd = m_hWnd;

	RECT rcMain;
	GetClientRect(&rcMain);

	// create command bar window
	HWND hWndCmdBar = cmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	HMENU mainMnu = ::GetMenu(m_hWnd);
	HMENU fileMnu = ::GetSubMenu(mainMnu, 0);
	HMENU mruMnu = ::GetSubMenu(fileMnu, 9);
	mruList.SetMaxEntries(9);
	mruList.SetMenuHandle(mruMnu);
	mruList.SetMaxItemLength(40);
	mruList.ReadFromRegistry(mruRegKey);

	// attach menu
	cmdBar.AttachMenu(GetMenu());

	// add bitmaps to menus
	cmdBar.SetImageMaskColor(RGB(192,192,192));
	cmdBar.SetImageSize(16, 16);
	cmdBar.AddBitmap(IDB_NewProject16, ID_PROJECT_NEW);
	cmdBar.AddBitmap(IDB_OpenProject16, ID_PROJECT_OPEN);
	cmdBar.AddBitmap(IDB_SaveProject16, ID_PROJECT_SAVE);
	cmdBar.AddBitmap(IDB_SaveProjectAs16, ID_PROJECT_SAVEAS);
	cmdBar.AddBitmap(IDB_EditItem16, ID_ITEM_EDIT);
	cmdBar.AddBitmap(IDB_Save16, ID_ITEM_SAVE);
	cmdBar.AddBitmap(IDB_CloseItem16, ID_ITEM_CLOSE);
	cmdBar.AddBitmap(IDB_AddItem16_1, ID_ITEM_ADD);
	cmdBar.AddBitmap(IDB_NewItem16, ID_ITEM_NEW);
	cmdBar.AddBitmap(IDB_CopyItem16, ID_ITEM_COPY);
	cmdBar.AddBitmap(IDB_DeleteItem16_1, ID_ITEM_REMOVE);
	cmdBar.AddBitmap(IDB_ItemProperties16, ID_ITEM_PROPERTIES);

	cmdBar.AddBitmap(IDB_Undo16, ID_EDIT_UNDO);
	cmdBar.AddBitmap(IDB_Redo16, ID_EDIT_REDO);
	cmdBar.AddBitmap(IDB_Cut16, ID_EDIT_CUT);
	cmdBar.AddBitmap(IDB_Copy16, ID_EDIT_COPY);
	cmdBar.AddBitmap(IDB_Paste16, ID_EDIT_PASTE);
	cmdBar.AddBitmap(IDB_Find16, ID_EDIT_FIND);
	cmdBar.AddBitmap(IDB_FindNext16, ID_EDIT_FINDNEXT);

	cmdBar.AddBitmap(IDB_ProjectProperties16, ID_PROJECT_OPTIONS);
	cmdBar.AddBitmap(IDB_CD16, ID_PROJECT_GENERATE);
	cmdBar.AddBitmap(IDB_Keyboard16, ID_PROJECT_PLAY);

	cmdBar.AddBitmap(IDB_Help16_1, ID_HELP_CONTENTS);
	cmdBar.AddBitmap(IDB_Info16_1, ID_APP_ABOUT);

	// remove old menu
	SetMenu(NULL);

	bVisible = EDITOR_PANE | TOOLBAR_PANE | STATUS_PANE;

	int *idbList;
	int tbImgSize;
	if (prjOptions.toolBarSize == 16)
	{
		idbList = idbToolbar16;
		tbImgSize = 16;
	}
	else
	{
		idbList = idbToolbar;
		tbImgSize = 32;
	}

	// create the toolbar
	DWORD tbWinStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD tbStyle = CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT;
	HWND hWndToolBar = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, tbWinStyle | tbStyle,
		0, 0, 100, 100, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR),
		_Module.GetModuleInstance(), NULL);
	::SendMessage(hWndToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);

	HIMAGELIST hImageList = ImageList_Create(tbImgSize, tbImgSize, ILC_COLOR24|ILC_MASK, 21, 5);
	for (int idIndex = 0; idbList[idIndex] > 0; idIndex++)
	{
		HBITMAP bm = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(idbList[idIndex]));
		if (bm)
		{
			ImageList_AddMasked(hImageList, bm, RGB(192,192,192));
			DeleteObject((HGDIOBJ)bm);
		}
		else
		{
			ATLTRACE("Could not load bitmap %d:%d\n", idIndex, idbList[idIndex]);
		}
	}
	::SendMessage(hWndToolBar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
	int numTools = sizeof(mainButtons32)/sizeof(mainButtons32[0]);
	::SendMessage(hWndToolBar, TB_ADDBUTTONS, numTools, (LPARAM) mainButtons32);

	// add menu and tools to the rebar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	RECT rcKbd;
	rcKbd.left = 0;
	rcKbd.top = 0;
	rcKbd.right = rcMain.right - 50;
	rcKbd.bottom = 250;
	kbdWnd.Create(m_hWnd, rcKbd, "", WS_CHILD|WS_BORDER|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 9);
	kbdWnd.GetClientRect(&rcKbd);
	if (!kbdWnd.IsWindow())
		MessageBox("Didn't create keyboard!", "");

	RECT rcSplit;
	rcSplit.left = 0;
	rcSplit.top = 0;
	rcSplit.right = rcMain.right;
	rcSplit.bottom = rcMain.bottom - rcKbd.bottom;
	m_hWndClient = splitTop.Create(m_hWnd, rcSplit, "client", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0);
	if (m_hWndClient == NULL)
		MessageBox("Didn't create splitter!", "");
	splitTop.SetSplitterExtendedStyle(0, SPLIT_PROPORTIONAL);
	prjList.Create(splitTop, rcDefault, "list", WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, ID_PROJECT_LIST);
	if (!prjList.IsWindow())
		MessageBox("Didn't create project list!", "");
	tabView.Create(splitTop, rcDefault, "tabber", WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE, ID_TAB_WND);
	if (!tabView.IsWindow())
		MessageBox("Didn't create tab view!", "");
	splitTop.SetSplitterPanes(prjList, tabView, false);
	splitTop.SetSplitterPos(200, 1);

	UIAddToolBar(hWndToolBar);
	
	UpdateEditUI(-1);
	UpdateItemUI(0);
	UpdateProjectUI();

	UISetCheck(ID_VIEW_PROJECT, 0);
	UISetCheck(ID_VIEW_KEYBOARD, 0);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	EnablePanes();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = cmdBar.GetMenu();
	tabView.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));
	// This next line will result in the tabview taking
	// control of the title bar and displaying the document name
	// as the user tabs between editors. 
	//tabView.SetTitleBarWindow(m_hWnd);

	return 0;
}

//////////////// Internal functions ///////////////////////////////

/// Toggle windows based on the bVisible variable.
void MainFrame::EnablePanes()
{
	if (bVisible & PROJECT_PANE)
		splitTop.SetSinglePaneMode(SPLIT_PANE_NONE);
	else
		splitTop.SetSinglePaneMode(SPLIT_PANE_RIGHT);

	kbdWnd.ShowWindow(bVisible & KEYBOARD_PANE ? SW_SHOW : SW_HIDE);
	UISetCheck(ID_VIEW_PROJECT, bVisible & PROJECT_PANE ? 1 : 0);
	UISetCheck(ID_VIEW_KEYBOARD, bVisible & KEYBOARD_PANE ? 1 : 0);
}

/// Adjust the size of the child windows based on the current frame size.
/// This needs to be called whenever bVisible is changed or the frame
/// window size is changed.
// Note: something weird is going on with the keyboard window. When
// the frame is resized, the keyboard position jumps up and down, causing
// wierd flicker. Don't know yet what to do about that.
void MainFrame::UpdateLayout(BOOL bResizeBars)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	if (wp.showCmd == SW_MINIMIZE || wp.showCmd == SW_SHOWMINIMIZED)
		return;

	RECT rect = { 0 };
	GetClientRect(&rect);

	// subtract out the areas for rebar and status bar and keyboard
	// as appropriate. The remaining rectangle is the client area.

	if (m_hWndToolBar)
	{
		RECT rectTB = { 0 };
		::GetWindowRect(m_hWndToolBar, &rectTB);
		rect.top += rectTB.bottom - rectTB.top;
	}

	if (m_hWndStatusBar && ::IsWindowVisible(m_hWndStatusBar))
	{
		RECT rectSB = { 0 };
		::GetWindowRect(m_hWndStatusBar, &rectSB);
		rect.bottom -= rectSB.bottom - rectSB.top;
	}

	HDWP pos = 0;

	if (kbdWnd.IsWindow() && (bVisible & KEYBOARD_PANE))
	{
		RECT kbrect = { 0 };
		kbdWnd.GetWindowRect(&kbrect);
		int kbh = kbrect.bottom - kbrect.top;
		int kbtop = rect.bottom - kbh;
		if (kbtop > 0)
		{
			pos = ::BeginDeferWindowPos(2);
			if (pos)
				pos = kbdWnd.DeferWindowPos(pos, NULL, rect.left, kbtop, rect.right - rect.left, kbh, SWP_NOZORDER|SWP_NOACTIVATE/*|SWP_NOCOPYBITS*/);
			else
				kbdWnd.SetWindowPos(NULL, rect.left, kbtop, rect.right - rect.left, kbh, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
			rect.bottom -= kbh;
		}
		else
		{
			bVisible &= ~KEYBOARD_PANE;
			kbdWnd.ShowWindow(SW_HIDE);
			UISetCheck(ID_VIEW_KEYBOARD, 0);
		}
	}

	// client window size is now set appropriately for the 
	// other windows that are visible. 
	if (splitTop.IsWindow())
	{
		// We don't need to redraw because the splitter will Invalidate
		// the splitter bar areas when moving panes around. Without
		// SWP_NOREDRAW, we get a lot of flicker. However, without
		// redraw, the frame doesn't get redrawn. sigh
		if (pos)
			pos = splitTop.DeferWindowPos(pos, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE /*| SWP_NOCOPYBITS | SWP_NOREDRAW*/);
		else
			splitTop.SetWindowPos(NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE /*| SWP_NOCOPYBITS | SWP_NOREDRAW*/);
	}
	if (pos)
		::EndDeferWindowPos(pos);
}

/// Make a backup of the project file in the TEMP directory.
/// This is intended to save the project changes in case
/// the program exited abnormally with an open project.
/// Currently, recovery is not automatic.
/// The user must locate the project file and copy it somewhere
/// or open the temp file and then do a Save As...
void MainFrame::SaveTemp(int sv)
{
	char tmpdir[MAX_PATH];
	char tmppath[MAX_PATH];
	tmpdir[0] = 0;
	SHGetFolderPath(m_hWnd, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, tmpdir);
	snprintf(tmppath, MAX_PATH, "%s\\BasicSynth\\%d.bsprj", tmpdir, GetCurrentProcessId());
	if (sv)
	{
		if (theProject)
			theProject->SaveProject(tmppath);
	}
	else
	{
		DeleteFile(tmppath);
	}
}

/// Things to do after a project is opened or created.
void MainFrame::AfterOpenProject()
{
	if (errWnd)
		errWnd->Clear();

	if (!theProject)
	{
		MessageBox("No project after open project!", "Huh?", MB_OK);
		return; // huh?
	}

	SynthWidget::colorMap.Load(prjOptions.formsDir, prjOptions.colorsFile, 1);

	kbdWnd.Load();

	bsString path;
	theProject->GetProjectDir(path);
	if (path.Length() > 0)
		SetCurrentDirectory(path);
	prjList.Expand((HTREEITEM)theProject->GetPSData(), TVE_EXPAND|TVE_EXPANDPARTIAL);
	bVisible |= PROJECT_PANE|KEYBOARD_PANE;
	EnablePanes();
	UpdateLayout(0);
	UpdateProjectUI();
	UpdateEditUI(-1);
	UpdateItemUI(theProject);
	bsString title;
	title = _Module.ProductName;
	title += " - ";
	title += theProject->GetName();
	SetWindowText(title);
	SaveTemp(0); // discard any temp file.
	//StartTimer()
}

/// Enable various things based on the editor state.
void MainFrame::UpdateEditUI(int pg)
{
	unsigned long flags = 0;
	if (pg >= 0)
	{
		EditorView *vw = (EditorView *)tabView.GetPageData(pg);
		if (vw)
			flags = vw->EditState();
	}

	UIEnable(ID_ITEM_CLOSE, tabView.GetPageCount() > 0);
	
	UIEnable(ID_EDIT_COPY, flags & VW_ENABLE_COPY);
	UIEnable(ID_EDIT_CUT, flags & VW_ENABLE_CUT);
	UIEnable(ID_EDIT_PASTE, flags & VW_ENABLE_PASTE);
	UIEnable(ID_EDIT_UNDO, flags & VW_ENABLE_UNDO);
	UIEnable(ID_EDIT_REDO, flags & VW_ENABLE_REDO);
	UIEnable(ID_EDIT_GOTOLINE, flags & VW_ENABLE_GOTO);
	UIEnable(ID_EDIT_FIND, flags & VW_ENABLE_FIND);
	UIEnable(ID_EDIT_FINDNEXT, flags & VW_ENABLE_FIND);
	UIEnable(ID_EDIT_SELECTALL, flags & VW_ENABLE_SELALL);
	UIEnable(ID_ITEM_SAVE, flags & VW_ENABLE_FILE);
	UIEnable(ID_MARKER_SET, flags & VW_ENABLE_MARK);
	UIEnable(ID_MARKER_NEXT, flags & VW_ENABLE_UNMARK);
	UIEnable(ID_MARKER_PREV, flags & VW_ENABLE_UNMARK);
	UIEnable(ID_MARKER_CLEAR, flags & VW_ENABLE_UNMARK);
}

/// Enable functions based on project tree selection.
void MainFrame::UpdateItemUI(ProjectItem *pi)
{
	int itemFlags = 0;
	int editFlags = 0;
	if (pi)
	{
		itemFlags = pi->ItemActions();
		EditorView *vw = pi->GetEditor();
		if (vw)
			editFlags = vw->EditState();
	}
	UIEnable(ID_ITEM_ADD, itemFlags & ITM_ENABLE_ADD);
	UIEnable(ID_ITEM_NEW, itemFlags & ITM_ENABLE_NEW);
	UIEnable(ID_ITEM_EDIT, itemFlags & ITM_ENABLE_EDIT);
	UIEnable(ID_ITEM_COPY, itemFlags & ITM_ENABLE_COPY);
	UIEnable(ID_ITEM_REMOVE, itemFlags & ITM_ENABLE_REM);
	UIEnable(ID_ITEM_PROPERTIES, itemFlags & ITM_ENABLE_PROPS);
//	UIEnable(ID_ITEM_CLOSE, itemFlags & ITM_ENABLE_CLOSE && editFlags & VW_ENABLE_FILE);
	UIEnable(ID_ITEM_SAVE, itemFlags & ITM_ENABLE_SAVE && editFlags & VW_ENABLE_FILE);
	UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);
}

/// Enable functions associated with an open project.
void MainFrame::UpdateProjectUI()
{
	int enable = theProject != 0;
	UIEnable(ID_PROJECT_SAVE, enable);
	UIEnable(ID_PROJECT_PROPERTIES, enable);
	UIEnable(ID_PROJECT_GENERATE, enable);
	UIEnable(ID_PROJECT_PLAY, enable);
	UIEnable(ID_VIEW_PROJECT, enable);
	UIEnable(ID_VIEW_KEYBOARD, enable);
	UIEnable(ID_ITEM_ERRORS, enable);
	UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);
}

//////////////// Callbacks ///////////////////////////////

// Use position changed instead of WM_SIZE since it is more efficient.
LRESULT MainFrame::OnPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS *pos = (WINDOWPOS*)lParam;
	if (!(pos->flags & SWP_NOSIZE))
	{
		if (::IsWindowVisible(m_hWndToolBar))
		{
			::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
			::InvalidateRect(m_hWndToolBar, NULL, FALSE);
		}
		if (::IsWindowVisible(m_hWndStatusBar))
		{
			::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			// The status bar seems to correctly invalidate.
			//::InvalidateRect(m_hWndStatusBar, NULL, FALSE);
		}
		UpdateLayout();
	}
	return 0;
}

/// We recieved a WM_SIZE. Size changes are handled in OnPosChanged
/// and the DefaultWindowProc is bypassed. So, we shouldnt get any
/// WM_SIZE messages. However, we do get called here during the ShowWindow() at startup.
/// We need to override this so that the WTL base class does not attempt to adjust
/// the child windows layout with its funky recursive thingy.
LRESULT MainFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 1;
}

LRESULT MainFrame::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (theProject)
	{
		EditorView *ed = GetActiveEditor();
		if (ed)
			ed->Focus();
		else if (bVisible & PROJECT_PANE)
			prjList.SetFocus();
	}
	return 0;
}

/// WM_PAINT callback. 	All main window areas are covered by child windows.
/// That's why we don't need to draw anything here. Of course we still need
/// to handle the message in order to clear the repaint status of the window.
LRESULT MainFrame::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	EndPaint(&ps);
	return 0;
}

// Erase the background - see OnPaint.
LRESULT MainFrame::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

/// This is the last message we handled. Everything should have been closed in the
/// WM_CLOSE handler. We just disconnect the frame window from the message loop
/// filters. This is not absolutely necessary since this window is only destroyed
/// when we are about to exit. But in case things change...
LRESULT MainFrame::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

/// WM_ACTIVATE handler. Right now this does nothing. It could be used
/// to turn the player on/off to save unnecessary CPU use.
LRESULT MainFrame::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == WA_INACTIVE)
	{
		// stop player?
	}

	return 0;
}

LRESULT MainFrame::OnSuspend(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == PBT_APMSUSPEND)
	{
		StopPlayer();
		SaveTemp(1);
	}
	return 0;
}

LRESULT MainFrame::OnTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam)
	{
		StopPlayer();
		SaveTemp(1);
	}
	return 0;
}

LRESULT MainFrame::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// only do this if the generate dialog is not up
	if (theProject)
		SaveTemp(1);
	return 0;
}

LRESULT MainFrame::OnQueryTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Windows is trying to shutdown NOW...
	int n = MessageBox("Windows wants to shut down. Do it?", "Hey...", MB_YESNO);
	if (n == IDYES)
	{
		StopPlayer();
		CloseProject(1);
		SaveTemp(0);
		return 1;
	}
	return 0;
}

/// Menu Exit command selected. Turn it into a CLOSE command
LRESULT MainFrame::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PostMessage(WM_CLOSE);
	return 0;
}

/// Request by the user to close the application.
LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (CloseProject(1))
	{
		SaveTemp(0);
		mruList.WriteToRegistry(mruRegKey);
		WINDOWPLACEMENT plc;
		GetWindowPlacement(&plc);
		prjOptions.frmLeft = plc.rcNormalPosition.left;
		prjOptions.frmTop = plc.rcNormalPosition.top;
		prjOptions.frmWidth = plc.rcNormalPosition.right - plc.rcNormalPosition.left;
		prjOptions.frmHeight = plc.rcNormalPosition.bottom - plc.rcNormalPosition.top;
		prjOptions.frmMax = plc.showCmd == SW_SHOWMAXIMIZED;
		DestroyWindow();
	}
	return 0;
}

/// Menu Open selected.
LRESULT MainFrame::OnOpenProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (OpenProject(0))
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddToList(path);
		AfterOpenProject();
	}

	return 0;
}

/// Menu Save command selected.
LRESULT MainFrame::OnSaveProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveProject();
	return 0;
}

/// Menu Save As command selected.
LRESULT MainFrame::OnSaveProjectAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveProjectAs();
	return 0;
}

/// File selected from the MRU list.
LRESULT MainFrame::OnRecentProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char fname[MAX_PATH];
	if (mruList.GetFromList(wID, fname, MAX_PATH))
	{
		SynthProject::NormalizePath(fname);
		if (OpenProject(fname))
		{
			AfterOpenProject();
			mruList.MoveToTop(wID);
		}
	}
	return 0;
}

/// Menu New command selected.
LRESULT MainFrame::OnNewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (NewProject())
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddToList(path);
		AfterOpenProject();
	}

	return 0;
}

/// OnItem* functions handle various commands associated with the currently
/// selected project tree item. With a few exceptions, we just need to call
/// the generic project frame function.
LRESULT MainFrame::OnEditItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditItem();
	return 0;
}

LRESULT MainFrame::OnSaveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveItem();
	return 0;
}

LRESULT MainFrame::OnCloseItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CloseItem();
	return 0;
}

LRESULT MainFrame::OnRemoveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	RemoveItem();
	return 0;
}

LRESULT MainFrame::OnCopyItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CopyItem();
	return 0;
}

LRESULT MainFrame::OnItemProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ItemProperties();
	return 0;
}

LRESULT MainFrame::OnNewItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	NewItem();
	return 0;
}

LRESULT MainFrame::OnAddItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	AddItem();
	return 0;
}

LRESULT MainFrame::OnItemErrors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!errWnd)
	{
		errWnd = new ScoreErrorsDlg;
		errWnd->Create(m_hWnd);
		errWnd->SetWindowPos(NULL, 100, 200, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		errWnd->ShowWindow(SW_SHOW);
		UISetCheck(ID_ITEM_ERRORS, 1);
	}
	else
	{
		errWnd->DestroyWindow();
		delete errWnd;
		errWnd = 0;
		UISetCheck(ID_ITEM_ERRORS, 0);
	}
	return 0;
}

/// Show the generate dialog to produce sound output.
LRESULT MainFrame::OnProjectGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Generate(0, -1);
	return 0;
}

LRESULT MainFrame::OnProjectPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (kbdWnd.IsRunning())
		StopPlayer();
	else
		StartPlayer();
	return 0;
}

LRESULT MainFrame::OnProjectOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OptionsDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

/// OnEdit* functions are passed to the current editor.
LRESULT MainFrame::OnEditUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditUndo();
	return 0;
}

LRESULT MainFrame::OnEditRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditRedo();
	return 0;
}

LRESULT MainFrame::OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditCopy();
	return 0;
}

LRESULT MainFrame::OnEditCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditCut();
	return 0;
}

LRESULT MainFrame::OnEditPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditPaste();
	return 0;
}

LRESULT MainFrame::OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditSelectAll();
	return 0;
}

LRESULT MainFrame::OnEditFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditFind();
	return 0;
}

LRESULT MainFrame::OnEditFindNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditFindNext();
	return 0;
}

LRESULT MainFrame::OnEditGotoLine(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EditGoto();
	return 0;
}

LRESULT MainFrame::OnMarkerSet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MarkerSet();
	return 0;
}
LRESULT MainFrame::OnMarkerNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MarkerNext();
	return 0;
}
LRESULT MainFrame::OnMarkerPrev(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MarkerPrev();
	return 0;
}

LRESULT MainFrame::OnMarkerClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MarkerClear();
	return 0;
}

/// OnView* functions toggle various child windows.
LRESULT MainFrame::OnViewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= PROJECT_PANE;
	if (bVisible & PROJECT_PANE)
	{
		UISetCheck(ID_VIEW_PROJECT, 1);
		splitTop.SetSinglePaneMode(SPLIT_PANE_NONE);
		UpdateItemUI(GetSelectedNode());
	}
	else
	{
		UISetCheck(ID_VIEW_PROJECT, 0);
		splitTop.SetSinglePaneMode(SPLIT_PANE_RIGHT);
		UpdateItemUI(NULL);
	}
	return 0;
}

LRESULT MainFrame::OnViewKeyboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= KEYBOARD_PANE;
	UISetCheck(ID_VIEW_KEYBOARD, bVisible & KEYBOARD_PANE ? 1 : 0);
	kbdWnd.ShowWindow(bVisible & KEYBOARD_PANE ? SW_SHOW : SW_HIDE);
	UpdateLayout(0);
	return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bVisible ^= TOOLBAR_PANE;
	int show = bVisible & TOOLBAR_PANE ? 1 : 0;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, show);
	UISetCheck(ID_VIEW_TOOLBAR, show);
	UpdateLayout(1);
	return 0;
}

LRESULT MainFrame::OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL show = !::IsWindowVisible(m_hWndStatusBar);
	if (show)
		::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
	::ShowWindow(m_hWndStatusBar, show ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, show);
	UpdateLayout(1);
	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CAboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnHelpContents(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!SynthFileExists(prjOptions.helpFile))
	{
		if (!BrowseFile(1, prjOptions.helpFile, "Help Files|*.chm|", "chm"))
			return 0;
	}
	HWND h = HtmlHelp(m_hWnd, prjOptions.helpFile, HH_DISPLAY_TOC, NULL);
	if (h == 0)
		Alert("Cannot locate help file.", "Ooops");
	return 0;
}

/// OnWindow* and OnPage functions handle messages from the tab view window.
LRESULT MainFrame::OnWindowClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CloseFile();
	return 0;
}

LRESULT MainFrame::OnWindowSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveFile();
	return 0;
}

LRESULT MainFrame::OnWindowCloseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CloseAllEditors();
	UpdateEditUI(-1);
	return 0;
}

LRESULT MainFrame::OnNextPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = tabView.GetActivePage();
	if (++pg >= tabView.GetPageCount())
		pg = 0;
	tabView.SetActivePage(pg);
	return 0;
}

LRESULT MainFrame::OnPrevPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = tabView.GetActivePage();
	if (--pg < 0)
		pg = tabView.GetPageCount() - 1;
	tabView.SetActivePage(pg);
	return 0;
}

LRESULT MainFrame::OnWindowActivate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int pg = wID - ID_WINDOW_TABFIRST;
	tabView.SetActivePage(pg);
	UpdateEditUI(pg);
	return 0;
}

LRESULT MainFrame::OnPageActivated(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int pg = pnmh->idFrom;
	if (pg >= 0)
	{
		EditorView *vw = (EditorView *)tabView.GetPageData(pg);
		if (vw)
		{
			ProjectItem *itm = vw->GetItem();
			if (itm)
			{
				prjList.SelectItem((HTREEITEM)itm->GetPSData());
				switch (itm->GetType())
				{
				case PRJNODE_INSTR:
				case PRJNODE_LIBINSTR:
					kbdWnd.SelectInstrument(((InstrItem *)itm)->GetConfig());
					break;
				}
			}
		}
	}
	UpdateEditUI(pg);
	return 0;
}

LRESULT MainFrame::OnTabContextMenu(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	TBVCONTEXTMENUINFO *cmi = (TBVCONTEXTMENUINFO*)pnmh;
	EditorView *ed = (EditorView *) tabView.GetPageData(cmi->hdr.idFrom);
	if (ed)
	{
		UpdateItemUI(ed->GetItem());
		CMenu menu;
		menu.LoadMenu(IDR_POPUP);
		HMENU pop = menu.GetSubMenu(0);
		if (pop)
			::TrackPopupMenu(pop, TPM_RIGHTBUTTON|TPM_VERTICAL, cmi->pt.x, cmi->pt.y, 0, m_hWnd, NULL);
	}
	return 0;
}

////////////////////////////// Project tree functions /////////////////////////////////

ProjectItem *MainFrame::GetClickedItem()
{
	TVHITTESTINFO ht;
	GetCursorPos(&ht.pt);
	prjList.ScreenToClient(&ht.pt);
	ht.flags = 0;
	ht.hItem = 0;
	prjList.HitTest(&ht);
	if (ht.flags & TVHT_ONITEM)
		return (ProjectItem*)prjList.GetItemData(ht.hItem);
	return 0;
}

LRESULT MainFrame::OnPrjRClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int subMnu = -1;
	ProjectItem *itm = GetClickedItem();
	if (itm)
	{
		UpdateItemUI(itm);
		prjList.SelectItem((HTREEITEM)itm->GetPSData());
		switch (itm->GetType())
		{
		case PRJNODE_PROJECT:
			subMnu = 1;
			break;
		case PRJNODE_WAVEOUT:
			subMnu = 3;
			break;
		case PRJNODE_SYNTH:
			subMnu = 2;
			break;
		case PRJNODE_NOTELIST:
		case PRJNODE_SEQLIST:
		case PRJNODE_TEXTLIST:
		case PRJNODE_LIBLIST:
		case PJRNODE_SCRIPTLIST:
		case PRJNODE_WVFLIST:
		case PRJNODE_SBLIST:
			subMnu = 4;
			break;
		case PRJNODE_LIB:
			subMnu = 6;
			break;
		default:
			subMnu = 5;
			break;
		}
	}
	else
		subMnu = 0;
	if (subMnu >= 0)
	{
		CMenu menu;
		menu.LoadMenu(IDR_POPUP);
		HMENU pop = menu.GetSubMenu(subMnu);
		if (pop)
		{
			POINT scrn;
			GetCursorPos(&scrn);
			::TrackPopupMenu(pop, TPM_RIGHTBUTTON|TPM_VERTICAL, scrn.x, scrn.y, 0, m_hWnd, NULL);
		}
	}
	return 0;
}

LRESULT MainFrame::OnPrjSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW *nmtv = (NMTREEVIEW*)pnmh;
	UpdateItemUI((ProjectItem*)nmtv->itemNew.lParam);
	return 0;
}

LRESULT MainFrame::OnPrjDblclick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
// If you call OpenEditor directly, you won't get the 
// focus moved to the new editor (???). The tree view
// takes it back.... (arrrggghhhh...)
//	OpenEditor((ProjectItem *)GetClickedItem());
	PostMessage(WM_COMMAND, ID_ITEM_EDIT, 0);
	return 1;
}

LRESULT MainFrame::OnPrjDelItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTREEVIEW *nmtv = (NMTREEVIEW*)pnmh;
	ProjectItem *pi = (ProjectItem *)nmtv->itemOld.lParam;
	if (pi)
		pi->Release();
	return 1;
}

void MainFrame::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (itm == 0)
		return;

	ProjectItem *parent = itm->GetParent();
	HTREEITEM htparent;
	if (parent)
		htparent = (HTREEITEM) parent->GetPSData();
	else
		htparent = TVI_ROOT;

	HTREEITEM htafter;
	if (sib)
		htafter = (HTREEITEM) sib->GetPSData();
	else
		htafter = TVI_LAST;

	HTREEITEM htitm = prjList.InsertItemUTF8(htparent, htafter, itm->GetName(), (void*)itm);
	itm->SetPSData((void*)htitm);

	itm->AddRef();
}

void MainFrame::SelectNode(ProjectItem *pi)
{
	if (pi)
		prjList.SelectItem((HTREEITEM)pi->GetPSData());
}

void MainFrame::RemoveNode(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return;

	if (itm->GetType() == PRJNODE_NOTELIST)
	{
		if (errWnd)
			errWnd->RemoveItem(itm);
	}

	HTREEITEM ht = (HTREEITEM) itm->GetPSData();
	itm->SetPSData(0);
//	prjList.SetItemData(ht, 0);
	prjList.DeleteItem(ht);
}

void MainFrame::UpdateNode(ProjectItem *itm)
{
	if (itm)
	{
		//prjList.SetItemText((HTREEITEM) itm->GetPSData(), itm->GetName());
		prjList.SetItemTextUTF8((HTREEITEM) itm->GetPSData(), itm->GetName());
	}
}

void MainFrame::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
	if (!itm || !prev)
		return;

	itm->AddRef();
	RemoveNode(itm);
	AddNode(itm, prev);
	itm->Release();
}

ProjectItem *MainFrame::GetSelectedNode()
{
	HTREEITEM itm = prjList.GetSelectedItem();
	if (itm != NULL)
		return (ProjectItem *)prjList.GetItemData(itm);
	return 0;
}

ProjectItem *MainFrame::FirstChild(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return 0;

	HTREEITEM child = prjList.GetChildItem((HTREEITEM) itm->GetPSData());
	if (child)
		return (ProjectItem *) prjList.GetItemData(child);
	return 0;
}

ProjectItem *MainFrame::NextSibling(ProjectItem *itm)
{
	if (itm == 0 || itm->GetPSData() == 0)
		return 0;

	HTREEITEM child = prjList.GetNextSiblingItem((HTREEITEM) itm->GetPSData());
	if (child)
		return (ProjectItem *) prjList.GetItemData(child);
	return 0;
}

void MainFrame::RemoveAll()
{
	prjList.DeleteAllItems();
	//theProject = 0;
}

int MainFrame::CloseAllEditors()
{
	tabView.ShowWindow(SW_HIDE);
	int ret = 1;
	int ndx = 0;
	while (ndx < tabView.GetPageCount())
	{
		EditorView *vw = (EditorView*)tabView.GetPageData(0);
		ProjectItem *pi = vw->GetItem();
		if (!pi->CloseItem())
		{
			ret = 0;
			ndx++;
		}
	}
	tabView.ShowWindow(SW_SHOW);
	UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);
	return ret;
}

///////////////////// Platform specific frame functions ////////////////////////////////////

int MainFrame::Verify(const char *msg, const char *title)
{
	switch (MessageBox(msg, title, MB_YESNOCANCEL|MB_ICONQUESTION))
	{
	case IDYES:
		return 1;
	case IDNO:
		return 0;
	}
	return -1;
}

PropertyBox *MainFrame::CreatePropertyBox(ProjectItem *pi, int type)
{
	PropertyBox *pb = 0;
	if (type == 0)
		type = pi->GetType();
	switch (type)
	{
	case PRJNODE_PROJECT:
	case PRJNODE_SYNTH:
	case PRJNODE_WAVEOUT:
		{
			ProjectPropertiesDlg *p = new ProjectPropertiesDlg;
			p->SetItem(pi);
			pb = static_cast<PropertyBox*>(p);
		}
		break;
	case PRJNODE_MIXER:
		{
			MixerSetupDlg *m = new MixerSetupDlg;
			m->SetItem(pi);
			pb = static_cast<PropertyBox*>(m);
		}
		break;
	case PRJNODE_REVERB:
	case PRJNODE_FLANGER:
	case PRJNODE_ECHO:
		{
			EffectsSetupDlg *e = new EffectsSetupDlg;
			e->SetItem(pi);
			pb = static_cast<PropertyBox*>(e);
		}
		break;
	case PRJNODE_NOTELIST:
	case PRJNODE_FILELIST:
	case PRJNODE_SEQLIST:
	case PRJNODE_TEXTLIST:
	case PJRNODE_SCRIPTLIST:
		{
			FilelistOrder *f = new FilelistOrder;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_NOTEFILE:
	case PRJNODE_SEQFILE:
	case PRJNODE_TEXTFILE:
	case PRJNODE_SCRIPT:
	case PRJNODE_MIDIFILE:
		{
			FilePropertiesDlg *f = new FilePropertiesDlg;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_SOUNDBANK:
		{
			SoundBankPropertiesDlg *f = new SoundBankPropertiesDlg;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_LIBLIST:
		break;
	case PRJNODE_LIB:
		{
			LibPropertiesDlg *f = new LibPropertiesDlg;
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_SELINSTR:
		{
			InstrSelectDlg *i = new InstrSelectDlg;
			i->SetItem(pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_INSTRLIST:
		break;
	case PRJNODE_LIBINSTR:
	case PRJNODE_INSTR:
		{
			InstrPropertiesDlg *i = new InstrPropertiesDlg;
			i->SetItem(pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_WVTABLE:
		{
			NamePropertiesDlg *n = new NamePropertiesDlg;
			n->SetItem(pi);
			pb = static_cast<PropertyBox*>(n);
		}
		break;
	case PRJNODE_WVFILE:
		{
			WavefilePropertiesDlg *w = new WavefilePropertiesDlg;
			w->SetItem(pi);
			pb = static_cast<PropertyBox*>(w);
		}
		break;
	case PRJNODE_SBLIST:
		break;
	}
	return pb;
}

FormEditor *MainFrame::CreateFormEditor(ProjectItem *pi)
{
	ScrollForm *scrl = new ScrollForm(0);
	scrl->Create(tabView, rcDefault, "scroll", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL, 0, ID_SCROLL_WND, 0);
	FormEditorWin *form = new FormEditorWin;
	form->SetItem(pi);
	Size sz(0,0);
	form->GetSize(sz);
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = sz.Width+1;
	rc.bottom = sz.Height+1;
	form->Create(scrl->m_hWnd, rc, "FormEdit", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, ID_FORM_WND);
	form->SetScrollWin(scrl);
	form->SetPSData((void*)form->m_hWnd);
	scrl->SetForm(form);
	tabView.AddPage(scrl->m_hWnd, pi->GetName(), -1, static_cast<EditorView*>(form));
	::SetFocus(form->m_hWnd);
	UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);
	return form;
}

TextEditor *MainFrame::CreateTextEditor(ProjectItem *pi)
{
	if (!pi)
		return 0;
	TextEditorWin *ed = new TextEditorWin;
	if (ed)
	{
		ed->SetItem(pi);
		ed->Create(tabView);
		ed->SetPSData((void*)ed->m_hWnd);
		tabView.AddPage(ed->m_hWnd, pi->GetName(), -1, static_cast<EditorView*>(ed));
		::SetFocus(ed->m_hWnd);
		UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);
		return ed;
	}
	return 0;
}

// NOTE: for now, we only create editors as tabs.
// If that changes, then search other info to find
// the editor and show/close it.

int MainFrame::CloseEditor(ProjectItem *itm)
{
	if (!itm)
		return 0;

	EditorView *vw = itm->GetEditor();
	if (!vw)
		return 0;
	int pg;
	for (pg = 0; pg < tabView.GetPageCount(); pg++)
	{
		if ((EditorView*)tabView.GetPageData(pg) == vw)
		{
			tabView.RemovePage(pg);
			break;
		}
	}
	itm->SetEditor(0);
	delete vw;
	UIEnable(ID_WINDOW_CLOSE_ALL, tabView.GetPageCount() > 0);

	return 1;
}

EditorView *MainFrame::GetActiveEditor()
{
	int pg = tabView.GetActivePage();
	if (pg >= 0)
		return (EditorView*) tabView.GetPageData(pg);
	return 0;
}

int MainFrame::OpenEditor(ProjectItem *itm)
{
	if (!itm)
		return 0;
	EditorView *vw = itm->GetEditor();
	if (vw)
	{
		if (vw == GetActiveEditor())
		{
			::SetFocus((HWND)vw->GetPSData());
			return 1;
		}
		int pgcnt = tabView.GetPageCount();
		for (int pg = 0; pg < pgcnt; pg++)
		{
			if ((EditorView*)tabView.GetPageData(pg) == vw)
			{
				tabView.SetActivePage(pg);
				return 1;
			}
		}
		itm->SetEditor(0);
	}
	if (itm->EditItem())
		return 1;

	if (itm->ItemProperties())
		UpdateNode(itm);

	return 0;
}

int MainFrame::SaveAllEditors(int query)
{
	for (int pg = 0; pg < tabView.GetPageCount(); pg++)
	{
		EditorView *vw = (EditorView *) tabView.GetPageData(pg);
		if (vw->IsChanged())
		{
			if (query)
			{
				bsString msg;
				msg = tabView.GetPageTitle(pg);
				msg += " has changed. Save?";
				int res = Verify(msg, "Wait...");
				if (res == 0) // no
					continue;
				if (res == -1) // cancel
					return 0;
			}
			ProjectItem *itm = vw->GetItem();
			if (itm)
				itm->SaveItem();
		}
	}
	return 1;
}

void MainFrame::EditStateChanged()
{
	int pg = tabView.GetActivePage();
	UpdateEditUI(pg);
}

int MainFrame::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	// append All Files and convert | to null
	wchar_t spcbuf[MAX_PATH];
	memset(spcbuf, 0, sizeof(spcbuf));
	::MultiByteToWideChar(CP_UTF8, 0, spec, (int)strlen(spec)+1, spcbuf, MAX_PATH);
	if (wcsstr(spcbuf, L"*.*") == NULL)
		wcscat(spcbuf, L"All Files|*.*|");
	wchar_t *pipe = spcbuf;
	while ((pipe = wcschr(pipe, '|')) != 0)
		*pipe++ = '\0';
	
	wchar_t filebuf[MAX_PATH];
	memset(filebuf, 0, sizeof(filebuf));
	::MultiByteToWideChar(CP_UTF8, 0, file, (int)strlen(file)+1, filebuf, MAX_PATH);
	// the term 'brain damage' comes to mind here...
	pipe = filebuf;
	while ((pipe = wcschr(pipe, '/')) != 0)
		*pipe++ = '\\';

	wchar_t extbuf[MAX_PATH];
	memset(extbuf, 0, sizeof(extbuf));
	::MultiByteToWideChar(CP_UTF8, 0, ext, (int)strlen(ext)+1, extbuf, MAX_PATH);

	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_NOCHANGEDIR;
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = spcbuf;
	ofn.lpstrDefExt = extbuf; 
	ofn.lpstrFile = filebuf;
	ofn.nMaxFile = MAX_PATH;
	int result;
	if (open)
	{
		result = GetOpenFileNameW(&ofn);
	}
	else
	{
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		result = GetSaveFileNameW(&ofn);
	}
	if (result)
	{
		memset(file, 0, MAX_PATH);
		::WideCharToMultiByte(CP_UTF8, 0, filebuf, (int)wcslen(filebuf), file, MAX_PATH, NULL, NULL);
		SynthProject::NormalizePath(file);
	}
	return result;
}

int MainFrame::BrowseFile(int open, bsString& file, const char *spec, const char *ext)
{
	char fnbuf[MAX_PATH];
	strncpy(fnbuf, file, MAX_PATH);
	int result = BrowseFile(open, fnbuf, spec, ext);
	if (result)
		file = fnbuf;
	return result;
}

int MainFrame::QueryValue(const char *prompt, char *value, int len)
{
	int pg = tabView.GetActivePage();
	QueryValueDlg dlg(prompt, value, len);
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		if (pg >= 0)
			::SetFocus(tabView.GetPageHWND(pg));
		return 1;
	}
	return 0;
}

int MainFrame::Alert(const char *msg, const char *title)
{
	MessageBox(msg, title ? title : "Wait...", MB_OK|MB_ICONHAND);
	return 0;
}

void MainFrame::InitPlayer()
{
	kbdWnd.InitInstrList();
	kbdWnd.UpdateChannels();
}

int MainFrame::StopPlayer()
{
	if (kbdWnd.IsWindow())
		return kbdWnd.Stop();
	return 0;
}

void MainFrame::StartPlayer()
{
	if (kbdWnd.IsWindow())
		kbdWnd.Start();
}

void MainFrame::ClearPlayer()
{
	if (kbdWnd.IsWindow())
		kbdWnd.Clear();
}

void MainFrame::InstrAdded(InstrConfig *inst)
{
	kbdWnd.AddInstrument(inst);
}

void MainFrame::InstrRemoved(InstrConfig *inst)
{
	kbdWnd.RemoveInstrument(inst);
}

void MainFrame::InstrChanged(InstrConfig *inst)
{
	kbdWnd.UpdateInstrument(inst);
}

void MainFrame::MixerChanged()
{
	kbdWnd.UpdateChannels();
}

void MainFrame::GenerateStarted()
{
	if (errWnd)
		errWnd->Clear();
}

void MainFrame::GenerateFinished()
{
	if (errWnd)
		errWnd->Refresh();
}

void MainFrame::Generate(int autoStart, int todisk)
{
	if (!theProject)
		return;

	int wasPlaying = StopPlayer();
	GenerateDlg dlg(autoStart, todisk);
	dlg.DoModal(m_hWnd);
	if (wasPlaying)
		StartPlayer();
}
