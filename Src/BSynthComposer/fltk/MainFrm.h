//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file MainFrame.h Main frame window class declaration
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

class MainMenu;
class ProjectTreeFltk;
class TabsView;
class KbdGenDlg;
class FindReplDlgFltk;
class TextEditorFltk;

#define TREE_VISIBLE 1
#define KBD_VISIBLE  2
#define TABS_VISIBLE 4

class MainFrame : 
	public Fl_Double_Window,
	public ProjectFrame
{
private:
	MainMenu *mnu;
	ProjectTreeFltk *tree;
	TabsView *tabs;
	KbdGenDlg *kbd;
	ProjectItem *curItem;
	EditorView *curEditor;
	Fl_Help_Dialog *helpDlg;
	int visflags;
	int shutdown;

public:
	FindReplDlgFltk *findDlg;

	MainFrame(int X, int Y, int W, int H, const char* t);
	virtual ~MainFrame();

	int Running() { return !shutdown; }

	void resize(int X, int Y, int W, int H);

	void ItemSelected(ProjectItem *p);
	void ItemDoubleClick(ProjectItem *p);
	void ItemContextMenu(ProjectItem *p);
	void EditorSelected(EditorView *vw);
	void ViewProject();
	void ViewKeyboard();
	void Layout();
	void AfterOpenProject();
	void ProjectOptions();
	void ShowAbout();
	void ShowHelp();
	void ShowFind(TextEditorFltk *ed);
	void OpenRecent(long n);

	// ProjectFrame interface
	virtual int Exit();
	virtual int NewProject();
	virtual int OpenProject(const char *fname);
	virtual int CloseProject(int q);
	virtual int BrowseFile(int open, char *file, const char *spec, const char *ext);
	virtual int BrowseFile(int open, bsString& file, const char *spec, const char *ext);
	virtual int Verify(const char *msg, const char *title);
	virtual PropertyBox *CreatePropertyBox(ProjectItem *pi, int type);
	virtual FormEditor *CreateFormEditor(ProjectItem *pi);
	virtual TextEditor *CreateTextEditor(ProjectItem *pi);
	virtual EditorView *GetActiveEditor();
	virtual void EditStateChanged();
	virtual int OpenEditor(ProjectItem *pi);
	virtual int CloseEditor(ProjectItem *pi);
	virtual int CloseAllEditors();
	virtual int SaveAllEditors(int q = 1);
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
	virtual void Generate(int autoStart, int todisk);
};

extern MainFrame *mainWnd;
#endif
