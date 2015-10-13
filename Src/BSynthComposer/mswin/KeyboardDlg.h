///////////////////////////////////////////////////////////////
// This is the Windows dialog version of the virtual keyboard.
// The WidgetForm version is preferred as it is mostly portable.
// So - this code is deprecated and only kept for reference.
///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Virtual Keyboard instrument player
//
// This window contains the list of instruments, keyboard, and start/stop
// controls. The keyboard is a separate window added to the dialog during
// initialization.
/////////////////////////////////////////////////////////////////////////////

#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Keyboard display window. This draws a piano keyboard on screen and sends
// messages when the user clicks on keys.
/////////////////////////////////////////////////////////////////////////////
#pragma once

class KbdWindow :
	public CWindowImpl<KbdWindow>
{
private:
	int lastKey;
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
	KbdWindow();
	~KbdWindow();
	int FindKey(POINT& pt);
	void InvalidateLast();

	BEGIN_MSG_MAP(KbdWindow)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_LBUTTONDOWN , OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnBtnUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	END_MSG_MAP()

	void SetNotify(HWND w);
	void SetOctaves(int n);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class KeyboardDlg : public CDialogImpl<KeyboardDlg>, public CUpdateUI<KeyboardDlg>,
		public CMessageFilter, public CIdleHandler
{
private:
	KbdWindow kbd;
	CListBox instrList;
	CComboBox chnlList;
	CTrackBarCtrl volCtrl;
	InstrConfig *activeInstr;
	InstrConfig *selectInstr;
	bsInt32 evtID;
	AmpValue curVol;
	int curNoteVol;
	int curChnl;
	int curRhythm;
	FrqValue curDur;
	int kbdRunning;
	HANDLE genThreadH;
	DWORD  genThreadID;
	HBRUSH bgBrush;

	class RecNote : public SynthList<RecNote>
	{
	public:
		int key;
		int dur;
		int vol;
		RecNote(int k, int d, int v)
		{
			key = k;
			dur = d;
			vol = v;
		}
	};

	int recording;
	int recGroup;
	int useSharps;
	RecNote *recHead;
	RecNote *recTail;

	void PitchString(int pit, bsString& str);
	void RhythmString(int rhy, bsString& str);
	void NumberString(int val, bsString& str);
	void ClearNotes();
	long GetTimeValue(int id);
	void SetButtonImage(int ctrl, int imgid);

public:
	enum { IDD = IDD_KEYBOARD };

	KeyboardDlg();
	~KeyboardDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(KeyboardDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(KeyboardDlg)
//		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDlgColor)
//		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDlgColor)
//		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDlgColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_VKBD, OnKbd)
		MESSAGE_HANDLER(WM_HSCROLL, OnVolume)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_INSTRUMENT, LBN_SELCHANGE, OnInstrChange)
		COMMAND_HANDLER(IDC_CHANNEL, CBN_SELENDOK, OnChnlChange)
		COMMAND_HANDLER(IDC_NOTE_W, BN_CLICKED, OnWholeNote)
		COMMAND_HANDLER(IDC_NOTE_H, BN_CLICKED, OnHalfNote)
		COMMAND_HANDLER(IDC_NOTE_Q, BN_CLICKED, OnQuarterNote)
		COMMAND_HANDLER(IDC_NOTE_E, BN_CLICKED, OnEighthNote)
		COMMAND_HANDLER(IDC_NOTE_S, BN_CLICKED, OnSixteenthNote)
		COMMAND_HANDLER(IDC_START, BN_CLICKED, OnStart)
		COMMAND_HANDLER(IDC_STOP, BN_CLICKED, OnStop)
		COMMAND_HANDLER(IDC_GENERATE, BN_CLICKED, OnGenerate)
		COMMAND_HANDLER(IDC_RECORD, BN_CLICKED, OnRecordNotes)
		COMMAND_HANDLER(IDC_COPYNOTES, BN_CLICKED, OnCopyNotes)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnDlgColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVolume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChnlChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWholeNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHalfNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnQuarterNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEighthNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSixteenthNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnKbd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRecordNotes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyNotes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	int IsRunning() { return kbdRunning; }
	int Stop();
	int Start();
	void Clear();
	void InitInstrList();
	void AddInstrument(InstrConfig *ic);
	void RemoveInstrument(InstrConfig *ic);
	void UpdateInstrument(InstrConfig *ic);
	void SelectInstrument(InstrConfig *ic);
	void UpdateChannels();
	void StartRecord();
	void StopRecord();
	void CopyNotes();
};

#endif
