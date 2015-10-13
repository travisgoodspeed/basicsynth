//////////////////////////////////////////////////////////////////////
// BasicSynth - Matrix Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "MatSynthEd.h"

MatrixSynthEdit::MatrixSynthEdit()
{
	curGen = 0;
	curEnv = 0;
	genOn = 0;
	genWdg = 0;
	envWdg = 0;
	frqGraph = 0;
	modGraph = 0;
	volGraph = 0;
	egWdg = 0;
}

MatrixSynthEdit::~MatrixSynthEdit()
{
}

void MatrixSynthEdit::SetInstrument(InstrConfig *ip)
{
	SynthEdit::SetInstrument(ip);
	curGen = 0;
	curEnv = 0;
	save = new VarParamEvent;
	tone->GetParams(save);

	genWdg = mainGroup->FindID(1);
	genWdg->SetValue(0);
	envWdg = mainGroup->FindID(100);
	envWdg->SetValue(0);

	SynthWidget *wdg;
	// env rate range
	wdg = mainGroup->FindID(206);
	wdg->SetValue(1);
	// env level range
	wdg = mainGroup->FindID(211);
	wdg->SetValue(1);

	egWdg = (EnvelopeWidget*) mainGroup->FindID(300);
	egWdg->SetTime(10);
	egWdg->SetLevel(1.0);
	frqGraph = (GraphWidget*) mainGroup->FindID(401);
	volGraph = (GraphWidget*) mainGroup->FindID(403);
	modGraph = (GraphWidget*) mainGroup->FindID(405);
}

void MatrixSynthEdit::GetParams()
{
	curGen = 0;
	genWdg->SetValue(0);
	SynthEdit::GetParams();

	for (int n = 0; n < MATGEN; n++)
	{
		bsInt16 idval = (1 << 11) + (n << 8);
		int on = (int) parms->GetParam(idval);
		if (on & TONE_ON)
		{
			frqGraph->SetVal(n, parms->GetParam(idval+3));
			volGraph->SetVal(n, parms->GetParam(idval+5));
			modGraph->SetVal(n, parms->GetParam(idval+4));
		}
		else
		{
			frqGraph->SetVal(n, 0);
			volGraph->SetVal(n, 0);
			modGraph->SetVal(n, 0);
		}
	}
}

void MatrixSynthEdit::LoadValues()
{
	if (tone && parms)
	{
		parms->Reset();
		tone->GetParams(parms);
		LoadLFOValues();
		LoadGenValues();
		LoadEnvValues();
	}
}

void MatrixSynthEdit::LoadLFOValues()
{
	SetWidgetValue(55, parms->GetParam(16)); // lfo frq
	SetWidgetValue(56, parms->GetParam(18)); // lfo atk
	SetWidgetValue(57, parms->GetParam(19)); // lfo lvl
	SetWidgetValue(61, parms->GetParam(17)); // lfo wt
	SetWidgetValue(351, parms->GetParam(25)); // pit bend amnt
	SetWidgetValue(353, parms->GetParam(26)); // pit bend wt
}

void MatrixSynthEdit::LoadGenValues()
{
	float val;
	bsInt16 idval = (1 << 11) | (curGen << 8);

	val = parms->GetParam(idval); // tone flags
	SetWidgetValue(11, val);
	genOn = ((int) val) & TONE_ON;

	val = parms->GetParam(idval+6);
	SetWidgetValue(100, val); // env. index
	curEnv = (int) val;

	SetWidgetValue(21, parms->GetParam(idval+1)); // mod-in flags
	SetWidgetValue(31, parms->GetParam(idval+2)); // wavetable
	SetWidgetValue(41, parms->GetParam(idval+3)); // frq mult
	SetWidgetValue(47, parms->GetParam(idval+4)); // mod lvl
	SetWidgetValue(46, parms->GetParam(idval+5)); // vol lvl
	SetWidgetValue(70, parms->GetParam(idval+7)); // fx1 lvl
	SetWidgetValue(71, parms->GetParam(idval+8)); // fx2 lvl
	SetWidgetValue(72, parms->GetParam(idval+9)); // fx3 lvl
	SetWidgetValue(73, parms->GetParam(idval+10)); // fx4 lvl
	SetWidgetValue(69, parms->GetParam(idval+11)); // lfo lvl
	SetWidgetValue(74, parms->GetParam(idval+14)); // Pan set
}

void MatrixSynthEdit::LoadEnvValues()
{
	float maxRt = 0;
	float maxLvl = 0;
	float rt;
	float lvl;
	float typ;
	float fix;
	int enb;
	SynthWidget *wdg;

	bsInt16 idval = (2 << 11) | (curEnv << 8);

	maxLvl = parms->GetParam(idval);
	SetWidgetValue(202, maxLvl); // env start
	egWdg->SetStart(maxLvl);

	int son = (int) parms->GetParam(idval+1); // env sus-on
	SetWidgetState(204, son);
	egWdg->SetSus(son);

	float nv = parms->GetParam(idval+6);
	SetWidgetValue(281, nv); // env segs
	int ns = (int) nv;
	egWdg->SetSegs(ns);

	int fid = 220;
	for (int sn = 0; sn < MATMAXSEG; sn++)
	{
		idval = (2 << 11) | (curEnv << 8) | (sn << 3);
		if ((enb = sn < ns))
		{
			rt = parms->GetParam(idval+2);
			lvl = parms->GetParam(idval+3);
			egWdg->SetVal(sn, rt, lvl);
			maxRt += rt;
			if (lvl > maxLvl)
				maxLvl = lvl;
			typ = parms->GetParam(idval+4); // env seg typ
			fix = parms->GetParam(idval+5); // env seg fix
		}
		else
		{
			rt = 0.0;
			lvl = 0.0;
			typ = 1.0;
			fix = 0;
		}
		wdg = mainGroup->FindID(fid+1);
		if (wdg)
		{
			wdg->SetValue(rt);
			wdg->SetEnable(enb);
		}
		wdg = mainGroup->FindID(fid+3);
		if (wdg)
		{
			wdg->SetValue(lvl);
			wdg->SetEnable(enb);
		}
		wdg = mainGroup->FindID(fid+5);
		if (wdg)
		{
			wdg->SetValue(typ);
			wdg->SetEnable(enb);
		}
		wdg = mainGroup->FindID(fid+9);
		if (wdg)
		{
			wdg->SetValue(fix);
			wdg->SetEnable(enb);
		}
		fid += 10;
	}
	if (maxRt <= 0.5)
		maxRt = 0.5;
	else if (maxRt <= 1.0)
		maxRt = 1.0;
	else if (maxRt <= 5.0)
		maxRt = 5.0;
	else if (maxRt <= 10)
		maxRt = 10.0;
	SetEnvRtRange(maxRt);
	SetWidgetValue(206, maxRt);

	if (maxLvl <= 1.0)
		maxLvl = 1.0;
	else if (maxLvl <= 5.0)
		maxLvl = 5.0;
	else if (maxLvl <= 20.0)
		maxLvl = 20.0;
	SetEnvLvlRange(maxLvl);
	SetWidgetValue(211, maxLvl);
}

void MatrixSynthEdit::SaveValues()
{
}

void MatrixSynthEdit::ValueChanged(SynthWidget *wdg)
{
	int id = wdg->GetID();
	if (id == 1)
	{
		int newGen = (int) wdg->GetValue();
		if (newGen != curGen)
		{
			curGen = newGen;
			LoadValues();
			Redraw(NULL);
		}
		return;
	}
	
	if (id == 1000 || id == 1001) // Set/Restore
	{
		SynthEdit::ValueChanged(wdg);
		return;
	}
	
	if (id == 206)
	{
		SetEnvRtRange(wdg->GetValue(), 1);
		return;
	}
	
	if (id == 211)
	{
		SetEnvLvlRange(wdg->GetValue(), 1);
		return;
	}

	theProject->SetChange(1);

	if (id == 12) // gen on/off
	{
		genOn = wdg->GetState();
		if (genOn)
		{
			frqGraph->SetVal(curGen, GetWidgetValue(41));
			volGraph->SetVal(curGen, GetWidgetValue(46));
			modGraph->SetVal(curGen, GetWidgetValue(47));
		}
		else
		{
			frqGraph->SetVal(curGen, 0);
			volGraph->SetVal(curGen, 0);
			modGraph->SetVal(curGen, 0);
		}
		Redraw(frqGraph);
		Redraw(volGraph);
		Redraw(modGraph);
	}
	else if (id == 38) // osc. wavetable
	{
		SelectWavetable(mainGroup->FindID(31), 1);
	}
	else if (id == 41) // Frq. X
	{
		if (genOn)
		{
			frqGraph->SetVal(curGen, wdg->GetValue());
			Redraw(frqGraph);
		}
	}
	else if (id == 46) // Volume
	{
		if (genOn)
		{
			volGraph->SetVal(curGen, wdg->GetValue());
			Redraw(volGraph);
		}
	}
	else if (id == 47) // Modulation
	{
		if (genOn)
		{
			modGraph->SetVal(curGen, wdg->GetValue());
			Redraw(modGraph);
		}
	}
	else if (id == 61 || id == 353) // LFO wt or PB wt
	{
		SelectWavetable(wdg, 0);
	}
	else if (id == 100) // envelope #
	{
		int newEnv = (int) wdg->GetValue();
		if (newEnv != curEnv)
		{
			curEnv = newEnv;
			LoadEnvValues();
			Redraw(egWdg);
			Redraw(mainGroup->FindID(200));
		}
	}
	else if (id == 202) // env start
	{
		egWdg->SetStart(wdg->GetValue());
		Redraw(egWdg);
	}
	else if (id == 204) // env sus-on
	{
		egWdg->SetSus((int)wdg->GetState());
		Redraw(egWdg);
	}
	else if (id == 221 || id == 231 || id == 241 || id == 251 || id == 261 || id == 271)
	{
		// env rate
		egWdg->SetVal((id - 221) / 10, wdg->GetValue(), GetWidgetValue(id+2));
		Redraw(egWdg);
	}
	else if (id == 223 || id == 233 || id == 243 || id == 253 || id == 263 || id == 273)
	{
		// env level
		egWdg->SetVal((id - 223) / 10, GetWidgetValue(id-2), wdg->GetValue());
		Redraw(egWdg);
	}
	else if (id == 281) // # env segments
	{
		// FIXME: refresh rate/levels 
		SynthWidget *grp = mainGroup->FindID(200);
		SynthWidget *rwdg, *lwdg, *lbl;
		int ns = (int) wdg->GetValue();
		egWdg->SetSegs(ns);
		int fid = 220;
		int n;
		int e;
		for (n = 0; n < MATMAXSEG; n++)
		{
			e = n < ns;
			rwdg = grp->FindID(fid+1);
			rwdg->SetEnable(e);
			lbl = grp->FindID(fid+2);
			lbl->SetEnable(e);
			lwdg = grp->FindID(fid+3);
			lwdg->SetEnable(e);
			lbl = grp->FindID(fid+4);
			lbl->SetEnable(e);
			lbl = grp->FindID(fid+5);
			lbl->SetEnable(e);
			lbl = grp->FindID(fid+9);
			lbl->SetEnable(e);
			if (e)
				egWdg->SetVal(n, rwdg->GetValue(), lwdg->GetValue());
			fid += 10;
		}
		Redraw(egWdg);
		Redraw(grp);
		//return;
	}

	bsInt16 iip = wdg->GetIP();
	if (iip < 0)
		return;
	if (iip & (1 << 11)) // generator
		iip |= (curGen << 8);
	else if (iip & (1 << 12)) // envelope
		iip |= (curEnv << 8);

	if (wdg->GetType() == wdgSwitch)
		tone->SetParam(iip, wdg->GetState());
	else
		tone->SetParam(iip, wdg->GetValue());
}

void MatrixSynthEdit::SetEnvLvlRange(float rng, int draw)
{
	SynthWidget *grp = mainGroup->FindID(200);
	egWdg->SetLevel(rng);
	if (draw)
		Redraw(egWdg);
	KnobWidget *knb;
	knb = (KnobWidget *)grp->FindID(202);
	knb->SetRange(0, rng, 3);
	if (draw)
		Redraw(knb);
	int fid = 223;
	while (fid <= 273)
	{
		knb = (KnobWidget *)grp->FindID(fid);
		knb->SetRange(0, rng, 3);
		if (draw)
			Redraw(knb);
		fid += 10;
	}
}

void MatrixSynthEdit::SetEnvRtRange(float rng, int draw)
{
	egWdg->SetTime(rng);
	KnobWidget *knb;
	SynthWidget *kgrp = mainGroup->FindID(200);
	int fid = 221;
	while (fid <= 271)
	{
		knb = (KnobWidget *)kgrp->FindID(fid);
		knb->SetRange(0, rng, 3);
		if (draw)
			Redraw(knb);
		fid += 10;
	}
	float incr = rng * 0.2f;
	float lbl = incr;
	SynthWidget *lgrp = mainGroup->FindID(301);
	for (fid = 302; fid <= 306; fid++)
	{
		SynthWidget *txt = lgrp->FindID(fid);
		txt->SetValue(lbl);
		lbl += incr;
	}
	if (draw)
	{
		Redraw(egWdg); // graph
		Redraw(lgrp); // labels
	}
}
