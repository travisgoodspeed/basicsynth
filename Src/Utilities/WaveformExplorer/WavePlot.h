//////////////////////////////////////////////////////////////////////
// Plot the waveform to screen or metafile.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEPLOT_H__3524234D_D254_44ED_880E_163CDA7F4C7B__INCLUDED_)
#define AFX_WAVEPLOT_H__3524234D_D254_44ED_880E_163CDA7F4C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CWavePlot  : public CWindowImpl<CWavePlot>
{
private:
	int periods;

public:
	CWavePlot();
	virtual ~CWavePlot();

	DECLARE_WND_CLASS(_T("WavePlot"))

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CWavePlot)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	int Plot(HDC dc, RECT& rc);
	int Plot(Graphics *gr, RECT& rc);
	void SetPeriods(int n) { periods = n; }
};

#endif // !defined(AFX_WAVEPLOT_H__3524234D_D254_44ED_880E_163CDA7F4C7B__INCLUDED_)
