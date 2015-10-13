//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "WavetableSelectDlg.h"

BEGIN_EVENT_TABLE(WavetableDraw,wxWindow)
	EVT_PAINT(WavetableDraw::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(WavetableSelectDlg,wxDialog)
	EVT_INIT_DIALOG(WavetableSelectDlg::OnInitDialog)
	EVT_LISTBOX(XRCID("IDC_WT_LIST"), WavetableSelectDlg::OnIndexChange)
	EVT_BUTTON(wxID_OK, WavetableSelectDlg::OnOK)
	EVT_BUTTON(wxID_CANCEL, WavetableSelectDlg::OnCancel)
END_EVENT_TABLE()


WavetableSelectDlg::WavetableSelectDlg(wxWindow *parent, int wt)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_WT_SELECT");
	initSelect = wt;
}

void WavetableSelectDlg::OnInitDialog(wxInitDialogEvent& evt)
{
	CenterOnParent();
	wxWindow *frm = FindWindow("IDC_WT_GRAPH");
	if (!frm)
		return;
	wxPoint pos = frm->GetPosition();
	wxSize  siz = frm->GetSize();
	frm->Destroy();

	plotter = new WavetableDraw(this, 99, pos, siz);
	plotter->index = initSelect;

	lst = (wxListBox*)FindWindow("IDC_WT_LIST");
	lst->Append("Sin", (void*)0);
	lst->Append("Sawtooth (sum)", (void*)1);
	lst->Append("Square (sum)", (void*)2);
	lst->Append("Triangle (sum)", (void*)3);
	lst->Append("Pulse (sum)", (void*)4);
	lst->Append("Sawtooth (direct)", (void*)5);
	lst->Append("Square (direct)", (void*)6);
	lst->Append("Triangle (direct)", (void*)7);
	lst->Append("Sawtooth (positive)", (void*)8);
	lst->Append("Triangle (positive)", (void*)9);

	int selected = initSelect;
	ProjectItem *pi = prjTree->FirstChild(theProject->synthInfo);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVTABLE)
		{
			WavetableItem *wi = (WavetableItem*)pi;
			int index = lst->Append(wi->GetName(), (void*)(long)wi->GetID());
			if (wi->GetID() == initSelect)
				selected = index;
		}
		pi = prjTree->NextSibling(pi);
	}
	if ((unsigned int)selected < lst->GetCount())
		lst->SetSelection(selected);
}

void WavetableSelectDlg::OnIndexChange(wxCommandEvent& evt)
{
	int n = lst->GetSelection();
	if (n >= 0)
	{
		plotter->index = (int) reinterpret_cast<long>(lst->GetClientData(n));
		plotter->Refresh();
	}
}

void WavetableSelectDlg::OnOK(wxCommandEvent& evt)
{
	initSelect = plotter->index;
	EndModal(1);
}

void WavetableSelectDlg::OnCancel(wxCommandEvent& evt)
{
	EndModal(0);
}

void WavetableDraw::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	wxSize sz = GetClientSize();
#if wxUSE_GRAPHICS_CONTEXT
	wxGraphicsContext *gr = wxGraphicsContext::Create((wxWindowDC&)dc);
	wxGraphicsBrush br(gr->CreateBrush(*wxWHITE_BRUSH));
	gr->SetBrush(br);
	wxGraphicsPen pn(gr->CreatePen(*wxBLACK_PEN));
	gr->SetPen(pn);
	gr->DrawRectangle(0, 0, sz.GetWidth()-1, sz.GetHeight()-1);
#else
	wxBrush whtBrush(wxColour(255,255,255));
	wxPen blkPen(wxColour(0,0,0));
	dc.SetBrush(whtBrush);
	dc.SetPen(blkPen);
    dc.DrawRectangle(0, 0, sz.GetWidth()-1, sz.GetHeight()-1);
#endif

	int cx = sz.GetWidth();
	int cy = sz.GetHeight();
	cx -= 2;
	cy -= 2;
	AmpValue *ampvals;
	if (index >= 0 && index <= wtSet.wavTblMax
	 && (ampvals = wtSet.GetWavetable(wtSet.FindWavetable(index))) != 0
	 && cx > 0 && cy > 0)
	{
		double inc = (double) synthParams.ftableLength / (double) cx;
		double tn = inc;
		double mid = (double) cy / 2.0;
		double yo = 1.0 + mid;
		double x1 = 1.0;
		double x2 = x1;
		double y1 = yo - (ampvals[0] * mid);
		double y2;
#if wxUSE_GRAPHICS_CONTEXT
		gr->StrokeLine(x1, yo, x1 + (double) cx, yo);
#else
        dc.DrawLine((wxCoord)x1, (wxCoord)yo, (wxCoord)x1 + cx, (wxCoord)yo);
#endif
		for (int x = 1; x < cx; x++)
		{
			x2 = x1 + 1;
			y2 = yo - (ampvals[(int)tn] * mid);
#if wxUSE_GRAPHICS_CONTEXT
			gr->StrokeLine(x1, y1, x2, y2);
#else
            dc.DrawLine((wxCoord)x1, (wxCoord)y1, (wxCoord)x2, (wxCoord)y2);
#endif
			y1 = y2;
			x1 = x2;
			tn += inc;
		}
		y2 = yo - (ampvals[0] * mid);
#if wxUSE_GRAPHICS_CONTEXT
		gr->StrokeLine(x1, y1, x1+1, y2);
#else
        dc.DrawLine((wxCoord)x1, (wxCoord)y1, (wxCoord)x1 + 1, (wxCoord)y2);
#endif
	}
#if wxUSE_GRAPHICS_CONTEXT
	delete gr;
#endif
}

int SynthEdit::SelectWavetable(int wt)
{
	WavetableSelectDlg dlg(::wxGetActiveWindow(), wt);
	if (dlg.ShowModal())
		wt = dlg.GetIndex();
	return wt;
}
