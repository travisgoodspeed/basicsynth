//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _FMSYNTH_EDIT_H
#define _FMSYNTH_EDIT_H

/// Editor for FMSynth.
/// @sa SynthEdit 
class FMSynthEdit : public SynthEdit
{
public:
	FMSynthEdit();
	~FMSynthEdit();
	
	void SaveValues();
	void ValueChanged(SynthWidget *wdg);
};

#endif
