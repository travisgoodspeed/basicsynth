//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WINDOWTYPES_H
#define WINDOWTYPES_H

class WidgetForm;
class ProjectItem;
class SynthWidget;

typedef void* DrawContext;

/// GenerateWindow defines the interface for wave file generation dialog.
/// This dialog provides feedback to the user during wave file generation.
/// This does not have to be a "window" but could write output to the
/// console, or to a log file. The WasCanceled() function allows the
/// wave file generator to poll the user to cancel the generation.
class GenerateWindow
{
public:
	virtual void AddMessage(const char *s) = 0;
	virtual void UpdateTime(long tm) = 0;
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt) = 0;
	virtual void Finished() = 0;
	virtual int WasCanceled() = 0;
};

#define VW_ENABLE_COPY  0x01
#define VW_ENABLE_CUT   0x02
#define VW_ENABLE_PASTE 0x04
#define VW_ENABLE_UNDO  0x08
#define VW_ENABLE_REDO  0x10
#define VW_ENABLE_FILE  0x20 // save, save as, close
#define VW_ENABLE_GOTO  0x40
#define VW_ENABLE_SELALL 0x80
#define VW_ENABLE_FIND   0x100
#define VW_ENABLE_MARK   0x200
#define VW_ENABLE_UNMARK 0x400

#define PRJ_ENABLE_SCORES  1
#define PRJ_ENABLE_SCRIPTS 2
#define PRJ_ENABLE_SEQ     3
#define PRJ_ENABLE_TEXT    4
#define PRJ_ENABLE_LIBS    5

/// EditorView defines the generic interface to one editor window.
class EditorView
{
private:
	void *psdata;  ///< Platform-specific data
public:
	EditorView() { psdata = 0; }
	virtual ~EditorView() { }
	virtual void SetPSData(void *p) { psdata = p; }
	virtual void *GetPSData() { return psdata; }
	virtual ProjectItem *GetItem()  = 0;
	virtual void SetItem(ProjectItem *p) = 0;
	virtual void Undo() = 0;
	virtual void Redo() = 0;
	virtual void Cut() = 0;
	virtual void Copy() = 0;
	virtual void Paste() = 0;
	virtual void Find() = 0;
	virtual void FindNext() = 0;
	virtual void SelectAll() = 0;
	virtual void GotoLine(int ln) = 0;
	virtual void GotoPosition(int pos) = 0;
	virtual void SetMarker() = 0;
	virtual void SetMarkerAt(int line, int on) = 0;
	virtual void NextMarker() = 0;
	virtual void PrevMarker() = 0;
	virtual void ClearMarkers() = 0;
	virtual void Cancel() = 0;
	virtual long EditState() = 0;
	virtual int IsChanged() = 0;
	virtual void Focus() = 0;
};

struct SelectInfo
{
	int startPos;
	int startLn;
	int startCh;
	int endPos;
	int endLn;
	int endCh;
};

#define TXTFIND_MATCHCASE 1
#define TXTFIND_WHOLEWORD 2
#define TXTFIND_WORDSTART 4
#define TXTFIND_REGEXP    8

/// TextEditor specializes EditorView for editing text files (e.g., Notelists).
class TextEditor : public EditorView
{
public:
	virtual ~TextEditor() { }
	virtual int OpenFile(const char *fname) = 0;
	virtual int SaveFile(const char *fname) = 0;
	virtual int GetText(bsString& text) = 0;
	virtual int SetText(bsString& text) = 0;
	virtual int Find(int flags, const char *ftext) = 0;
	virtual int MatchSel(int flags, const char *ftext) = 0;
	virtual void Replace(const char *rtext) = 0;
	virtual int ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& sel) = 0;
	virtual int GetSelection(SelectInfo& sel) = 0;
	virtual void SetSelection(SelectInfo& sel) = 0;
};

/// FormEditor specializes EditorView for editing instruments and
/// anything else that uses a WidgetForm to display the editor.
class FormEditor : public EditorView
{
public:
	virtual ~FormEditor() { }
	virtual void SetForm(WidgetForm *frm) = 0;
	virtual WidgetForm *GetForm() = 0;
	virtual void Capture() = 0;
	virtual void Release() = 0;
	virtual void Redraw(SynthWidget *wdg) = 0;
	virtual void Resize() = 0;
	virtual SynthWidget *SystemWidget(const char *type) = 0;
};

// Generic property page IDS
#define PROP_NAME  1
#define PROP_DESC  2
#define PROP_FILE  3
#define PROP_INCL  4
#define PROP_INUM  5
#define PROP_ITYP  6
#define PROP_ILST  7
#define PROP_WVID  8
#define PROP_REN   9
#define PROP_PRELOAD 10
#define PROP_SCALE 11

// Mixer property page IDs
#define PROP_MIX_CHNL 1
#define PROP_MIX_PLIN 2
#define PROP_MIX_PTRG 3
#define PROP_MIX_PSQR 4
#define PROP_MIX_FX   5

// Effects unit property page IDs
#define PROP_FX_VOL 3
#define PROP_FX_PAN 4
#define PROP_FX_V1  5
#define PROP_FX_V2  6
#define PROP_FX_V3  7
#define PROP_FX_V4  8
#define PROP_FX_V5  9

// Project property page IDs
#define PROP_PRJ_NAME 1
#define PROP_PRJ_AUTH 2
#define PROP_PRJ_CPYR 3
#define PROP_PRJ_DESC 4
#define PROP_PRJ_PATH 5
#define PROP_PRJ_OUTF 6
#define PROP_PRJ_LEAD 7
#define PROP_PRJ_TAIL 8
#define PROP_PRJ_SRT  9
#define PROP_PRJ_SFMT 10
#define PROP_PRJ_WTSZ 11
#define PROP_PRJ_WTU  12
#define PROP_PRJ_WVIN 13

/// PropertyBox defines the interface for a properties page
class PropertyBox
{
public:
	virtual ~PropertyBox() { }
	virtual int Activate(int modal) = 0;
	virtual ProjectItem *GetItem()  = 0;
	virtual void SetItem(ProjectItem *p) = 0;
	virtual void SetCaption(const char *text) = 0;
	virtual void EnableValue(int id, int enable) = 0;
	virtual void SetValue(int id, const char *text, const char *lbl) = 0;
	virtual void SetValue(int id, float val, const char *lbl) = 0;
	virtual void SetValue(int id, long val, const char *lbl) = 0;
	virtual int GetValue(int id, char *text, int len) = 0;
	virtual int GetValue(int id, bsString& text) = 0;
	virtual int GetValue(int id, float& value) = 0;
	virtual int GetValue(int id, long& value) = 0;
	virtual void SetState(int id, short onoff) = 0;
	virtual int GetState(int id, short& onoff) = 0;
	virtual void SetSelection(int id, short sel) = 0;
	virtual int GetSelection(int id, short& sel) = 0;
	virtual void SetSelection(int id, void *sel) = 0;
	virtual int GetSelection(int id, void **sel) = 0;
	virtual int ListChildren(int id, ProjectItem *parent) = 0;
	virtual int GetListCount(int id, int& count) = 0;
	virtual ProjectItem *GetListItem(int id, int ndx) = 0;
};

/// ProjectTree defines the interface to the list of project items.
class ProjectTree
{
public:
	virtual void AddNode(ProjectItem *itm, ProjectItem *sib = 0) = 0;
	virtual void SelectNode(ProjectItem *itm) = 0;
	virtual void RemoveNode(ProjectItem *itm) = 0;
	virtual void UpdateNode(ProjectItem *itm) = 0;
	virtual void MoveNode(ProjectItem *itm, ProjectItem *prev) = 0;
	virtual void RemoveAll() = 0;
	virtual ProjectItem *FirstChild(ProjectItem *itm) = 0;
	virtual ProjectItem *NextSibling(ProjectItem *itm) = 0;
	virtual ProjectItem *GetSelectedNode() = 0;
};

/// ProjectFrame defines the base clas for the frame window.
/// The frame window (not necessarily a graphic window) provides
/// the general functions for the project. Much
/// of the ProjectFrame functionality is implemted in this class.
/// Platform specific functions are pure-virtual and must be
/// implemented by the platform dependent code. A typical implementation
/// passes menu/toolbar callbacks to one of the base class functions.
class ProjectFrame
{
public:
	//////////////////////Platform independent ///////////////////////////////
	const char *prjFileTypes;
	const char *prjFileExt;

	ProjectFrame();
	virtual ~ProjectFrame();
	virtual int NewProject();
	virtual int OpenProject(const char *fname);
	virtual int SaveProject();
	virtual int SaveProjectAs();
	virtual int CloseProject(int query);
	virtual int SaveFile();
	virtual void SaveBackup();
	virtual int CloseFile();
	virtual void GenerateStarted();
	virtual void GenerateFinished();

	virtual void EditItem();
	virtual void SaveItem();
	virtual void CloseItem();
	virtual void NewItem();
	virtual void AddItem();
	virtual void CopyItem();
	virtual void RemoveItem();
	virtual void ItemProperties();

	virtual void EditUndo();
	virtual void EditRedo();
	virtual void EditCopy();
	virtual void EditCut();
	virtual void EditPaste();
	virtual void EditFind();
	virtual void EditFindNext();
	virtual void EditSelectAll();
	virtual void EditGoto();
	virtual void MarkerSet();
	virtual void MarkerNext();
	virtual void MarkerPrev();
	virtual void MarkerClear();

	virtual int QuerySaveProject();

	//////////////////////Platform specific ///////////////////////////////

	virtual int BrowseFile(int open, char *file, const char *spec, const char *ext) = 0;
	virtual int BrowseFile(int open, bsString& file, const char *spec, const char *ext) = 0;
	virtual int QueryValue(const char *prompt, char *value, int len) = 0;
	virtual int Alert(const char *msg, const char *title) = 0;
	virtual int Verify(const char *msg, const char *title) = 0;
	virtual PropertyBox *CreatePropertyBox(ProjectItem *pi, int type) = 0;
	virtual FormEditor *CreateFormEditor(ProjectItem *pi) = 0;
	virtual TextEditor *CreateTextEditor(ProjectItem *pi) = 0;
	virtual EditorView *GetActiveEditor() = 0;
	virtual void EditStateChanged() = 0;
	virtual int OpenEditor(ProjectItem *itm) = 0;
	virtual int CloseEditor(ProjectItem *itm) = 0;
	virtual int CloseAllEditors() = 0;
	virtual int SaveAllEditors(int q = 1) = 0;
	virtual void InitPlayer() = 0;
	virtual int StopPlayer() = 0;
	virtual void StartPlayer() = 0;
	virtual void ClearPlayer() = 0;
	virtual void InstrAdded(InstrConfig *inst) = 0;
	virtual void InstrRemoved(InstrConfig *inst) = 0;
	virtual void InstrChanged(InstrConfig *inst) = 0;
	virtual void MixerChanged() = 0;
	virtual void Generate(int autoStart, int todisk) = 0;
};

/// Global project frame object
extern ProjectFrame *prjFrame;
/// Global project item list object.
extern ProjectTree *prjTree;
/// Global wave generate dialog.
extern GenerateWindow *prjGenerate;

#endif
