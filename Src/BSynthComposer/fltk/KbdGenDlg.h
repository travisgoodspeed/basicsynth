#ifndef KBDGENDLG_H
#define KDBGENDLG_H

#define VKBD_START 1
#define VKDB_STOP  2
#define VKDB_CHANGE 3

struct KbdMsg
{
	int evt;
	int key;
};

class KbdWdgFltk : public Fl_Widget
{
private:
	int lastKey;
	int octs;
	int whtKeys;
	int blkKeys;
	int playing;
	wdgRect *rcWhite;
	wdgRect *rcBlack;
	wdgRect *rcLastKey;
	int knWhite[7];
	int knBlack[5];

	void CreateKeyboard();

public:
	KbdWdgFltk(int X, int Y, int W, int H);
	~KbdWdgFltk();

	void draw();
	int handle(int e);
	void resize(int, int, int, int);

	int FindKey(int mx, int my);
	void InvalidateLast();
	void SetOctaves(int n);
	void OnBtnDown(int mx, int my);
	void OnBtnUp(int mx, int my);
	void OnMouseMove(int mx, int my);
};

class KbdGenDlg : public Fl_Group
{
private:
	Fl_Button *gen;
	Fl_Button *all;
	Fl_Button *some;
	Fl_Button *spkr;
	Fl_Button *disk;
	Fl_Input  *from;
	Fl_Input  *to;
	
	KbdWdgFltk *kbd;
	Fl_Value_Slider *volLvl;
	Fl_Choice *chnlList;
	Fl_Button *startPlay;
	Fl_Button *stopPlay;
	Fl_Button *wholeNote;
	Fl_Button *halfNote;
	Fl_Button *quartNote;
	Fl_Button *eightNote;
	Fl_Button *sixtnNote;
	Fl_Button *recNotes;
	Fl_Button *grpNotes;
	Fl_Button *cpyNotes;

	Fl_Hold_Browser *instrList;
	int kbdRunning;
	int curChnl;
	double curVol;
	InstrConfig *activeInstr;
public:
	KbdGenDlg(int X, int Y, int W, int H);

	int GetPlayLive();
	int GetPlayAll();
	long GetFrom();
	long GetTo();

	void OnInstrChange();
	void OnStartBtn();
	void OnStopBtn();
	void OnKbdEvent(KbdMsg *msg);

	int IsRunning() { return kbdRunning; }
	int Stop();
	int Start();
	void Clear();
	void InitInstrList();
	void AddInstrument(InstrConfig *ic);
	void RemoveInstrument(InstrConfig *ic);
	void UpdateInstrument(InstrConfig *ic);
	void UpdateChannels();
	void StartRecord();
	void StopRecord();
	void CopyNotes();
};

class GenerateDlg : 
	public Fl_Window,
	public GenerateWindow
{
private:
	bsString lastMsg;
	long lastTime;
	int canceled;
	int genAuto;

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
	Fl_Text_Display *msgInp;
	Fl_Button *closeBtn;
	Fl_Input *tmInp;
	Fl_Output *pkLft;
	Fl_Output *pkRgt;

	KbdGenDlg *kbdDlg;

	int StartThread();
	void EndThread();

public:
	GenerateDlg(KbdGenDlg *d);
	~GenerateDlg();

	static long playLive;
	static long playFrom;
	static long playTo;

	virtual void AddMessage(const char *s);
	virtual void UpdateTime(long tm);
	virtual void UpdatePeak(AmpValue lft, AmpValue rgt);
	virtual void Finished();
	virtual int WasCanceled();

	void OnStart(int autoStart);
	void OnStop();
	void OnClose();
	void FormatPeak();
};

#endif
