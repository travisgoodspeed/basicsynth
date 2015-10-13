//////////////////////////////////////////////////////////////////////
/// @file KeyboardDlg.cpp Keyboard form
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include <KeyboardWidget.h>
#include "KeyboardDlg.h"
#include <wx/clipbrd.h>

#if wxUSE_GRAPHICS_CONTEXT
#define NO_KEY_BITMAPS 0
#else
#define NO_KEY_BITMAPS 1
#endif

void KeyboardWidget::CreateBitmaps()
{
#if !NO_KEY_BITMAPS
	DeleteBitmaps();

	wxMemoryDC *mdc;
	wxBitmap *bm;

	wxBrush whtUp(wxColour(Red(kclr[0]),Green(kclr[0]),Blue(kclr[0])));
	wxBrush whtDn(wxColour(Red(kclr[1]),Green(kclr[1]),Blue(kclr[1])));
	wxBrush blkUp(wxColour(Red(kclr[2]),Green(kclr[2]),Blue(kclr[2])));
	wxBrush blkDn(wxColour(Red(kclr[3]),Green(kclr[3]),Blue(kclr[3])));
	wxPen olPen(wxColour(0,0,0));

	int cx = rcWhite[0].Width();
	int cy = rcWhite[0].Height();

	bm = new wxBitmap(cx, cy);
	bmKey[0] = (void*) bm;
	mdc = new wxMemoryDC(*bm);
	mdc->SetPen(olPen);
	mdc->SetBrush(whtUp);
	mdc->DrawRectangle(0, 0, cx, cy);

	bm = new wxBitmap(cx, cy);
	bmKey[1] = (void*) bm;
	mdc->SelectObject(*bm);
	mdc->SetBrush(whtDn);
	mdc->DrawRectangle(0, 0, cx, cy);

	cx = rcBlack[0].Width();
	cy = rcBlack[0].Height();

	bm = new wxBitmap(cx, cy);
	bmKey[2] = (void*) bm;
	mdc->SelectObject(*bm);
	mdc->SetBrush(blkUp);
	mdc->DrawRectangle(0, 0, cx, cy);

	bm = new wxBitmap(cx, cy);
	bmKey[3] = (void*) bm;
	mdc->SelectObject(*bm);
	mdc->SetBrush(blkDn);
	mdc->DrawRectangle(0, 0, cx, cy);

	delete mdc;
#endif
}

void KeyboardWidget::DeleteBitmaps()
{
#if !NO_KEY_BITMAPS
	for (int n = 0; n < 4; n++)
	{
		if (bmKey[n])
		{
			delete (wxBitmap*)bmKey[n];
			bmKey[n] = 0;
		}
	}
#endif
}

// Draw the virtual keyboard
void KeyboardWidget::Paint(DrawContext dc)
{
	if (!rcWhite || !rcBlack)
		return;

	int all = 0;
	if (upd.IsEmpty())
		all = 1;

#if wxUSE_GRAPHICS_CONTEXT
	wxGraphicsContext *gr = (wxGraphicsContext*)dc;
	if (!all)
		gr->Clip((wxDouble)upd.x, (wxDouble)upd.y, (wxDouble)upd.w, (wxDouble)upd.h);
#else
	wxDC *gr = (wxDC *)dc;
	if (!all)
		gr->SetClippingRegion(upd.x, upd.y, upd.w, upd.h);
#endif


#if NO_KEY_BITMAPS
	wxPen blkPn(wxColour(0,0,0));
	gr->SetPen(blkPn);
	wxBrush whtUp(wxColour(Red(kclr[0]),Green(kclr[0]),Blue(kclr[0])));
	wxBrush whtDn(wxColour(Red(kclr[1]),Green(kclr[1]),Blue(kclr[1])));
	wxBrush blkUp(wxColour(Red(kclr[2]),Green(kclr[2]),Blue(kclr[2])));
	wxBrush blkDn(wxColour(Red(kclr[3]),Green(kclr[3]),Blue(kclr[3])));
	gr->SetBrush(whtUp);
	wdgRect *rp = rcWhite;
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (all || rp->Intersects(upd))
		{
			if (rp == rcLastKey)
			{
				gr->SetBrush(whtDn);
				gr->DrawRectangle(rp->x, rp->y, rp->w, rp->h);
				gr->SetBrush(whtUp);
			}
			else
				gr->DrawRectangle(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}

	gr->SetBrush(blkUp);
	rp = rcBlack;
	for (i = 0; i < blkKeys; i++)
	{
		if (all || rp->Intersects(upd))
		{
			if (rp == rcLastKey)
			{
				gr->SetBrush(blkDn);
				gr->DrawRectangle(rp->x, rp->y, rp->w, rp->h);
				gr->SetBrush(blkUp);
			}
			else
				gr->DrawRectangle(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}
#else
	wdgRect *rp = rcWhite;
	wxDouble cx = rp->Width();
	wxDouble cy = rp->Height();
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (all || rp->Intersects(upd))
		{
			wxBitmap *bm;
			if (rp == rcLastKey)
				bm = (wxBitmap*)bmKey[1];
			else
				bm = (wxBitmap*)bmKey[0];
			gr->DrawBitmap(*bm, (wxDouble)rp->x, (wxDouble)rp->y, cx, cy);
		}
		rp++;
	}
	rp = rcBlack;
	cx = rp->Width();
	cy = rp->Height();
	for (i = 0; i < blkKeys; i++)
	{
		if (all || rp->Intersects(upd))
		{
			wxBitmap *bm;
			if (rp == rcLastKey)
				bm = (wxBitmap*)bmKey[3];
			else
				bm = (wxBitmap*)bmKey[2];
			gr->DrawBitmap(*bm, (wxDouble)rp->x, (wxDouble)rp->y, cx, cy);
		}
		rp++;
	}
#endif
	upd.SetEmpty();
	if (!all)
#if wxUSE_GRAPHICS_CONTEXT
		gr->ResetClip();
#else
		gr->DestroyClippingRegion();
#endif
}


// Copy the accumulated notes to the clipboard.
// TODO: This should be moved to ProjectFrame
void KeyboardWidget::CopyToClipboard(bsString& str)
{
#if wxUSE_CLIPBOARD &&  wxMAJOR_VERSION >= 2 && wxMINOR_VERSION >= 9
	if (str.Length() > 0)
	{
		wxClipboard *clip = wxTheClipboard;
		if (!clip->IsOpened() && clip->Open())
		{
			wxString data((const char *)str);
			clip->Clear();
			clip->SetData(new wxTextDataObject(data));
			clip->Close();
		}
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(KeyboardDlg,wxWindow)
	EVT_PAINT(KeyboardDlg::OnPaint)
	EVT_ERASE_BACKGROUND(KeyboardDlg::OnErase)
	EVT_LEFT_DCLICK(KeyboardDlg::OnBtnDown)
	EVT_LEFT_DOWN(KeyboardDlg::OnBtnDown)
	EVT_LEFT_UP(KeyboardDlg::OnBtnUp)
	EVT_MOTION(KeyboardDlg::OnMouseMove)
	EVT_LISTBOX(101, KeyboardDlg::OnInstrChange)
END_EVENT_TABLE()

wxBitmap *KeyboardDlg::offscrn;
wxRect KeyboardDlg::offsSize;

KeyboardDlg::KeyboardDlg(wxWindow *parent, int w, int h) :
	wxWindow(parent, wxWindowID(ID_KEYBOARD_WND), wxPoint(0,0), wxSize(w,h), wxBORDER_SIMPLE),
	bgColor(0x80,0x80,0x80)
{
	form = 0;
	instrList = new wxListBox(this, 101, wxPoint(5, 5), wxSize(190, 180));
}

KeyboardDlg::~KeyboardDlg()
{
	delete form;
}

// Load the keyboard form
void KeyboardDlg::Load()
{
	if (!theProject) // "cannot happen?"
		return;

	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.Set((clr>>16)&0xff,(clr>>8)&0xff,clr&0xff,(clr>>24)&0xff);
	bsString fileName;
	if (!theProject->FindForm(fileName, "KeyboardEd.xml"))
	{
		prjFrame->Alert("Could not locate keyboard KeyboardEd.xml!", "Huh?");
	}
	else
	{
		form = new KeyboardForm();
		form->SetFormEditor(this);
		wdgRect a(5,5,190,180);
		if (form->Load(fileName, 0, 0) == 0 && form->GetKeyboard())
		{
			// Attach the instrument listbox to the form
			SynthWidget *wdg = form->GetInstrList();
			if (wdg)
			{
				a = wdg->GetArea();
				wdg->Remove();
				delete wdg;
			}
			instrList->SetSize(a.x, a.y, a.w, a.h);
			instrList->Show(true);
			if (instrList->GetCount() > 0)
			{
				instrList->SetSelection(0);
				InstrConfig *ic = (InstrConfig*)instrList->GetClientData(0);
				form->GetKeyboard()->SetInstrument(ic);
				if (ic)
					theProject->prjMidiIn.SetInstrument(ic->inum);
			}
			int cx = 200;
			int cy = 100;
			form->GetSize(cx, cy);
			cx += 10;
			cy += 10;
			SetSize(cx, cy);
		}
		else
		{
			bsString msg;
			msg = "Could not load keyboard form: ";
			msg += fileName;
			prjFrame->Alert(msg, "Huh?");
			delete form;
			form = 0;
		}
	}
}

// insure off-screen buffer is big enough
int KeyboardDlg::CheckBuffer(int w, int h)
{
	if (!offscrn || w > offsSize.GetWidth() || h > offsSize.GetHeight())
	{
		// grow the frame buffer
		if (offscrn)
			delete offscrn;
		if (w > offsSize.GetWidth())
			offsSize.SetWidth(w);
		if (h > offsSize.GetHeight())
			offsSize.SetHeight(h);
		offscrn = new wxBitmap(offsSize.GetWidth(), offsSize.GetHeight());
	}
	return offscrn != NULL;
}

// handle paint - redraw the whole form
void KeyboardDlg::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);

	int winWidth;
	int winHeight;
	GetClientSize(&winWidth, &winHeight);
	wxBrush br(bgColor);
	wxPen pn(bgColor);

	if (form)
	{
		wxMemoryDC *mdc = 0;
		int formWidth;
		int formHeight;
		form->GetSize(formWidth, formHeight);
		int bmWidth = (formWidth > winWidth) ? formWidth : winWidth;
		int bmHeight = (formHeight > winHeight) ? formHeight : winHeight;

		if (!CheckBuffer(bmWidth, bmHeight))
		{
			dc.SetBrush(br);
			dc.SetPen(pn);
			dc.DrawRectangle(0, 0, winWidth, winHeight);
#if wxUSE_GRAPHICS_CONTEXT
			wxGraphicsContext *gr = wxGraphicsContext::Create((wxWindowDC&)dc);
			form->RedrawForm((DrawContext)gr);
			delete gr;
#else
			form->RedrawForm((DrawContext)&dc);
#endif
		}
		else
		{
			mdc = new wxMemoryDC(*offscrn);
#if wxUSE_GRAPHICS_CONTEXT
			wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);
			gr->SetBrush(br);
			gr->SetPen(pn);
			gr->DrawRectangle(0, 0, bmWidth, bmHeight);
			form->RedrawForm((DrawContext)gr);
			delete gr;
#else
			mdc->SetBrush(br);
			mdc->SetPen(pn);
			mdc->DrawRectangle(0, 0, bmWidth, bmHeight);
			form->RedrawForm((DrawContext)mdc);
#endif
			//dc.Blit(0, 0, bmWidth, bmHeight, mdc, 0, 0);
			dc.Blit(0, 0, winWidth, winHeight, mdc, 0, 0);
			delete mdc;
		}
	}
	else
	{
		dc.SetBrush(br);
		dc.SetPen(pn);
		dc.DrawRectangle(0, 0, winWidth, winHeight);
	}
}

void KeyboardDlg::Redraw(SynthWidget *wdg)
{
	if (wdg)
	{
		SynthWidget *bud = wdg->GetBuddy2();

		wxClientDC dc(this);
		if (!offscrn)
		{
#if wxUSE_GRAPHICS_CONTEXT
			wxGraphicsContext *gr = wxGraphicsContext::Create((wxWindowDC&)dc);
			wdg->Paint(gr);
			if (bud)
				bud->Paint(gr);
			delete gr;
#else
			wdg->Paint(&dc);
			if (bud)
				bud->Paint(&dc);
#endif
		}
		else
		{
			wxMemoryDC *mdc = new wxMemoryDC(*offscrn);
			//wxRect rc = GetRect();
			wdgRect a = wdg->GetArea();
#if wxUSE_GRAPHICS_CONTEXT
			wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);
			wdg->Paint(gr);
			dc.Blit(a.x, a.y, a.w, a.h, mdc, a.x, a.y);
			if (bud)
			{
				bud->Paint(gr);
				a = bud->GetArea();
				dc.Blit(a.x, a.y, a.w, a.h, mdc, a.x, a.y);
			}
			delete gr;
#else
			wdg->Paint(mdc);
			dc.Blit(a.x, a.y, a.w, a.h, mdc, a.x, a.y);
			if (bud)
			{
				bud->Paint(mdc);
				a = bud->GetArea();
				dc.Blit(a.x, a.y, a.w, a.h, mdc, a.x, a.y);
			}
#endif
			delete mdc;
		}
	}
	else
		Refresh();
}

void KeyboardDlg::OnErase(wxEraseEvent& evt)
{
	// paint will completely redraw the background.
}

void KeyboardDlg::OnBtnDown(wxMouseEvent& evt)
{
	if (form)
		form->BtnDn((int)evt.GetX(), (int)evt.GetY(), evt.ControlDown(), evt.ShiftDown());
}

void KeyboardDlg::OnBtnUp(wxMouseEvent& evt)
{
	if (form)
		form->BtnUp((int)evt.GetX(), (int)evt.GetY(), evt.ControlDown(), evt.ShiftDown());
}

void KeyboardDlg::OnMouseMove(wxMouseEvent& evt)
{
	if (form)
		form->MouseMove((int)evt.GetX(), (int)evt.GetY(), evt.ControlDown(), evt.ShiftDown());
}

void KeyboardDlg::OnInstrChange(wxCommandEvent& evt)
{
	InstrConfig *ic = 0;
	int sel = instrList->GetSelection();
	if (sel != wxNOT_FOUND)
		ic = (InstrConfig*)instrList->GetClientData(sel);
	if (form)
		form->GetKeyboard()->SetInstrument(ic);
	if (ic && theProject)
		theProject->prjMidiIn.SetInstrument(ic->inum);
}

int KeyboardDlg::IsRunning()
{
	if (theProject)
		return theProject->IsPlaying();
	return 0;
}

int KeyboardDlg::Stop()
{
	if (form)
		return form->Stop();
	return 0;
}

int KeyboardDlg::Start()
{
	if (form)
		return form->Start();
	return 0;
}

void KeyboardDlg::Clear()
{
	if (form)
	{
		form->Stop();
		form->GetKeyboard()->SetInstrument(0);
	}
	if (theProject)
		theProject->prjMidiIn.SetInstrument(0);
	instrList->Clear();
}

void KeyboardDlg::InitInstrList()
{
	instrList->Clear();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList->GetCount() > 0)
	{
		instrList->SetSelection(0);
		ic = (InstrConfig*)instrList->GetClientData(0);
		if (form)
			form->GetKeyboard()->SetInstrument(ic);
		if (ic)
			theProject->prjMidiIn.SetInstrument(ic->inum);
	}
}

int KeyboardDlg::AddInstrument(InstrConfig *ic)
{
	const char *np = ic->GetName();
	if (*np == '[')
	{
		// internal control instrument - ignore it
		return wxNOT_FOUND;
	}
	bsString fnbuf;
	if (!np || *np == 0)
	{
		fnbuf = "#";
		fnbuf.Append((long)ic->inum);
		np = (const char*)fnbuf;
	}
	return instrList->Append(wxString(np, cutf8), (void*)ic);
}

int KeyboardDlg::FindInstrument(InstrConfig *ic)
{
	InstrConfig *ic2;
	int count = instrList->GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList->GetClientData(ndx);
		if (ic == ic2)
			return ndx;
		ndx++;
	}
	return wxNOT_FOUND;
}

int KeyboardDlg::SelectInstrument(InstrConfig *ic)
{
	int ndx = FindInstrument(ic);
	if (ndx != wxNOT_FOUND)
	{
		instrList->SetSelection(ndx);
		if (form)
			form->GetKeyboard()->SetInstrument(ic);
		if (ic && theProject)
			theProject->prjMidiIn.SetInstrument(ic->inum);
	}
	return ndx;
}

int KeyboardDlg::RemoveInstrument(InstrConfig *ic)
{
	if (form)
		form->GetKeyboard()->SetInstrument(0);

	int sel = instrList->GetSelection();
	int ndx = FindInstrument(ic);
	if (ndx != wxNOT_FOUND)
	{
		instrList->Delete(ndx);
		if (ndx == sel)
		{
			int count = instrList->GetCount();
			if (count == ndx)
				ndx--;
			instrList->SetSelection(ndx);
		}
	}

	return ndx;
}

// this is called if the instrument name gets changed.
int KeyboardDlg::UpdateInstrument(InstrConfig *ic)
{
	int sel = FindInstrument(ic);
	if (sel != wxNOT_FOUND)
		instrList->SetString(sel, wxString(ic->GetName(), cutf8));
	return sel;
}

void KeyboardDlg::UpdateChannels()
{
}
