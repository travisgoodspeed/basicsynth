//////////////////////////////////////////////////////////////////////
/// @file LFO.cpp Implementation of the LFO unit.
// BasicSynth LFO: Low Frequency Oscillator
//
// Used for vibrato and similar effects. By default this uses the
// fast GenWave32 oscillator and introduces minimal additional
// calculation to the instrument when applied to the PhaseModWT input
// of other oscillators.
// A one-segment attack envelope is built-in to the LFO unit to allow
// for delayed onset of vibrato.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "LFO.h"

LFO::LFO()
{
	osc.InitWT(3.5, WT_SIN);
	//atk.InitSeg(0, 0, 1.0);
	atkRt = 0.1;
	depth = 0;
	sigFrq = 0;
	ampLvl = 1.0;
	lfoOn = 0;
}

LFO::~LFO()
{

}

void LFO::InitLFO(FrqValue frq, int wvf, FrqValue rt, AmpValue amp, FrqValue sig)
{
	osc.InitWT(frq, wvf);
	atkRt = rt;
	depth = amp;
	sigFrq = sig;
	lfoOn = amp > 0;
}

void LFO::GetSettings(FrqValue &frq, int &wvf, FrqValue& rt, AmpValue& amp)
{
	frq = osc.GetFrequency();
	wvf = osc.GetWavetable();
	rt = atkRt;
	amp = depth;
}

void LFO::Copy(LFO *tp)
{
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	atkRt = tp->atkRt;
	depth = tp->depth;
	sigFrq = tp->sigFrq;
	lfoOn = depth > 0;
}

void LFO::Init(int n, float *f)
{
	if (n == 4)
		InitLFO(f[0], (int)f[1], f[2], f[3], 1.0);
}

// if signal frequency set, treat depth
// as a value in cents deviation.
void LFO::Reset(float initPhs)
{
	if (initPhs == 0)
		atk.InitSeg(atkRt, 0.0, 1.0);
	else
		atk.Reset(initPhs);
	if (sigFrq != 0)
	{
		//FrqValue f1 = sigFrq * FrqValue(pow(2.0, depth / 12.0));
		FrqValue f1 = sigFrq * synthParams.GetCentsMult((int)(depth * 100.0));
		ampLvl = AmpValue(fabs(f1 - sigFrq));
	}
	else
		ampLvl = depth;
	osc.Reset(initPhs);
}

int LFO::Load(XmlSynthElem *elem)
{
	float dvals[3];
	short ival;

	elem->GetAttribute("frq", dvals[0]);
	elem->GetAttribute("wt", ival);
	elem->GetAttribute("atk", dvals[1]);
	elem->GetAttribute("amp", dvals[2]);
	InitLFO(FrqValue(dvals[0]), (int)ival, FrqValue(dvals[1]), AmpValue(dvals[2]), 0.0);
	return 0;
}


int LFO::Save(XmlSynthElem *elem)
{
	elem->SetAttribute("frq", osc.GetFrequency());
	elem->SetAttribute("wt",  (short) osc.GetWavetable());
	elem->SetAttribute("atk", atkRt);
	elem->SetAttribute("amp", depth);
	return 0;
}

