//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _TEXT_EDITOR_STC
#define _TEXT_EDITOR_STC 1

#define NLSTYLE_DEFAULT 1
#define NLSTYLE_KEYWORD 2
#define NLSTYLE_QUOTE   3
#define NLSTYLE_DELIM   4
#define NLSTYLE_COMMENT 5

/// @brief Text editor window.
class TextEditorStc :
	public wxWindow,
	public TextEditor
{
private:
    DECLARE_EVENT_TABLE()
	wxStyledTextCtrl *edwnd;

	int sciFlags;

	ProjectItem *pi;
	int changed;
	bsString file;

	wxRegEx findRE;
	wxString findText;
	wxString matchText;
	int findFlags;
	long findStart;
	long findEnd;
	long matchStart;
	long matchEnd;

	wxTextAttr defStyle;

	int loading;

	bool InitFind(int flags, const char *text, bool insel = false);
	bool DoFind();
	void Restyle(int position);
	void SetMargins();
	void SetSearchFlags(int flags);

public:
	TextEditorStc(wxWindow *parent, ProjectItem *p = 0);
	~TextEditorStc();

	static void DeleteFindReplDlg();

	virtual ProjectItem *GetItem();
	virtual void SetItem(ProjectItem *p);
	virtual void Undo();
	virtual void Redo();
	virtual void Cut();
	virtual void Copy();
	virtual void Paste();
	virtual void Find();
	virtual void FindNext();
	virtual void SelectAll();
	virtual void GotoLine(int ln);
	virtual void GotoPosition(int pos);
	virtual void SetMarker();
	virtual void SetMarkerAt(int line, int on);
	virtual void NextMarker();
	virtual void PrevMarker();
	virtual void ClearMarkers();
	virtual void Cancel();
	virtual long EditState();
	virtual int IsChanged();
	virtual void Focus();

	virtual int OpenFile(const char *fname);
	virtual int SaveFile(const char *fname);
	virtual int GetText(bsString& text);
	virtual int SetText(bsString& text);
	virtual int Find(int flags, const char *ftext);
	virtual int MatchSel(int flags, const char *ftext);
	virtual void Replace(const char *rtext);
	virtual int ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& sel);
	virtual int GetSelection(SelectInfo& sel);
	virtual void SetSelection(SelectInfo& sel);

	// Event handlers
	void OnPaint(wxPaintEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	void OnSize(wxSizeEvent& evt);
	void OnTextChange(wxCommandEvent& evt);
	void OnEditRestyle(wxStyledTextEvent& evt);
	void OnEditUpdateUI(wxStyledTextEvent& evt);
	void OnFocus(wxFocusEvent& evt);
};

#endif
