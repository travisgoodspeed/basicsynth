/////////////////////////////////////////////////////////////////////////////
// Waveform explore main window. See MainDlg.cpp for details.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "WavePlot.h"

class CMainDlg : 
	public CDialogImpl<CMainDlg>, 
	public CUpdateUI<CMainDlg>,
	public CMessageFilter, 
	public CIdleHandler
{
private:
	WTL::CTrackBarCtrl sliders[WFI_MAXPART];
	WTL::CEdit         levels[WFI_MAXPART];
	WTL::CButton       btnPlay;
	WTL::CButton       btnStop;
	WTL::CButton       btnLoop;
	WTL::CEdit         pitchEd;
	CWavePlot wndPlot;
	int gibbs;
	double picWidth;
	double picHeight;
	int picInchOrMM;
	char picFileName[MAX_PATH];
	GenWaveI wv;
	EnvGen eg;
#ifdef USE_DIRECTSOUND
	IDirectSound *dirSndObj;
	IDirectSoundBuffer *dirSndBuf;
#else
	WAVEHDR wh;
	HWAVEOUT woHandle;
#endif
	UINT idTimer;

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
		COMMAND_ID_HANDLER(IDC_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(IDC_SIN, OnSin)
		COMMAND_ID_HANDLER(IDC_SAWTOOTH, OnSawtooth)
		COMMAND_ID_HANDLER(IDC_SQUARE, OnSquare)
		COMMAND_ID_HANDLER(IDC_RAMP, OnRamp)
		COMMAND_ID_HANDLER(IDC_PLAY, OnPlay)
		COMMAND_ID_HANDLER(IDC_LOOP, OnLoop)
		COMMAND_ID_HANDLER(IDC_STOP, OnStop)
		COMMAND_ID_HANDLER(IDC_GIBBS, OnGibbs)
		COMMAND_ID_HANDLER(IDC_COPYCLIP, OnCopyClip)
		COMMAND_ID_HANDLER(IDC_SAVEPIC, OnSavePic)
		COMMAND_ID_HANDLER(IDC_SAVEWAV, OnSaveWave)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
		COMMAND_ID_HANDLER(IDC_HELP2, OnHelp)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(MM_WOM_DONE, OnWaveDone)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGibbs(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSawtooth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRamp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSquare(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSavePic(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSaveWave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	double GetFrequency();
	void CloseDialog(int nVal);
	void UpdatePlot();
	void SetPartValue(int ndx, double dval);
	int Play(double frq, double dur, int loop);
	int Stop();
};
