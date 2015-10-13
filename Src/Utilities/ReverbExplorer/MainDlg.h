/////////////////////////////////////////////////////////////////////////////
// Reverb explorer main window
// See MainDlg.cpp for details.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
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
	
	CTrackBarCtrl rvbSend;
	CTrackBarCtrl rvbT1Del;
	CTrackBarCtrl rvbT2Del;
	CTrackBarCtrl rvbT3Del;
	CTrackBarCtrl rvbT4Del;
	CTrackBarCtrl rvbA1Del;
	CTrackBarCtrl rvbA1Rvt;
	CTrackBarCtrl rvbA2Del;
	CTrackBarCtrl rvbA2Rvt;
	CTrackBarCtrl rvbRegen;
	CTrackBarCtrl rvbMix;
	CEdit rvbSendEd;
	CEdit rvbT1DelEd;
	CEdit rvbT2DelEd;
	CEdit rvbT3DelEd;
	CEdit rvbT4DelEd;
	CEdit rvbA1DelEd;
	CEdit rvbA1RvtEd;
	CEdit rvbA2DelEd;
	CEdit rvbA2RvtEd;
	CEdit rvbRegenEd;
	CEdit rvbMixEd;

	CButton       btnPlay;
	CButton       btnStop;
	CButton       btnLoop;

	CEdit         pitchEd;

	UINT idTimer;
#ifdef USE_DIRECTSOUND
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;
#else
	WAVEHDR wh;
	HWAVEOUT woHandle;
#endif

	GenWaveI sigOsc;
	EnvGenExp sigEG;
	DelayLineR dlr[4];
	AllPassDelay ap[2];

	WaveFileIn wvIn;
	bsInt32 wvfSampleTotal;
	bsInt32 wvfSampleNumber;
	AmpValue *wvfSamples;

	DelayLineT dlt;
	float sendVal;
	float mixVal;
	float prev;
	float amp;

	int enbT1;
	int enbT2;
	int enbT3;
	int enbT4;
	int enbA1;
	int enbA2;

	int sigFile;

public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(IDC_PLAY, OnPlay)
		COMMAND_ID_HANDLER(IDC_LOOP, OnLoop)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopyClip)
		COMMAND_ID_HANDLER(IDC_SAVEWAV, OnSaveWave)
		COMMAND_ID_HANDLER(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_ID_HANDLER(IDC_SIG_FILE, OnSigFile)
		COMMAND_ID_HANDLER(IDC_SIG_INTERNAL, OnSigInternal)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDC_HELP2, OnHelp2)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_PRESET1, OnPreset)
		COMMAND_ID_HANDLER(IDC_PRESET2, OnPreset)
		COMMAND_ID_HANDLER(IDC_T1_ON, TnOn)
		COMMAND_ID_HANDLER(IDC_T2_ON, TnOn)
		COMMAND_ID_HANDLER(IDC_T3_ON, TnOn)
		COMMAND_ID_HANDLER(IDC_T4_ON, TnOn)
		COMMAND_ID_HANDLER(IDC_A1_ON, TnOn)
		COMMAND_ID_HANDLER(IDC_A2_ON, TnOn)
		MESSAGE_HANDLER(MM_WOM_DONE, OnWaveDone)
//		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSaveWave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSigInternal(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSigFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPreset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT TnOn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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

