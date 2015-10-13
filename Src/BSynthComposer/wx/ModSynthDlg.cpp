//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include <ModSynthEd.h>
#include "ModSynthDlg.h"

void ModSynthEdit::OnConfig()
{
	ModSynthConfig dlg(msobj);
	if (dlg.ShowModal())
	{
		theProject->SetChange(1);
		delete mainGroup;
		mainGroup = new WidgetGroup;
		mainGroup->SetForm(this);
		CreateLayout();
		Resize();
	}
}

void ModSynthEdit::OnConnect()
{
	ModSynthConnect dlg(msobj);
	if (dlg.ShowModal())
		theProject->SetChange(1);
}

BEGIN_EVENT_TABLE(ModSynthConfig,wxDialog)
	EVT_INIT_DIALOG(ModSynthConfig::OnInitDialog)
	EVT_BUTTON(XRCID("IDC_MS_ADD"), ModSynthConfig::OnAdd)
	EVT_BUTTON(XRCID("IDC_MS_REM"), ModSynthConfig::OnRemove)
	EVT_BUTTON(XRCID("IDC_MS_MVUP"), ModSynthConfig::OnMoveUp)
	EVT_BUTTON(XRCID("IDC_MS_MVDN"), ModSynthConfig::OnMoveDown)
	EVT_LISTBOX(XRCID("IDC_MS_TYPE"), ModSynthConfig::OnChange)
	EVT_LISTBOX(XRCID("IDC_MS_UGENS"), ModSynthConfig::OnChange)
	EVT_TEXT(XRCID("IDC_MS_NAME"), ModSynthConfig::OnChange)
	EVT_BUTTON(wxID_OK, ModSynthConfig::OnOK)
	EVT_BUTTON(wxID_CANCEL, ModSynthConfig::OnCancel)
END_EVENT_TABLE()


void ModSynthConfig::OnInitDialog(wxInitDialogEvent& evt)
{
	CenterOnParent();

	btnAdd = (wxBitmapButton*)FindWindow("IDC_MS_ADD");
	btnRem = (wxBitmapButton*)FindWindow("IDC_MS_REM");
	btnDn = (wxBitmapButton*)FindWindow("IDC_MS_MVDN");
	btnUp = (wxBitmapButton*)FindWindow("IDC_MS_MVUP");
	lbType = (wxListBox*)FindWindow("IDC_MS_TYPE");
	lbUgens = (wxListBox*)FindWindow("IDC_MS_UGENS");
	edName = (wxTextCtrl*)FindWindow("IDC_MS_NAME");

	int tndx;
	for (tndx = 0; ugTypes[tndx].name; tndx++)
	{
		if (ugTypes[tndx].fn)
		{
			lbType->Append(ugTypes[tndx].longName, (void*)ugTypes[tndx].name);
		}
	}

	int nndx = -1;
	ModSynthUG *ug = ms->FirstUnit();
	while (ug)
	{
		const char *nm = ug->GetName();
		if (*nm != '@' && strcmp(nm, "out"))
		{
			nndx = lbUgens->Append(nm, (void*)ug);
		}
		ug = ms->NextUnit(ug);
	}
	if (nndx >= 0)
		lbUgens->SetSelection(nndx);

	EnableButtons();
}

void ModSynthConfig::OnChange(wxCommandEvent& evt)
{
	EnableButtons();
}

void ModSynthConfig::OnAdd(wxCommandEvent& evt)
{
	int tndx = lbType->GetSelection();
	if (tndx < 0)
		return;
	char *type = (char *) lbType->GetClientData(tndx);
	wxString name(edName->GetValue());
	if (name.Length() > 0)
	{
		ModSynthUG *before = 0;
		int nndx = lbUgens->GetSelection();
		if (nndx < 0)
			nndx = lbUgens->GetCount();
		else
			before = (ModSynthUG *) lbUgens->GetClientData(nndx);

		ModSynthUG *ug = ms->AddUnit(type, name, before);
		if (ug)
		{
			ug->InitDefault();
			lbUgens->Insert(name, nndx, (void*)ug);
			changed++;
		}
	}
	name.Empty();
	edName->ChangeValue(name);
	EnableButtons();
}

void ModSynthConfig::OnRemove(wxCommandEvent& evt)
{
	int nndx = lbUgens->GetSelection();
	if (nndx >= 0)
	{
		ModSynthUG *ug = (ModSynthUG *) lbUgens->GetClientData(nndx);
		if (ug)
			ms->RemoveUnit(ug, 1);
		lbUgens->Delete(nndx);
		changed++;
	}
	EnableButtons();
}

void ModSynthConfig::OnMoveUp(wxCommandEvent& evt)
{
	int nndx = lbUgens->GetSelection();
	if (nndx > 0)
	{
		ModSynthUG *ug1 = (ModSynthUG *)lbUgens->GetClientData(nndx);
		ModSynthUG *ug2 = (ModSynthUG *)lbUgens->GetClientData(nndx-1);
		lbUgens->Delete(nndx);
		lbUgens->Insert((const char *)ug1->GetName(), nndx-1, (void*)ug1);
		lbUgens->SetSelection(nndx-1);
		ms->MoveUnit(ug1, ug2);
		changed++;
	}
	EnableButtons();
}

void ModSynthConfig::OnMoveDown(wxCommandEvent& evt)
{
	int nndx = lbUgens->GetSelection();
	if (nndx >= 0 && nndx < ((int)lbUgens->GetCount()-1))
	{
		ModSynthUG *ug1 = (ModSynthUG *)lbUgens->GetClientData(nndx);
		ModSynthUG *ug2 = (ModSynthUG *)lbUgens->GetClientData(nndx+1);
		lbUgens->Delete(nndx);
		lbUgens->Insert(ug1->GetName(), nndx+1, (void*)ug1);
		lbUgens->SetSelection(nndx+1);
		ms->MoveUnit(ug2, ug1);
		changed++;
	}
	EnableButtons();
}

void ModSynthConfig::OnOK(wxCommandEvent& evt)
{
	EndModal(changed);
}

void ModSynthConfig::OnCancel(wxCommandEvent& evt)
{
	EndDialog(0);
}

void ModSynthConfig::EnableButtons()
{
	int undx = lbUgens->GetSelection();
	int ucnt = lbUgens->GetCount() - 1;
	int tndx = lbType->GetSelection();
	int nlen = edName->GetLastPosition();
	btnAdd->Enable(tndx >= 0 && nlen > 0);
	btnRem->Enable(undx >= 0);
	btnUp->Enable(undx > 0);
	btnDn->Enable(undx >= 0 && undx < ucnt);
}

////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ModSynthConnect,wxDialog)
	EVT_INIT_DIALOG(ModSynthConnect::OnInitDialog)
	EVT_BUTTON(XRCID("IDC_MC_ADD"), ModSynthConnect::OnAdd)
	EVT_BUTTON(XRCID("IDC_MC_REM"), ModSynthConnect::OnRemove)
	EVT_LISTBOX(XRCID("IDC_MC_SRC"), ModSynthConnect::OnSelSrc)
	EVT_LISTBOX(XRCID("IDC_MC_DST"), ModSynthConnect::OnSelDst)
	EVT_LISTBOX(XRCID("IDC_MC_PARAM"), ModSynthConnect::OnSelChange)
	EVT_LISTBOX(XRCID("IDC_MC_CONN"), ModSynthConnect::OnSelChange)
	EVT_BUTTON(wxID_OK, ModSynthConnect::OnOK)
	EVT_BUTTON(wxID_CANCEL, ModSynthConnect::OnCancel)
END_EVENT_TABLE()

void ModSynthConnect::OnInitDialog(wxInitDialogEvent& evt)
{
	CenterOnParent();

	btnAdd = (wxBitmapButton*)FindWindow("IDC_MC_ADD");
	btnRem = (wxBitmapButton*)FindWindow("IDC_MC_REM");
	lbConn = (wxListBox*)FindWindow("IDC_MC_CONN");
	lbSrc = (wxListBox*)FindWindow("IDC_MC_SRC");
	lbDst = (wxListBox*)FindWindow("IDC_MC_DST");
	lbParam = (wxListBox*)FindWindow("IDC_MC_PARAM");

	char buf[240];
	const UGParam *param;
	ConnInfo *ci;
	ModSynthConn *cn;
	ModSynthUG *ug = ms->FirstUnit();
	while (ug)
	{
		const char *nm = ug->GetName();
		if (strcmp(nm, "out"))
			lbSrc->Append(nm, (void*)ug);
		if (*nm != '@')
			lbDst->Append(nm, (void*)ug);
		for (cn = ug->ConnectList(0); cn; cn = ug->ConnectList(cn))
		{
			if (!cn->ug)
				continue;
			if ((param = cn->ug->FindParam(cn->index)) == 0)
				continue;
			ci = new ConnInfo;
			if (ci)
			{
				ci->src = ug;
				ci->dst = cn->ug;
				ci->index = cn->index;
				snprintf(buf, 240, "%s > %s.%s", ug->GetName(), cn->ug->GetName(), param->name);
				lbConn->Append(buf, (void*)ci);
			}
		}
		ug = ms->NextUnit(ug);
	}
	lbSrc->SetSelection(wxNOT_FOUND);
	lbDst->SetSelection(wxNOT_FOUND);

	EnableButtons();
}

void ModSynthConnect::OnOK(wxCommandEvent& evt)
{
	ClearConnInfo();
	EndModal(changed);
}

void ModSynthConnect::OnCancel(wxCommandEvent& evt)
{
	ClearConnInfo();
	EndModal(0);
}

void ModSynthConnect::OnAdd(wxCommandEvent& evt)
{
	int sndx = lbSrc->GetSelection();
	int dndx = lbDst->GetSelection();
	int pndx = lbParam->GetSelection();
	if (sndx < 0 || dndx < 0 || pndx < 0)
		return;

	ModSynthUG *src = (ModSynthUG *) lbSrc->GetClientData(sndx);
	ModSynthUG *dst = (ModSynthUG *) lbDst->GetClientData(dndx);
	UGParam *param = (UGParam *) lbParam->GetClientData(pndx);
	if (!src || !dst || !param)
		return;

	ConnInfo *ci = new ConnInfo;
	if (!ci)
		return;
	ci->src = src;
	ci->dst = dst;
	ci->index = param->index;
	char conn[240];
	snprintf(conn, 240, "%s > %s.%s", src->GetName(), dst->GetName(), param->name);
	lbConn->Append(conn, (void*)ci);
	ms->Connect(src, dst, param->index);
	changed++;
	EnableButtons();
}

void ModSynthConnect::OnRemove(wxCommandEvent& evt)
{
	wxArrayInt selbuf;
	int selcnt = lbConn->GetSelections(selbuf);
	if (selcnt > 0)
	{
		int cndx;
		for (cndx = 0; cndx < selcnt; cndx++)
		{
			ConnInfo *ci = (ConnInfo *) lbConn->GetClientData(selbuf[cndx]);
			if (ci)
			{
				ms->Disconnect(ci->src, ci->dst, ci->index);
				delete ci;
				changed++;
			}
		}
		for (cndx = selcnt-1; cndx >= 0; cndx--)
			lbConn->Delete(selbuf[cndx]);
	}
	EnableButtons();
}

void ModSynthConnect::OnSelSrc(wxCommandEvent& evt)
{
	int sndx = lbSrc->GetSelection();
	if (sndx == wxNOT_FOUND)
		return;

	ModSynthUG *ug = (ModSynthUG *) lbSrc->GetClientData(sndx);
	if (!ug)
		return;

	ConnInfo *ci;
	int count = lbConn->GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		ci = (ConnInfo *)lbConn->GetClientData(index);
		if (ci && ci->src == ug)
			lbConn->Select(index);
		else
			lbConn->Deselect(index);
	}
}

void ModSynthConnect::OnSelDst(wxCommandEvent& evt)
{
	lbParam->Clear();

	int dndx = lbDst->GetSelection();
	if (dndx == wxNOT_FOUND)
		return;

	ModSynthUG *ug = (ModSynthUG *) lbDst->GetClientData(dndx);
	const UGParam *param = ug->GetParamList();
	while (param->name)
	{
		if (!(param->when & UGP_DEF))
			lbParam->Append((const char *)param->name, (void*)param);
		param++;
	}
	EnableButtons();
}

void ModSynthConnect::OnSelChange(wxCommandEvent& evt)
{
	EnableButtons();
}

void ModSynthConnect::ClearConnInfo()
{
	int count = lbConn->GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		ConnInfo *ci = (ConnInfo *) lbConn->GetClientData(index);
		if (ci)
			delete ci;
	}
}

void ModSynthConnect::EnableButtons()
{
	wxArrayInt selbuf;
	int selcnt = lbConn->GetSelections(selbuf);
	int sndx = lbSrc->GetSelection();
	int dndx = lbDst->GetSelection();
	int pndx = lbParam->GetSelection();
	btnAdd->Enable(sndx >= 0 && dndx >= 0 && pndx >= 0);
	btnRem->Enable(selcnt > 0);
}
