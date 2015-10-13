//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef TONESYNTH_ED_H
#define TONESYNTH_ED_H

class ToneSynthEdit : public SynthEdit
{
public:
	ToneSynthEdit();
	~ToneSynthEdit();

	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

class ToneFMSynthEdit : public ToneSynthEdit
{
public:
	ToneFMSynthEdit();
};

#endif
