//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#pragma once

#define WINDOW_MENU_POSITION	4

//#include "KeyboardDlg.h"
#include "KeyboardDlg2.h"
#include "ScoreErrorsDlg.h"

class MainFrame : 
	public CFrameWindowImpl<MainFrame>, 
	public CUpdateUI<MainFrame>,
	public CMessageFilter, 
	public CIdleHandler,
	public ProjectFrame,
	public ProjectTree
{
public:
	DECLARE_FRAME_WND_CLASS("BSynthComposer", IDR_MAINFRAME)

	int bVisible;
	//CTreeViewCtrl prjList;
	utf8TreeViewCtrl prjList;
	CTabView tabView;
	//KeyboardDlg kbdWnd;
	KeyboardDlg2 kbdWnd;
	ScoreErrorsDlg *errWnd;
	CSplitterWindow splitTop;
	CCommandBarCtrl cmdBar;
	CRecentDocumentList mruList;

	MainFrame()
	{
		prjTree = static_cast<ProjectTree*>(this);
		prjFrame = static_cast<ProjectFrame*>(this);
		prjFileTypes = "Projects|*.bsprj|XML Files|*.xml|All files|*.*|";
		prjFileExt = "bsprj";
		errWnd = 0;
	}

	~MainFrame()
	{
		ATLTRACE("MainFrame destroyed\n");
		prjTree = 0;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	void UpdateLayout(BOOL bResizeBars = 1);
	void EnablePanes();
	void UpdateEditUI(int pg);
	void UpdateItemUI(ProjectItem *pi);
	void UpdateProjectUI();
	ProjectItem *GetClickedItem();
	void SaveTemp(int sv);
	void AfterOpenProject();

	// project Tree interface
	virtual void AddNode(ProjectItem *itm, ProjectItem *sib = 0);
	virtual void SelectNode(ProjectItem *itm);
	virtual void RemoveNode(ProjectItem *itm);
	virtual void UpdateNode(ProjectItem *itm);
	virtual void MoveNode(ProjectItem *itm, ProjectItem *sib);
	virtual void RemoveAll();
	virtual ProjectItem *FirstChild(ProjectItem *itm);
	virtual ProjectItem *NextSibling(ProjectItem *itm);
	virtual ProjectItem *GetSelectedNode();

	// ProjectFrame interface
	virtual int BrowseFile(int open, char *file, const char *spec, const char *ext);
	virtual int BrowseFile(int open, bsString& file, const char *spec, const char *ext);
	virtual int Verify(const char *msg, const char *title);
	virtual PropertyBox *CreatePropertyBox(ProjectItem *pi, int type);
	virtual FormEditor *CreateFormEditor(ProjectItem *pi);
	virtual TextEditor *CreateTextEditor(ProjectItem *pi);
	virtual EditorView *GetActiveEditor();
	virtual void EditStateChanged();
	virtual int OpenEditor(ProjectItem *itm);
	virtual int CloseEditor(ProjectItem *pi);
	virtual int CloseAllEditors();
	virtual int SaveAllEditors(int q);
	virtual int QueryValue(const char *prompt, char *value, int len);
	virtual int Alert(const char *msg, const char *title);
	virtual void InitPlayer();
	virtual int StopPlayer();
	virtual void StartPlayer();
	virtual void ClearPlayer();
	virtual void InstrAdded(InstrConfig *inst);
	virtual void InstrRemoved(InstrConfig *inst);
	virtual void InstrChanged(InstrConfig *inst);
	virtual void MixerChanged();
	virtual void GenerateStarted();
	virtual void GenerateFinished();
	virtual void Generate(int autoStart, int todisk);


	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_SELECTALL, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_FIND, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_FINDNEXT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_GOTOLINE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MARKER_SET, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MARKER_NEXT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MARKER_PREV, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MARKER_CLEAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_ITEM_ADD, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_NEW, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_EDIT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_COPY, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_CLOSE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_SAVE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_REMOVE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_ITEM_PROPERTIES, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_ITEM_ERRORS, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_PROJECT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_KEYBOARD, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PROJECT_NEW, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PROJECT_OPEN, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PROJECT_SAVE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PROJECT_SAVEAS, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_WINDOW_CLOSE_ALL, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PROJECT_PROPERTIES, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PROJECT_GENERATE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PROJECT_PLAY, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_HELP_CONTENTS, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(MainFrame)
		NOTIFY_CODE_HANDLER(TBVN_PAGEACTIVATED, OnPageActivated)
		NOTIFY_CODE_HANDLER(TBVN_CONTEXTMENU, OnTabContextMenu)
		NOTIFY_HANDLER(ID_PROJECT_LIST, NM_DBLCLK, OnPrjDblclick)
		NOTIFY_HANDLER(ID_PROJECT_LIST, NM_RCLICK, OnPrjRClick)
		NOTIFY_HANDLER(ID_PROJECT_LIST, TVN_DELETEITEM, OnPrjDelItem)
		NOTIFY_HANDLER(ID_PROJECT_LIST, TVN_SELCHANGED, OnPrjSelChanged)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
		COMMAND_ID_HANDLER(ID_EDIT_REDO, OnEditRedo)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnEditFind)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEXT, OnEditFindNext)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL, OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_GOTOLINE, OnEditGotoLine)
		COMMAND_ID_HANDLER(ID_MARKER_SET, OnMarkerSet)
		COMMAND_ID_HANDLER(ID_MARKER_PREV, OnMarkerPrev)
		COMMAND_ID_HANDLER(ID_MARKER_NEXT, OnMarkerNext)
		COMMAND_ID_HANDLER(ID_MARKER_CLEAR, OnMarkerClear)
		COMMAND_ID_HANDLER(ID_PROJECT_NEW, OnNewProject)
		COMMAND_ID_HANDLER(ID_PROJECT_OPEN, OnOpenProject)
		COMMAND_ID_HANDLER(ID_PROJECT_SAVE, OnSaveProject)
		COMMAND_ID_HANDLER(ID_PROJECT_SAVEAS, OnSaveProjectAs)
		COMMAND_ID_HANDLER(ID_PROJECT_GENERATE, OnProjectGenerate)
		COMMAND_ID_HANDLER(ID_PROJECT_PLAY, OnProjectPlay)
		COMMAND_ID_HANDLER(ID_PROJECT_OPTIONS, OnProjectOptions)
		COMMAND_ID_HANDLER(ID_ITEM_NEW, OnNewItem)
		COMMAND_ID_HANDLER(ID_ITEM_ADD, OnAddItem)
		COMMAND_ID_HANDLER(ID_ITEM_EDIT, OnEditItem)
		COMMAND_ID_HANDLER(ID_ITEM_SAVE, OnSaveItem)
		COMMAND_ID_HANDLER(ID_ITEM_CLOSE, OnCloseItem)
		COMMAND_ID_HANDLER(ID_ITEM_COPY, OnCopyItem)
		COMMAND_ID_HANDLER(ID_ITEM_REMOVE, OnRemoveItem)
		COMMAND_ID_HANDLER(ID_ITEM_PROPERTIES, OnItemProperties)
		COMMAND_ID_HANDLER(ID_ITEM_ERRORS, OnItemErrors)
		COMMAND_ID_HANDLER(ID_PROJECT_PROPERTIES, OnItemProperties)
		COMMAND_ID_HANDLER(ID_VIEW_PROJECT, OnViewProject)
		COMMAND_ID_HANDLER(ID_VIEW_KEYBOARD, OnViewKeyboard)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE, OnWindowClose)
		COMMAND_ID_HANDLER(ID_WINDOW_SAVE, OnWindowSave)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
		COMMAND_ID_HANDLER(ID_NEXT_PANE, OnNextPane)
		COMMAND_ID_HANDLER(ID_PREV_PANE, OnPrevPane)
		COMMAND_RANGE_HANDLER(ID_WINDOW_TABFIRST, ID_WINDOW_TABLAST, OnWindowActivate)
		COMMAND_RANGE_HANDLER(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnRecentProject)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_HELP_CONTENTS, OnHelpContents)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnPosChanged)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_POWERBROADCAST, OnSuspend)
		MESSAGE_HANDLER(WM_QUERYENDSESSION, OnQueryTerminate)
		MESSAGE_HANDLER(WM_ENDSESSION, OnTerminate)
		NOTIFY_CODE_HANDLER(RBN_AUTOSIZE, OnRBAutoSize)
		CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<MainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

	LRESULT OnRBAutoSize(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) { return 0; }
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSuspend(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnQueryTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOpenProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSaveProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSaveProjectAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRecentProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnProjectGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnProjectPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnProjectOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnNewItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSaveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCloseItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnItemProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnItemErrors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSaveAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnEditUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditGotoLine(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditFindNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMarkerSet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMarkerNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMarkerPrev(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMarkerClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnViewProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewKeyboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHelpContents(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowCloseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNextPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPrevPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowActivate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT OnPageActivated(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTabContextMenu(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPrjDblclick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPrjRClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPrjDelItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPrjSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
};
