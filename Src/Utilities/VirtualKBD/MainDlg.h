/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Virtual Keyboard instrument player, Main window
//
// This window contains the list of instruments, keyboard, and start/stop
// controls. The keyboard is a separate window added to the dialog during
// initialization.
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
	KbdWindow kbd;
	CComboBox instrList;
	CTrackBarCtrl volCtrl;
	InstrConfig *activeInstr;
	bsInt32 evtID;
	HANDLE genThreadH;
	DWORD genThreadID;

public:
	enum { IDD = IDD_MAINDLG };

	CMainDlg()
	{
		evtID = 1;
		genThreadH = INVALID_HANDLE_VALUE;
		genThreadID = 0;
		activeInstr = 0;
	}

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
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_VKBD, OnKbd)
		MESSAGE_HANDLER(WM_VSCROLL, OnVolume)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_CODE_HANDLER(CBN_SELENDOK, OnInstrChange)
		COMMAND_ID_HANDLER(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER(IDC_START, OnStart)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

		volCtrl = GetDlgItem(IDC_VOLUME);
		volCtrl.SetTicFreq(10);
		volCtrl.SetRange(0, 100, 0);
		RECT rc0;
		volCtrl.GetClientRect(&rc0);
		volCtrl.MapWindowPoints(m_hWnd, &rc0);
		volCtrl.SetPos(30);

		instrList = GetDlgItem(IDC_INSTRUMENT);
		RECT rc1;
		instrList.GetClientRect(&rc1);
		instrList.MapWindowPoints(m_hWnd, &rc1);

		RECT rc2;
		GetClientRect(&rc2);
		rc2.top = rc1.bottom + 8;
		rc2.left = 8;
		rc2.right = rc0.left - 8;
		rc2.bottom -= 8;

		kbd.SetOctaves(7);
		kbd.SetNotify(m_hWnd);
		kbd.Create(m_hWnd, &rc2, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0);

		activeInstr = 0;
		evtID = 1;

		return TRUE;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnVolume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int code = LOWORD(wParam);
		if (code == TB_ENDTRACK || code == TB_THUMBTRACK)
		{
			AmpValue val = (AmpValue) (100 - volCtrl.GetPos()) / 100.0;
			theMixer.ChannelVolume(0, val);
		}
		return 0;
	}

	LRESULT OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		int sel = instrList.GetCurSel();
		if (sel != CB_ERR)
			activeInstr = (InstrConfig*)instrList.GetItemDataPtr(sel);
		else
			activeInstr = NULL;
		return 0;
	}

	LRESULT OnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		char fnbuf[MAX_PATH];
		fnbuf[0] = '\0';
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFilter = "Projects\0*.bsprj\0Libraries\0*.bslib\0XML Files\0*.xml\0All files (*.*)\0*.*\0";
		ofn.lpstrDefExt = "bsprj";
		ofn.lpstrFile = fnbuf;
		ofn.nMaxFile = MAX_PATH;

		if (GetOpenFileName(&ofn))
		{
			instrList.ResetContent();
			instrMgr.Clear();
			SynthProject prj;
			if (prj.LoadProject(fnbuf, instrMgr) != 0)
				MessageBox("Something didn't work. Not all instruments were loaded.", "Oooops...", MB_OK);
			InstrConfig *ime = 0;
			while ((ime = instrMgr.EnumInstr(ime)) != 0)
			{
				const char *np = ime->GetName();
				if (!np || *np == 0)
				{
					np = fnbuf;
					sprintf(fnbuf, "#%d", ime->inum);
				}
				int ndx = instrList.AddString(np);
				instrList.SetItemDataPtr(ndx, ime);
			}
			instrList.SetCurSel(0);
			activeInstr = (InstrConfig*)instrList.GetItemDataPtr(0);
		}
		return 0;
	}

	LRESULT OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnKbd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PlayNote((int)wParam, (int)(lParam & 0xff), (lParam >> 8) & 0xff);
		return 0;
	}

	void PlayNote(int e, int key, int vel)
	{
		if (activeInstr == NULL || genThreadH == INVALID_HANDLE_VALUE)
			return;

		if (e == VKBD_CHANGE)
		{
			PlayNote(VKBD_KEYUP, key, vel);
			e = VKBD_KEYDN;
		}

		NoteEvent *evt = (NoteEvent*) instrMgr.ManufEvent(activeInstr);
		//evt->SetParam(P_INUM, activeInstr->inum);
		evt->SetChannel(0);
		evt->SetStart(0);
		evt->SetDuration(0);
		evt->SetVolume(1.0);
		evt->SetPitch(key);
		evt->SetVelocity(vel);

		switch (e)
		{
		case VKBD_KEYDN:
			evt->type = SEQEVT_START;
			evtID = (evtID + 1) & 0x7FFFFFFF;
			break;
		case VKBD_KEYUP:
			evt->type = SEQEVT_STOP;
			break;
		//case VKBD_CHANGE:
		//	evt->type = SEQEVT_PARAM;
		//	break;
		default:
			//OutputDebugString("Kbd event is unknown...\r\n");
			return;
		}
		evt->evid = evtID;

		//char buf[80];
		//sprintf(buf, "Add event %d type = %d, pitch = %d\r\n", evt->evid, evt->type, evt->pitch);
		//OutputDebugString(buf);
		thePlayer.AddEvent(evt);
		// NB: player will delete event. Don't touch it after calling AddEvent!
	}

	static DWORD WINAPI GenerateProc(LPVOID param)
	{
		thePlayer.Play(instrMgr);
		ExitThread(0);
	}

	LRESULT OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (genThreadH != INVALID_HANDLE_VALUE)
		{
			thePlayer.Halt();
			WaitForSingleObject(genThreadH, 10000);
			waveOut.Stop();
			genThreadH = INVALID_HANDLE_VALUE;
			CheckDlgButton(IDC_START, BST_UNCHECKED);
			SetDlgItemText(IDC_START, "Start");
			::EnableWindow(GetDlgItem(IDC_LOAD), TRUE);
		}
		else
		{
			if (waveOut.Setup(m_hWnd, 0.02, 4) != 0)
			{
				MessageBox("Failed to create playback output", "Ooops...", MB_OK);
				return 0;
			}
			genThreadH = CreateThread(NULL, 0, GenerateProc, NULL, CREATE_SUSPENDED, &genThreadID);
			if (genThreadH != INVALID_HANDLE_VALUE)
			{
				SetDlgItemText(IDC_START, "Stop");
				CheckDlgButton(IDC_START, BST_CHECKED);
				::EnableWindow(GetDlgItem(IDC_LOAD), FALSE);
				ResumeThread(genThreadH);
			}
		}
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CloseDialog(1);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		BOOL b = 1;
		if (genThreadH != INVALID_HANDLE_VALUE)
			OnStart(0, 0, 0, b);
		DestroyWindow();
		::PostQuitMessage(nVal);
	}
};
