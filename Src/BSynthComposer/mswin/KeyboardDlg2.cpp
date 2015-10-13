//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "KeyboardDlg2.h"
//#include "GenerateDlg.h"

void KeyboardWidget::CreateBitmaps()
{
	DeleteBitmaps();

	Graphics *gr;

	int cx = rcWhite[0].Width();
	int cy = rcWhite[0].Height();

	bmKey[0] = new Bitmap(cx, cy);
	bmKey[1] = new Bitmap(cx, cy);

	RectF wr(0, 0, cx, cy);
	LinearGradientBrush whup(wr, Color(kclr[0]), Color(32,32,32), 0.0, FALSE);
	LinearGradientBrush whdn(wr, Color(kclr[1]), Color(8,8,8), 0.0, FALSE);
	REAL fact[4] = { 1.0f, 0.0f, 0.0f, 1.0f, };
	REAL pos[4] = { 0.0f, 0.1f, 0.9f, 1.0f };
	whup.SetBlend(fact, pos, 4);
	whdn.SetBlend(fact, pos, 4);

	gr = Graphics::FromImage((Bitmap*)bmKey[0]);
	gr->FillRectangle(&whup, 0, 0, cx, cy);
	delete gr;

	gr = Graphics::FromImage((Bitmap*)bmKey[1]);
	gr->FillRectangle(&whdn, 0, 0, cx, cy);
	delete gr;

	cx = rcBlack[0].Width();
	cy = rcBlack[0].Height();
	bmKey[2] = new Bitmap(cx, cy);
	bmKey[3] = new Bitmap(cx, cy);

	SolidBrush blkup(kclr[2]);
	SolidBrush blkdn(kclr[3]);

	gr = Graphics::FromImage((Bitmap*)bmKey[2]);
	gr->FillRectangle(&blkup, 0, 0, cx, cy);
	delete gr;

	gr = Graphics::FromImage((Bitmap*)bmKey[3]);
	gr->FillRectangle(&blkdn, 0, 0, cx, cy);
	delete gr;
}

void KeyboardWidget::DeleteBitmaps()
{
	for (int n = 0; n < 4; n++)
	{
		if (bmKey[n])
		{
			delete (Bitmap *)bmKey[n];
			bmKey[n] = 0;
		}
	}
}

void KeyboardWidget::Paint(DrawContext dc)
{
	if (!rcWhite || !rcBlack)
		return;

	Graphics *gr = (Graphics*)dc;
	Region clipRgn;
	gr->GetClip(&clipRgn);
	int all = 0;
	if (upd.IsEmpty())
		all = 1;
	else
	{
		Rect rc(upd.x, upd.y, upd.w, upd.h);
		gr->SetClip(rc);
	}

#ifdef NO_KEY_BITMAPS
	// I'm not sure why, but every now and then there is a memory exception
	// deep, deep down in GDI+. For now, we just trap the exception and blaze
	// onward. The worst case is part of the screen is not redrawn.
	try
	{
		//SolidBrush bk(Color(bgColor));
		//gr->Clear(Color(92,92,64));
		RectF wr(rcWhite[0].x, rcWhite[0].y, rcWhite[0].w, rcWhite[0].h);
		LinearGradientBrush lgw(wr, Color(255, 255, 255), Color(32,32,32), 0.0, FALSE);
		LinearGradientBrush lgk(wr, Color(220, 220, 220), Color(8,8,8), 0.0, FALSE);
		REAL fact[4] = { 1.0f, 0.0f, 0.0f, 1.0f, };
		REAL pos[4] = { 0.0f, 0.1f, 0.9f, 1.0f };
		lgw.SetBlend(fact, pos, 4);
		lgk.SetBlend(fact, pos, 4);
		wdgRect *rp = rcWhite;
		int i;
		for (i = 0; i < whtKeys; i++)
		{
			if (all || rp->Intersects(upd))
			{
				if (rp == rcLastKey)
					gr->FillRectangle(&lgk, rp->x, rp->y, rp->w, rp->h);
				else
					gr->FillRectangle(&lgw, rp->x, rp->y, rp->w, rp->h);
			}
			rp++;
		}

		SolidBrush lgb(Color(8,8,8));
		SolidBrush lgk2(Color(64,64,64));
		rp = rcBlack;
		for (i = 0; i < blkKeys; i++)
		{
			if (all || upd.Intersects(*rp))
			{
				if (rp == rcLastKey)
					gr->FillRectangle(&lgk2, rp->x, rp->y, rp->w, rp->h);
				else
					gr->FillRectangle(&lgb, rp->x, rp->y, rp->w, rp->h);
			}
			rp++;
		}
	}
	catch(...)
	{
		OutputDebugString("Funky exception in GDI+...\r\n");
	}
#else
	wdgRect *rp = rcWhite;
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (all || rp->Intersects(upd))
		{
			if (rp == rcLastKey)
				gr->DrawImage((Bitmap*)bmKey[1], rp->x, rp->y);
			else
				gr->DrawImage((Bitmap*)bmKey[0], rp->x, rp->y);
		}
		rp++;
	}
	rp = rcBlack;
	for (i = 0; i < blkKeys; i++)
	{
		if (all || upd.Intersects(*rp))
		{
			if (rp == rcLastKey)
				gr->DrawImage((Bitmap*)bmKey[3], rp->x, rp->y);
			else
				gr->DrawImage((Bitmap*)bmKey[2], rp->x, rp->y);
		}
		rp++;
	}

#endif
	Pen pn(Color(0,0,0));
	gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);

	upd.SetEmpty();
	gr->SetClip(&clipRgn);
}

// TODO: This should be moved to ProjectFrame
void KeyboardWidget::CopyToClipboard(bsString& str)
{
	OpenClipboard(_Module.mainWnd);
	EmptyClipboard();
	size_t len = str.Length() + 1;
	if (len > 1)
	{
		HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, len);
		if (h)
		{
			char *p2 = (char *) GlobalLock(h);
			strncpy(p2, (const char *)str, len);
			GlobalUnlock(h);
			SetClipboardData(CF_TEXT, h);
		} 
	}
	CloseClipboard();
}

///////////////////////////////////////////////////////////////////////////////

KeyboardDlg2::KeyboardDlg2()
{
	form = 0;
	bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
}

KeyboardDlg2::~KeyboardDlg2()
{
	delete form;
	DeleteObject(listFont);
}

void KeyboardDlg2::Load()
{
	if (!theProject) // "cannot happen?"
		return;

	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.SetValue((ARGB)clr);
	bsString fileName;
	if (!theProject->FindForm(fileName, "KeyboardEd.xml"))
	{
		prjFrame->Alert("Could not locate keyboard KeyboardEd.xml!", "Huh?");
	}
	else
	{
		form = new KeyboardForm();
		form->SetFormEditor(this);
		if (form->Load(fileName, 0, 0) == 0 && form->GetKeyboard())
		{
			int cx = 200;
			int cy = 100;
			form->GetSize(cx, cy);
			wdgRect a(5, 5, 150, cy-10);
			SynthWidget *wdg = form->GetInstrList();
			if (wdg)
			{
				a = wdg->GetArea();
				wdg->Remove();
				delete wdg;
			}
			instrList.SetWindowPos(NULL, a.x, a.y, a.w, a.h, SWP_NOZORDER|SWP_NOACTIVATE);
			instrList.ShowWindow(SW_SHOW);
			if (instrList.GetCount() > 0)
			{
				instrList.SetCurSel(0);
				InstrConfig *ic = (InstrConfig*)instrList.GetItemDataPtr(0);
				form->GetKeyboard()->SetInstrument(ic);
				if (ic)
					theProject->prjMidiIn.SetInstrument(ic->inum);
			}
			cx += 10;
			cy += 10;
			SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
		else
		{
			bsString msg;
			msg = "Could not load keyboard form: ";
			msg += fileName;
			prjFrame->Alert(msg, "Oooops...");
			delete form;
			form = 0;
		}
	}
}

LRESULT KeyboardDlg2::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: get the height of the standard font.
	listFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
		DEFAULT_CHARSET,  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, FF_DONTCARE|DEFAULT_PITCH , "Tahoma");
	RECT rcList;
	rcList.left = 5;
	rcList.top = 5;
	rcList.right = 190;
	rcList.bottom = 180;
	instrList.Create(m_hWnd, &rcList, NULL, WS_CHILD|WS_BORDER|LBS_NOTIFY|LBS_SORT|WS_VSCROLL, 0, 101);
	instrList.SetFont(listFont, 0);
	return 0;
}

LRESULT KeyboardDlg2::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	//ATLTRACE("Kbd OnPaint %d,%d,%d,%d\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
	Graphics gr(dc);
	RECT rcWnd;
	GetClientRect(&rcWnd);
	Bitmap *offscrn = new Bitmap(rcWnd.right, rcWnd.bottom);
	Graphics *ctx = Graphics::FromImage(offscrn);
	SolidBrush br(bgColor);
	ctx->Clear(bgColor);
	if (form)
		form->RedrawForm((DrawContext)ctx);
	gr.DrawImage(offscrn, 0, 0);
	delete ctx;
	delete offscrn;

	EndPaint(&ps);
	return 0;
}

LRESULT KeyboardDlg2::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// paint will completely redraw the background.
	return 0;
}

LRESULT KeyboardDlg2::OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->BtnDn((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->BtnUp((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		form->MouseMove((int)x, (int)y, 
			GetKeyState(VK_CONTROL) & 0x8000, 
			GetKeyState(VK_SHIFT) & 0x8000);
	}
	return 0;
}

LRESULT KeyboardDlg2::OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	InstrConfig *ic = 0;
	int sel = instrList.GetCurSel();
	if (sel != CB_ERR)
		ic = (InstrConfig*)instrList.GetItemDataPtr(sel);
	if (form)
		form->GetKeyboard()->SetInstrument(ic);
	if (ic && theProject)
		theProject->prjMidiIn.SetInstrument(ic->inum);
	return 0;
}

int KeyboardDlg2::IsRunning()
{
	if (theProject)
		return theProject->IsPlaying();
	return 0;
}

int KeyboardDlg2::Stop()
{
	if (form)
		return form->Stop();
	return 0;
}

int KeyboardDlg2::Start()
{
	if (form)
		return form->Start();
	return 0;
}

void KeyboardDlg2::Clear()
{
	if (form)
	{
		form->Stop();
		form->GetKeyboard()->SetInstrument(0);
	}
	if (theProject)
		theProject->prjMidiIn.SetInstrument(0);
	instrList.ResetContent();
}

void KeyboardDlg2::InitInstrList()
{
	instrList.ResetContent();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList.GetCount() > 0)
	{
		instrList.SetCurSel(0);
		ic = (InstrConfig*)instrList.GetItemDataPtr(0);
		if (form)
			form->GetKeyboard()->SetInstrument(ic);
		if (ic)
			theProject->prjMidiIn.SetInstrument(ic->inum);
	}
}

int KeyboardDlg2::AddInstrument(InstrConfig *ic)
{
	bsString fnbuf;
	const char *np = ic->GetName();
	if (*np == '[')
	{
		// internal control instrument - ignore it
		return -1;
	}
	if (!np || *np == 0)
	{
		fnbuf = "#";
		fnbuf += (long)ic->inum;
		np = (const char *)fnbuf;
		ic->SetName(np);
	}
	int ndx = instrList.AddStringUTF8(np);
	instrList.SetItemDataPtr(ndx, ic);
	return ndx;
}

int KeyboardDlg2::FindInstrument(InstrConfig *ic)
{
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
			return ndx;
		ndx++;
	}
	return -1;
}

int KeyboardDlg2::SelectInstrument(InstrConfig *ic)
{
	int ndx = FindInstrument(ic);
	if (ndx >= 0)
	{
		instrList.SetCurSel(ndx);
		if (form)
			form->GetKeyboard()->SetInstrument(ic);
		if (ic && theProject)
			theProject->prjMidiIn.SetInstrument(ic->inum);
	}
	return ndx;
}

int KeyboardDlg2::RemoveInstrument(InstrConfig *ic)
{
	if (form)
		form->GetKeyboard()->SetInstrument(0);

	int sel = instrList.GetCurSel();
	int ndx = FindInstrument(ic);
	if (ndx >= 0)
	{
		instrList.DeleteString(ndx);
		if (ndx == sel)
		{
			int count = instrList.GetCount();
			if (ndx == count)
				ndx--;
			instrList.SetCurSel(ndx);
		}
	}
	return ndx;
}

// this is called if the instrument name gets changed.
int KeyboardDlg2::UpdateInstrument(InstrConfig *ic)
{
	int ndx = FindInstrument(ic);
	if (ndx >= 0)
	{
		instrList.DeleteString(ndx);
		if (--ndx < 0)
			ndx = 0;
		ndx = InsertInstrument(ndx, ic);
	}
	return ndx;
}

void KeyboardDlg2::UpdateChannels()
{
}

int KeyboardDlg2::InsertInstrument(int ndx, InstrConfig *ic)
{
	ndx = instrList.InsertStringUTF8(ndx, ic->GetName());
	instrList.SetItemDataPtr(ndx, ic);
	return ndx;
}
