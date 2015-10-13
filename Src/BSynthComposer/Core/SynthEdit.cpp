//////////////////////////////////////////////////////////////////////
// BasicSynth - Base class for instrument editors.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"

SynthEdit::SynthEdit()
{ 
	instr = 0; 
	tone = 0;
	parms = 0;
	save = 0;
	changed = 0;
}

SynthEdit::~SynthEdit()
{
	if (parms)
	{
		delete parms;
		parms = 0;
	}
	if (save)
	{
		delete save;
		save = 0;
	}
}

int SynthEdit::IsChanged()
{ 
	return changed; 
}

void SynthEdit::SetInstrument(InstrConfig *ip)
{ 
	instr = ip;
	if (ip)
	{
		tone = (InstrumentVP *) ip->instrTmplt;
		if (tone)
		{
			save = (VarParamEvent *) tone->AllocParams();
			parms = (VarParamEvent *)tone->AllocParams();
			if (save)
				tone->GetParams(save);
		}
	}
}

void SynthEdit::SetPair(int knobID, int lblID)
{
	if (mainGroup)
	{
		SynthWidget *knb = mainGroup->FindID(knobID);
		SynthWidget *lbl = mainGroup->FindID(lblID);
		if (knb && lbl && knb != lbl)
		{
			knb->SetBuddy2(lbl);
			lbl->SetBuddy1(knb);
		}
	}
}

// Cancel restores the instrument to the last save point
void SynthEdit::Cancel()
{
	if (save && tone)
		tone->SetParams(save);
}

// GetParams gets the parameters from the instrument and fills in the form.
// This may appear unnecessary since it only calls LoadValues, but some
// instruments need special code on the first Load. GetParams is generally
// only called once when the editor is first loaded.
// copy tone -> params -> form
void SynthEdit::GetParams()
{
	LoadValues();
}

// SetParams makes a copy of the current instrument
// parameters into the save buffer
void SynthEdit::SetParams()
{
	if (save && tone)
	{
		save->Reset();
		tone->GetParams(save);
	}
}

// copy parms -> form
void SynthEdit::LoadValues()
{
	if (parms == 0 || tone == 0 || mainGroup == 0)
		return;

	parms->Reset();
	tone->GetParams(parms);

	int n = parms->numParam;
	bsInt16 *ids = parms->idParam;
	float *vals = parms->valParam;
	while (--n >= 0)
	{
		SynthWidget *wdg = mainGroup->FindIP(*ids);
		if (wdg)
		{
			if (wdg->GetType() == wdgSwitch)
				wdg->SetState((int) *vals);
			else
				wdg->SetValue(*vals);
		}
		ids++;
		vals++;
	}
}

void SynthEdit::SaveValues()
{
	// implemented by derived class...
}

// copy form -> save
// This is not currently used. Because parameters are passed
// directly to the instrument as they change, and we can 
// update the save buffer easiest by grabbing them back
// from the instrument, it is not necessary to have 
// a "Save" function. However, if the instrument were to 
// get changed by something other than the editor, 
// this method can be called to refresh the save buffer
// (and/or instrument) from the form values.
// N.B.: This won't work if some instrument parameters
// are not on the screen!
// This value is typically called from the SaveValues()
// method of a derived class which must pass an array
// of instrument parameter ids that are to be fetched
// from the form.
void SynthEdit::SaveValues(int *ips)
{
	if (save == 0 || tone == 0 || mainGroup == 0 || ips == 0)
		return;

	save->Reset();
	SynthWidget *wdg;
	for (int n = 0; ips[n] != -1; n++)
	{
		wdg = mainGroup->FindIP(ips[n]);
		if (wdg)
		{
			if (wdg->GetType() == wdgSwitch)
				save->SetParam(ips[n], (float) wdg->GetState());
			else
				save->SetParam(ips[n], wdg->GetValue());
		}
	}
}

// ValueChanged passes the new value to the instrument.
// We also handle the default Set/Restore commands.
void SynthEdit::ValueChanged(SynthWidget *wdg)
{
	if (tone == 0 || wdg == 0)
		return;

	int id = wdg->GetID();
	if (id == 1000) // Set save point.
	{
		SetParams();
		return;
	}
	if (id == 1001) // Restore save point.
	{
		if (tone && save)
			tone->SetParams(save);
		LoadValues();
		Redraw(NULL);
		return;
	}

	bsInt16 iid = wdg->GetIP();
	if (iid > 0)
	{
		if (wdg->GetType() == wdgSwitch)
			tone->SetParam(iid, (float) wdg->GetState());
		else
			tone->SetParam(iid, wdg->GetValue());
		theProject->SetChange(1);
	}
}

// Select a wavetable 
void SynthEdit::SelectWavetable(SynthWidget *wdg, int draw)
{
	int val1 = (int) wdg->GetValue();
	int val2 = SelectWavetable(val1);
	if (val1 != val2)
	{
		float val = (float) val2;
		wdg->SetValue(val);
		tone->SetParam(wdg->GetIP(), val);
		if (draw)
			Redraw(wdg);
		theProject->SetChange(1);
	}
}

#if _TOO_LAZY_TO_CREATE_A_DIALOG_
// This method is intended to be used to load a 
// platform-specific wavetable selector.
// It should display available wavetables, a nice graph
// of the wavetable, and let the user pick one.
// However, this can be used until such a dialog is done.
void SynthEdit::SelectWavetable(int wt)
{
	int nwt;
	char buf[20];
	do
	{
		nwt = wt;
		snprintf(buf, 20, "%d", nwt);
		if (!prjFrame->QueryValue("Enter wavetable #", buf, 20))
			break;
		nwt = atoi(buf);
	} while (nwt < 0 || nwt >= wtSet.wavTblMax);
	return nwt;
}
#endif

