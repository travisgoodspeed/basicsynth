//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "ProjectItemDlg.h"
#include "MixerSetupDlg.h"

static const char *fxLblIDS[] =
{
	"NONE",
	"IDC_NAME_LBL",
	"NONE",
	"IDC_VOL_LBL",
	"IDC_PAN_LBL",
	"IDC_VAL1",
	"IDC_VAL2",
	"IDC_VAL3",
	"IDC_VAL4",
	"IDC_VAL5"
};

static const char *fxValIDS[] =
{
	"NONE",
	"IDC_ITEM_NAME",
	"IDC_ITEM_DESCR",
	"IDC_VOL",
	"IDC_PAN",
	"IDC_EDIT1",
	"IDC_EDIT2",
	"IDC_EDIT3",
	"IDC_EDIT4",
	"IDC_EDIT5"
};

BEGIN_EVENT_TABLE(EffectsSetupDlg,wxDialog)
	EVT_BUTTON(wxID_OK, EffectsSetupDlg::OnOK)
	EVT_BUTTON(wxID_CANCEL, EffectsSetupDlg::OnCancel)
	EVT_TEXT(XRCID("IDC_RESET"), EffectsSetupDlg::OnReset)
	EVT_INIT_DIALOG(EffectsSetupDlg::OnInitDialog)
END_EVENT_TABLE()

EffectsSetupDlg::EffectsSetupDlg(wxWindow *parent, ProjectItem *pi)
{
	SetItem(pi);
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_EFFECTS");
}

EffectsSetupDlg::~EffectsSetupDlg()
{
}

int EffectsSetupDlg::GetFieldID(int id, wxWindow **child)
{
	if (id < (int)(sizeof(fxValIDS)/sizeof(char*)))
		*child = FindWindow(fxValIDS[id]);
	else
		*child = 0;
	if (*child)
		return 1;
	return 0;
}


int EffectsSetupDlg::GetLabelID(int id, wxWindow **child)
{
	if (id < (int)(sizeof(fxLblIDS)/sizeof(char*)))
		*child = FindWindow(fxLblIDS[id]);
	else
		*child = 0;
	if (*child)
		return 1;
	return 0;
}

void EffectsSetupDlg::OnInitDialog(wxInitDialogEvent&)
{
	CenterOnParent();

	pi->LoadProperties(static_cast<PropertyBox*>(this));
}

void EffectsSetupDlg::OnReset(wxCommandEvent&)
{
	pi->LoadProperties(static_cast<PropertyBox*>(this));
}

void EffectsSetupDlg::OnOK(wxCommandEvent&)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndModal(1);
}

void EffectsSetupDlg::OnCancel(wxCommandEvent&)
{
	EndModal(0);
}

//////////////////////////////////////////////////////////

static const char *mixIDVals[] =
{
	"NONE",
	"IDC_INPUTS",
	"IDC_PAN_LIN",
	"IDC_PAN_TRIG",
	"IDC_PAN_SQRT",
	"IDC_FX_LIST",
	"IDC_FX_NAME",
	"IDC_FX_TYPE"
};

BEGIN_EVENT_TABLE(MixerSetupDlg,wxDialog)
	EVT_BUTTON(wxID_OK, MixerSetupDlg::OnOK)
	EVT_BUTTON(wxID_CANCEL, MixerSetupDlg::OnCancel)
	EVT_BUTTON(XRCID("IDC_FX_UP"), MixerSetupDlg::OnFxMvup)
	EVT_BUTTON(XRCID("IDC_FX_DN"), MixerSetupDlg::OnFxMvdn)
	EVT_BUTTON(XRCID("IDC_FX_ADD"), MixerSetupDlg::OnFxAdd)
	EVT_BUTTON(XRCID("IDC_FX_REM"), MixerSetupDlg::OnFxRem)
	EVT_BUTTON(XRCID("IDC_FX_EDIT"), MixerSetupDlg::OnFxEdit)
	EVT_LISTBOX(XRCID("IDC_FX_LIST"), MixerSetupDlg::OnFxSelect)
	EVT_INIT_DIALOG(MixerSetupDlg::OnInitDialog)
END_EVENT_TABLE()


MixerSetupDlg::MixerSetupDlg(wxWindow *parent, ProjectItem *pi)
{
	SetItem(pi);
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_MIXER_SETUP");
}

MixerSetupDlg::~MixerSetupDlg()
{
}

int MixerSetupDlg::GetFieldID(int id, wxWindow **child)
{
	if (id < (int)(sizeof(mixIDVals)/sizeof(char*)))
		*child = FindWindow(mixIDVals[id]);
	else
		*child = 0;
	if (*child)
		return 1;
	return 0;
}

void MixerSetupDlg::OnInitDialog(wxInitDialogEvent&)
{
	CenterOnParent();

	edName = (wxTextCtrl *)FindWindow("IDC_FX_NAME");
	fxList = (wxListBox*)FindWindow("IDC_FX_LIST");
	fxType = (wxComboBox*)FindWindow("IDC_FX_TYPE");
	// TODO: mixer should fill this list...
	fxType->Insert("reverb", 0);
	fxType->Insert("flanger", 1);
	fxType->Insert("echo", 2);
	fxType->SetSelection(0);

	pi->LoadProperties(this);

	EnableUpDn();
}

void MixerSetupDlg::OnOK(wxCommandEvent&)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndModal(1);
}

void MixerSetupDlg::OnCancel(wxCommandEvent&)
{
	EndModal(0);
}

void MixerSetupDlg::EnableUpDn()
{
	int count = (int)fxList->GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = fxList->GetSelection();
	if (sel == wxNOT_FOUND)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	FindWindow("IDC_FX_UP")->Enable(mvUp);
	FindWindow("IDC_FX_DN")->Enable(mvDn);
	FindWindow("IDC_FX_EDIT")->Enable(sel >= 0);
}

void MixerSetupDlg::OnFxSelect(wxCommandEvent& evt)
{
	EnableUpDn();
}

void MixerSetupDlg::OnFxMvup(wxCommandEvent& evt)
{
	int sel = fxList->GetSelection();
	if (sel == wxNOT_FOUND || sel == 0)
		return;

	FxItem *fx = (FxItem *) fxList->GetClientData(sel);
	fxList->Delete((unsigned int)sel);
	sel--;
	fxList->Insert(fx->GetName(), (unsigned int)sel, reinterpret_cast<void*>(fx));
	fxList->SetSelection(sel);

	EnableUpDn();
}

void MixerSetupDlg::OnFxMvdn(wxCommandEvent& evt)
{
	int sel = fxList->GetSelection();
	if (sel == wxNOT_FOUND || sel >= (int)fxList->GetCount())
		return;

	FxItem *fx = (FxItem *) fxList->GetClientData(sel);
	fxList->Delete((unsigned int)sel);
	sel++;
	fxList->Insert(fx->GetName(), (unsigned int)sel, reinterpret_cast<void*>(fx));
	fxList->SetSelection(sel);

	EnableUpDn();
}

void MixerSetupDlg::OnFxAdd(wxCommandEvent& evt)
{
	int sel = fxType->GetSelection();
	if (sel != wxNOT_FOUND)
	{
		wxString type(fxType->GetString(sel));
		FxItem *fx = ((MixerItem*)pi)->AddEffect(type);
		if (fx)
		{
			wxString name(edName->GetValue());
			if (name.Length() == 0)
				name = type;
			fx->SetName(name);
			fx->SetUnit(-1);
			fxList->Insert(name, fxList->GetCount(), reinterpret_cast<void*>(fx));
		}

		EnableUpDn();
	}
}

void MixerSetupDlg::OnFxRem(wxCommandEvent& evt)
{
	int sel = fxList->GetSelection();
	if (sel != wxNOT_FOUND)
	{
		//FxItem *fx = (FxItem *) fxList->GetClientData(sel);
		fxList->Delete(sel);
	}

	EnableUpDn();
}

void MixerSetupDlg::OnFxEdit(wxCommandEvent& evt)
{
	int sel = fxList->GetSelection();
	if (sel != wxNOT_FOUND)
	{
		FxItem *fx = (FxItem *) fxList->GetClientData(sel);
		if (fx && fx->ItemProperties())
			fxList->SetString(sel, fx->GetName());
	}
}
