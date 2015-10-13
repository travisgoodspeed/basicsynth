//////////////////////////////////////////////////////////////////////
// BasicSynth - Additive Synthesis instrument 
//
/// @file AddSynthEd.h AddSynth editor 
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "AddSynthEd.h"

AddSynthEdit::AddSynthEdit()
{
	genWdg = 0;
	rtRange = 0;
	frqGraph = 0;
	curPart = 0;
	numParts = 0;
	as = 0;
}

AddSynthEdit::~AddSynthEdit()
{
}

void AddSynthEdit::SetInstrument(InstrConfig *ip)
{
	SynthEdit::SetInstrument(ip);
	
	curPart = 0;
	if ((as = (AddSynth*)tone) != 0)
	{
		numParts = as->GetNumParts();
		if (numParts > MAXADDPART)
			numParts = MAXADDPART;
	}
	else
		numParts = 0;

	genWdg = mainGroup->FindID(2);
	genWdg->SetValue(0);
	frqGraph = (GraphWidget*) mainGroup->FindID(401);
	rtRange = mainGroup->FindID(206);
	rtRange->SetValue(1.0);
	egWdg = (EnvelopeWidget*)mainGroup->FindID(350);
}

// copy tone -> params -> form
void AddSynthEdit::GetParams()
{
	curPart = 0;
	LoadValues();
	EnableParts();
}

void AddSynthEdit::EnableParts()
{
	int id = 3;
	SynthWidget *wdg;
	for (int pn = 0; pn < MAXADDPART; pn++)
	{
		wdg = mainGroup->FindID(id++);
		if (pn < numParts)
		{
			wdg->SetEnable(1);
			frqGraph->SetVal(pn, (float) parms->GetParam((pn+1)<<8));
		}
		else
		{
			wdg->SetEnable(0);
			frqGraph->SetVal(pn, 0);
		}
	}
}

void AddSynthEdit::EnableEnvSegs(int ns, int redraw)
{
	int iid = (curPart+1) << 8;
	if (egWdg)
	{
		egWdg->SetSegs(ns);
		float egval;
		tone->GetParam(iid+3, &egval);
		egWdg->SetStart(egval);
		tone->GetParam(iid+4, &egval);
		egWdg->SetSus((int)egval);
	}
	SynthWidget *grp = mainGroup->FindID(200);
	SynthWidget *wdg;
	int fid = 220;
	for (int sn = 0; sn < MAXADDSEG; sn++)
	{
		int e = sn < ns;
		wdg = grp->FindID(fid+1);
		wdg->SetEnable(e);
		wdg = grp->FindID(fid+2);
		wdg->SetEnable(e);
		wdg = grp->FindID(fid+3);
		wdg->SetEnable(e);
		wdg = grp->FindID(fid+4);
		wdg->SetEnable(e);
		wdg = grp->FindID(fid+5);
		wdg->SetEnable(e);
		wdg = grp->FindID(fid+9);
		wdg->SetEnable(e);
		fid += 10;
		if (e && egWdg)
		{
			float rt;
			float lvl;
			int iid2 = iid + (sn<<4);
			tone->GetParam(iid2+5, &rt);
			tone->GetParam(iid2+6, &lvl);
			egWdg->SetVal(sn, rt, lvl);
		}

	}
	if (redraw)
		Redraw(grp);
}

void AddSynthEdit::SetRateRange(float rtVal)
{
	if (rtVal == 0)
		rtVal = rtRange->GetValue();

	KnobBased *rt;
	int fid = 223;
	for (int sn = 0; sn < MAXADDSEG; sn++)
	{
		rt = (KnobBased *)mainGroup->FindID(fid);
		rt->SetRange(0, rtVal);
		fid += 10;
	}
	int pn = (curPart+1) << 8;
	float ns = parms->GetParam(pn | 9);
	egWdg->SetTime(ns*rtVal);
}

static int ipsGen[] = { 16, 17, 18, 19, 256, 258, 259, 260, -1};
static int ipsSeg[] = { 261, 262, 263, 264, -1 };

// copy form -> save
// This doesn't really work since we don't have all values on the form...
void AddSynthEdit::SaveValues()
{
	if (!save)
		return;

	save->Reset();

	float val;
	int pn = (curPart+1) << 8;
	int ipn;
	SynthWidget *wdg;
	int n;
	for (n = 0; ipsGen[n] != -1; n++)
	{
		ipn = ipsGen[n];
		wdg = mainGroup->FindIP(ipn);
		if (wdg->GetType() == wdgSwitch)
			val = (float) wdg->GetState();
		else
			val = wdg->GetValue();
		if (ipn >= 256)
			ipn = pn | (ipn & 0xf);
		save->SetParam(ipn, val);
	}

	wdg = mainGroup->FindIP(291); // num segs
	int ns = (int) wdg->GetValue();
	save->SetParam(pn | 9, (float)ns);

	int sn;
	for (sn = 0; sn < ns; sn++)
	{
		int sip = sn << 4;
		for (n = 0; ipsSeg[n] != -1; n++)
		{
			wdg = mainGroup->FindIP(ipsSeg[n] | sip);
			if (wdg->GetType() == wdgSwitch)
				val = (float) wdg->GetState();
			else
				val = wdg->GetValue();
			save->SetParam((ipsSeg[n] & 0xf) | sip | pn, val);
		}
	}
}

// copy parms -> form
void AddSynthEdit::LoadValues()
{
	if (!parms || !tone)
		return;

	parms->Reset();
	tone->GetParams(parms);

	float val;
	int pn = (curPart+1) << 8;
	SynthWidget *wdg;
	int ipn;
	int n;
	for (n = 0; ipsGen[n] != -1; n++)
	{
		ipn = ipsGen[n];
		if (ipn >= 256)
			ipn = pn | (ipn & 0xf);
		val = parms->GetParam(ipn);
		wdg = mainGroup->FindIP(ipsGen[n]);
		if (wdg)
		{
			if (wdg->GetType() == wdgSwitch)
				wdg->SetState((int)val);
			else
				wdg->SetValue(val);
			if (ipsGen[n] == 256)
				frqGraph->SetVal(curPart, val);
		}
	}

	int ns = (int) parms->GetParam(pn | 9);
	wdg = mainGroup->FindIP(265); // num segs
	if (wdg)
		wdg->SetValue((float)ns);

	SynthWidget *env = mainGroup->FindID(200);
	int sn;
	for (sn = 0; sn < MAXADDSEG; sn++)
	{
		int sip = sn << 4;
		for (n = 0; ipsSeg[n] != -1; n++)
		{
			ipn = ipsSeg[n] | sip;
			wdg = env->FindIP(ipn);
			if (wdg)
			{
				if (sn < ns)
				{
					ipn = (ipn & 0xff) | pn;
					val = parms->GetParam(ipn);
				}
				else
					val = 0;
				if (wdg->GetType() == wdgSwitch)
					wdg->SetState((int)val);
				else
					wdg->SetValue(val);
			}
		}
	}
	EnableEnvSegs(ns, 0);
}

void AddSynthEdit::ValueChanged(SynthWidget *wdg)
{
	int id = wdg->GetID();
	if (id == 2) // switch part
	{
		int newPart = (int) wdg->GetValue();
		if (newPart != curPart)
		{
			curPart = newPart;
			LoadValues();
			Redraw(mainGroup->FindID(21));
			Redraw(mainGroup->FindID(200));
			Redraw(frqGraph);
			theProject->SetChange(1);
		}
		return;
	}

	if (id == 32) // osc. wavetable
	{
		// need to modify IP for current partial
		//SelectWavetable(mainGroup->FindID(25), 1);
		SynthWidget *wtgrp = mainGroup->FindID(25);
		if (wtgrp != NULL)
		{
			float val = (float) SelectWavetable((int) wtgrp->GetValue());
			wtgrp->SetValue(val);
			tone->SetParam(((curPart+1) << 8) | (wtgrp->GetIP() & 0xff), val);
			Redraw(wdg);
			theProject->SetChange(1);
		}
		return;
	}

	if (id == 300) // # parts
	{
		char buf[40];
		//snprintf(buf, 40, "%d", numParts);
		bsString::NumToStr(numParts, buf, sizeof(buf));
		if (prjFrame->QueryValue("Enter number of parts (1-16)", buf, sizeof(buf)))
		{
			int newParts = bsString::StrToNum(buf);
			if (newParts != numParts)
			{
				numParts = newParts;
				as->SetNumParts(newParts);
				EnableParts();
				Redraw(mainGroup->FindID(2));
				Redraw(frqGraph);
				theProject->SetChange(1);
			}
		}
		return;
	}
	if (id == 206) // env rate range
	{
		SetRateRange(wdg->GetValue());
		Redraw(mainGroup->FindID(200));
		return;
	}

	if (id == 313) // LFO wt
	{
		SelectWavetable(mainGroup->FindID(315));
		return;
	}

	if (id == 291) // # env segs
	{
		float ns = wdg->GetValue();
		tone->SetParam(((curPart+1) << 8) | 9, ns);
		EnableEnvSegs((int)ns, 1);
		theProject->SetChange(1);
		return;
	}

	int iid = wdg->GetIP();
	if (iid < 0)
		return;

	float val;
	if (wdg->GetType() == wdgSwitch)
		val = (float) wdg->GetState();
	else
		val = wdg->GetValue();
	if (iid >= 256)
		iid = ((curPart+1) << 8) | (iid & 0xff);
	tone->SetParam(iid, val);
	theProject->SetChange(1);

	if (id == 33) // frq
	{
		frqGraph->SetVal(curPart, val);
		Redraw(frqGraph);
	}
	else if (id == 202) // start
	{
		egWdg->SetStart(val);
		Redraw(egWdg);
	}
	else if (id == 204) // sustain
	{
		egWdg->SetSus((int)val);
		Redraw(egWdg);
	}
	else
	{
		int vn = iid & 0xf;
		if (vn == 5 || vn == 6)
		{
			iid = ((curPart+1) << 8) | (iid & 0xf0);
			float rt;
			float lvl;
			tone->GetParam(iid+5, &rt);
			tone->GetParam(iid+6, &lvl);
			egWdg->SetVal((iid & 0xf0)>>4, rt, lvl);
			Redraw(egWdg);
		}
	}
}
