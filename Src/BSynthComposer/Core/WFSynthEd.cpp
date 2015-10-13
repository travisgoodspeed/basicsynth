//////////////////////////////////////////////////////////////////////
// BasicSynth - Wavefile synthesis instrument editor.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "WFSynthEd.h"

WFSynthEdit::WFSynthEdit()
{
}

WFSynthEdit::~WFSynthEdit()
{
}

void WFSynthEdit::SaveValues()
{
	static int ips[] = { 16, 17, 18, 19, 20, -1 };
	parms->Reset();
	parms->SetParam(P_VOLUME, 1.0);
	SynthEdit::SaveValues(ips);
}

void WFSynthEdit::ValueChanged(SynthWidget *wdg)
{
	switch (wdg->GetID())
	{
	case 100: // Set
		SetParams();
		break;
	case 101: // Restore
		tone->SetParams(save);
		LoadValues();
		Redraw(0);
		break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}
