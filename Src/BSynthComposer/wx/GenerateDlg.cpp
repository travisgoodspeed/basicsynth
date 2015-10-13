//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "GenerateDlg.h"

long GenerateDlg::playFrom;
long GenerateDlg::playTo;
int  GenerateDlg::playLive;
int  GenerateDlg::playSome;
long GenerateDlg::updateRate = 1;
int GenerateDlg::pkOn = 1;
int GenerateDlg::tmOn = 1;

DEFINE_EVENT_TYPE(genMessage)
DEFINE_EVENT_TYPE(genEnd)
DEFINE_EVENT_TYPE(genUpdTime)
DEFINE_EVENT_TYPE(genUpdPeak)

BEGIN_EVENT_TABLE(GenerateDlg,wxDialog)
	EVT_BUTTON(XRCID("IDC_PLAY_LIVE"), GenerateDlg::OnSelectLive)
	EVT_BUTTON(XRCID("IDC_DISK"), GenerateDlg::OnSelectDisk)
	EVT_BUTTON(XRCID("IDC_PLAY_SOME"), GenerateDlg::OnSelectSome)
	EVT_BUTTON(XRCID("IDC_PLAY_ALL"), GenerateDlg::OnSelectAll)
	EVT_BUTTON(XRCID("IDC_START"), GenerateDlg::OnStart)
	EVT_BUTTON(XRCID("IDC_STOP"), GenerateDlg::OnStop)
	EVT_BUTTON(XRCID("IDC_PAUSE"), GenerateDlg::OnPause)
	EVT_CHECKBOX(XRCID("IDC_TIME_ON"), GenerateDlg::OnTimeOn)
	EVT_CHECKBOX(XRCID("IDC_PEAK_ON"), GenerateDlg::OnPeakOn)
	EVT_BUTTON(wxID_OK, GenerateDlg::OnOK)
	EVT_BUTTON(wxID_CANCEL, GenerateDlg::OnCancel)
	EVT_COMMAND(XRCID("DLG_GENERATE"), genMessage, GenerateDlg::OnUpdateMsg)
	EVT_COMMAND(XRCID("DLG_GENERATE"), genEnd, GenerateDlg::OnGenFinished)
	EVT_COMMAND(XRCID("DLG_GENERATE"), genUpdTime, GenerateDlg::OnUpdateTime)
	EVT_COMMAND(XRCID("DLG_GENERATE"), genUpdPeak, GenerateDlg::OnUpdatePeak)
END_EVENT_TABLE()

GenerateDlg::GenerateDlg(wxWindow *parent, int a, int pl)
{
	genAuto = a;
	lastTime = 0;
	canceled = 0;
	paused = 0;
	if (pl >= 0)
		playLive = pl;
	dlgLock.Create();

	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_GENERATE");
	CenterOnParent();
	EnableOK(1, 0);

	wxSize sz;
	wxPoint pt;

	ed = (wxTextCtrl*)FindWindow("IDC_TEXT");
	tm = (wxTextCtrl*)FindWindow("IDC_TIME");
	tm->ChangeValue("00:00.0");
	sz = tm->GetSize();
	pt = tm->GetPosition();
	pt.x += sz.GetWidth() + 2;
	wxWindow *btn = FindWindow(wxID_OK);
	sz.SetWidth(btn->GetPosition().x + btn->GetSize().GetWidth() - pt.x);
	tml = new BarMeter(this, wxWindowID(102), pt, sz);

	lpk = (wxTextCtrl*)FindWindow("IDC_LEFT_PEAK");
	rpk = (wxTextCtrl*)FindWindow("IDC_RIGHT_PEAK");

	sz = lpk->GetSize();
	pt = lpk->GetPosition();
	pt.x += sz.GetWidth() + 2;
	sz.SetWidth(sz.GetWidth() * 2);
	lpkMtr = new BarMeter(this, wxWindowID(100), pt, sz);
	lpkMtr->SetRange(1.2);
	lpkMtr->Show();

	sz = rpk->GetSize();
	pt = rpk->GetPosition();
	pt.x += sz.GetWidth() + 2;
	sz.SetWidth(sz.GetWidth() * 2);
	rpkMtr = new BarMeter(this, wxWindowID(100), pt, sz);
	rpkMtr->SetRange(1.2);
	rpkMtr->Show();

	rateWnd = (wxComboBox*)FindWindow("IDC_RATE");
	rateWnd->Append("0.1 Sec", (void*)1);
	rateWnd->Append("0.5 Sec", (void*)5);
	rateWnd->Append("1.0 Sec", (void*)10);
	rateWnd->Append("5.0 Sec", (void*)50);
	rateWnd->Select(0);

	SetPlayOutput();
	SetPlayRange();

	FormatTime("IDC_PLAY_FROM", playFrom);
	FormatTime("IDC_PLAY_TO", playTo);

	wxCheckBox *on;
	on = (wxCheckBox*)FindWindow("IDC_PEAK_ON");
	on->SetValue(pkOn);
	on = (wxCheckBox*)FindWindow("IDC_TIME_ON");
	on->SetValue(tmOn);

	if (genAuto)
	{
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, XRCID("IDC_START"));
		evt.SetEventObject(this);
		wxPostEvent(this, evt);
	}
}

GenerateDlg::~GenerateDlg(void)
{
	prjGenerate = 0;
	dlgLock.Destroy();
}

void GenerateDlg::SetPlayOutput()
{
	wxColor sel(200,200,255);
	wxColor uns(240,240,255);
	wxBitmapButton *tbtn;
	tbtn = (wxBitmapButton*)FindWindow("IDC_DISK");
	tbtn->SetBackgroundColour(playLive ? uns : sel);
	tbtn->Enable(playLive);
	tbtn = (wxBitmapButton*)FindWindow("IDC_PLAY_LIVE");
	tbtn->SetBackgroundColour(playLive ? sel : uns);
	tbtn->Enable(!playLive);
}

void GenerateDlg::SetPlayRange()
{
	wxColor sel(200,200,255);
	wxColor uns(240,240,255);
	wxBitmapButton *tbtn;
	tbtn = (wxBitmapButton*)FindWindow("IDC_PLAY_ALL");
	tbtn->SetBackgroundColour(playSome ? uns : sel);
	tbtn->Enable(playSome);
	tbtn = (wxBitmapButton*)FindWindow("IDC_PLAY_SOME");
	tbtn->Enable(!playSome);
	tbtn->SetBackgroundColour(playSome ? sel : uns);
}

void GenerateDlg::OnSelectSome(wxCommandEvent& evt)
{
	playSome = 1;
	SetPlayRange();
}

void GenerateDlg::OnSelectAll(wxCommandEvent& evt)
{
	playSome = 0;
	SetPlayRange();
}

void GenerateDlg::OnSelectDisk(wxCommandEvent& evt)
{
	playLive = 0;
	SetPlayOutput();
}

void GenerateDlg::OnSelectLive(wxCommandEvent& evt)
{
	playLive = 1;
	SetPlayOutput();
}

void GenerateDlg::OnTimeOn(wxCommandEvent& evt)
{
	tmOn = evt.IsChecked();
}


void GenerateDlg::OnPeakOn(wxCommandEvent& evt)
{
	pkOn = evt.IsChecked();
}

void GenerateDlg::OnStart(wxCommandEvent& evt)
{
	prjFrame->GenerateStarted();
	EnableOK(0, 0);
	ed->AppendText("---------- Start ----------\r\n");
	canceled = 0;
	paused = 0;

	int sel = rateWnd->GetCurrentSelection();
	if (sel != wxNOT_FOUND)
		updateRate = (long) rateWnd->GetClientData(sel);

	if (!genAuto)
	{
		playFrom = GetTimeValue("IDC_PLAY_FROM");
		playTo  = GetTimeValue("IDC_PLAY_TO");
	}
	if (playSome)
	{
		theProject->StartTime(playFrom);
		theProject->EndTime(playTo);
		startTime = playFrom;
	}
	else
	{
		theProject->StartTime(0);
		theProject->EndTime(0);
		startTime = 0;
	}
	lastTime = 0;
	theProject->PlayMode(playLive);

	lastMsg = "";
	FormatTime("IDC_TIME", startTime);
	tml->Reset();
	lpkMtr->Reset();
	rpkMtr->Reset();
	lftPeak = 0;
	rgtPeak = 0;
	lftMax = 0;
	rgtMax = 0;
	FormatPeak();
	prjGenerate = static_cast<GenerateWindow*>(this);

	theProject->CallbackRate((float)updateRate / 10.0f);
	if (theProject->Start() == 0)
		EnableOK(0, 1);
	else
		EnableOK(1, 0);
}

void GenerateDlg::OnStop(wxCommandEvent& evt)
{
	dlgLock.Enter();
	lastMsg += "*Cancel*\r\n";
	ed->AppendText(lastMsg);
	lastMsg = "";
	canceled = 1;
	try
	{
		if (theProject->cvtActive)
			theProject->cvtActive->Cancel();
	}
	catch(...)
	{
	}
	dlgLock.Leave();
}

void GenerateDlg::OnPause(wxCommandEvent& evt)
{
//	wxBitmapButton *btn = (wxBitmapButton*)FindWindow("IDC_PAUSE");
	if (!paused)
	{
		theProject->Pause();
		paused = 1;
	}
	else
	{
		theProject->Resume();
		paused = 1;
	}
}


void GenerateDlg::OnGenFinished(wxCommandEvent& evt)
{
	theProject->WaitThread();
	prjGenerate = 0;
	char pk[200];
	snprintf(pk, 200, "Peak: [%.3f, %.3f]\r\n-------- Finished ---------\r\n", lftMax, rgtMax);
	ed->AppendText(pk);
	prjFrame->GenerateFinished();
	lftPeak = lftMax;
	rgtPeak = rgtMax;
	FormatPeak(0);
	if (genAuto)
		EndModal(1);
	else
		EnableOK(1, 0);
}

void GenerateDlg::OnUpdateMsg(wxCommandEvent& evt)
{
	dlgLock.Enter();
	if (lastMsg.Length() != 0)
	{
		ed->AppendText(lastMsg);
		lastMsg = "";
	}
	dlgLock.Leave();
}

void GenerateDlg::OnUpdateTime(wxCommandEvent& evt)
{
	long thisTime = evt.GetExtraLong();
	if (thisTime == 0)
	{
		tml->SetRange((float)(theProject->seq.GetLength() * 10) / synthParams.sampleRate);
	}
	lastTime = (thisTime * updateRate) + startTime;
	if (tmOn)
	{
		FormatTime("IDC_TIME", lastTime);
		tml->SetValue((float)lastTime);
	}
}

void GenerateDlg::OnUpdatePeak(wxCommandEvent& evt)
{
	FormatPeak(1);
}

void GenerateDlg::OnOK(wxCommandEvent& evt)
{
	EndModal(1);
}

void GenerateDlg::OnCancel(wxCommandEvent& evt)
{
	EndModal(0);
}

void GenerateDlg::FormatTime(const char *id, long secs)
{
	wxWindow *w = FindWindow(id);
	if (w && w->IsKindOf(CLASSINFO(wxTextCtrl)))
	{
		char txt[16];
		snprintf(txt, 16, "%02ld:%02ld.%1ld", secs / 600, (secs / 10) % 60, secs % 10);
		((wxTextCtrl*)w)->ChangeValue(txt);
	}
}

void GenerateDlg::FormatPeak(int oor)
{
	char pkText[100];
	snprintf(pkText, 100, "%.3f", lftPeak);
	lpk->ChangeValue(pkText);
	snprintf(pkText, 100, "%.3f", rgtPeak);
	rpk->ChangeValue(pkText);
	if (oor && (lftPeak > 1.0 || rgtPeak > 1.0))
	{
		snprintf(pkText, 100, "Out of range (%6.3f, %6.3f) at %02ld:%02ld.%1ld\r\n",
			lftPeak, rgtPeak, lastTime / 600, (lastTime / 10) % 60, lastTime % 10);
		ed->AppendText(pkText);
	}
	lpkMtr->SetValue(lftPeak);
	rpkMtr->SetValue(rgtPeak);
	if (lftPeak > lftMax)
		lftMax = lftPeak;
	if (rgtPeak > rgtMax)
		rgtMax = rgtPeak;
}

void GenerateDlg::EnableOK(int e, int c)
{
	FindWindow(wxID_OK)->Enable(e);
	FindWindow("IDC_RATE")->Enable(e);
	FindWindow("IDC_PLAY_SOME")->Enable(e && !playSome);
	FindWindow("IDC_PLAY_ALL")->Enable(e && playSome);
	FindWindow("IDC_DISK")->Enable(e && playLive);
	FindWindow("IDC_PLAY_LIVE")->Enable(e && !playLive);
	FindWindow("IDC_STOP")->Enable(c);
	FindWindow("IDC_START")->Enable(!c);
	FindWindow("IDC_PAUSE")->Enable(c);
}

long GenerateDlg::GetTimeValue(const char *id)
{
	int minutes = 0;
	int seconds = 0;
	int tenths = 0;
	wxTextCtrl *w = (wxTextCtrl *)FindWindow(id);
	if (w && w->IsKindOf(CLASSINFO(wxTextCtrl)))
	{
		wxString txt(w->GetValue());
		int delim = txt.Find(':');
		if (delim >= 0)
		{
			wxString m(txt.Left(delim));
			minutes = atol(m);
			wxString s(txt.Mid(delim+1));
			seconds = atol(s);
		}
		else
			seconds = atol(txt);
		delim = txt.Find('.');
		if (delim >= 0)
		{
			wxString t(txt.Mid(delim+1));
			tenths = atol(t);
		}
	}
	return (minutes * 600) + (seconds * 10) + tenths;
}

void GenerateDlg::AddMessage(const char *s)
{
	dlgLock.Enter();
	lastMsg += s;
	lastMsg += "\r\n";
	dlgLock.Leave();
	wxCommandEvent evt(genMessage, GetId());
	evt.SetEventObject(this);
	wxPostEvent(this, evt);
}

void GenerateDlg::UpdateTime(long tmval)
{
	wxCommandEvent evt(genUpdTime, GetId());
	evt.SetEventObject(this);
	evt.SetExtraLong(tmval);
	wxPostEvent(this, evt);
}

void GenerateDlg::Finished()
{
	wxCommandEvent evt(genEnd, GetId());
	evt.SetEventObject(this);
	wxPostEvent(this, evt);
}

int GenerateDlg::WasCanceled()
{
	return canceled;
}

void GenerateDlg::UpdatePeak(AmpValue lft, AmpValue rgt)
{
	lftPeak = lft;
	rgtPeak = rgt;
	if (pkOn)
	{
		wxCommandEvent evt(genUpdPeak, GetId());
		evt.SetEventObject(this);
		wxPostEvent(this, evt);
	}
}

///////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(BarMeter,wxWindow)
	EVT_PAINT(BarMeter::OnPaint)
	EVT_ERASE_BACKGROUND(BarMeter::OnEraseBackground)
END_EVENT_TABLE()

void BarMeter::SetRange(float rng)
{
	barValue = 0.0;
	barRange = rng;
}

void BarMeter::SetValue(float val)
{
	if (val != barValue)
	{
		if ((barValue = val) > barRange)
			barValue = barRange;
		Refresh();
	}
}

void BarMeter::OnEraseBackground(wxEraseEvent& evt)
{
}

void BarMeter::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	wxSize sz = GetClientSize();
	int cx = sz.GetWidth();
	int cy = sz.GetHeight();
	wxBrush br(back);
	dc.SetBrush(br);
	wxPen pn(back);
	dc.SetPen(pn);
	dc.DrawRectangle(0, 0, cx, cy);
	br.SetColour(bar);
	pn.SetColour(bar);
	dc.SetBrush(br);
	dc.SetPen(pn);
	dc.DrawRectangle(2, 2, (int)((float)cx * barValue / barRange), cy-4);
}
