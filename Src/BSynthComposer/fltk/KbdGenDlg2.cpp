//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file KbdGenDlg2.cpp Keyboard window implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "KbdGenDlg2.h"
#include "MainFrm.h"


long GenerateDlg::playFrom;
long GenerateDlg::playTo;
long GenerateDlg::playLive;
long GenerateDlg::updateRate = 1;

static void FormatTime(Fl_Input *wdg, long secs)
{
	char txt[40];
	snprintf(txt, 40, "%3d:%02d.%1d", (secs / 600), (secs / 10) % 60, secs % 10);
	wdg->value(txt);
}

static long GetTimeValue(Fl_Input *wdg)
{
	long minutes = 0;
	long seconds = 0;
	long tenths = 0;
	char buf[80];
	strncpy(buf, wdg->value(), 80);
	char *bp = buf;
	while (isspace(*bp))
		bp++;
	char *delim = strchr(bp, ':');
	if (delim)
	{
		minutes = atol(buf);
		bp = delim + 1;
	}
	if (isdigit(*bp))
		seconds = atol(bp);
	if ((delim = strchr(bp, '.')) != NULL)
		tenths = atol(delim+1);
	return (minutes * 600) + (seconds * 10) + tenths;
}


static void CloseCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnClose();
}

static void StartCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnStart(0);
}

static void StopCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnStop();
}

static void PauseCB(Fl_Widget *wdg, void *arg)
{
	((GenerateDlg*)arg)->OnPause();
}

GenerateDlg::GenerateDlg(int live) : Fl_Window(100, 100, 600, 320, "Generate")
{
    updateRate = 1;
	playLive = live;
	playFrom = 0;
	playTo = 0;
	prjGenerate = static_cast<GenerateWindow*>(this);

	lastTime = 0;
	canceled = 0;

	Fl_Box *lbl;
	Fl_Group *grp;

	int topx = 5;
	int topy = 5;
	int topw = 40;
	int toph = 40;
	startBtn = new Fl_Button(topx, topy, topw, toph, "@+2|>");
	startBtn->labelcolor(FL_GREEN);
	startBtn->callback(StartCB, (void*)this);
	startBtn->activate();
	topx += topw + 5;

	stopBtn = new Fl_Button(topx, topy, topw, toph, "@+2square");
	stopBtn->labelcolor(FL_RED);
	stopBtn->callback(StopCB, (void*)this);
	stopBtn->deactivate();
	topx += topw + 5;

	pauseBtn = new Fl_Button(topx, topy, topw, toph, "@+2||");
	pauseBtn->type(FL_TOGGLE_BUTTON);
	pauseBtn->callback(PauseCB, (void*)this);
	pauseBtn->deactivate();
	pauseBtn->labelcolor(FL_BLUE);
	pauseBtn->value(0);
	topx += topw + 5;

	topw = 60;
	grp = new Fl_Group(topx, topy, (topw*2)+15, toph, 0);
	topx += 5;
	spkrBtn = new Fl_Light_Button(topx, topy, topw, toph, "Audio");
	spkrBtn->type(FL_RADIO_BUTTON);
	spkrBtn->value(playLive);
	topx += topw + 5;
	diskBtn = new Fl_Light_Button(topx, topy, topw, toph, "Disk");
	diskBtn->type(FL_RADIO_BUTTON);
	diskBtn->value(!playLive);
	topx += topw + 5;
	grp->end();

	grp = new Fl_Group(topx, topy, (topw*2)+15, toph, 0);
	topx += 5;
	allBtn = new Fl_Light_Button(topx, topy, topw, toph, "All");
	allBtn->type(FL_RADIO_BUTTON);
	allBtn->value(1);
	topx += topw + 5;
	someBtn = new Fl_Light_Button(topx, topy, topw, toph, "Part");
	someBtn->type(FL_RADIO_BUTTON);
	someBtn->value(0);
	topx += topw + 5;
	grp->end();

	topw = 40;
	toph = 20;
	lbl = new Fl_Box(topx, topy, topw, toph, "From");
	lbl = new Fl_Box(topx, topy+toph, topw, toph, "To");
	topx += topw + 5;
	topw = 60;
	fromInp = new Fl_Input(topx, topy, topw, toph, 0);
	toInp = new Fl_Input(topx, topy+toph, topw, toph, 0);
	topx += topw + 5;

	closeBtn = new Fl_Button(topx, topy, topw, toph*2, "Close");
	closeBtn->callback(CloseCB, (void*)this);
	int dlgw = topx + topw + 5;

	topx = 5;
	topy = topy + (toph*2) + 5;
	int topw2 = 60;
	int topx2 = topx + topw + 5;
	lbl = new Fl_Box(topx, topy, topw, toph, "Time");
	tmInp = new Fl_Output(topx2, topy, topw2, toph, 0);
	topx2 += topw2 + 5;
	lbl = new Fl_Box(topx2, topy, dlgw - topx2 - 5, toph, "Time Bar Meter");
	tmBar = new BarMeter(topx2, topy, dlgw - topx2 - 5, toph);

	topy += toph+5;
	lbl = new Fl_Box(topx, topy, topw, toph, "L. Pk");
	topx += topw + 5;
	pkLft = new Fl_Output(topx, topy, topw2, toph, 0);
    topx += 85;
    lpkMtr = new BarMeter(topx, topy, 85, toph);
    lpkMtr->SetRange(1.2);

//	topx += topw2 + 5;
    topx = dlgw / 2;
	lbl = new Fl_Box(topx, topy, topw, toph, "R. Pk");
	topx += topw + 5;
	pkRgt = new Fl_Output(topx, topy, topw2, toph, 0);
    topx += 85;
    rpkMtr = new BarMeter(topx, topy, 85, toph);
    rpkMtr->SetRange(1.2);

	topx = 5;
	topy += toph + 5;
	msgInp = new Fl_Text_Display(topx, topy, dlgw - 10, 300, 0);
	msgInp->buffer(new Fl_Text_Buffer);
	int dlgh = topy + 305;

	end();
	resizable(0);
	resize((Fl::w() - dlgw) / 2, (Fl::h() - dlgh) / 2, dlgw, dlgh);

	dlgLock.Create();
}

GenerateDlg::~GenerateDlg()
{
	prjGenerate = 0;
	dlgLock.Destroy();
}

void GenerateDlg::AddMessage(const char *s)
{
	dlgLock.Enter();
	lastMsg += s;
	lastMsg += '\n';
	dlgLock.Leave();
	Fl::awake((void*)4);
}

void GenerateDlg::UpdateTime(long tm)
{
	dlgLock.Enter();
	if (tm == 0)
		tmBar->SetRange((float)(theProject->seq.GetLength() * 10) / synthParams.sampleRate);
	lastTime = (tm * updateRate) + startTime;
	dlgLock.Leave();
	Fl::awake((void*)3);
}

void GenerateDlg::UpdatePeak(AmpValue lft, AmpValue rgt)
{
	dlgLock.Enter();
	lftPeak = lft;
	rgtPeak = rgt;
	dlgLock.Leave();
	// FLTK seems to drop messages, (at least under Windows)
	// so we only send the message after UpdateTime...
	//Fl::awake((void*)2);
}

void GenerateDlg::Finished()
{
	// The background thread has stopped generating...
	Fl::awake((void*)1);
}

int GenerateDlg::WasCanceled()
{
	return canceled;
}

void GenerateDlg::Run(int autoStart)
{
	set_modal();
	show();
	if (autoStart)
		OnStart(autoStart);
	else
	{
		closed = 0;
		while (!closed)
			Fl::wait();
	}
	Fl::delete_widget(this);
}

void GenerateDlg::OnStart(int autoStart)
{

	canceled = 0;
	Fl_Text_Buffer *msgbuf = msgInp->buffer();
	msgbuf->append("---------- Start ----------\n");
	lastMsg = "";
	if (!autoStart)
	{
		playFrom = GetTimeValue(fromInp);;
		playTo = GetTimeValue(toInp);
		playLive = spkrBtn->value();
	}
    if (someBtn->value())
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
	FormatTime(tmInp, playFrom);
	lftPeak = 0;
	rgtPeak = 0;
	lftMax = 0;
	rgtMax = 0;
	FormatPeak();
	theProject->CallbackRate((float)updateRate / 10.0f);
	theProject->PlayMode(playLive);
	theProject->Start();
//	StartThread();
	stopBtn->activate();
	pauseBtn->activate();
	startBtn->deactivate();
	closeBtn->deactivate();
	while (Fl::wait() > 0)
	{
		void *msg = Fl::thread_message();
		if (msg)
		{
			if (msg == (void*)1)
				break;
			dlgLock.Enter();
			//if (msg == (void*)2)
			//	FormatPeak();
			if (msg == (void*)3)
			{
				FormatTime(tmInp, lastTime);
				tmBar->SetValue(lastTime);
				FormatPeak();
			}
			else if (msg == (void*)4)
			{
				msgbuf->append(lastMsg);
				lastMsg = "";
			}
			dlgLock.Leave();
		}
	}
	theProject->WaitThread();
	char buf[100];
	snprintf(buf, 100, "Peak: [%.6f, %.6f]\n-------- Finished ---------\n", lftMax, rgtMax);
	msgbuf->append(buf);
	lpkMtr->SetValue(lftMax);
	rpkMtr->SetValue(rgtMax);
	stopBtn->deactivate();
	pauseBtn->deactivate();
	startBtn->activate();
	closeBtn->activate();
}

void GenerateDlg::OnStop()
{
	Fl_Text_Buffer *msgbuf = msgInp->buffer();
	msgbuf->append("*Cancel*\nHalting sequencer...\n");
	canceled = 1;
}

void GenerateDlg::OnPause()
{
	if (pauseBtn->value())
		theProject->Pause();
	else
		theProject->Resume();
}

void GenerateDlg::OnClose()
{
//	Fl::delete_widget(this);
	closed = 1;
}

void GenerateDlg::FormatPeak()
{
	char pkText[200];
	snprintf(pkText,80, " %.3f", lftPeak);
	pkLft->value(pkText);

	snprintf(pkText,80, " %.3f", rgtPeak);
	pkRgt->value(pkText);

	if (lftPeak > 1.0 || rgtPeak > 1.0)
	{
		snprintf(pkText, 200, "Out of range (%.3f, %.3f) at %02d:%02d.%1d\n",
			lftPeak, rgtPeak, (lastTime / 600), (lastTime / 10) % 60, lastTime % 10);
		msgInp->buffer()->append(pkText);
	}
	if (lftPeak > lftMax)
		lftMax = lftPeak;
	if (rgtPeak > rgtMax)
		rgtMax = rgtPeak;
	lpkMtr->SetValue(lftPeak);
	rpkMtr->SetValue(rgtPeak);
}

////////////////////////////////////////////////////////////////////////////
/// Virtual Keyboard
////////////////////////////////////////////////////////////////////////////

static void InstrSelCB(Fl_Widget *wdg, void *arg)
{
	KbdGenDlg *dlg = (KbdGenDlg *)wdg->parent();
	if (dlg)
		dlg->OnInstrChange();
}

KbdGenDlg::KbdGenDlg(int X, int Y, int W, int H) : Fl_Group(X, Y, W, H, 0)
{
	form = 0;
	end();
	box(FL_NO_BOX);

	instrList = new Fl_Hold_Browser(X, Y+5, 200, H-15);
	instrList->callback(InstrSelCB);
	add(instrList);

	resizable(NULL);
}

KbdGenDlg::~KbdGenDlg()
{
	delete form;
}

int KbdGenDlg::handle(int e)
{
	int mx = Fl::event_x();
	int my = Fl::event_y();
	if (mx >= instrList->x() && mx <= instrList->x() + instrList->w()
	 && my >= instrList->y() && my <= instrList->y() + instrList->h())
	{
		//printf("Pass to hscroll\n");
		if (instrList->handle(e) && e == FL_PUSH)
			Fl::pushed(instrList);
		return 0;
	}

	if (form && (e == FL_PUSH || e == FL_DRAG || e == FL_RELEASE))
	{
		switch (e)
		{
		case FL_PUSH:
			form->BtnDn(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_DRAG:
			form->MouseMove(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_RELEASE:
			form->BtnUp(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		}
		return 1;
	}
	return 0;
}

void KbdGenDlg::resize(int X, int Y, int W, int H)
{
//	printf("FormEditor: resize(%d,%d,%d,%d)\n", X, Y, W,H);
	int dx = X - x();
	int dy = Y - y();
	Fl_Widget::resize(X, Y, W, H);
	if (form)
		form->MoveTo(X+5, Y+5);
	instrList->resize(instrList->x() + dx, instrList->y() + dy, instrList->w(), instrList->h());
}

void KbdGenDlg::draw()
{
	if (damage() == FL_DAMAGE_CHILD)
	{
		update_child(*instrList);
	}
	else
	{ // total redraw
		int offs[2] = {0,0};
		DrawContext ctx = (DrawContext)&offs[0];
		int part = damage() == FL_DAMAGE_USER1;
		if (!part)
			fl_push_clip(x(), y(), w(), h());
		Fl_Color clr = (Fl_Color) (bgColor << 8);
		fl_color(clr);
		fl_rectf(x(), y(), w(), h());
		fl_draw_box(FL_EMBOSSED_FRAME, x(), y(), w(), h(), clr);
		if (form)
			form->RedrawForm(ctx);
		if (!part)
			fl_pop_clip();
		// now draw all the children atop the background:
		draw_child(*instrList);
	}
}

void KbdGenDlg::Load()
{
	if (form)
		return;

	bsString fileName;
	if (!theProject->FindForm(fileName, "KeyboardEd.xml"))
	{
		prjFrame->Alert("Could not locate keyboard form KeyboardEd.xml!", "Huh?");
		return;
	}

	form = new KeyboardForm();
	form->SetFormEditor(this);
	wdgColor clr;
	if (SynthWidget::colorMap.Find("bg", clr))
		bgColor = clr;
	if (form->Load(fileName, 0, 0) == 0 && form->GetKeyboard())
	{
		// remove the place-holder and insert the list box
		SynthWidget *wdg = form->GetInstrList();
		wdgRect a = wdg->GetArea();
		wdg->Remove();
		delete wdg;

		instrList->resize(x()+a.x+5, y()+a.y+5, a.w, a.h);
		instrList->redraw();
		int sel = instrList->value();
		if (sel > 0)
		{
			InstrConfig *inc = (InstrConfig*)instrList->data(sel);
			if (form)
				form->GetKeyboard()->SetInstrument(inc);
			if (inc)
				theProject->prjMidiIn.SetInstrument(inc->inum);
		}

		int cx = 200;
		int cy = 100;
		form->GetSize(cx, cy);
		cx += 15;
		cy += 15;
		size(cx, cy);
	}
	else
	{
		bsString msg;
		msg = "Could not load keyboard form: ";
		msg += fileName;
		prjFrame->Alert(msg, "Huh?");
		delete form;
		form = 0;
	}
}

void KbdGenDlg::OnInstrChange()
{
	if (form)
	{
		int sel = instrList->value();
		if (sel > 0)
		{
			InstrConfig *inc = (InstrConfig*)instrList->data(sel);
			if (form)
				form->GetKeyboard()->SetInstrument(inc);
			if (inc)
				theProject->prjMidiIn.SetInstrument(inc->inum);
		}
	}
}

int KbdGenDlg::Stop()
{
	if (form)
		return form->Stop();
	return 0;
}

int KbdGenDlg::Start()
{
	if (form)
		return form->Start();
	return 0;
}

void KbdGenDlg::Clear()
{
	instrList->clear();
	if (form)
	{
		form->GetKeyboard()->SetInstrument(0);
		theProject->prjMidiIn.SetInstrument(0);
	}
}

void KbdGenDlg::InitInstrList()
{
	instrList->clear();
	if (!theProject)
		return;

	InstrConfig *ic = 0;
	while ((ic = theProject->mgr.EnumInstr(ic)) != 0)
		AddInstrument(ic);
	if (instrList->size() > 0)
	{
		instrList->select(1);
		InstrConfig *inc = (InstrConfig*)instrList->data(1);
		if (form)
			form->GetKeyboard()->SetInstrument(inc);
		if (inc)
			theProject->prjMidiIn.SetInstrument(inc->inum);
	}
}

void KbdGenDlg::AddInstrument(InstrConfig *ic)
{
	const char *nm = ic->GetName();
	if (*nm != '[')
		instrList->add(nm, (void*)ic);
}

void KbdGenDlg::RemoveInstrument(InstrConfig *ic)
{
	int count = instrList->size();
	for (int ndx = 1; ndx <= count; ndx++)
	{
		if (instrList->data(ndx) == ic)
		{
			instrList->remove(ndx);
			break;
		}
	}
}

void KbdGenDlg::UpdateInstrument(InstrConfig *ic)
{
	int count = instrList->size();
	for (int ndx = 1; ndx <= count; ndx++)
	{
		if (instrList->data(ndx) == ic)
		{
			instrList->text(ndx, ic->GetName());
			break;
		}
	}
}

void KbdGenDlg::UpdateChannels()
{
}


/////////////////////////////////////////////////////////////////////////////
// The virtual keyboard widget. See the Windows version for discussion.
/////////////////////////////////////////////////////////////////////////////

// since fltk uses simple rectangle drawing, bitmaps don't help much.
void KeyboardWidget::CreateBitmaps() { }
void KeyboardWidget::DeleteBitmaps() { }

void KeyboardWidget::CopyToClipboard(bsString& str)
{
	Fl::copy(str, str.Length(), 1);
}

void KeyboardWidget::Paint(DrawContext dc)
{
	//printf("Keyboard Draw %d,%d,%d,%d\n", x(), y(), w(), h());

	if (!rcWhite || !rcBlack)
	{
		fl_color(92,92,64);
		fl_rectf(area.x, area.y, area.w, area.h);
		return;
	}

	wdgRect *rp = rcWhite;
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (fl_not_clipped(rp->x, rp->y, rp->w, rp->h))
		{
			//printf("Keyboard Draw white key %d = %d,%d,%d,%d\n", i, rp->x, rp->y, rp->w, rp->h);
			if (rp == rcLastKey)
				fl_color(Red(kclr[1]), Green(kclr[1]), Blue(kclr[1]));
			else
				fl_color(Red(kclr[0]), Green(kclr[0]), Blue(kclr[0]));
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
			fl_color(8,8,8);
			fl_rect(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}

	rp = rcBlack;
	for (i = 0; i < blkKeys; i++)
	{
		if (fl_not_clipped(rp->x, rp->y, rp->w, rp->h))
		{
			//printf("Keyboard Draw black key %d = %d,%d,%d,%d\n", i, rp->x, rp->y, rp->w, rp->h);
			if (rp == rcLastKey)
				fl_color(Red(kclr[3]), Green(kclr[3]), Blue(kclr[3]));
			else
				fl_color(Red(kclr[2]), Green(kclr[2]), Blue(kclr[2]));
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}
}

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
		damage(FL_DAMAGE_USER1, x(), y(), w(), y());
	}
}

void BarMeter::draw()
{
	fl_color(128,128,128);
	fl_rectf(x(), y(), w(), h());
	fl_color(0,192,0);
	fl_rectf(x()+2, y()+2, (int)((float)w() * barValue / barRange), h()-4);
}
