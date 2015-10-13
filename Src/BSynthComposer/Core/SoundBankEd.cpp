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
#include "SoundBankEd.h"

extern int SelectSoundBankPreset(SFPlayerInstr *instr);

SoundBankEdit::SoundBankEdit()
{
	sb = 0;
}

SoundBankEdit::~SoundBankEdit()
{
}

void SoundBankEdit::SetInstrument(InstrConfig *ip)
{
	SynthEdit::SetInstrument(ip);

	sb = (SFPlayerInstr*)tone;

}

void SoundBankEdit::GetParams()
{
	LoadValues();
	SynthWidget *wdg = mainGroup->FindID(2);
	wdg->SetText(sb->GetSoundFile());
	wdg = mainGroup->FindID(6);
	wdg->SetText(sb->GetInstrName());
	float bank = 0;
	float preset = 0;
	sb->GetParam(16, &bank);
	sb->GetParam(17, &preset);
	//ListPitches(bank, preset);
}

void SoundBankEdit::ValueChanged(SynthWidget *wdg)
{
	const char *txt;
	switch (wdg->GetID())
	{
	case 2: // soundbank name
		txt = wdg->GetText();
		sb->SetSoundFile(txt);
		//sb->SetSoundBank(SFSoundBank::FindBank(txt);
		theProject->SetChange(1);
		break;

	case 8: // preset selector
		if (SelectSoundBankPreset(sb))
		{
			float bank;
			float preset;
			wdg = mainGroup->FindID(2);
			wdg->SetText(sb->GetSoundFile());
			sb->GetParam(16, &bank);

			wdg = mainGroup->FindID(3);
			wdg->SetValue(bank);
			sb->GetParam(17, &preset);

			wdg = mainGroup->FindID(4);
			wdg->SetValue(preset);

			wdg = mainGroup->FindID(6);
			wdg->SetText(sb->GetInstrName());

			wdg = mainGroup->FindID(1);
			Redraw(wdg);
			theProject->SetChange(1);
			//ListPitches(bank, preset);
		}
		break;
	case 74: // Vib LFO wavetable
		SelectWavetable(mainGroup->FindID(71));
		break;
	case 80: // Pitch bend wavetable
		SelectWavetable(wdg);
		break;
	default:
		SynthEdit::ValueChanged(wdg);
		break;
	}
}

// Not currently used...
void SoundBankEdit::ListPitches(float bnum, float pnum)
{
	static const char *pitltr[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
	bsInt16 lo = 127;
	bsInt16 hi = 0;

	SBZone *zone;
	SBInstr *instr;
	SoundBank *sndbnk = SoundBank::FindBank(sb->GetSoundFile());
	if (sndbnk)
	{
		instr = sndbnk->GetInstr((bsInt16)bnum, (bsInt16)pnum);
		if (instr)
		{
			zone = 0;
			while ((zone = instr->EnumZones(zone)) != 0)
			{
				if (zone->chan == 0)
				{
					if (zone->lowKey < lo)
						lo = zone->lowKey;
					if (zone->highKey > hi)
						hi = zone->highKey;
				}
			}
		}
	}

	char buf[80];
	if (lo < hi)
		snprintf(buf, 80, "{%s%d,%s%d} ",  pitltr[lo%12], (lo/12)-1, pitltr[hi%12], (hi/12)-1);
	else
		buf[0] = 0;
	SynthWidget *wdg = mainGroup->FindID(200);
	wdg->SetText(buf);
	Redraw(wdg);
}

///////////////// GM /////////////////////////////

extern int SelectSoundBankPreset(GMPlayer *gm);


GMPlayerEdit::GMPlayerEdit()
{
	gm = 0;
}

GMPlayerEdit::~GMPlayerEdit()
{
}

void GMPlayerEdit::SetInstrument(InstrConfig *ip)
{
	SynthEdit::SetInstrument(ip);
	gm = (GMPlayer*)tone;
}

void GMPlayerEdit::GetParams()
{
	SynthWidget *wdg = mainGroup->FindID(2);
	if (wdg && gm)
		wdg->SetText(gm->GetSoundFile());
	SynthEdit::GetParams();
}

void GMPlayerEdit::SetParams()
{
	SynthWidget *wdg = mainGroup->FindID(2);
	if (wdg && gm)
		gm->SetSoundFile(wdg->GetText());
	SynthEdit::SetParams();
}

void GMPlayerEdit::ValueChanged(SynthWidget *wdg)
{
	if (!gm)
		return;

	int id = wdg->GetID();
	if (id == 2)
	{
		gm->SetSoundFile(wdg->GetText());
		theProject->SetChange(1);
	}
	else if (id == 16)
	{
		if (SelectSoundBankPreset(gm))
		{
			float val;
			wdg = mainGroup->FindID(2);
			if (wdg)
			{
				wdg->SetText(gm->GetSoundFile());
				Redraw(wdg);
			}
			wdg = mainGroup->FindIP(GMPLAYER_BANK);
			if (wdg)
			{
				gm->GetParam(GMPLAYER_BANK, &val);
				wdg->SetValue(val);
				Redraw(wdg);
			}
			wdg = mainGroup->FindIP(GMPLAYER_PROG);
			if (wdg)
			{
				gm->GetParam(GMPLAYER_PROG, &val);
				wdg->SetValue(val);
				Redraw(wdg);
			}
			theProject->SetChange(1);
		}
	}
	else
		SynthEdit::ValueChanged(wdg);
}
