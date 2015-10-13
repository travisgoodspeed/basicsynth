//////////////////////////////////////////////////////////////////////
// BasicSynth - Editor for wavetables.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "WavetableEdit.h"


void WavetableEdit::GetParams()
{
	SynthWidget *sw;
	KnobWidget *knb;

	int numParts = 0;
	saveSum = 1;
	saveGibbs = 0;
	if (wi)
	{
		numParts = wi->GetParts();
		saveSum = wi->GetSum();
		saveGibbs = wi->GetGibbs();
	}

	wav = (WaveWidget *)mainGroup->FindID(199);

	int ndx;
	int wid = 0;
	for (ndx = 0; ndx < WT_MAX_PARTIAL; ndx++)
	{
		sw = mainGroup->FindID(wid+1);
		if (sw)
			sw->SetState(0);
		knb = (KnobWidget*) mainGroup->FindID(wid+2);
		if (knb)
		{
			knb->SetRange(-1.0, 1.0, 4);
			knb->SetValue(0.0); // side-effect: sets label value as well
		}
		knb = (KnobWidget*) mainGroup->FindID(wid+4);
		if (knb)
		{
			knb->SetRange(0.0, (saveSum == 1) ? twoPI : 1.0, 4);
			knb->SetValue(0.0);
		}
		wid += 10;
		if (wav)
		{
			wav->SetOn(ndx, 0);
			wav->SetAmp(ndx, 0);
			wav->SetPhs(ndx, 0);
		}
		if (ndx < numParts)
		{
			double amp = 0;
			double phs = 0;
			bsInt32 mul = 0;
			wi->GetPart(ndx, mul, amp, phs);
			savebuf[mul-1].on = 1;
			savebuf[mul-1].amp = amp;
			savebuf[mul-1].phs = phs;
		}
	}

	LoadValues();
}

void WavetableEdit::SetParams()
{
	if (wi == 0)
		return;

	SynthWidget *wdg;
	SynthWidget *knb;
	int parts = 0;
	int ndx;
	int pndx;
	int wid = 0;
	for (ndx = 0; ndx < WT_MAX_PARTIAL; ndx++)
	{
		wdg = mainGroup->FindID(wid+1);
		if (wdg->GetState())
		{
			parts++;
			knb = mainGroup->FindID(wid+2);
			savebuf[ndx].amp = knb->GetValue();
			knb = mainGroup->FindID(wid+4);
			savebuf[ndx].phs = knb->GetValue();
			savebuf[ndx].on = 1;
		}
		else
		{
			savebuf[ndx].amp = 0;
			savebuf[ndx].phs = 0;
			savebuf[ndx].on = 0;
		}
		wid += 10;
	}
	if (parts < 1)
		parts = 1;
	if (wi->AllocParts(parts) != 0)
	{
		// Alert("Out of memory");
		return;
	}
	pndx = 0;
	for (ndx = 0; ndx < WT_MAX_PARTIAL; ndx++)
	{
		if (savebuf[ndx].on)
			wi->SetPart(pndx++, ndx+1, savebuf[ndx].amp, savebuf[ndx].phs);
	}

	wdg = mainGroup->FindID(200);
	saveSum = (short)wdg->GetValue();
	wi->SetSum(saveSum);
	wdg = mainGroup->FindID(203);
	saveGibbs = (short)wdg->GetState();
	wi->SetGibbs(saveGibbs);
	wi->InitWaveform();
	changed = 0;
	theProject->SetChange(1);
}

void WavetableEdit::LoadValues()
{
	if (!wi)
		return;

	int sumParts = wi->GetSum();
	SynthWidget *wdg = mainGroup->FindID(200);
	if (wdg)
		wdg->SetValue((float)sumParts);
	wdg = mainGroup->FindID(203);
	if (wdg)
		wdg->SetState(wi->GetGibbs());

	if (wav)
	{
		wav->SetGibbs(wi->GetGibbs());
		wav->SetWaveType(sumParts);
	}

	bsInt32 mul = 0;
	double amp = 0;
	double phs = 0;
	int ndx;
	int parts = wi->GetParts();
	for (ndx = 0; ndx < parts; ndx++)
	{
		wi->GetPart(ndx, mul, amp, phs);
		SetPart(mul, (float)amp, (float)phs, 1);
	}
	changed = 0;
}

void WavetableEdit::Cancel()
{
	int on = 0;
	int ndx;
	for (ndx = 0; ndx < WT_MAX_PARTIAL; ndx++)
	{
		if (savebuf[ndx].on)
			wi->SetPart(on++, ndx+1, savebuf[ndx].amp, savebuf[ndx].phs);
		SetPart(ndx+1, 0.0, 0.0, 0);
	}
	wi->SetGibbs(saveGibbs);
	wi->SetSum(saveSum);
	LoadValues();
	Redraw(0);
}

void WavetableEdit::ValueChanged(SynthWidget *wdg)
{
	changed = 1;
	int id = wdg->GetID();
	if (id == 200) // Sum/Segs
	{
		int ty = (int)wdg->GetValue();
		if (wav)
			wav->SetWaveType(ty);
		float rng = (ty == 1) ? twoPI : 1.0;
		KnobWidget *knb;
		int ndx;
		int wid = 0;
		for (ndx = 0; ndx < WT_MAX_PARTIAL; ndx++)
		{
			knb = (KnobWidget*) mainGroup->FindID(wid+4);
			if (knb)
				knb->SetRange(0.0, rng, 4);
			wid += 10;
		}
		Redraw(0);
	}
	else if (id == 203) // Gibbs on/off
	{
		if (wav)
		{
			wav->SetGibbs(wdg->GetState());
			Redraw(wav);
		}
	}
	else if (id == 300) // Clear
	{
		SetPart(1, 1.0f, 0.0f, 1);
		for (int pn = 2; pn <= WT_MAX_PARTIAL; pn++)
			SetPart(pn, 0.0f, 0.0f, 0);
		Redraw(0);
	}
	else if (id == 301) // Save
	{
		SetParams();
	}
	else if (id == 302) // Undo (back to last-saved point)
	{
		Cancel();
	}
	else if (id > 0 && id < 170) // on/off, amp, phs
	{
		if (wav)
		{
			int ndx = id / 10;
			int val = id % 10;
			if (val == 1)
				wav->SetOn(ndx, wdg->GetState()); // set part on/off
			else if (val == 2)
				wav->SetAmp(ndx, wdg->GetValue()); // set part amplitude
			else if (val == 4)
				wav->SetPhs(ndx, wdg->GetValue()); // set part phase
			Redraw(wav);
		}
	}
}

void WavetableEdit::SetPart(int mul, float amp, float phs, int on)
{
	if (mul < 1 || mul > WT_MAX_PARTIAL)
		return;

	int pn = (mul - 1);
	int wid = pn * 10;
	SynthWidget *wdg = mainGroup->FindID(wid+1); // enable
	if (wdg)
		wdg->SetState(on);

	wdg = mainGroup->FindID(wid+2); // amp
	if (wdg)
		wdg->SetValue(amp);

	wdg = mainGroup->FindID(wid+4); // phs/time
	if (wdg)
		wdg->SetValue(phs);

	if (wav)
	{
		wav->SetOn(pn, on);
		wav->SetAmp(pn, amp);
		wav->SetPhs(pn, phs);
	}
}

