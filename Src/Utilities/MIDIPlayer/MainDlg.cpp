#include "StdAfx.h"
#include "resource.h"
#include "MainDlg.h"

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT (WINAPI *tDirectSoundEnumerate)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

static BOOL CALLBACK DSDevEnum(LPGUID lpGUID, 
             LPCTSTR lpszDesc,
             LPCTSTR lpszDrvName, 
             LPVOID lpContext)
{
	CWindow *cb = (CWindow *) lpContext;
	int ndx = cb->SendMessage(CB_ADDSTRING, 0, (LPARAM)lpszDesc);
	GUID *g = NULL;
	if (lpGUID)
	{
		g = new GUID;
		memcpy(g, lpGUID, sizeof(GUID));
	}
	cb->SendMessage(CB_SETITEMDATA, ndx, (LPARAM) g);
	return TRUE;
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

	CheckDlgButton(IDC_PRELOAD, BST_UNCHECKED);
	SetDlgItemText(IDC_SCALE_ATTEN, "1.0");
	SetDlgItemText(IDC_KBD_CHANNEL, "1");
	SendDlgItemMessage(IDC_CHANNEL_SPIN, UDM_SETRANGE, 0, MAKELONG(16,1));
	SendDlgItemMessage(IDC_CHANNEL_SPIN, UDM_SETPOS, 0, 1);

	CWindow volTrk = GetDlgItem(IDC_VOLUME);

	volTrk.SendMessage(TBM_SETRANGE, 0, (LPARAM) MAKELONG(-60, 12));
	volTrk.SendMessage(TBM_SETPAGESIZE, 0, (LPARAM) 6);
	volTrk.SendMessage(TBM_SETTICFREQ, 6, 0);
	volTrk.SendMessage(TBM_SETPOS, 1, 0);
	sampleRate = 22050;

	CWindow kbdR = GetDlgItem(IDC_KBD);
	RECT rcKbd;
	kbdR.GetClientRect(&rcKbd);
	kbdR.MapWindowPoints(m_hWnd, &rcKbd);
	kbdR.DestroyWindow();

	kbd.SetOctaves(7);
	kbd.SetNotify(m_hWnd);
	kbd.Create(m_hWnd, &rcKbd, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0);

	// fill-in the list of WAV devices and select default
	CWindow cb = GetDlgItem(IDC_WAV_DEVICE);
	HMODULE h = LoadLibrary("dsound.dll");
	tDirectSoundEnumerate pDirectSoundEnumerate = NULL;
	if (h)
		pDirectSoundEnumerate = (tDirectSoundEnumerate)GetProcAddress(h, "DirectSoundEnumerateA");
	if (pDirectSoundEnumerate == NULL)
		MessageBox("Cannot locate dsound.dll.", "Sorry...", MB_OK|MB_ICONSTOP);
	else
		pDirectSoundEnumerate(DSDevEnum, &cb);
	cb.SendMessage(CB_SETCURSEL, 0, 0);

	// Load the saved file names
	CRegKey rk;
	if (rk.Open(HKEY_CURRENT_USER, "Software\\BasicSynth\\GMSynth") == ERROR_SUCCESS)
	{
		char file[MAX_PATH];
		ULONG len = MAX_PATH;
		file[0] = 0;
		if (rk.QueryStringValue("SoundBank", file, &len) == ERROR_SUCCESS)
			sf2File = file;
		len = MAX_PATH;
		file[0] = 0;
		if (rk.QueryStringValue("MIDIFile", file, &len) == ERROR_SUCCESS)
			midFile = file;
		len = MAX_PATH;
		file[0] = 0;
		if (rk.QueryStringValue("WAVFile", file, &len) == ERROR_SUCCESS)
			wavFile = file;
		DWORD dw = 0;
		if (rk.QueryDWORDValue("SampleRate", dw) == ERROR_SUCCESS)
			sampleRate = (bsInt32) dw;
		len = MAX_PATH;
		file[0] = 0;
		if (rk.QueryStringValue("Scale", file, &len) == ERROR_SUCCESS)
			SetDlgItemText(IDC_SCALE_ATTEN, file);
		rk.Close();
	}

	CWindow midiDev = GetDlgItem(IDC_MIDI_DEVICE1);
	int midiDevNo = 0;
	UINT ndev = midiInGetNumDevs(); 
	if (ndev > 0)
	{
		for (UINT n = 0; n < ndev; n++)
		{
			MIDIINCAPS caps;
			memset(&caps, 0, sizeof(caps));
			midiInGetDevCaps(n, &caps, sizeof(caps));
			midiDev.SendMessage(CB_ADDSTRING, 0, (LPARAM)caps.szPname);
			// TODO: compare name with default
		}
		midiDev.SendMessage(CB_SETCURSEL, midiDevNo, 0);
		midiAvailable = 1;
	}
	else
	{
		midiAvailable = 0;
		midiDev.EnableWindow(FALSE);
		::EnableWindow(GetDlgItem(IDC_MIDI_ON), FALSE);
	}

	// Open the synthesizer
	if (sampleRate == 44100)
		CheckDlgButton(IDC_44K, BST_CHECKED);
	else
		CheckDlgButton(IDC_22K, BST_CHECKED);
	GMSynthOpen(m_hWnd, sampleRate);
	GMSynthSetCallback(SynthMonitor, 1000, (Opaque)this);

	EnableButtons();

	return TRUE;
}

LRESULT CMainDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShutDown();
	return 0;
}

LRESULT CMainDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ShutDown();
	return 0;
}

void CMainDlg::ShutDown()
{
	HCURSOR c = SetCursor(LoadCursor(0, IDC_WAIT));
	KeyboardOff();
	GMSynthStop();
	GMSynthClose();
	CRegKey rk;
	if (rk.Create(HKEY_CURRENT_USER, "Software\\BasicSynth\\GMSynth") == ERROR_SUCCESS)
	{
		rk.SetStringValue("SoundBank", sf2File);
		rk.SetStringValue("MIDIFile", midFile);
		rk.SetStringValue("WAVFile", wavFile);
		rk.SetDWORDValue("SampleRate", (DWORD) sampleRate);
		char value[80];
		GetDlgItemText(IDC_SCALE_ATTEN, value, 80);
		rk.SetStringValue("Scale", value);
		rk.Close();
	}
	SetCursor(c);
	//DestroyWindow();
	::PostQuitMessage(0);
}

LRESULT CMainDlg::OnKbd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	static short lastKey = -1;

	int mkey = (lParam&0xff)+12;
	int mvel = (lParam>>8)&0xff;
	switch (wParam)
	{
	case VKBD_CHANGE:
		if (lastKey != -1)
			::GMSynthMIDIEvent(MIDI_NOTEON|kbdChannel, lastKey, 0);
	case VKBD_KEYDN:
		::GMSynthMIDIEvent(MIDI_NOTEON|kbdChannel, mkey, mvel);
		lastKey = mkey;
		break;
	case VKBD_KEYUP:
		::GMSynthMIDIEvent(MIDI_NOTEON|kbdChannel, mkey, 0);
		lastKey = -1;
		break;
	default:
		//OutputDebugString("Kbd event is unknown...\r\n");
		return 0;
	}
	return 0;
}

int CMainDlg::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	char spcbuf[MAX_PATH];
	strncpy(spcbuf, spec, MAX_PATH);
	if (strstr(spcbuf, "*.*") == NULL)
		strcat(spcbuf, "All Files|*.*|");
	char *pipe = spcbuf;
	while ((pipe = strchr(pipe, '|')) != 0)
		*pipe++ = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_NOCHANGEDIR;
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = spcbuf;
	ofn.lpstrDefExt = ext; 
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	int result;
	if (open)
	{
		result = GetOpenFileName(&ofn);
	}
	else
	{
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		result = GetSaveFileName(&ofn);
	}
	return result;
}

LRESULT CMainDlg::OnBnClickedLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char file[MAX_PATH];
	strcpy_s(file, MAX_PATH, midFile);
	if (BrowseFile(1, file, "MIDI|*.mid|", "mid"))
	{
		midFile = file;
		if (GMSynthLoadSequence(file, "SF", 0xffff) != GMSYNTH_NOERROR)
		{
			MessageBox("Could not load MIDI file", "Ooops...", MB_OK|MB_ICONHAND);
			return 0;
		}
		::GMSynthMetaText(GMSYNTH_META_SEQNAME, file, MAX_PATH);
		SetDlgItemText(IDC_MIDI_FILE, file);
		loaded |= 1;
		EnableButtons();
	}
	return 0;
}


LRESULT CMainDlg::OnBnClickedLoadsb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	KeyboardOff();

	char file[MAX_PATH];
	strcpy_s(file, MAX_PATH, sf2File);
	if (BrowseFile(1, file, "SoundFont|*.sf2|DLS|*.dls|", "sf2"))
	{
		sf2File = file;
		GMSynthUnload();
		Clear();

		int preload = IsDlgButtonChecked(IDC_PRELOAD);
		char scale[40];
		scale[0] = 0;
		GetDlgItemText(IDC_SCALE_ATTEN, scale, 40);
		float scl = atof(scale);
		if (scl == 0.0)
			scl = 1.0;
		SetCursor(LoadCursor(NULL,IDC_WAIT));
		if (GMSynthLoadSoundBank(file, "SF", preload, scl) != GMSYNTH_NOERROR)
		{
			MessageBox("Could not load soundfont file", "ooops", MB_OK|MB_ICONHAND);
			return 0;
		}
		::GMSynthMetaText(GMSYNTH_META_SBKNAME, file, MAX_PATH);
		SetDlgItemText(IDC_SF2_FILE, file);
		SetCursor(LoadCursor(NULL,IDC_ARROW));

		CWindow bnkList = GetDlgItem(IDC_BANK);
		bnkList.SendMessage(LB_RESETCONTENT, 0, 0);
		short banks[129];
		memset(banks, 0, sizeof(banks));
		int numBanks = ::GMSynthGetBanks(banks, 129);
		int index;
		int sel;
		for (index = 0; index < numBanks; index++)
		{
			sprintf_s(scale, 40, "Bank %d", banks[index]);
			sel = bnkList.SendMessage(LB_ADDSTRING, 0, (LPARAM)scale);
			bnkList.SendMessage(LB_SETITEMDATA, sel, banks[index]);
		}
		bnkList.SendMessage(LB_SETCURSEL, 0, 0);
		BankSelected();
		loaded |= 2;
		EnableButtons();
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	void *dev = 0;
	int sel = SendDlgItemMessage(IDC_WAV_DEVICE, CB_GETCURSEL);
	if (sel != CB_ERR)
		dev = (void *)SendDlgItemMessage(IDC_WAV_DEVICE, CB_GETITEMDATA, (WPARAM)sel);

	running = 1;
	GMSynthStart(GMSYNTH_MODE_SEQUENCE, 0, 0, dev);
	return 0;
}

LRESULT CMainDlg::OnBnClickedStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GMSynthStop();
	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	return 0;
}

LRESULT CMainDlg::OnBnClickedPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!paused)
		GMSynthPause();
	return 0;
}


LRESULT CMainDlg::OnBnClickedResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (paused)
		GMSynthResume();
	return 0;
}


LRESULT CMainDlg::OnBnClickedGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char file[MAX_PATH];
	strcpy_s(file, MAX_PATH, wavFile);
	if (BrowseFile(1, file, "WAV File|*.wav|", "wav"))
	{
		wavFile = file;
		SetDlgItemText(IDC_WAV_FILE, file);
		running = 1;
		GMSynthGenerate(file);
	}

	return 0;
}

LRESULT CMainDlg::OnStartStop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (playing)
	{
		if (wParam)
			PresetSelected();
	}
	else
		running = wParam != 0;
	EnableButtons();
	return 0;
}

LRESULT CMainDlg::OnPauseResume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	paused = (int)wParam;
	EnableButtons();
	return 0;
}

LRESULT CMainDlg::OnTick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	char buf[20];
	sprintf_s(buf, 20, "%02d:%02d", (wParam / 60), (wParam % 60));
	SetDlgItemText(IDC_TIME, buf);
	ShowTime(IDC_TIME_END);
	return 0;
}

float CMainDlg::GetTime(int id)
{
	char buf[40];
	buf[0] = 0;
	GetDlgItemText(id, buf, 40);
	char *col = strchr(buf, ':');
	if (col != 0)
	{
		*col++ = '\0';
		return (atof(buf) * 60.0) + atof(col);
	}
	return atof(buf);
}

void CMainDlg::ShowTime(int id)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	char buf[20];
	sprintf_s(buf, 20, "%02d:%02d", st.wMinute, st.wSecond);
	SetDlgItemText(id, buf);
}

void CMainDlg::EnableButtons()
{
	int off = !running && !playing;
	int pse = paused;
	int seq = running;
	int kbd = (loaded & 2) && (off || playing);
	::EnableWindow(GetDlgItem(IDCANCEL), off);
	::EnableWindow(GetDlgItem(IDC_LOAD), off && loaded & 2);
	::EnableWindow(GetDlgItem(IDC_LOADSB), off);
	::EnableWindow(GetDlgItem(IDC_STOP), seq || pse);
	::EnableWindow(GetDlgItem(IDC_PAUSE), seq && !pse);
	::EnableWindow(GetDlgItem(IDC_RESUME), pse);
	::EnableWindow(GetDlgItem(IDC_PLAY), off && loaded == 3);
	::EnableWindow(GetDlgItem(IDC_GENERATE), off && loaded == 3);
	::EnableWindow(GetDlgItem(IDC_22K), off);
	::EnableWindow(GetDlgItem(IDC_44K), off);
	::EnableWindow(GetDlgItem(IDC_WAV_DEVICE), off);
	::EnableWindow(GetDlgItem(IDC_KBD_ON), kbd);
	::EnableWindow(GetDlgItem(IDC_MIDI_ON), kbd && midiAvailable);
	::EnableWindow(GetDlgItem(IDC_MIDI_DEVICE1), kbd && midiAvailable);
	::EnableWindow(GetDlgItem(IDC_BANK), kbd);
	::EnableWindow(GetDlgItem(IDC_PRESET), kbd);
}

// Callbacks from the synth. These come in on the player thread,
// so we need to return ASAP.
void CMainDlg::SynthMonitor(bsInt32 evtid, bsInt32 count, Opaque arg)
{
	CMainDlg *dlg = (CMainDlg*)arg;
	switch (evtid)
	{
	case GMSYNTH_EVENT_START:
		dlg->PostMessage(WM_USER+10, 1, 0);
		break;
	case GMSYNTH_EVENT_STOP:
		dlg->PostMessage(WM_USER+10, 0, 0);
		break;
	case GMSYNTH_EVENT_TICK:
		dlg->PostMessage(WM_USER+11, (WPARAM) count, 0);
		break;
	case GMSYNTH_EVENT_PAUSE:
		dlg->PostMessage(WM_USER+12, 1, 0);
		break;
	case GMSYNTH_EVENT_RESUME:
		dlg->PostMessage(WM_USER+12, 0, 0);
		break;
	}
}

LRESULT CMainDlg::OnBnClicked22k(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SampleRate();
	return 0;
}

LRESULT CMainDlg::OnBnClicked44k(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SampleRate();
	return 0;
}

void CMainDlg::SampleRate()
{
	int newSR = 0;
	if (IsDlgButtonChecked(IDC_22K))
		newSR = 22050;
	else
		newSR = 44100;
	if (newSR == sampleRate)
		return;
	if (loaded)
	{
		if (MessageBox("Changing the sample rate will clear all loaded files. Proceed?", "Wait...", MB_OKCANCEL) == IDCANCEL)
			return;
	}
	KeyboardOff();
	sampleRate = newSR;
	GMSynthClose();
	GMSynthOpen(m_hWnd, sampleRate);
	GMSynthSetCallback(SynthMonitor, 1000, (Opaque)this);
	loaded = 0;
	running = 0;
	playing = 0;
	Clear();
	EnableButtons();
}

LRESULT CMainDlg::OnVolume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD op = LOWORD(wParam);
	CWindow volTrk = GetDlgItem(IDC_VOLUME);
    int pos = (int) volTrk.SendMessage(TBM_GETPOS, 0, 0);
	switch (op)
	{
	case TB_BOTTOM:
		pos = -60;
		break;
	case TB_TOP:
		pos = 12;
		break;
	case TB_LINEDOWN:
		pos--;
		break;
	case TB_LINEUP:
		pos++;
		break;
	case TB_PAGEDOWN:
		pos -= 6;
		break;
	case TB_PAGEUP:
		pos += 6;
		break;
	case TB_THUMBPOSITION:
	case TB_ENDTRACK:
	case TB_THUMBTRACK:
		break;
	}
    volTrk.SendMessage(TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
	GMSynthSetVolume((float)pos, 0.5);
	return 0;
}

LRESULT CMainDlg::OnKbdOn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	kbdChannel = (GetDlgItemInt(IDC_KBD_CHANNEL) - 1) & 0x0F;
	if (IsDlgButtonChecked(IDC_KBD_ON) == BST_CHECKED)
	{
		playing = 1;
		void *dev = 0;
		int sel = SendDlgItemMessage(IDC_WAV_DEVICE, CB_GETCURSEL);
		if (sel != CB_ERR)
			dev = (void *)SendDlgItemMessage(IDC_WAV_DEVICE, CB_GETITEMDATA, (WPARAM)sel);
		::GMSynthStart(GMSYNTH_MODE_PLAY, 0, 0, dev);
		SetBank();
	}
	else
	{
		playing = 0;
		::GMSynthStop();
	}

	return 0;
}

LRESULT CMainDlg::OnBank(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BankSelected();
	return 0;
}

LRESULT CMainDlg::OnPreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PresetSelected();
	return 0;
}

LRESULT CMainDlg::OnDeltaposChannelSpin(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
//	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	kbdChannel = (GetDlgItemInt(IDC_KEYBOARD_CHANNEL) - 1) & 0x0F;

	return 0;
}

void CMainDlg::BankSelected()
{
	CWindow bankList = GetDlgItem(IDC_BANK);
	int sel = bankList.SendMessage(LB_GETCURSEL, 0, 0);
	CWindow preList = GetDlgItem(IDC_PRESET);
	preList.SendMessage(LB_RESETCONTENT, 0, 0);
	if (sel == LB_ERR)
	{
		sf2Bank = 0;
		sf2Preset = 0;
	}
	else
	{
		sf2Bank = bankList.SendMessage(LB_GETITEMDATA, sel, 0);
		for (int preset = 0; preset < 128; preset++)
		{
			char name[80];
			memset(name, 0, 80);
			if (::GMSynthGetPreset(sf2Bank, preset, name, 80) == GMSYNTH_NOERROR)
			{
				char disp[80];
				sprintf_s(disp, 80, "%03d: %s", preset, name);
				int index = preList.SendMessage(LB_ADDSTRING, 0, (LPARAM)disp);
				preList.SendMessage(LB_SETITEMDATA, index, preset);
			}
		}
		sf2Preset = preList.SendMessage(LB_GETITEMDATA, 0, 0);
		preList.SendMessage(LB_SETCURSEL, 0, 0);
	}
	SetBank();
	PresetSelected();
}

void CMainDlg::PresetSelected()
{
	CWindow preList = GetDlgItem(IDC_PRESET);
	int sel = preList.SendMessage(LB_GETCURSEL, 0, 0);
	if (sel == LB_ERR)
		return;
	sf2Preset = preList.SendMessage(LB_GETITEMDATA, sel, 0);
	::GMSynthMIDIEvent(MIDI_PRGCHG|kbdChannel, sf2Preset, 0);
}

void CMainDlg::SetBank()
{
	if (sf2Bank == 128)
	{
		::GMSynthMIDIEvent(MIDI_CTLCHG|kbdChannel, MIDI_CTRL_BANK, 0x78);
		::GMSynthMIDIEvent(MIDI_CTLCHG|kbdChannel, MIDI_CTRL_BANK_LSB, 0);
	}
	else
	{
		::GMSynthMIDIEvent(MIDI_CTLCHG|kbdChannel, MIDI_CTRL_BANK, 0x79);
		::GMSynthMIDIEvent(MIDI_CTLCHG|kbdChannel, MIDI_CTRL_BANK_LSB, (short)sf2Bank);
	}
}

void CMainDlg::Clear()
{
	SetDlgItemText(IDC_SF2_FILE, "");
	SetDlgItemText(IDC_MIDI_FILE, "");
	SendDlgItemMessage(IDC_BANK, LB_RESETCONTENT, 0);
	SendDlgItemMessage(IDC_PRESET, LB_RESETCONTENT, 0);
}

void CMainDlg::KeyboardOff()
{
	if (playing)
	{
		::GMSynthStop();
		playing = 0;
		CheckDlgButton(IDC_KBD_ON, BST_UNCHECKED);
	}
}