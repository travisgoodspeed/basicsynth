#include "globinc.h"
#if POSIX
#include <pthread.h>
#endif
#include "MainFrm.h"

long GenerateDlg::playFrom;
long GenerateDlg::playTo;
long GenerateDlg::playLive = 0;


#if _WIN32
static CRITICAL_SECTION genDlgGuard;
HANDLE genThreadH;
DWORD  genThreadID;

static void GenDlgLock()
{
	EnterCriticalSection(&genDlgGuard);
}
static void GenDlgUnlock()
{
	LeaveCriticalSection(&genDlgGuard);
}

static DWORD WINAPI GenerateProc(void* param)
{
	try
	{
		theProject->Generate(!GenerateDlg::playLive, GenerateDlg::playFrom, GenerateDlg::playTo);
	}
	catch(...)
	{
	}
	prjGenerate->Finished();
	ExitThread(0);
	return 0;
}

int GenerateDlg::StartThread()
{
	InitializeCriticalSection(&genDlgGuard);
	genThreadH = CreateThread(NULL, 0, GenerateProc, NULL, CREATE_SUSPENDED, &genThreadID);
	if (genThreadH == INVALID_HANDLE_VALUE)
		return -1;
	ResumeThread(genThreadH);
	return 0;
}

void GenerateDlg::EndThread()
{
	WaitForSingleObject(genThreadH, 10000);
	DeleteCriticalSection(&genDlgGuard);
}

#endif

#if POSIX
static pthread_mutex_t genDlgGuard;
static pthread_t genThreadID;

static void GenDlgLock()
{
	pthread_mutex_lock(&genDlgGuard);
}

static void GenDlgUnlock()
{
	pthread_mutex_unlock(&genDlgGuard);
}

static void *GenerateProc(void *param)
{
	// synchronize with the main thread...
	GenDlgLock();
	GenDlgUnlock();
	try
	{
		theProject->Generate(!GenerateDlg::playLive, GenerateDlg::playFrom, GenerateDlg::playTo);
	}
	catch (...)
	{
	}
	prjGenerate->Finished();
	pthread_exit((void*)0);
	return 0;
}

int GenerateDlg::StartThread()
{
	pthread_mutex_init(&genDlgGuard, NULL);
	GenDlgLock();
	int err = pthread_create(&genThreadID, NULL, GenerateProc, 0);
	GenDlgUnlock();
	return err;
}

void GenerateDlg::EndThread()
{
	pthread_join(genThreadID, NULL); 
	pthread_mutex_destroy(&genDlgGuard);
}
#endif

static void FormatTime(Fl_Input *wdg, long secs)
{
	char txt[40];
	snprintf(txt, 40, "%02d:%02d", secs / 60, secs % 60);
	wdg->value(txt);
}

static long GetTimeValue(Fl_Input *wdg)
{
	char buf[80];
	strncpy(buf, wdg->value(), 80);
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

GenerateDlg::GenerateDlg(KbdGenDlg *d) : Fl_Window(100, 100, 600, 320, "Generate")
{
	kbdDlg = d;
	prjGenerate = static_cast<GenerateWindow*>(this);

	lastTime = 0;
	canceled = 0;

	Fl_Box *lbl;
	Fl_Group *grp;

	lbl = new Fl_Box(5, 5, 80, 30, "Out @->");
	lbl->labelcolor(0x00400000);
	grp = new Fl_Group(85, 5, 270, 30, 0);
	spkrBtn = new Fl_Light_Button(90, 5, 80, 30, "Live");
	spkrBtn->type(FL_RADIO_BUTTON);
	spkrBtn->value(playLive);
	diskBtn = new Fl_Light_Button(180, 5, 80, 30, "Disk");
	diskBtn->type(FL_RADIO_BUTTON);
	diskBtn->value(!playLive);
	grp->end();

	grp = new Fl_Group(0, 40, 270, 35, 0);
	lbl = new Fl_Box(5, 40, 80, 30, "Play");
	allBtn = new Fl_Light_Button(90, 40, 80, 30, "All");
	allBtn->type(FL_RADIO_BUTTON);
	allBtn->value(1);
	someBtn = new Fl_Light_Button(180, 40, 80, 30, "Part");
	someBtn->type(FL_RADIO_BUTTON);
	someBtn->value(0);
	grp->end();

	lbl = new Fl_Box(5, 75, 80, 30, "From");
	fromInp = new Fl_Input(90, 75, 170, 30, 0);
	lbl = new Fl_Box(5, 110, 80, 30, "To");
	toInp = new Fl_Input(90, 110, 170, 30, 0);

	startBtn = new Fl_Button(90, 145, 80, 30, "Start"); 
	startBtn->callback(StartCB, (void*)this);
	startBtn->activate();

	stopBtn = new Fl_Button(180, 145, 80, 30, "Stop"); 
	stopBtn->callback(StopCB, (void*)this);
	stopBtn->deactivate();

	lbl = new Fl_Box(5, 180, 80, 30, "Time");
	tmInp = new Fl_Output(90, 180, 170, 30, 0);
	lbl = new Fl_Box(5, 215, 80, 30, "Peak Left");
	pkLft = new Fl_Output(90, 215, 170, 30, 0);
	lbl = new Fl_Box(5, 250, 80, 30, "Peak Right");
	pkRgt = new Fl_Output(90, 250, 170, 30, 0);

	closeBtn = new Fl_Button(90, 285, 170, 30, "Close");
	closeBtn->callback(CloseCB, (void*)this);

	msgInp = new Fl_Text_Display(280, 5, 310, 310, 0);
	msgInp->buffer(new Fl_Text_Buffer);

	end();
	resizable(0);
	resize((Fl::w() - w()) / 2, (Fl::h() - h()) / 2, w(), h());
	set_modal();
}

GenerateDlg::~GenerateDlg()
{
	prjGenerate = 0;
	//if (mainWnd)
	//	mainWnd->make_current();
}

void GenerateDlg::AddMessage(const char *s)
{
	GenDlgLock();
	lastMsg += s;
	lastMsg += '\n';
	GenDlgUnlock();
	Fl::awake((void*)4);
}

void GenerateDlg::UpdateTime(long tm)
{
	GenDlgLock();
	lastTime = tm;
	GenDlgUnlock();
	Fl::awake((void*)3);
}

void GenerateDlg::UpdatePeak(AmpValue lft, AmpValue rgt)
{
	GenDlgLock();
	lftPeak = lft;
	rgtPeak = rgt;
	GenDlgUnlock();
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

void GenerateDlg::OnStart(int autoStart)
{
	canceled = 0;
	Fl_Text_Buffer *msgbuf = msgInp->buffer();
	msgbuf->append("---------- Start ----------\n");
	lastMsg = "";
	if (!autoStart)
	{
		if (someBtn->value())
		{
			playFrom = GetTimeValue(fromInp);;
			playTo = GetTimeValue(toInp);
		}
		else
		{
			playFrom = 0;
			playTo = 0;
		}
		playLive = spkrBtn->value();
	}
	lastTime = 0;
	FormatTime(tmInp, playFrom);
	lftPeak = 0;
	rgtPeak = 0;
	FormatPeak();
	StartThread();
	stopBtn->activate();
	startBtn->deactivate();
	closeBtn->deactivate();
	while (Fl::wait() > 0)
	{
		void *msg = Fl::thread_message();
		if (msg)
		{
			if (msg == (void*)1)
				break;
			GenDlgLock();
			//if (msg == (void*)2)
			//	FormatPeak();
			if (msg == (void*)3)
			{
				FormatTime(tmInp, lastTime);
				FormatPeak();
			}
			else if (msg == (void*)4)
			{
				msgbuf->append(lastMsg);
				lastMsg = "";
			}
			GenDlgUnlock();
		}
	}
	EndThread();
	char buf[100];
	snprintf(buf, 100, "Peak: [%f, %f]\n-------- Finished ---------\n", lftMax, rgtMax);
	msgbuf->append(buf);
	stopBtn->deactivate();
	startBtn->activate();
	closeBtn->activate();
}

void GenerateDlg::OnStop()
{
	canceled = 1;
}

void GenerateDlg::OnClose()
{
	Fl::delete_widget(this);
}

void GenerateDlg::FormatPeak()
{
	char pkText[200];
	snprintf(pkText,80, "%.6f", lftPeak);
	pkLft->value(pkText);

	snprintf(pkText,80, "%.6f", rgtPeak);
	pkRgt->value(pkText);

	if (lftPeak > 1.0 || rgtPeak > 1.0)
	{
		snprintf(pkText, 200, "Out of range (%f, %f) at %02d:%02d\n", 
			lftPeak, rgtPeak, lastTime / 60, lastTime % 60);
		msgInp->buffer()->append(pkText);
	}
}

////////////////////////////////////////////////////////////////////////////
/// Virtual Keyboard
////////////////////////////////////////////////////////////////////////////

static void GenCB(Fl_Widget *wdg, void *arg)
{
	KbdGenDlg *dlg = (KbdGenDlg *)arg;
	if (dlg->GetPlayAll())
	{
		GenerateDlg::playFrom = dlg->GetFrom();
		GenerateDlg::playTo = dlg->GetTo();
	}
	else
	{
		GenerateDlg::playFrom = 0;
		GenerateDlg::playTo = 0;
	}
	GenerateDlg::playLive = dlg->GetPlayLive();
	mainWnd->Generate(1, GenerateDlg::playLive);
}

static void InstrSelCB(Fl_Widget *wdg, void *arg)
{
	KbdGenDlg *dlg = (KbdGenDlg *)wdg->parent();
	if (dlg)
		dlg->OnInstrChange();
}

static void KbdEventCB(Fl_Widget *wdg, void *arg)
{
	KbdGenDlg *dlg = (KbdGenDlg *)wdg->parent();
	if (dlg)
		dlg->OnKbdEvent((KbdMsg *)arg);
}

KbdGenDlg::KbdGenDlg(int X, int Y, int W, int H) : Fl_Group(X, Y, W, H, 0)
{
	curVol = 1.0;
	curChnl = 0;
	kbdRunning = 0;
	activeInstr = 0;

	end();
	box(FL_UP_BOX);
	Fl_Box *lbl;
	Fl_Group *grp;
	int xpos;
	int ypos;
	int btnWidth = 30;

	instrList = new Fl_Hold_Browser(X, Y+5, 200, H-15);
	instrList->callback(InstrSelCB);
	add(instrList);

	ypos = Y+5;
	xpos = X+210;

	grp = new Fl_Group(xpos, ypos, 2*btnWidth, 30, 0);
	add(grp);
	grp->box(FL_NO_BOX);
	startPlay = new Fl_Button(xpos, ypos, btnWidth, 30, "@|>");
	startPlay->type(FL_TOGGLE_BUTTON);
	startPlay->labelcolor(0x20ff2000);
	xpos += startPlay->w();

	stopPlay = new Fl_Button(xpos, ypos, btnWidth, 30, "@||");
	stopPlay->type(FL_TOGGLE_BUTTON);
	stopPlay->labelcolor(0xa0000000);
	xpos += stopPlay->w() + 50;
	grp->end();

	volLvl = new Fl_Value_Slider(xpos, ypos, 200, 30, "Vol.");
	volLvl->align(FL_ALIGN_LEFT);
	volLvl->type(FL_HOR_NICE_SLIDER);
	volLvl->minimum(0.0);
	volLvl->maximum(1.0);
	volLvl->precision(2);
	volLvl->value(1.0);
	add(volLvl);
	xpos += volLvl->w() + 50;

	chnlList = new Fl_Choice(xpos, ypos, 50, 30, "Chnl.");
	chnlList->align(FL_ALIGN_LEFT);
	add(chnlList);
	xpos += chnlList->w() + btnWidth/2;

	grp = new Fl_Group(xpos, ypos, 5*btnWidth, 30, 0);
	add(grp);
	grp->box(FL_NO_BOX);

	wholeNote = new Fl_Button(xpos, ypos, btnWidth, 30, "W");
	wholeNote->type(FL_RADIO_BUTTON);
	wholeNote->box(FL_UP_BOX);
	xpos += wholeNote->w();

	halfNote = new Fl_Button(xpos, ypos, btnWidth, 30, "H");
	halfNote->type(FL_RADIO_BUTTON);
	halfNote->box(FL_UP_BOX);
	xpos += halfNote->w();

	quartNote = new Fl_Button(xpos, ypos, btnWidth, 30, "Q");
	quartNote->type(FL_RADIO_BUTTON);
	quartNote->box(FL_UP_BOX);
	xpos += quartNote->w();

	eightNote = new Fl_Button(xpos, ypos, btnWidth, 30, "E");
	eightNote->type(FL_RADIO_BUTTON);
	eightNote->box(FL_UP_BOX);
	xpos += eightNote->w();

	sixtnNote = new Fl_Button(xpos, ypos, btnWidth, 30, "S");
	sixtnNote->type(FL_RADIO_BUTTON);;
	sixtnNote->box(FL_UP_BOX);
	xpos += sixtnNote->w();
	xpos += btnWidth/2;
	grp->end();

	recNotes = new Fl_Button(xpos, ypos, btnWidth, 30, "@square");
	recNotes->labelcolor(0xaa000000);
	recNotes->type(FL_TOGGLE_BUTTON);
	recNotes->box(FL_UP_BOX);
	add(recNotes);
	xpos += recNotes->w();

	grpNotes = new Fl_Button(xpos, ypos, btnWidth, 30, "{ }");
	grpNotes->type(FL_TOGGLE_BUTTON);
	grpNotes->box(FL_UP_BOX);
	add(grpNotes);
	xpos += grpNotes->w();

	cpyNotes = new Fl_Button(xpos, ypos, btnWidth, 30, "Cpy");
	cpyNotes->type(FL_NORMAL_BUTTON);
	cpyNotes->box(FL_UP_BOX);
	add(cpyNotes);
	xpos += cpyNotes->w();

	xpos = X+210;
	kbd = new KbdWdgFltk(xpos, Y+40, 688, H-50);
	kbd->SetOctaves(7);
	kbd->callback(KbdEventCB);
	add(kbd);
	xpos += kbd->w() + 5;

	lbl = new Fl_Box(xpos, Y+5, 4, H-10, "");
	add(lbl);
	lbl->box(FL_DOWN_BOX);
	xpos += 10;

	/*************************
	grp = new Fl_Group(xpos, Y, 270, 35);
	add(grp);
	gen = new Fl_Button(xpos+5, Y+5, 80, 30, "Out@->");
	gen->labelcolor(0x00400000);
	gen->type(FL_NORMAL_BUTTON);
	gen->callback(GenCB, (void*)this);
	spkr = new Fl_Light_Button(xpos+90, Y+5, 80, 30, "Live");
	spkr->type(FL_RADIO_BUTTON);
	spkr->value(0);
	disk = new Fl_Light_Button(xpos+180, Y+5, 80, 30, "Disk");
	disk->type(FL_RADIO_BUTTON);
	disk->value(1);
	grp->end();

	grp = new Fl_Group(xpos, Y+40, 270, 35);
	add(grp);
	lbl = new Fl_Box(xpos+5, Y+40, 80, 30, "Play");
	all = new Fl_Light_Button(xpos+90, Y+40, 80, 30, "All");
	all->type(FL_RADIO_BUTTON);
	all->value(1);
	some = new Fl_Light_Button(xpos+180, Y+40, 80, 30, "Part");
	some->type(FL_RADIO_BUTTON);
	some->value(0);
	grp->end();

	from = new Fl_Input(xpos+90, Y+75, 170, 20, "From");
	add(from);
	to = new Fl_Input(xpos+90, Y+100, 170, 20, "To");
	add(to);
	lbl = new Fl_Box(xpos+270, Y+5, 4, H-10, "");
	add(lbl);
	lbl->box(FL_DOWN_BOX);
	************************************/

	resizable(NULL);
//	InitInstrList();
//	UpdateChannels();
}

void KbdGenDlg::OnKbdEvent(KbdMsg *msg)
{
	printf("Keyboard event %d, key %d, instrument %s\n", msg->evt, msg->key, activeInstr ? activeInstr->GetName() : "<none>");
}

void KbdGenDlg::OnInstrChange()
{
	int sel = instrList->value();
	if (sel > 0)
		activeInstr = (InstrConfig*)instrList->data(sel);
	else
		activeInstr = NULL;
}

int KbdGenDlg::GetPlayLive()
{
	return spkr->value();
}

int KbdGenDlg::GetPlayAll()
{
	return all->value();
}

long KbdGenDlg::GetFrom()
{
	return GetTimeValue(from);
}

long KbdGenDlg::GetTo()
{
	return GetTimeValue(to);
}

int KbdGenDlg::Stop()
{
	return 0;
}

int KbdGenDlg::Start()
{
	return 0;
}

void KbdGenDlg::Clear()
{
	instrList->clear();
	activeInstr = 0;
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
		activeInstr = (InstrConfig*)instrList->data(1);
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
	chnlList->clear();
	if (theProject == NULL)
		return;

	int chnls = theProject->mixInfo->GetMixerInputs();
	int n;
	for (n = 0; n < chnls; n++)
	{
		char fnbuf[20];
		sprintf(fnbuf, "%d", n);
		chnlList->add(fnbuf);
	}
	if (curChnl >= chnls)
		curChnl = 0;
	chnlList->value(curChnl);
}

void KbdGenDlg::StartRecord()
{
}

void KbdGenDlg::StopRecord()
{
}

void KbdGenDlg::CopyNotes()
{
}


/////////////////////////////////////////////////////////////////////////////
// The virtual keyboard widget. See the Windows version for discussion.
/////////////////////////////////////////////////////////////////////////////

KbdWdgFltk::KbdWdgFltk(int X, int Y, int W, int H)
	: Fl_Widget(X, Y, W, H, 0)
{
	playing = 0;
	lastKey = -1;
	octs = 1;
	whtKeys = 7;
	blkKeys = 5;
	rcWhite = 0;
	rcBlack = 0;
	rcLastKey = 0;
}

KbdWdgFltk::~KbdWdgFltk()
{
	delete[] rcWhite;
	delete[] rcBlack;
}

void KbdWdgFltk::SetOctaves(int n)
{ 
	if (n < 1)
		n = 1;
	octs = n;
	whtKeys = n * 7;
	blkKeys = n * 5;
	CreateKeyboard();
}

void KbdWdgFltk::CreateKeyboard()
{
	delete[] rcWhite;
	delete[] rcBlack;

	rcWhite = new wdgRect[whtKeys];
	rcBlack = new wdgRect[blkKeys];

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
	int widWhite = w() / whtKeys;
	int hiWhite = h();
	if (hiWhite > (6*widWhite))
		hiWhite = 6*widWhite;
	else if (hiWhite < (5*widWhite))
		widWhite = hiWhite / 5;
	int hiBlack = (hiWhite * 2) / 3;
	int widBlack = (widWhite * 7) / 10;
	int space = (w() - (widWhite * whtKeys)) / 2;

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
			rcWhite[ndxw].x = x() + (int) xWhite;
			rcWhite[ndxw].y = y();
			rcWhite[ndxw].w = (int) widWhite;
			rcWhite[ndxw].h = hiWhite;
			xWhite += widWhite;
			ndxw++;
		}

		ndx = on * 5;
		for (i = 0; i < 2; i++)
		{
			rcBlack[ndxb].x = x() + (int) xBlack;
			rcBlack[ndxb].y = y();
			rcBlack[ndxb].w = (int) widBlack;
			rcBlack[ndxb].h = hiBlack;
			xBlack += widWhite;
			ndxb++;
		}

		xBlack += widWhite;
		for ( i = 0; i < 3; i++)
		{
			rcBlack[ndxb].x = x() + (int) xBlack;
			rcBlack[ndxb].y = y();
			rcBlack[ndxb].w = (int) widBlack;
			rcBlack[ndxb].h = hiBlack;
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
}

void KbdWdgFltk::resize(int X, int Y, int W, int H)
{
	int dx = X - x();
	int dy = Y - y();
	Fl_Widget::resize(X, Y, W, H);
	wdgRect *rp;
	int i;
	if (rcWhite)
	{
		rp = rcWhite;
		for (i = 0; i < whtKeys; i++)
		{
			rp->x += dx;
			rp->y += dy;
			rp++;
		}
	}
	if (rcBlack)
	{
		rp = rcBlack;
		for (i = 0; i < blkKeys; i++)
		{
			rp->x += dx;
			rp->y += dy;
			rp++;
		}
	}
}

void KbdWdgFltk::draw()
{
	//printf("Keyboard Draw background %d,%d,%d,%d\n", x(), y(), w(), h());

	fl_color(92,92,64);
	fl_rectf(x(), y(), w(), h());
	if (!rcWhite || !rcBlack)
		return;


	wdgRect *rp = rcWhite;
	int i;
	for (i = 0; i < whtKeys; i++)
	{
		if (fl_not_clipped(rp->x, rp->y, rp->w, rp->h))
		{
			//printf("Keyboard Draw white key %d = %d,%d,%d,%d\n", i, rp->x, rp->y, rp->w, rp->h);
			if (rp == rcLastKey)
				fl_color(128, 128, 128);
			else
				fl_color(240, 240, 230);
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
			fl_color(8,8,8);
			fl_line(rp->x, rp->y, rp->x, rp->y+rp->h-1);
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
				fl_color(64, 64, 64);
			else
				fl_color(8, 8, 8);
			fl_rectf(rp->x, rp->y, rp->w, rp->h);
		}
		rp++;
	}

	fl_color(0,0,0);
	int rw = (rcWhite[whtKeys-1].x + rcWhite[0].w) - rcWhite[0].x;
	fl_rect(rcWhite[0].x, rcWhite[0].y, rw, rcWhite[0].h);
}

int KbdWdgFltk::handle(int e)
{
	int mx = Fl::event_x();
	int my = Fl::event_y();
	switch (e)
	{
	case FL_PUSH:
		OnBtnDown(mx, my);
		return 1;
	case FL_DRAG:
		OnMouseMove(mx, my);
		return 1;
	case FL_RELEASE:
		OnBtnUp(mx, my);
		return 1;
	}
	return 0;
}

void KbdWdgFltk::OnBtnDown(int mx, int my)
{
	playing = 1;
	lastKey = -1;
	rcLastKey = 0;
	FindKey(mx, my);
	KbdMsg m;
	m.evt = VKBD_START;
	m.key = lastKey;
	do_callback(this, (void*)&m);
}

void KbdWdgFltk::OnBtnUp(int mx, int my)
{
	playing = 0;
	KbdMsg m;
	m.evt = VKDB_STOP;
	m.key = lastKey;
	do_callback(this, (void*)&m);
	InvalidateLast();
	rcLastKey = 0;
}

void KbdWdgFltk::OnMouseMove(int mx, int my)
{
	if (playing)
	{
		if (FindKey(mx, my))
		{
			KbdMsg m;
			m.evt = VKDB_CHANGE;
			m.key = lastKey;
			do_callback(this, (void*)&m);
		}
	}
}

int KbdWdgFltk::FindKey(int mx, int my)
{
	//printf("Findkey %d, %d\n", mx, my);

	int ndx;
	int kdown = -1;
	wdgRect *rcNewKey = 0;
	for (ndx = 0; ndx < blkKeys; ndx++)
	{
		if (rcBlack[ndx].Inside(mx, my))
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
			if (rcWhite[ndx].Inside(mx, my))
			{
				kdown = knWhite[ndx % 7] + ((ndx / 7) * 12);
				rcNewKey = &rcWhite[ndx];
				break;
			}
		}
	}
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

void KbdWdgFltk::InvalidateLast()
{
	if (rcLastKey)
		damage(FL_DAMAGE_CHILD, rcLastKey->x, rcLastKey->y, rcLastKey->w, rcLastKey->h);
}

/////////////////////////////////////////////////////////////////////////////
// Platform-specific project functions
/////////////////////////////////////////////////////////////////////////////

int SynthProject::Generate(int todisk, long from, long to)
{
	if (!todisk)
	{
		prjGenerate->AddMessage("Live playback is not supported.\n");
		return -1;
	}
	if (!wvoutInfo)
	{
		prjGenerate->AddMessage("Wave output info is NULL\n");
		return -1;
	}
	return GenerateToFile(from, to);
}

int SynthProject::Play()
{
	prjFrame->Alert("Live playback is not supported at this time.", "Sorry.");
	return 0;
}

int SynthProject::Stop()
{
	return 0;
}

int SynthProject::PlayEvent(SeqEvent *evt)
{
	delete evt;
	return 0;
}
