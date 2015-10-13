//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include "ScoreErrorsDlg.h"
#include "KeyboardDlg.h"

/// @brief Object to hold project item data for a tree node.
class bsTreeItemData : public wxTreeItemData
{
public:
	ProjectItem *prjItem;
	wxTreeItemIdValue cookie;

	bsTreeItemData(ProjectItem *p)
	{
		if ((prjItem = p) != NULL)
		{
			p->SetPSData((void*)this);
			p->AddRef();
		}
	}
	virtual ~bsTreeItemData()
	{
		if (prjItem)
			prjItem->Release();
	}
};

#define PROJECT_PANE  0x01
#define MIXER_PANE    0x02
#define PLAYER_PANE   0x04
#define EDITOR_PANE   0x08
#define KEYBOARD_PANE 0x10
#define TOOLBAR_PANE  0x20
#define STATUS_PANE   0x40

class MainFrame;

class MainTree :
	public wxTreeCtrl
{
private:
    DECLARE_EVENT_TABLE()
	MainFrame *frame;
	wxSize szSave;

public:
	MainTree(MainFrame *frm, wxWindow *parent, int w, int h)
		:  wxTreeCtrl(parent, wxWindowID(ID_PROJECT_WND), wxPoint(0, 0), wxSize(w, h),
		wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxBORDER_SIMPLE,
		wxDefaultValidator, wxString("tree"))
	{
	    frame = frm;
	}

	void SaveSize() { szSave = GetSize(); }
	int SavedWidth() { return szSave.GetWidth(); }

	void OnLftDClick(wxMouseEvent& evt);
};

/// @brief Main frame window.
/// @details The main frame window creates the application window,
/// menu, toolbars, project tree and keyboard windows. It receives
/// events from the menu/toolbars and project tree then dispatches
/// to the appropriate Project function.
class MainFrame :
	public wxFrame,
	public ProjectFrame,
	public ProjectTree
{
private:
    DECLARE_EVENT_TABLE()
	wxMenuBar *wxPrjMenu;
	wxToolBar *wxPrjToolBar;
	wxStatusBar *wxPrjStatusBar;
	wxSplitterWindow *wxPrjSplit;
//	wxTreeCtrl *wxPrjTree;
	MainTree *wxPrjTree;
	wxNotebook *wxPrjTabs;
	wxTreeItemId wxTreeRoot;
	wxMenu *ctxProject;
	wxMenu *ctxWaveout;
	wxMenu *ctxSynth;
	wxMenu *ctxFileList;
	wxMenu *ctxLibItem;
	wxMenu *ctxProjectItem;
	wxHtmlHelpController *help;
	wxFileHistory mruList;
	wxString tmppath;
	wxTimer  autosave;
	wxMBConvUTF8 cutf8;
	int bVisible;
	int inGenerate;
	long editFlags;
	long itemFlags;

	ScoreErrorsDlg *errWnd;
	KeyboardDlg *kbdWnd;

	ProjectItem *GetClickedItem();
	int GetItemTab(ProjectItem *itm);
	int GetItemTab(EditorView *vw);

	void AfterOpenProject();
	void UpdateLayout(int rd = 1);
	void EnablePanes();
	void InitAccel();
	void UpdateRecent();
	void SaveTemp(int sv);

public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MainFrame();

	void OnClose(wxCloseEvent& evt);
	void OnSize(wxSizeEvent& evt);
	void OnErase(wxEraseEvent& evt);
	void OnSetFocus(wxFocusEvent& evt);
	void OnExit(wxCommandEvent& evt);
	void OnNewProject(wxCommandEvent& evt);
	void OnOpenProject(wxCommandEvent& evt);
	void OnSaveProject(wxCommandEvent& evt);
	void OnSaveProjectAs(wxCommandEvent& evt);
	void OnProjectGenerate(wxCommandEvent& evt);
	void OnProjectPlay(wxCommandEvent& evt);
	void OnProjectOptions(wxCommandEvent& evt);
	void OnMRUFile(wxCommandEvent& evt);
	void OnTimer(wxTimerEvent& evt);

	void OnNewItem(wxCommandEvent& evt);
	void OnAddItem(wxCommandEvent& evt);
	void OnEditItem(wxCommandEvent& evt);
	void OnSaveItem(wxCommandEvent& evt);
	void OnCloseItem(wxCommandEvent& evt);
	void OnRemoveItem(wxCommandEvent& evt);
	void OnCopyItem(wxCommandEvent& evt);
	void OnItemProperties(wxCommandEvent& evt);
	void OnItemErrors(wxCommandEvent& evt);
	void OnCloseAll(wxCommandEvent& evt);

	void OnEditUndo(wxCommandEvent& evt);
	void OnEditRedo(wxCommandEvent& evt);
	void OnEditCopy(wxCommandEvent& evt);
	void OnEditCut(wxCommandEvent& evt);
	void OnEditPaste(wxCommandEvent& evt);
	void OnEditSelectAll(wxCommandEvent& evt);
	void OnEditGotoLine(wxCommandEvent& evt);
	void OnEditFind(wxCommandEvent& evt);
	void OnEditFindNext(wxCommandEvent& evt);
	void OnMarkerSet(wxCommandEvent& evt);
	void OnMarkerNext(wxCommandEvent& evt);
	void OnMarkerPrev(wxCommandEvent& evt);
	void OnMarkerClear(wxCommandEvent& evt);

	void OnViewProject(wxCommandEvent& evt);
	void OnViewKeyboard(wxCommandEvent& evt);
	void OnViewToolBar(wxCommandEvent& evt);
	void OnViewStatusBar(wxCommandEvent& evt);
	void OnNextPane(wxCommandEvent& evt);
	void OnPrevPane(wxCommandEvent& evt);

	void OnAppAbout(wxCommandEvent& evt);
	void OnHelpContents(wxCommandEvent& evt);

	void OnLftDClick(wxMouseEvent& evt);

	void OnUpdateProjectEnable(wxUpdateUIEvent& evt);
	void OnUpdateEditUndo(wxUpdateUIEvent& evt);
	void OnUpdateEditRedo(wxUpdateUIEvent& evt);
	void OnUpdateEditCopy(wxUpdateUIEvent& evt);
	void OnUpdateEditCut(wxUpdateUIEvent& evt);
	void OnUpdateEditPaste(wxUpdateUIEvent& evt);
	void OnUpdateEditSelectAll(wxUpdateUIEvent& evt);
	void OnUpdateEditGotoLine(wxUpdateUIEvent& evt);
	void OnUpdateEditFind(wxUpdateUIEvent& evt);
	void OnUpdateEditFindNext(wxUpdateUIEvent& evt);
	void OnUpdateEditMark(wxUpdateUIEvent& evt);

	void OnUpdateNewItem(wxUpdateUIEvent& evt);
	void OnUpdateAddItem(wxUpdateUIEvent& evt);
	void OnUpdateEditItem(wxUpdateUIEvent& evt);
	void OnUpdateSaveItem(wxUpdateUIEvent& evt);
	void OnUpdateCloseItem(wxUpdateUIEvent& evt);
	void OnUpdateCopyItem(wxUpdateUIEvent& evt);
	void OnUpdateRemoveItem(wxUpdateUIEvent& evt);
	void OnUpdateItemProperties(wxUpdateUIEvent& evt);
	void OnUpdateItemErrors(wxUpdateUIEvent& evt);
	void OnUpdateCloseAll(wxUpdateUIEvent& evt);

	void OnUpdateViewProject(wxUpdateUIEvent& evt);
	void OnUpdateViewKeyboard(wxUpdateUIEvent& evt);
	void OnUpdateViewToolBar(wxUpdateUIEvent& evt);
	void OnUpdateViewStatusBar(wxUpdateUIEvent& evt);
	void OnUpdateNextPane(wxUpdateUIEvent& evt);

	//void OnPageActivated(wxCommandEvent& evt);
	//void OnTabContextMenu(wxCommandEvent& evt);
	//void OnPrjDblclick(wxCommandEvent& evt);
	void OnPrjRClick(wxTreeEvent& evt);
	void OnPrjSelChanged(wxTreeEvent& evt);
	void OnPageActivated(wxNotebookEvent& evt);

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
};

#endif
