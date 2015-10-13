//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file WvtableSelectDlg.cpp Wavetable selection dialog implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "WvtableSelectDlg.h"
#include <ModSynthEd.h>

static void SelectCB(Fl_Widget *wdg, void *arg)
{
	((WvtableSelectDlg*)arg)->OnSelect();
}

static void OkCB(Fl_Widget *wdg, void *arg)
{
	((WvtableSelectDlg*)arg)->OnOK();
}

static void CancelCB(Fl_Widget *wdg, void *arg)
{
	((WvtableSelectDlg*)arg)->OnCancel();
}

WvtableSelectDlg::WvtableSelectDlg()
	: Fl_Window(100, 100, 500, 300, "Select Wavetable")
{
	done = 0;
	lst = new Fl_Hold_Browser(5, 5, 150, 250);
	lst->callback(SelectCB, (void *)this);
	plotter = new WavetableDraw(160, 5, 330, 250);
	okbtn = new Fl_Button(5, 260, 90, 25, "OK");
	okbtn->callback(OkCB, (void *)this);
	canbtn = new Fl_Button(100, 260, 90, 25, "Cancel");
	canbtn->callback(CancelCB, (void *)this);

	lst->add("Sin", (void*)0);
	lst->add("Sawtooth (sum)", (void*)1);
	lst->add("Square (sum)", (void*)2);
	lst->add("Triangle (sum)", (void*)3);
	lst->add("Pulse (sum)", (void*)4);
	lst->add("Sawtooth (direct)", (void*)5);
	lst->add("Square (direct)", (void*)6);
	lst->add("Triangle (direct)", (void*)7);
	lst->add("Sawtooth (positive)", (void*)7);
	lst->add("Triangle (positive)", (void*)9);

	ProjectItem *pi = prjTree->FirstChild(theProject->synthInfo);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVTABLE)
		{
			WavetableItem *wi = (WavetableItem*)pi;
			lst->add(wi->GetName(), (void*) wi->GetID());
		}
		pi = prjTree->NextSibling(pi);
	}
	end();
	resizable(0);
	position((Fl::w() - w()) / 2, (Fl::h() - h()) / 2);
}

WvtableSelectDlg::~WvtableSelectDlg()
{
}

void WvtableSelectDlg::OnSelect()
{
	int index = lst->value()-1;
	if (index >= 0)
	{
		plotter->index = (int)(long) lst->data(index);
		plotter->redraw();
	}
}

void WvtableSelectDlg::OnOK()
{
	done = 1;
}

void WvtableSelectDlg::OnCancel()
{
	done = 2;
}

int WvtableSelectDlg::SelectWaveform(int index)
{
	lst->value(wtSet.FindWavetable(index)+1);
	plotter->index = index;
	done = 0;
	set_modal();
	show();
	while (!done)
		Fl::wait();
	return plotter->index;
}

int SynthEdit::SelectWavetable(int wt)
{
	WvtableSelectDlg *dlg = new WvtableSelectDlg;
	wt = dlg->SelectWaveform(wt);
	delete dlg;
	return wt;
}

void ModSynthEdit::OnConfig()
{
}

void ModSynthEdit::OnConnect()
{
}

WavetableDraw::WavetableDraw(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H, 0)
{
	box(FL_DOWN_BOX);
	index = 0;
}

void WavetableDraw::draw()
{
	draw_box();

	AmpValue *ampvals;
	int cx = w() - 8;
	int cy = h() - 8;
	if (index >= 0 && index <= wtSet.wavTblMax
	 && (ampvals = wtSet.GetWavetable(wtSet.FindWavetable(index))) != 0
	 && cx > 0 && cy > 0)
	{
		double inc = synthParams.ftableLength / (float) cx;
		double tn = 0;
		double mid = (double) cy / 2.0;
		double xo = (double) (x() + 4);
		double yo = (double) (y() + 4) + mid;
		fl_color(128,128,128);
		fl_line((int)xo, (int)yo, (int)xo + cx, (int)yo);
		fl_color(0,0,0);
		fl_begin_line();
		for (int x = 0; x < cx; x++)
		{
			fl_vertex(xo, yo - (ampvals[(int)tn] * mid));
			tn += inc;
			xo += 1.0;
		}
		fl_vertex(xo, yo - (ampvals[0] * mid));
		fl_end_line();
	}
}

