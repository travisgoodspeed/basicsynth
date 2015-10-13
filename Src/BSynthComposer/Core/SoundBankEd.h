//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef SOUNDBANK_ED_H
#define SOUNDBANK_ED_H

class SoundBankEdit : public SynthEdit
{
private:
	SFPlayerInstr *sb;

	void ListPitches(float bnum, float pnum);

public:
	SoundBankEdit();
	~SoundBankEdit();

	void GetParams();
	void SetInstrument(InstrConfig *ip);
	void ValueChanged(SynthWidget *wdg);
};

class GMPlayerEdit : public SynthEdit
{
private:
	GMPlayer *gm;

public:
	GMPlayerEdit();
	~GMPlayerEdit();

	void GetParams();
	void SetParams();
	void SetInstrument(InstrConfig *ip);
	void ValueChanged(SynthWidget *wdg);
};

#endif
