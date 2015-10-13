//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
/// Text editor window.
/// This is the editor for Notelist, sequencer, script and text files. For Notelist it provides
/// syntax coloring. The editor window is mainly a container for the scintilla editor. It is
/// possible to replace the scintilla editor with something else by writing the equivalent of 
/// the ScintillaCtrl class for the editor. (but you would be hard-pressed to find a better pulgin editor) 
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#define SCI_NAMESPACE
#include <Scintilla.h>
#include <SciLexer.h>

#define NLSTYLE_DEFAULT 1
#define NLSTYLE_KEYWORD 2
#define NLSTYLE_QUOTE   3
#define NLSTYLE_DELIM   4
#define NLSTYLE_COMMENT 5

// Wrapper for Scintilla functions.
// N.B. - Since this uses the direct function, it is not thread safe.
class ScintillaCtrl : public CWindow
{
private:
	int sciFlags;
	SciFnDirect sciMsg;
	sptr_t sciWnd;

public:
	ScintillaCtrl() 
	{ 
		sciFlags = 0; 
		sciMsg = 0;
		sciWnd = 0;
	}

	void Init()
	{
		sciMsg = (SciFnDirect)SendMessage(SCI_GETDIRECTFUNCTION, 0, 0);
		sciWnd = (sptr_t)SendMessage(SCI_GETDIRECTPOINTER, 0, 0);
	}

	inline int CallScintilla(int msg, WPARAM wp = 0, LPARAM lp = 0)
	{
		return sciMsg(sciWnd, msg, wp, lp);
	}

	inline int IsChanged()  { return sciMsg(sciWnd, SCI_GETMODIFY, 0, 0); }
	inline void Undo()      { sciMsg(sciWnd, SCI_UNDO, 0, 0); }
	inline void Redo()      { sciMsg(sciWnd, SCI_REDO, 0, 0); }
	inline void Cut()       { sciMsg(sciWnd, SCI_CUT, 0, 0); }
	inline void Copy()      { sciMsg(sciWnd, SCI_COPY, 0, 0); }
	inline void Paste()     { sciMsg(sciWnd, SCI_PASTE, 0, 0); }
	inline void SetSelAll() { sciMsg(sciWnd, SCI_SELECTALL, 0, 0); }
	inline int CanPaste()   { return sciMsg(sciWnd, SCI_CANPASTE, 0, 0); }
	inline int CanUndo()    { return sciMsg(sciWnd, SCI_CANUNDO, 0, 0); }
	inline int CanRedo()    { return sciMsg(sciWnd, SCI_CANREDO, 0, 0); }
	inline void GotoLine(int line) { sciMsg(sciWnd, SCI_GOTOLINE, line, 0); }
	inline void GotoPosition(int pos) { sciMsg(sciWnd, SCI_GOTOPOS, pos, 0); }
	inline void SetSavePoint() { sciMsg(sciWnd, SCI_SETSAVEPOINT, 0, 0); }
	inline int GetTargetEnd()  { return sciMsg(sciWnd, SCI_GETTARGETEND, 0, 0); }

	inline void BeginUndoAction() { sciMsg(sciWnd, SCI_BEGINUNDOACTION, 0, 0); }
	inline void EndUndoAction()   { sciMsg(sciWnd, SCI_ENDUNDOACTION, 0, 0); }
	inline void EmptyUndoBuffer() { sciMsg(sciWnd, SCI_EMPTYUNDOBUFFER, 0, 0); }

	// to support styling
	inline int GetEndStyled() { return sciMsg(sciWnd, SCI_GETENDSTYLED, 0, 0); }
	inline int LineFromPosition(int pos) { return sciMsg(sciWnd, SCI_LINEFROMPOSITION, pos, 0); }
	inline int PositionFromLine(int pos) { return sciMsg(sciWnd, SCI_POSITIONFROMLINE, pos, 0); }
	inline int GetLineEndPosition(int pos) { return sciMsg(sciWnd, SCI_GETLINEENDPOSITION, pos, 0); }
	inline int GetCharAt(int pos) { return 0xff & sciMsg(sciWnd, SCI_GETCHARAT, pos, 0); }
	inline int StartStyling(int pos) { return sciMsg(sciWnd, SCI_STARTSTYLING, pos, 0x1F); }
	inline int SetStyling(int len, int style) { return sciMsg(sciWnd, SCI_SETSTYLING, len, style); }

	inline int Length()
	{
		// This function gets called from the generator thread...
		return SendMessage(SCI_GETLENGTH, 0, 0);
	}

	inline int CurrentPos()
	{
		return sciMsg(sciWnd, SCI_GETCURRENTPOS, 0, 0);
	}

	inline int CurrentLine()
	{
		return sciMsg(sciWnd, SCI_LINEFROMPOSITION, CurrentPos(), 0);
	}

	inline void GetText(char *t, int len)
	{
		// This function gets called from the generator thread...
		SendMessage(SCI_GETTEXT, len, (LPARAM) t);
	}

	inline void SetText(const char *text)
	{
	    sciMsg(sciWnd, SCI_SETTEXT, 0, (LPARAM) text);
		//SendMessage(SCI_SETTEXT, 0, (LPARAM) text);
	}

	void SetupStyling(int nl)
	{
		sciMsg(sciWnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
		sciMsg(sciWnd, SCI_STYLESETFORE, STYLE_DEFAULT, prjOptions.editTextColor);
		sciMsg(sciWnd, SCI_STYLESETSIZE, STYLE_DEFAULT, prjOptions.editFontSize);
		sciMsg(sciWnd, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM) prjOptions.editFontFace);
		sciMsg(sciWnd, SCI_SETTABWIDTH, prjOptions.editTabSize, 0);
		sciMsg(sciWnd, SCI_SETINDENT, 0, 0);
		sciMsg(sciWnd, SCI_SETUSETABS, 1, 0);
		sciMsg(sciWnd, SCI_MARKERDEFINE, 0, SC_MARK_CIRCLE);
		sciMsg(sciWnd, SCI_MARKERSETFORE, 0, RGB(0,0,160));
		sciMsg(sciWnd, SCI_MARKERSETBACK, 0, RGB(0,0,160));
		if (nl)
		{
			sciMsg(sciWnd, SCI_SETLEXER, SCLEX_CONTAINER, 0);
			sciMsg(sciWnd, SCI_STYLECLEARALL, 0, 0);
			sciMsg(sciWnd, SCI_STYLESETFORE, NLSTYLE_KEYWORD, RGB(0,0,0x80));
			sciMsg(sciWnd, SCI_STYLESETBOLD, NLSTYLE_KEYWORD, 1);
			//sciMsg(sciWnd, SCI_STYLESETBOLD, NLSTYLE_KEYWORD, 1);
			sciMsg(sciWnd, SCI_STYLESETFORE, NLSTYLE_QUOTE, RGB(0xC0,0x20,0));
			sciMsg(sciWnd, SCI_STYLESETFORE, NLSTYLE_DELIM, RGB(0x80,0,0x20));
			//sciMsg(sciWnd, SCI_STYLESETBOLD, NLSTYLE_DELIM, 1);
			sciMsg(sciWnd, SCI_STYLESETFORE, NLSTYLE_COMMENT, RGB(0,0x60,0));
			for (int n = NLSTYLE_DEFAULT; n <= NLSTYLE_COMMENT; n++)
				sciMsg(sciWnd, SCI_STYLESETFONT, n, (LPARAM) prjOptions.editFontFace);
			//SendMessage(SCI_STYLESETITALIC, NLSTYLE_COMMENT, 1);
		}
		else
		{
			sciMsg(sciWnd, SCI_SETLEXER, SCLEX_NULL, 0);
		}
		sciMsg(sciWnd, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	}

	void SetMargins()
	{
		int marginLines = sciMsg(sciWnd, SCI_GETLINECOUNT, 0, 0) * 10;
		if (marginLines < 999)
			marginLines = 999;
		char digstr[20];
		int ndx = 0;
		for (int d = marginLines; d > 0 && ndx < 20; d /= 10)
			digstr[ndx++] = '9';
		digstr[ndx] = 0;
		sciMsg(sciWnd, SCI_SETMARGINWIDTHN, 0, 
			sciMsg(sciWnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)digstr));
	}

	void MarkerAdd(int line)
	{
		sciMsg(sciWnd, SCI_MARKERADD, line, 0);
	}

	void MarkerDel(int line)
	{
		if (line >= 0)
			sciMsg(sciWnd, SCI_MARKERDELETE, line, 0);
		else
			sciMsg(sciWnd, SCI_MARKERDELETEALL, 0, 0);
	}

	int MarkerNext(int line)
	{
		return sciMsg(sciWnd, SCI_MARKERNEXT, line, 1);
	}

	int MarkerPrev(int line)
	{
		return sciMsg(sciWnd, SCI_MARKERPREVIOUS, line, 1);
	}

	int MarkerGet(int line)
	{
		return sciMsg(sciWnd, SCI_MARKERGET, line, 0);
	}

	inline void SetTarget(int start, int end)
	{
		sciMsg(sciWnd, SCI_SETTARGETSTART, start, 0);
		sciMsg(sciWnd, SCI_SETTARGETEND, end, 0);
	}

	int GetSelection(SelectInfo& info)
	{
		info.startPos = sciMsg(sciWnd, SCI_GETSELECTIONSTART, 0, 0);
		info.endPos = sciMsg(sciWnd, SCI_GETSELECTIONEND, 0, 0);
		info.startLn = sciMsg(sciWnd, SCI_LINEFROMPOSITION, info.startPos, 0);
		info.startCh = info.startPos - sciMsg(sciWnd, SCI_POSITIONFROMLINE, info.startLn, 0);
		info.endLn = sciMsg(sciWnd, SCI_LINEFROMPOSITION, info.endPos, 0);
		info.endCh = info.endPos - sciMsg(sciWnd, SCI_POSITIONFROMLINE, info.endLn, 0);
		return info.startPos != info.endPos;
	}

	int GetSelection(long& start, long& end) 
	{
		start = sciMsg(sciWnd, SCI_GETSELECTIONSTART, 0, 0);
		end = sciMsg(sciWnd, SCI_GETSELECTIONEND, 0, 0);
		return 0;
	}

	void SetSelection(SelectInfo& info)
	{
		int start = sciMsg(sciWnd, SCI_POSITIONFROMLINE, info.startLn, 0) + info.startCh;
		int end   = sciMsg(sciWnd, SCI_POSITIONFROMLINE, info.endLn, 0) + info.endCh;
		SetSelection(start, end);
	}

	void SetSelection(int start, int end)
	{
		sciMsg(sciWnd, SCI_SETANCHOR, start, 0);
		sciMsg(sciWnd, SCI_SETCURRENTPOS, end, 0);
		sciMsg(sciWnd, SCI_SCROLLCARET, 0, 0);
	}

	int SetSearchFlags(int flags)
	{
		sciFlags = 0;
		if (flags & TXTFIND_MATCHCASE)
			sciFlags |= SCFIND_MATCHCASE;
		if (flags & TXTFIND_WHOLEWORD)
			sciFlags |= SCFIND_WHOLEWORD;
		if (flags & TXTFIND_WORDSTART)
			sciFlags |= SCFIND_WORDSTART;
		if (flags & TXTFIND_REGEXP)
			sciFlags |= SCFIND_REGEXP;
		sciMsg(sciWnd, SCI_SETSEARCHFLAGS, sciFlags, 0);
		return sciFlags;
	}

	void ReplaceSel(const char *rtext)
	{
		sciMsg(sciWnd, SCI_TARGETFROMSELECTION, 0, 0);
		int msg = (sciFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
		sciMsg(sciWnd, msg, (WPARAM) -1, (LPARAM) (LPCTSTR) rtext);
	}

	int Find(const char *ftext, int start, int end = -1)
	{
		Scintilla::TextToFind ttf;
		ttf.chrg.cpMin = start;
		if (end == -1)
			ttf.chrg.cpMax = Length() + 1;
		else
			ttf.chrg.cpMax = end;
		ttf.lpstrText = (char *)ftext;
		int found = sciMsg(sciWnd, SCI_FINDTEXT, (WPARAM) sciFlags, (LPARAM) &ttf);
		if (found >= 0)
			SetSelection(ttf.chrgText.cpMin, ttf.chrgText.cpMax);
		return found;
	}

	inline int SearchInTarget(int flen, const char *ftext)
	{
		return sciMsg(sciWnd, SCI_SEARCHINTARGET, flen, (LPARAM) ftext);
	}

	/// Test the current selection for a match with the text.
	int Match(const char *ftext)
	{
		int start = sciMsg(sciWnd, SCI_GETSELECTIONSTART, 0, 0);
		int end = sciMsg(sciWnd, SCI_GETSELECTIONEND, 0, 0);
		if (start == end)
			return 0;
		Scintilla::TextToFind ttf;
		ttf.chrg.cpMin = start;
		ttf.chrg.cpMax = end;
		ttf.lpstrText = (char *)ftext;
		if (sciMsg(sciWnd, SCI_FINDTEXT, (WPARAM) sciFlags, (LPARAM) &ttf) >= 0)
			return start == ttf.chrg.cpMin && end == ttf.chrg.cpMax;
		return 0;
	}

	int MatchBrace(int pos1 = -1)
	{
		if (pos1 == -1)
			pos1 = CurrentPos();
		return sciMsg(sciWnd, SCI_BRACEMATCH, pos1, 0);
	}
};

class TextEditorWin : 
	public CWindowImpl<TextEditorWin>,
	public TextEditor
{
protected:
	ScintillaCtrl edwnd;
	bsString file;
	ProjectItem *pi;
	int changed;
	int findFlags;
	bsString findText;
	Color bgColor;

public:
	TextEditorWin();
	virtual ~TextEditorWin();

	HWND CreateEditorWindow(HWND parent, RECT& rc);
	void TextChanged();
	void Restyle(int position);
	void UpdateUI();

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

	DECLARE_WND_CLASS_EX("EditFrame", CS_HREDRAW|CS_VREDRAW, 0)

	BEGIN_MSG_MAP(TextEditorWin)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
		NOTIFY_CODE_HANDLER(SCN_UPDATEUI, OnEditUpdateUI)
		NOTIFY_CODE_HANDLER(SCN_STYLENEEDED, OnEditRestyle)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditUpdateUI(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnEditRestyle(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
};

#endif
