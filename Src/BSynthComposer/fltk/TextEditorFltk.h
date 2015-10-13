//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Text editor window class declaration.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef TEXT_EDITOR_FLTK_H
#define TEXT_EDITOR_FLTK_H

#include <regex.h>
#define RESUBS 10

class TextEditorFltk :
	public Fl_Text_Editor,
	public TextEditor
{
protected:
	bsString file;
	ProjectItem *pi;
	int changed;
	int findFlags;
	bsString findText;
	bsString matchText;
	int findStart;
	int findEnd;
	int matchStart;
	int matchEnd;
	regex_t re;
	regmatch_t resubs[RESUBS];

	bool InitFind(int flags, const char *text, bool insel);
	bool DoFind();
	int DoReplace(const char *rtext);

public:
	TextEditorFltk(int X, int Y, int W, int H);
	virtual ~TextEditorFltk();

	void TextChanged(int pos, int inserted, int deleted, int restyled);
	void CallbackEvent(Fl_Widget *wdg);
	void Resize(wdgRect& rc);
	void Restyle(int position);
	void UpdateUI();
	int IsKeyword(char *txt);

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
	virtual void SetMarker() { }
	virtual void SetMarkerAt(int line, int on) { }
	virtual void NextMarker() { }
	virtual void PrevMarker() { }
	virtual void ClearMarkers() { }
	virtual void Cancel();
	virtual long EditState();
	virtual int IsChanged();
	virtual void Focus() { take_focus(); }

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
};
#endif
