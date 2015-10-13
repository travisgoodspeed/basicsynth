/////////////////////////////////////////////////////////////////////////////
// This program demonstrates the audio effect of summing harmonic frequencies. 
// As such, it implements a simple Fourier series synthesis method.
//
// The amplitude of each partial is set with the sliders or edit fields. 
// The amplitude ranges from 0-1. Changing a slider automatically updates 
// the edit field. Entering a value in the edit field will update the slider
// on exit from the edit field. Changes to the amplitude values will update
// the waveform plot when one of the top row function buttons is pressed.
//
// Refresh - this button will update the plot with the current values.
//
// Preset Waveforms - The four preset waveform buttons will set the amplitudes
// of the partials to produce the indicated waveform.
//
// Gibbs On/Off - When "ON" this will adjust the amplitude values for the 
// "Gibbs phenonmen" by applying the Lanczos sigma value to the amplitude of all partias > 1.
//
// To hear the waveform, press the Play (>) button or Loop () button. The play 
// button will play two seconds of the sound. The Loop button will repeat the sound 
// until the Stop [] button is pressed. 
//
// The pitch entry field can be used to change the pitch. Pitch is specified with 
// an integer value 0-120 with Middle C at 60 (ala MIDI).
//
// The periods field affects how many periods of the waveform are displayed. This
// does not affect the sound, only the display.
//
// The clipboard button copies the amplitude values to the clipboard.
// The "loudspeaker" button will save the sound to a .WAV file. The "disk" button 
// will save the plot to a metafile (.EMF) that can be displayed in a word 
// processor or graphics program.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "SaveArgs.h"
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
	idTimer = 0;
	picWidth = 4.0;
	picHeight = 3.0;
	picInchOrMM = 0; // inches
	memset(picFileName, 0, sizeof(picFileName));

	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	btnPlay = GetDlgItem(IDC_PLAY);
	btnStop = GetDlgItem(IDC_STOP);
	btnLoop = GetDlgItem(IDC_LOOP);
	RECT btnrc;
	btnPlay.GetClientRect(&btnrc);
	int cxy;
	if ((btnrc.bottom - btnrc.top) < 30)
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

	CButton btnStyle = GetDlgItem(IDC_SAWTOOTH);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SAW), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SQUARE);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SQUARE), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SIN);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SIN), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_RAMP);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_RAMP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_COPYCLIP);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_COPY), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SAVEPIC);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_DISK), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SAVEWAV);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_WVFILE), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_HELP2);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_HELP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	pitchEd = GetDlgItem(IDC_PITCH);
	pitchEd.SetWindowText("48");

	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		sliders[ndx] = GetDlgItem(IDC_PART1+ndx);
		sliders[ndx].SetRange(0, 200, FALSE);
		sliders[ndx].SetTicFreq(20);
		sliders[ndx].SetLineSize(1);
		sliders[ndx].SetPageSize(10);
		levels[ndx] = GetDlgItem(IDC_LVL1+ndx);
	}

	SetDlgItemInt(IDC_PERIODS, 1);

	RECT rcPlot;
	CWindow frm = GetDlgItem(IDC_WVFRAME);
	frm.GetClientRect(&rcPlot);
	frm.MapWindowPoints(m_hWnd, &rcPlot);
	InflateRect(&rcPlot, -2, -2);
	wndPlot.Create(m_hWnd, rcPlot, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0/*WS_EX_CLIENTEDGE*/);
	frm.ShowWindow(SW_HIDE);

	gibbs = FALSE;
	BOOL dummy;
	OnSin(0, IDC_SIN, btnStyle, dummy);

	return TRUE;
}

LRESULT CMainDlg::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int code = LOWORD(wParam);
	if (code == TB_ENDTRACK || code == TB_THUMBTRACK)
	{
		for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
		{
			if (sliders[ndx].m_hWnd == (HWND) lParam)
			{
				int val = 100 - sliders[ndx].GetPos();
				char valstr[20];
				sprintf(valstr, "%06.4f", (double) val / 100.0);
				levels[ndx].SetWindowText(valstr);
				break;
			}
		}
	}
	return 0;
}


LRESULT CMainDlg::OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int ndx = wID - IDC_LVL1;
	if (ndx >= 0 && ndx < WFI_MAXPART)
	{
		char valstr[40];
		levels[ndx].GetWindowText(valstr, 40);
		double dval = atof(valstr);
		if (dval < -1)
			dval = -1;
		else if (dval > 1.0)
			dval = 1.0;
		SetPartValue(ndx, dval);
	}
	return 0;
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	idTimer = 0;
	OnStop(uMsg, IDC_STOP, NULL, bHandled);
	return 0;
}

double CMainDlg::GetFrequency()
{
	char valstr[40];
	valstr[0] = 0;
	pitchEd.GetWindowText(valstr, 40);
	int pit = atoi(valstr);
	return synthParams.GetFrequency(pit);
}

LRESULT CMainDlg::OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UpdatePlot();
	double dur = 2.0;
	double frq = GetFrequency();
	if (Play(frq, dur, FALSE) == 0)
	{
		btnStop.EnableWindow(TRUE);
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
	}
	return 0;
}

LRESULT CMainDlg::OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UpdatePlot();
	double dur = 2.0;
	double frq = GetFrequency();
	if (Play(frq, dur, TRUE) == 0)
	{
		btnStop.EnableWindow(TRUE);
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
	}
	return 0;
}

LRESULT CMainDlg::OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	btnStop.EnableWindow(FALSE);
	btnPlay.EnableWindow(TRUE);
	btnLoop.EnableWindow(TRUE);
	Stop();
	return 0;
}

LRESULT CMainDlg::OnGibbs(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	gibbs = IsDlgButtonChecked(IDC_GIBBS) == BST_CHECKED;
	SetDlgItemText(IDC_GIBBS, gibbs ? "Gibbs ON" : "Gibbs OFF");
	UpdatePlot();
	return 0;
}

LRESULT CMainDlg::OnRefresh(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UpdatePlot();
	return 0;
}

LRESULT CMainDlg::OnSin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	double amps[WFI_MAXPART];
	amps[0] = 1.0;
	SetPartValue(0, 1.0);
	for (int ndx = 1; ndx < WFI_MAXPART; ndx++)
	{
		amps[ndx] = 0.0;
		SetPartValue(ndx, 0.0);
	}

	try {
		wtSet.SetWaveTable(WT_USR(0), WFI_MAXPART, NULL, amps, NULL, gibbs);
	} catch(...) {
		OutputDebugString("Sin\r\n");
	}
	wndPlot.Invalidate();
	return 0;
}

LRESULT CMainDlg::OnSawtooth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	double amps[WFI_MAXPART];
	double dval;
	double part = 1.0;
	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		dval = 1.0 / part;
		if (ndx & 1)
			dval = -dval;
		amps[ndx] = dval;
		SetPartValue(ndx, dval);
		part += 1.0;
	}
	try {
		wtSet.SetWaveTable(WT_USR(0), WFI_MAXPART, NULL, amps, NULL, gibbs);
	} catch(...) {
		OutputDebugString("Sawtooth\r\n");
	}
	wndPlot.Invalidate();
	return 0;
}

LRESULT CMainDlg::OnRamp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	double amps[WFI_MAXPART];
	double dval;
	double part = 1.0;
	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		dval = 1.0 / part;
		amps[ndx] = dval;
		SetPartValue(ndx, dval);
		part += 1.0;
	}

	try {
		wtSet.SetWaveTable(WT_USR(0), WFI_MAXPART, NULL, amps, NULL, gibbs);
	} catch(...) {
		OutputDebugString("Ramp\r\n");
	}
	wndPlot.Invalidate();
	return 0;
}

LRESULT CMainDlg::OnSquare(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	double amps[WFI_MAXPART];
	double dval;
	double part = 1.0;
	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		if (!(ndx & 1))
			dval = 1.0 / part;
		else
			dval = 0.0;
		amps[ndx] = dval;
		SetPartValue(ndx, dval);
		part += 1.0;
	}

	try {
		wtSet.SetWaveTable(WT_USR(0), WFI_MAXPART, NULL, amps, NULL, gibbs);
	} catch(...) {
		OutputDebugString("Square\r\n");
	}
	wndPlot.Invalidate();
	return 0;
}


LRESULT CMainDlg::OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OpenClipboard();
	EmptyClipboard();

	char valstr[10*WFI_MAXPART];
	char *p = valstr;
	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		if (ndx > 0)
			*p++ = ',';
		levels[ndx].GetWindowText(p, 40);
		while (*p)
			p++;
	}
	*p++ = '\r';
	*p++ = '\n';
	*p = 0;

	HANDLE mem = GlobalAlloc(0, strlen(valstr)+1);
	char *ptxt = (char*)GlobalLock(mem);
	strcpy(ptxt, valstr);
	GlobalUnlock(mem);

	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();
	return 0;
}

LRESULT CMainDlg::OnSavePic(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSaveArgs dlg;
	dlg.width = picWidth;
	dlg.height = picHeight;
	dlg.inchOrMM = picInchOrMM;
	strcpy(dlg.fileName, picFileName);

	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		picWidth = dlg.width;
		picHeight = dlg.height;
		picInchOrMM = dlg.inchOrMM;
		strcpy(picFileName, dlg.fileName);

		int dx, dy;
		int iWidthMM, iHeightMM;
		int iWidthPels, iHeightPels;

		HDC hdcRef = NULL;
		int isIC = FALSE;

		/*
		 * Not really necessary, but useful...
		 * This will calculate resolution for the default printer.
		 * That generates a larger image generally and maybe will
		 * look better when printed from a document file.
		 */
		PRINTDLG pd;
		memset(&pd, 0, sizeof(pd));
		pd.lStructSize = sizeof(PRINTDLG); 
		pd.Flags = PD_RETURNDEFAULT; 
		pd.hwndOwner = m_hWnd; 
		if (PrintDlg(&pd))
		{
			DEVNAMES *dn = (DEVNAMES*)GlobalLock(pd.hDevNames);
			char *driver = ((char *)dn) + dn->wDriverOffset;
			char *device = ((char *)dn) + dn->wDeviceOffset;
			char *port   = ((char *)dn) + dn->wOutputOffset;

			hdcRef = CreateIC(driver, device, port, NULL);
			if (hdcRef)
				isIC = TRUE;

			GlobalUnlock(pd.hDevNames);
			GlobalFree(pd.hDevNames);
			GlobalFree(pd.hDevMode);
		}
		if (!hdcRef)
			hdcRef = GetDC(); 

		iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE); 
		iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE); 
		iWidthPels = GetDeviceCaps(hdcRef, HORZRES); 
		iHeightPels = GetDeviceCaps(hdcRef, VERTRES);
		dx = GetDeviceCaps(hdcRef, LOGPIXELSX);
		dy = GetDeviceCaps(hdcRef, LOGPIXELSY);

		// Calculate bounds at output resolution
		RECT rcpix;
		rcpix.left = 0;
		rcpix.top = 0;
		if (picInchOrMM)
		{
			rcpix.right = (int) ((double)dx * (picWidth / 25.4));
			rcpix.bottom = (int) ((double) dy * (picHeight / 25.4));
		}
		else
		{
			rcpix.right = (int) ((double)dx * picWidth);
			rcpix.bottom = (int) ((double) dy * picHeight);
		}

		// produce a bounding box in HIMETRIC for the metafile
		RECT rc;
		rc.left = (rcpix.left * iWidthMM * 100)/iWidthPels; 
		rc.top = (rcpix.top * iHeightMM * 100)/iHeightPels; 
		rc.right = (rcpix.right * iWidthMM * 100)/iWidthPels; 
		rc.bottom = (rcpix.bottom * iHeightMM * 100)/iHeightPels; 

		HDC drawdc = CreateEnhMetaFile(hdcRef, picFileName, &rc, "Waveform Explorer, (c) Daniel R. Mitchell\0Waveform Explorer\0");
		//HFONT hf = CreateFont((8*dy)/72, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
		//	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE|DEFAULT_PITCH, "Tahoma");
		//SelectObject(drawdc, hf); 
		wndPlot.Plot(drawdc, rcpix);

		HENHMETAFILE mf = CloseEnhMetaFile(drawdc);
		DeleteEnhMetaFile(mf);
	//	DeleteObject(hf);

		if (isIC)
			DeleteDC(hdcRef);
		else
			ReleaseDC(hdcRef);
	}

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
		double frq = GetFrequency();
		GenWaveI wv;
		wv.InitWT(frq, WT_USR(0));
		EnvGen eg;
		eg.InitEG(1.0, 2.0, 0.05, 0.05);
		WaveFile wvf;
		wvf.OpenWaveFile(fileName, 2);
		long totalSamples = (long) (2.0 * synthParams.sampleRate);
		for (long n = 0; n < totalSamples; n++)
			wvf.Output1(eg.Gen() * wv.Gen());
		wvf.CloseWaveFile();

	}
	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHelpDlg dlg;
	dlg.DoModal();
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

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseDialog(0);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

void CMainDlg::UpdatePlot()
{
	double amps[WFI_MAXPART];
	char valstr[40];

	for (int ndx = 0; ndx < WFI_MAXPART; ndx++)
	{
		levels[ndx].GetWindowText(valstr, 40);
		amps[ndx] = atof(valstr);
	}
	wtSet.SetWaveTable(WT_USR(0), WFI_MAXPART, NULL, amps, NULL, gibbs);
	wndPlot.SetPeriods(GetDlgItemInt(IDC_PERIODS));
	wndPlot.Invalidate();
}

void CMainDlg::SetPartValue(int ndx, double dval)
{
	char valstr[40];
	int pos = 200 - (int) ((dval + 1) * 100);
	sliders[ndx].SetPos(pos);
	sprintf(valstr, "%06.4f", dval);
	levels[ndx].SetWindowText(valstr);
}


int CMainDlg::Play(double frq, double dur, int loop)
{
	wv.InitWT(frq, WT_USR(0));
	eg.InitEG(0.707, dur, 0.05, 0.05);
	dur += 0.1;

	DWORD totalSamples = (DWORD) (dur * synthParams.sampleRate);
	SampleValue *samples;

	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
    wf.nSamplesPerSec = synthParams.sampleRate;
    wf.nAvgBytesPerSec = synthParams.sampleRate * 2;
	wf.nBlockAlign = 2; 
    wf.wBitsPerSample = 16;
	wf.cbSize = 0;

#ifdef USE_DIRECTSOUND
	HRESULT hr = S_OK;
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
			}
		}
		if (hr != S_OK)
		{
			MessageBox("Cannot open DirectSound output", "Error", MB_OK|MB_ICONSTOP);
			return -1;
		}
	}

	if (dirSndBuf != NULL)
	{
		dirSndBuf->Stop();
		dirSndBuf->Release();
		dirSndBuf = NULL;
	}

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = 0; 
	dsbd.dwBufferBytes = totalSamples * 2;
	dsbd.dwReserved = 0; 
	dsbd.lpwfxFormat = &wf;
	
	hr = dirSndObj->CreateSoundBuffer(&dsbd, &dirSndBuf, NULL);
	if (hr != S_OK)
		return -1;

	void *pAudio1 = NULL;
	void *pAudio2 = NULL;
	DWORD dwAudio1 = 0;
	DWORD dwAudio2 = 0;

	hr = dirSndBuf->Lock(0, totalSamples, &pAudio1, &dwAudio1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
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
	wh.dwBufferLength = totalSamples * sizeof(SampleValue);
	wh.lpData = (LPSTR) malloc(wh.dwBufferLength);
	waveOutPrepareHeader(woHandle, &wh, sizeof(wh));
	samples = (SampleValue *) wh.lpData;
#endif

	DWORD n = 0;
	while (!eg.IsFinished() && n < totalSamples)
	{
		*samples++ = (SampleValue) (synthParams.sampleScale * wv.Gen() * eg.Gen());
		n++;
	}
	while (n++ < totalSamples)
		*samples++ = 0;


#ifdef USE_DIRECTSOUND
	dirSndBuf->Unlock(pAudio1, dwAudio1, pAudio2, dwAudio2);
	dirSndBuf->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
	if (!loop)
		idTimer = SetTimer(1, (UINT) ((dur * 1000.0) + 100));
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

int CMainDlg::Stop()
{
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
	return 0;
}

LRESULT CMainDlg::OnWaveDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (idTimer)
	{
		KillTimer(idTimer);
		idTimer = 0;
	}

#ifndef USE_DIRECTSOUND
    waveOutUnprepareHeader((HWAVEOUT) wParam, (LPWAVEHDR) lParam, sizeof(WAVEHDR)); 
	free(wh.lpData);
	memset(&wh, 0, sizeof(wh));
#endif
	btnStop.EnableWindow(FALSE);
	btnPlay.EnableWindow(TRUE);
	btnLoop.EnableWindow(TRUE);
	return 0;
}
