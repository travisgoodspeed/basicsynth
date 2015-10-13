//////////////////////////////////////////////////////////////////////
// BasicSynth - Chuffer Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "ChufferEd.h"

ChufferEdit::ChufferEdit()
{
}

ChufferEdit::~ChufferEdit()
{
}

// copy: instrument -> parms -> form
void ChufferEdit::GetParams()
{
	LoadValues();
	SetFRange(mainGroup->FindID(51), 0);
	//SetQRange(mainGroup->FindID(70), 0);
}

void ChufferEdit::SaveValues()
{
	static int ips[] = {
		16, 17, 18, 19, 20, 21, 22, 
		23, 24, 25, 26, 27,
		30, 31, 32, 33, 34, 35, 36,
		40, 41, 42, 43, 44, 45, 46, 
		50, -1 };

	SynthEdit::SaveValues(ips);
}

void ChufferEdit::ValueChanged(SynthWidget *wdg)
{
	switch(wdg->GetID())
	{
	//case 1000: // Set
	//	SetParams();
	//	break;
	case 1001: // Restore
		if (tone && save)
		{
			tone->SetParams(save);
			LoadValues();
			SetFRange(mainGroup->FindID(51), 0);
			//SetQRange(mainGroup->FindID(70), 0);
			Redraw(NULL);
		}
		break;
	case 27:
	case 87:
		SelectWavetable(wdg, 0);
		break;
	case 51: // flt end freq./percent
		SetFRange(wdg, 1);
		SynthEdit::ValueChanged(wdg);
		break;
	//case 70: // q end val/percent
	//	SetQRange(wdg, 1);
	//	SynthEdit::ValueChanged(wdg);
	//	break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}

void ChufferEdit::SetFRange(SynthWidget *wdg, int draw)
{
	KnobWidget *endVal = (KnobWidget*) mainGroup->FindID(45);
	if (wdg->GetState()) // track start
		endVal->SetScale(1.0);
	else // actual value
		endVal->SetScale(1000.0);
	if (draw)
		Redraw(endVal);
}

void ChufferEdit::SetQRange(SynthWidget *wdg, int draw)
{
	KnobWidget *endVal = (KnobWidget*) mainGroup->FindID(65);
	if (wdg->GetState()) // track start
		endVal->SetRange(0.01, 20.0, 3);
	else // actual value
		endVal->SetRange(1.0, 20.0, 3);
	if (draw)
		Redraw(endVal);
}
