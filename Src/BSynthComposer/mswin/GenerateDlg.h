//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef GENERATE_DLG_H
#define GENERATE_DLG_H

#pragma once

struct BarColor
{
	float limit;
	Color color;
	int gradient;
};

class BarMeter  : public CWindowImpl<BarMeter>
{
public:
	Bitmap *backBits;
	Bitmap *meterBits;
	float barRange;
	float barValue;
	float barAvg;
	float barSum;
	float barMax;
	Color foreColor;
	Color backColor;
	Color highColor;
	Color *limColors;
	REAL  *limPositions;
	int numLimits;
	int numDivisions;
	int showPeak;
	int gradient;
	int avgCount;
	int avgIndex;
	float *avgVals;

	void CreateBitmaps();
	void DestroyBitmaps();

public:
	BarMeter() : backColor(128, 128, 128), highColor(255, 255, 255), foreColor(0, 128, 0)
	{
		backBits = 0;
		meterBits = 0;
		barValue = 0;
		barSum = 0;
		barAvg = 0;
		barMax = 0;
		barRange = 1.0;
		limColors = 0;
		limPositions = 0;
		numLimits = 0;
		numDivisions = 0;
		showPeak = 0;
		gradient = 1;
		avgVals = 0;
		avgIndex = 0;
		avgCount = 0;
	}

	~BarMeter()
	{
		delete limColors;
		delete limPositions;
		DestroyBitmaps();
	}

	void Reset()
	{
		barValue = 0;
		barAvg = 0;
		barSum = 0;
		barMax = 0;
		avgIndex = 0;
		for (int i = 0; i < avgCount; i++)
			avgVals[i] = 0;
	}

	void Average(int n)
	{
		avgCount = n;
		delete avgVals;
		if (n > 0)
		{
			avgVals = new float[n];
			for (int i = 0; i < n; i++)
				avgVals[i] = 0;
		}
		else
			avgVals = 0;
	}

	void ShowPeak(int on)
	{
		showPeak = on;
	}

	void Divisions(int n)
	{
		numDivisions = n;
	}

	void SetForeground(int r, int g, int b, int a = 255)
	{
		foreColor.SetValue(ARGB((a << ALPHA_SHIFT) | (r << RED_SHIFT) | (g << GREEN_SHIFT) | b));
	}

	void SetBackground(int r, int g, int b, int a = 255)
	{
		backColor.SetValue(ARGB((a << ALPHA_SHIFT) | (r << RED_SHIFT) | (g << GREEN_SHIFT) | b));
	}

	void SetHighlight(int r, int g, int b, int a = 255)
	{
		highColor.SetValue(ARGB((a << ALPHA_SHIFT) | (r << RED_SHIFT) | (g << GREEN_SHIFT) | b));
	}

	void SetLimits(int n, int g)
	{
		delete limColors;
		delete limPositions;
		numLimits = n;
		gradient = g;
		if (n > 0)
		{
			limColors = new Color[n+1];
			limPositions = new REAL[n+1];
			limColors[0] = foreColor;
			limPositions[0] = REAL(0.0);
			for (int i = 1; i <= n; i++)
			{
				limColors[i] = foreColor;
				limPositions[i] = REAL(i) / REAL(n);
			}
		}
		else
		{
			limColors = 0;
			limPositions = 0;
		}
		DestroyBitmaps();
	}

	void SetLimit(int n, float val, int r, int g, int b, int a = 255)
	{
		SetLimit(n, val, ARGB((a << ALPHA_SHIFT) | (r << RED_SHIFT) | (g << GREEN_SHIFT) | b));
	}

	void SetLimit(int n, float val, ARGB c)
	{
		if (n <= numLimits)
		{
			limColors[n].SetValue(c);
			if (n > 0)
				limPositions[n] = val / barRange;
			DestroyBitmaps();
		}
	}

	void SetRange(float rng)
	{
		barRange = rng;
	}

	void SetValue(float val);

	DECLARE_WND_CLASS(_T("BarMeter"))

	BEGIN_MSG_MAP(BarMeter)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		MESSAGE_HANDLER(WM_SIZE,  OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class GenerateDlg : public CDialogImpl<GenerateDlg>, public GenerateWindow
{
private:
	CEdit ed;
	CEdit tm;
	CEdit lpk;
	CEdit rpk;
	CComboBox rateWnd;

	BarMeter lpkMtr;
	BarMeter rpkMtr;
	//CWindow tml;
	BarMeter tml;
	bsString lastMsg;
	long lastTime;
	long startTime;
	int canceled;
	int genAuto;
	static int pkOn;
	static int tmOn;
	static long updateRate; // in 1/10 secs, i.e. 10 == 1 second
	static long playFrom;
	static long playTo;
	static int  playLive;
	static int  playSome;

	float lftPeak;
	float rgtPeak;
	float lftMax;
	float rgtMax;
	SynthMutex dlgLock;

	void EnableOK(int e, int c);
	long GetTimeValue(int id);
	void FormatTime(HWND w, long secs);
	void FormatPeak(int oor);
	void SetButtonImage(int ctrl, int imgid);

public:
	GenerateDlg(int a = 0, int pl = -1)
	{
		genAuto = a;
		lastTime = 0;
		canceled = 0;
		if (pl >= 0)
			playLive = pl;
		dlgLock.Create();
	}

	~GenerateDlg()
	{
		prjGenerate = 0;
		dlgLock.Destroy();
	}

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(float lft, float rgt);
	virtual void Finished();
	virtual int WasCanceled();

	enum { IDD = IDD_GENERATE };

	BEGIN_MSG_MAP(GenerateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_GENMSG, OnUpdateMsg)
		MESSAGE_HANDLER(WM_GENEND, OnGenFinished)
		MESSAGE_HANDLER(WM_GENUPDTM, OnUpdateTime)
		MESSAGE_HANDLER(WM_GENUPDPK, OnUpdatePeak)
		COMMAND_HANDLER(IDC_PLAY_FROM, EN_KILLFOCUS, OnPlayFrom)
		COMMAND_HANDLER(IDC_PLAY_TO, EN_KILLFOCUS, OnPlayTo)
		COMMAND_HANDLER(IDC_PEAK_ON, BN_CLICKED, OnPeakClick)
		COMMAND_HANDLER(IDC_TIME_ON, BN_CLICKED, OnTimeClick)
		COMMAND_ID_HANDLER(IDC_START, OnStart)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_ID_HANDLER(IDC_PAUSE, OnPause)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPlayFrom(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPlayTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPeakClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTimeClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUpdateTime(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUpdatePeak(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUpdateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGenFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif
