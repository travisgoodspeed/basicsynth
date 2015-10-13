//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "ScoreErrorsDlg.h"

BEGIN_EVENT_TABLE(ScoreErrorsDlg, wxDialog)
	EVT_BUTTON(XRCID("IDC_ERROR_NEXT"), ScoreErrorsDlg::OnNext)
	EVT_BUTTON(XRCID("IDC_ERROR_PREV"), ScoreErrorsDlg::OnPrev)
	EVT_BUTTON(XRCID("IDC_ERROR_GOTO"), ScoreErrorsDlg::OnGoto)
	EVT_BUTTON(XRCID("IDC_ERROR_CHECK"), ScoreErrorsDlg::OnCheck)
	EVT_BUTTON(XRCID("IDC_ERROR_MARK"), ScoreErrorsDlg::OnMark)
	EVT_BUTTON(XRCID("IDC_ERROR_REFRESH"), ScoreErrorsDlg::OnRefresh)
	EVT_COMBOBOX(XRCID("IDC_ERROR_ITEMS"), ScoreErrorsDlg::OnItemSel)
	EVT_LISTBOX(XRCID("IDC_ERROR_LIST"), ScoreErrorsDlg::OnErrSel)
	EVT_LISTBOX_DCLICK(XRCID("IDC_ERROR_LIST"), ScoreErrorsDlg::OnErrGoto)
	EVT_BUTTON(wxID_CANCEL, ScoreErrorsDlg::OnCloseCmd)
END_EVENT_TABLE()


ScoreErrorsDlg::ScoreErrorsDlg(wxWindow *parent)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_SCORE_ERRORS");
	CenterOnParent();

	errLst = (wxListBox*)FindWindow("IDC_ERROR_LIST");
	itmSel = (wxComboBox*)FindWindow("IDC_ERROR_ITEMS");
	nextBtn = (wxButton*)FindWindow("IDC_ERROR_NEXT");
	prevBtn = (wxButton*)FindWindow("IDC_ERROR_PREV");
	gotoBtn = (wxButton*)FindWindow("IDC_ERROR_GOTO");

	Refresh();
}

ScoreErrorsDlg::~ScoreErrorsDlg(void)
{
}

void ScoreErrorsDlg::OnGoto(wxCommandEvent& evt)
{
	GotoLine();
}

void ScoreErrorsDlg::OnPrev(wxCommandEvent& evt)
{
	int index = errLst->GetSelection();
	if (index > 0)
	{
		errLst->SetSelection(--index);
		ErrSelect(index, errLst->GetCount());
	}
	GotoLine();
}

void ScoreErrorsDlg::OnNext(wxCommandEvent& evt)
{
	int index = errLst->GetSelection();
	int count = errLst->GetCount();
	if (++index < count)
	{
		errLst->SetSelection(index);
		ErrSelect(index, count);
	}
	GotoLine();
}

void ScoreErrorsDlg::OnMark(wxCommandEvent& evt)
{
	MarkErrors();
}

void ScoreErrorsDlg::OnCheck(wxCommandEvent& evt)
{
	CheckSyntax();
}

void ScoreErrorsDlg::OnRefresh(wxCommandEvent& evt)
{
	Refresh();
}

void ScoreErrorsDlg::OnItemSel(wxCommandEvent& evt)
{
	ShowErrors();
}

void ScoreErrorsDlg::OnErrGoto(wxCommandEvent& evt)
{
	GotoLine();
}

void ScoreErrorsDlg::OnErrSel(wxCommandEvent& evt)
{
	ErrSelect(errLst->GetSelection(), errLst->GetCount());
}

void ScoreErrorsDlg::OnCloseCmd(wxCommandEvent& evt)
{
	// post message to main window
	wxCommandEvent cmd(wxEVT_COMMAND_MENU_SELECTED, XRCID("ID_ITEM_ERRORS"));
	evt.SetEventObject(this);
	wxPostEvent(GetParent(), cmd);
}

void ScoreErrorsDlg::RemoveItem(ProjectItem *itm)
{
	int show = 0;
	int count = itmSel->GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		if ((ProjectItem*)itmSel->GetClientData(index) == itm)
		{
			show = (index == itmSel->GetSelection());
			itmSel->Delete(index);
			break;
		}
	}
	if (show)
		ShowErrors();
}

void ScoreErrorsDlg::Clear()
{
	// remove everything
	errLst->Clear();
	itmSel->Clear();
	ErrSelect(-1, 0);
}

void ScoreErrorsDlg::Refresh()
{
	itmSel->Clear();

	ProjectItem *pi = prjTree->FirstChild(theProject->nlInfo);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_NOTEFILE)
			itmSel->Append(pi->GetName(), (void*)pi);
		pi = prjTree->NextSibling(pi);
	}
	if (itmSel->GetCount() > 0)
		itmSel->SetSelection(0);
	ShowErrors();
}

void ScoreErrorsDlg::CheckSyntax()
{
	int index = itmSel->GetSelection();
	if (index != wxNOT_FOUND)
	{
		NotelistItem *ni = (NotelistItem*)itmSel->GetClientData(index);
		ni->SyntaxCheck();
		ShowErrors();
	}
}

void ScoreErrorsDlg::ShowErrors()
{
	errLst->Clear();
	int count = 0;
	int index = itmSel->GetSelection();
	if (index != wxNOT_FOUND)
	{
		//wxString msg;
		NotelistItem *ni = (NotelistItem*)itmSel->GetClientData(index);
		ScoreError *err = ni->EnumErrors(0);
		while (err)
		{
			// does not work:
			// msg.Format("%d: %s : %s", (int)err->GetLine(), err->GetMsg(), err->GetToken());
			bsString fmt;
			fmt = err->GetLine();
			fmt += ": ";
			fmt += err->GetMsg();
			fmt += " : ";
			fmt += err->GetToken();
			errLst->Append(wxString(fmt), reinterpret_cast<void*>(err->GetPosition()));
			err = ni->EnumErrors(err);
			count++;
		}
		if (count > 0)
			errLst->SetSelection(0);
	}
	else
		errLst->SetSelection(wxNOT_FOUND);
	ErrSelect(0, count);
}

void ScoreErrorsDlg::GotoLine()
{
	int index = itmSel->GetSelection();
	if (index != wxNOT_FOUND)
	{
		NotelistItem *itm = (NotelistItem *)itmSel->GetClientData(index);
		if (itm)
		{
			index = errLst->GetSelection();
			if (index < 0)
				index = 0;
			long errPos = reinterpret_cast<long>(errLst->GetClientData(index));
			if (errPos > 0) // the editor places the cursor after the char at this position
				errPos--;
			prjFrame->OpenEditor(itm);
			EditorView *ed = itm->GetEditor();
			if (ed)
				ed->GotoPosition((int)errPos);
		}
	}
}

void ScoreErrorsDlg::MarkErrors()
{
	int index = itmSel->GetSelection();
	if (index != wxNOT_FOUND)
	{
		NotelistItem *itm = (NotelistItem*)itmSel->GetClientData(index);

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
	nextBtn->Enable(index >= 0 && index < (count-1));
	prevBtn->Enable(index > 0 && count > 0);
	gotoBtn->Enable(count > 0);
}
