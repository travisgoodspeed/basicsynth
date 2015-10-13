//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "FormEditor.h"
#include <wx/dcbuffer.h>

wxBitmap *FormEditorWin::offscrn;
wxRect FormEditorWin::offsSize;

BEGIN_EVENT_TABLE(FormEditorWin,wxWindow)
	EVT_PAINT(FormEditorWin::OnPaint)
	EVT_SIZE(FormEditorWin::OnSize)
	EVT_SCROLLWIN(FormEditorWin::OnScroll)
	EVT_ERASE_BACKGROUND(FormEditorWin::OnEraseBackground)
	EVT_LEFT_DCLICK(FormEditorWin::OnBtnDown)
	EVT_LEFT_DOWN(FormEditorWin::OnBtnDown)
	EVT_LEFT_UP(FormEditorWin::OnBtnUp)
	EVT_MOTION(FormEditorWin::OnMouseMove)
	EVT_SET_FOCUS(FormEditorWin::OnSetFocus)
	EVT_KILL_FOCUS(FormEditorWin::OnKillFocus)
//	EVT_COMMAND_RANGE(0, 0xffff, event, FormEditorWin::OnCommand)
END_EVENT_TABLE()

FormEditorWin::FormEditorWin(wxWindow *parent)
	: wxWindow(parent, 101, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL|wxBORDER_SIMPLE)
{
	item = 0;
	form = 0;
//	fgColor.InitRGBA(0,0,0,wxALPHA_OPAQUE);
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.Set((clr >> 16) & 0xFF, (clr >> 8) & 0xFF, clr & 0xFF, (clr >> 24) & 0xFF);
}

FormEditorWin::~FormEditorWin(void)
{
	delete form;
}

void FormEditorWin::OnEraseBackground(wxEraseEvent& evt)
{
	// do nothing - prevents flicker
}

void FormEditorWin::OnSize(wxSizeEvent& evt)
{
	UpdateScrollBars();
}

void FormEditorWin::UpdateScrollBars()
{
	int scrollWidth = 0;
	int scrollHeight = 0;
	int formWidth = 0;
	int formHeight = 0;

	GetClientSize(&scrollWidth, &scrollHeight);
	if (form)
	{
		form->GetSize(formWidth, formHeight);
	}

//	::wxLogDebug("SetScroll: Width(%d,%d) Height(%d,%d)\r\n", scrollWidth, formWidth, scrollHeight, formHeight);
	SetScrollbar(wxHORIZONTAL, 0, scrollWidth, formWidth);
	SetScrollbar(wxVERTICAL, 0, scrollHeight, formHeight);
}

void FormEditorWin::OnScroll(wxScrollWinEvent& evt)
{
	int newpos;
	int orientation = evt.GetOrientation();
	int maxpos = GetScrollRange(orientation) - GetScrollThumb(orientation);
	int oldpos = GetScrollPos(orientation);
	wxEventType type = evt.GetEventType();
	if (type == wxEVT_SCROLLWIN_BOTTOM)
		newpos = maxpos;
	else if (type == wxEVT_SCROLLWIN_TOP)
		newpos = 0;
	else if (type == wxEVT_SCROLLWIN_LINEUP)
		newpos = oldpos - 1;
	else if (type == wxEVT_SCROLLWIN_LINEDOWN)
		newpos = oldpos + 1;
	else if (type == wxEVT_SCROLLWIN_PAGEUP)
		newpos = 0;
	else if (type == wxEVT_SCROLLWIN_PAGEDOWN)
		newpos = maxpos;
	else if (type == wxEVT_SCROLLWIN_THUMBTRACK
	 || type == wxEVT_SCROLLWIN_THUMBRELEASE)
		newpos = evt.GetPosition();
	else
		return;
	if (newpos < 0)
		newpos = 0;
	else if (newpos > maxpos)
		newpos = maxpos;
//	::wxLogDebug("Scroll: %d -> %d\r\n", oldpos, newpos);
#ifdef __WXMSW__
	if (newpos != oldpos)
#endif
	{
		SetScrollPos(orientation, newpos);
		Refresh();
	}
}

void FormEditorWin::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	if (form)
	{
		int xoffs = GetScrollPos(wxHORIZONTAL);
		int yoffs = GetScrollPos(wxVERTICAL);

		wxMemoryDC *mdc = 0;
		int formWidth = 0;
		int formHeight = 0;
		form->GetSize(formWidth, formHeight);

		int winWidth = 0;
		int winHeight = 0;
		GetClientSize(&winWidth, &winHeight);
		int bmWidth = (formWidth > winWidth) ? formWidth : winWidth;
		int bmHeight = (formHeight > winHeight) ? formHeight : winHeight;
		bmWidth += xoffs;
		bmHeight += yoffs;

		wxBrush br(bgColor);
		wxPen pn(bgColor);
//		if (IsDoubleBuffered() || !CheckBuffer(bmWidth, bmHeight))
		if (!CheckBuffer(bmWidth, bmHeight))
		{
			dc.SetBrush(br);
			dc.SetPen(pn);
			dc.DrawRectangle(0, 0, winWidth, winHeight);
			dc.SetDeviceOrigin(-xoffs, -yoffs);
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
			form->RedrawForm(mdc);
#endif
			delete mdc;
			dc.DrawBitmap(*offscrn, -xoffs, -yoffs);
		}
	}
}


void FormEditorWin::Redraw(SynthWidget *wdg)
{
	if (wdg)
	{
		SynthWidget *bud = wdg->GetBuddy2();

		wxClientDC dc(this);
		int xoffs = GetScrollPos(wxHORIZONTAL);
		int yoffs = GetScrollPos(wxVERTICAL);
		dc.SetDeviceOrigin(-xoffs, -yoffs);
//		if (IsDoubleBuffered() || !offscrn)
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


void FormEditorWin::OnSetFocus(wxFocusEvent& evt)
{
	if (form)
		form->SetFocus();
}

void FormEditorWin::OnKillFocus(wxFocusEvent& evt)
{
	if (form)
		form->KillFocus();
}

void FormEditorWin::OnBtnDown(wxMouseEvent& evt)
{
	if (form)
	{
		int xoffs = GetScrollPos(wxHORIZONTAL);
		int yoffs = GetScrollPos(wxVERTICAL);
		form->BtnDn((int)evt.GetX()+xoffs, (int)evt.GetY()+yoffs, evt.ControlDown(), evt.ShiftDown());
	}
}

void FormEditorWin::OnBtnUp(wxMouseEvent& evt)
{
	if (form)
	{
		int xoffs = GetScrollPos(wxHORIZONTAL);
		int yoffs = GetScrollPos(wxVERTICAL);
		form->BtnUp((int)evt.GetX()+xoffs, (int)evt.GetY()+yoffs, evt.ControlDown(), evt.ShiftDown());
	}
}

void FormEditorWin::OnMouseMove(wxMouseEvent& evt)
{
	if (form)
	{
		int xoffs = GetScrollPos(wxHORIZONTAL);
		int yoffs = GetScrollPos(wxVERTICAL);
		form->MouseMove((int)evt.GetX()+xoffs, (int)evt.GetY()+yoffs, evt.ControlDown(), evt.ShiftDown());
	}
}

void FormEditorWin::OnCommand(wxCommandEvent& evt)
{
//	if (form)
//	{
//		if (lParam)
//			form->Command(LOWORD(wParam), HIWORD(wParam));
//	}
}

SynthWidget *FormEditorWin::SystemWidget(const char *type)
{
	// TODO: if this is a windows control, create the control
	// and a SynthWidget-derived wrapper object.
	return NULL;
}
void FormEditorWin::Resize()
{
	UpdateScrollBars();
	Refresh();
}

long FormEditorWin::EditState()
{
	long state = 0;
	if (form)
	{
		state = VW_ENABLE_FILE;
		if (form->CanCopy())
			state |= VW_ENABLE_COPY|VW_ENABLE_CUT;
		if (form->CanPaste())
			state |= VW_ENABLE_PASTE;
		if (form->IsChanged())
			state |= VW_ENABLE_UNDO;
	}
	return state;
}

void FormEditorWin::CopyToClipboard()
{
	bsString text;
	if (form)
	{
		form->CopyText(text);
		size_t len = text.Length();
		if (len > 0)
		{
			//
		}
	}
}

void FormEditorWin::PasteFromClipboard()
{
	if (form)
	{
		//form->PasteText(valbuf);
	}
}

int FormEditorWin::CheckBuffer(int w, int h)
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
