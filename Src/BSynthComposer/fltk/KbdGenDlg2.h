//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file KbdGenDlg.h Keyboard/Generator dialog (virtual keyboard)
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef KBDGENDLG_H
#define KDBGENDLG_H

class KbdGenDlg :
	public Fl_Group,
	public FormEditor
{
private:
	Fl_Hold_Browser *instrList;
	KeyboardForm *form;
	wdgColor bgColor;

public:
	KbdGenDlg(int X, int Y, int W, int H);
	~KbdGenDlg();

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

	virtual ProjectItem *GetItem() { return 0; }
	virtual void SetItem(ProjectItem *p) { }
	virtual void Undo() { }
	virtual void Redo() { }
	virtual void Cut() { }
	virtual void Copy() { }
	virtual void Paste() { }
	virtual void Find() { }
	virtual void FindNext() { }
	virtual void SelectAll() { }
	virtual void GotoLine(int ln) { }
	virtual void GotoPosition(int pos) { }
	virtual void SetMarker() { }
	virtual void SetMarkerAt(int line, int on) { }
	virtual void NextMarker() { }
	virtual void PrevMarker() { }
	virtual void ClearMarkers() { }
	virtual void Cancel() { }
	virtual long EditState() { return 0; }
	virtual int IsChanged()  { return 0; }
	virtual void SetForm(WidgetForm *frm) { }
	virtual void Focus() { take_focus(); }


	virtual WidgetForm *GetForm()
	{
		return form;
	}

	virtual void Capture() { }
	virtual void Release() { }

	virtual void Redraw(SynthWidget *wdg)
	{
		if (wdg)
		{
			wdgRect area = wdg->GetArea();
			damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
			SynthWidget *bud = wdg->GetBuddy2();
			while (bud)
			{
				area = bud->GetArea();
				damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
				bud = bud->GetBuddy2();
			}
		}
		else
			redraw();
	}

	virtual void Resize() { }
	SynthWidget *SystemWidget(const char *) { return 0; }

	void OnInstrChange();

	void Load();
	int IsRunning();
	int Stop();
	int Start();
	void Clear();
	void InitInstrList();
	void AddInstrument(InstrConfig *ic);
	void RemoveInstrument(InstrConfig *ic);
	void UpdateInstrument(InstrConfig *ic);
	void SelectInstrument(InstrConfig *ic);
	void UpdateChannels();
};

class BarMeter : public Fl_Widget
{
private:
	float barValue;
	float barRange;

public:
	BarMeter(int X, int Y, int W, int H)
		: Fl_Widget(X, Y, W, H, "")
	{
		barValue = 0.0;
		barRange = 1.0;
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

	void draw();
};


class GenerateDlg :
	public Fl_Window,
	public GenerateWindow
{
private:
	bsString lastMsg;
	int canceled;
	int closed;
	long lastTime;
	long startTime;
	float playRate;
	SynthMutex dlgLock;
	static long playFrom;
	static long playTo;
	static long playLive;
	static long updateRate;

	AmpValue lftPeak;
	AmpValue rgtPeak;
	AmpValue lftMax;
	AmpValue rgtMax;

	Fl_Button *spkrBtn;
	Fl_Button *diskBtn;
	Fl_Button *allBtn;
	Fl_Button *someBtn;
	Fl_Input *fromInp;
	Fl_Input *toInp;
	Fl_Button *startBtn;
	Fl_Button *stopBtn;
	Fl_Button *pauseBtn;
	Fl_Text_Display *msgInp;
	Fl_Button *closeBtn;
	Fl_Input *tmInp;
	Fl_Output *pkLft;
	Fl_Output *pkRgt;
	BarMeter *tmBar;
	BarMeter *lpkMtr;
	BarMeter *rpkMtr;

	int StartThread();
	void EndThread();

public:
	GenerateDlg(int live);
	~GenerateDlg();

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt);
	virtual void Finished();
	virtual int WasCanceled();

	void Run(int autoStart);
	void OnStart(int autoStart);
	void OnStop();
	void OnPause();
	void OnClose();
	void FormatPeak();
};

#endif
