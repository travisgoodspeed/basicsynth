//////////////////////////////////////////////////////////////////////
/// @file Chuffer.cpp Implementation of the Chuffer instrument
// BasicSynth - "Chuffer" a noise based instrument 
//
// Combines a noise generator and filter.
// The noise generator consists of a sampled white noise source
// with variable rate and an optional ring modulator.
// The filter is a two-pole bandpass with variable Q. The filter
// frequency and Q can be varied over the duration of the sound
// linearly or swept with an LFO.
// Amplitude control is a AR envelope and optional LFO tremolo.
//
// The timing rates for the variable frequency, Q, and envelope
// can be set proportional to the note. For example, the center
// frequency can follow the note pitch and the envelope rate
// can follow the note duration.
// 
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Includes.h"
#include "Chuffer.h"

Instrument *Chuffer::ChufferFactory(InstrManager *m, Opaque tmplt)
{
	Chuffer *ip = new Chuffer;
	if (tmplt)
		ip->Copy((Chuffer*)tmplt);
	ip->im = m;
	return ip;
}

SeqEvent *Chuffer::ChufferEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 50;
	return (SeqEvent*)ep;
}

VarParamEvent *Chuffer::AllocParams()
{
	return (VarParamEvent*) ChufferEventFactory(0);
}

static InstrParamMap chuffParams[] = 
{
	{"atk", 16},
	{"champ", 23},
	{"chfrq", 22},
	{"chwt", 24},
	{"cr", 50},
	{"etrkd", 19},
	{"f1", 30}, 
	{"f2", 31}, 
	{"fmul", 33},
	{"frt", 32},
	{"ftrkd", 34},
	{"ftrkf", 36},
	{"ftrkm", 35},
	{"hrate", 20},
	{"modfrq", 21},
	{"q1", 40},
	{"q2", 41},
	{"qmul", 43},
	{"qrt", 42},
	{"qtrkd", 45},
	{"qtrkm", 46},
	{"rel", 17},
	{"suson", 18},
	{"swpamp", 26},
	{"swpfrq", 25},
	{"swpwt", 27}
};

bsInt16 Chuffer::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, chuffParams, sizeof(chuffParams)/sizeof(InstrParamMap));
}

const char *Chuffer::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, chuffParams, sizeof(chuffParams)/sizeof(InstrParamMap));
}

Chuffer::Chuffer()
{
	SetDefaults();
}

Chuffer::Chuffer(Chuffer *tp)
{
	SetDefaults();
	if (tp)
		Copy(tp);
}

Chuffer::~Chuffer()
{
}

void Chuffer::SetDefaults()
{
	im = 0;
	chnl = 0;
	vol = 0;
	frq = 0;
	dur = 0;

	nzSampl = 1.0;

	modOn = 0;
	modFrq = 0;

	swpOn = 0;
	swpFrq = 0;
	swpAmp = 0;
	swpWT = WT_SIN;

	chpOn = 0;
	chpFrq = 0;
	chpAmp = 0;
	chpWT = WT_SIN;

	envTrackDur = 0;
	envSusOn = 1;
	envAtk = 0;
	envRel = 0;

	fltOn = 1;
	fltStart = 1000.0;
	fltEnd = 1000.0;
	fltDur = 0;
	fltMul = 1;
	fltTrackFrq = 0;
	fltTrackDur = 0;
	fltTrackMul = 0;

	resStart = 1;
	resEnd = 1;
	resDur = 0;
	resMul = 1;
	resTrackDur = 0;
	resTrackMul = 0;

	coefRate = 0;
	coefCount = 0;
	lastFc = 0;
	lastQ = 0;
}

void Chuffer::Copy(Chuffer *tp)
{
	im = tp->im;
	chnl = tp->chnl;
	vol = tp->vol;
	frq = tp->frq;
	dur = tp->dur;
	modOn = tp->modOn;
	swpOn = tp->swpOn;
	chpOn = tp->chpOn;
	fltOn = tp->fltOn;

	nzSampl = tp->nzSampl;
	modFrq = tp->modFrq;

	envTrackDur = tp->envTrackDur;
	envSusOn = tp->envSusOn;
	envAtk = tp->envAtk;
	envRel = tp->envRel;

	chpOn = tp->chpOn;
	chpFrq = tp->chpFrq;
	chpAmp = tp->chpAmp;
	chpWT = tp->chpWT;

	fltTrackFrq = tp->fltTrackFrq;
	fltTrackDur = tp->fltTrackDur;
	fltTrackMul = tp->fltTrackMul;
	resTrackDur = tp->resTrackDur;
	resTrackMul = tp->resTrackMul;

	fltStart = tp->fltStart;
	fltEnd = tp->fltEnd;
	fltDur = tp->fltDur;
	fltMul = tp->fltMul;

	resStart = tp->resStart;
	resEnd = tp->resEnd;
	resDur = tp->resDur;
	resMul = tp->resMul;

	swpFrq = tp->swpFrq;
	swpAmp = tp->swpAmp;
	swpWT  = tp->swpWT;

	coefRate = tp->coefRate;
	coefCount = tp->coefCount;
	lastFc = tp->lastFc;
	lastQ = tp->lastQ;
}

void Chuffer::Destroy()
{
	delete this;
}

void Chuffer::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);

	envSig.SetAtkRt(envAtk);
	envSig.SetRelRt(envTrackDur ? dur : envRel);
	envSig.SetSus(vol);
	envSig.SetSusOn(envSusOn);
	envSig.Reset(0);

	FrqValue frt = fltTrackDur ? dur : fltDur;
	AmpValue fst = fltTrackFrq ? frq : fltStart;
	AmpValue fend = fltTrackMul ? fst * fltMul : fltEnd;
	envFlt.InitSeg(frt, fst, fend);

	FrqValue rrt = resTrackDur ? dur : resDur;
	AmpValue rst = resStart;
	AmpValue rend = resTrackMul ? rst * resMul : resEnd;
	envRes.InitSeg(rrt, rst, rend);

	nz.InitH(nzSampl * synthParams.sampleRate);
	chpOsc.InitWT(chpFrq, chpWT);
	modOsc.InitWT(modFrq, WT_SIN);
	swpOsc.InitWT(swpFrq, swpWT);

	filt.CalcCoef(lastFc = fst, lastQ = rst);
	filt.Reset(0);
	coefCount = coefRate;

	fltOn = fst != 0;
	modOn = modFrq != 0;
	chpOn = chpFrq != 0;
	swpOn = swpFrq != 0;
}

void Chuffer::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
		return; // TODO: process controller changes
	SetParams((VarParamEvent*)evt);
	if (chpOn)
		chpOsc.Reset(-1);
	if (modOn)
		modOsc.Reset(-1);
	if (swpOn)
		swpOsc.Reset(-1);
}

void Chuffer::Stop()
{
	envSig.Release();
}

void Chuffer::Tick()
{
	AmpValue fc = envFlt.Gen();
	AmpValue q = envRes.Gen();
	if (swpOn)
		fc += swpAmp * swpOsc.Gen();
	if (--coefCount <= 0)
	{
		if (fc != lastFc || q != lastQ)
		{
			filt.CalcCoef(fc, q);
			lastFc = fc;
			lastQ = q;
		}
		coefCount = coefRate;
	}

	// Note: there are two ways to do this:
	// 1) filter the noise then apply amplitude
	// 2) apply amplitude then filter
	// The difference (if any) is subtle; we do #1 here...
	AmpValue out = nz.Gen();
	if (modOn)
		out *= modOsc.Gen();
	out = filt.Sample(out);
	if (chpOn)
		out *= chpAmp * ((chpOsc.Gen() + 1.0) * 0.5);
	im->Output(chnl, out * envSig.Gen());
}

int  Chuffer::IsFinished()
{
	return envSig.IsFinished();
}

int Chuffer::SetParams(VarParamEvent *params)
{
	int err = 0;

	chnl = params->chnl;
	vol = params->vol;
	if (params->noteonvel > 0)
		vol *= ((float)params->noteonvel / 127.0);
	frq = params->frq;
	dur = params->duration / synthParams.sampleRate;
	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int Chuffer::SetParam(bsInt16 idval, float val)
{
	switch (idval)
	{
	case 16:
		envAtk = FrqValue(val);
		break;
	case 17:
		envRel = FrqValue(val);
		break;
	case 18:
		envSusOn = (bsInt16) val;
		break;
	case 19:
		envTrackDur = (bsInt16) val;
		break;

	case 20:
		nzSampl = FrqValue(val);
		break;
	case 21:
		modFrq = FrqValue(val);
		break;

	case 22:
		chpFrq = FrqValue(val);
		break;
	case 23:
		chpAmp = AmpValue(val);
		break;
	case 24:
		chpWT = (int) val;
		break;

	case 25:
		swpFrq = FrqValue(val);
		break;
	case 26:
		swpAmp = AmpValue(val);
		break;
	case 27:
		swpWT = (int) val;
		break;

	case 30: // f1
		fltStart = AmpValue(val);
		break;
	case 31: // f2
		fltEnd = AmpValue(val);
		break;
	case 32: // f rate
		fltDur = FrqValue(val);
		break;
	case 33: // f %
		fltMul = AmpValue(val);
		break;
	case 34:
		fltTrackDur = (bsInt16) val;
		break;
	case 35:
		fltTrackMul = (bsInt16) val;
		break;
	case 36:
		fltTrackFrq = (bsInt16) val;
		break;

	case 40: // q1
		resStart = AmpValue(val);
		break;
	case 41: // q2
		resEnd = AmpValue(val);
		break;
	case 42: // q rate
		resDur = FrqValue(val);
		break;
	case 43: // q %
		resMul = AmpValue(val);
		break;
	case 44:
		resTrackDur = (bsInt16) val;
		break;
	case 46:
		resTrackMul = (bsInt16) val;
		break;

	case 50:
		coefRate = (bsInt32) val;
		break;

	default:
		return 1;
	}
	return 0;
}

int Chuffer::GetParams(VarParamEvent *params)
{
	params->SetParam(P_FREQ, (float) frq);
	params->SetParam(P_VOLUME, (float) vol);
	params->SetParam(P_DUR, (float) dur);
	params->SetParam(16, (float) envAtk);
	params->SetParam(17, (float) envRel);
	params->SetParam(18, (float) envSusOn);
	params->SetParam(19, (float) envTrackDur);

	params->SetParam(20, (float) nzSampl);
	params->SetParam(21, (float) modFrq);
	params->SetParam(22, (float) chpFrq);
	params->SetParam(23, (float) chpAmp);
	params->SetParam(24, (float) chpWT);
	params->SetParam(25, (float) swpFrq);
	params->SetParam(26, (float) swpAmp);
	params->SetParam(27, (float) swpWT);

	params->SetParam(30, (float) fltStart);
	params->SetParam(31, (float) fltEnd);
	params->SetParam(32, (float) fltDur);
	params->SetParam(33, (float) fltMul);
	params->SetParam(34, (float) fltTrackDur);
	params->SetParam(35, (float) fltTrackMul);
	params->SetParam(36, (float) fltTrackFrq);
	params->SetParam(37, (float) fltOn);

	params->SetParam(40, (float) resStart);
	params->SetParam(41, (float) resEnd);
	params->SetParam(42, (float) resDur);
	params->SetParam(43, (float) resMul);
	params->SetParam(44, (float) resTrackDur);
	params->SetParam(45, (float) resTrackMul);

	params->SetParam(50, (float) coefRate);

	return 0;
}

int Chuffer::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case 16:
		*val = (float) envAtk;
		break;
	case 17:
		*val = (float) envRel;
		break;
	case 18:
		*val = (float) envSusOn;
		break;
	case 19:
		*val = (float) envTrackDur;
		break;

	case 20:
		*val = (float) nzSampl;
		break;
	case 21:
		*val = (float) modFrq;
		break;

	case 22:
		*val = (float) chpFrq;
		break;
	case 23:
		*val = (float) chpAmp;
		break;
	case 24:
		*val = (float) chpWT;
		break;

	case 25:
		*val = (float) swpFrq;
		break;
	case 26:
		*val = (float) swpAmp;
		break;
	case 27:
		*val = (float) swpWT;
		break;

	case 30: // f1
		*val = (float) fltStart;
		break;
	case 31: // f2
		*val = (float) fltEnd;
		break;
	case 32: // f rate
		*val = (float) fltDur;
		break;
	case 33: // f %
		*val = (float) fltMul;
		break;
	case 34:
		*val = (float) fltTrackDur;
		break;
	case 35:
		*val = (float) fltTrackMul;
		break;
	case 36:
		*val = (float) fltTrackFrq;
		break;

	case 40: // q1
		*val = (float) resStart;
		break;
	case 41: // q2
		*val = (float) resEnd;
		break;
	case 42: // q rate
		*val = (float) resDur;
		break;
	case 43: // q %
		*val = (float) resMul;
		break;
	case 44:
		*val = (float) resTrackDur;
		break;
	case 46:
		*val = (float) resTrackMul;
		break;

	case 50:
		*val = (float) coefRate;
		break;

	default:
		return 1;
	}
	return 0;
}


int Chuffer::Load(XmlSynthElem *parent)
{
	float dval;
	short ival;
	long lval;

	XmlSynthElem elem(parent->Document());
	XmlSynthElem *next = parent->FirstChild(&elem);
	while (next != NULL)
	{
		if (elem.TagMatch("sig"))
		{
			if (elem.GetAttribute("atk", dval) == 0)
				envAtk = FrqValue(dval);
			if (elem.GetAttribute("rel", dval) == 0)
				envRel = FrqValue(dval);
			if (elem.GetAttribute("son", ival) == 0)
				envSusOn = ival;
			if (elem.GetAttribute("trkd", ival) == 0)
				envTrackDur = ival;
			if (elem.GetAttribute("hrate", dval) == 0)
				nzSampl = FrqValue(dval);
			if (elem.GetAttribute("mfrq", dval) == 0)
				modFrq = FrqValue(dval);
		}
		else if (elem.TagMatch("flt"))
		{
			if (elem.GetAttribute("f1", dval) == 0)
				fltStart = AmpValue(dval);
			if (elem.GetAttribute("f2", dval) == 0)
				fltEnd = AmpValue(dval);
			if (elem.GetAttribute("rt", dval) == 0)
				fltDur = FrqValue(dval);
			if (elem.GetAttribute("mul", dval) == 0)
				fltMul = dval;
			if (elem.GetAttribute("trkf", ival) == 0)
				fltTrackFrq = ival;
			if (elem.GetAttribute("trkd", ival) == 0)
				fltTrackDur = ival;
			if (elem.GetAttribute("trkm", ival) == 0)
				fltTrackMul = ival;
			if (elem.GetAttribute("cr", lval) == 0)
				coefRate = lval;
		}
		else if (elem.TagMatch("res"))
		{
			if (elem.GetAttribute("q1", dval) == 0)
				resStart = AmpValue(dval);
			if (elem.GetAttribute("q2", dval) == 0)
				resEnd = AmpValue(dval);
			if (elem.GetAttribute("rt", dval) == 0)
				resDur = FrqValue(dval);
			if (elem.GetAttribute("mul", dval) == 0)
				resMul = dval;
			if (elem.GetAttribute("trkd", ival) == 0)
				resTrackDur = ival;
			if (elem.GetAttribute("trkm", ival) == 0)
				resTrackMul = ival;
		}
		else if (elem.TagMatch("chp"))
		{
			if (elem.GetAttribute("frq", dval) == 0)
				chpFrq = FrqValue(dval);
			if (elem.GetAttribute("amp", dval) == 0)
				chpAmp = AmpValue(dval);
			if (elem.GetAttribute("wt", ival) == 0)
				chpWT = ival;
		}
		else if (elem.TagMatch("swp"))
		{
			if (elem.GetAttribute("frq", dval) == 0)
				swpFrq = FrqValue(dval);
			if (elem.GetAttribute("amp", dval) == 0)
				swpAmp = AmpValue(dval);
			if (elem.GetAttribute("wt", ival) == 0)
				swpWT = ival;
		}
		next = elem.NextSibling(&elem);
	}
	return 0;
}

int Chuffer::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem(parent->Document());
	if (!parent->AddChild("sig", &elem))
		return -1;
	elem.SetAttribute("atk", (float)envAtk);
	elem.SetAttribute("rel", (float)envRel);
	elem.SetAttribute("son", (short)envSusOn);
	elem.SetAttribute("trkd", (short)envTrackDur);
	elem.SetAttribute("hrate", (float)nzSampl);
	elem.SetAttribute("mfrq", (float) modFrq);

	if (!parent->AddChild("flt", &elem))
		return -1;
	elem.SetAttribute("f1", (float)fltStart);
	elem.SetAttribute("f2", (float)fltEnd);
	elem.SetAttribute("rt", (float)fltDur);
	elem.SetAttribute("mul", (float)fltMul);
	elem.SetAttribute("trkf", (short)fltTrackFrq);
	elem.SetAttribute("trkd", (short)fltTrackDur);
	elem.SetAttribute("trkm", (short)fltTrackMul);
	elem.SetAttribute("cr", (float)coefRate);

	if (!parent->AddChild("res", &elem))
		return -1;
	elem.SetAttribute("q1", (float)resStart);
	elem.SetAttribute("q2", (float)resEnd);
	elem.SetAttribute("rt", (float)resDur);
	elem.SetAttribute("mul", (float)resMul);
	elem.SetAttribute("trkd", (short)resTrackDur);
	elem.SetAttribute("trkp", (short)resTrackMul);

	if (!parent->AddChild("chp", &elem))
		return -1;
	elem.SetAttribute("frq", (float) chpFrq);
	elem.SetAttribute("amp", (float) chpAmp);
	elem.SetAttribute("wt", (short) chpWT);

	if (!parent->AddChild("swp", &elem))
		return -1;
	elem.SetAttribute("frq", (float) swpFrq);
	elem.SetAttribute("amp", (float) swpAmp);
	elem.SetAttribute("wt", (short) swpWT);
	return 0;
}
