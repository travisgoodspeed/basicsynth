//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _MIDICONTROLED_H_
#define _MIDICONTROLED_H_

#include "WidgetForm.h"

/////////////////////////////////////////////////////////////////
/// MIDIControlEd is the editor for MIDI channel information.
///
/// The MIDIControl object is global, contained as a member of
/// theProject. This form sets the bank, patch, volume, pitch
/// bend sensitivity, etc. Its purpose is to do what a MIDI
/// instrument would do with various control messages. When
/// playing live from a MIDI keyboard, these values can be
/// set through the keyboard. But when playing instruments
/// by some other form (such as the virtual keyboard) we need
/// to setup controller values directly.
////////////////////////////////////////////////////////////////
class MIDIControlEd : public WidgetForm
{
protected:
	bsString sbFile;

public:
	MIDIControlEd();
	virtual ~MIDIControlEd();
	virtual void ValueChanged(SynthWidget *wdg);
	virtual void Cancel();
	virtual void GetParams();
	virtual void SetParams();
	virtual void LoadValues();
};

#endif
