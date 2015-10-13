//////////////////////////////////////////////////////////////////////
// Copyright 2010 Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#pragma once

class BarMeter  : public wxWindow
{
private:
    DECLARE_EVENT_TABLE()
	float barValue;
	float barRange;
	wxColour back;
	wxColour bar;

public:
	BarMeter(wxWindow *parent, wxWindowID id, wxPoint& pos, wxSize& sz)
		: wxWindow(parent, id, pos, sz, wxBORDER_SIMPLE)
	{
		barValue = 0.0;
		barRange = 1.0;
		back.Set(128,128,128);
		bar.Set(0,200,0);
	}

	~BarMeter()
	{
	}

	void Reset()
	{
		barValue = 0;
	}

	void SetRange(float rng);
	void SetValue(float val);

	void OnEraseBackground(wxEraseEvent& evt);
	void OnPaint(wxPaintEvent& evt);
};

/// @brief Generate output dialog.
/// @details This dialog handles generation of wave files as well
/// as playback of the score to the sound card.
class GenerateDlg :
	public wxDialog,
	public GenerateWindow
{
private:
    DECLARE_EVENT_TABLE()
	wxTextCtrl *ed;
	wxTextCtrl *tm;
	wxTextCtrl *lpk;
	wxTextCtrl *rpk;
	wxComboBox *rateWnd;
	//wxGauge    *tml;
	BarMeter *tml;
	wxString lastMsg;
	BarMeter *lpkMtr;
	BarMeter *rpkMtr;
	long lastTime;
	long startTime;
	int canceled;
	int paused;
	int genAuto;
	static long updateRate;
	static long playFrom;
	static long playTo;
	static int  playLive;
	static int  playSome;
	static int pkOn;
	static int tmOn;

	AmpValue lftPeak;
	AmpValue rgtPeak;
	AmpValue lftMax;
	AmpValue rgtMax;
	SynthMutex dlgLock;

	void EnableOK(int e, int c);
	long GetTimeValue(const char *id);
	void FormatTime(const char *id, long secs);
	void FormatPeak(int oor = 1);
	void SetButtonImage(int ctrl, int imgid);
	void SetPlayOutput();
	void SetPlayRange();

public:
	GenerateDlg(wxWindow *parent, int a = 0, int pl = -1);
	~GenerateDlg();

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt);
	virtual void Finished();
	virtual int WasCanceled();

	void OnSelectLive(wxCommandEvent&);
	void OnSelectDisk(wxCommandEvent&);
	void OnSelectSome(wxCommandEvent&);
	void OnSelectAll(wxCommandEvent&);
	void OnStart(wxCommandEvent&);
	void OnStop(wxCommandEvent&);
	void OnPause(wxCommandEvent&);
	void OnTimeOn(wxCommandEvent& evt);
	void OnPeakOn(wxCommandEvent& evt);
	void OnOK(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnUpdateTime(wxCommandEvent&);
	void OnUpdatePeak(wxCommandEvent&);
	void OnUpdateMsg(wxCommandEvent&);
	void OnGenFinished(wxCommandEvent&);
};

DECLARE_EVENT_TYPE(genMessage, 0)
DECLARE_EVENT_TYPE(genEnd, 0)
DECLARE_EVENT_TYPE(genUpdTime, 0)
DECLARE_EVENT_TYPE(genUpdPeak, 0)
