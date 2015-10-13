///////////////////////////////////////////////////////////////
// This is the Windows dialog version of the virtual keyboard.
// The WidgetForm version is preferred as it is mostly portable.
// So - this code is deprecated and only kept for reference.
///////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "resource.h"
#include "KeyboardDlg.h"
#include "GenerateDlg.h"

KbdWindow::KbdWindow()
{
	playing = 0;
	lastKey = -1;
	octs = 1;
	whtKeys = 7;
	blkKeys = 5;
	rcWhite = 0;
	rcBlack = 0;
	rcLastKey = 0;
	notifyWnd = 0;
}

KbdWindow::~KbdWindow()
{
	delete[] rcWhite;
	delete[] rcBlack;
}

void KbdWindow::SetNotify(HWND w)
{
	notifyWnd = w;
}

void KbdWindow::SetOctaves(int n)
{ 
	if (n < 1)
		n = 1;
	octs = n;
	whtKeys = n * 7;
	blkKeys = n * 5;
}

LRESULT KbdWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	rcWhite = new Rect[whtKeys];
	rcBlack = new Rect[blkKeys];

	// First calculate an appropriate width/height ratio for keys
	// based on the overall size of the display area.
	// Start by calculating the width of a key as the
	// total width divided by the number of keys and the
	// height as the total height of the window. (We assume
	// the window is rectangular, wider than high.) A typical
	// piano keyboard has a ratio of 6/1 for the length
	// to width of a white key. Thus - if the resulting height
	// is more than 6 times the width, reduce the height accordingly.
	// If the resulting height is less than 5 times the width, reduce the
	// width accordingly. The black keys have a width ratio of app. 7/15 the
	// size of a white key, but the gaps in between the keys make them appear
	// wider and we use 7/10 as the ratio to make it look right and give the
	// user more area to hit with the mouse. Finally, the keyboard is centered
	// left-right at the top of the window.
	RECT rc;
	GetClientRect(&rc);
	int widWhite = (rc.right - rc.left) / whtKeys;
	int hiWhite = rc.bottom - rc.top;
	if (hiWhite > (6*widWhite))
		hiWhite = 6*widWhite;
	else if (hiWhite < (5*widWhite))
		widWhite = hiWhite / 5;
	int hiBlack = (hiWhite * 2) / 3;
	int widBlack = (widWhite * 7) / 10;
	int space = ((rc.right - rc.left) - (widWhite * whtKeys)) / 2;

	// Pre-calculate the rectangles of the keys. White keys are
	// equally spaced. Black keys repeat in the 2, 3 pattern with
	// a gap inbetween the groups equal to the width of a white key.
	int xWhite = space;
	int xBlack = space + widWhite - (widBlack / 2);
	int i, on, ndx;
	int ndxw = 0;
	int ndxb = 0;
	for (on = 0; on < octs; on++)
	{
		ndx = on * 7;
		for (i = 0; i < 7; i++)
		{
			rcWhite[ndxw].X = (int) xWhite;
			rcWhite[ndxw].Y = 0;
			rcWhite[ndxw].Width = (int) widWhite;
			rcWhite[ndxw].Height = hiWhite;
			xWhite += widWhite;
			ndxw++;
		}

		ndx = on * 5;
		for (i = 0; i < 2; i++)
		{
			rcBlack[ndxb].X = (int) xBlack;
			rcBlack[ndxb].Y = 0;
			rcBlack[ndxb].Width = (int) widBlack;
			rcBlack[ndxb].Height = hiBlack;
			xBlack += widWhite;
			ndxb++;
		}

		xBlack += widWhite;
		for ( i = 0; i < 3; i++)
		{
			rcBlack[ndxb].X = (int) xBlack;
			rcBlack[ndxb].Y = 0;
			rcBlack[ndxb].Width = (int) widBlack;
			rcBlack[ndxb].Height = hiBlack;
			xBlack += widWhite;
			ndxb++;
		}
		xBlack += widWhite;
	}

	// pitch class conversion for white and black keys
	knWhite[0] = 0;
	knWhite[1] = 2;
	knWhite[2] = 4;
	knWhite[3] = 5;
	knWhite[4] = 7;
	knWhite[5] = 9;
	knWhite[6] = 11;
	knBlack[0] = 1;
	knBlack[1] = 3;
	knBlack[2] = 6;
	knBlack[3] = 8;
	knBlack[4] = 10;

	lastKey = -1;
	rcLastKey = 0;
	return 0;
}

LRESULT KbdWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	RECT rc = ps.rcPaint;
	Rect upd(ps.rcPaint.left, ps.rcPaint.top, (ps.rcPaint.right - ps.rcPaint.left), (ps.rcPaint.bottom - ps.rcPaint.top));
	if (upd.IsEmptyArea())
	{
		RECT rc;
		GetClientRect(&rc);
		upd.X = rc.left;
		upd.Y = rc.top;
		upd.Width = (rc.right - rc.left);
		upd.Height = (rc.bottom - rc.top);
	}
	// I'm not sure why, but every now and then there is a memory exception
	// deep, deep down in GDI+. For now, we just trap the exception and blaze
	// onward. The worst case is part of the screen is not redrawn.
	try
	{
		Graphics gr(dc);
		gr.Clear(Color(92,92,64));
		LinearGradientBrush lgw(rcWhite[0], Color(255, 255, 255), Color(32,32,32), 0.0, FALSE);
		LinearGradientBrush lgk(rcWhite[0], Color(220, 220, 220), Color(8,8,8), 0.0, FALSE);
		REAL fact[4] = { 1.0f, 0.0f, 0.0f, 1.0f, };
		REAL pos[4] = { 0.0f, 0.1f, 0.9f, 1.0f };
		lgw.SetBlend(fact, pos, 4);
		lgk.SetBlend(fact, pos, 4);
		Rect *rp = rcWhite;
		int i;
		for (i = 0; i < whtKeys; i++)
		{
			if (rp->IntersectsWith(upd))
			{
				if (rp == rcLastKey)
					gr.FillRectangle(&lgk, *rp);
				else
					gr.FillRectangle(&lgw, *rp);
			}
			rp++;
		}

		SolidBrush lgb(Color(8,8,8));
		SolidBrush lgk2(Color(64,64,64));
		rp = rcBlack;
		for (i = 0; i < blkKeys; i++)
		{
			if (upd.IntersectsWith(*rp))
			{
				if (rp == rcLastKey)
					gr.FillRectangle(&lgk2, *rp);
				else
					gr.FillRectangle(&lgb, *rp);
			}
			rp++;
		}
	}
	catch(...)
	{
		OutputDebugString("Funky exception in GDI+...\r\n");
	}
	EndPaint(&ps);
	return 0;
}

LRESULT KbdWindow::OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	playing = 1;
	SetCapture();
	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	lastKey = -1;
	rcLastKey = 0;
	FindKey(pt);
	::PostMessage(notifyWnd, WM_VKBD, VKBD_KEYDN, (LPARAM)lastKey);
	return 0;
}

LRESULT KbdWindow::OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	playing = 0;
	ReleaseCapture();
	::PostMessage(notifyWnd, WM_VKBD, VKBD_KEYUP, (LPARAM)lastKey);
	InvalidateLast();
	rcLastKey = 0;

	return 0;
}

LRESULT KbdWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (playing)
	{
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (FindKey(pt))
			::PostMessage(notifyWnd, WM_VKBD, VKBD_CHANGE, (LPARAM)lastKey);
	}
	return 0;
}

int KbdWindow::FindKey(POINT& pt)
{
	int ndx;
	int kdown = -1;
	Rect *rcNewKey = 0;
	for (ndx = 0; ndx < blkKeys; ndx++)
	{
		if (rcBlack[ndx].Contains(pt.x, pt.y))
		{
			kdown = knBlack[ndx % 5] + ((ndx / 5) * 12);
			rcNewKey = &rcBlack[ndx];
			break;
		}
	}
	if (kdown == -1)
	{
		for (ndx = 0; ndx < whtKeys; ndx++)
		{
			if (rcWhite[ndx].Contains(pt.x, pt.y))
			{
				kdown = knWhite[ndx % 7] + ((ndx / 7) * 12);
				rcNewKey = &rcWhite[ndx];
				break;
			}
		}
	}
//	ATLTRACE("Keydown = %d\n", kdown);
	if (kdown != -1 && kdown != lastKey)
	{
		InvalidateLast();
		lastKey = kdown;
		rcLastKey = rcNewKey;
		InvalidateLast();
		return 1;
	}
	return 0;
}

void KbdWindow::InvalidateLast()
{
	if (rcLastKey)
	{
		RECT r;
		r.left = rcLastKey->GetLeft();
		r.right = rcLastKey->GetRight();
		r.top = rcLastKey->GetTop();
		r.bottom = rcLastKey->GetBottom();
		InvalidateRect(&r, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

KeyboardDlg::KeyboardDlg()
{
	evtID = 1;
	activeInstr = 0;
	selectInstr = 0;
	curVol = 1.0;
	curChnl = 0;
	curDur = 1.0;
	curNoteVol = 100;
	curRhythm = 1;
	kbdRunning = 0;
	genThreadH = 0;
	genThreadID = 0;
	recGroup = 0;
	recording = 0;
	useSharps = 1;
	recHead = new RecNote(0,0,0);
	recTail = new RecNote(0,0,0);
	recHead->Insert(recTail);
	LOGBRUSH lb;
	lb.lbColor = RGB(0x80,0x80,0x80);
	lb.lbHatch = 0;
	lb.lbStyle = BS_SOLID;
	bgBrush = CreateBrushIndirect(&lb);
}

KeyboardDlg::~KeyboardDlg()
{
	DeleteObject((HGDIOBJ)bgBrush);
	ClearNotes();
	delete recTail;
	delete recHead;
}

BOOL KeyboardDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL KeyboardDlg::OnIdle()
{
	return FALSE;
}

LRESULT KeyboardDlg::OnDlgColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HDC dc = (HDC)wParam;
	::SelectObject(dc, bgBrush);
	return (INT_PTR) bgBrush;
}

LRESULT KeyboardDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	volCtrl = GetDlgItem(IDC_VOLUME);
	instrList = GetDlgItem(IDC_INSTRUMENT);
	chnlList = GetDlgItem(IDC_CHANNEL);

	volCtrl.SetTicFreq(10);
	volCtrl.SetRange(0, 100, 0);
	volCtrl.SetPos(100);
	curNoteVol = 100;
	curVol = 1.0;

	CWindow frm;

	RECT rcKbd;
	GetClientRect(&rcKbd);

	RECT rc0;
	frm = GetDlgItem(IDC_KBD_LEFT);
	frm.GetClientRect(&rc0);
	frm.MapWindowPoints(m_hWnd, &rc0);

	RECT rc1;
	frm = GetDlgItem(IDC_KBD_RIGHT);
	frm.GetClientRect(&rc1);
	frm.MapWindowPoints(m_hWnd, &rc1);

	int kh = (rc0.bottom - rc0.top) / 6;
	int wh = (kh * 6) + 2;
	int ww = (kh * 49) + 4;

	rcKbd.top = rc0.top;
	rcKbd.bottom = rc0.top + wh;
	rcKbd.left = rc0.left + (((rc1.right - rc0.left) - ww)/2);
	rcKbd.right = rcKbd.left + ww;

	kbd.SetOctaves(7);
	kbd.SetNotify(m_hWnd);
	kbd.Create(m_hWnd, &rcKbd, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER, 0);

	activeInstr = 0;
	selectInstr = 0;
	evtID = 1;

	InitInstrList();
	UpdateChannels();
	CheckRadioButton(IDC_NOTE_W, IDC_NOTE_E, IDC_NOTE_Q);
	curRhythm = 4;

	SetButtonImage(IDC_NOTE_W, IDI_WHOLENOTE);
	SetButtonImage(IDC_NOTE_H, IDI_HALFNOTE);
	SetButtonImage(IDC_NOTE_Q, IDI_QUARTNOTE);
	SetButtonImage(IDC_NOTE_E, IDI_EIGHTHNOTE);
	SetButtonImage(IDC_NOTE_S, IDI_SIXTEENTHNOTE);

	SetButtonImage(IDC_GENERATE, IDI_GENERATE);
	SetButtonImage(IDC_SPEAKER, IDI_SPEAKER);
	SetButtonImage(IDC_DISK, IDI_AUDIOCD);
	SetButtonImage(IDC_START, IDI_START);
	SetButtonImage(IDC_STOP, IDI_STOP);
	::EnableWindow(GetDlgItem(IDC_STOP), 0);

	SetButtonImage(IDC_RECORD, IDI_RECORD);
	SetButtonImage(IDC_GROUP, IDI_NOTEGROUP);
	SetButtonImage(IDC_COPYNOTES, IDI_COPYNOTES);

	SetDlgItemText(IDC_PLAY_FROM, "00:00");
	SetDlgItemText(IDC_PLAY_TO, "00:00");
	CheckRadioButton(IDC_SPEAKER, IDC_DISK, IDC_SPEAKER);
	CheckRadioButton(IDC_PLAY_ALL, IDC_PLAY_SOME, IDC_PLAY_ALL);
	CheckRadioButton(IDC_USE_SHARPS, IDC_USE_FLATS, IDC_USE_SHARPS);
	return TRUE;
}

LRESULT KeyboardDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}

LRESULT KeyboardDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT KeyboardDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT KeyboardDlg::OnVolume(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int code = LOWORD(wParam);
	if (code == TB_ENDTRACK || code == TB_THUMBTRACK)
	{
		curNoteVol = volCtrl.GetPos();
		curVol = (AmpValue) curNoteVol / 100.0;
	}
	return 0;
}

LRESULT KeyboardDlg::OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = instrList.GetCurSel();
	if (sel != CB_ERR)
		selectInstr = (InstrConfig*)instrList.GetItemDataPtr(sel);
	else
		selectInstr = NULL;
	return 0;
}

LRESULT KeyboardDlg::OnChnlChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = chnlList.GetCurSel();
	if (sel != CB_ERR)
		curChnl = sel;
	return 0;
}

LRESULT KeyboardDlg::OnWholeNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	curDur = 2.0;
	curRhythm = 1;
	return 0;
}

LRESULT KeyboardDlg::OnHalfNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	curDur = 1.0;
	curRhythm = 2;
	return 0;
}

LRESULT KeyboardDlg::OnQuarterNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	curDur = 0.5;
	curRhythm = 4;
	return 0;
}

LRESULT KeyboardDlg::OnEighthNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	curDur = 0.25;
	curRhythm = 8;
	return 0;
}

LRESULT KeyboardDlg::OnSixteenthNote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	curDur = 0.125;
	curRhythm = 16;
	return 0;
}

LRESULT KeyboardDlg::OnRecordNotes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	recording = IsDlgButtonChecked(IDC_RECORD);
	useSharps = IsDlgButtonChecked(IDC_USE_SHARPS);
	::EnableWindow(GetDlgItem(IDC_COPYNOTES), !recording);
	return 0;
}

LRESULT KeyboardDlg::OnCopyNotes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!recording)
		CopyNotes();
	return 0;
}

LRESULT KeyboardDlg::OnKbd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((selectInstr == NULL && activeInstr == NULL) || theProject == NULL)
		return 0;
	if (!kbdRunning)
	{
		// auto-start
		if (!Start())
			return 0;
		while (!theProject->IsPlaying())
			Sleep(10);
	}
	// OK - this is a "wierd" possibility, (I don't think it can happen)
	// but we can handle it...
	// If a note got started and then somehow the user
	// was able to select a different instrument we need to
	// continue to use the same instrument until we get a STOP...
	if (!activeInstr)
		activeInstr = selectInstr;

	NoteEvent *evt = (NoteEvent*) theProject->mgr.ManufEvent(activeInstr);
	evt->SetParam(P_CHNL, curChnl);
	evt->SetParam(P_START, 0);
	evt->SetParam(P_DUR, curDur);
	evt->SetParam(P_VOLUME, curVol);
	evt->SetParam(P_PITCH, (float) lParam+12);
	switch (wParam)
	{
	case VKBD_KEYDN:
		evt->type = SEQEVT_START;
		evtID = (evtID + 1) & 0x7FFFFFFF;
		break;
	case VKBD_KEYUP:
		evt->type = SEQEVT_STOP;
		activeInstr = 0;
		break;
	case VKBD_CHANGE:
		evt->type = SEQEVT_PARAM;
		break;
	default:
		//OutputDebugString("Kbd event is unknown...\r\n");
		evt->Destroy();
		return 0;
	}
	evt->evid = evtID;

	//ATLTRACE("Add event %d type = %d, pitch = %d\r\n", evt->evid, evt->type, evt->pitch);
	theProject->PlayEvent(evt);
	// NB: player will delete event. Don't touch it after calling PlayEvent!

	if (recording && wParam == VKBD_KEYDN)
	{
		RecNote *note = new RecNote((int)lParam+12, curRhythm, curNoteVol);
		recTail->InsertBefore(note);
	}

	return 0;
}

LRESULT KeyboardDlg::OnGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (theProject == NULL)
		return 0;

	if (kbdRunning)
		Stop();

	GenerateDlg::playLive = IsDlgButtonChecked(IDC_SPEAKER);
	if (IsDlgButtonChecked(IDC_PLAY_SOME))
	{
		GenerateDlg::playFrom = GetTimeValue(IDC_PLAY_FROM);
		GenerateDlg::playTo  = GetTimeValue(IDC_PLAY_TO);
	}
	else
	{
		GenerateDlg::playFrom = 0;
		GenerateDlg::playTo = 0;
	}
	GenerateDlg gen(1);
	gen.DoModal();

	return 0;
}

LRESULT KeyboardDlg::OnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (theProject == NULL)
		return 0;

	if (!kbdRunning)
		Start();
	return 0;
}

LRESULT KeyboardDlg::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (theProject == NULL)
		return 0;

	if (kbdRunning)
		Stop();
	return 0;
}

void KeyboardDlg::SetButtonImage(int ctrl, int imgid)
{
	HICON ico = (HICON) LoadImage(_Module.GetResourceInstance(), (LPCSTR) imgid, IMAGE_ICON, 32, 32, 0);
	CButton btn = GetDlgItem(ctrl);
	btn.SetIcon(ico);
}

int KeyboardDlg::Stop()
{
	int wasRunning = kbdRunning;
	if (theProject)
		theProject->Stop();
	activeInstr = 0;
	CButton btn;
	btn = GetDlgItem(IDC_START);
	btn.EnableWindow(1);
	btn = GetDlgItem(IDC_STOP);
	btn.EnableWindow(0);
	return wasRunning;
}

int KeyboardDlg::Start()
{
	if (theProject)
	{
		kbdRunning = theProject->Start();
		if (kbdRunning)
		{
			CButton btn;
			btn = GetDlgItem(IDC_START);
			btn.EnableWindow(0);
			btn = GetDlgItem(IDC_STOP);
			btn.EnableWindow(1);
		}
	}
	return kbdRunning;
}

void KeyboardDlg::Clear()
{
	Stop();
	instrList.ResetContent();
	selectInstr = 0;
}

void KeyboardDlg::InitInstrList()
{
	instrList.ResetContent();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList.GetCount() > 0)
	{
		instrList.SetCurSel(0);
		selectInstr = (InstrConfig*)instrList.GetItemDataPtr(0);
	}
}

void KeyboardDlg::AddInstrument(InstrConfig *ic)
{
	char fnbuf[20];
	const char *np = ic->GetName();
	if (*np == '[')
	{
		// internal control instrument - ignore it
		return;
	}
	if (!np || *np == 0)
	{
		np = fnbuf;
		sprintf(fnbuf, "#%d", ic->inum);
	}
	int ndx = instrList.AddString(np);
	instrList.SetItemDataPtr(ndx, ic);
}

void KeyboardDlg::SelectInstrument(InstrConfig *ic)
{
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.SetCurSel(ndx);
			selectInstr = ic;
			break;
		}
		ndx++;
	}
}

void KeyboardDlg::RemoveInstrument(InstrConfig *ic)
{
	if (ic == selectInstr)
		selectInstr = 0;
	if (ic == activeInstr)
		activeInstr = 0;
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.DeleteString(ndx);
			break;
		}
		ndx++;
	}
}

// this is called if the instrument name gets changed.
void KeyboardDlg::UpdateInstrument(InstrConfig *ic)
{
	const char *nm = ic->GetName();
	if (nm == 0 || *nm == '[')
		return;
	InstrConfig *ic2;
	int count = instrList.GetCount();
	int ndx = 0;
	while (ndx < count)
	{
		ic2 = (InstrConfig*)instrList.GetItemDataPtr(ndx);
		if (ic == ic2)
		{
			instrList.DeleteString(ndx);
			ndx = instrList.AddString(nm);
			instrList.SetItemDataPtr(ndx, ic);
			break;
		}
		ndx++;
	}
}

void KeyboardDlg::UpdateChannels()
{
	chnlList.ResetContent();
	if (theProject == NULL)
		return;

	int chnls = theProject->mixInfo->GetMixerInputs();
	int n;
	for (n = 0; n < chnls; n++)
	{
		char fnbuf[20];
		sprintf(fnbuf, "%d", n);
		chnlList.InsertString(n, fnbuf);
	}
	if (curChnl >= chnls)
		curChnl = 0;
	chnlList.SetCurSel(curChnl);
}

long KeyboardDlg::GetTimeValue(int id)
{
	char buf[80];
	GetDlgItemText(id, buf, 80);
	long sec = 0;
	char *col = strchr(buf, ':');
	if (col == NULL)
	{
		sec = atol(buf);
	}
	else
	{
		*col++ = 0;
		sec = (atol(buf) * 60) + atol(col);
	}
	return sec;
}

void KeyboardDlg::ClearNotes()
{
	RecNote *note = recHead->next;
	while (note != recTail)
	{
		note->Remove();
		delete note;
	}
}

void KeyboardDlg::StartRecord()
{
	ClearNotes();
	recording = 0;
}

void KeyboardDlg::StopRecord()
{
	recording = 0;
}

void KeyboardDlg::PitchString(int pit, bsString& str)
{
	static char *pitchLtrsS[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	static char *pitchLtrsF[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
	if (useSharps)
		str += pitchLtrsS[pit%12];
	else
		str += pitchLtrsF[pit%12];
	NumberString(pit / 12, str);
}

void KeyboardDlg::RhythmString(int rhy, bsString& str)
{
	str += '%';
	NumberString(rhy, str);
}

void KeyboardDlg::NumberString(int num, bsString& str)
{
	char buf[20];
	buf[0] = 0;
	snprintf(buf, 20, "%d", num);
	str += buf;
}

void KeyboardDlg::CopyNotes()
{
	char *comma = ", ";
	char *semi  = ";\r\n";
	char *obrack = "{";
	char *cbrack = "}";

	int groupCount = 0;
	RecNote *note;
	bsString pitches;
	if (IsDlgButtonChecked(IDC_GROUP))
	{
		bsString rhythms;
		bsString volumes;
		pitches = "{ ";
		rhythms = "{ ";
		volumes = "{ ";
		while ((note = recHead->next) != recTail)
		{
			PitchString(note->key, pitches);
			RhythmString(note->dur, rhythms);
			NumberString(note->vol, volumes);
			note->Remove();
			delete note;
			if (recHead->next == recTail)
				break;
			pitches += comma;
			rhythms += comma;
			volumes += comma;
		}
		pitches += cbrack;
		pitches += comma;
		pitches += rhythms;
		pitches += cbrack;
		pitches += comma;
		pitches += volumes;
		pitches += cbrack;
		pitches += semi;
	}
	else
	{
		while ((note = recHead->next) != recTail)
		{
			PitchString(note->key, pitches);
			pitches += comma;
			RhythmString(note->dur, pitches);
			pitches += comma;
			NumberString(note->vol, pitches);
			pitches += semi;
			note->Remove();
			delete note;
		}
	}
	OpenClipboard();
	EmptyClipboard();
	size_t len = pitches.Length() + 1;
	if (len > 1)
	{
		HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, len);
		if (h)
		{
			char *p2 = (char *) GlobalLock(h);
			strncpy(p2, (const char *)pitches, len);
			GlobalUnlock(h);
			SetClipboardData(CF_TEXT, h);
		} 
	}
	CloseClipboard();
}

