/////////////////////////////////////////////////////////////////////////////
// See MainDlg.cpp for an explanation
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
private:
	CTrackBarCtrl sigVol;
	CTrackBarCtrl sigAtk;
	CTrackBarCtrl sigSus;
	CTrackBarCtrl sigDec;
	CEdit         sigVolEd;
	CEdit         sigAtkEd;
	CEdit         sigSusEd;
	CEdit         sigDecEd;

	CTrackBarCtrl flngLvl;
	CTrackBarCtrl flngMix;
	CTrackBarCtrl flngFbk;
	CTrackBarCtrl flngCntr;
	CTrackBarCtrl flngDpth;
	CTrackBarCtrl flngFrq;
	CEdit         flngLvlEd;
	CEdit         flngMixEd;
	CEdit         flngFbkEd;
	CEdit         flngCntrEd;
	CEdit         flngDpthEd;
	CEdit         flngFrqEd;

	CButton       btnPlay;
	CButton       btnStop;
	CButton       btnLoop;
	CEdit         pitchEd;
	
#ifdef USE_DIRECTSOUND
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;
#else
	WAVEHDR wh;
	HWAVEOUT woHandle;
#endif
	UINT idTimer;

	Flanger  flng;
	AmpValue amp;
	GenWaveI sigOsc;
	EnvGenExp sigEG;
	WaveFileIn wvIn;
	bsInt32 wvfSampleTotal;
	bsInt32 wvfSampleNumber;
	AmpValue *wvfSamples;
	int sigFile;

public:
	enum { IDD = IDD_MAINDLG };

	CMainDlg();
	~CMainDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDC_HELP2, OnHelp2)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_PLAY, OnPlay)
		COMMAND_ID_HANDLER(IDC_LOOP, OnLoop)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
		COMMAND_ID_HANDLER(IDC_SAVEWAV, OnSaveWave)
		COMMAND_ID_HANDLER(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_ID_HANDLER(IDC_SIG_FILE, OnSigFile)
		COMMAND_ID_HANDLER(IDC_SIG_INTERNAL, OnSigFile)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		MESSAGE_HANDLER(MM_WOM_DONE, OnWaveDone)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHelp2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLoop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSaveWave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSigFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq, int def, double val);
	void TrackValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void UpdateValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void SetValue(CTrackBarCtrl& slid, CEdit& ed, double val, int pos);
	double GetSetting(CEdit& ed);
	double GetFrequency();
	double InitGen();
	AmpValue Generate();
	int Play(int loop);
	void StopPlaying();
	void CloseDialog(int nVal);
};

