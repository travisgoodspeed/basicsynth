/////////////////////////////////////////////////////////////////////////////
// Keyboard display window. This draws a piano keyboard on screen and sends
// messages when the user clicks on keys.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
#pragma once

#define WM_VKBD (WM_USER+13)
#define VKBD_KEYDN 1
#define VKBD_KEYUP 2
#define VKBD_CHANGE 3

class KbdWindow :
	public CWindowImpl<KbdWindow>
{
private:
	int lastKey;
	int lastVel;
	int octs;
	int whtKeys;
	int blkKeys;
	int playing;
	Rect *rcWhite;
	Rect *rcBlack;
	Rect *rcLastKey;
	int knWhite[7];
	int knBlack[5];
	HWND notifyWnd;

public:
	KbdWindow()
	{
		playing = 0;
		lastKey = -1;
		lastVel = 100;
		octs = 1;
		whtKeys = 7;
		blkKeys = 5;
		rcWhite = 0;
		rcBlack = 0;
		rcLastKey = 0;
		notifyWnd = 0;
	}

	~KbdWindow()
	{
	}

	BEGIN_MSG_MAP(KbdWindow)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONDOWN , OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnBtnUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	END_MSG_MAP()

	void SetNotify(HWND w)
	{
		notifyWnd = w;
	}

	void SetOctaves(int n)
	{ 
		if (n < 1)
			n = 1;
		octs = n;
		whtKeys = n * 7;
		blkKeys = n * 5;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		rcWhite = new Rect[whtKeys];
		rcBlack = new Rect[blkKeys];

		RECT rc;
		GetClientRect(&rc);
		int widWhite = (rc.right - rc.left) / whtKeys;
		int hiWhite = rc.bottom - rc.top;
		if (hiWhite > (5*widWhite))
			hiWhite = 5*widWhite;
		else if (hiWhite < (3*widWhite))
			widWhite = hiWhite / 3;
		int hiBlack = (hiWhite * 2) / 3;
		int widBlack = (widWhite * 7) / 10;
		int space = ((rc.right - rc.left) - (widWhite * whtKeys)) / 2;

		int xWhite = space;
		int xBlack = space + widWhite - (widBlack / 2);
		int i, on, ndx;
		int ndxw = 0;
		int ndxb = 0;
		for (on = 0; on < octs; on++)
		{
			ndx = on * 7;
			for (i = 0; i < 7; i++)
			{
				rcWhite[ndxw].X = (int) xWhite;
				rcWhite[ndxw].Y = 0;
				rcWhite[ndxw].Width = (int) widWhite;
				rcWhite[ndxw].Height = hiWhite;
				xWhite += widWhite;
				ndxw++;
			}

			ndx = on * 5;
			for (i = 0; i < 2; i++)
			{
				rcBlack[ndxb].X = (int) xBlack;
				rcBlack[ndxb].Y = 0;
				rcBlack[ndxb].Width = (int) widBlack;
				rcBlack[ndxb].Height = hiBlack;
				xBlack += widWhite;
				ndxb++;
			}

			xBlack += widWhite;
			for ( i = 0; i < 3; i++)
			{
				rcBlack[ndxb].X = (int) xBlack;
				rcBlack[ndxb].Y = 0;
				rcBlack[ndxb].Width = (int) widBlack;
				rcBlack[ndxb].Height = hiBlack;
				xBlack += widWhite;
				ndxb++;
			}
			xBlack += widWhite;
		}

		knWhite[0] = 0;
		knWhite[1] = 2;
		knWhite[2] = 4;
		knWhite[3] = 5;
		knWhite[4] = 7;
		knWhite[5] = 9;
		knWhite[6] = 11;
		knBlack[0] = 1;
		knBlack[1] = 3;
		knBlack[2] = 6;
		knBlack[3] = 8;
		knBlack[4] = 10;

		lastKey = -1;
		rcLastKey = 0;
		return 0;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(&ps);
		Graphics gr(dc);
		gr.Clear(Color(92,92,64));
		LinearGradientBrush lgw(rcWhite[0], Color(255, 255, 255), Color(32,32,32), 0.0, FALSE);
		LinearGradientBrush lgk(rcWhite[0], Color(220, 220, 220), Color(8,8,8), 0.0, FALSE);
		REAL fact[4] = { 1.0f, 0.0f, 0.0f, 1.0f, };
		REAL pos[4] = { 0.0f, 0.1f, 0.9f, 1.0f };
		lgw.SetBlend(fact, pos, 4);
		lgk.SetBlend(fact, pos, 4);
		Rect *rp = rcWhite;
		int i;
		for (i = 0; i < whtKeys; i++)
		{
			if (rp == rcLastKey)
				gr.FillRectangle(&lgk, *rp);
			else
				gr.FillRectangle(&lgw, *rp);
			rp++;
		}
		//gr.FillRectangles(&lgw, rcWhite, octs*7);

		SolidBrush lgb(Color(8,8,8));
		SolidBrush lgk2(Color(64,64,64));
		rp = rcBlack;
		for (i = 0; i < blkKeys; i++)
		{
			if (rp == rcLastKey)
				gr.FillRectangle(&lgk2, *rp);
			else
				gr.FillRectangle(&lgb, *rp);
			rp++;
		}
		//gr.FillRectangles(&lgb, rcBlack, blkKeys);

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		playing = 1;
		SetCapture();
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		lastKey = -1;
		rcLastKey = 0;
		if (FindKey(pt))
			::PostMessage(notifyWnd, WM_VKBD, VKBD_KEYDN, (LPARAM)((lastVel << 8)|lastKey));
		return 0;
	}

	LRESULT OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (playing)
		{
			playing = 0;
			ReleaseCapture();
			//OutputDebugString("Send key up\r\n");
			::PostMessage(notifyWnd, WM_VKBD, VKBD_KEYUP, (LPARAM)((lastVel << 8)|lastKey));
			InvalidateLast();
			rcLastKey = 0;
		}
		return 0;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (playing)
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (FindKey(pt))
				::PostMessage(notifyWnd, WM_VKBD, VKBD_CHANGE, (LPARAM)((lastVel << 8)|lastKey));
		}
		return 0;
	}

	int FindKey(POINT& pt)
	{
		int ndx;
		int kdown = -1;
		Rect *rcNewKey = 0;
		for (ndx = 0; ndx < blkKeys; ndx++)
		{
			if (rcBlack[ndx].Contains(pt.x, pt.y))
			{
				kdown = knBlack[ndx % 5] + ((ndx / 5) * 12);
				rcNewKey = &rcBlack[ndx];
				break;
			}
		}
		if (kdown == -1)
		{
			for (ndx = 0; ndx < whtKeys; ndx++)
			{
				if (rcWhite[ndx].Contains(pt.x, pt.y))
				{
					kdown = knWhite[ndx % 7] + ((ndx / 7) * 12);
					rcNewKey = &rcWhite[ndx];
					break;
				}
			}
		}
		//char buf[40];
		//sprintf(buf, "Kdown = %d\r\n", kdown);
		//OutputDebugString(buf);
		if (kdown != -1 && kdown != lastKey)
		{
			if (rcNewKey)
			{
				float range = (float) (rcNewKey->GetBottom() - rcNewKey->GetTop());
				float kpos = pt.y - rcNewKey->GetTop();
				lastVel = (int) (127.0 * kpos / range);
			}
			InvalidateLast();
			lastKey = kdown;
			rcLastKey = rcNewKey;
			InvalidateLast();
			return 1;
		}
		return 0;
	}

	void InvalidateLast()
	{
		if (rcLastKey)
		{
			RECT r;
			r.left = rcLastKey->GetLeft();
			r.right = rcLastKey->GetRight();
			r.top = rcLastKey->GetTop();
			r.bottom = rcLastKey->GetBottom();
			InvalidateRect(&r, 0);
		}
	}
};
