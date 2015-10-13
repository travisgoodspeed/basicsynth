//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WFSYNTH_ED_H
#define WFSYNTH_ED_H

class WFSynthEdit : public SynthEdit
{
protected:

public:
	WFSynthEdit();
	~WFSynthEdit();

	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};
#endif
