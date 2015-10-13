//////////////////////////////////////////////////////////////////////
// BasicSynth - Simple tone Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "ToneSynthEd.h"

ToneSynthEdit::ToneSynthEdit()
{
}

ToneSynthEdit::~ToneSynthEdit()
{
}

void ToneSynthEdit::SaveValues()
{
	static int ips[] = { 16, 18, 19, 20, 21, 22, 24, 25, 27, 28, 29, 35, 36, 40, 41, -1 };
	SynthEdit::SaveValues(ips);
}

void ToneSynthEdit::ValueChanged(SynthWidget *wdg)
{
	switch (wdg->GetID())
	{
	case 8: // osc. wavetable
		SelectWavetable(mainGroup->FindID(1));
		break;
	case 49: // LFO wavetable
		SelectWavetable(mainGroup->FindID(101));
		break;
	case 80: // PB wavetable
		SelectWavetable(wdg);
		break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}

ToneFMSynthEdit::ToneFMSynthEdit()
{
}
