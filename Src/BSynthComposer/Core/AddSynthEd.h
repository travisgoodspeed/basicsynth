//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _ADDSYNTH_ED_H
#define _ADDSYNTH_ED_H

#define MAXADDPART 16
#define MAXADDSEG 7

/// Editor for AddSynth.
/// AddSynth supports a variable number of oscillators which are summed 
/// together. Typical use is for each oscillator to represent a partial
/// that is summed to produce a complex waveform. However, the oscillators
/// support selectable wavetables and thus can be used for wavetable synthesis as well.
/// The instrument can support any number of oscillators, but
/// the editor only supports 1-16. The form displays one oscillator and envelope generator
/// at a time. A set of radio buttons selects the unit to edit. 
/// A bar graph shows the relative frequencies of the oscillators.
/// An envelope graph shows the envelope for the currently selected
/// oscillator.
class AddSynthEdit : public SynthEdit
{
private:
	int curPart;
	int numParts;
	AddSynth *as;
	SynthWidget *genWdg;
	SynthWidget *rtRange;
	GraphWidget *frqGraph;
	EnvelopeWidget *egWdg;

	void EnableParts();
	void EnableEnvSegs(int ns, int redraw);
	void SetRateRange(float rtVal);

public:
	AddSynthEdit();
	virtual ~AddSynthEdit();

	virtual void SetInstrument(InstrConfig *ip);
	virtual void GetParams();
	virtual void SaveValues();
	virtual void LoadValues();
	virtual void ValueChanged(SynthWidget *wdg);

};

#endif
