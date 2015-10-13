//////////////////////////////////////////////////////////////////////
// BasicSynth - Widget that displays a piano keyboard.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include <MIDIDefs.h>

KeyboardWidget::KeyboardWidget()
{
	playing = 0;
	lastKey = -1;
	lastVel = 127;
	baseKey = 12;
	octs = 0;
	whtKeys = 7;
	blkKeys = 5;
	rcWhite = 0;
	rcBlack = 0;
	rcLastKey = 0;
	bmKey[0] = 0;
	bmKey[1] = 0;
	bmKey[2] = 0;
	bmKey[3] = 0;
	kclr[0] = ClrRGBA(255,255,240);
	kclr[1] = ClrRGBA(200,200,192);
	kclr[2] = ClrRGBA(8,8,8);
	kclr[3] = ClrRGBA(40,40,40);
	evtID = 1;
	activeInstr = 0;
	selectInstr = 0;
	curVol = 1.0;
	curChnl = 0;
	curDur = 0.5;
	curRhythm = 4;
	recording = 0;
	recGroup = 0;
	useSharps = 0;
	changeNew = true;
	recHead = new RecNote(0,0,0);
	recTail = new RecNote(0,0,0);
	recHead->Insert(recTail);
}

KeyboardWidget::~KeyboardWidget()
{
	DeleteBitmaps();
	delete[] rcWhite;
	delete[] rcBlack;
	ClearNotes();
	delete recTail;
	delete recHead;
}

int KeyboardWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);

	short sval;
	if (elem->GetAttribute("octs", sval) == 0)
		SetOctaves(sval);
	return 0;
}

void KeyboardWidget::SetOctaves(int n)
{
	if (n < 1)
		n = 1;
	octs = n;
	whtKeys = n * 7;
	blkKeys = n * 5;
	Setup();
}

void KeyboardWidget::SetArea(wdgRect& a)
{
	area = a;
	if (octs > 0)
		Setup();
}

void KeyboardWidget::Setup()
{
	delete[] rcWhite;
	delete[] rcBlack;
	DeleteBitmaps();

	colorMap.Find("kwup", kclr[0]);
	colorMap.Find("kwdn", kclr[1]);
	colorMap.Find("kbup", kclr[2]);
	colorMap.Find("kbdn", kclr[3]);

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
	int widWhite = area.w / whtKeys;
	int hiWhite = area.h;
	if (hiWhite > (6*widWhite))
		hiWhite = 6*widWhite;
	else if (hiWhite < (5*widWhite))
		widWhite = hiWhite / 5;
	int hiBlack = (hiWhite * 2) / 3;
	int widBlack = (widWhite * 7) / 10;
	int space = (area.w - (widWhite * whtKeys)) / 2;

	// Pre-calculate the rectangles of the keys. White keys are
	// equally spaced. Black keys repeat in the 2, 3 pattern with
	// a gap inbetween the groups equal to the width of a white key.
	int xWhite = space;
	int xBlack = space + widWhite - (widBlack / 2);
	int i, on;
	int ndxw = 0;
	int ndxb = 0;
	for (on = 0; on < octs; on++)
	{
		for (i = 0; i < 7; i++)
		{
			rcWhite[ndxw].x = (int) xWhite + area.x;
			rcWhite[ndxw].y = area.y;
			rcWhite[ndxw].w = (int) widWhite;
			rcWhite[ndxw].h = hiWhite;
			xWhite += widWhite;
			ndxw++;
		}

		for (i = 0; i < 2; i++)
		{
			rcBlack[ndxb].x = (int) xBlack + area.x;
			rcBlack[ndxb].y = area.y;
			rcBlack[ndxb].w = (int) widBlack;
			rcBlack[ndxb].h = hiBlack;
			xBlack += widWhite;
			ndxb++;
		}

		xBlack += widWhite;
		for ( i = 0; i < 3; i++)
		{
			rcBlack[ndxb].x = (int) xBlack + area.x;
			rcBlack[ndxb].y = area.y;
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
	CreateBitmaps();
}

int KeyboardWidget::BtnDown(int x, int y, int ctrl, int shift)
{
	playing = 1;
	FindKey(x, y);
	form->Redraw(this);
	PlayNote(lastKey+baseKey, KEY_DOWN, lastVel);
	return 1;
}

int KeyboardWidget::BtnUp(int x, int y, int ctrl, int shift)
{
	playing = 0;
	PlayNote(lastKey+baseKey, KEY_UP, lastVel);
	if (rcLastKey)
	{
		InvalidateLast();
		rcLastKey = 0;
		lastKey = -1;
		form->Redraw(this);
	}
	return 1;
}

int KeyboardWidget::MouseMove(int x, int y, int ctrl, int shift)
{
	if (playing)
	{
		if (FindKey(x, y))
		{
			PlayNote(lastKey+baseKey, KEY_CHANGE, lastVel);
			form->Redraw(this);
		}
		return 1;
	}
	return 0;
}

int KeyboardWidget::FindKey(int x, int y)
{
	int ndx;
	int kdown = -1;
	wdgRect *rcNewKey = 0;
	for (ndx = 0; ndx < blkKeys; ndx++)
	{
		if (rcBlack[ndx].Inside(x, y))
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
			if (rcWhite[ndx].Inside(x, y))
			{
				kdown = knWhite[ndx % 7] + ((ndx / 7) * 12);
				rcNewKey = &rcWhite[ndx];
				break;
			}
		}
	}
	if (kdown != -1 && kdown != lastKey)
	{
		if (rcNewKey)
		{
			float range = (float) (rcNewKey->GetBottom() - rcNewKey->GetTop());
			float kpos = y - rcNewKey->GetTop();
			lastVel = (int) (127.0 * kpos / range);
		}
		InvalidateLast();
		lastKey = kdown;
		rcLastKey = rcNewKey;
		InvalidateLast();
		return 1;
	}
	return 0;
}

void KeyboardWidget::InvalidateLast()
{
	if (rcLastKey)
		upd.Combine(*rcLastKey);
}

void KeyboardWidget::PlayNote(int key, int e, int vel)
{
	if (selectInstr == NULL || theProject == NULL)
		return;

	if (e == KEY_CHANGE && changeNew)
	{
		// stop last note
		PlayNote(key, KEY_UP, vel);
		e = KEY_DOWN;
	}

	// activeInstr keeps track of which instrument started the note.
	// We need to keep playing on the same instrument until a Key up happens,
	// even if the selected instrument changes.
	if (!activeInstr)
		activeInstr = selectInstr;

	NoteEvent *evt = (NoteEvent*) theProject->mgr.ManufEvent(activeInstr);
	evt->SetChannel(curChnl);
	evt->SetStart(0);
	evt->SetDuration((bsInt32) (synthParams.sampleRate * curDur));
	evt->SetVolume(curVol);
	evt->SetPitch(key);
	evt->SetVelocity(vel);

	switch (e)
	{
	case KEY_CHANGE:
		// change current note
		evt->SetType(SEQEVT_PARAM);
		break;
	case KEY_DOWN:
		evtID = (evtID + 1) & 0x7FFFFFFF;
		evt->SetType(SEQEVT_START);
		break;
	case KEY_UP:
		evt->SetType(SEQEVT_STOP);
		activeInstr = 0;
		break;
	default:
		//OutputDebugString("Kbd event is unknown...\r\n");
		evt->Destroy();
		return;
	}
	evt->SetID(evtID);

	theProject->PlayEvent(evt);
	// NB: player will delete event. Don't touch it after calling PlayEvent!

	if (recording && e == KEY_DOWN)
	{
		RecNote *note = new RecNote(key, curRhythm, (int)(curVol*100.0));
		recTail->InsertBefore(note);
	}
}

void KeyboardWidget::ClearNotes()
{
	RecNote *note = recHead->next;
	while (note != recTail)
	{
		note->Remove();
		delete note;
	}
}

void KeyboardWidget::PitchString(int pit, bsString& str)
{
	static const char *pitchLtrsS[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	static const char *pitchLtrsF[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
	if (useSharps)
		str += pitchLtrsS[pit%12];
	else
		str += pitchLtrsF[pit%12];
	NumberString(pit / 12, str);
}

void KeyboardWidget::RhythmString(int rhy, bsString& str)
{
	str += '%';
	NumberString(rhy, str);
}

void KeyboardWidget::NumberString(int num, bsString& str)
{
	//char buf[20];
	//buf[0] = 0;
	//snprintf(buf, 20, "%d", num);
	//str += buf;
	str += (long)num;
}

void KeyboardWidget::CopyNotes()
{
	const char *comma = ", ";
	const char *semi  = ";\r\n";
//	const char *obrack = "{";
	const char *cbrack = "}";

	RecNote *note;
	bsString pitches;
	if (recGroup)
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
	CopyToClipboard(pitches);
}

