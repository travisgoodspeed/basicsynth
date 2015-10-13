//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef BUZZSYNTH_EDIT_H
#define BUZZSYNTH_EDIT_H

class BuzzSynthEdit : public SynthEdit
{
private:

public:
	BuzzSynthEdit();
	~BuzzSynthEdit();

	void LoadValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
