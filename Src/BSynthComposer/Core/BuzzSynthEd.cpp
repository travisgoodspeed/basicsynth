//////////////////////////////////////////////////////////////////////
// BasicSynth - BUZZ (pulse wave) Synthesis instrument editor
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "BuzzSynthEd.h"

BuzzSynthEdit::BuzzSynthEdit()
{
}

BuzzSynthEdit::~BuzzSynthEdit()
{
}

void BuzzSynthEdit::LoadValues()
{
	SynthEdit::LoadValues();
}

void BuzzSynthEdit::ValueChanged(SynthWidget *wdg)
{
	switch (wdg->GetID())
	{
	case 807: // LFO wavetable
		SelectWavetable(mainGroup->FindID(804));
		break;
	case 904: // PB wavetable
		SelectWavetable(wdg);
		break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}

