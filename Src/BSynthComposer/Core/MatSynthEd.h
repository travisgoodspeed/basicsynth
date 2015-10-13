//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MATSYNTH_ED_H
#define MATSYNTH_ED_H

#define MATMAXSEG 6

class MatrixSynthEdit : public SynthEdit
{
public:
	int curGen;
	int curEnv;
	int genOn;
	SynthWidget *genWdg;
	SynthWidget *envWdg;
	EnvelopeWidget *egWdg;
	GraphWidget *frqGraph;
	GraphWidget *volGraph;
	GraphWidget *modGraph;

	void LoadLFOValues();
	void LoadGenValues();
	void LoadEnvValues();
	void SetEnvLvlRange(float rng, int draw = 0);
	void SetEnvRtRange(float rng, int draw = 0);

public:
	MatrixSynthEdit();
	virtual ~MatrixSynthEdit();

	virtual void SetInstrument(InstrConfig *ip);
	virtual void GetParams();
	virtual void LoadValues();
	virtual void SaveValues();
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
