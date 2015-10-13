//////////////////////////////////////////////////////////////////////
// BasicSynth - Subtractive Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "SubSynthEd.h"

SubSynthEdit::SubSynthEdit()
{
	oldft = -1;
}

SubSynthEdit::~SubSynthEdit()
{
}

void SubSynthEdit::LoadValues()
{
	SynthEdit::LoadValues();
	float f = 0;
	tone->GetParam(18, &f); // filter type
	oldft = (int)f;
	SetResonRange(oldft, 0);
	KnobWidget *res = (KnobWidget*)mainGroup->FindID(87); // resonance knob
	if (res)
		res->SetValue(parms->GetParam(res->GetIP()));
}

/*
void SubSynthEdit::SaveValues()
{
	static int ips[] = { 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 28,
		30, 31, 32, 33, 34, 35, 36, 37, 40, 41, 42, 43, -1 };

	SynthEdit::SaveValues(ips);
}
*/
void SubSynthEdit::ValueChanged(SynthWidget *wdg)
{

	switch (wdg->GetID())
	{
	case 9: // user wavetable
		SelectWavetable(mainGroup->FindID(2));
		break;
	case 102: // LFO wavetable
		SelectWavetable(mainGroup->FindID(103));
		break;
	case 216: // PB wavetable
		SelectWavetable(wdg);
		break;
	case 12: // filter type
		SetResonRange((int) wdg->GetValue(), 1);
		// FALLTHROUGH...
	default: // everything else is a parameter value
		SynthEdit::ValueChanged(wdg);
		break;
	}
}

void SubSynthEdit::SetResonRange(int ft, int draw)
{
	KnobWidget *res = (KnobWidget*)mainGroup->FindID(87); // resonance knob
	TextWidget *lbl = (TextWidget*)mainGroup->FindID(90); // resonance label;
	float lo = 0.0;
	float hi = 1.0;
	float val = res->GetValue();
	int enable;
	if (ft == 2 || ft == 4) // Bandpass || LP w/Res
	{
		if (oldft == 3)
			val *= 20.0;
		lo = 0.5;
		hi = 20.0;
		enable = 1;
	}
	else if (ft == 3) // Reson
	{
		if (oldft == 2 || oldft == 4)
			val /= 20.0;
		if (val > 0.999)
			val = 0.999;
		lo = 0.001;
		hi = 0.999;
		enable = 1;
	}
	else // lowpass or highpass
	{
		enable = 0;
	}
	oldft = ft;
	if (lbl)
		lbl->SetEnable(enable);
	if (res)
	{
		if (enable)
		{
			res->SetRange(lo, hi, 4);
			res->SetValue(val);
		}
		res->SetEnable(enable);
		if (draw)
		{
			if (enable)
				SynthEdit::ValueChanged(res);
			Redraw(res);
		}
	}
}

