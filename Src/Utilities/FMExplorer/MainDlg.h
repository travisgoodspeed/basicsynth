// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

struct FMInstrParam
{
	int algorithm;
	AmpValue gen1Start;
	FrqValue gen1Atk;
	AmpValue gen1Pk;
	FrqValue gen1Dec;
	AmpValue gen1Sus;
	FrqValue gen1Rel;
	AmpValue gen1End;

	FrqValue gen2Mult;
	AmpValue gen2Start;
	FrqValue gen2Atk;
	AmpValue gen2Pk;
	FrqValue gen2Dec;
	AmpValue gen2Sus;
	FrqValue gen2Rel;
	AmpValue gen2End;

	FrqValue gen3Mult;
	AmpValue gen3Start;
	FrqValue gen3Atk;
	AmpValue gen3Pk;
	FrqValue gen3Dec;
	AmpValue gen3Sus;
	FrqValue gen3Rel;
	AmpValue gen3End;
};

class CMainDlg : public CDialogImpl<CMainDlg>, 
	public CUpdateUI<CMainDlg>,
	public CMessageFilter, 
	public CIdleHandler
{
private:
	CTrackBarCtrl gen1Frq;
	CTrackBarCtrl gen1Atk;
	CTrackBarCtrl gen1Vol;
	CTrackBarCtrl gen1Dec;
	CTrackBarCtrl gen1Sus;
	CTrackBarCtrl gen1Rel;
	CEdit gen1FrqEd;
	CEdit gen1AtkEd;
	CEdit gen1VolEd;
	CEdit gen1DecEd;
	CEdit gen1SusEd;
	CEdit gen1RelEd;

	CTrackBarCtrl gen2Mul;
	CTrackBarCtrl gen2Ndx;
	CTrackBarCtrl gen2Atk;
	CTrackBarCtrl gen2Pck;
	CTrackBarCtrl gen2Dec;
	CTrackBarCtrl gen2Sus;
	CTrackBarCtrl gen2Rel;
	CTrackBarCtrl gen2End;
	CEdit gen2MulEd;
	CEdit gen2NdxEd;
	CEdit gen2AtkEd;
	CEdit gen2PckEd;
	CEdit gen2DecEd;
	CEdit gen2SusEd;
	CEdit gen2RelEd;
	CEdit gen2EndEd;

	CTrackBarCtrl gen3Mul;
	CTrackBarCtrl gen3Ndx;
	CTrackBarCtrl gen3Atk;
	CTrackBarCtrl gen3Pck;
	CTrackBarCtrl gen3Dec;
	CTrackBarCtrl gen3Sus;
	CTrackBarCtrl gen3Rel;
	CTrackBarCtrl gen3End;
	CEdit gen3MulEd;
	CEdit gen3NdxEd;
	CEdit gen3AtkEd;
	CEdit gen3PckEd;
	CEdit gen3DecEd;
	CEdit gen3SusEd;
	CEdit gen3RelEd;
	CEdit gen3EndEd;

	CEdit durValEd;
	CEdit volValEd;
	FrqValue durTotal;  // duration including relase
	FrqValue durAtkSus; // duration through sustain
	AmpValue volMaster;

	CButton btnPlay;
	CButton btnLoop;
	CButton btnStop;

	int algorithm;
	double gen2Scale;
	int gen2SlidRange;
	// Fast oscillator (32-bit phase accumulator):
	//GenWave32 gen1Osc;
	//GenWave32 gen2Osc;
	//GenWave32 gen3Osc;
	// Interpolating oscillator (slightly less noise):
	GenWaveI gen1Osc;
	GenWaveI gen2Osc;
	GenWaveI gen3Osc;
	EnvGenADSR gen1EG;
	EnvGenADSR gen2EG;
	EnvGenADSR gen3EG;

#ifdef USE_DIRECTSOUND
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;
#else
	WAVEHDR wh;
	HWAVEOUT woHandle;
#endif
	UINT idTimer;

public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDC_PLAY, OnPlay)
		COMMAND_ID_HANDLER(IDC_LOOP, OnLoop)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_ID_HANDLER(IDC_SAVE, OnSave)
		COMMAND_ID_HANDLER(IDC_STACK, OnStack)
		COMMAND_ID_HANDLER(IDC_STACK2, OnStack2)
		COMMAND_ID_HANDLER(IDC_WYE, OnWye)
		COMMAND_ID_HANDLER(IDC_DELTA, OnDelta)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_HELP2, OnHelp)
		COMMAND_ID_HANDLER(IDC_COPY_CLIP, OnCopyClip)
		COMMAND_ID_HANDLER(IDC_PRE_PIANO, OnPresetPiano)
		COMMAND_ID_HANDLER(IDC_PRE_ORGAN, OnPresetOrgan)
		COMMAND_ID_HANDLER(IDC_PRE_STRINGS, OnPresetStrings)
		COMMAND_ID_HANDLER(IDC_PRE_BRASS, OnPresetBrass)
		COMMAND_ID_HANDLER(IDC_PRE_CLAR, OnPresetClarinet)
		COMMAND_ID_HANDLER(IDC_PRE_FLUTE, OnPresetFlute)
		COMMAND_ID_HANDLER(IDC_PRE_SYNTH, OnPresetSynth)
		COMMAND_ID_HANDLER(IDC_PRE_PLUCK, OnPresetPlucked)
		COMMAND_ID_HANDLER(IDC_PRE_BASS, OnPresetBass)
		COMMAND_ID_HANDLER(IDC_PRE_RESET, OnPresetReset)
		COMMAND_ID_HANDLER(IDC_PRE_BELL1, OnPresetBell1)
		COMMAND_ID_HANDLER(IDC_PRE_BELL2, OnPresetBell2)
		COMMAND_ID_HANDLER(IDC_PRE_BELL3, OnPresetBell3)
		COMMAND_ID_HANDLER(IDC_PRE_WOOD, OnPresetWood)
		MESSAGE_HANDLER(MM_WOM_DONE, OnWaveDone)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStack2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWye(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelta(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnPresetPiano(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetStrings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBrass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetClarinet(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetOrgan(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetFlute(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetSynth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetPlucked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetReset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell1(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetBell3(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetWood(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void CloseDialog(int nVal);
	//void InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq, int def, double val);
	void InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq);
	void TrackValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void UpdateValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv = 0);
	void SetPreset(FMInstrParam *p);
	float GetSetting(CEdit& ed);
	void PutSetting(CTrackBarCtrl& slid, CEdit& ed, double val, double div, int inv = 0);
	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult);
	void InitGen();
	AmpValue Generate();
	void NoteOff();
	int PlayFM(int loop);
	void StopPlaying();
};

