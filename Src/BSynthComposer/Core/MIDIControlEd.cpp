//////////////////////////////////////////////////////////////////////
// BasicSynth - MIDIControl object setup.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "MIDIControlEd.h"

extern int SelectSoundBankPreset(bsString& file, MIDIControl *instr, int chnl);

MIDIControlEd::MIDIControlEd()
{ 
}

MIDIControlEd::~MIDIControlEd()
{
}

// GetParams gets the values from the MIDIControl object
// and fills in the form. Widget IDs are setup such that
//    id = (channel * 100) + CC#Index
// CCIndex is an index into the following table.
// Program change (which should be a CC#) is mapped to
// a CC# value of 128 and handled as an exception.
// In addition, we treat bank# as an exception so we
// don't have to mess with MSB/LSB stuff.
// At present, we only support a few values, listed
// in the following array. To add more values, add them
// to the form and add the CC# to this list.

static bsInt16 midiCCNum[] = 
{
	128, // bank Number
	129, // Program change
	MIDI_CTRL_VOL,
	MIDI_CTRL_PAN
};

static int maxCCIndex = sizeof(midiCCNum) / sizeof(bsInt16);

void MIDIControlEd::GetParams()
{
	sbFile = theProject->midiInfo->GetSoundBankFile();
	if (sbFile.Length() == 0)
	{
		SoundBank *sb = SoundBank::SoundBankList.next;
		if (sb)
			sbFile = sb->name;
	}
	SynthWidget *wdg = mainGroup->FindID(2);
	if (wdg)
		wdg->SetText(sbFile);
	LoadValues();
}

void MIDIControlEd::LoadValues()
{
	MIDIControl *ctl = &theProject->mgr;
	SynthWidget *wdg;
	int chnl;
	int ndx;
	int ccn;
	int id;
	for (chnl = 1; chnl <= 16; chnl++)
	{
		for (ndx = 0; ndx < maxCCIndex; ndx++)
		{
			ccn = midiCCNum[ndx];
			id = (chnl * 100) + ndx;
			wdg = mainGroup->FindID(id);
			if (wdg)
			{
				if (ccn < 128)
				{
					bsInt16 v = ctl->GetCC(chnl-1, ccn);
					wdg->SetValue((float)v);
				}
				else if (ccn == 128) // bank #
					wdg->SetValue((float)ctl->GetBank(chnl-1));
				else if (ccn == 129) // program change
					wdg->SetValue((float)ctl->GetPatch(chnl-1));
			}
		}
	}
}
// SetParams makes a copy of the current form
// values into the MIDIControl instrument. 
// Since each value is set as soon as changed,
// we have nothing to do here. 
void MIDIControlEd::SetParams()
{
}

void MIDIControlEd::Cancel()
{
}

// ValueChanged passes the new value to the instrument.
void MIDIControlEd::ValueChanged(SynthWidget *wdg)
{
	int id = wdg->GetID();
	int chnl;
	int ndx;
	int ccn;

	switch (id)
	{
	case 2:
		sbFile = wdg->GetText();
		theProject->midiInfo->SetSoundBankFile(sbFile);
		theProject->SetChange(1);
		break;
	case 3:
		if (SelectSoundBankPreset(sbFile, NULL, -1))
		{
			if (wdg)
			{
				wdg->SetText(sbFile);
				theProject->midiInfo->SetSoundBankFile(sbFile);
				Redraw(wdg);
			}
			theProject->SetChange(1);
		}
		break;
	case 4:
		GetParams();
		Redraw(0);
		break;
	default:
		chnl = (id / 100);
		ndx = id % 100;
		if (ndx == 99)
		{
			if (SelectSoundBankPreset(sbFile, &theProject->mgr, chnl-1))
			{
				wdg = mainGroup->FindID(chnl * 100);
				if (wdg)
				{
					wdg->SetValue(theProject->mgr.GetBank(chnl-1));
					Redraw(wdg);
				}
				wdg = mainGroup->FindID((chnl * 100) + 1);
				if (wdg)
				{
					wdg->SetValue(theProject->mgr.GetPatch(chnl-1));
					Redraw(wdg);
				}
			}
		}
		else if (ndx >= 0 && ndx < maxCCIndex && chnl >= 1 && chnl <= 16)
		{
			ccn = midiCCNum[ndx];
			bsInt16 val = (bsInt16) wdg->GetValue();
			if (ccn < 128)
				theProject->mgr.SetCC(chnl-1, ccn, val);
			else if (ccn == 128)
				theProject->mgr.SetBank(chnl-1, val);
			else if (ccn == 129)
				theProject->mgr.SetPatch(chnl-1, val);
		}
		theProject->SetChange(1);
		break;
	}
}
