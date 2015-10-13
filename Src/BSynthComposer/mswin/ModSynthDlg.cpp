//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <ModSynthEd.h>
#include "ModSynthDlg.h"

void ModSynthEdit::OnConfig()
{
	ModSynthConfig dlg(msobj);
	if (dlg.DoModal())
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
	if (dlg.DoModal())
		theProject->SetChange(1);
}

LRESULT ModSynthConfig::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	CButton btn;

	btnAdd = GetDlgItem(IDC_MS_ADD);
	btnRem = GetDlgItem(IDC_MS_REM);
	btnDn = GetDlgItem(IDC_MS_MVDN);
	btnUp = GetDlgItem(IDC_MS_MVUP);
	lbType = GetDlgItem(IDC_MS_TYPE);
	lbUgens = GetDlgItem(IDC_MS_UGENS);
	edName = GetDlgItem(IDC_MS_NAME);

	RECT rc;
	btnUp.GetClientRect(&rc);
	int cx = 16;
	if (rc.right >= 32)
		cx = 32;

	HICON mvUp = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVUP), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON mvDn = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVDN), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON add = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON rem = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_REM), IMAGE_ICON, cx, cx, LR_SHARED);

	btnUp.SetIcon(mvUp);
	btnDn.SetIcon(mvDn);
	btnAdd.SetIcon(add);
	btnRem.SetIcon(rem);

	int tndx;
	int ndx;
	for (tndx = 0; ugTypes[tndx].name; tndx++)
	{
		if (ugTypes[tndx].fn)
		{
			ndx = lbType.AddString(ugTypes[tndx].longName);
			lbType.SetItemDataPtr(ndx, (void*)ugTypes[tndx].name);
		}
	}

	int nndx = -1;
	ModSynthUG *ug = ms->FirstUnit();
	while (ug)
	{
		const char *nm = ug->GetName();
		if (*nm != '@' && strcmp(nm, "out"))
		{
			nndx = lbUgens.AddString(nm);
			lbUgens.SetItemDataPtr(nndx, (void*)ug);
		}
		ug = ms->NextUnit(ug);
	}
	if (nndx >= 0)
		lbUgens.SetCurSel(nndx);

	EnableButtons();
	return 1;
}

LRESULT ModSynthConfig::OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableButtons();
	return 0;
}

LRESULT ModSynthConfig::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int tndx = lbType.GetCurSel();
	if (tndx < 0)
		return 0;
	char *type = (char *) lbType.GetItemDataPtr(tndx);
	char name[80];
	name[0] = 0;
	edName.GetWindowText(name, 80);
	if (name[0])
	{
		ModSynthUG *before = 0;
		int nndx = lbUgens.GetCurSel();
		if (nndx < 0)
			nndx = lbUgens.GetCount();
		else
			before = (ModSynthUG *) lbUgens.GetItemDataPtr(nndx);

		ModSynthUG *ug = ms->AddUnit(type, name, before);
		if (ug)
		{
			ug->InitDefault();
			nndx = lbUgens.InsertString(nndx, name);
			lbUgens.SetItemDataPtr(nndx, (void*)ug);
			changed++;
		}
	}
	edName.SetWindowText("");
	EnableButtons();
	return 0;
}

LRESULT ModSynthConfig::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int nndx = lbUgens.GetCurSel();
	if (nndx >= 0)
	{
		ModSynthUG *ug = (ModSynthUG *) lbUgens.GetItemDataPtr(nndx);
		if (ug)
			ms->RemoveUnit(ug, 1);
		lbUgens.DeleteString(nndx);
		changed++;
	}
	EnableButtons();
	return 0;
}

LRESULT ModSynthConfig::OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int nndx = lbUgens.GetCurSel();
	if (nndx > 0)
	{
		ModSynthUG *ug1 = (ModSynthUG *)lbUgens.GetItemDataPtr(nndx);
		ModSynthUG *ug2 = (ModSynthUG *)lbUgens.GetItemDataPtr(nndx-1);
		lbUgens.DeleteString(nndx);
		nndx = lbUgens.InsertString(nndx-1, ug1->GetName());
		lbUgens.SetItemDataPtr(nndx, (void*)ug1);
		lbUgens.SetCurSel(nndx);
		ms->MoveUnit(ug1, ug2);
		changed++;
	}
	EnableButtons();
	return 0;
}

LRESULT ModSynthConfig::OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int nndx = lbUgens.GetCurSel();
	if (nndx >= 0 && nndx < (lbUgens.GetCount()-1))
	{
		ModSynthUG *ug1 = (ModSynthUG *)lbUgens.GetItemDataPtr(nndx);
		ModSynthUG *ug2 = (ModSynthUG *)lbUgens.GetItemDataPtr(nndx+1);
		lbUgens.DeleteString(nndx);
		nndx = lbUgens.InsertString(nndx+1, ug1->GetName());
		lbUgens.SetItemDataPtr(nndx, ug1);
		lbUgens.SetCurSel(nndx);
		ms->MoveUnit(ug2, ug1);
		changed++;
	}
	EnableButtons();
	return 0;
}

LRESULT ModSynthConfig::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(changed);
	return 0;
}

LRESULT ModSynthConfig::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

void ModSynthConfig::EnableButtons()
{
	int undx = lbUgens.GetCurSel();
	int ucnt = lbUgens.GetCount() - 1;
	int tndx = lbType.GetCurSel();
	int nlen = edName.GetWindowTextLength();
	btnAdd.EnableWindow(tndx >= 0 && nlen > 0);
	btnRem.EnableWindow(undx >= 0);
	btnUp.EnableWindow(undx > 0);
	btnDn.EnableWindow(undx >= 0 && undx < ucnt);
}

////////////////////////////////////////////////////////////////

LRESULT ModSynthConnect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	btnAdd = GetDlgItem(IDC_MC_ADD);
	btnRem = GetDlgItem(IDC_MC_REM);

	RECT rc;
	btnAdd.GetClientRect(&rc);
	int cx = 16;
	if (rc.right >= 32)
		cx = 32;

	HICON add = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON rem = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_REM), IMAGE_ICON, cx, cx, LR_SHARED);

	btnAdd.SetIcon(add);
	btnRem.SetIcon(rem);

	lbConn = GetDlgItem(IDC_MC_CONN);
	lbSrc = GetDlgItem(IDC_MC_SRC);
	lbDst = GetDlgItem(IDC_MC_DST);
	lbParam = GetDlgItem(IDC_MC_PARAM);

	int ndx;
	int cndx;
	char buf[240];
	const UGParam *param;
	ConnInfo *ci;
	ModSynthConn *cn;
	ModSynthUG *ug = ms->FirstUnit();
	while (ug)
	{
		const char *nm = ug->GetName();
		if (strcmp(nm, "out"))
		{
			ndx = lbSrc.AddString(nm);
			lbSrc.SetItemDataPtr(ndx, (void*)ug);
		}
		if (*nm != '@')
		{
			ndx = lbDst.AddString(nm);
			lbDst.SetItemDataPtr(ndx, (void*)ug);
		}
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
				cndx = lbConn.AddString(buf);
				lbConn.SetItemDataPtr(cndx, (void*)ci);
			}
		}
		ug = ms->NextUnit(ug);
	}
	lbSrc.SetCurSel(-1);
	lbDst.SetCurSel(-1);

	EnableButtons();
	return 1;
}

LRESULT ModSynthConnect::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ClearConnInfo();
	EndDialog(changed);
	return 0;
}

LRESULT ModSynthConnect::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ClearConnInfo();
	EndDialog(0);
	return 0;
}

LRESULT ModSynthConnect::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sndx = lbSrc.GetCurSel();
	int dndx = lbDst.GetCurSel();
	int pndx = lbParam.GetCurSel();
	if (sndx < 0 || dndx < 0 || pndx < 0)
		return 0;

	ModSynthUG *src = (ModSynthUG *) lbSrc.GetItemDataPtr(sndx);
	ModSynthUG *dst = (ModSynthUG *) lbDst.GetItemDataPtr(dndx);
	UGParam *param = (UGParam *) lbParam.GetItemDataPtr(pndx);
	if (!src || !dst || !param)
		return 0;

	ConnInfo *ci = new ConnInfo;
	if (!ci)
		return 0;
	ci->src = src;
	ci->dst = dst;
	ci->index = param->index;
	char conn[240];
	snprintf(conn, 240, "%s > %s.%s", src->GetName(), dst->GetName(), param->name);
	int cndx = lbConn.AddString(conn);
	lbConn.SetItemDataPtr(cndx, ci);
	//lbConn.SetCurSel(cndx);
	ms->Connect(src, dst, param->index);
	changed++;
	EnableButtons();
	return 0;
}

LRESULT ModSynthConnect::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int selcnt = lbConn.GetSelCount();
	if (selcnt > 0)
	{
		int *selbuf = new int[selcnt];
		lbConn.GetSelItems(selcnt, selbuf);
		int cndx;
		for (cndx = 0; cndx < selcnt; cndx++)
		{
			ConnInfo *ci = (ConnInfo *) lbConn.GetItemDataPtr(selbuf[cndx]);
			if (ci)
			{
				ms->Disconnect(ci->src, ci->dst, ci->index);
				delete ci;
				changed++;
			}
		}
		for (cndx = selcnt-1; cndx >= 0; cndx--)
			lbConn.DeleteString(selbuf[cndx]);
		delete selbuf;
	}
	EnableButtons();
	return 0;
}

LRESULT ModSynthConnect::OnSelSrc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sndx = lbSrc.GetCurSel();
	if (sndx == LB_ERR)
		return 0;

	ModSynthUG *ug = (ModSynthUG *) lbSrc.GetItemDataPtr(sndx);
	if (!ug)
		return 0;

	ConnInfo *ci;
	int count = lbConn.GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		ci = (ConnInfo *)lbConn.GetItemDataPtr(index);
		lbConn.SetSel(index, ci && ci->src == ug);
	}

	return 0;
}

LRESULT ModSynthConnect::OnSelDst(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	lbParam.ResetContent();

	int dndx = lbDst.GetCurSel();
	if (dndx == LB_ERR)
		return 0;

	ModSynthUG *ug = (ModSynthUG *) lbDst.GetItemDataPtr(dndx);
	const UGParam *param = ug->GetParamList();
	while (param->name)
	{
		if (!(param->when & UGP_DEF))
		{
			int pndx = lbParam.AddString(param->name);
			lbParam.SetItemDataPtr(pndx, (void*)param);
		}
		param++;
	}
	EnableButtons();
	return 0;
}

LRESULT ModSynthConnect::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableButtons();
	return 0;
}

void ModSynthConnect::ClearConnInfo()
{
	int count = lbConn.GetCount();
	int index;
	for (index = 0; index < count; index++)
	{
		ConnInfo *ci = (ConnInfo *) lbConn.GetItemDataPtr(index);
		if (ci)
			delete ci;
	}
}

void ModSynthConnect::EnableButtons()
{
	int sndx = lbSrc.GetCurSel();
	int dndx = lbDst.GetCurSel();
	int pndx = lbParam.GetCurSel();
	btnAdd.EnableWindow(sndx >= 0 && dndx >= 0 && pndx >= 0);
	btnRem.EnableWindow(lbConn.GetSelCount() > 0);
}
