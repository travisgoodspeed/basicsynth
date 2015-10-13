//////////////////////////////////////////////////////////////////////
// Platform-specifc text editor
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "TextEditor.h"
#include "FindReplDlg.h"

static FindReplDlg *findDlg;

void TextEditorWX::DeleteFindReplDlg()
{
	if (findDlg)
	{
		findDlg->Destroy();
		findDlg = 0;
	}
}

BEGIN_EVENT_TABLE(TextEditorWX,wxWindow)
	EVT_PAINT(TextEditorWX::OnPaint)
	EVT_SIZE(TextEditorWX::OnSize)
	EVT_ERASE_BACKGROUND(TextEditorWX::OnEraseBackground)
END_EVENT_TABLE()

TextEditorWX::TextEditorWX(wxWindow *parent, ProjectItem *p)
  : wxWindow(parent, wxWindowID(ID_EDIT_WND), wxDefaultPosition, wxDefaultSize)
{
	pi = p;
	edwnd = new wxTextCtrl(this, wxWindowID(ID_EDIT_CTL), "", wxPoint(0,0), wxDefaultSize,
		wxVSCROLL|wxHSCROLL|wxBORDER_SIMPLE|wxTE_MULTILINE|wxTE_RICH2|wxTE_NOHIDESEL|wxTE_PROCESS_TAB);
	wxFont font(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, prjOptions.editFontFace);
	edwnd->SetFont(font);
}

TextEditorWX::~TextEditorWX(void)
{
}

void TextEditorWX::OnSize(wxSizeEvent& evt)
{
	edwnd->SetSize(GetClientSize());
}

void TextEditorWX::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	// do nothing - prevents flicker
}


void TextEditorWX::OnEraseBackground(wxEraseEvent& evt)
{
	// do nothing - prevents flicker
}

/*
void TextEditorWX::TextChanged()
{
	changed = 1;
}

void TextEditorWX::UpdateUI()
{
	prjFrame->EditStateChanged();
}
*/

ProjectItem *TextEditorWX::GetItem()
{
	return pi;
}

void TextEditorWX::SetItem(ProjectItem *p)
{
	pi = p;
	pi->SetEditor(this);
}

void TextEditorWX::Undo()
{
	edwnd->Undo();
}

void TextEditorWX::Redo()
{
	edwnd->Redo();
}

void TextEditorWX::Cut()
{
	edwnd->Cut();
}

void TextEditorWX::Copy()
{
	edwnd->Copy();
}

void TextEditorWX::Paste()
{
	edwnd->Paste();
}

void TextEditorWX::Find()
{
	if (!findDlg)
		findDlg = new FindReplDlg(wxTheApp->GetTopWindow());
	findDlg->SetEditor(this);
	findDlg->Show();
}

void TextEditorWX::FindNext()
{
	if (findText.Length() == 0)
		Find();
	else
		Find(findFlags, findText);
}

void TextEditorWX::SelectAll()
{
	edwnd->SetSelection(-1,-1);
}

void TextEditorWX::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[40];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number:", value, 40))
			return;
		ln = atoi(value);
	}
	long pos = edwnd->XYToPosition(0, ln);
	if (pos < 0)
		pos = edwnd->GetLastPosition();
	edwnd->ShowPosition(pos);
	edwnd->SetInsertionPoint(pos);
}

void TextEditorWX::GotoPosition(int pos)
{
	edwnd->ShowPosition(pos);
	edwnd->SetInsertionPoint(pos);
}

void TextEditorWX::Cancel()
{
	edwnd->Undo();
}

void TextEditorWX::SetMarker()
{
//	SetMarkerAt(edwnd->CurrentLine(), -1);
}

void TextEditorWX::SetMarkerAt(int line, int on)
{
//	if (on == -1 && edwnd->MarkerGet(line))
//		edwnd->MarkerDel(line);
//	else if (on)
//		edwnd->MarkerAdd(line);
//	else
//		edwnd->MarkerDel(line);
}

void TextEditorWX::NextMarker()
{
//	int line = edwnd->CurrentLine();
//	if (edwnd->MarkerGet(line))
//		line++;
//	int found = edwnd->MarkerNext(line);
//	if (found == -1 && line)
//		found = edwnd->MarkerNext(0);
//	if (found >= 0)
//		edwnd->WndProc(SCI_GOTOLINE, found, 0);
}

void TextEditorWX::PrevMarker()
{
//	int line = edwnd->CurrentLine();
//	if (edwnd->MarkerGet(line))
//		line--;
//	line = edwnd->MarkerPrev(line);
//	if (line >= 0)
//		edwnd->WndProc(SCI_GOTOLINE, line, 0);
}

void TextEditorWX::ClearMarkers()
{
//	edwnd->MarkerDel(-1);
}

long TextEditorWX::EditState()
{
	long flags = VW_ENABLE_FILE | VW_ENABLE_GOTO | VW_ENABLE_SELALL | VW_ENABLE_FIND | VW_ENABLE_MARK;
	if (edwnd->CanPaste())
		flags |= VW_ENABLE_PASTE;
	if (edwnd->CanRedo())
		flags |= VW_ENABLE_REDO;
	if (edwnd->CanUndo())
		flags |= VW_ENABLE_UNDO;
	if (edwnd->CanCopy())
		flags |= VW_ENABLE_COPY|VW_ENABLE_CUT;
	return flags;
}

int TextEditorWX::IsChanged()
{
	return edwnd->IsModified();
}

void TextEditorWX::Focus()
{
	SetFocus();
}

int TextEditorWX::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;
	edwnd->Clear();
	if (::SynthFileExists(file))
	{
		if (!edwnd->LoadFile(wxString((const char *)file)))
			return -1;
	}
	edwnd->DiscardEdits();
	return 0;
}

int TextEditorWX::SaveFile(const char *fname)
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

	if (!edwnd->SaveFile(wxString((const char *)file)))
		return -1;
	edwnd->SetModified(false);
	return 0;
}

int TextEditorWX::GetText(bsString& text)
{
	text = edwnd->GetValue().c_str();
	return text.Length();
}

int TextEditorWX::SetText(bsString& text)
{
	edwnd->ChangeValue((const char*)text);
	return 0;
}

bool TextEditorWX::InitFind(int flags, const char *text, bool insel)
{
	findFlags = flags;
	findText = text;
	edwnd->GetSelection(&findStart, &findEnd);
	if (!insel)
	{
		findStart = findEnd + 1;
		findEnd = edwnd->GetLastPosition();
	}
	if (flags & TXTFIND_REGEXP)
	{
		int reflags = wxRE_DEFAULT|wxRE_NEWLINE;
		if (!(flags & TXTFIND_MATCHCASE))
			reflags |= wxRE_ICASE;
		if (!findRE.Compile(findText, reflags))
			return false;
	}
	return true;
}

static bool isword(wxChar ch)
{
	return ch == ' '
		|| ch == '\t'
		|| ch == '\r'
		|| ch == '\n'
		|| ch == '.'
		|| ch == ','
		|| ch == ';'
		|| ch == '"'
		|| ch == '\'';
}

bool TextEditorWX::DoFind()
{
	if (findFlags & TXTFIND_REGEXP)
	{
		if (findRE.Matches(matchText))
		{
			size_t restart = 0;
			size_t relen = 0;
			findRE.GetMatch(&restart, &relen);
			matchStart = findStart + restart;
			matchEnd = matchStart + relen;
			return true;
		}
	}
	else
	{
		int index = matchText.Find(findText);
		if (index != wxNOT_FOUND)
		{
			matchStart = findStart + index;
			matchEnd = matchStart + findText.Len();
			if (findFlags & (TXTFIND_WHOLEWORD|TXTFIND_WORDSTART))
			{
				long st = matchStart;
				if (st > 0)
					st--;
				wxString wordTxt(edwnd->GetRange(st, matchEnd+1));
				wxChar ch = wordTxt.GetChar(0);
				if (matchStart != 0 && !isword(ch))
					return false;
				if (findFlags & TXTFIND_WHOLEWORD)
				{
					int n1 = 1;
					int n2 = wordTxt.Len()-1;
					while (n1 < n2)
					{
						ch = wordTxt.GetChar(n1++);
						if (isword(ch))
							return false;
					}
					ch = wordTxt.GetChar(n2);
					if (!isword(ch))
						return false;
				}
			}
			return true;
		}
	}
	return false;
}

int TextEditorWX::Find(int flags, const char *text)
{
	if (!InitFind(flags, text, false))
		return -1;

	int wrap = 0;
	while (1)
	{
		matchText = edwnd->GetRange(findStart, findEnd);
		if (DoFind())
		{
			edwnd->SetSelection(matchStart, matchEnd);
			return wrap;
		}
		if (wrap)
			return -1;
		findEnd = findStart-1;
		findStart = 0;
		wrap = 1;
	}
	return -1;
}

int TextEditorWX::MatchSel(int flags, const char *text)
{
	if (!InitFind(flags, text, true))
		return -1;
	matchText = edwnd->GetStringSelection();
	return DoFind();
}

void TextEditorWX::Replace(const char *rtext)
{
	long from = 0;
	long to = 0;
	edwnd->GetSelection(&from, &to);
	if (findFlags & TXTFIND_REGEXP)
	{
		wxString txt(edwnd->GetStringSelection());
		findRE.ReplaceFirst(&txt, wxString(rtext));
		edwnd->Replace(from, to+1, txt);
	}
	else
		edwnd->Replace(from, to+1, rtext);
}

int TextEditorWX::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	if (!InitFind(flags, ftext, false))
		return -1;

	int count = 0;
	findStart = info.startPos;
	findEnd = info.endPos;
	if (findStart == findEnd)
		findEnd = edwnd->GetLastPosition();

	wxString repl(rtext);
	if (findFlags & TXTFIND_REGEXP)
	{
		matchText = edwnd->GetRange(findStart, findEnd);
		count = findRE.ReplaceAll(&matchText, wxString(rtext));
		if (count > 0)
			edwnd->Replace(findStart, findEnd, matchText);
	}
	else
	{
		long rlen = repl.Len() + 1;
		do
		{
			matchText = edwnd->GetRange(findStart, findEnd);
			if (!DoFind())
				break;
			count++;
			edwnd->Replace(matchStart, matchEnd, repl);
			findStart = matchStart + rlen;
		} while (findStart < findEnd);
	}
	return count;
}


int TextEditorWX::GetSelection(SelectInfo& info)
{
	long from = 0;
	long to = 0;
	edwnd->GetSelection(&from, &to);
	info.startPos = from;
	info.endPos = to;
	long ch = 0;
	long ln = 0;
	edwnd->PositionToXY(from, &ch, &ln);
	info.startLn = ln;
	info.startCh = ch;
	edwnd->PositionToXY(to, &ch, &ln);
	info.endLn = ln;
	info.endCh = ch;
	return 0;
}

void TextEditorWX::SetSelection(SelectInfo& info)
{
	edwnd->SetSelection(info.startPos, info.endPos);
}
