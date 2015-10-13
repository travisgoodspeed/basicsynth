//////////////////////////////////////////////////////////////////////
// BasicSynth - FM Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "FMSynthEd.h"

FMSynthEdit::FMSynthEdit()
{
}

FMSynthEdit::~FMSynthEdit()
{
}

void FMSynthEdit::SaveValues()
{
	static int ips[] = { 18, 16, 17, 60, 61,
		30, 31, 32, 33, 34, 35, 36, 37, 38, 39, // Gen I
		40, 41, 42, 43, 44, 45, 46, 47, 48, 49, // Gen II
		50, 51, 52, 53, 54, 55, 56, 57, 58, 59, // Gen III
		62, 63, 64, 65, 66, 67, 68, 69, 70, 71, // Noize
		80, 81, 82, 90, 91, 92, 93, -1 }; // Delay - LFO
	parms->Reset();
	SynthEdit::SaveValues(ips);
}

void FMSynthEdit::ValueChanged(SynthWidget *wdg)
{
	int id = wdg->GetID();
	switch (id)
	{
	case 11: // Gen I wt
		SelectWavetable(mainGroup->FindID(6));
		break;
	case 61: // Gen II wt
		SelectWavetable(mainGroup->FindID(56));
		break;
	case 111: // Gen III wt
		SelectWavetable(mainGroup->FindID(106));
		break;
	case 228: // LFO wt
		SelectWavetable(mainGroup->FindID(223));
		break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}
