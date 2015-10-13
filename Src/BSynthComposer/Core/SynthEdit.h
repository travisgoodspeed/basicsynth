//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _SYNTHEDIT_H_
#define _SYNTHEDIT_H_

#include "WidgetForm.h"

/////////////////////////////////////////////////////////////////
/// SynthEdit provides a generic editor for instruments dervied
/// from InstrumentVP. 
/// WidgetForm does most of the work of drawing the display.
/// SynthEdit handles the specialized work of connecting form
/// widgets to instrument parameters. When a widget signals
/// a change by calling ValueChanged, the editor copies the
/// widget value and/or state to the associated instrument
/// parameter. For instruments where there is one widget
/// for each editable parameter, the derived class only needs
/// to handle specialized ValueChanged functions. More complex
/// instruments where only some values are displayed must
/// implement specialized code for LoadValues and ValueChanged.
/// (See MatSynthEd for an example.)
////////////////////////////////////////////////////////////////
class SynthEdit : public WidgetForm
{
protected:
	InstrConfig *instr;
	InstrumentVP  *tone;
	VarParamEvent *parms;
	VarParamEvent *save;
	int changed;

public:
	SynthEdit();
	virtual ~SynthEdit();
	virtual int IsChanged();
	virtual void SetInstrument(InstrConfig *ip);
	virtual void SetPair(int knobID, int lblID);
	virtual void ValueChanged(SynthWidget *wdg);
	virtual int SelectWavetable(int wt);
	virtual void SelectWavetable(SynthWidget *wdg, int draw = 1);

	virtual void Cancel();
	virtual void GetParams();
	virtual void SetParams();
	virtual void LoadValues();
	virtual void SaveValues();
	virtual void SaveValues(int *ips);
};

#endif
