/////////////////////////////////////////////////////////////////////////////
// BasicSynth Flanger Explorer
//
// This program implements a Flanger/Chorus unit. 
// This setup provides several variable parameters. Depending on the settings, 
// the flanger can produce a vibrato, chorus, flanger, or a siren-like warble effect. 
// Depth values for a flanger effect will typically vary from 1 to 10ms with a center 
// delay of 10ms or less and a very slow (.15 Hz) sweep rate. Chorus typically uses 
// very small depth values but greater center values and faster sweep rates. Longer 
// delays and wider range of delays produces a more dramatic effect, as does 
// increasing the mixture of the delayed signal. As the center delay time increases, 
// the sound acquires a distinct echo.
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
	InitValue(sigSus, sigSusEd, 0, 5000, 500, 1500, 1.5);
	InitValue(sigDec, sigDecEd, 0, 1000, 100, 100, 0.1);

	flngLvl = GetDlgItem(IDC_LVL);
	flngMix = GetDlgItem(IDC_MIX);
	flngFbk = GetDlgItem(IDC_FEEDBACK);
	flngCntr = GetDlgItem(IDC_CENTER);
	flngDpth = GetDlgItem(IDC_DEPTH);
	flngFrq = GetDlgItem(IDC_SWEEP);
	flngLvlEd = GetDlgItem(IDC_LVL_EDIT);
	flngMixEd = GetDlgItem(IDC_MIX_EDIT);
	flngFbkEd = GetDlgItem(IDC_FEEDBACK_EDIT);
	flngCntrEd = GetDlgItem(IDC_CENTER_EDIT);
	flngDpthEd = GetDlgItem(IDC_DEPTH_EDIT);
	flngFrqEd = GetDlgItem(IDC_SWEEP_EDIT);

	InitValue(flngLvl, flngLvlEd, 0, 100,   10,  30, 0.7);
	InitValue(flngMix, flngMixEd, 0, 100,   10,  30, 0.7);
	InitValue(flngFbk, flngFbkEd, 0, 100,   10, 100, 0);
	InitValue(flngCntr, flngCntrEd, 0, 1000, 100,  40, 0.004);
	InitValue(flngDpth, flngDpthEd, 0, 1000, 100,  40, 0.004);
	InitValue(flngFrq, flngFrqEd, 0, 500,  50,  15, 0.15);

	CheckDlgButton(IDC_SAWTOOTH, BST_CHECKED);
	CheckDlgButton(IDC_SIG_INTERNAL, BST_CHECKED);

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseDialog(0);
	return 0;
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

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
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
		else if (wnd == flngLvl)
			TrackValue(flngLvl, flngLvlEd, 100.0, 100);
		else if (wnd == flngMix)
			TrackValue(flngMix, flngMixEd, 100.0, 100);
		else if (wnd == flngFbk)
			TrackValue(flngFbk, flngFbkEd, 100.0, 100);
		else if (wnd == flngCntr)
			TrackValue(flngCntr, flngCntrEd, 10000.0);
		else if (wnd == flngDpth)
			TrackValue(flngDpth, flngDpthEd, 10000.0);
		else if (wnd == flngFrq)
			TrackValue(flngFrq, flngFrqEd, 100.0);
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
	case IDC_LVL_EDIT:
		UpdateValue(flngLvl, flngLvlEd, 100.0, 100);
		break;
	case IDC_MIX_EDIT:
		UpdateValue(flngMix, flngMixEd, 100.0, 100);
		break;
	case IDC_FEEDBACK_EDIT:
		UpdateValue(flngFbk, flngFbkEd, 100.0, 100);
		break;
	case IDC_CENTER_EDIT:
		UpdateValue(flngCntr, flngCntrEd, 10000.0, 0);
		break;
	case IDC_DEPTH_EDIT:
		UpdateValue(flngDpth, flngDpthEd, 10000.0, 0);
		break;
	case IDC_SWEEP_EDIT:
		UpdateValue(flngFrq, flngFrqEd, 100.0, 0);
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

LRESULT CMainDlg::OnSigFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	sigFile = IsDlgButtonChecked(IDC_SIG_FILE) == BST_CHECKED;
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
		CheckRadioButton(IDC_SIG_FILE, IDC_SIG_INTERNAL, IDC_SIG_FILE);
	}
	else
	{
		MessageBox("Cannot load that wave file. Check the format.", "Ooops.", MB_OK);
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

	AmpValue lvl = GetSetting(flngLvlEd);
	AmpValue mix = GetSetting(flngMixEd);
	AmpValue fbk = GetSetting(flngFbkEd);
	FrqValue centr = GetSetting(flngCntrEd);
	FrqValue depth = GetSetting(flngDpthEd);
	FrqValue sweep = GetSetting(flngFrqEd);

	FrqValue maxdel = centr + depth;

	flng.InitFlanger(lvl, mix, fbk, centr, depth, sweep);

	return dur + maxdel;
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

	sampl = flng.Sample(sampl);
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
#endif

	StopPlaying();

	double dur = InitGen();

	DWORD totalSamples = (DWORD) (dur * synthParams.sampleRate);
	DWORD bufSize = totalSamples + (DWORD) (0.1 * synthParams.sampleRate);
	SampleValue *samples;

#ifdef USE_DIRECTSOUND
	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = 0; 
	dsbd.dwBufferBytes = bufSize * 2;
	dsbd.dwReserved = 0; 
	dsbd.lpwfxFormat = &wf;
	
	hr = dirSndObj->CreateSoundBuffer(&dsbd, &dirSndBuf, NULL);
	if (hr != S_OK)
		return -1;

	void *pAudio1 = NULL;
	void *pAudio2 = NULL;
	DWORD dwAudio1 = 0;
	DWORD dwAudio2 = 0;

	hr = dirSndBuf->Lock(0, bufSize, &pAudio1, &dwAudio1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (hr != S_OK)
		return -1;
	samples = (SampleValue *) pAudio1;
#else
	memset(&wh, 0, sizeof(wh));
	wh.dwBufferLength = bufSize * sizeof(SampleValue);
	wh.lpData = (LPSTR) malloc(wh.dwBufferLength);
	res = waveOutPrepareHeader(woHandle, &wh, sizeof(wh));
	samples = (SampleValue *) wh.lpData;
#endif

	DWORD n;
	for (n = 0; n < totalSamples; n++)
		*samples++ = (SampleValue) (synthParams.sampleScale * Generate());
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
