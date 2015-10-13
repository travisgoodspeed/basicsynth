//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "ScoreErrorsDlg.h"

ScoreErrorsDlg::ScoreErrorsDlg()
{
}

ScoreErrorsDlg::~ScoreErrorsDlg()
{
}

LRESULT ScoreErrorsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	errLst = GetDlgItem(IDC_ERROR_LIST);
	itmSel = GetDlgItem(IDC_ERROR_ITEMS);
	nextBtn = GetDlgItem(IDC_ERROR_NEXT);
	prevBtn = GetDlgItem(IDC_ERROR_PREV);
	gotoBtn = GetDlgItem(IDC_ERROR_GOTO);

	Refresh();

	return 1;
}

LRESULT ScoreErrorsDlg::OnGoto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GotoLine();
	return 0;
}

LRESULT ScoreErrorsDlg::OnPrev(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int index = errLst.GetCurSel();
	if (index > 0)
	{
		errLst.SetCurSel(--index);
		ErrSelect(index, errLst.GetCount());
	}
	GotoLine();
	return 0;
}

LRESULT ScoreErrorsDlg::OnNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int index = errLst.GetCurSel();
	int count = errLst.GetCount();
	if (index < count)
	{
		errLst.SetCurSel(++index);
		ErrSelect(index, count);
	}
	GotoLine();
	return 0;
}

LRESULT ScoreErrorsDlg::OnCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CheckSyntax();
	return 0;
}

LRESULT ScoreErrorsDlg::OnMark(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MarkErrors();
	return 0;
}

LRESULT ScoreErrorsDlg::OnRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Refresh();
	return 0;
}

LRESULT ScoreErrorsDlg::OnItemSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShowErrors();
	return 0;
}

LRESULT ScoreErrorsDlg::OnErrGoto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GotoLine();
	return 0;
}

LRESULT ScoreErrorsDlg::OnErrSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ErrSelect(errLst.GetCurSel(), errLst.GetCount());
	return 0;
}

LRESULT ScoreErrorsDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::PostMessage(GetParent(), WM_COMMAND, ID_ITEM_ERRORS, 0);
	return 0;
}

void ScoreErrorsDlg::RemoveItem(ProjectItem *itm)
{
	int show = 0;
	int count = itmSel.GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		if ((ProjectItem*)itmSel.GetItemDataPtr(index) == itm)
		{
			show = (index == itmSel.GetCurSel());
			itmSel.DeleteString(index);
			break;
		}
	}
	if (show)
		ShowErrors();
}

void ScoreErrorsDlg::Clear()
{
	// remove everything
	errLst.ResetContent();
	itmSel.ResetContent();
	ErrSelect(-1, 0);
}

void ScoreErrorsDlg::Refresh()
{
	itmSel.ResetContent();

	ProjectItem *pi = prjTree->FirstChild(theProject->nlInfo);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_NOTEFILE)
		{
			int ndx = itmSel.AddString(pi->GetName());
			itmSel.SetItemDataPtr(ndx, (void*)pi);
		}
		pi = prjTree->NextSibling(pi);
	}
	itmSel.SetCurSel(0);
	ShowErrors();
}

void ScoreErrorsDlg::CheckSyntax()
{
	int index = itmSel.GetCurSel();
	if (index >= 0)
	{
		NotelistItem *ni = (NotelistItem*)itmSel.GetItemDataPtr(index);
		ni->SyntaxCheck();
		ShowErrors();
	}
}

void ScoreErrorsDlg::ShowErrors()
{
	errLst.ResetContent();
	int count = 0;
	int index = itmSel.GetCurSel();
	if (index >= 0)
	{
		//char msg[256];
		NotelistItem *ni = (NotelistItem*)itmSel.GetItemDataPtr(index);
		ScoreError *err = ni->EnumErrors(0);
		while (err)
		{
			bsString fmt;
			fmt = err->GetLine();
			fmt += ": ";
			fmt += err->GetMsg();
			fmt += " : ";
			fmt += err->GetToken();
			//snprintf(msg, 256, "%d: %s : %s", err->GetLine(), err->GetMsg(), err->GetToken());
			index = errLst.AddString(fmt);
			errLst.SetItemData(index, err->GetPosition());
			err = ni->EnumErrors(err);
			count++;
		}
		errLst.SetCurSel(0);
	}
	else
		errLst.SetCurSel(-1);
	ErrSelect(0, count);
}

void ScoreErrorsDlg::GotoLine()
{
	int index = itmSel.GetCurSel();
	if (index >= 0)
	{
		NotelistItem *itm = (NotelistItem *)itmSel.GetItemDataPtr(index);
		index = errLst.GetCurSel();
		if (index < 0)
			index = 0;
		int errPos = errLst.GetItemData(index);
		if (errPos > 0) // the editor places the cursor after the char at this position
			errPos--;
		prjFrame->OpenEditor(itm);
		EditorView *ed = itm->GetEditor();
		ed->GotoPosition(errPos);
	}
}

void ScoreErrorsDlg::MarkErrors()
{
	int count = 0;
	int index = itmSel.GetCurSel();
	if (index >= 0)
	{
		NotelistItem *itm = (NotelistItem *)itmSel.GetItemDataPtr(index);

		prjFrame->OpenEditor(itm);
		EditorView *ed = itm->GetEditor();
		ed->ClearMarkers();

		ScoreError *err = itm->EnumErrors(0);
		while (err)
		{
			ed->SetMarkerAt(err->GetLine()-1, 1);
			err = itm->EnumErrors(err);
		}
	}
}

void ScoreErrorsDlg::ErrSelect(int index, int count)
{
	nextBtn.EnableWindow(index >= 0 && index < (count-1));
	prevBtn.EnableWindow(index > 0 && count > 0);
	gotoBtn.EnableWindow(count > 0);
}
