//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "FindReplDlg.h"

FindReplDlg::FindReplDlg()
{
	ed = NULL;
}

LRESULT FindReplDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	findText = GetDlgItem(IDC_FIND_TEXT);
	replText = GetDlgItem(IDC_REPL_TEXT);
	findBtn = GetDlgItem(IDC_FIND_NEXT);
	replBtn = GetDlgItem(IDC_REPLACE);
	replAllBtn = GetDlgItem(IDC_REPLACE_ALL);
	replSelBtn = GetDlgItem(IDC_REPLACE_SELALL);
	matchBtn = GetDlgItem(IDC_MATCH_CASE);
	fullWordBtn = GetDlgItem(IDC_WHOLE_WORD);
	startWordBtn = GetDlgItem(IDC_START_WORD);
	regexpBtn = GetDlgItem(IDC_REGEXP);
	statText = GetDlgItem(IDC_STAT_TEXT);
	statText.SetWindowText("");
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	return TRUE;
}

LRESULT FindReplDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
//	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT FindReplDlg::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetEnable();
	findText.SetFocus();
	return 0;
}

LRESULT FindReplDlg::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetEnable();
	return 0;
}

LRESULT FindReplDlg::OnFindNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!ed)
		return 0;

	bsString ftext;
	int sel = GetFindText(findText, ftext);
	int flags = GetFlags();
	int res = ed->Find(flags, ftext);
	char *msg = "";
	if (res == -1)
		msg = "Search text was not found";
	else if (res == 1)
		msg = "Passed end of file.";
	statText.SetWindowText(msg);
	return 0;
}

LRESULT FindReplDlg::OnReplace(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!ed)
		return 0;

	bsString ftext;
	bsString rtext;
	int fsel = GetFindText(findText, ftext);
	int rsel = GetFindText(replText, rtext);
	int flags = GetFlags();
	char *msg = "Search text was not found";
	if (ed->MatchSel(flags, ftext))
		ed->Replace(rtext);
	if (ed->Find(flags, ftext) != -1)
		msg = "";
	statText.SetWindowText(msg);

	return 0;
}

LRESULT FindReplDlg::OnReplaceAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DoReplace(0);
	return 0;
}

LRESULT FindReplDlg::OnReplaceSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DoReplace(1);
	return 0;
}

void FindReplDlg::DoReplace(int inSel)
{
	if (!ed)
		return;

	bsString ftext;
	bsString rtext;
	int fsel = GetFindText(findText, ftext);
	int rsel = GetFindText(replText, rtext);

	int flags = GetFlags();
	SelectInfo si;
	if (inSel)
		ed->GetSelection(si);
	else
		si.startPos = si.endPos = 0;
	int count = ed->ReplaceAll(flags, ftext, rtext, si);
	if (inSel)
		ed->SetSelection(si);
	char msg[256];
	snprintf(msg, 256, "%d replacement%s", count, count == 1 ? "." : "s.");
	statText.SetWindowText(msg);
}

LRESULT FindReplDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (ed)
		ed->Focus();
	ShowWindow(SW_HIDE);
	return 0;
}

int FindReplDlg::GetFlags()
{
	int flags = 0;
	if (matchBtn.GetCheck())
		flags |= TXTFIND_MATCHCASE;
	if (fullWordBtn.GetCheck())
		flags |= TXTFIND_WHOLEWORD;
	if (startWordBtn.GetCheck())
		flags |= TXTFIND_WORDSTART;
	if (regexpBtn.GetCheck())
		flags |= TXTFIND_REGEXP; // SCFIND_POSIX
	return flags;
}

void FindReplDlg::SetEnable()
{
	int enableFind = (ed != NULL) && (findText.GetWindowTextLength() > 0);
	findBtn.EnableWindow(enableFind);
	replAllBtn.EnableWindow(enableFind);
	replSelBtn.EnableWindow(enableFind);
	replBtn.EnableWindow(ed != NULL);
}

int FindReplDlg::GetFindText(CComboBox& wnd, bsString& text)
{
	int len;
	char *tp;
	int sel = wnd.GetCurSel();
	if (sel == -1)
	{
		len = wnd.GetWindowTextLength()+1;
		tp = new char[len];
		wnd.GetWindowText(tp, len);
		sel = wnd.FindStringExact(-1, tp);
		if (sel == -1)
		{
			sel = wnd.InsertString(0, tp);
			wnd.SetCurSel(sel);
		}
		int cnt;
		while ((cnt = wnd.GetCount()) >= 20)
			wnd.DeleteString(cnt-1);
	}
	else
	{
		len = wnd.GetLBTextLen(sel)+1;
		tp = new char[len];
		wnd.GetLBText(sel, tp);
	}
	text.Attach(tp);
	return sel;
}
