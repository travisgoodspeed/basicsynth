//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "ProjectItemDlg.h"
#include "MixerSetupDlg.h"

EffectsSetupDlg::EffectsSetupDlg()
{
}

EffectsSetupDlg::~EffectsSetupDlg()
{
}

static int fxLblIDS[] = { -1, IDC_NAME_LBL, -1, IDC_VOL_LBL, IDC_PAN_LBL, IDC_VAL1, IDC_VAL2, IDC_VAL3, IDC_VAL4, IDC_VAL5 };
static int fxValIDS[] = { -1, IDC_ITEM_NAME, IDC_ITEM_DESCR, IDC_VOL, IDC_PAN, IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5 };

int EffectsSetupDlg::GetFieldID(int id, int& idval)
{
	if (id < (sizeof(fxValIDS)/sizeof(int)))
	{
		idval = fxValIDS[id];
		return 1;
	}
	return 0;
}


int EffectsSetupDlg::GetLabelID(int id, int& idval)
{
	if (id < (sizeof(fxLblIDS)/sizeof(int)))
	{
		idval = fxLblIDS[id];
		return 1;
	}
	return 0;
}

LRESULT EffectsSetupDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (pi == NULL)
	{
		DestroyWindow();
		return 0;
	}

	CenterWindow();

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	return 1;
}

LRESULT EffectsSetupDlg::OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	pi->LoadProperties(static_cast<PropertyBox*>(this));
	return 0;
}

LRESULT EffectsSetupDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndDialog(1);
	return 0;
}

LRESULT EffectsSetupDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

//////////////////////////////////////////////////////////

static int mixIDVals[] = {	-1, 
	IDC_INPUTS, IDC_PAN_LIN, IDC_PAN_TRIG, IDC_PAN_SQRT,
	IDC_FX_LIST, IDC_FX_NAME, IDC_FX_TYPE 
};

MixerSetupDlg::MixerSetupDlg()
{
}

MixerSetupDlg::~MixerSetupDlg()
{
}

int MixerSetupDlg::GetFieldID(int id, int& idval)
{
	if (id < (sizeof(mixIDVals)/sizeof(int)))
	{
		idval = mixIDVals[id];
		return 1;
	}
	return 0;
}

LRESULT MixerSetupDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	edName = GetDlgItem(IDC_FX_NAME);
	fxList = GetDlgItem(IDC_FX_LIST);
	fxType = GetDlgItem(IDC_FX_TYPE);
	// TODO: this list should come from the project info...
	fxType.InsertString(0, "reverb");
	fxType.InsertString(1, "flanger");
	fxType.InsertString(2, "echo");
	fxType.SetCurSel(0);

	SetButtonImage(IDC_FX_ADD, IDI_ADD);
	SetButtonImage(IDC_FX_REM, IDI_REM);
	SetButtonImage(IDC_FX_UP, IDI_MVUP);
	SetButtonImage(IDC_FX_DN, IDI_MVDN);
	SetButtonImage(IDC_FX_EDIT, IDI_INFO);

	pi->LoadProperties(this);

	EnableUpDn();
	return 1;
}

LRESULT MixerSetupDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (pi->SaveProperties(this))
		EndDialog(1);
	return 0;
}

LRESULT MixerSetupDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

void MixerSetupDlg::EnableUpDn()
{
	int count = fxList.GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = fxList.GetCurSel();
	if (sel == LB_ERR)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	::EnableWindow(GetDlgItem(IDC_FX_UP), mvUp);
	::EnableWindow(GetDlgItem(IDC_FX_DN), mvDn);
	::EnableWindow(GetDlgItem(IDC_FX_EDIT), sel >= 0);
}

LRESULT MixerSetupDlg::OnFxSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableUpDn();
	return 0;
}

LRESULT MixerSetupDlg::OnFxMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fxList.GetCurSel();
	if (sel == LB_ERR || sel == 0)
		return 0;
	
	FxItem *fx = (FxItem *) fxList.GetItemDataPtr(sel);
	fxList.DeleteString(sel);
	sel--;
	fxList.InsertString(sel, fx->GetName());
	fxList.SetItemDataPtr(sel, fx);
	fxList.SetCurSel(sel);

	EnableUpDn();

	return 0;
}

LRESULT MixerSetupDlg::OnFxMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fxList.GetCurSel();
	if (sel == LB_ERR && sel < fxList.GetCount())
		return 0;

	FxItem *fx = (FxItem *) fxList.GetItemDataPtr(sel);
	fxList.DeleteString(sel);
	sel++;
	fxList.InsertString(sel, fx->GetName());
	fxList.SetItemDataPtr(sel, fx);
	fxList.SetCurSel(sel);

	EnableUpDn();

	return 0;
}

LRESULT MixerSetupDlg::OnFxAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char name[80];
	char type[80];
	memset(name, 0, sizeof(name));
	memset(type, 0, sizeof(type));
	if (fxType.GetCurSel() != CB_ERR)
	{
		fxType.GetWindowText(type, MAX_PATH);
		FxItem *fx = ((MixerItem*)pi)->AddEffect(type);
		if (fx)
		{
			edName.GetWindowText(name, MAX_PATH);
			if (name[0] == 0)
				strcpy(name, type);
			fx->SetName(name);
			fx->SetUnit(-1);
			int ndx = fxList.GetCount();
			ndx = fxList.InsertString(ndx, name);
			fxList.SetItemDataPtr(ndx, fx);
		}

		EnableUpDn();
	}
	return 0;
}

LRESULT MixerSetupDlg::OnFxRem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fxList.GetCurSel();
	if (sel != LB_ERR)
	{
		FxItem *fx = (FxItem *) fxList.GetItemDataPtr(sel);
		fxList.DeleteString(sel);
	}

	EnableUpDn();

	return 0;
}

LRESULT MixerSetupDlg::OnFxEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fxList.GetCurSel();
	if (sel != LB_ERR)
	{
		FxItem *fx = (FxItem *) fxList.GetItemDataPtr(sel);
		if (fx->ItemProperties())
		{
			fxList.DeleteString(sel);
			fxList.InsertString(sel, fx->GetName());
			fxList.SetItemDataPtr(sel, fx);
			fxList.SetCurSel(sel);
		}
	}

	return 0;
}

void MixerSetupDlg::SetButtonImage(int ctrl, int imgid)
{
	HICON ico = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(imgid), IMAGE_ICON, 32, 32, 0);
	CButton btn = GetDlgItem(ctrl);
	btn.SetIcon(ico);
}
