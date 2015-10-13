//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "GenerateDlg.h"

long GenerateDlg::playFrom;
long GenerateDlg::playTo;
int  GenerateDlg::playLive;
int  GenerateDlg::playSome;
long GenerateDlg::updateRate = 1;
int GenerateDlg::pkOn = 1;
int GenerateDlg::tmOn = 1;

LRESULT GenerateDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	EnableOK(1, 0);
	SetButtonImage(IDC_START, IDI_START);
	SetButtonImage(IDC_STOP, IDI_STOP);
	SetButtonImage(IDC_PAUSE, IDI_PAUSE);
	SetButtonImage(IDC_PLAY_LIVE, IDI_SPEAKER);
	SetButtonImage(IDC_DISK, IDI_CD);
	SetButtonImage(IDC_PLAY_ALL, IDI_ALL);
	SetButtonImage(IDC_PLAY_SOME, IDI_PART);

	CheckDlgButton(IDC_PEAK_ON, pkOn ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_TIME_ON, tmOn ? BST_CHECKED : BST_UNCHECKED);
	rateWnd = GetDlgItem(IDC_RATE);
	rateWnd.AddString("0.1 Sec");
	rateWnd.AddString("0.5 Sec");
	rateWnd.AddString("1.0 Sec");
	rateWnd.AddString("5.0 Sec");
	int rt = 0;
	switch (updateRate)
	{
	case 1: rt = 0; break;
	case 5: rt = 1; break;
	case 10: rt = 2; break;
	case 50: rt = 3; break;
	}
	rateWnd.SetCurSel(rt);

	ed = GetDlgItem(IDC_TEXT);
	tm = GetDlgItem(IDC_TIME);
	FormatTime(tm, 0);
	CWindow tmp = GetDlgItem(IDC_TIME_PROGRESS);
	RECT rctml;
	tmp.GetClientRect(&rctml);
	tmp.MapWindowPoints(m_hWnd, &rctml);
	tmp.DestroyWindow();
	tml.Create(m_hWnd, rctml, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0, 200);

	lpk = GetDlgItem(IDC_LEFT_PEAK);
	rpk = GetDlgItem(IDC_RIGHT_PEAK);
	RECT rcmtr;
	CWindow mtr;
	mtr = GetDlgItem(IDC_LPK_METER);
	mtr.GetClientRect(&rcmtr);
	mtr.MapWindowPoints(m_hWnd, &rcmtr);
	mtr.DestroyWindow();
	// setting these before the window is created
	// avoids recreating the bitmaps.
	lpkMtr.SetRange(1.2);
	lpkMtr.Divisions(12);
	lpkMtr.SetForeground(0, 100, 0);
	lpkMtr.SetLimits(3, 1);
	lpkMtr.SetLimit(1, 0.9, 100, 255, 0);
	lpkMtr.SetLimit(2, 1.0, 255, 255, 0);
	lpkMtr.SetLimit(3, 1.2, 255, 100, 0);
	lpkMtr.ShowPeak(1);
	lpkMtr.Create(m_hWnd, rcmtr, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0, 100);
	// SetValue should only be called after the window is created since
	// it does a refresh.
	lpkMtr.SetValue(1.2);
	//lpkMtr.Average(10);
	
	mtr = GetDlgItem(IDC_RPK_METER);
	mtr.GetClientRect(&rcmtr);
	mtr.MapWindowPoints(m_hWnd, &rcmtr);
	mtr.DestroyWindow();
	rpkMtr.SetRange(1.2);
	rpkMtr.Divisions(12);
	rpkMtr.SetForeground(0, 100, 0);
	rpkMtr.SetLimits(3, 1);
	rpkMtr.SetLimit(1, 0.9, 100, 255, 0);
	rpkMtr.SetLimit(2, 1.0, 255, 255, 0);
	rpkMtr.SetLimit(3, 1.2, 255, 100, 0);
	rpkMtr.ShowPeak(1);
	rpkMtr.Create(m_hWnd, rcmtr, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0, 101);
	rpkMtr.SetValue(1.2);

	CheckRadioButton(IDC_PLAY_ALL, IDC_PLAY_SOME, playSome ? IDC_PLAY_SOME : IDC_PLAY_ALL);
	FormatTime(GetDlgItem(IDC_PLAY_FROM), playFrom);
	FormatTime(GetDlgItem(IDC_PLAY_TO), playTo);
	CheckRadioButton(IDC_PLAY_LIVE, IDC_DISK, playLive ? IDC_PLAY_LIVE : IDC_DISK);
	if (genAuto)
		PostMessage(WM_COMMAND, IDC_START, 0);
	return TRUE;
}

LRESULT GenerateDlg::OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	prjFrame->GenerateStarted();
	EnableOK(0, 0);
	ed.AppendText("---------- Start ----------\r\n");
	canceled = 0;

	switch (SendDlgItemMessage(IDC_RATE, CB_GETCURSEL, 0, 0))
	{
	case 0:
		updateRate = 1;
		break;
	case 1:
		updateRate = 5;
		break;
	case 2:
		updateRate = 10;
		break;
	case 3:
		updateRate = 50;
		break;
	}

	if (!genAuto)
	{
		playLive = IsDlgButtonChecked(IDC_PLAY_LIVE);
		playSome = IsDlgButtonChecked(IDC_PLAY_SOME);
		playFrom = GetTimeValue(IDC_PLAY_FROM);
		playTo  = GetTimeValue(IDC_PLAY_TO);
	}
	if (playSome)
	{
		theProject->StartTime(playFrom);
		theProject->EndTime(playTo);
		startTime = playFrom;
	}
	else
	{
		theProject->StartTime(0);
		theProject->EndTime(0);
		startTime = 0;
	}
	lastTime = 0;
	theProject->PlayMode(playLive);

	lastMsg = "";
	FormatTime(tm, startTime);

	pkOn = IsDlgButtonChecked(IDC_PEAK_ON) == BST_CHECKED;
	tmOn = IsDlgButtonChecked(IDC_TIME_ON) == BST_CHECKED;

	lpkMtr.Reset();
	rpkMtr.Reset();

	lftPeak = 0;
	rgtPeak = 0;
	lftMax = 0;
	rgtMax = 0;
	FormatPeak(0);
	prjGenerate = static_cast<GenerateWindow*>(this);

	theProject->CallbackRate((float)updateRate / 10.0f);
	if (theProject->Start() == 0)
		EnableOK(0, 1);
	else
		EnableOK(1, 0);
	return 0;
}

LRESULT GenerateDlg::OnStop(WORD cd, WORD wID, HWND hwnd, BOOL& bHandled)
{
	dlgLock.Enter();
	lastMsg += "*Cancel*\r\n";
	ed.AppendText(lastMsg);
	lastMsg = "";
	canceled = 1;
	try
	{
		if (theProject->cvtActive)
			theProject->cvtActive->Cancel();
	}
	catch(...)
	{
	}
	dlgLock.Leave();
	return 0;
}

LRESULT GenerateDlg::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_PAUSE))
	{
		if (!theProject->Pause())
			CheckDlgButton(IDC_PAUSE, 0);
	}
	else
		theProject->Resume();
	return 0;
}

LRESULT GenerateDlg::OnTimeClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	tmOn = ::SendMessage(hWndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED;
	return 0;
}

LRESULT GenerateDlg::OnPeakClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	pkOn = ::SendMessage(hWndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED;
	return 0;
}

LRESULT GenerateDlg::OnPlayFrom(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	playFrom = GetTimeValue(IDC_PLAY_FROM);
	FormatTime(hWndCtl, playFrom);
	return 0;
}

LRESULT GenerateDlg::OnPlayTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	playTo  = GetTimeValue(IDC_PLAY_TO);
	FormatTime(hWndCtl, playTo);
	return 0;
}

LRESULT GenerateDlg::OnGenFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	theProject->WaitThread();
	prjGenerate = 0;
	char pk[1024];
	snprintf(pk, 1024, "Peak: [%.3f, %.3f]\r\n-------- Finished ---------\r\n", lftMax, rgtMax);
	ed.AppendText(pk);
	lftPeak = lftMax;
	rgtPeak = rgtMax;
	FormatPeak(0);
	prjFrame->GenerateFinished();
	if (genAuto)
		EndDialog(IDOK);
	else
		EnableOK(1, 0);
	return 0;
}

LRESULT GenerateDlg::OnUpdateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	dlgLock.Enter();
	if (lastMsg.Length() != 0)
	{
		ed.AppendText(lastMsg);
		lastMsg = "";
	}
	dlgLock.Leave();
	return 0;
}

LRESULT GenerateDlg::OnUpdateTime(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (lParam == 0)
	{
		float len = ((float)theProject->seq.GetLength() * 10.0) / synthParams.sampleRate;
		tml.SetRange(len);
	}
	FormatTime(tm, lastTime);
	tml.SetValue((float)lastTime);
	return 0;
}

LRESULT GenerateDlg::OnUpdatePeak(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FormatPeak(1);
	return 0;
}

LRESULT GenerateDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDOK);
	return 0;
}

LRESULT GenerateDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
//	EndDialog(IDCANCEL);
	return 0;
}

void GenerateDlg::FormatTime(HWND w, long secs)
{
	char txt[80];
	snprintf(txt, 80, "%02d:%02d.%1d", secs / 600, (secs / 10) % 60, secs % 10);
	::SendMessage(w, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)txt);
}

void GenerateDlg::FormatPeak(int oor)
{
	char pkText[256];
	snprintf(pkText, 256, "%.3f", lftPeak);
	lpk.SetWindowText(pkText);
	snprintf(pkText, 256, "%.3f", rgtPeak);
	rpk.SetWindowText(pkText);
	if (oor && (lftPeak > 1.0 || rgtPeak > 1.0))
	{
		snprintf(pkText, 256, "Out of range (%.3f, %.3f) at %02d:%02d.%1d\r\n", 
			lftPeak, rgtPeak, lastTime / 600, (lastTime / 10) % 60, lastTime % 10);
		ed.AppendText(pkText);
	}
	lpkMtr.SetValue(lftPeak);
	rpkMtr.SetValue(rgtPeak);
}

void GenerateDlg::EnableOK(int e, int c)
{
	::EnableWindow(GetDlgItem(IDOK), e);
	::EnableWindow(GetDlgItem(IDC_PLAY_ALL), e);
	::EnableWindow(GetDlgItem(IDC_PLAY_SOME), e);
	::EnableWindow(GetDlgItem(IDC_PLAY_LIVE), e);
	::EnableWindow(GetDlgItem(IDC_DISK), e);
	::EnableWindow(GetDlgItem(IDC_RATE), e);
	::EnableWindow(GetDlgItem(IDC_STOP), c);
	::EnableWindow(GetDlgItem(IDC_START), !c);
	::EnableWindow(GetDlgItem(IDC_PAUSE), c);
}

long GenerateDlg::GetTimeValue(int id)
{
	char buf[80];
	GetDlgItemText(id, buf, 80);
	char *bp = buf;
	long minutes = 0;
	long tenths = 0;
	long seconds = 0;
	char *delim = strchr(bp, ':');
	if (delim != NULL)
	{
		minutes = atol(bp);
		bp = delim + 1;
	}
	seconds = atol(bp);
	if ((delim = strchr(bp, '.')) != NULL)
	{
		if (isdigit(delim[1]))
			tenths = delim[1] - '0';
	}
	return (minutes * 600) + (seconds * 10) + tenths;
}

void GenerateDlg::AddMessage(const char *s)
{
	dlgLock.Enter();
	lastMsg += s;
	lastMsg += "\r\n";
	dlgLock.Leave();
	PostMessage(WM_GENMSG, 0, 0);
}

void GenerateDlg::UpdateTime(long tmval)
{
	lastTime = (tmval * updateRate) + startTime;
	if (tmOn)
		PostMessage(WM_GENUPDTM, 0, tmval);
}

void GenerateDlg::Finished()
{
	PostMessage(WM_GENEND, 0, 0);
}

int GenerateDlg::WasCanceled()
{
	return canceled;
}

void GenerateDlg::UpdatePeak(float lft, float rgt)
{
	lftPeak = lft;
	rgtPeak = rgt;
	if (lftPeak > lftMax)
		lftMax = lftPeak;
	if (rgtPeak > rgtMax)
		rgtMax = rgtPeak;
	if (pkOn)
		PostMessage(WM_GENUPDPK, 0, 0);
}

void GenerateDlg::SetButtonImage(int ctrl, int imgid)
{
	HICON ico = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(imgid), IMAGE_ICON, 32, 32, 0);
	CButton btn = GetDlgItem(ctrl);
	btn.SetIcon(ico);
}
/////////////////////////////////////////////////////////////////////

void BarMeter::CreateBitmaps()
{
	RECT rcWnd;
	GetClientRect(&rcWnd);
	RectF brrc(0, 0, rcWnd.right, rcWnd.bottom);
	backBits = new Bitmap(rcWnd.right, rcWnd.bottom);
	meterBits = new Bitmap(rcWnd.right, rcWnd.bottom);

	Graphics *ctx;
	ctx = Graphics::FromImage(backBits);
	LinearGradientBrush bg(brrc, highColor, backColor, 90.0);
	ctx->FillRectangle(&bg, 0, 0, rcWnd.right+1, rcWnd.bottom+1);
	if (numDivisions)
	{
		Pen pn(backColor);
		REAL val = 0;
		REAL end = REAL(rcWnd.right);
		REAL incr = end / REAL(numDivisions);
		while (val < end)
		{
			ctx->DrawLine(&pn, (int)val, 0, (int)val, rcWnd.bottom);
			val += incr;
		}
	}
	delete ctx;

	ctx = Graphics::FromImage(meterBits);
	ctx->DrawImage(backBits, 0, 0);
	if (numLimits < 1)
	{
		SolidBrush fg(foreColor);
		ctx->FillRectangle(&fg, 0, 2, rcWnd.right+1, rcWnd.bottom-4);
	}
	else
	{
		if (gradient)
		{
			LinearGradientBrush fg(brrc, foreColor, foreColor, 0.0);
			fg.SetInterpolationColors(limColors, limPositions, numLimits+1);
			ctx->FillRectangle(&fg, 0, 2, rcWnd.right+1, rcWnd.bottom-4);
		}
		else
		{
			int left = 0;
			int right;
			for (int i = 1; i <= numLimits; i++)
			{
				SolidBrush fg(limColors[i-1]);
				right = (int)(rcWnd.right * limPositions[i]);
				ctx->FillRectangle(&fg, left, 2, right, rcWnd.bottom-4);
				left = right+1;
			}
			if (left < rcWnd.right)
			{
				SolidBrush fg(limColors[numLimits]);
				ctx->FillRectangle(&fg, right, 2, rcWnd.right, rcWnd.bottom-4);
			}
		}
	}

	delete ctx;
}

void BarMeter::DestroyBitmaps()
{
	if (backBits)
	{
		delete backBits;
		backBits = 0;
	}
	if (meterBits)
	{
		delete meterBits;
		meterBits = 0;
	}
}

void BarMeter::SetValue(float val)
{
	barValue = val;
	if (val > barMax)
		barMax = val;
	else
		barMax *= 0.95;
	if (avgVals)
	{
		barSum += val;
		barSum -= avgVals[avgIndex];
		avgVals[avgIndex] = val;
		if (++avgIndex >= avgCount)
			avgIndex = 0;
		barAvg = barSum / (float)avgCount;
	}
	Invalidate(0);
}

LRESULT BarMeter::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}
	
LRESULT BarMeter::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	Graphics ctx(dc);

	RECT rcWnd;
	GetClientRect(&rcWnd);
	int w = rcWnd.right - rcWnd.left;
	int h = rcWnd.bottom - rcWnd.top;

	if (backBits == 0)
		CreateBitmaps();

	if (backBits)
		ctx.DrawImage(backBits, 0, 0);
	else
	{
		SolidBrush bg(backColor);
		ctx.FillRectangle(&bg, 0, 0, w, h);
	}
	if (meterBits)
	{
		int x;
		if (avgCount > 0)
			x = (int) (w * barAvg / barRange);
		else
			x = (int) (w * barValue / barRange);
		ctx.DrawImage(meterBits, 0, 0, 0, 0, x, h, UnitPixel);
	}
	else
	{
		SolidBrush br(foreColor);
		ctx.FillRectangle(&br, 0, 0, (int) (w * barValue / barRange), h);
	}
	if (showPeak)
	{
		Pen pn(Color(40,40,40));
		int x = (int) (w * barMax / barRange);
		ctx.DrawLine(&pn, x, 0, x, h);
	}

	EndPaint(&ps);
	return 0;
}

LRESULT BarMeter::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyBitmaps();
	CreateBitmaps();
	return 0;
}

LRESULT BarMeter::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyBitmaps();
	return 0;
}

