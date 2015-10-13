//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FormEditor.h"

/////////////////////////////////////////////////
// All form editors share the same frame buffer.
// The buffer will grow in size until it is as
// big as the biggest form. Having the buffer
// always around makes the immediate redraw of
// widgets fast, smooth, and flicker free...
////////////////////////////////////////////////
Bitmap *FormEditorWin::offscrn = 0;
RECT FormEditorWin::offsSize = {0,0,0,0};

FormEditorWin::FormEditorWin()
{
	scrl = 0;
	item = 0;
	form = 0;
	bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
	fgColor.SetFromCOLORREF(0);
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.SetValue((ARGB)clr);
}

FormEditorWin::~FormEditorWin()
{
	delete form;
	delete scrl;
}

BOOL FormEditorWin::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// TODO: handle TAB, etc...
	}
	return FALSE;
}

LRESULT FormEditorWin::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//	if (offscrn)
//	{
//		delete offscrn;
//		offscrn = 0;
//	}
	return 0;
}

LRESULT FormEditorWin::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
/*	RECT rcWnd;
	GetClientRect(&rcWnd);
	Graphics gr((HDC)wParam);
	SolidBrush br(bgColor);
	gr.FillRectangle(&br, 0, 0, rcWnd.right+1, rcWnd.bottom+1);*/
	return 1;
}

LRESULT FormEditorWin::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	Graphics ctx(dc);
	RECT rcWnd;
	GetClientRect(&rcWnd);
	SolidBrush br(bgColor);
	if (form)
	{
		if (!offscrn || rcWnd.right > offsSize.right || rcWnd.bottom > offsSize.bottom)
		{
			// grow the frame buffer
			if (offscrn)
				delete offscrn;
			if (rcWnd.right > offsSize.right)
				offsSize.right = rcWnd.right;
			if (rcWnd.bottom > offsSize.bottom)
				offsSize.bottom = rcWnd.bottom;
			offscrn = new Bitmap(offsSize.right, offsSize.bottom);
		}
		Graphics *ctx2;
		if (offscrn)
			ctx2 = Graphics::FromImage(offscrn);
		else // justin case...
			ctx2 = new Graphics(dc);
		ctx2->FillRectangle(&br, 0, 0, rcWnd.right+1, rcWnd.bottom+1);
		form->RedrawForm((DrawContext)ctx2);
		delete ctx2;
		ctx.DrawImage(offscrn, 0, 0);
	}
	else
	{
		ctx.FillRectangle(&br, rcWnd.left, rcWnd.top, (rcWnd.right - rcWnd.left)+1, (rcWnd.bottom - rcWnd.top)+1);
	}
	EndPaint(&ps);
	return 0;
}

LRESULT FormEditorWin::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	return 0;
}

LRESULT FormEditorWin::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	return 0;
}

LRESULT FormEditorWin::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
		form->SetFocus();
	return 0;
}

LRESULT FormEditorWin::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
		form->KillFocus();
	return 0;
}

LRESULT FormEditorWin::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT FormEditorWin::OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT FormEditorWin::OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT FormEditorWin::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT FormEditorWin::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (form)
	{
		if (lParam)
			form->Command(LOWORD(wParam), HIWORD(wParam));
	}
	return 0;
}

LRESULT FormEditorWin::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

SynthWidget *FormEditorWin::SystemWidget(const char *type)
{
	// TODO: if this is a windows control, create the control
	// and a SynthWidget-derived wrapper object.
	return NULL;
}

void FormEditorWin::Redraw(SynthWidget *wdg)
{
	if (wdg)
	{
		Graphics *gr = new Graphics(m_hWnd);
		Graphics *gr2 = 0;
		if (offscrn)
			gr2 = Graphics::FromImage(offscrn);
		else
			gr2 = gr;
		wdg->Paint((DrawContext)gr2);
		SynthWidget *bud = wdg->GetBuddy2();
		if (bud)
			bud->Paint((DrawContext)gr2);
		if (offscrn)
		{
			delete gr2;
			wdgRect a = wdg->GetArea();
			gr->DrawImage(offscrn, a.x, a.y, a.x, a.y, a.w, a.h, UnitPixel);
			if (bud)
			{
				a = bud->GetArea();
				gr->DrawImage(offscrn, a.x, a.y, a.x, a.y, a.w, a.h, UnitPixel);
			}
		}
		delete gr;
	}
	else
		InvalidateRect(NULL, 0);
}

void FormEditorWin::Resize()
{
	if (form)
	{
		Size sz(0,0);
		form->GetSize(sz.Width, sz.Height);
		SetWindowPos(NULL, 0, 0, sz.Width+1, sz.Height+1, SWP_NOZORDER|SWP_NOMOVE);
		InvalidateRect(0,0);
		if (scrl)
			scrl->Layout();
	}
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
	OpenClipboard();
	EmptyClipboard();
	bsString text;
	if (form)
	{
		form->CopyText(text);
		size_t len = text.Length();
		if (len > 0)
		{
			HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, len+1);
			if (h)
			{
				char *p2 = (char *) GlobalLock(h);
				strncpy(p2, text, len+1);
				GlobalUnlock(h);
				SetClipboardData(CF_TEXT, h);
			}
		} 
	}
	CloseClipboard();
}

void FormEditorWin::PasteFromClipboard()
{
	if (form)
	{
		OpenClipboard();
		HANDLE h = GetClipboardData(CF_TEXT);
		if (h)
		{
			char *ptext = (char *) GlobalLock(h);
			int len = GlobalSize(h);
			char *valbuf = new char[len+1];
			memcpy(valbuf, ptext, len);
			valbuf[len] = 0;
			char *cr = strchr(valbuf, '\r');
			if (cr)
				*cr = 0;
			GlobalUnlock(h);
			form->PasteText(valbuf);
			delete valbuf;
		}
		CloseClipboard();
	}
}

/////////////////////////////////////////////////////////////////////////

LRESULT ScrollForm::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor.SetValue((ARGB)clr);
	return 0;
}

LRESULT ScrollForm::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (formWnd)
		formWnd->SetFocus();
	return 0;
}

LRESULT ScrollForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (cmdID)
		::PostMessage(GetParent(), WM_COMMAND, cmdID, 0);
	return 0;
}

LRESULT ScrollForm::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	Graphics gr(dc);
	SolidBrush br(bgColor);
	if (formWnd)
	{
		RECT rcForm;
		formWnd->GetClientRect(&rcForm);
		formWnd->MapWindowPoints(m_hWnd, &rcForm);
		if (rcForm.right <= ps.rcPaint.right)
		{
			gr.FillRectangle(&br, rcForm.right, ps.rcPaint.top, 
				(ps.rcPaint.right - rcForm.right)+1, (ps.rcPaint.bottom - ps.rcPaint.top)+1);
		}
		if (rcForm.bottom <= ps.rcPaint.bottom)
		{
			gr.FillRectangle(&br, ps.rcPaint.left, rcForm.bottom,
				(ps.rcPaint.right - ps.rcPaint.left)+1, (ps.rcPaint.bottom - rcForm.bottom)+1);
		}
	}
	else
	{
		gr.FillRectangle(&br, ps.rcPaint.left, ps.rcPaint.top, 
			(ps.rcPaint.right - ps.rcPaint.left)+1, (ps.rcPaint.bottom - ps.rcPaint.top)+1);
	}
	EndPaint(&ps);
	return 0;
}

LRESULT ScrollForm::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT ScrollForm::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int code = (int) LOWORD(wParam);
	int pos = (int) HIWORD(wParam);
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0, 0};
    GetScrollInfo(SB_VERT, &si);
	RECT rcForm;
	ATLTRACE("OnVScroll: pos = %d (%d), code = %d\n", pos, si.nPos, code);

	switch (code)
	{
	case SB_ENDSCROLL:
		pos = si.nPos;
		if (formWnd)
		{
			formWnd->GetClientRect(&rcForm);
			formWnd->MapWindowPoints(m_hWnd, &rcForm);
			formWnd->SetWindowPos(0, rcForm.left, -si.nPos, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		}
		break;
	case SB_RIGHT:
		break;
	case SB_LEFT:
		break;
	case SB_LINELEFT:
		pos = si.nPos - 1;
		break;
	case SB_LINERIGHT:
		pos = si.nPos + 1;
		break;
	case SB_PAGERIGHT:
		pos = si.nPos + si.nPage;
		break;
	case SB_PAGELEFT:
		pos = si.nPos - si.nPage;
		break;
	case SB_THUMBPOSITION:
		break;
	}
	if (pos < 0)
		pos = 0;
	else if (pos > si.nMax)
		pos = si.nMax;
	si.fMask = SIF_POS;
	si.nPos = pos;
	SetScrollInfo(SB_VERT, &si, TRUE);

	return 0;
}

LRESULT ScrollForm::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int code = (int) LOWORD(wParam);
	int pos = (int) HIWORD(wParam);
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0, 0};
    GetScrollInfo(SB_HORZ, &si);
	RECT rcForm;
	//ATLTRACE("OnHScroll: pos = %d, code = %d\n", pos, code);
	switch (code)
	{
	case SB_ENDSCROLL:
		pos = si.nPos;
		if (formWnd)
		{
			formWnd->GetClientRect(&rcForm);
			formWnd->MapWindowPoints(m_hWnd, &rcForm);
			formWnd->SetWindowPos(0, -si.nPos, rcForm.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		}
		break;
	case SB_RIGHT:
	case SB_LEFT:
		break;
	case SB_LINELEFT:
		pos = si.nPos - 1;
		break;
	case SB_LINERIGHT:
		pos = si.nPos + 1;
		break;
	case SB_PAGERIGHT:
		pos = si.nPos + si.nPage;
		break;
	case SB_PAGELEFT:
		pos = si.nPos - si.nPage;
		break;
	case SB_THUMBPOSITION:
		break;
	}
	if (pos < 0)
		pos = 0;
	else if (pos > si.nMax)
		pos = si.nMax;
	si.fMask = SIF_POS;
	si.nPos = pos;
	SetScrollInfo(SB_HORZ, &si, TRUE);
	return 0;
}

LRESULT ScrollForm::OnWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (formWnd)
	{
		int delta = (((int) wParam) >> 16) / WHEEL_DELTA;
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0, 0};
		GetScrollInfo(SB_VERT, &si);
		if ((int)si.nPage >= si.nMax)
			return 0;
		int pos = si.nPos + (delta * si.nPage);
		ATLTRACE("Wheel delta %d pos %d\n", delta, pos);
		if (pos < 0)
			pos = 0;
		else if ((pos + (int)si.nPage) >= si.nMax)
			pos = si.nMax - si.nPage;
		si.fMask = SIF_POS;
		si.nPos = pos;
		ATLTRACE("Wheel set pos %d pg=%d max=%d\n", pos, si.nPage, si.nMax);
		SetScrollInfo(SB_VERT, &si, TRUE);
		RECT rcForm;
		formWnd->GetClientRect(&rcForm);
		formWnd->MapWindowPoints(m_hWnd, &rcForm);
		formWnd->SetWindowPos(0, rcForm.left, -pos, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	}
	return 0;
}

LRESULT ScrollForm::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Layout();
	return 0;
}

void ScrollForm::Layout()
{
	RECT rc;
	GetClientRect(&rc);
	int scrollWidth = rc.right - rc.left;
	int scrollHeight = rc.bottom - rc.top;
	int formWidth;
	int formHeight;
	if (formWnd && formWnd->IsWindow())
	{
		formWnd->SetWindowPos(0, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		formWnd->GetClientRect(&rc);
		formWidth = rc.right - rc.left;
		formHeight = rc.bottom - rc.top;
	}
	else
	{
		formWidth = 0;
		formHeight = 0;
	}

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
	si.nPos = 0;
	si.nTrackPos = 0;
	si.nMin = 0;
	si.nMax = formWidth;
	si.nPage = scrollWidth;
	SetScrollInfo(SB_HORZ, &si, 0);

	si.nMax = formHeight;
	si.nPage = scrollHeight;
	SetScrollInfo(SB_VERT, &si, 0);
}
