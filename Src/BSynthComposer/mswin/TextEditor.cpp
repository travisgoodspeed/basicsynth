//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "TextEditor.h"
#include "FindReplDlg.h"

static FindReplDlg findDlg;
// load SciLexer to use other built-in lexers.
#ifdef SCI_LEXER
#if _DEBUG
static char dllname[] = "SciLexerD.dll";
#else
static char dllname[] = "SciLexer.dll";
#endif
#else
#if _DEBUG
static char dllname[] = "ScintillaD.dll";
#else
static char dllname[] = "Scintilla.dll";
#endif
#endif

void LoadEditorDLL()
{
	HMODULE h = LoadLibrary(dllname);
	if (h == 0)
	{
		MessageBox(HWND_DESKTOP, "The Editor dll (Scintilla) was not loaded. Must abort...", "Sorry...", MB_OK);
		exit(0);
	}
}

TextEditorWin::TextEditorWin()
{
	findFlags = 0;
	pi = 0;
	bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
}

TextEditorWin::~TextEditorWin()
{
	if ((TextEditorWin*)findDlg.GetEditor() == this)
		findDlg.SetEditor(0);
}

void TextEditorWin::TextChanged()
{
	changed = 1;
}

void TextEditorWin::UpdateUI()
{
	prjFrame->EditStateChanged();
}

ProjectItem *TextEditorWin::GetItem()
{
	return pi;
}

void TextEditorWin::SetItem(ProjectItem *p)
{
	pi = p;
	pi->SetEditor(this);
}

void TextEditorWin::Undo()
{
	edwnd.Undo();
}

void TextEditorWin::Redo()
{
	edwnd.Redo();
}

void TextEditorWin::Cut()
{
	edwnd.Cut();
}

void TextEditorWin::Copy()
{
	edwnd.Copy();
}

void TextEditorWin::Paste()
{
	edwnd.Paste();
}

void TextEditorWin::Find()
{
	if (!findDlg.IsWindow())
		findDlg.Create(_Module.mainWnd);
	findDlg.SetEditor(this);
	findDlg.ShowWindow(SW_SHOWNORMAL);
}

void TextEditorWin::FindNext()
{
	if (findText.Length() == 0)
		Find();
	else
		Find(findFlags, findText);
}

void TextEditorWin::SelectAll()
{
	edwnd.SetSelAll();
}

void TextEditorWin::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[40];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number:", value, 40))
			return;
		ln = atoi(value);
	}
	edwnd.GotoLine(ln);
}

void TextEditorWin::GotoPosition(int pos)
{
	edwnd.GotoPosition(pos);
}

void TextEditorWin::Cancel()
{
	edwnd.Undo();
}

void TextEditorWin::SetMarker()
{
	SetMarkerAt(edwnd.CurrentLine(), -1);
}

void TextEditorWin::SetMarkerAt(int line, int on)
{
	if (on == -1 && edwnd.MarkerGet(line))
		edwnd.MarkerDel(line);
	else if (on)
		edwnd.MarkerAdd(line);
	else
		edwnd.MarkerDel(line);
}

void TextEditorWin::NextMarker()
{
	int line = edwnd.CurrentLine();
	if (edwnd.MarkerGet(line))
		line++;
	int found = edwnd.MarkerNext(line);
	if (found == -1 && line)
		found = edwnd.MarkerNext(0);
	if (found >= 0)
		edwnd.GotoLine(found);
}

void TextEditorWin::PrevMarker()
{
	int line = edwnd.CurrentLine();
	if (edwnd.MarkerGet(line))
		line--;
	line = edwnd.MarkerPrev(line);
	if (line >= 0)
		edwnd.GotoLine(line);
}

void TextEditorWin::ClearMarkers()
{
	edwnd.MarkerDel(-1);
}

long TextEditorWin::EditState()
{
	long flags = VW_ENABLE_FILE | VW_ENABLE_GOTO | VW_ENABLE_SELALL 
	           | VW_ENABLE_FIND | VW_ENABLE_MARK | VW_ENABLE_UNMARK;
	if (edwnd.CanPaste())
		flags |= VW_ENABLE_PASTE;
	if (edwnd.CanRedo())
		flags |= VW_ENABLE_REDO;
	if (edwnd.CanUndo())
		flags |= VW_ENABLE_UNDO;
	long start = 0;
	long end = 0;
	edwnd.GetSelection(start, end);
	if (start != end)
		flags |= VW_ENABLE_COPY|VW_ENABLE_CUT;
	return flags;
}

int TextEditorWin::IsChanged()
{
	return edwnd.IsChanged();
}

void TextEditorWin::Focus()
{
	if (edwnd.IsWindow())
		edwnd.SetFocus();
}

#define KWMAXLEN 10
// isalpha(c) throws debug exceptions on UNICODE chars
#define AtoZ(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

void TextEditorWin::Restyle(int position)
{
	int endStyled = edwnd.GetEndStyled();
	int lineNumber = edwnd.LineFromPosition(endStyled);
	int startPos = edwnd.PositionFromLine(lineNumber);

	//ATLTRACE("Restyle %d: endStyled=%d lineNumber=%d startPos=%d\n", position, endStyled, lineNumber, startPos);

	int chPos = startPos;
	int ch = edwnd.GetCharAt(chPos);
	while (chPos < position)
	{
		if (strchr(",;{}[]():=", ch) != 0)
		{
			if (startPos != chPos)
			{
				edwnd.StartStyling(startPos);
				edwnd.SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			edwnd.StartStyling(chPos);
			int lenPos = 0;
			do
			{
				lenPos++;
				ch = edwnd.GetCharAt(++chPos);
			} while (ch > 0 && strchr(",;{}[]():=", ch) != 0 && chPos < position);
			edwnd.SetStyling(lenPos, NLSTYLE_DELIM);
			startPos = chPos;
		}
		else if (ch == '!' || ch == '\'')
		{
			if (startPos != chPos)
			{
				edwnd.StartStyling(startPos);
				edwnd.SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			int eol = edwnd.GetLineEndPosition(edwnd.LineFromPosition(chPos));
			edwnd.StartStyling(chPos);
			edwnd.SetStyling(eol - chPos, NLSTYLE_COMMENT);
			chPos = eol+1;
			ch = edwnd.GetCharAt(chPos);
			startPos = chPos;
		}
		else if (ch == '"')
		{
			if (startPos != chPos)
			{
				edwnd.StartStyling(startPos);
				edwnd.SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
			}
			startPos = chPos;
			do
			{
				ch = edwnd.GetCharAt(++chPos);
			} while (ch > 0 && ch != '"' && ch != '\n' && chPos < position);
			if (ch == '"')
				ch = edwnd.GetCharAt(++chPos);
			edwnd.StartStyling(startPos);
			edwnd.SetStyling(chPos - startPos, NLSTYLE_QUOTE);
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
				ch = edwnd.GetCharAt(++chPos);
			} while (ch > 0 && AtoZ(ch) && kwpos <= KWMAXLEN && chPos < position);
			kwbuf[kwpos] = '\0';
			if (nlLex::IsKeyword(kwbuf) != T_ENDOF)
			{
				if (startPos != kwStart)
				{
					edwnd.StartStyling(startPos);
					edwnd.SetStyling(kwStart - startPos, NLSTYLE_DEFAULT);
				}
				edwnd.StartStyling(kwStart);
				edwnd.SetStyling(chPos - kwStart, NLSTYLE_KEYWORD);
				startPos = chPos;
			}
		}
		else
			ch = edwnd.GetCharAt(++chPos);
	}
	if (startPos != chPos)
	{
		edwnd.StartStyling(startPos);
		edwnd.SetStyling(chPos - startPos, NLSTYLE_DEFAULT);
	}
}

int TextEditorWin::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;

	HANDLE fh;
	fh = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh != INVALID_HANDLE_VALUE)
	{
		// get file size
		DWORD sizHigh = 0;
		DWORD siz = GetFileSize(fh, &sizHigh);
		if (sizHigh)
		{
			// bigger than 4 gigabytes? No way... :)
			CloseHandle(fh);
			return -2;
		}

		if (siz > 0)
		{
			char *text = new char[siz+1];
			if (text != NULL)
			{
				DWORD nread = 0;
				ReadFile(fh, (LPVOID)text, siz, &nread, NULL);
				text[siz] = 0;
				bsString tmp;
				tmp.Attach(text);
				SetText(tmp);
			}
		}
		CloseHandle(fh);
	}

	edwnd.SetMargins();
	edwnd.EmptyUndoBuffer();
	edwnd.SetSavePoint();
	return 0;
}

int TextEditorWin::SaveFile(const char *fname)
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

	int ret = 0;
	bsString text;
	int siz = GetText(text);

	HANDLE fh = CreateFile(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh != INVALID_HANDLE_VALUE)
	{
		if (siz > 0)
		{
			DWORD nwrit = 0;
			WriteFile(fh, (LPVOID) (const char *)text, (DWORD)siz, &nwrit, NULL);
			if ((DWORD)siz != nwrit)
				ret = -2;
		}
		CloseHandle(fh);
	}
	else
		ret = -1;

	edwnd.SetSavePoint();

	return ret;
}

int TextEditorWin::GetText(bsString& text)
{
	char *t = NULL;
	int siz = edwnd.Length();
	t = new char[siz+1];
	if (t != NULL)
	{
		if (siz > 0)
			edwnd.GetText(t, siz+1);
		else
			*t = 0;
	}
	text.Attach(t);
	return siz;
}

int TextEditorWin::SetText(bsString& text)
{
	edwnd.SetText(text);
	return 0;
}

int TextEditorWin::Find(int flags, const char *text)
{
	findFlags = flags;
	findText = text;
	edwnd.SetSearchFlags(flags);
	int wrap = 0;
	int start = edwnd.CurrentPos();
	int found = edwnd.Find(text, start);
	if (found == -1 && start > 0)
	{
		found = edwnd.Find(text, 0, start);
		wrap = 1;
	}
	if (found >= 0)
		return wrap;
	return -1;
}

int TextEditorWin::MatchSel(int flags, const char *text)
{
	edwnd.SetSearchFlags(flags);
	return edwnd.Match(text);
}

void TextEditorWin::Replace(const char *rtext)
{
	edwnd.ReplaceSel(rtext);
}

int TextEditorWin::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	findFlags = flags;
	findText = ftext;
	int count = 0;
	int flen = (int)strlen(ftext);
	int rlen = (int)strlen(rtext);
	edwnd.SetSearchFlags(flags);
	edwnd.BeginUndoAction();
	SelectInfo si;
	si.startPos = info.startPos;
	si.endPos = info.endPos;
	if (si.startPos == si.endPos)
	{
		si.endPos = edwnd.Length();
		si.endLn = edwnd.LineFromPosition(si.endPos)+1;
		si.endCh = 0;
	}
	else
	{
		si.endLn = info.endLn;
		si.endCh = info.endCh;
	}

	do
	{
		edwnd.SetTarget(si.startPos, si.endPos);
		if (edwnd.SearchInTarget(flen, ftext) == -1)
			break;
		count++;
		int msg = (flags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
		edwnd.CallScintilla(msg, rlen, (LPARAM) rtext);
		si.startPos = edwnd.GetTargetEnd();
		si.endPos = edwnd.PositionFromLine(si.endLn) + si.endCh;
	} while (si.startPos <= si.endPos);
	edwnd.EndUndoAction();
	return count;
}


int TextEditorWin::GetSelection(SelectInfo& info)
{
	return edwnd.GetSelection(info);
}

void TextEditorWin::SetSelection(SelectInfo& info)
{
	edwnd.SetSelection(info);
}

/////////////////////////////////////////////////////////////////////////////////////

LRESULT TextEditorWin::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	edwnd.m_hWnd = CreateWindowEx(0,
		"Scintilla","", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, rc.right - rc.left, rc.bottom - rc.top, m_hWnd, (HMENU)ID_EDIT_WND, _Module.GetModuleInstance(), NULL);
	if (edwnd.IsWindow())
	{
		edwnd.Init();
		if (pi)
			edwnd.SetupStyling(pi->GetType() == PRJNODE_NOTEFILE);
	}
	return 0;
}

LRESULT TextEditorWin::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	edwnd.SetFocus();
	findDlg.SetEditor(this);
	return 0;
}

LRESULT TextEditorWin::OnShow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (findDlg.GetEditor() == this)
		findDlg.SetEditor(wParam ? this : 0);
	return 0;
}

LRESULT TextEditorWin::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	edwnd.SetWindowPos(HWND_TOP, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
	return 0;
}

LRESULT TextEditorWin::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
//	RECT rcWnd;
//	GetClientRect(&rcWnd);
//	Graphics gr(dc);
//	SolidBrush br(bgColor);
//	gr.FillRectangle(&br, 0, 0, rcWnd.right+1, rcWnd.bottom+1);
	EndPaint(&ps);
	return 0;
}

LRESULT TextEditorWin::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT TextEditorWin::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TextChanged();
	return 0;
}

LRESULT TextEditorWin::OnEditUpdateUI(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	UpdateUI();
	return 0;
}

LRESULT TextEditorWin::OnEditRestyle(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	Scintilla::SCNotification *scn = (Scintilla::SCNotification *) pnmh;
	Restyle(scn->position);
	return 0;
}
