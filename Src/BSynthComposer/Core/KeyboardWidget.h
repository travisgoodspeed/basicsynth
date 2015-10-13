//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef KEYBOARD_WIDGET_H
#define KEYBOARD_WIDGET_H

#define KEY_DOWN   1
#define KEY_UP     2
#define KEY_CHANGE 3


/// Virtual keyboard widget.
/// This widget displays a piano keyboard and generates events
/// when keys are pressed or released. It also can record
/// key presses and releases. The "time" for the press, however,
/// is set by the current duration value, not actual time. Thus
/// it implements a "step" input recording.
/// 
/// The "changeNew" flag controls how a slide across the keys
/// is processed. When changeNew is true, the old note is stopped
/// and a new note is started. When false, the new note is
/// sent to the currently playing note as a parameter change.
class KeyboardWidget : public SynthWidget
{
private:
	int lastKey;
	int baseKey;
	int lastVel;
	int octs;
	int whtKeys;
	int blkKeys;
	int playing;
	int changeNew;
	wdgRect *rcWhite;
	wdgRect *rcBlack;
	wdgRect *rcLastKey;
	wdgRect upd;
	int knWhite[7];
	int knBlack[5];
	void *bmKey[4]; ///< 0 = white up, 1 = white down, 2 = black up, 3 = black down
	wdgColor kclr[4];

	InstrConfig *activeInstr;
	InstrConfig *selectInstr;
	bsInt32 evtID;
	AmpValue curVol;
	FrqValue curDur;
	int curRhythm;
	int curChnl;

	class RecNote : public SynthList<RecNote>
	{
	public:
		int key;
		int dur;
		int vol;
		RecNote(int k, int d, int v)
		{
			key = k;
			dur = d;
			vol = v;
		}
	};

	int recording;
	int recGroup;
	int useSharps;
	RecNote *recHead;
	RecNote *recTail;

	int FindKey(int x, int y);
	void InvalidateLast();

	void PitchString(int pit, bsString& str);
	void RhythmString(int rhy, bsString& str);
	void NumberString(int val, bsString& str);
	void ClearNotes();
	void CopyToClipboard(bsString& pitches);
	void CreateBitmaps();
	void DeleteBitmaps();

public:
	KeyboardWidget();
	~KeyboardWidget();
	void SetOctaves(int n);
	void Setup();
	virtual void SetArea(wdgRect& r);
	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Tracking() { return playing; }
	virtual void Paint(DrawContext dc);
	virtual int Load(XmlSynthElem *elem);

	void SetInstrument(InstrConfig *ip)
	{
		selectInstr = ip;
	}

	void SetChannel(int n)
	{
		curChnl = n;
	}

	int GetChannel()
	{
		return curChnl;
	}

	void SetDuration(float d)
	{
		curRhythm = (int) d;
		curDur = 2.0f / d;
	}

	void SetVolume(float v)
	{
		curVol = v;
	}

	float GetVolume()
	{
		return curVol;
	}

	void SetRecord(int on)
	{
		recording = on;
	}

	void SetRecGroup(int on)
	{
		recGroup = on;
	}

	void SetRecSharps(int on)
	{
		useSharps = on;
	}

	void CopyNotes();

	void PlayNote(int key, int e, int vel);
};

#endif
