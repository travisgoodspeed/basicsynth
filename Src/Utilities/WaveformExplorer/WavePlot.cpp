//////////////////////////////////////////////////////////////////////
// Plot the waveform to screen or metafile.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WavePlot.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWavePlot::CWavePlot()
{
	periods  = 1;
}

CWavePlot::~CWavePlot()
{
}

LRESULT CWavePlot::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	RECT rc;
	GetClientRect(&rc);
	FillRect(dc, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
	InflateRect(&rc, -2, -2);
	Graphics *gr = new Graphics(dc);
	Plot(gr, rc);
	delete gr;
	return 0;
}

int CWavePlot::Plot(HDC dc, RECT& rc)
{
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	if (cx > 0 && cy > 0)
	{
		int x, y;

		HPEN dots = CreatePen(PS_DOT, 0, RGB(128,128,128));
		HGDIOBJ sav = SelectObject(dc, dots);
		int mid = rc.top + (cy / 2);
		MoveToEx(dc, 0, mid, NULL);
		LineTo(dc, cx, mid);
		SelectObject(dc, sav);

		GenWaveI wv;
		wv.InitWT((float) periods * synthParams.sampleRate / (float) cx, WT_USR(0));

		float val = wv.Gen();
		y = (int) (val * mid);
		MoveToEx(dc, rc.left, mid - y, NULL);
		for (x = 0; x < cx; x++)
		{
			val = wv.Gen();
			y = (int) (val * mid);
			LineTo(dc, rc.left + x, mid - y);
		}
	}

	return 0;
}

int CWavePlot::Plot(Graphics *gr, RECT& rc)
{

try
{
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	if (cx > 0 && cy > 0)
	{
		gr->SetSmoothingMode(SmoothingModeHighQuality);
		SolidBrush bg(Color::White);
		Pen pn2(Color::Gray, 0);
		Pen pn(Color::Black);
		gr->DrawLine(&pn2, rc.left, rc.top + (cy / 2), rc.right, rc.top + (cy / 2));

		GenWaveI wv;
		wv.InitWT((float) periods * synthParams.sampleRate / (float) cx, WT_USR(0));

		REAL mid = (REAL) cy / 2.0;
		REAL yo = (REAL) (rc.top + 1) + mid;
		REAL x1 = (REAL) (rc.left + 1);
		REAL x2 = x1;
		REAL y1 = yo - (wv.Gen() * mid);
		REAL y2;
		gr->DrawLine(&pn2, x1, yo, x1 + (REAL) cx, yo);
		for (int x = 1; x < cx; x++)
		{
			x2 = x1 + 1;
			y2 = yo - (wv.Gen() * mid);
			gr->DrawLine(&pn, x1, y1, x2, y2);
			y1 = y2;
			x1 = x2;
		}
	}
}
catch(...)
{
	OutputDebugString("Exception during draw\r\n");
}
	return 0;
}
