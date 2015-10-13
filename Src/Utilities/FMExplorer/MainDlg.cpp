///////////////////////////////////////////////////////////////////////////
// FM Synthesis Explorer
//
// This program implements a 3 oscillator FM synthesis instrument.
//
// The four buttons on the left bottom select the "algorithm." In the single
// modulator stack only Modulator I affects the carrier. In the double
// stack configuration, Modulator II is applied to Modulator I and then
// Modulator I is applied to the carrier. In the "Y" configuration, the
// modulators are summed and the sum applied to the carrier. Algorithm 4
// is a combination of the stack and "Y" algorithms.
//
// Carrier frequency is specified as a pitch number (0-120) with middle
// C at index 60 (ala MIDI). Modulator frequency is specified as a multiple
// of the carrier in the range 0-10 and may be harmonic (integer values)
// or inharmonic (real values).
//
// The envelope generators are ADSR types with attack and decay
// rates in the range 0-1 second.
//
// The amplitude of the carrier EG sets the volume level. The amplitude
// of a modulator sets the modulation depth and is specified as an index
// of modulation, per Chowning's method. The index of modulation has a
// range of 0-50.
//
// Values can be entered using the sliders or by typing directly into 
// the edit fields. Moving out of an edit field automatically updates
// the slider position.
//
// Sound can be played from this program through the Windows directX
// system. The "Play" button (>) plays the sound once. The "Loop" button
// plays the sound repeatedly.
//
// The sound can be saved as a single channel (monophonic) .WAV file.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////////////////

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

#define ALG_STACK 1
#define ALG_STACK2 2
#define ALG_WYE 3
#define ALG_DELTA 4

static FMInstrParam resetParams = 
{    1,  0.000,  0.010,  1.000,  0.000,  1.000,  0.100,  0.000,
     1,  1.000,  0.000,  1.000,  0.000,  1.000,  0.000,  1.000,
     2,  1.000,  0.000,  1.000,  0.000,  1.000,  0.000,  1.000 
};

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
	algorithm = ALG_STACK;
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

	gen1Frq = GetDlgItem(IDC_CAR_FREQ);
	gen1Atk = GetDlgItem(IDC_CAR_ATTACK);
	gen1Dec = GetDlgItem(IDC_CAR_DECAY);
	gen1Vol = GetDlgItem(IDC_CAR_VOLUME);
	gen1Sus = GetDlgItem(IDC_CAR_SUSTAIN);
	gen1Rel = GetDlgItem(IDC_CAR_RELEASE);
	gen1FrqEd = GetDlgItem(IDC_CFRQ_EDIT);
	gen1AtkEd = GetDlgItem(IDC_CATK_EDIT);
	gen1DecEd = GetDlgItem(IDC_CDEC_EDIT);
	gen1VolEd = GetDlgItem(IDC_CVOL_EDIT);
	gen1SusEd = GetDlgItem(IDC_CSUS_EDIT);
	gen1RelEd = GetDlgItem(IDC_CREL_EDIT);

	InitValue(gen1Frq, gen1FrqEd, 0, 120, 12);
	PutSetting(gen1Frq, gen1FrqEd, 48.0, 1, 120);
	InitValue(gen1Vol, gen1VolEd, 0, 100, 10);
	InitValue(gen1Sus, gen1SusEd, 0, 100, 10);
	InitValue(gen1Atk, gen1AtkEd, 0, 4000, 400);
	InitValue(gen1Dec, gen1DecEd, 0, 4000, 400);
	InitValue(gen1Rel, gen1RelEd, 0, 4000, 400);

	gen2Mul = GetDlgItem(IDC_MOD1_MULT);
	gen2Ndx = GetDlgItem(IDC_MOD1_START);
	gen2Atk = GetDlgItem(IDC_MOD1_ATTACK);
	gen2Pck = GetDlgItem(IDC_MOD1_PEAK);
	gen2Dec = GetDlgItem(IDC_MOD1_DECAY);
	gen2Sus = GetDlgItem(IDC_MOD1_SUSTAIN);
	gen2Rel = GetDlgItem(IDC_MOD1_RELEASE);
	gen2End = GetDlgItem(IDC_MOD1_END);
	gen2MulEd = GetDlgItem(IDC_M1M_EDIT);
	gen2NdxEd = GetDlgItem(IDC_M1I_EDIT);
	gen2AtkEd = GetDlgItem(IDC_M1A_EDIT);
	gen2PckEd = GetDlgItem(IDC_M1P_EDIT);
	gen2DecEd = GetDlgItem(IDC_M1D_EDIT);
	gen2SusEd = GetDlgItem(IDC_M1S_EDIT);
	gen2RelEd = GetDlgItem(IDC_M1R_EDIT);
	gen2EndEd = GetDlgItem(IDC_M1E_EDIT);

	InitValue(gen2Mul, gen2MulEd, 0,  100,  10);
	InitValue(gen2Ndx, gen2NdxEd, 0,  500,  50);
	InitValue(gen2Pck, gen2PckEd, 0,  500,  50);
	InitValue(gen2Sus, gen2SusEd, 0,  500,  50);
	InitValue(gen2End, gen2EndEd, 0,  500,  50);
	InitValue(gen2Atk, gen2AtkEd, 0, 4000, 400);
	InitValue(gen2Dec, gen2DecEd, 0, 4000, 400);
	InitValue(gen2Rel, gen2RelEd, 0, 4000, 400);

	gen3Mul = GetDlgItem(IDC_MOD2_MULT);
	gen3Ndx = GetDlgItem(IDC_MOD2_START);
	gen3Atk = GetDlgItem(IDC_MOD2_ATTACK);
	gen3Pck = GetDlgItem(IDC_MOD2_PEAK);
	gen3Dec = GetDlgItem(IDC_MOD2_DECAY);
	gen3Sus = GetDlgItem(IDC_MOD2_SUSTAIN);
	gen3Rel = GetDlgItem(IDC_MOD2_RELEASE);
	gen3End = GetDlgItem(IDC_MOD2_END);
	gen3MulEd = GetDlgItem(IDC_M2M_EDIT);
	gen3NdxEd = GetDlgItem(IDC_M2I_EDIT);
	gen3AtkEd = GetDlgItem(IDC_M2A_EDIT);
	gen3PckEd = GetDlgItem(IDC_M2P_EDIT);
	gen3DecEd = GetDlgItem(IDC_M2D_EDIT);
	gen3SusEd = GetDlgItem(IDC_M2S_EDIT);
	gen3RelEd = GetDlgItem(IDC_M2R_EDIT);
	gen3EndEd = GetDlgItem(IDC_M2E_EDIT);

	InitValue(gen3Mul, gen3MulEd, 0,  100,  10);
	InitValue(gen3Ndx, gen3NdxEd, 0,  500,  50);
	InitValue(gen3Pck, gen3PckEd, 0,  500,  50);
	InitValue(gen3Sus, gen3SusEd, 0,  500,  50);
	InitValue(gen3End, gen3EndEd, 0,  500,  50);
	InitValue(gen3Atk, gen3AtkEd, 0, 4000, 400);
	InitValue(gen3Dec, gen3DecEd, 0, 4000, 400);
	InitValue(gen3Rel, gen3RelEd, 0, 4000, 400);

	durValEd = GetDlgItem(IDC_DUR_EDIT);
	durValEd.SetWindowText("4.0");
	volValEd = GetDlgItem(IDC_VOL);
	volValEd.SetWindowText("0.707");

	btnPlay = GetDlgItem(IDC_PLAY);
	btnLoop = GetDlgItem(IDC_LOOP);
	btnStop = GetDlgItem(IDC_STOP);
	RECT btnrc;
	btnPlay.GetClientRect(&btnrc);
	int cxy;
	if ((btnrc.bottom - btnrc.top) < 30)
		cxy = 16;
	else
		cxy = 32;
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnPlay.SetIcon(hIcon);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_LOOP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnLoop.SetIcon(hIcon);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStop.SetIcon(hIcon);
	btnStop.EnableWindow(FALSE);

	CButton btnStyle;
	HBITMAP bm;

	btnStyle = GetDlgItem(IDC_STACK);
	bm = (HBITMAP) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_STACK), IMAGE_BITMAP, 48, 48, LR_DEFAULTCOLOR);
	btnStyle.SetBitmap(bm);
	btnStyle.SetCheck(BST_CHECKED);
	btnStyle = GetDlgItem(IDC_STACK2);
	bm = (HBITMAP) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_STACK2), IMAGE_BITMAP, 48, 48, LR_DEFAULTCOLOR);
	btnStyle.SetBitmap(bm);
	btnStyle = GetDlgItem(IDC_WYE);
	bm = (HBITMAP) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_WYE), IMAGE_BITMAP, 48, 48, LR_DEFAULTCOLOR);
	btnStyle.SetBitmap(bm);
	btnStyle = GetDlgItem(IDC_DELTA);
	bm = (HBITMAP) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DELTA), IMAGE_BITMAP, 48, 48, LR_DEFAULTCOLOR);
	btnStyle.SetBitmap(bm);

	btnStyle = GetDlgItem(IDC_COPY_CLIP);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_COPY), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_SAVE);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_WVFILE), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);
	btnStyle = GetDlgItem(IDC_HELP2);
	hIcon = (HICON) ::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_HELP2), IMAGE_ICON, cxy, cxy, LR_DEFAULTCOLOR);
	btnStyle.SetIcon(hIcon);

	SetPreset(&resetParams);

	return TRUE;
}

void CMainDlg::InitValue(CTrackBarCtrl& slid, CEdit& ed, int minval, int maxval, int ticfrq)
{
	slid.SetRange(minval, maxval);
	slid.SetTicFreq(ticfrq);
	slid.SetLineSize(1);
	slid.SetPageSize(ticfrq);
}

void CMainDlg::TrackValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv)
{
	int val = slid.GetPos();
	if (inv)
		val = inv - val;
	char valstr[40];
	sprintf(valstr, "%6.3f", (double) val / div);
	ed.SetWindowText(valstr);
}

LRESULT CMainDlg::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int id = HIWORD(wParam);
	int code = LOWORD(wParam);
	if (code == TB_ENDTRACK || code == TB_THUMBTRACK)
	{
		HWND w = (HWND) lParam;
		if (w == gen1Frq)
			TrackValue(gen1Frq, gen1FrqEd, 1.0, 120);
		else if (w == gen1Vol)
			TrackValue(gen1Vol, gen1VolEd, 100.0, 100);
		else if (w == gen1Atk)
			TrackValue(gen1Atk, gen1AtkEd, 1000.0);
		else if (w == gen1Dec)
			TrackValue(gen1Dec, gen1DecEd, 1000.0);
		else if (w == gen1Sus)
			TrackValue(gen1Sus, gen1SusEd, 100.0, 100);
		else if (w == gen1Rel)
			TrackValue(gen1Rel, gen1RelEd, 1000.0);

		else if (w == gen2Mul)
			TrackValue(gen2Mul, gen2MulEd, 10.0);
		else if (w == gen2Ndx)
			TrackValue(gen2Ndx, gen2NdxEd, 10.0, 500);
		else if (w == gen2Atk)
			TrackValue(gen2Atk, gen2AtkEd, 1000.0);
		else if (w == gen2Pck)
			TrackValue(gen2Pck, gen2PckEd, 10.0, 500);
		else if (w == gen2Dec)
			TrackValue(gen2Dec, gen2DecEd, 1000.0);
		else if (w == gen2Sus)
			TrackValue(gen2Sus, gen2SusEd, 10.0, 500);
		else if (w == gen2Rel)
			TrackValue(gen2Rel, gen2RelEd, 1000.0);
		else if (w == gen2End)
			TrackValue(gen2End, gen2EndEd, 10.0, 500);

		else if (w == gen3Mul)
			TrackValue(gen3Mul, gen3MulEd, 10.0);
		else if (w == gen3Ndx)
			TrackValue(gen3Ndx, gen3NdxEd, 10.0, 500);
		else if (w == gen3Atk)
			TrackValue(gen3Atk, gen3AtkEd, 1000.0);
		else if (w == gen3Pck)
			TrackValue(gen3Pck, gen3PckEd, 10.0, 500);
		else if (w == gen3Dec)
			TrackValue(gen3Dec, gen3DecEd, 1000.0);
		else if (w == gen3Sus)
			TrackValue(gen3Sus, gen3SusEd, 10.0, 500);
		else if (w == gen3Rel)
			TrackValue(gen3Rel, gen3RelEd, 1000.0);
		else if (w == gen3End)
			TrackValue(gen3End, gen3EndEd, 10.0, 500);
	}
	return 0;
}

void CMainDlg::UpdateValue(CTrackBarCtrl& slid, CEdit& ed, double div, int inv)
{
	char valstr[40];
	ed.GetWindowText(valstr, 40);
	PutSetting(slid, ed, atof(valstr), div, inv);
}

LRESULT CMainDlg::OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch (wID)
	{
	case IDC_CFRQ_EDIT:
		UpdateValue(gen1Frq, gen1FrqEd, 1.0, 120);
		break;
	case IDC_CVOL_EDIT:
		UpdateValue(gen1Vol, gen1VolEd, 100.0, 100);
		break;
	case IDC_CATK_EDIT:
		UpdateValue(gen1Atk, gen1AtkEd, 1000.0);
		break;
	case IDC_CDEC_EDIT:
		UpdateValue(gen1Dec, gen1DecEd, 1000.0);
		break;
	case IDC_CSUS_EDIT:
		UpdateValue(gen1Sus, gen1SusEd, 100.0, 100);
		break;
	case IDC_CREL_EDIT:
		UpdateValue(gen1Rel, gen1RelEd, 1000.0);
		break;

	case IDC_M1M_EDIT:
		UpdateValue(gen2Mul, gen2MulEd, 10.0);
		break;
	case IDC_M1I_EDIT:
		UpdateValue(gen2Ndx, gen2NdxEd, 10.0, 500);
		break;
	case IDC_M1A_EDIT:
		UpdateValue(gen2Atk, gen2AtkEd, 1000.0);
		break;
	case IDC_M1P_EDIT:
		UpdateValue(gen2Pck, gen2PckEd, 10.0, 500);
		break;
	case IDC_M1D_EDIT:
		UpdateValue(gen2Dec, gen2DecEd, 1000.0);
		break;
	case IDC_M1S_EDIT:
		UpdateValue(gen2Sus, gen2SusEd, 10.0, 500);
		break;
	case IDC_M1R_EDIT:
		UpdateValue(gen2Rel, gen2RelEd, 1000.0);
		break;
	case IDC_M1E_EDIT:
		UpdateValue(gen2End, gen2EndEd, 10.0, 500);
		break;

	case IDC_M2M_EDIT:
		UpdateValue(gen3Mul, gen3MulEd, 10.0);
		break;
	case IDC_M2I_EDIT:
		UpdateValue(gen3Ndx, gen3NdxEd, 10.0, 500);
		break;
	case IDC_M2A_EDIT:
		UpdateValue(gen3Atk, gen3AtkEd, 1000.0);
		break;
	case IDC_M2P_EDIT:
		UpdateValue(gen3Pck, gen3PckEd, 10.0, 500);
		break;
	case IDC_M2D_EDIT:
		UpdateValue(gen3Dec, gen3DecEd, 1000.0);
		break;
	case IDC_M2S_EDIT:
		UpdateValue(gen3Sus, gen3SusEd, 10.0, 500);
		break;
	case IDC_M2R_EDIT:
		UpdateValue(gen3Rel, gen3RelEd, 1000.0);
		break;
	case IDC_M2E_EDIT:
		UpdateValue(gen3End, gen3EndEd, 10.0, 500);
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnPlay(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (PlayFM(FALSE) == 0)
	{
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
		btnStop.EnableWindow(TRUE);
	}
	return 0;
}

LRESULT CMainDlg::OnLoop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (PlayFM(TRUE) == 0)
	{
		btnPlay.EnableWindow(FALSE);
		btnLoop.EnableWindow(FALSE);
		btnStop.EnableWindow(TRUE);
	}
	return 0;
}

LRESULT CMainDlg::OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (idTimer)
		KillTimer(idTimer);
	StopPlaying();
	btnPlay.EnableWindow(TRUE);
	btnLoop.EnableWindow(TRUE);
	btnStop.EnableWindow(FALSE);
	return 0;
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnStop(0, IDC_STOP, NULL, bHandled);
	return 0;
}

LRESULT CMainDlg::OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		WaveFile wvf;
		wvf.OpenWaveFile(fileName, 2);
		InitGen();
		long totalSamples = (long) ((durTotal * synthParams.sampleRate) + 0.5);
		long atkSamples = (long) (durAtkSus * synthParams.sampleRate);
		long n;
		for (n = 0; n < atkSamples; n++)
			wvf.Output1(Generate());
		NoteOff();
		while (n++ < totalSamples)
			wvf.Output1(Generate());
		for (n = 0; n < 440; n++)
			wvf.Output1(0);
		wvf.CloseWaveFile();
	}
	return 0;
}

LRESULT CMainDlg::OnStack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	algorithm = ALG_STACK;
	return 0;
}

LRESULT CMainDlg::OnStack2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	algorithm = ALG_STACK2;
	return 0;
}

LRESULT CMainDlg::OnWye(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	algorithm = ALG_WYE;
	return 0;
}

LRESULT CMainDlg::OnDelta(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	algorithm = ALG_DELTA;
	return 0;
}

void CMainDlg::SetPreset(FMInstrParam *p)
{
	int btn = ALG_STACK;
	switch (p->algorithm)
	{
	case 1:
		algorithm = ALG_STACK;
		btn = IDC_STACK;
		break;
	case 2:
		algorithm = ALG_STACK2;
		btn = IDC_STACK2;
		break;
	case 3:
		algorithm = ALG_WYE;
		btn = IDC_WYE;
		break;
	case 4:
		algorithm = ALG_DELTA;
		btn = IDC_DELTA;
		break;
	}
	CheckRadioButton(IDC_STACK, IDC_DELTA, btn);

	PutSetting(gen1Vol, gen1VolEd, p->gen1Pk,  100.0, 100);
	PutSetting(gen1Atk, gen1AtkEd, p->gen1Atk, 1000.0);
	PutSetting(gen1Dec, gen1DecEd, p->gen1Dec, 1000.0);
	PutSetting(gen1Sus, gen1SusEd, p->gen1Sus, 100.0, 100);
	PutSetting(gen1Rel, gen1RelEd, p->gen1Rel, 1000.0);

	PutSetting(gen2Mul, gen2MulEd, p->gen2Mult, 10.0);
	PutSetting(gen2Ndx, gen2NdxEd, p->gen2Start, 10.0, 500);
	PutSetting(gen2Atk, gen2AtkEd, p->gen2Atk, 1000.0);
	PutSetting(gen2Pck, gen2PckEd, p->gen2Pk, 10.0, 500);
	PutSetting(gen2Dec, gen2DecEd, p->gen2Dec, 1000.0);
	PutSetting(gen2Sus, gen2SusEd, p->gen2Sus, 10.0, 500);
	PutSetting(gen2Rel, gen2RelEd, p->gen2Rel, 1000.0);
	PutSetting(gen2End, gen2EndEd, p->gen2End, 10.0, 500);

	PutSetting(gen3Mul, gen3MulEd, p->gen3Mult, 10.0);
	PutSetting(gen3Ndx, gen3NdxEd, p->gen3Start, 10.0, 500);
	PutSetting(gen3Atk, gen3AtkEd, p->gen3Atk, 1000.0);
	PutSetting(gen3Pck, gen3PckEd, p->gen3Pk, 10.0, 500);
	PutSetting(gen3Dec, gen3DecEd, p->gen3Dec, 1000.0);
	PutSetting(gen3Sus, gen3SusEd, p->gen3Sus, 10.0, 500);
	PutSetting(gen3Rel, gen3RelEd, p->gen3Rel, 1000.0);
	PutSetting(gen3End, gen3EndEd, p->gen3End, 10.0, 500);

}

LRESULT CMainDlg::OnPresetReset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetPreset(&resetParams);
	return 0;
}

LRESULT CMainDlg::OnPresetPiano(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam piano = 
{    2,  0.000,  0.015,  1.000,  3.000,  0.000,  0.100,  0.000,
 2.996,  3.000,  0.010,  1.000,  2.000,  0.600,  0.150,  0.200,
 5.001,  2.000,  0.010,  1.500,  1.000,  0.200,  0.100,  0.100 };

	SetPreset(&piano);
	return 0;
}

LRESULT CMainDlg::OnPresetOrgan(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam organ = 
{    3,  0.000,  0.050,  1.000,  0.100,  0.950,  0.100,  0.000,
 2.001,  1.000,  0.050,  3.000,  0.000,  3.000,  0.050,  1.000,
 5.999,  1.000,  0.100,  1.200,  0.000,  1.200,  0.050,  0.500 };

	SetPreset(&organ);
	return 0;
};

LRESULT CMainDlg::OnPresetFlute(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam flute = 
{    3,  0.000,  0.200,  1.000,  0.200,  0.900,  0.200,  0.000,
 3.000,  0.000,  0.030,  0.700,  0.100,  0.150,  0.200,  0.000,
 1.000,  0.400,  0.010,  0.500,  0.200,  0.200,  0.150,  0.000 };

	SetPreset(&flute);
	return 0;
};

LRESULT CMainDlg::OnPresetStrings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam string = 
{    2,  0.000,  0.150,  1.000,  0.100,  0.950,  0.150,  0.000,
 4.000,  2.000,  0.050,  1.500,  0.000,  1.500,  0.150,  1.000,
 3.000,  0.000,  0.010,  1.000,  0.000,  1.000,  0.150,  0.500 };


	SetPreset(&string);
	return 0;
}

LRESULT CMainDlg::OnPresetBrass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam brass = 
{    3,  0.000,  0.100,  1.000,  0.200,  0.800,  0.200,  0.000,
 1.000,  0.000,  0.080,  1.200,  0.200,  0.800,  0.200,  0.000,
 2.000,  0.500,  0.050,  1.500,  0.100,  0.500,  0.100,  0.300 };

	SetPreset(&brass);
	return 0;
}

LRESULT CMainDlg::OnPresetClarinet(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam clar = 
{    3,  0.000,  0.100,  1.000,  0.200,  0.900,  0.200,  0.000,
 2.000,  0.500,  0.030,  2.000,  0.100,  0.800,  0.200,  0.200,
 4.000,  0.200,  0.010,  0.600,  0.100,  0.400,  0.100,  0.000 };

	SetPreset(&clar);
	return 0;
}

LRESULT CMainDlg::OnPresetSynth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam synth = 
{    2,  0.000,  0.040,  1.000,  0.100,  0.700,  0.200,  0.000,
 1.000,  0.000,  0.040,  3.000,  0.150,  0.800,  0.200,  0.000,
 4.000,  0.000,  0.050,  2.000,  0.100,  0.200,  0.150,  0.000 };

	SetPreset(&synth);
	return 0;
}

LRESULT CMainDlg::OnPresetPlucked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam plucked =
{    2,  0.000,  0.001,  1.000,  1.000,  0.000,  0.100,  0.000,
 1.500,  3.000,  0.001,  2.000,  1.000,  0.500,  0.100,  0.000,
 4.000,  1.000,  0.001,  2.000,  0.300,  0.500,  0.050,  0.000 };

	SetPreset(&plucked);
	return 0;
}

LRESULT CMainDlg::OnPresetBell1(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam bell1 =
{    1,  0.000,  0.010,  1.000,  3.000,  0.000,  0.100,  0.000,
 1.414, 10.000,  0.010,  4.000,  3.000,  1.000,  0.100,  0.000,
 1.000,  1.000,  0.000,  1.000,  0.000,  1.000,  0.000,  1.000 };

	SetPreset(&bell1);
	return 0;
}

LRESULT CMainDlg::OnPresetBell2(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam bell2 =
{    1,  0.000,  0.010,  1.000,  3.000,  0.000,  0.100,  0.000,
 1.667,  0.000,  0.010,  3.000,  2.500,  0.500,  0.100,  0.000,
 4.000,  0.000,  0.010,  2.000,  3.000,  0.000,  0.100,  0.000 };

	SetPreset(&bell2);
	return 0;
}

LRESULT CMainDlg::OnPresetBass(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam bass =
{    3,  0.000,  0.005,  1.000,  1.400,  0.000,  0.080,  0.000,
 1.000,  1.000,  0.200,  1.200,  1.000,  0.500,  0.100,  0.200,
 2.000,  1.500,  0.005,  1.000,  0.100,  0.600,  0.100,  0.000 };

	SetPreset(&bass);
	return 0;
}

LRESULT CMainDlg::OnPresetBell3(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam bell3 =
{    1,  0.000,  0.010,  1.000,  3.000,  0.000,  0.100,  0.000,
 1.414, 10.000,  1.000,  5.000,  2.000,  1.000,  0.100,  0.000,
 4.000,  0.000,  0.010,  2.000,  3.000,  0.000,  0.100,  0.000 };

	SetPreset(&bell3);
	return 0;
}

LRESULT CMainDlg::OnPresetWood(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
static FMInstrParam wood1 =
{    2,  0.000,  0.001,  1.000,  0.400,  0.000,  0.100,  0.000,
 2.000,  4.500,  0.015,  0.500,  0.400,  0.100,  0.100,  0.000,
 4.000,  2.000,  0.100,  0.500,  0.200,  0.100,  0.100,  0.000 };

	SetPreset(&wood1);
	return 0;
}


LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseDialog(0);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHelpDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnCopyClip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OpenClipboard();
	EmptyClipboard();

	FrqValue atk = GetSetting(gen1AtkEd);
	AmpValue pk  = GetSetting(gen1VolEd);
	FrqValue dec = GetSetting(gen1DecEd);
	AmpValue sus = GetSetting(gen1SusEd);
	FrqValue rel = GetSetting(gen1RelEd);

	FrqValue mul1 = GetSetting(gen2MulEd);
	AmpValue amp1 = GetSetting(gen2NdxEd);
	FrqValue atk1 = GetSetting(gen2AtkEd);
	AmpValue pk1  = GetSetting(gen2PckEd);
	FrqValue dec1 = GetSetting(gen2DecEd);
	AmpValue sus1 = GetSetting(gen2SusEd);
	FrqValue rel1 = GetSetting(gen2RelEd);
	AmpValue end1 = GetSetting(gen2EndEd);

	FrqValue mul2 = GetSetting(gen3MulEd);
	AmpValue amp2 = GetSetting(gen3NdxEd);
	FrqValue atk2 = GetSetting(gen3AtkEd);
	AmpValue pk2  = GetSetting(gen3PckEd);
	FrqValue dec2 = GetSetting(gen3DecEd);
	AmpValue sus2 = GetSetting(gen3SusEd);
	FrqValue rel2 = GetSetting(gen3RelEd);
	AmpValue end2 = GetSetting(gen3EndEd);

	char txt[1024];
	sprintf(txt, "{ %4d, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f,\r\n%6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f,\r\n%6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f },\r\n",
		algorithm, 0.0, atk, pk, dec, sus, rel, 0.0,
		mul1, amp1, atk1, pk1, dec1, sus1, rel1, end1,
		mul2, amp2, atk2, pk2, dec2, sus2, rel2, end2);

	HANDLE mem = GlobalAlloc(0, strlen(txt)+1);
	char *ptxt = (char*)GlobalLock(mem);
	strcpy(ptxt, txt);
	GlobalUnlock(mem);

	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

float CMainDlg::GetSetting(CEdit& ed)
{
	char varstr[40];
	ed.GetWindowText(varstr, 40);
	return (float) atof(varstr);
}

void CMainDlg::PutSetting(CTrackBarCtrl& slid, CEdit& ed, double val, double div, int inv)
{
	if (inv)
		slid.SetPos(inv - (int) (val * div));
	else
		slid.SetPos((int) (val * div));
	char valstr[40];
	sprintf(valstr, "%6.3f", val);
	ed.SetWindowText(valstr);
}

AmpValue CMainDlg::CalcPhaseMod(AmpValue amp, FrqValue mult)
{
	amp = (amp * mult) * synthParams.frqTI;
	AmpValue maxPhs = synthParams.ftableLength / 2;
	if (algorithm == ALG_WYE)// || algorithm == ALG_DELTA)
		maxPhs /= 2;
	if (amp > maxPhs)
		amp = maxPhs;
	return amp;
}
	
void CMainDlg::InitGen()
{
	float pit;
	AmpValue amp;
	FrqValue frq;
	FrqValue atk;
	FrqValue dec;
	AmpValue sus;
	FrqValue rel;
	AmpValue end;
	FrqValue mul1, mul2;
	AmpValue pk;
	FrqValue nyq = synthParams.sampleRate / 2;
	
	durTotal = GetSetting(durValEd);
	volMaster = GetSetting(volValEd);
	
	pit = GetSetting(gen1FrqEd);
	frq = synthParams.GetFrequency(pit);
	if (frq >= nyq)
		frq = nyq;

	amp = GetSetting(gen1VolEd);
	atk = GetSetting(gen1AtkEd);
	dec = GetSetting(gen1DecEd);
	sus = GetSetting(gen1SusEd);
	rel = GetSetting(gen1RelEd);
	gen1EG.InitADSR(0, atk, amp, dec, sus, rel, 0, linSeg);
	gen1Osc.InitWT(frq, WT_SIN);
	if (durTotal < (rel+atk))
	{
		durTotal = atk + rel;
		durAtkSus = atk;
	}
	else
		durAtkSus = durTotal - rel;

	mul1 = GetSetting(gen2MulEd) * frq;
	amp = GetSetting(gen2NdxEd);
	atk = GetSetting(gen2AtkEd);
	pk  = GetSetting(gen2PckEd);
	dec = GetSetting(gen2DecEd);
	sus = GetSetting(gen2SusEd);
	rel = GetSetting(gen2RelEd);
	end = GetSetting(gen2EndEd);

	if (mul1 > nyq)
		mul1 = nyq;
	gen2Osc.InitWT(mul1, WT_SIN);
	if (algorithm != ALG_DELTA)
	{
		amp = CalcPhaseMod(amp, mul1);
		pk  = CalcPhaseMod(pk, mul1);
		sus = CalcPhaseMod(sus, mul1);
		end = CalcPhaseMod(end, mul1);
	}
	gen2EG.InitADSR(amp, atk, pk, dec, sus, rel, end, linSeg); 

	mul2 = GetSetting(gen3MulEd) * frq;
	amp = GetSetting(gen3NdxEd);
	atk = GetSetting(gen3AtkEd);
	pk  = GetSetting(gen3PckEd);
	dec = GetSetting(gen3DecEd);
	sus = GetSetting(gen3SusEd);
	rel = GetSetting(gen3RelEd);
	end = GetSetting(gen3EndEd);

	if (mul2 > nyq)
		mul2 = nyq;
	gen3Osc.InitWT(mul2, WT_SIN);
	amp = CalcPhaseMod(amp, mul2);
	pk  = CalcPhaseMod(pk, mul2);
	sus = CalcPhaseMod(sus, mul2);
	end = CalcPhaseMod(end, mul2);
	gen3EG.InitADSR(amp, atk, pk, dec, sus, rel, end, linSeg);
}

AmpValue CMainDlg::Generate()
{
	AmpValue gen1val = gen1EG.Gen() * gen1Osc.Gen();
	AmpValue gen2val = gen2EG.Gen() * gen2Osc.Gen();
	AmpValue gen3val = gen3EG.Gen() * gen3Osc.Gen();
	switch (algorithm)
	{
	case ALG_STACK2:
		gen2Osc.PhaseModWT(gen3val);
		// FALLTHROUGH
	case ALG_STACK:
		gen1Osc.PhaseModWT(gen2val);
		break;
	case ALG_WYE:
		gen1Osc.PhaseModWT(gen2val + gen3val);
		break;
	case ALG_DELTA:
		gen2Osc.PhaseModWT(gen3val);
		gen1Osc.PhaseModWT(gen3val);
		gen1val += gen2val;
		break;
	}
	return gen1val * volMaster;
}

void CMainDlg::NoteOff()
{
	gen1EG.Release();
	gen2EG.Release();
	gen3EG.Release();
}

int CMainDlg::PlayFM(int loop)
{
	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
    wf.nSamplesPerSec = synthParams.isampleRate;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * 2;
	wf.nBlockAlign = 2; 
    wf.wBitsPerSample = 16;
	wf.cbSize = 0;

	StopPlaying();

	InitGen();

	DWORD atkSusSamples = (DWORD) (durAtkSus * synthParams.sampleRate);
	DWORD totalSamples = (DWORD) (durTotal * synthParams.sampleRate);
	// add room for silence at end to minimize "click" when player turns off.
	DWORD bufSize = totalSamples + (DWORD) (0.1 * synthParams.sampleRate);
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
				MessageBox("Cannot open direct sound output", "Error", MB_OK|MB_ICONSTOP);
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
	{
		MessageBox("Cannot create direct sound buffer", "Error", MB_OK|MB_ICONSTOP);
		return -1;
	}
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
	if (wh.lpData == NULL)
	{
		MessageBox("Cannot create wave out buffer", "Error", MB_OK|MB_ICONSTOP);
		return -1;
	}

	res = waveOutPrepareHeader(woHandle, &wh, sizeof(wh));
	samples = (SampleValue *) wh.lpData;
#endif

	DWORD n = 0; 
	while (n++ < atkSusSamples)
		*samples++ = (SampleValue) (Generate() * synthParams.sampleScale);
	NoteOff();
	while (!gen1EG.IsFinished() && n++ < bufSize)
		*samples++ = (SampleValue) (Generate() * synthParams.sampleScale);
	while (n++ < bufSize)
		*samples++ = 0;

#ifdef USE_DIRECTSOUND
	dirSndBuf->Unlock(pAudio1, dwAudio1, pAudio2, dwAudio2);
	dirSndBuf->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
	if (!loop)
		idTimer = SetTimer(1, (UINT) (durTotal * 1000.0) + 500);
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
