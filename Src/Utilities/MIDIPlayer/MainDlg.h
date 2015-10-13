// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"
#include "aboutdlg.h"
#include "KbdWindow.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	HANDLE gm;
	int paused;
	int running;
	int playing;
	int loaded;
	int midiAvailable;
	int sf2Bank;
	int sf2Preset;
	bsInt32 sampleRate;
	KbdWindow kbd;
	int kbdChannel;
	CString sf2File;
	CString midFile;
	CString wavFile;

	enum { IDD = IDD_MAINDLG };

	CMainDlg()
	{
		loaded = 0;
		running = 0;
		playing = 0;
		paused = 0;
		gm = 0;
		sampleRate = 22050;
		sf2Bank = 0;
		sf2Preset = 0;
		kbdChannel = 0;
		midiAvailable = 0;
	}
	~CMainDlg()
	{
		gm = 0;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	void ShutDown();
	void Clear();
	void KeyboardOff();
	void EnableButtons();
	float GetTime(int id);
	void ShowTime(int id);
	void SampleRate();
	int BrowseFile(int open, char *file, const char *spec, const char *ext);
	void BankSelected();
	void PresetSelected();
	void SetBank();
	static void SynthMonitor(bsInt32 evtid, bsInt32 count, Opaque arg);

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_PLAY, BN_CLICKED, OnBnClickedPlay)
		COMMAND_HANDLER(IDC_GENERATE, BN_CLICKED, OnBnClickedGenerate)
		COMMAND_HANDLER(IDC_STOP, BN_CLICKED, OnBnClickedStop)
		COMMAND_HANDLER(IDC_PAUSE, BN_CLICKED, OnBnClickedPause)
		MESSAGE_HANDLER(WM_USER+10, OnStartStop)
		MESSAGE_HANDLER(WM_USER+11, OnTick)
		MESSAGE_HANDLER(WM_USER+12, OnPauseResume)
		MESSAGE_HANDLER(WM_VKBD, OnKbd)
		COMMAND_HANDLER(IDC_LOAD, BN_CLICKED, OnBnClickedLoad)
		COMMAND_HANDLER(IDC_RESUME, BN_CLICKED, OnBnClickedResume)
		COMMAND_HANDLER(IDC_LOADSB, BN_CLICKED, OnBnClickedLoadsb)
		COMMAND_HANDLER(IDC_22K, BN_CLICKED, OnBnClicked22k)
		COMMAND_HANDLER(IDC_44K, BN_CLICKED, OnBnClicked44k)
		MESSAGE_HANDLER(WM_HSCROLL, OnVolume)
		COMMAND_HANDLER(IDC_KBD_ON, BN_CLICKED, OnKbdOn)
//		COMMAND_HANDLER(IDC_MIDI_ON, BN_CLICKED, OnMidiOn)
		COMMAND_HANDLER(IDC_BANK, LBN_SELCHANGE, OnBank)
		COMMAND_HANDLER(IDC_PRESET, LBN_SELCHANGE, OnPreset)
		NOTIFY_HANDLER(IDC_CHANNEL_SPIN, UDN_DELTAPOS, OnDeltaposChannelSpin)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

	LRESULT OnStartStop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPauseResume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedMidiBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedSf2Browse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedLoadsb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedWavBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClicked22k(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClicked44k(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnVolume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKbdOn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnKbd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBank(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDeltaposChannelSpin(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
};
