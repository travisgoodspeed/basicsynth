//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "FindReplDlg.h"

BEGIN_EVENT_TABLE(FindReplDlg, wxDialog)
	EVT_ACTIVATE(FindReplDlg::OnActivate)
	EVT_TEXT(XRCID("IDC_FIND_TEXT"), FindReplDlg::OnTextChanged)
	EVT_TEXT(XRCID("IDC_REPL_TEXT"), FindReplDlg::OnTextChanged)
	EVT_BUTTON(XRCID("IDC_FIND_NEXT"), FindReplDlg::OnFindNext)
	EVT_BUTTON(XRCID("IDC_REPLACE"), FindReplDlg::OnReplace)
	EVT_BUTTON(XRCID("IDC_REPLACE_ALL"), FindReplDlg::OnReplaceAll)
	EVT_BUTTON(XRCID("IDC_REPLACE_SELALL"), FindReplDlg::OnReplaceSel)
	EVT_BUTTON(wxID_OK, FindReplDlg::OnCloseCmd)
	EVT_BUTTON(wxID_CANCEL, FindReplDlg::OnCloseCmd)
END_EVENT_TABLE()

FindReplDlg::FindReplDlg(wxWindow *parent)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_FINDREPL");
	CenterOnParent();

	findText = (wxComboBox*)FindWindow("IDC_FIND_TEXT");
	replText = (wxComboBox*)FindWindow("IDC_REPL_TEXT");
	findBtn = (wxButton*)FindWindow("IDC_FIND_NEXT");
	replBtn = (wxButton*)FindWindow("IDC_REPLACE");
	replAllBtn = (wxButton*)FindWindow("IDC_REPLACE_ALL");
	replSelBtn = (wxButton*)FindWindow("IDC_REPLACE_SELALL");
	matchBtn = (wxCheckBox*)FindWindow("IDC_MATCH_CASE");
	fullWordBtn = (wxCheckBox*)FindWindow("IDC_WHOLE_WORD");
	startWordBtn = (wxCheckBox*)FindWindow("IDC_START_WORD");
	regexpBtn = (wxCheckBox*)FindWindow("IDC_REGEXP");
	statText = FindWindow("IDC_STAT_TEXT");
	statText->SetLabel("");
}

FindReplDlg::~FindReplDlg(void)
{
}


void FindReplDlg::OnActivate(wxActivateEvent& evt)
{
	SetEnable();
	findText->SetFocus();
}

void FindReplDlg::OnFindNext(wxCommandEvent& evt)
{
	if (!ed)
		return;

	bsString ftext;
	GetFindText(findText, ftext);
	int flags = GetFlags();
	int res = ed->Find(flags, ftext);
	const char *msg = "";
	if (res == -1)
		msg = "Search text was not found";
	else if (res == 1)
		msg = "Passed end of file.";
	statText->SetLabel(msg);
}

void FindReplDlg::OnReplace(wxCommandEvent& evt)
{
	if (!ed)
		return;

	bsString ftext;
	bsString rtext;
	GetFindText(findText, ftext);
	GetFindText(replText, rtext);
	int flags = GetFlags();
	const char *msg = "Search text was not found";
	if (ed->MatchSel(flags, ftext))
		ed->Replace(rtext);
	if (ed->Find(flags, ftext) != -1)
		msg = "";
	statText->SetLabel(msg);
}

void FindReplDlg::OnReplaceAll(wxCommandEvent& evt)
{
	DoReplace(0);
}

void FindReplDlg::OnReplaceSel(wxCommandEvent& evt)
{
	DoReplace(1);
}

void FindReplDlg::OnCloseCmd(wxCommandEvent& evt)
{
	if (ed)
		ed->Focus();
	Show(false);
}

void FindReplDlg::OnTextChanged(wxCommandEvent& evt)
{
	SetEnable();
}

void FindReplDlg::DoReplace(int inSel)
{
	if (!ed)
		return;

	bsString ftext;
	bsString rtext;
	GetFindText(findText, ftext);
	GetFindText(replText, rtext);

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
	statText->SetLabel(msg);
}

int FindReplDlg::GetFlags()
{
	int flags = 0;
	if (matchBtn->GetValue())
		flags |= TXTFIND_MATCHCASE;
	if (fullWordBtn->GetValue())
		flags |= TXTFIND_WHOLEWORD;
	if (startWordBtn->GetValue())
		flags |= TXTFIND_WORDSTART;
	if (regexpBtn->GetValue())
		flags |= TXTFIND_REGEXP; // SCFIND_POSIX
	return flags;
}

void FindReplDlg::SetEnable()
{
	wxString txt = findText->GetValue();
	bool enableFind = txt.Length() > 0;
	findBtn->Enable(enableFind);
	replAllBtn->Enable(enableFind);
	replSelBtn->Enable(enableFind);
	replBtn->Enable(ed != NULL);
}

int FindReplDlg::GetFindText(wxComboBox* wnd, bsString& text)
{
	wxString cbtxt = wnd->GetValue();
	int sel = wnd->FindString(cbtxt);
	if (sel == -1)
	{
		sel = wnd->Append(cbtxt);
		wnd->SetSelection(sel);
		int cnt;
		while ((cnt = wnd->GetCount()) >= 20)
			wnd->Delete(cnt-1);
	}
	text = cbtxt;
	return sel;
}
