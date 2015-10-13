//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WAVETABLE_EDIT_H
#define WAVETABLE_EDIT_H

#define WT_MAX_PARTIAL 16

class WavetableEdit : public WidgetForm
{
private:
	WavetableItem *wi;
	WaveWidget *wav;
	int changed;
	long blkColor;
	long txtColor;
	long hiColor;
	long loColor;

	struct SavePart
	{
		int on;
		float amp;
		float phs;
	} savebuf[WT_MAX_PARTIAL];
	short saveGibbs;
	short saveSum;

	void SetPart(int mul, float amp, float phs, int on = 1);
	void LoadValues();

public:
	WavetableEdit(WavetableItem *p)
	{
		wav = 0;
		wi = p;
		changed = 0;
		bgColor = 0xff808080;
		fgColor = 0xff202020;
		txtColor = 0xfff0f0a0;
		hiColor = 0xfff0f0f0;
		loColor = 0xff808000;
		saveGibbs = 0;
		saveSum = 1;
		memset(&savebuf, 0, sizeof(savebuf));
	}

	int IsChanged() { return changed; }

	virtual void Cancel();
	virtual void GetParams();
	virtual void SetParams();
	virtual void ValueChanged(SynthWidget *wdg);
};
#endif
