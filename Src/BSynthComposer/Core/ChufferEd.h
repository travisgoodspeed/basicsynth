//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _CHUFFER_EDIT_H
#define _CHUFFER_EDIT_H

/// Editor for Chuffer instrument.
/// The editor sets the parameters for the noise source, AR envelope,
/// amplitude modulator, and frequency/Q for the filter.
class ChufferEdit : public SynthEdit
{
protected:
	void SetFRange(SynthWidget *wdg, int draw = 0);
	void SetQRange(SynthWidget *wdg, int draw = 0);

public:
	ChufferEdit();
	~ChufferEdit();

	void GetParams();
	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
