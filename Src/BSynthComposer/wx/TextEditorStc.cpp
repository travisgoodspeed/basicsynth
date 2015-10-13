//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#if wxUSE_STC
#include "TextEditorStc.h"
#include "FindReplDlg.h"

static FindReplDlg *findDlg;

void TextEditorStc::DeleteFindReplDlg()
{
	if (findDlg)
	{
		findDlg->Destroy();
		findDlg = 0;
	}
}

BEGIN_EVENT_TABLE(TextEditorStc,wxWindow)
	EVT_SET_FOCUS(TextEditorStc::OnFocus)
	EVT_PAINT(TextEditorStc::OnPaint)
	EVT_SIZE(TextEditorStc::OnSize)
	EVT_ERASE_BACKGROUND(TextEditorStc::OnEraseBackground)
	EVT_STC_STYLENEEDED(ID_EDIT_CTL, TextEditorStc::OnEditRestyle)
	EVT_STC_UPDATEUI(ID_EDIT_CTL, TextEditorStc::OnEditUpdateUI)
END_EVENT_TABLE()


TextEditorStc::TextEditorStc(wxWindow *parent, ProjectItem *p)
  : wxWindow(parent, wxWindowID(ID_EDIT_WND), wxDefaultPosition, wxDefaultSize)
{
	sciFlags = 0;
	findFlags = 0;
	pi = p;
	edwnd = new wxStyledTextCtrl(this, wxWindowID(ID_EDIT_CTL), wxPoint(0,0), wxDefaultSize, wxVSCROLL|wxHSCROLL|wxBORDER_SIMPLE);

	wxString face(prjOptions.editFontFace);
	edwnd->SetCodePage(wxSTC_CP_UTF8);
//	edwnd->StyleSetForeground(STYLE_DEFAULT, wxColour(prjOptions.editTextColor));
	edwnd->StyleSetSize(wxSTC_STYLE_DEFAULT, prjOptions.editFontSize);
	edwnd->StyleSetFaceName(wxSTC_STYLE_DEFAULT, face);
	edwnd->SetTabWidth(prjOptions.editTabSize);
	edwnd->SetIndent(prjOptions.editTabSize);
	edwnd->SetUseTabs(true);
	edwnd->MarkerDefine(0, wxSTC_MARK_ROUNDRECT, wxColour(0,0,200), wxColour(0,0,160));
	if (pi && pi->GetType() == PRJNODE_NOTEFILE)
	{
		edwnd->SetLexer(wxSTC_LEX_CONTAINER);
		edwnd->StyleClearAll();
		edwnd->StyleSetForeground(NLSTYLE_DEFAULT, wxColour(prjOptions.editTextColor));
		edwnd->StyleSetForeground(NLSTYLE_KEYWORD, wxColour(0,0,0x80));
		edwnd->StyleSetBold(NLSTYLE_KEYWORD, true);
		edwnd->StyleSetForeground(NLSTYLE_QUOTE, wxColour(0x60,0x20,0));
		edwnd->StyleSetForeground(NLSTYLE_DELIM, wxColour(0x40,0,0));
		//edwnd->StyleSetBold(NLSTYLE_DELIM, true);
		edwnd->StyleSetForeground(NLSTYLE_COMMENT, wxColour(0,0x80,0));
		for (int n = NLSTYLE_DEFAULT; n <= NLSTYLE_COMMENT; n++)
			edwnd->StyleSetFaceName(n, face);
	}
	else
		edwnd->SetLexer(wxSTC_LEX_NULL);
	edwnd->SetMarginType(0, wxSTC_MARGIN_NUMBER);
}

TextEditorStc::~TextEditorStc()
{
	if (findDlg && (TextEditorStc*)findDlg->GetEditor() == this)
		findDlg->SetEditor(0);
}

ProjectItem *TextEditorStc::GetItem()
{
	return pi;
}

void TextEditorStc::SetItem(ProjectItem *p)
{
	pi = p;
	pi->SetEditor(this);
}

void TextEditorStc::Undo()
{
	edwnd->Undo();
}

void TextEditorStc::Redo()
{
	edwnd->Redo();
}

void TextEditorStc::Cut()
{
	edwnd->Cut();
}

void TextEditorStc::Copy()
{
	edwnd->Copy();
}

void TextEditorStc::Paste()
{
	edwnd->Paste();
}

void TextEditorStc::Find()
{
	if (!findDlg)
		findDlg = new FindReplDlg(wxTheApp->GetTopWindow());
	findDlg->SetEditor(this);
	findDlg->Show();
}

void TextEditorStc::FindNext()
{
	if (findText.Length() == 0)
		Find();
	else
		Find(findFlags, findText);
}

void TextEditorStc::SelectAll()
{
	edwnd->SelectAll();
}

void TextEditorStc::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[20];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number", value, 20))
			return;
		ln = atoi(value);
	}
	edwnd->GotoLine(ln);
	edwnd->EnsureVisible(ln);
	edwnd->EnsureCaretVisible();
}

void TextEditorStc::GotoPosition(int pos)
{
	edwnd->GotoPos(pos);
	edwnd->EnsureCaretVisible();
}

void TextEditorStc::Cancel()
{
	edwnd->Undo();
}

void TextEditorStc::SetMarker()
{
	SetMarkerAt(edwnd->GetCurrentLine(), -1);
}

void TextEditorStc::SetMarkerAt(int line, int on)
{
	if (on == -1 && edwnd->MarkerGet(line))
		edwnd->MarkerDelete(line, 0);
	else if (on)
		edwnd->MarkerAdd(line, 0);
	else
		edwnd->MarkerDelete(line, 0);
}

void TextEditorStc::NextMarker()
{
	int line = edwnd->GetCurrentLine();
	if (edwnd->MarkerGet(line))
		line++;
	int found = edwnd->MarkerNext(line, 1);
	if (found == -1 && line)
		found = edwnd->MarkerNext(0, 1);
	if (found >= 0)
		GotoLine(found);
}

void TextEditorStc::PrevMarker()
{
	int line = edwnd->GetCurrentLine();
	if (edwnd->MarkerGet(line))
		line--;
	line = edwnd->MarkerPrevious(line, 1);
	if (line >= 0)
		GotoLine(line);
}

void TextEditorStc::ClearMarkers()
{
	edwnd->MarkerDelete(-1, 0);
}

long TextEditorStc::EditState()
{
	long flags = VW_ENABLE_FILE | VW_ENABLE_GOTO | VW_ENABLE_SELALL
	           | VW_ENABLE_FIND | VW_ENABLE_MARK | VW_ENABLE_UNMARK;
	if (edwnd->CanPaste())
		flags |= VW_ENABLE_PASTE;
	if (edwnd->CanRedo())
		flags |= VW_ENABLE_REDO;
	if (edwnd->CanUndo())
		flags |= VW_ENABLE_UNDO;
	long start = 0;
	long end = 0;
	edwnd->GetSelection(&start, &end);
	if (start != end)
		flags |= VW_ENABLE_COPY|VW_ENABLE_CUT;
	return flags;
}

int TextEditorStc::IsChanged()
{
	return edwnd->GetModify() ? 1 : 0;
}

void TextEditorStc::Focus()
{
	edwnd->SetFocus();
}


#define KWMAXLEN 10
// isalpha(c) throws debug exceptions on UNICODE chars
#define AtoZ(c) (( (c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

void TextEditorStc::Restyle(int position)
{
	int endStyled = edwnd->GetEndStyled();
	int lineNumber = edwnd->LineFromPosition(endStyled);
	int startPos = edwnd->PositionFromLine(lineNumber);

	int chPos = startPos;
	int ch = edwnd->GetCharAt(chPos);
	while (chPos < position)
	{
		if (strchr(",;{}[]():=", ch) != 0)
		{
			if (startPos != chPos)
			{
				edwnd->StartStyling(startPos, 0x1F);
				edwnd->SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			edwnd->StartStyling(chPos, 0x1F);
			int lenPos = 0;
			do
			{
				lenPos++;
				ch = edwnd->GetCharAt(++chPos);
			} while (ch > 0 && strchr(",;{}[]():=", ch) != 0 && chPos < position);
			edwnd->SetStyling(lenPos, NLSTYLE_DELIM);
			startPos = chPos;
		}
		else if (ch == '!' || ch == '\'')
		{
			if (startPos != chPos)
			{
				edwnd->StartStyling(startPos, 0x1F);
				edwnd->SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			int eol = edwnd->GetLineEndPosition(edwnd->LineFromPosition(chPos));
			edwnd->StartStyling(chPos, 0x1F);
			edwnd->SetStyling(eol - chPos, NLSTYLE_COMMENT);
			chPos = eol+1;
			ch = edwnd->GetCharAt(chPos);
			startPos = chPos;
		}
		else if (ch == '"')
		{
			if (startPos != chPos)
			{
				edwnd->StartStyling(startPos, 0x1F);
				edwnd->SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			startPos = chPos;
			do
			{
				ch = edwnd->GetCharAt(++chPos);
			} while (ch > 0 && ch != '"' && ch != '\n' && chPos < position);
			if (ch == '"')
				ch = edwnd->GetCharAt(++chPos);
			edwnd->StartStyling(startPos, 0x1F);
			edwnd->SetStyling(chPos - startPos, NLSTYLE_QUOTE);
			startPos = chPos;
		}
		else if (AtoZ(ch))
		{
			// possible keyword
			char kwbuf[KWMAXLEN+1];
			int kwpos = 0;
			int kwStart = chPos;
			do
			{
				kwbuf[kwpos++] = ch;
				ch = edwnd->GetCharAt(++chPos);
			} while (ch > 0 && AtoZ(ch) && kwpos <= KWMAXLEN && chPos < position);
			kwbuf[kwpos] = '\0';
			if (nlLex::IsKeyword(kwbuf) != T_ENDOF)
			{
				if (startPos != kwStart)
				{
					edwnd->StartStyling(startPos, 0x1F);
					edwnd->SetStyling(kwStart - startPos, NLSTYLE_DEFAULT);
				}
				edwnd->StartStyling(kwStart, 0x1F);
				edwnd->SetStyling(chPos - kwStart, NLSTYLE_KEYWORD);
				startPos = chPos;
			}
		}
		else
			ch = edwnd->GetCharAt(++chPos);
	}
	if (startPos != chPos)
	{
		edwnd->StartStyling(startPos, 0x1F);
		edwnd->SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
	}
}

int TextEditorStc::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;

	if (::SynthFileExists(file))
	{
		wxMBConvUTF8 cutf8;
		edwnd->LoadFile(wxString((const char *)file, cutf8));
		// the LoadFile function selects all text, pffftttt...
		edwnd->SetSelection(0,0);
	}
	else
	{
		edwnd->AddText(wxString("\n"));
		edwnd->EmptyUndoBuffer();
		edwnd->SetSavePoint();
		edwnd->SetCurrentPos(0);
	}

	SetMargins();
	edwnd->SetFocus();

	return 0;
}

void TextEditorStc::SetMargins()
{
	int marginLines = edwnd->GetLineCount();
	if (marginLines < 999)
		marginLines = 999;
	wxString digstr;
	for (int d = marginLines; d > 0; d /= 10)
		digstr += '9';
	edwnd->SetMarginWidth(0, edwnd->TextWidth(wxSTC_STYLE_LINENUMBER, digstr));
}

int TextEditorStc::SaveFile(const char *fname)
{
	if (pi == NULL)
		return -1;

	if (fname == 0 || *fname == 0)
	{
		if (file.Length() == 0)
		{
			const char *spc = ProjectItem::GetFileSpec(pi->GetType());
			const char *ext = ProjectItem::GetFileExt(pi->GetType());
			if (!prjFrame->BrowseFile(0, file, spc, ext))
				return -1;
		}
	}
	else
		file = fname;

	wxMBConvUTF8 cutf8;
	if (!edwnd->SaveFile(wxString((const char *)file, cutf8)))
		return -1;

	edwnd->SetSavePoint();
	return 0;
}

int TextEditorStc::GetText(bsString& text)
{
	text = (const char *) edwnd->GetText().c_str();
	return (int) text.Length();
}

int TextEditorStc::SetText(bsString& text)
{
	wxString t((const char*)text);
	edwnd->SetText(t);
	return 0;
}

void TextEditorStc::SetSearchFlags(int flags)
{
	sciFlags = 0;
	if (flags & TXTFIND_MATCHCASE)
		sciFlags |= wxSTC_FIND_MATCHCASE;
	if (flags & TXTFIND_WHOLEWORD)
		sciFlags |= wxSTC_FIND_WHOLEWORD;
	if (flags & TXTFIND_WORDSTART)
		sciFlags |= wxSTC_FIND_WORDSTART;
	if (flags & TXTFIND_REGEXP)
		sciFlags |= wxSTC_FIND_REGEXP;
	edwnd->SetSearchFlags(sciFlags);
}

// copied from scintilla.h
struct Sci_CharacterRange {
	long cpMin;
	long cpMax;
};

struct Sci_TextToFind {
	struct Sci_CharacterRange chrg;
	char *lpstrText;
	struct Sci_CharacterRange chrgText;
};

int TextEditorStc::Find(int flags, const char *text)
{
	findFlags = flags;
	findText = text;
	SetSearchFlags(flags);
	int wrap = 0;
	int start = edwnd->GetCurrentPos();
// wx doesn't provide find with the TextFind struct, which is
// needed to determine the amount of text matched. So, we
// send the message directly.
//	int found = edwnd->FindText(start, edwnd->GetLength() - 1, findText, sciFlags);
	Sci_TextToFind ttf;
	ttf.chrg.cpMin = start;
	ttf.chrg.cpMax = edwnd->GetLength() + 1;
	ttf.lpstrText = (char *)text;
	int found = edwnd->SendMsg(2150, sciFlags, (wxIntPtr) &ttf);
	if (found == -1 && start > 0)
	{
		//found = edwnd->FindText(0, start, findText, sciFlags);
		ttf.chrg.cpMin = 0;
		found = edwnd->SendMsg(2150, sciFlags, (wxIntPtr) &ttf);
		wrap = 1;
	}
	if (found >= 0)
	{
		edwnd->SetSelection(ttf.chrgText.cpMin, ttf.chrgText.cpMax);
		edwnd->SetAnchor(ttf.chrgText.cpMin);
		edwnd->SetCurrentPos(ttf.chrgText.cpMax);
		edwnd->EnsureCaretVisible();
		return wrap;
	}
	return -1;
}

int TextEditorStc::MatchSel(int flags, const char *text)
{
	SetSearchFlags(flags);
	int start = edwnd->GetSelectionStart();
	int end = edwnd->GetSelectionEnd();
	if (start == end)
		return 0;
//	int found = edwnd->FindText(start, end, wxString(text), sciFlags);
	Sci_TextToFind ttf;
	ttf.chrg.cpMin = start;
	ttf.chrg.cpMax = end;
	ttf.lpstrText = (char *)text;
	int found = edwnd->SendMsg(2150, sciFlags, (wxIntPtr) &ttf);
	if (found >= 0)
	{
		return start == ttf.chrg.cpMin && end == ttf.chrg.cpMax;
	}
	return 0;
}

void TextEditorStc::Replace(const char *rtext)
{
	edwnd->ReplaceSelection(wxString(rtext));
}

int TextEditorStc::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	findFlags = flags;
	findText = ftext;
	int count = 0;
	SetSearchFlags(flags);
	edwnd->BeginUndoAction();
	SelectInfo si;
	si.startPos = info.startPos;
	si.endPos = info.endPos;
	if (si.startPos == si.endPos)
	{
		si.endPos = edwnd->GetLength();
		si.endLn = edwnd->LineFromPosition(si.endPos)+1;
		si.endCh = 0;
	}
	else
	{
		si.endLn = info.endLn;
		si.endCh = info.endCh;
	}

	wxString rt(rtext);
	wxString ft(ftext);

	do
	{
		edwnd->SetTargetStart(si.startPos);
		edwnd->SetTargetEnd(si.endPos);
		if (edwnd->SearchInTarget(ft) == -1)
			break;
		count++;
		if (flags & wxSTC_FIND_REGEXP)
			edwnd->ReplaceTargetRE(rt);
		else
			edwnd->ReplaceTarget(rt);
		si.startPos = edwnd->GetTargetEnd();
		si.endPos = edwnd->PositionFromLine(si.endLn) + si.endCh;
	} while (si.startPos <= si.endPos);
	edwnd->EndUndoAction();
	return count;
}


int TextEditorStc::GetSelection(SelectInfo& info)
{
	info.startPos = edwnd->GetSelectionStart();
	info.endPos = edwnd->GetSelectionEnd();
	info.startLn = edwnd->LineFromPosition(info.startPos);
	info.startCh = info.startPos - edwnd->PositionFromLine(info.startLn);
	info.endLn = edwnd->LineFromPosition(info.endPos);
	info.endCh = info.endPos - edwnd->PositionFromLine(info.endLn);
	return info.startPos != info.endPos;
}

void TextEditorStc::SetSelection(SelectInfo& info)
{
	int start = edwnd->PositionFromLine(info.startLn) + info.startCh;
	int end   = edwnd->PositionFromLine(info.endLn) + info.endCh;
	edwnd->SetAnchor(start);
	edwnd->SetCurrentPos(end);
	edwnd->ScrollToLine(info.startLn);
}

/////////////////////////////////////////////////////////////////////////////////////

void TextEditorStc::OnSize(wxSizeEvent& evt)
{
	edwnd->SetSize(GetClientSize());
}

void TextEditorStc::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	// do nothing - prevents flicker
}


void TextEditorStc::OnEraseBackground(wxEraseEvent& evt)
{
	// do nothing - prevents flicker
}


void TextEditorStc::OnEditRestyle(wxStyledTextEvent& evt)
{
	Restyle(evt.GetPosition());
}

void TextEditorStc::OnEditUpdateUI(wxStyledTextEvent& evt)
{
	prjFrame->EditStateChanged();
}

void TextEditorStc::OnFocus(wxFocusEvent& evt)
{
	edwnd->SetFocus();
}
#endif
