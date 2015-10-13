//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "MainFrame.h"
#include "AboutDlg.h"
#include "ProjectItemDlg.h"
#include "ProjectPropertiesDlg.h"
#include "OptionsDlg.h"
#include "FormEditor.h"
#include "QueryValueDlg.h"
#include "GenerateDlg.h"
#include "ItemPropertiesDlg.h"
#include "MixerSetupDlg.h"
#if wxUSE_STC
#include "TextEditorStc.h"
#else
#include "TextEditor.h"
#endif

#ifndef USE_AUTOSAVE
#define USE_AUTOSAVE 0
#endif
#define AUTOSAVE_TIMER_ID 999

BEGIN_EVENT_TABLE(MainTree, wxTreeCtrl)
	EVT_LEFT_DCLICK(MainTree::OnLftDClick)
END_EVENT_TABLE()

void MainTree::OnLftDClick(wxMouseEvent& evt)
{
	frame->OnLftDClick(evt);
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_CLOSE(MainFrame::OnClose)
	EVT_SIZE(MainFrame::OnSize)
//	EVT_ERASE_BACKGROUND(MainFrame::OnErase)
	EVT_SET_FOCUS(MainFrame::OnSetFocus)
	EVT_MENU(wxID_UNDO, MainFrame::OnEditUndo)
	EVT_UPDATE_UI(wxID_UNDO, MainFrame::OnUpdateEditUndo)
	EVT_MENU(XRCID("ID_EDIT_REDO"), MainFrame::OnEditRedo)
	EVT_UPDATE_UI(XRCID("ID_EDIT_REDO"), MainFrame::OnUpdateEditRedo)
	EVT_MENU(wxID_COPY, MainFrame::OnEditCopy)
	EVT_UPDATE_UI(wxID_COPY, MainFrame::OnUpdateEditCopy)
	EVT_MENU(wxID_CUT, MainFrame::OnEditCut)
	EVT_UPDATE_UI(wxID_CUT, MainFrame::OnUpdateEditCut)
	EVT_MENU(wxID_PASTE, MainFrame::OnEditPaste)
	EVT_UPDATE_UI(wxID_PASTE, MainFrame::OnUpdateEditPaste)
	EVT_MENU(XRCID("ID_EDIT_FIND"), MainFrame::OnEditFind)
	EVT_UPDATE_UI(XRCID("ID_EDIT_FIND"), MainFrame::OnUpdateEditFind)
	EVT_MENU(XRCID("ID_EDIT_FINDNEXT"), MainFrame::OnEditFindNext)
	EVT_UPDATE_UI(XRCID("ID_EDIT_FINDNEXT"), MainFrame::OnUpdateEditFindNext)
	EVT_MENU(XRCID("ID_EDIT_SELECTALL"), MainFrame::OnEditSelectAll)
	EVT_UPDATE_UI(XRCID("ID_EDIT_SELECTALL"), MainFrame::OnUpdateEditSelectAll)
	EVT_MENU(XRCID("ID_EDIT_GOTOLINE"), MainFrame::OnEditGotoLine)
	EVT_UPDATE_UI(XRCID("ID_EDIT_GOTOLINE"), MainFrame::OnUpdateEditGotoLine)
	EVT_MENU(XRCID("ID_MARKER_SET"), MainFrame::OnMarkerSet)
	EVT_UPDATE_UI(XRCID("ID_MARKER_SET"), MainFrame::OnUpdateEditMark)
	EVT_MENU(XRCID("ID_MARKER_PREV"), MainFrame::OnMarkerPrev)
	EVT_UPDATE_UI(XRCID("ID_MARKER_PREV"), MainFrame::OnUpdateEditMark)
	EVT_MENU(XRCID("ID_MARKER_NEXT"), MainFrame::OnMarkerNext)
	EVT_UPDATE_UI(XRCID("ID_MARKER_NEXT"), MainFrame::OnUpdateEditMark)
	EVT_MENU(XRCID("ID_MARKER_CLEAR"), MainFrame::OnMarkerClear)
	EVT_UPDATE_UI(XRCID("ID_MARKER_CLEAR"), MainFrame::OnUpdateEditMark)
	EVT_MENU(XRCID("ID_PROJECT_NEW"), MainFrame::OnNewProject)
	EVT_MENU(XRCID("ID_PROJECT_OPEN"), MainFrame::OnOpenProject)
	EVT_MENU(XRCID("ID_PROJECT_SAVE"), MainFrame::OnSaveProject)
	EVT_UPDATE_UI(XRCID("ID_PROJECT_SAVE"), MainFrame::OnUpdateProjectEnable)
	EVT_MENU(XRCID("ID_PROJECT_SAVEAS"), MainFrame::OnSaveProjectAs)
	EVT_UPDATE_UI(XRCID("ID_PROJECT_SAVEAS"), MainFrame::OnUpdateProjectEnable)
	EVT_MENU(XRCID("ID_PROJECT_GENERATE"), MainFrame::OnProjectGenerate)
	EVT_UPDATE_UI(XRCID("ID_PROJECT_GENERATE"), MainFrame::OnUpdateProjectEnable)
	EVT_MENU(XRCID("ID_PROJECT_PLAY"), MainFrame::OnProjectPlay)
	EVT_UPDATE_UI(XRCID("ID_PROJECT_PLAY"), MainFrame::OnUpdateProjectEnable)
	EVT_MENU(XRCID("ID_PROJECT_OPTIONS"), MainFrame::OnProjectOptions)
	//EVT_UPDATE_UI(XRCID("ID_PROJECT_OPTIONS"), MainFrame::OnUpdateProjectEnable)
	EVT_MENU(XRCID("ID_ITEM_NEW"), MainFrame::OnNewItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_NEW"), MainFrame::OnUpdateNewItem)
	EVT_MENU(XRCID("ID_ITEM_ADD"), MainFrame::OnAddItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_ADD"), MainFrame::OnUpdateAddItem)
	EVT_MENU(XRCID("ID_ITEM_EDIT"), MainFrame::OnEditItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_EDIT"), MainFrame::OnUpdateEditItem)
	EVT_MENU(XRCID("ID_ITEM_SAVE"), MainFrame::OnSaveItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_SAVE"), MainFrame::OnUpdateSaveItem)
	EVT_MENU(XRCID("ID_ITEM_CLOSE"), MainFrame::OnCloseItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_CLOSE"), MainFrame::OnUpdateCloseItem)
	EVT_MENU(XRCID("ID_ITEM_COPY"), MainFrame::OnCopyItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_COPY"), MainFrame::OnUpdateCopyItem)
	EVT_MENU(XRCID("ID_ITEM_REMOVE"), MainFrame::OnRemoveItem)
	EVT_UPDATE_UI(XRCID("ID_ITEM_REMOVE"), MainFrame::OnUpdateRemoveItem)
	EVT_MENU(XRCID("ID_ITEM_PROPERTIES"), MainFrame::OnItemProperties)
	EVT_UPDATE_UI(XRCID("ID_ITEM_PROPERTIES"), MainFrame::OnUpdateItemProperties)
	EVT_MENU(XRCID("ID_ITEM_ERRORS"), MainFrame::OnItemErrors)
	EVT_UPDATE_UI(XRCID("ID_ITEM_ERRORS"), MainFrame::OnUpdateItemErrors)
	EVT_MENU(XRCID("ID_PROJECT_PROPERTIES"), MainFrame::OnItemProperties)
	EVT_MENU(XRCID("ID_VIEW_PROJECT"), MainFrame::OnViewProject)
	EVT_UPDATE_UI(XRCID("ID_VIEW_PROJECT"), MainFrame::OnUpdateViewProject)
	EVT_MENU(XRCID("ID_VIEW_KEYBOARD"), MainFrame::OnViewKeyboard)
	EVT_UPDATE_UI(XRCID("ID_VIEW_KEYBOARD"), MainFrame::OnUpdateViewKeyboard)
	EVT_MENU(XRCID("ID_VIEW_TOOLBAR"), MainFrame::OnViewToolBar)
	EVT_UPDATE_UI(XRCID("ID_VIEW_TOOLBAR"), MainFrame::OnUpdateViewToolBar)
	EVT_MENU(XRCID("ID_VIEW_STATUS_BAR"), MainFrame::OnViewStatusBar)
	EVT_UPDATE_UI(XRCID("ID_VIEW_STATUS_BAR"), MainFrame::OnUpdateViewStatusBar)
	EVT_MENU(XRCID("ID_WINDOW_CLOSE_ALL"), MainFrame::OnCloseAll)
	EVT_UPDATE_UI(XRCID("ID_WINDOW_CLOSE_ALL"), MainFrame::OnUpdateCloseAll)
	EVT_MENU(wxID_ABOUT, MainFrame::OnAppAbout)
	EVT_MENU(XRCID("ID_HELP_CONTENTS"), MainFrame::OnHelpContents)
	EVT_MENU(wxID_EXIT, MainFrame::OnExit)
	EVT_MENU(XRCID("ID_NEXT_PANE"), MainFrame::OnNextPane)
	EVT_UPDATE_UI(XRCID("ID_NEXT_PANE"), MainFrame::OnUpdateNextPane)
	EVT_MENU(XRCID("ID_PREV_PANE"), MainFrame::OnPrevPane)
	EVT_UPDATE_UI(XRCID("ID_PREV_PANE"), MainFrame::OnUpdateNextPane)
    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MainFrame::OnMRUFile)
	EVT_TREE_SEL_CHANGED(ID_PROJECT_WND, MainFrame::OnPrjSelChanged)
	EVT_TREE_ITEM_MENU(ID_PROJECT_WND, MainFrame::OnPrjRClick)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_TAB_WND, MainFrame::OnPageActivated)
	EVT_TIMER(AUTOSAVE_TIMER_ID, MainFrame::OnTimer)
END_EVENT_TABLE()

void MainFrame::InitAccel()
{
	wxAcceleratorEntry entries[23];
	entries[0].Set(wxACCEL_CTRL,   (int) 'c',     wxID_COPY);
	entries[1].Set(wxACCEL_CTRL,   (int) 'x',     wxID_CUT);
	entries[2].Set(wxACCEL_CTRL,   (int) 'v',     wxID_PASTE);
	entries[3].Set(wxACCEL_CTRL,   (int) 'Z',     wxID_UNDO);
	entries[4].Set(wxACCEL_CTRL,   (int) 'y',     XRCID("ID_EDIT_REDO"));
	entries[5].Set(wxACCEL_CTRL,   (int) 'f',     XRCID("ID_EDIT_FIND"));
	entries[6].Set(wxACCEL_CTRL,   (int) 'h',     XRCID("ID_EDIT_FIND"));
	entries[7].Set(wxACCEL_CTRL,   (int) 'g',     XRCID("ID_EDIT_GOTOLINE"));
	entries[8].Set(wxACCEL_CTRL,   (int) 'a',     XRCID("ID_EDIT_SELECTALL"));
	entries[9].Set(wxACCEL_CTRL+wxACCEL_SHIFT,   (int) 'n',     XRCID("ID_PROJECT_NEW"));
	entries[10].Set(wxACCEL_CTRL+wxACCEL_SHIFT,  (int) 'o',     XRCID("ID_PROJECT_OPEN"));
	entries[10].Set(wxACCEL_CTRL+wxACCEL_SHIFT,  (int) 's',     XRCID("ID_PROJECT_SAVE"));
	entries[11].Set(wxACCEL_CTRL,  (int) 's',     XRCID("ID_ITEM_SAVE"));
	entries[12].Set(wxACCEL_CTRL,   WXK_F2,       XRCID("ID_MARKER_SET"));
	entries[13].Set(wxACCEL_NORMAL, WXK_F2,       XRCID("ID_MARKER_NEXT"));
	entries[14].Set(wxACCEL_SHIFT,  WXK_F2,       XRCID("ID_MARKER_PREV"));
	entries[15].Set(wxACCEL_NORMAL, WXK_F3,       XRCID("ID_EDIT_FINDNEXT"));
	entries[16].Set(wxACCEL_CTRL,   WXK_F4,       XRCID("ID_ITEM_CLOSE"));
	entries[17].Set(wxACCEL_NORMAL, WXK_F5,       XRCID("ID_PROJECT_GENERATE"));
	entries[18].Set(wxACCEL_NORMAL, WXK_F7,       XRCID("ID_PROJECT_PLAY"));
	entries[19].Set(wxACCEL_CTRL,   WXK_F6,       XRCID("ID_NEXT_PANE"));
	entries[20].Set(wxACCEL_SHIFT,  WXK_F6,       XRCID("ID_PREV_PANE"));
	entries[21].Set(wxACCEL_CTRL,   WXK_INSERT,   wxID_COPY);
	entries[22].Set(wxACCEL_CTRL,   WXK_F1,       XRCID("ID_HELP_CONTENTS"));

	wxAcceleratorTable accel(23, entries);
	SetAcceleratorTable(accel);
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame(NULL, wxID_ANY, title, pos, size)
{
	bVisible = PROJECT_PANE | EDITOR_PANE | TOOLBAR_PANE | STATUS_PANE | KEYBOARD_PANE;
	inGenerate = 0;
	editFlags = 0;
	itemFlags = 0;

	prjTree = static_cast<ProjectTree*>(this);
	prjFrame = static_cast<ProjectFrame*>(this);
	prjFileTypes = "Projects|*.bsprj|XML Files|*.xml|All files|*.*";
	prjFileExt = "bsprj";
	errWnd = 0;
	help = 0;

#ifdef _WIN32
	wxString icoid("IDI_MAIN");
	wxIcon mainico(icoid, wxBITMAP_TYPE_ICO_RESOURCE, 16, 16);
#else
    wxString icoid("res/main.png");
    wxIcon mainico(icoid, wxBITMAP_TYPE_PNG);
#endif
    if (mainico.IsOk())
        SetIcon(mainico);

	wxPrjMenu = wxXmlResource::Get()->LoadMenuBar("MB_MAINFRAME");
	if (wxPrjMenu == NULL)
	{
		wxMessageBox("Menu bar didn't load");
		exit(1);
	}
	SetMenuBar(wxPrjMenu);

	wxMenu *file = wxPrjMenu->GetMenu(0);
	wxMenuItem *recent = file->FindItemByPosition(9);
	wxMenu *docMenu = recent->GetSubMenu();
	if (docMenu)
		mruList.UseMenu(docMenu);
	mruList.Load(*wxConfig::Get());

	ctxProject = wxXmlResource::Get()->LoadMenu("ctxProject");
	ctxWaveout = wxXmlResource::Get()->LoadMenu("ctxWaveout");
	ctxSynth = wxXmlResource::Get()->LoadMenu("ctxSynth");
	ctxFileList = wxXmlResource::Get()->LoadMenu("ctxFileList");
	ctxLibItem = wxXmlResource::Get()->LoadMenu("ctxLibItems");
	ctxProjectItem = wxXmlResource::Get()->LoadMenu("ctxProjectItems");

	if (prjOptions.toolBarSize == 16)
		wxPrjToolBar = wxXmlResource::Get()->LoadToolBar(this, "TB_MAINFRAME16");
	else
		wxPrjToolBar = wxXmlResource::Get()->LoadToolBar(this, "TB_MAINFRAME");
	SetToolBar(wxPrjToolBar);
#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
	// work-around background drawing bug.
	// For some reason, when wxWidgets is compiled with mingw
	// the erase background message is sent to the toolbar
	// rather than the frame window. This next line
	// sets a hook that will redraw the toolbar background.
	// The toolbar will set the hook on its parent automatically.
	wxPrjToolBar->MSWSetEraseBgHook(wxPrjToolBar);
#endif

	wxPrjStatusBar = CreateStatusBar();

	InitAccel();

	int defKbdHeight = 250;
	int defTreeWidth = 200;
	int clientHeight = size.GetHeight();
	int clientWidth = size.GetWidth();
	int treeHeight = clientHeight - defKbdHeight; // initial allotment for keyboard at bottom

// the splitter works - mostly.
// The splitter will never shrink a contained window below its original size.
// we create the tree and tabs at the minimum size.
	int minWidth = 20;
	wxPrjSplit = new wxSplitterWindow(this, wxWindowID(ID_SPLITTER_WND), wxPoint(0, 0), wxSize(clientWidth,treeHeight), wxSP_3D);
	wxPrjTree = new MainTree(this, wxPrjSplit, minWidth, treeHeight);
	wxPrjTabs = new wxNotebook(wxPrjSplit, wxWindowID(ID_TAB_WND),
		wxPoint(0, 0), wxSize(minWidth, treeHeight),
		wxNB_TOP, wxString("tabs"));
	wxPrjSplit->SplitVertically(wxPrjTree, wxPrjTabs);
	wxPrjSplit->SetMinimumPaneSize(minWidth);
	wxPrjSplit->SetSashPosition(defTreeWidth);
	wxPrjSplit->Show(false);
	wxPrjTree->SaveSize();

	// The actual size of the keyboard pane is set
	// after the keyboard form is loaded.
	kbdWnd = new KeyboardDlg(this, clientWidth, defKbdHeight);
	kbdWnd->Show(false);
}

MainFrame::~MainFrame()
{
//	delete wxPrjToolBar;
//	delete wxPrjStatusBar;
//	delete wxPrjMenu;
//	delete wxPrjSplit;
	delete wxPrjTree;
	delete wxPrjTabs;
	delete ctxProject;
	delete ctxWaveout;
	delete ctxSynth;
	delete ctxFileList;
	delete ctxLibItem;
	delete ctxProjectItem;
	delete help;
	delete kbdWnd;
}


/// Make a backup of the project file in the TEMP directory.
/// This is intended to save the project changes in case
/// the program aborts with an open project.
/// Currently, recovery is not automatic.
/// The user must locate the project file and copy it somewhere
/// or open the temp file and then do Save As...
void MainFrame::SaveTemp(int sv)
{
	if (sv)
	{
		if (theProject)
		{
			if (tmppath.Len() == 0)
				tmppath = wxFileName::CreateTempFileName("bsynth");
			theProject->SaveProject(tmppath);
		}
	}
	else
	{
		if (tmppath.Len() != 0)
		{
			if (wxFileName::FileExists(tmppath))
				wxRemoveFile(tmppath);
			tmppath.Empty();
		}
	}
}

void MainFrame::OnTimer(wxTimerEvent& evt)
{
	if (!inGenerate)
		SaveTemp(1);
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
	if (!evt.CanVeto() || CloseProject(1))
	{
#if USE_AUTOSAVE
		autosave.Stop();
#endif
#if wxUSE_STC
		TextEditorStc::DeleteFindReplDlg();
#else
		TextEditorWX::DeleteFindReplDlg();
#endif
		SaveTemp(0);
		wxRect rc = GetScreenRect();
		prjOptions.frmLeft = rc.GetLeft();
		prjOptions.frmTop = rc.GetTop();
		prjOptions.frmWidth = rc.GetWidth();
		prjOptions.frmHeight = rc.GetHeight();
//		prjOptions.frmMax = maximized;
		prjOptions.Save();
		mruList.Save(*wxConfig::Get());
		Destroy();
		wxTheApp->ExitMainLoop(); // justin case...
	}
	else
		evt.Veto(true);
}

void MainFrame::OnSetFocus(wxFocusEvent& evt)
{
	if (theProject)
	{
		EditorView *ed = GetActiveEditor();
		if (ed)
			ed->Focus();
		else if (bVisible & PROJECT_PANE)
			wxPrjTree->SetFocus();
	}
}

void MainFrame::OnSize(wxSizeEvent& evt)
{
	if (theProject)
		UpdateLayout();
}

void MainFrame::OnErase(wxEraseEvent& evt)
{
	wxDC *dc = evt.GetDC();
	if (dc)
	{
		wxRect rc(GetClientAreaOrigin(), GetClientSize());
		rc.height++;
		rc.width++;
		wxBrush bck(GetBackgroundColour(), wxSOLID);
		dc->SetBrush(bck);
		dc->DrawRectangle(rc);
	}
}

void MainFrame::EnablePanes()
{
	if (bVisible & PROJECT_PANE)
		wxPrjSplit->SplitVertically(wxPrjTree, wxPrjTabs);
	else
		wxPrjSplit->Unsplit(wxPrjTree);
	wxPrjSplit->Show(true);
	kbdWnd->Show(bVisible & KEYBOARD_PANE ? true : false);
}

void MainFrame::UpdateLayout(int rd)
{
	// if minimized || maximized
	if (!IsShown())
		return;

	wxSize frmSz = GetClientSize();

	int kbtop;

	if (bVisible & KEYBOARD_PANE)
	{
		int kh, kw;
		kbdWnd->GetSize(&kw, &kh);
		kbtop = frmSz.GetHeight() - kh;
		if (kbtop > 0)
		{
			kbdWnd->SetSize(0, kbtop, frmSz.GetWidth(), kh);
			frmSz.SetHeight(kbtop);
		}
		else
		{
			kbtop = frmSz.GetHeight();
			bVisible &= ~KEYBOARD_PANE;
			kbdWnd->Show(false);
		}
	}
	else
		kbtop = frmSz.GetHeight();
	wxPrjSplit->SetSize(0, 0, frmSz.GetWidth(), kbtop);
	if (rd)
		Refresh();
}

void MainFrame::OnExit(wxCommandEvent& evt)
{
	Close(false);
}

void MainFrame::OnUpdateProjectEnable(wxUpdateUIEvent& evt)
{
	evt.Enable(theProject != NULL);
}

void MainFrame::OnNewProject(wxCommandEvent& evt)
{
	if (NewProject())
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddFileToHistory((const char *)path);
		AfterOpenProject();
	}
}

void MainFrame::OnOpenProject(wxCommandEvent& evt)
{
	if (OpenProject(0))
	{
		bsString path;
		theProject->GetProjectPath(path);
		mruList.AddFileToHistory((const char *)path);
		AfterOpenProject();
	}
}

void MainFrame::AfterOpenProject()
{
	if (errWnd)
		errWnd->Clear();

	if (!theProject)
	{
		Alert("No project after open project!", "Huh?");
		return; // huh?
	}

	SynthWidget::colorMap.Load(prjOptions.formsDir, prjOptions.colorsFile, 1);

	kbdWnd->Load();

	wxPrjTree->Expand(wxPrjTree->GetRootItem());
	EnablePanes();
	UpdateLayout();
	wxString title(prjOptions.programName);
	title += " Composer"; // get from project options
	title += " - ";
	title += theProject->GetName();
	SetTitle(title);
#ifdef _WIN32
	prjOptions.dsoundHWND = (HWND) GetHandle();
#endif

	SaveTemp(0); // discard any temp file.
#if USE_AUTOSAVE
	autosave.SetOwner(this);
	autosave.Start(60000, wxTIMER_CONTINUOUS);
#endif
}

void MainFrame::OnSaveProject(wxCommandEvent& evt)
{
	SaveProject();
}

void MainFrame::OnSaveProjectAs(wxCommandEvent& evt)
{
	SaveProjectAs();
}

void MainFrame::OnProjectGenerate(wxCommandEvent& evt)
{
	Generate(0, -1);
}

void MainFrame::OnProjectPlay(wxCommandEvent& evt)
{
	if (kbdWnd->IsRunning())
		StopPlayer();
	else
		StartPlayer();
}

void MainFrame::OnProjectOptions(wxCommandEvent& evt)
{
	OptionsDlg *dlg = new OptionsDlg(this);
	dlg->ShowModal();
	delete dlg;
}

void MainFrame::OnNewItem(wxCommandEvent& evt)
{
	NewItem();
}

void MainFrame::OnUpdateNewItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_NEW);
}

void MainFrame::OnAddItem(wxCommandEvent& evt)
{
	AddItem();
}

void MainFrame::OnUpdateAddItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_ADD);
}

void MainFrame::OnEditItem(wxCommandEvent& evt)
{
	EditItem();
}

void MainFrame::OnUpdateEditItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_EDIT);
}

void MainFrame::OnSaveItem(wxCommandEvent& evt)
{
	SaveItem();
}

void MainFrame::OnUpdateSaveItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_SAVE && editFlags & VW_ENABLE_FILE);
}

void MainFrame::OnCloseItem(wxCommandEvent& evt)
{
	CloseItem();
}

void MainFrame::OnUpdateCloseItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_CLOSE && editFlags & VW_ENABLE_FILE);
}

void MainFrame::OnRemoveItem(wxCommandEvent& evt)
{
	RemoveItem();
}

void MainFrame::OnUpdateRemoveItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_REM);
}

void MainFrame::OnCopyItem(wxCommandEvent& evt)
{
	CopyItem();
}

void MainFrame::OnUpdateCopyItem(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_COPY);
}

void MainFrame::OnItemProperties(wxCommandEvent& evt)
{
	ItemProperties();
}

void MainFrame::OnUpdateItemProperties(wxUpdateUIEvent& evt)
{
	evt.Enable(itemFlags & ITM_ENABLE_PROPS);
}

void MainFrame::OnItemErrors(wxCommandEvent& evt)
{
	if (!errWnd)
	{
		errWnd = new ScoreErrorsDlg(this);
		errWnd->Show(true);
	}
	else
	{
		errWnd->Destroy();
		delete errWnd;
		errWnd = 0;
	}
}

void MainFrame::OnUpdateItemErrors(wxUpdateUIEvent& evt)
{
	evt.Enable(theProject != NULL);
	evt.Check(errWnd != NULL);
}

void MainFrame::OnEditUndo(wxCommandEvent& evt)
{
	EditUndo();
}

void MainFrame::OnUpdateEditUndo(wxUpdateUIEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
	evt.Enable(editFlags & VW_ENABLE_UNDO ? true : false);
}

void MainFrame::OnEditRedo(wxCommandEvent& evt)
{
	EditRedo();
}

void MainFrame::OnUpdateEditRedo(wxUpdateUIEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
	evt.Enable(editFlags & VW_ENABLE_REDO ? true : false);
}

void MainFrame::OnEditCopy(wxCommandEvent& evt)
{
	EditCopy();
}

void MainFrame::OnUpdateEditCopy(wxUpdateUIEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
	evt.Enable(editFlags & VW_ENABLE_COPY ? true : false);
}

void MainFrame::OnEditCut(wxCommandEvent& evt)
{
	EditCut();
}

void MainFrame::OnUpdateEditCut(wxUpdateUIEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
	evt.Enable(editFlags & VW_ENABLE_CUT ? true : false);
}

void MainFrame::OnEditPaste(wxCommandEvent& evt)
{
	EditPaste();
}

void MainFrame::OnUpdateEditPaste(wxUpdateUIEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
	evt.Enable(editFlags & VW_ENABLE_PASTE ? true : false);
}

void MainFrame::OnEditSelectAll(wxCommandEvent& evt)
{
	EditSelectAll();
}

void MainFrame::OnUpdateEditSelectAll(wxUpdateUIEvent& evt)
{
	evt.Enable(editFlags & VW_ENABLE_SELALL ? true : false);
}

void MainFrame::OnEditGotoLine(wxCommandEvent& evt)
{
	EditGoto();
}

void MainFrame::OnUpdateEditGotoLine(wxUpdateUIEvent& evt)
{
	evt.Enable(editFlags & VW_ENABLE_GOTO ? true : false);
}

void MainFrame::OnEditFind(wxCommandEvent& evt)
{
	EditFind();
}

void MainFrame::OnUpdateEditFind(wxUpdateUIEvent& evt)
{
	evt.Enable(editFlags & VW_ENABLE_FIND ? true : false);
}

void MainFrame::OnEditFindNext(wxCommandEvent& evt)
{
	EditFindNext();
}

void MainFrame::OnUpdateEditFindNext(wxUpdateUIEvent& evt)
{
	evt.Enable(editFlags & VW_ENABLE_FIND ? true : false);
}

void MainFrame::OnMarkerSet(wxCommandEvent& evt)
{
	MarkerSet();
}

void MainFrame::OnMarkerNext(wxCommandEvent& evt)
{
	MarkerNext();
}
void MainFrame::OnMarkerPrev(wxCommandEvent& evt)
{
	MarkerPrev();
}
void MainFrame::OnMarkerClear(wxCommandEvent& evt)
{
	MarkerClear();
}

void MainFrame::OnUpdateEditMark(wxUpdateUIEvent& evt)
{
	evt.Enable(editFlags & VW_ENABLE_MARK ? true : false);
}

void MainFrame::OnViewProject(wxCommandEvent& evt)
{
	// the wx splitter, when restoring the split,
	// will split down the middle. We save the
	// project tree size and reset the splitter
	// position.
	bVisible ^= PROJECT_PANE;
	if (bVisible & PROJECT_PANE)
	{
		wxPrjSplit->SplitVertically(wxPrjTree, wxPrjTabs);
		wxPrjSplit->SetSashPosition(wxPrjTree->SavedWidth());
		ProjectItem *itm = GetClickedItem();
		if (itm)
			itemFlags = itm->ItemActions();
		else
			itemFlags = 0;
	}
	else
	{
		wxPrjTree->SaveSize();
		itemFlags = 0;
		wxPrjSplit->Unsplit(wxPrjTree);
	}
	UpdateLayout();
}

void MainFrame::OnUpdateViewProject(wxUpdateUIEvent& evt)
{
	evt.Check(bVisible & PROJECT_PANE);
	evt.Enable(theProject != NULL);
}

void MainFrame::OnViewKeyboard(wxCommandEvent& evt)
{
	bVisible ^= KEYBOARD_PANE;
	kbdWnd->Show(bVisible & KEYBOARD_PANE ? true : false);
	UpdateLayout();
}

void MainFrame::OnUpdateViewKeyboard(wxUpdateUIEvent& evt)
{
	evt.Check(bVisible & KEYBOARD_PANE);
	evt.Enable(theProject != NULL);
}

void MainFrame::OnViewToolBar(wxCommandEvent& evt)
{
	bVisible ^= TOOLBAR_PANE;
	if (bVisible & TOOLBAR_PANE)
		SetToolBar(wxPrjToolBar);
	else
		SetToolBar(NULL);
	UpdateLayout();
}

void MainFrame::OnUpdateViewToolBar(wxUpdateUIEvent& evt)
{
	evt.Check(bVisible & TOOLBAR_PANE);
}

void MainFrame::OnViewStatusBar(wxCommandEvent& evt)
{
	bVisible ^= STATUS_PANE;
	if (bVisible & STATUS_PANE)
		SetStatusBar(wxPrjStatusBar);
	else
		SetStatusBar(NULL);
	UpdateLayout();
}

void MainFrame::OnUpdateViewStatusBar(wxUpdateUIEvent& evt)
{
	evt.Check(bVisible & STATUS_PANE);
}

void MainFrame::OnPrevPane(wxCommandEvent& evt)
{
	int cnt = wxPrjTabs->GetPageCount();
	if (cnt > 1)
	{
		int pg = wxPrjTabs->GetSelection();
		if (--pg < 0)
			pg = cnt - 1;
		wxPrjTabs->SetSelection(pg);
	}
}

void MainFrame::OnNextPane(wxCommandEvent& evt)
{
	int cnt = wxPrjTabs->GetPageCount();
	if (cnt > 1)
	{
		int pg = wxPrjTabs->GetSelection();
		if (++pg >= cnt)
			pg = 0;
		wxPrjTabs->SetSelection(pg);
	}
}

void MainFrame::OnUpdateNextPane(wxUpdateUIEvent& evt)
{
	evt.Enable(wxPrjTabs->GetPageCount() > 1);
}

void MainFrame::OnAppAbout(wxCommandEvent& evt)
{
	AboutDlg *dlg = new AboutDlg(this);
	dlg->ShowModal();
	delete dlg;
}

void MainFrame::OnHelpContents(wxCommandEvent& evt)
{
	if (help == 0)
	{
		wxString hf(prjOptions.helpFile);
		wxFileName hp(hf);
		if (!hp.FileExists())
		{
			hf = wxFileSelector("Select Help File", hp.GetPath(), hp.GetFullName(), "hhp", "Help Files|*.hhp|All Files|*.*", wxFD_OPEN, this);
			if (hf.Len() == 0)
				return;
		}
		help = new wxHtmlHelpController(wxHF_DEFAULT_STYLE, this);
		if (!help)
			return;
		help->AddBook(hf, false);
	}
	help->DisplayContents();
}

void MainFrame::OnCloseAll(wxCommandEvent& evt)
{
	CloseAllEditors();
}

void MainFrame::OnUpdateCloseAll(wxUpdateUIEvent& evt)
{
	evt.Enable(GetActiveEditor() != NULL);
}

void MainFrame::OnMRUFile(wxCommandEvent& evt)
{
	int index = evt.GetId() - wxID_FILE1;
	wxString path(mruList.GetHistoryFile(index));
	if (path.Length() != 0)
	{
		if (OpenProject(path))
		{
			AfterOpenProject();
			// rotate to first in list
			mruList.RemoveFileFromHistory(index);
			mruList.AddFileToHistory(path);
		}
	}
}
void MainFrame::OnLftDClick(wxMouseEvent& evt)
{
	if (evt.GetEventObject() == wxPrjTree)
	{
		wxCommandEvent cmd(wxEVT_COMMAND_MENU_SELECTED, XRCID("ID_ITEM_EDIT"));
		evt.SetEventObject(wxPrjTree);
		wxPostEvent(this, cmd);
	}
}


void MainFrame::OnPrjRClick(wxTreeEvent& evt)
{
	// popup context menu
	ProjectItem *itm = NULL;
	bsTreeItemData *dp = (bsTreeItemData *)wxPrjTree->GetItemData(evt.GetItem());
	if (dp)
	{
		wxPrjTree->SelectItem(dp->GetId());
		itm = dp->prjItem;
	}
	if (itm)
	{
		wxMenu *ctxMenu = NULL;
		switch (itm->GetType())
		{
		case PRJNODE_PROJECT:
			ctxMenu = ctxProject;
			break;
		case PRJNODE_WAVEOUT:
			ctxMenu = ctxWaveout;
			break;
		case PRJNODE_SYNTH:
			ctxMenu = ctxSynth;
			break;
		case PRJNODE_NOTELIST:
		case PRJNODE_SEQLIST:
		case PRJNODE_TEXTLIST:
		case PRJNODE_LIBLIST:
		case PJRNODE_SCRIPTLIST:
		case PRJNODE_WVFLIST:
		case PRJNODE_SBLIST:
			ctxMenu = ctxFileList;
			break;
		case PRJNODE_LIB:
			ctxMenu = ctxLibItem;
			break;
		default:
			ctxMenu = ctxProjectItem;
			break;
		}
		if (ctxMenu)
			PopupMenu(ctxMenu);
	}
}

void MainFrame::OnPrjSelChanged(wxTreeEvent& evt)
{
	ProjectItem *itm = NULL;
	bsTreeItemData *dp = (bsTreeItemData *)wxPrjTree->GetItemData(evt.GetItem());
	if (dp)
	{
		itm = dp->prjItem;
		itemFlags = itm->ItemActions();
	}
}

void MainFrame::OnPageActivated(wxNotebookEvent& evt)
{
	EditorView *vw = GetActiveEditor();
	if (vw)
	{
		editFlags = vw->EditState();
		ProjectItem *itm = vw->GetItem();
		if (itm)
		{
			bsTreeItemData *dp = (bsTreeItemData*)  itm->GetPSData();
			if (dp)
				wxPrjTree->SelectItem(dp->GetId());
            PIType typ = itm->GetType();
            if (typ == PRJNODE_INSTR || typ == PRJNODE_LIBINSTR)
				kbdWnd->SelectInstrument(((InstrItem *)itm)->GetConfig());
		}
	}
	else
		editFlags = 0;
}

////////////////////////////// Project tree functions /////////////////////////////////

ProjectItem *MainFrame::GetClickedItem()
{
	wxTreeItemId id = wxPrjTree->GetSelection();
	if (id)
	{
		bsTreeItemData *dp = (bsTreeItemData *)wxPrjTree->GetItemData(id);
		if (dp)
			return dp->prjItem;
	}
	return 0;
}

void MainFrame::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (itm == 0)
		return;

	ProjectItem *parent = itm->GetParent();

	bsTreeItemData *dp = new bsTreeItemData(itm);

	wxString name(itm->GetName(), cutf8);
	if (parent == NULL)
		wxPrjTree->AddRoot(name, -1, -1, dp);
	else
	{
		bsTreeItemData *pp = (bsTreeItemData*)parent->GetPSData();
		if (sib)
		{
			bsTreeItemData *sp = (bsTreeItemData*)sib->GetPSData();
			wxPrjTree->InsertItem(pp->GetId(), sp->GetId(), name, -1, -1, dp);
		}
		else
			wxPrjTree->AppendItem(pp->GetId(), name, -1, -1, dp);
	}
}

void MainFrame::SelectNode(ProjectItem *itm)
{
	if (itm)
	{
		itemFlags = itm->ItemActions();
		bsTreeItemData *data = (bsTreeItemData*)itm->GetPSData();
		if (data)
		{
			wxPrjTree->SelectItem(data->GetId());
			UpdateWindowUI();
		}
	}
	else
		itemFlags = 0;
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

	bsTreeItemData *data = (bsTreeItemData*)itm->GetPSData();
	if (data)
		wxPrjTree->Delete(data->GetId());
}

void MainFrame::UpdateNode(ProjectItem *itm)
{
	if (itm)
	{
		bsTreeItemData *data = (bsTreeItemData*)itm->GetPSData();
		if (data)
		{
			wxString name(itm->GetName(), cutf8);
			wxPrjTree->SetItemText(data->GetId(), name);
			int pg = GetItemTab(itm);
			if (pg >= 0)
				wxPrjTabs->SetPageText((size_t)pg, name);
		}
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
	bsTreeItemData *data = (bsTreeItemData *)wxPrjTree->GetItemData(wxPrjTree->GetSelection());
	if (data)
		return (ProjectItem*)data->prjItem;
	return 0;
}

ProjectItem *MainFrame::FirstChild(ProjectItem *itm)
{
	if (itm == 0)
		return 0;

	bsTreeItemData *data = (bsTreeItemData*)itm->GetPSData();
	if (data == 0)
		return 0;

	wxTreeItemId child = wxPrjTree->GetFirstChild(data->GetId(), data->cookie);
	if (child.IsOk())
	{
		data = (bsTreeItemData *)wxPrjTree->GetItemData(child);
		if (data)
			return data->prjItem;
	}
	return 0;
}

ProjectItem *MainFrame::NextSibling(ProjectItem *itm)
{
	if (itm == 0)
		return 0;

	bsTreeItemData *data = (bsTreeItemData*)itm->GetPSData();
	if (data == 0)
		return 0;

	wxTreeItemId child = wxPrjTree->GetNextSibling(data->GetId());
	if (child.IsOk())
	{
		data = (bsTreeItemData *)wxPrjTree->GetItemData(child);
		if (data)
			return data->prjItem;
	}
	return 0;
}

void MainFrame::RemoveAll()
{
	wxPrjTree->DeleteAllItems();
}

int MainFrame::CloseAllEditors()
{
	int ret = 1;
	size_t pg = 0;
	while (pg < wxPrjTabs->GetPageCount())
	{
		FormEditorWin *form = static_cast<FormEditorWin*>(wxPrjTabs->GetPage(pg));
		ProjectItem *pi = form->GetItem();
		if (!pi->CloseItem())
			ret = 0;
		else
			pg++;
	}
	if (ret)
		wxPrjTabs->DeleteAllPages(); // justin case...
	//wxPrjTabs->Show(false);
	editFlags = 0;
	return ret;
}

///////////////////// Platform specific frame functions ////////////////////////////////////

int MainFrame::Verify(const char *msg, const char *title)
{
	wxString caption;
	if (title)
		caption = title;
	else
		caption = "Wait...";
	wxMessageDialog *dlg = new wxMessageDialog(this, msg, caption, wxCANCEL | wxYES_NO | wxICON_QUESTION);
	switch (dlg->ShowModal())
	{
	case wxID_YES:
		return 1;
	case wxID_NO:
		return 0;
	}
	return -1;
}

// TODO: It should be possible to create a generic property box
// and populate it from the XRC file.
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
			ProjectPropertiesDlg *p = new ProjectPropertiesDlg(this, pi);
			pb = static_cast<PropertyBox*>(p);
		}
		break;
	case PRJNODE_MIXER:
		{
			MixerSetupDlg *m = new MixerSetupDlg(this, pi);
			pb = static_cast<PropertyBox*>(m);
		}
		break;
	case PRJNODE_REVERB:
	case PRJNODE_FLANGER:
	case PRJNODE_ECHO:
		{
			EffectsSetupDlg *e = new EffectsSetupDlg(this, pi);
			pb = static_cast<PropertyBox*>(e);
		}
		break;
	case PRJNODE_NOTELIST:
	case PRJNODE_FILELIST:
	case PRJNODE_SEQLIST:
	case PRJNODE_TEXTLIST:
	case PJRNODE_SCRIPTLIST:
		{
			FilelistOrder *f = new FilelistOrder(this, pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_NOTEFILE:
	case PRJNODE_SEQFILE:
	case PRJNODE_TEXTFILE:
	case PRJNODE_SCRIPT:
		{
			FilePropertiesDlg *f = new FilePropertiesDlg(this, pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_SOUNDBANK:
		{
			SoundBankPropertiesDlg *f = new SoundBankPropertiesDlg(this, pi);
			f->SetItem(pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_LIBLIST:
		break;
	case PRJNODE_LIB:
		{
			LibPropertiesDlg *f = new LibPropertiesDlg(this, pi);
			pb = static_cast<PropertyBox*>(f);
		}
		break;
	case PRJNODE_SELINSTR:
		{
			InstrSelectDlg *i = new InstrSelectDlg(this, pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_INSTRLIST:
		break;
	case PRJNODE_LIBINSTR:
	case PRJNODE_INSTR:
		{
			InstrPropertiesDlg *i = new InstrPropertiesDlg(this, pi);
			pb = static_cast<PropertyBox*>(i);
		}
		break;
	case PRJNODE_WVTABLE:
		{
			NamePropertiesDlg *n = new NamePropertiesDlg(this, pi);
			pb = static_cast<PropertyBox*>(n);
		}
		break;
	case PRJNODE_WVFILE:
		{
			WavefilePropertiesDlg *w = new WavefilePropertiesDlg(this, pi);
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
	FormEditorWin *form = new FormEditorWin(wxPrjTabs);
	form->SetItem(pi);
	form->SetPSData(form);
	form->Resize();
	wxPrjTabs->AddPage(form, wxString(pi->GetName(), cutf8), true);
	wxPrjTabs->Show(true);
	return form;
}

TextEditor *MainFrame::CreateTextEditor(ProjectItem *pi)
{
	if (!pi)
		return 0;
#if wxUSE_STC
	TextEditorStc *ed = new TextEditorStc(wxPrjTabs, pi);
#else
	TextEditorWX *ed = new TextEditorWX(wxPrjTabs, pi);
#endif
	if (ed)
	{
		pi->SetEditor(ed);
		//ed->SetItem(pi);
		ed->SetPSData((void*)ed);
		wxPrjTabs->AddPage(ed, wxString(pi->GetName(), cutf8), true);
		//wxPrjTabs->Show(true);
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
	wxWindow *form = reinterpret_cast<wxWindow*>(vw->GetPSData());

	size_t pg;
	for (pg = 0; pg < wxPrjTabs->GetPageCount(); pg++)
	{
		if (wxPrjTabs->GetPage(pg) == form)
		{
			wxPrjTabs->DeletePage(pg);
			break;
		}
	}
	itm->SetEditor(0);
	if (wxPrjTabs->GetPageCount() == 0)
	{
//		wxPrjTabs->Show(false);
		editFlags = 0;
	}
	return 1;
}

EditorView *MainFrame::GetActiveEditor()
{
	FormEditorWin *form = static_cast<FormEditorWin*>(wxPrjTabs->GetCurrentPage());
	if (form)
		return static_cast<EditorView*>(form);
	return 0;
}

int MainFrame::GetItemTab(ProjectItem *itm)
{
	if (itm)
		return GetItemTab(itm->GetEditor());
	return -1;
}

int MainFrame::GetItemTab(EditorView *vw)
{
	if (vw)
	{
		FormEditorWin *form = reinterpret_cast<FormEditorWin*>(vw->GetPSData());
		if (form)
		{
			size_t pg = 0;
			for (pg = 0; pg < wxPrjTabs->GetPageCount(); pg++)
			{
				if (wxPrjTabs->GetPage(pg) == form)
					return (int)pg;
			}
		}
	}
	return -1;
}

int MainFrame::OpenEditor(ProjectItem *itm)
{
	if (!itm)
		return 0;

	EditorView *vw = itm->GetEditor();
	if (vw)
	{
		FormEditorWin *form = reinterpret_cast<FormEditorWin*>(vw->GetPSData());
		if (wxPrjTabs->GetCurrentPage() == form)
		{
			form->SetFocus();
			return 1;
		}
		int pg = GetItemTab(vw);
		if (pg >= 0)
		{
			wxPrjTabs->SetSelection((size_t)pg);
			return 1;
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
	size_t cnt = wxPrjTabs->GetPageCount();
	size_t pg;
	for (pg = 0; pg < cnt; pg++)
	{
		FormEditorWin *form = static_cast<FormEditorWin*>(wxPrjTabs->GetPage(pg));
		if (form->IsChanged())
		{
			if (query)
			{
				wxString msg;
				msg = wxPrjTabs->GetPageText(pg);
				msg += " has changed. Save?";
				int res = Verify(msg, "Wait...");
				if (res == 0) // no
					continue;
				if (res == -1) // cancel
					return 0;
			}
			ProjectItem *itm = form->GetItem();
			if (itm)
				itm->SaveItem();
		}
	}
	return 1;
}

void MainFrame::EditStateChanged()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		editFlags = vw->EditState();
}

int MainFrame::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	wxString spcbuf(spec);
	if (spcbuf.Find("*.*") == wxNOT_FOUND)
		spcbuf += "All Files|*.*";

	wxString title("Select File");
	wxString defDir;
	wxString defFile;
	wxString defExt;
	if (file && *file)
	{
		wxFileName::SplitPath(file, &defDir, &defFile, &defExt);
	}
	else
	{
		defDir = prjOptions.defPrjDir;
		defExt = ext;
	}
	long style = open ? wxFD_OPEN : wxFD_SAVE;
	wxFileDialog *dlg = new wxFileDialog(this, title, defDir, defFile, spcbuf, style);

	if (dlg->ShowModal() == wxID_OK)
	{
		strncpy(file, dlg->GetPath(), MAX_PATH);
		SynthProject::NormalizePath(file);
		if (ext && *ext)
			SynthProject::CheckExtension(file, ext);
		return 1;
	}
	return 0;
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
	int pg = wxPrjTabs->GetSelection();
	QueryValueDlg dlg(this, prompt, value, len);
	if (dlg.ShowModal())
	{
		if (pg >= 0)
			wxPrjTabs->GetPage(pg)->SetFocus();
		return 1;
	}
	return 0;
}

int MainFrame::Alert(const char *msg, const char *title)
{
	wxString caption;
	if (title)
		caption = title;
	else
		caption = "Wait...";
	wxMessageDialog *dlg = new wxMessageDialog(this, msg, caption, wxOK | wxICON_HAND);
	dlg->ShowModal();
	return 0;
}

void MainFrame::InitPlayer()
{
	kbdWnd->InitInstrList();
	kbdWnd->UpdateChannels();
}

int MainFrame::StopPlayer()
{
	return kbdWnd->Stop();
}

void MainFrame::StartPlayer()
{
	kbdWnd->Start();
}

void MainFrame::ClearPlayer()
{
	kbdWnd->Clear();
}

void MainFrame::InstrAdded(InstrConfig *inst)
{
	kbdWnd->AddInstrument(inst);
}

void MainFrame::InstrRemoved(InstrConfig *inst)
{
	kbdWnd->RemoveInstrument(inst);
}

void MainFrame::InstrChanged(InstrConfig *inst)
{
	kbdWnd->UpdateInstrument(inst);
}

void MainFrame::MixerChanged()
{
	kbdWnd->UpdateChannels();
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

	++inGenerate;
	int wasPlaying = StopPlayer();
	GenerateDlg dlg(this, autoStart, todisk);
	dlg.ShowModal();
	if (wasPlaying)
		StartPlayer();
	--inGenerate;
}
