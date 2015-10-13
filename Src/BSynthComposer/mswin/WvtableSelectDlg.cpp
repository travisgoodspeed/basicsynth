//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "resource.h"
#include "WvtableSelectDlg.h"

WvtableSelectDlg::WvtableSelectDlg()
{
}

LRESULT WvtableSelectDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	RECT rcPlot;
	CWindow frm = GetDlgItem(IDC_WT_GRAPH);
	frm.GetClientRect(&rcPlot);
	frm.MapWindowPoints(m_hWnd, &rcPlot);
	plotter.Create(m_hWnd, rcPlot, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0/*WS_EX_CLIENTEDGE*/);
	frm.ShowWindow(SW_HIDE);

	int ndx;
	lst = GetDlgItem(IDC_WT_LIST);
	ndx = lst.AddString("Sin");
	lst.SetItemData(ndx, 0);
	ndx = lst.AddString("Sawtooth (sum)");
	lst.SetItemData(ndx, 1);
	ndx = lst.AddString("Square (sum)");
	lst.SetItemData(ndx, 2);
	ndx = lst.AddString("Triangle (sum)");
	lst.SetItemData(ndx, 3);
	ndx = lst.AddString("Pulse (sum)");
	lst.SetItemData(ndx, 4);
	ndx = lst.AddString("Sawtooth (direct)");
	lst.SetItemData(ndx, 5);
	ndx = lst.AddString("Square (direct)");
	lst.SetItemData(ndx, 6);
	ndx = lst.AddString("Triangle (direct)");
	lst.SetItemData(ndx, 7);
	ndx = lst.AddString("Sawtooth (positive)");
	lst.SetItemData(ndx, 8);
	ndx = lst.AddString("Triangle (positive)");
	lst.SetItemData(ndx, 9);

	int selected = plotter.index;
	ProjectItem *pi = prjTree->FirstChild(theProject->synthInfo);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVTABLE)
		{
			WavetableItem *wi = (WavetableItem*)pi;
			ndx = lst.AddStringUTF8(wi->GetName());
			lst.SetItemData(ndx, wi->GetID());
			if (wi->GetID() == plotter.index)
				selected = ndx;
		}
		pi = prjTree->NextSibling(pi);
	}
	lst.SetCurSel(selected);
	return 0;
}

LRESULT WvtableSelectDlg::OnIndexChange(WORD code, WORD id, HWND ctl, BOOL& bHandled)
{
	int n = lst.GetCurSel();
	if (n >= 0)
	{
		plotter.index = lst.GetItemData(n);
		plotter.Invalidate();
	}
	return 0;
}

LRESULT WvtableSelectDlg::OnOK(WORD code, WORD id, HWND ctl, BOOL& bHandled)
{
	EndDialog(IDOK);
	return 0;
}

LRESULT WvtableSelectDlg::OnCancel(WORD code, WORD id, HWND ctl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT WavetableDraw::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps;
	BeginPaint(&ps);
	Graphics *gr = new Graphics(ps.hdc);
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush bg(Color::White);
	Pen pn(Color::Black);
	Pen pn2(Color::Gray, 0);

	RECT rc;
	GetClientRect(&rc);
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;
	gr->FillRectangle(&bg, rc.left, rc.right, cx, cy);
	cx -= 2;
	cy -= 2;
	AmpValue *ampvals;
	if (index >= 0 && index <= wtSet.wavTblMax
	 && (ampvals = wtSet.GetWavetable(wtSet.FindWavetable(index))) != 0
	 && cx > 0 && cy > 0)
	{
		double inc = synthParams.ftableLength / (float) cx;
		double tn = inc;
		REAL mid = (REAL) cy / 2.0;
		REAL yo = (REAL) (rc.top + 1) + mid;
		REAL x1 = (REAL) (rc.left + 1);
		REAL x2 = x1;
		REAL y1 = yo - (ampvals[0] * mid);
		REAL y2;
		gr->DrawLine(&pn2, x1, yo, x1 + (REAL) cx, yo);
		for (int x = 1; x < cx; x++)
		{
			x2 = x1 + 1;
			y2 = yo - (ampvals[(int)(tn+0.5)] * mid);
			gr->DrawLine(&pn, x1, y1, x2, y2);
			y1 = y2;
			x1 = x2;
			tn += inc;
		}
		y2 = yo - (ampvals[0] * mid);
		gr->DrawLine(&pn, x1, y1, x1+1, y2);
	}
	delete gr;
	EndPaint(&ps);
	return 0;
}

int SynthEdit::SelectWavetable(int wt)
{
	WvtableSelectDlg dlg;
	dlg.SetIndex(wt);
	if (dlg.DoModal() == IDOK)
		wt = dlg.GetIndex();
	return wt;
}
