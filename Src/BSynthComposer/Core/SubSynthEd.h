//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef SUBSYNTH_EDIT_H
#define SUBSYNTH_EDIT_H

class SubSynthEdit : public SynthEdit
{
private:
	int oldft;
	void SetResonRange(int ft, int draw);

public:
	SubSynthEdit();
	~SubSynthEdit();

	void LoadValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
