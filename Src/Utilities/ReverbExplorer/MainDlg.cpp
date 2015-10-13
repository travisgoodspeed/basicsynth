/////////////////////////////////////////////////////////////////////////////
// Reverb explorer
//
// This program implements a configurable Shcroeder style reverb utilizing
// four parallel comb filtes and two series allpass filters.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "maindlg.h"

#ifdef USE_DIRECTSOUND
#if !USE_SDK_DSOUND
CLSID CLSID_DirectSound = {0x47d4d946, 0x62e8, 0x11cf, {0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0} };
IID IID_IDirectSound = {0x279AFA83, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60} };
typedef HRESULT (WINAPI *tDirectSoundCreate)(const GUID *pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
tDirectSoundCreate DirectSoundCreate;
#endif
#endif

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

CMainDlg::CMainDlg()
{
#ifdef USE_DIRECTSOUND
#if !USE_SDK_DSOUND
	DirectSoundCreate = (tDirectSoundCreate)GetProcAddress(LoadLibrary("dsound.dll"), "DirectSoundCreate");
#endif
	dirSndObj = NULL;
	dirSndBuf = NULL;
#else
	woHandle = 0;
#endif
	idTimer = 0;
	sigFile = 0;
}

CMainDlg::~CMainDlg()
{
#ifdef USE_DIRECTSOUND
	if (dirSndObj)
		dirSndObj->Release();
#else
    waveOutClose(woHandle); 
#endif
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	btnPlay = GetDlgItem(IDC_PLAY);
	btnStop = GetDlgItem(IDC_STOP);
	btnLoop = GetDlgItem(IDC_LOOP);
	RECT btnrc;
	btnPlay.GetClientRect(&btnrc);
	int cxy;
	if ((btnrc.bottom - btnrc.top) < 34)
		cxy = 16;
	else
		cxy = 32;
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnPlay.SetIcon(hIcon);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStop.SetIcon(hIcon);
	btnStop.EnableWindow(FALSE);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_LOOP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnLoop.SetIcon(hIcon);

	CButton btnStyle;
	btnStyle = GetDlgItem(IDC_SIN);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SIN), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SAWTOOTH);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SAW), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_RAMP);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_RAMP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SQUARE);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SQUARE), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);

	btnStyle = GetDlgItem(IDC_SAVEWAV);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_WVFILE), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_HELP2);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_HELP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_COPY);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_COPY), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);

	pitchEd = GetDlgItem(IDC_PITCH_ED);
	pitchEd.SetWindowText("48");

	sigVol = GetDlgItem(IDC_VOLUME);
	sigAtk = GetDlgItem(IDC_ATTACK);
	sigSus = GetDlgItem(IDC_SUSTAIN);
	sigDec = GetDlgItem(IDC_DECAY);
	sigVolEd = GetDlgItem(IDC_VOLUME_EDIT);
	sigAtkEd = GetDlgItem(IDC_ATTACK_EDIT);
	sigSusEd = GetDlgItem(IDC_SUSTAIN_EDIT);
	sigDecEd = GetDlgItem(IDC_DECAY_EDIT);

	InitValue(sigVol, sigVolEd, 0, 100,   10, 30, 0.7);
	InitValue(sigAtk, sigAtkEd, 0, 1000, 100,  50, 0.05);
	InitValue(sigSus, sigSusEd, 0, 5000, 500, 500, 0.5);
	InitValue(sigDec, sigDecEd, 0, 1000, 100, 100, 0.1);

	CheckDlgButton(IDC_SAWTOOTH, BST_CHECKED);
	CheckDlgButton(IDC_SIG_INTERNAL, BST_CHECKED);
	CheckDlgButton(IDC_SIG_FILE, BST_UNCHECKED);
	CheckDlgButton(IDC_SIG_INTERNAL, BST_CHECKED);

	rvbSend = GetDlgItem(IDC_RVB_SEND);
	rvbT1Del = GetDlgItem(IDC_T1);
	rvbT2Del = GetDlgItem(IDC_T2);
	rvbT3Del = GetDlgItem(IDC_T3);
	rvbT4Del = GetDlgItem(IDC_T4);
	rvbA1Del = GetDlgItem(IDC_A1);
	rvbA1Rvt = GetDlgItem(IDC_A1_RT);
	rvbA2Del = GetDlgItem(IDC_A2);
	rvbA2Rvt = GetDlgItem(IDC_A2_RT);
	rvbRegen = GetDlgItem(IDC_RVB_DEC);
	rvbMix = GetDlgItem(IDC_RVB_MIX);
	rvbSendEd = GetDlgItem(IDC_RVB_SEND_EDIT);
	rvbT1DelEd = GetDlgItem(IDC_T1_EDIT);
	rvbT2DelEd = GetDlgItem(IDC_T2_EDIT);
	rvbT3DelEd = GetDlgItem(IDC_T3_EDIT);
	rvbT4DelEd = GetDlgItem(IDC_T4_EDIT);
	rvbA1DelEd = GetDlgItem(IDC_A1_EDIT);
	rvbA1RvtEd = GetDlgItem(IDC_A1_RT_EDIT);
	rvbA2DelEd = GetDlgItem(IDC_A2_EDIT);
	rvbA2RvtEd = GetDlgItem(IDC_A2_RT_EDIT);
	rvbRegenEd = GetDlgItem(IDC_RVB_DEC_EDIT);
	rvbMixEd = GetDlgItem(IDC_RVB_MIX_EDIT);

	// Shroeder values .0297f, .0371f, .0411f, .0437f, .005f, .0017f
	InitValue(rvbSend,  rvbSendEd,  0,  100,  10,    0, 1.0);
	InitValue(rvbT4Del, rvbT4DelEd, 0, 2000, 200,  297, 0.0297);
	InitValue(rvbT3Del, rvbT3DelEd, 0, 2000, 200,  371, 0.0371);
	InitValue(rvbT2Del, rvbT2DelEd, 0, 2000, 200,  411, 0.0411);
	InitValue(rvbT1Del, rvbT1DelEd, 0, 2000, 200,  437, 0.0437);
	InitValue(rvbA1Del, rvbA1DelEd, 0, 2000, 200,  968, 0.0968);
	InitValue(rvbA1Rvt, rvbA1RvtEd, 0, 2000, 200,   50, 0.0050);
	InitValue(rvbA2Del, rvbA2DelEd, 0, 2000, 200,  329, 0.0329);
	InitValue(rvbA2Rvt, rvbA2RvtEd, 0, 2000, 200,   17, 0.0017);
	InitValue(rvbRegen, rvbRegenEd, 0, 4000, 400, 1000, 1.0);
	InitValue(rvbMix,   rvbMixEd,   0,  100,  10,   90, 0.1);

	CheckDlgButton(IDC_T1_ON, BST_CHECKED);
	CheckDlgButton(IDC_T2_ON, BST_CHECKED);
	CheckDlgButton(IDC_T3_ON, BST_CHECKED);
	CheckDlgButton(IDC_T4_ON, BST_CHECKED);
	CheckDlgButton(IDC_A1_ON, BST_CHECKED);
	CheckDlgButton(IDC_A2_ON, BST_CHECKED);

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnHelp2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHelpDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseDialog(0);
	return 0;
}


LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq, int def, double val)
{
	slid.SetRange(minval, maxval);
	slid.SetTicFreq(ticfrq);
	slid.SetLineSize(1);
	slid.SetPageSize(ticfrq);
	SetValue(slid, ed, val, def);
}

void CMainDlg::TrackValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv)
{
	int val = slid.GetPos();
	if (inv)
		val = inv - val;
	char valstr[40];
	sprintf(valstr, "%6.4f", (double) val / div);
	ed.SetWindowText(valstr);
}

LRESULT CMainDlg::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int code = LOWORD(wParam);
	if (code == TB_ENDTRACK || code == TB_THUMBTRACK)
	{
		HWND wnd = (HWND) lParam;
		if (wnd == sigVol)
			TrackValue(sigVol, sigVolEd, 100.0, 100);
		else if (wnd == sigAtk)
			TrackValue(sigAtk, sigAtkEd, 1000.0);
		else if (wnd == sigSus)
			TrackValue(sigSus, sigSusEd, 1000.0);
		else if (wnd == sigDec)
			TrackValue(sigDec, sigDecEd, 1000.0);
		else if (wnd == rvbSend)
			TrackValue(rvbSend, rvbSendEd, 100.0, 100);
		else if (wnd == rvbT1Del)
			TrackValue(rvbT1Del, rvbT1DelEd, 10000.0);
		else if (wnd == rvbT2Del)
			TrackValue(rvbT2Del, rvbT2DelEd, 10000.0);
		else if (wnd == rvbT3Del)
			TrackValue(rvbT3Del, rvbT3DelEd, 10000.0);
		else if (wnd == rvbT4Del)
			TrackValue(rvbT4Del, rvbT4DelEd, 10000.0);
		else if (wnd == rvbA1Del)
			TrackValue(rvbA1Del, rvbA1DelEd, 10000.0);
		else if (wnd == rvbA1Rvt)
			TrackValue(rvbA1Rvt, rvbA1RvtEd, 10000.0);
		else if (wnd == rvbA2Del)
			TrackValue(rvbA2Del, rvbA2DelEd, 10000.0);
		else if (wnd == rvbA2Rvt)
			TrackValue(rvbA2Rvt, rvbA2RvtEd, 10000.0);
		else if (wnd == rvbRegen)
			TrackValue(rvbRegen, rvbRegenEd, 1000.0);
		else if (wnd == rvbMix)
			TrackValue(rvbMix, rvbMixEd, 100.0, 100);
	}
	return 0;
}

void CMainDlg::UpdateValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv)
{
	char valstr[40];
	ed.GetWindowText(valstr, 40);
	double val = atof(valstr);
	if (inv)
		SetValue(slid, ed, val, inv - (int) (val * div));
	else
		SetValue(slid, ed, val, (int) (val * div));
}

LRESULT CMainDlg::OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch (wID)
	{
	case IDC_VOLUME_EDIT:
		UpdateValue(sigVol, sigVolEd, 100.0, 100);
		break;
	case IDC_ATTACK_EDIT:
		UpdateValue(sigAtk, sigAtkEd, 1000.0, 0);
		break;
	case IDC_SUSTAIN_EDIT:
		UpdateValue(sigSus, sigSusEd, 1000.0, 0);
		break;
	case IDC_DECAY_EDIT:
		UpdateValue(sigDec, sigDecEd, 1000.0, 0);
		break;
	case IDC_RVB_SEND_EDIT:
		UpdateValue(rvbSend, rvbSendEd, 100.0, 100);
		break;
	case IDC_T1_EDIT:
		UpdateValue(rvbT1Del, rvbT1DelEd, 10000.0, 0);
		break;
	case IDC_T2_EDIT:
		UpdateValue(rvbT2Del, rvbT2DelEd, 10000.0, 0);
		break;
	case IDC_T3_EDIT:
		UpdateValue(rvbT3Del, rvbT3DelEd, 10000.0, 0);
		break;
	case IDC_T4_EDIT:
		UpdateValue(rvbT4Del, rvbT4DelEd, 10000.0, 0);
		break;
	case IDC_A1_EDIT:
		UpdateValue(rvbA1Del, rvbA1DelEd, 10000.0, 0);
		break;
	case IDC_A1_RT_EDIT:
		UpdateValue(rvbA1Rvt, rvbA1RvtEd, 10000.0, 0);
		break;
	case IDC_A2_EDIT:
		UpdateValue(rvbA2Del, rvbA2DelEd, 10000.0, 0);
		break;
	case IDC_A2_RT_EDIT:
		UpdateValue(rvbA2Rvt, rvbA2RvtEd, 10000.0, 0);
		break;
	case IDC_RVB_DEC_EDIT:
		UpdateValue(rvbRegen, rvbRegenEd, 1000.0, 0);
		break;
	case IDC_RVB_MIX_EDIT:
		UpdateValue(rvbMix, rvbMixEd, 100.0, 100);
		break;
	}

	return 0;
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnStop(uMsg, IDC_STOP, NULL, bHandled);
	return 0;
}


LRESULT CMainDlg::OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (Play(FALSE) == 0)
	{
		btnStop.EnableWindow(TRUE);
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
	}
	return 0;
}

LRESULT CMainDlg::OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (Play(TRUE) == 0)
	{
		btnStop.EnableWindow(TRUE);
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
	}
	return 0;
}

LRESULT CMainDlg::OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	StopPlaying();
	btnStop.EnableWindow(FALSE);
	btnPlay.EnableWindow(TRUE);
	btnLoop.EnableWindow(TRUE);
	return 0;
}

LRESULT CMainDlg::OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UINT err = OpenClipboard();
	if (err == 0)
		return 0;

	EmptyClipboard();

	FrqValue tdel[9];
	tdel[0] = GetSetting(rvbT1DelEd);
	tdel[1] = GetSetting(rvbT2DelEd);
	tdel[2] = GetSetting(rvbT3DelEd);
	tdel[3] = GetSetting(rvbT4DelEd);
	tdel[4] = GetSetting(rvbA1DelEd);
	tdel[5] = GetSetting(rvbA2DelEd);
	tdel[6] = GetSetting(rvbRegenEd);
	tdel[7] = GetSetting(rvbA1RvtEd);
	tdel[8] = GetSetting(rvbA2RvtEd);
	FrqValue mixVal  = GetSetting(rvbMixEd);

	char txt[1024];
	sprintf(txt, "LT1     LT2     LT3     LT4     AT1     AT2     RVT     ART1    ART1    MIX\r\n"
		"%6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f\r\n",
		tdel[0], tdel[1], tdel[2], tdel[3], tdel[4], tdel[5], tdel[6], tdel[7], tdel[8], mixVal);

	HANDLE mem = GlobalAlloc(0, strlen(txt)+1);
	char *ptxt = (char*)GlobalLock(mem);
	strcpy(ptxt, txt);
	GlobalUnlock(mem);

	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();

	return 0;
}

LRESULT CMainDlg::OnSaveWave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	char fileName[MAX_PATH];
	memset(fileName, 0, sizeof(fileName));
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = "Sound Files(*.wav)\0*.wav\0All Files(*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = fileName;
	ofn.lpstrDefExt = ".wav";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	
	if (GetSaveFileName (&ofn))
	{
		double dur = InitGen();
		WaveFile wvf;
		wvf.OpenWaveFile(fileName, 2);
		long totalSamples = (long) (dur * synthParams.sampleRate);
		for (long n = 0; n < totalSamples; n++)
			wvf.Output1(Generate());
		wvf.CloseWaveFile();
	}
	return 0;
}

void CMainDlg::SetValue(CTrackBarCtrl& slid, CEdit& ed, double val, int pos)
{
	char valstr[40];
	sprintf(valstr, "%6.4f", (double) val);
	ed.SetWindowText(valstr);
	slid.SetPos(pos);
}


LRESULT CMainDlg::OnPreset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDC_PRESET1:
		SetValue(rvbT1Del, rvbT1DelEd, 0.0437, 437);
		SetValue(rvbT2Del, rvbT2DelEd, 0.0411, 411);
		SetValue(rvbT3Del, rvbT3DelEd, 0.0371, 371);
		SetValue(rvbT4Del, rvbT4DelEd, 0.0297, 297);
		break;
	case IDC_PRESET2:
		SetValue(rvbT1Del, rvbT1DelEd, 0.179, 1790);
		SetValue(rvbT2Del, rvbT2DelEd, 0.097,  970);
		SetValue(rvbT3Del, rvbT3DelEd, 0.041,  410);
		SetValue(rvbT4Del, rvbT4DelEd, 0.023,  230);
		break;
	default:
		return 0;
	}

	return 0;
}

LRESULT CMainDlg::TnOn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int on = IsDlgButtonChecked(wID) == BST_CHECKED ? 1 : 0;
	switch (wID)
	{
	case IDC_T1_ON:
		enbT1 = on;
		break;
	case IDC_T2_ON:
		enbT2 = on;
		break;
	case IDC_T3_ON:
		enbT3 = on;
		break;
	case IDC_T4_ON:
		enbT4 = on;
		break;
	case IDC_A1_ON:
		enbA1 = on;
		break;
	case IDC_A2_ON:
		enbA2 = on;
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnSigInternal(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wNotifyCode == BN_CLICKED)
	{
		sigFile = FALSE;
		CheckDlgButton(IDC_SIG_FILE, BST_UNCHECKED);
		CheckDlgButton(IDC_SIG_INTERNAL, BST_CHECKED);
	}

	return 0;
}

LRESULT CMainDlg::OnSigFile(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wNotifyCode == BN_CLICKED)
	{
		sigFile = TRUE;
		CheckDlgButton(IDC_SIG_FILE, BST_CHECKED);
		CheckDlgButton(IDC_SIG_INTERNAL, BST_UNCHECKED);
	}

	return 0;
}

LRESULT CMainDlg::OnLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	char fileName[MAX_PATH];
	fileName[0] = 0;
	GetDlgItemText(IDC_SIG_FILENAME, fileName, MAX_PATH);
	if (fileName[0] == 0)
		return 0;
	if (wvIn.LoadWaveFile(fileName, 1) == 0)
	{
		sigFile = 1;
		CheckDlgButton(IDC_SIG_FILE, BST_CHECKED);
		CheckDlgButton(IDC_SIG_INTERNAL, BST_UNCHECKED);
	}
	return 0;
}

LRESULT CMainDlg::OnBrowse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	char fileName[MAX_PATH];
	memset(fileName, 0, sizeof(fileName));
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = "Sound Files(*.wav)\0*.wav\0All Files(*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = fileName;
	ofn.lpstrDefExt = ".wav";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	
	if (GetOpenFileName (&ofn))
	{
		SetDlgItemText(IDC_SIG_FILENAME, fileName);
	}

	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

double CMainDlg::GetFrequency()
{
	char valstr[40];
	valstr[0] = 0;
	pitchEd.GetWindowText(valstr, 40);
	int pit = atoi(valstr);
	return (double) synthParams.GetFrequency(pit);
}

double CMainDlg::GetSetting(CEdit& ed)
{
	char varstr[40];
	ed.GetWindowText(varstr, 40);
	return atof(varstr);
}
	
double CMainDlg::InitGen()
{
	FrqValue frq;
	FrqValue atk;
	FrqValue dec;
	FrqValue dur;

	if (sigFile)
	{
		wvfSamples = wvIn.GetSampleBuffer();
		wvfSampleTotal = wvIn.GetInputLength();
		wvfSampleNumber = 0;
		dur = (float) wvfSampleTotal / synthParams.sampleRate;
		amp = GetSetting(sigVolEd);
	}
	else
	{
		frq = GetFrequency();

		amp = GetSetting(sigVolEd);
		atk = GetSetting(sigAtkEd);
		dec = GetSetting(sigDecEd);
		dur = GetSetting(sigSusEd) + atk + dec;
		sigEG.InitEG(amp, dur, atk, dec);
		int wtIndex;
		if (IsDlgButtonChecked(IDC_SIN))
			wtIndex = WT_SIN;
		else if (IsDlgButtonChecked(IDC_SAWTOOTH))
			wtIndex = WT_SAW;
		else if (IsDlgButtonChecked(IDC_SQUARE))
			wtIndex = WT_SQR;
		else if (IsDlgButtonChecked(IDC_RAMP))
			wtIndex = WT_TRI;
		sigOsc.InitWT(frq, wtIndex);
	}

	FrqValue tdel[9];
	tdel[0] = GetSetting(rvbT1DelEd);
	tdel[1] = GetSetting(rvbT2DelEd);
	tdel[2] = GetSetting(rvbT3DelEd);
	tdel[3] = GetSetting(rvbT4DelEd);
	tdel[4] = GetSetting(rvbA1DelEd);
	tdel[5] = GetSetting(rvbA2DelEd);
	tdel[6] = GetSetting(rvbRegenEd);
	tdel[7] = GetSetting(rvbA1RvtEd);
	tdel[8] = GetSetting(rvbA2RvtEd);
	enbT1 = IsDlgButtonChecked(IDC_T1_ON) == BST_CHECKED ? 1 : 0;
	enbT2 = IsDlgButtonChecked(IDC_T2_ON) == BST_CHECKED ? 1 : 0;
	enbT3 = IsDlgButtonChecked(IDC_T3_ON) == BST_CHECKED ? 1 : 0;
	enbT4 = IsDlgButtonChecked(IDC_T4_ON) == BST_CHECKED ? 1 : 0;
	enbA1 = IsDlgButtonChecked(IDC_A1_ON) == BST_CHECKED ? 1 : 0;
	enbA2 = IsDlgButtonChecked(IDC_A2_ON) == BST_CHECKED ? 1 : 0;

	dlr[0].InitDLR(tdel[0], tdel[6], 0.001);
	dlr[1].InitDLR(tdel[1], tdel[6], 0.001);
	dlr[2].InitDLR(tdel[2], tdel[6], 0.001);
	dlr[3].InitDLR(tdel[3], tdel[6], 0.001);
	ap[0].InitDLR(tdel[4], tdel[7], 0.001); 
	ap[1].InitDLR(tdel[5], tdel[8], 0.001);

	sendVal = GetSetting(rvbSendEd);
	mixVal  = GetSetting(rvbMixEd);

	prev = 0;

	return dur + tdel[6];
}

AmpValue CMainDlg::Generate()
{
	AmpValue sampl;
	if (sigFile)
	{
		if (wvfSampleNumber >= wvfSampleTotal)
			sampl = 0;
		else
			sampl = wvfSamples[wvfSampleNumber++] * amp;
	}
	else
	{
		if (sigEG.IsFinished())
			sampl = 0;
		else
			sampl = sigEG.Gen() * sigOsc.Gen();
	}
	AmpValue rvbOut = 0;
	AmpValue rvbIn = sampl * sendVal;
	if (enbT1)
		rvbOut += dlr[0].Sample(rvbIn);
	if (enbT2)
		rvbOut += dlr[1].Sample(rvbIn);
	if (enbT3)
		rvbOut += dlr[2].Sample(rvbIn);
	if (enbT4)
		rvbOut += dlr[3].Sample(rvbIn);
	if (enbA1)
		rvbOut = ap[0].Sample(rvbOut);
	if (enbA2)
		rvbOut = ap[1].Sample(rvbOut);

	sampl = (sampl * (1.0 - mixVal)) + (rvbOut * mixVal);
	if (sampl > 1.0)
		sampl = 1.0;
	else if (sampl < -1.0)
		sampl = -1.0;
	return sampl;
}

int CMainDlg::Play(int loop)
{
	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
    wf.nSamplesPerSec = synthParams.isampleRate;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * 2;
	wf.nBlockAlign = 2; 
    wf.wBitsPerSample = 16;
	wf.cbSize = 0;

	float dur = InitGen();

	DWORD totalSamples = (DWORD) (dur * synthParams.sampleRate);
	DWORD bufSize = totalSamples;// + (DWORD) (0.01 * synthParams.isampleRate);
	SampleValue *samples;

#ifdef USE_DIRECTSOUND
	HRESULT hr;
	if (dirSndObj == NULL)
	{
		hr = DirectSoundCreate(NULL, &dirSndObj, NULL);
		if (hr == S_OK)
		{
			hr = dirSndObj->SetCooperativeLevel(m_hWnd, DSSCL_NORMAL);
			if (hr != S_OK)
			{
				dirSndObj->Release();
				dirSndObj = NULL;
				return -1;
			}
		}
	}
	void *pAudio1 = NULL;
	void *pAudio2 = NULL;
	DWORD dwAudio1 = 0;
	DWORD dwAudio2 = 0;

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = 0; 
	dsbd.dwBufferBytes = bufSize * 2;
	dsbd.dwReserved = 0; 
	dsbd.lpwfxFormat = &wf;
	
	hr = dirSndObj->CreateSoundBuffer(&dsbd, &dirSndBuf, NULL);
	if (hr != S_OK)
		return -1;
	hr = dirSndBuf->Lock(0, bufSize, &pAudio1, &dwAudio1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (hr != S_OK)
		return -1;
	samples = (SampleValue *) pAudio1;
#else
	MMRESULT res; 
	if (woHandle == NULL)
	{
		res = waveOutOpen(&woHandle, WAVE_MAPPER, &wf, (DWORD) m_hWnd, 0, CALLBACK_WINDOW);
		if (res != MMSYSERR_NOERROR)
		{
			MessageBox("Cannot open wave output", "Error", MB_OK|MB_ICONSTOP);
			return -1;
		}
	}
	memset(&wh, 0, sizeof(wh));
	wh.dwBufferLength = bufSize * sizeof(SampleValue);
	wh.lpData = (LPSTR) malloc(wh.dwBufferLength);
	res = waveOutPrepareHeader(woHandle, &wh, sizeof(wh));
	samples = (SampleValue *) wh.lpData;
#endif

	DWORD n;
	for (n = 0; n < totalSamples; n++)
	{
		float v = Generate();
		*samples++ = (short) (32767.0 * v);
	}
	while (n++ < bufSize)
		*samples++ = 0;

#ifdef USE_DIRECTSOUND
	dirSndBuf->Unlock(pAudio1, dwAudio1, pAudio2, dwAudio2);
	dirSndBuf->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
	if (!loop)
		idTimer = SetTimer(1, (UINT) (dur * 1000.0) + 500);
#else
	if (loop)
	{
		wh.dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
		wh.dwLoops = 100;
	}
	waveOutWrite(woHandle, &wh, sizeof(wh));
#endif

	return 0;
}

void CMainDlg::StopPlaying()
{
	if (idTimer)
	{
		KillTimer(idTimer);
		idTimer = 0;
	}

#ifdef USE_DIRECTSOUND
	if (dirSndBuf != NULL)
	{
		dirSndBuf->Stop();
		dirSndBuf->Release();
		dirSndBuf = NULL;
	}
#else
	waveOutReset(woHandle);
#endif
}

LRESULT CMainDlg::OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#ifndef USE_DIRECTSOUND
    waveOutUnprepareHeader((HWAVEOUT) wParam, (LPWAVEHDR) lParam, sizeof(WAVEHDR)); 
	free(wh.lpData);
	memset(&wh, 0, sizeof(wh));
	btnStop.EnableWindow(FALSE);
	btnPlay.EnableWindow(TRUE);
	btnLoop.EnableWindow(TRUE);
#endif
	return 0;
}
