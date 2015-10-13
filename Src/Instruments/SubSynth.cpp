//////////////////////////////////////////////////////////////////////
/// @file SubSynth.cpp Implementation of the SubSynth instrument.
//
// BasicSynth Subtractive synthesis instrument
//
// See _BasicSynth_ Chapter 21 for a full explanation
//
// This instrument contains an Oscillator, Noise source, filter,
// and envelope generators for amplitude and filter frequency.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "SubSynth.h"

Instrument *SubSynth::SubSynthFactory(InstrManager *m, Opaque tmplt)
{
	SubSynth *ip;
	if (tmplt)
		ip = new SubSynth((SubSynth*)tmplt);
	else
		ip = new SubSynth;
	if (ip)
		ip->im = m;
	return ip;
}

SeqEvent *SubSynth::SubSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 55;
	return (SeqEvent *) ep;
}

VarParamEvent *SubSynth::AllocParams()
{
	return (VarParamEvent *) SubSynthEventFactory(0);
}

static InstrParamMap subSynthParams[] = 
{
	{"crt", 53},
	{"egfatk", 31}, {"egfdec", 33}, {"egfend", 36}, {"egfpk", 32},
	{"egfrel", 35}, {"egfst", 30},  {"egfsus", 34}, {"egfty", 37},
	{"egsatk", 22}, {"egsdec", 24}, {"egsend", 27}, {"egspk", 23},
	{"egsrel", 26}, {"egsst", 21},  {"egssus", 25}, {"egsty", 28},
	{"lfoamp", 43}, {"lfoatk", 42}, {"lfofrq", 40}, {"lfowt", 41},
	{"oscfg", 19},  {"oscfr", 20},  {"oscfrq", 5 }, {"oscft", 18},
	{"oscmix", 16}, {"oscvol", 6},  {"oscwt", 17},
	{"pbamp", 50},  {"pba1", 47},   {"pba2", 48},   {"pba3", 49},
	{"pbdly", 54},  {"pbdur", 52},  {"pbfrq", 52},  {"pbmode", 55},
	{"pbon", 44},   {"pbr1", 45},   {"pbr2", 46},   {"pbwt", 51}
};

bsInt16 SubSynth::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, subSynthParams, sizeof(subSynthParams)/sizeof(InstrParamMap));
}

const char *SubSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, subSynthParams, sizeof(subSynthParams)/sizeof(InstrParamMap));
}

SubSynth::SubSynth()
{
	im = NULL;
	vol = 1.0;
	chnl = 0;
	fltGain = 1.0;
	fltRes = 0.5;
	fltType = 4;
	filt = 0;
	sigMix = 1.0;
	nzMix = 0.0;
	nzOn = 0;
	pbOn = 0;
	coefRate = 0;
}

SubSynth::SubSynth(SubSynth *tp)
{
	im = NULL;
	filt = NULL;
	Copy(tp);
}

SubSynth::~SubSynth()
{
	delete filt;
}

void SubSynth::Copy(SubSynth *tp)
{
	vol = tp->vol;
	chnl = tp->chnl;
	osc.SetWavetable(tp->osc.GetWavetable());
	sigMix = tp->sigMix;
	nzMix  = 1.0 - sigMix;
	fltGain = tp->fltGain;
	fltRes = tp->fltRes;
	fltType = tp->fltType;
	envSig.Copy(&tp->envSig);
	envFlt.Copy(&tp->envFlt);
	CreateFilter();
	lfoGen.Copy(&tp->lfoGen);
	nzOn = nzMix > 0;
	pbOn = tp->pbOn;
	pbGen.Copy(&tp->pbGen);
	pbWT.Copy(&tp->pbWT);
	coefRate = tp->coefRate;
}

void SubSynth::CreateFilter()
{
	delete filt;
	switch (fltType)
	{
	case 0:
		filt = new SubFiltLP;
		break;
	case 1:
		filt = new SubFiltHP;
		break;
	case 2:
		filt = new SubFiltBP;
		break;
	case 3:
		filt = new SubFiltRES;
		break;
	case 4:
		filt = new SubFiltLPR;
		break;
	default:
		filt = new SubFilt;
		break;
	}
	filt->Init(&envFlt, fltGain, fltRes);
	filt->SetCalcRate(coefRate);
}

void SubSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);
	osc.Reset(0);
	envSig.Reset(0);
	envFlt.Reset(0);
	if (!filt)
		CreateFilter();
	filt->Init(&envFlt, fltGain, fltRes);
	filt->Reset(0);
	if (lfoGen.On())
		lfoGen.Reset(0);
	if (pbOn)
		pbGen.Reset(0);
	if (pbWT.On())
	{
		pbWT.SetDurationS(evt->duration);
		pbWT.Reset(0);
	}
	pwFrq = (frq * synthParams.GetCentsMult((int)im->GetPitchbendC(chnl))) - frq;
}

void SubSynth::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
	{
		ControlEvent *cevt = (ControlEvent *)evt;
		if ((cevt->mmsg & MIDI_EVTMSK) == MIDI_PWCHG)
			pwFrq = (frq * synthParams.GetCentsMult((int)im->GetPitchbendC(chnl))) - frq;
		return; // TODO: process controller changes
	}
	SetParams((VarParamEvent *)evt);
	osc.Reset(-1);
	filt->Reset(-1);
	if (lfoGen.On())
		lfoGen.Reset(-1);
	if (pbOn)
		pbGen.Reset(-1);
	if (pbWT.On())
		pbWT.Reset(-1);
}

int SubSynth::SetParams(VarParamEvent *evt)
{
	int err = 0;
	chnl = evt->chnl;
	vol = evt->vol;
	frq = evt->frq;
	osc.SetFrequency(frq);
	pbGen.SetFrequency(frq);
	lfoGen.SetSigFrq(frq);
	pbWT.SetSigFrq(frq);
	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	int n;
	for (n = evt->numParam; n > 0; n--)
		err += SetParam(*id++, *valp++);
	if (filt)
		filt->SetCalcRate(coefRate);
	return err;
}

int SubSynth::SetParam(bsInt16 id, float val)
{
	short ft;
	switch (id)
	{
	case 16: //	Sets the mixture of oscillator output and noise output.
		sigMix = val;
		nzMix  = 1.0 - sigMix;
		nzOn = nzMix > 0;
		break;
	case 17: //Wave table index.
		osc.SetWavetable((int) val);
		break;
	case 18: // Filter type
		ft = (short) val;
		if (ft != fltType)
		{
			fltType = ft;
			CreateFilter();
		}
		break;
	case 19: //Filter gain
		fltGain = AmpValue(val);
		break;
	case 20:
		fltRes = AmpValue(val);
		break;
	case 21: //Oscillator envelope start value.
		envSig.SetStart(AmpValue(val));
		break;
	case 22: //Oscillator envelope attack rate
		envSig.SetAtkRt(FrqValue(val));
		break;
	case 23: //Oscillator envelope peak level
		envSig.SetAtkLvl(AmpValue(val));
		break;
	case 24: //	Oscillator envelope decay rate
		envSig.SetDecRt(FrqValue(val));
		break;
	case 25: // Oscillator envelope sustain level
		envSig.SetSusLvl(AmpValue(val));
		break;
	case 26:
		envSig.SetRelRt(FrqValue(val));
		break;
	case 27: //	Oscillator envelope release level
		envSig.SetRelLvl(AmpValue(val));
		break;
	case 28: //Oscillator envelope curve type
		envSig.SetType((EGSegType) (int) val);
		break;
	case 30: //Filter envelope start value.
		envFlt.SetStart(AmpValue(val));
		break;
	case 31: //Filter envelope attack rate
		envFlt.SetAtkRt(FrqValue(val));
		break;
	case 32: //Filter envelope peak level
		envFlt.SetAtkLvl(AmpValue(val));
		break;
	case 33: //Filter envelope decay rate
		envFlt.SetDecRt(FrqValue(val));
		break;
	case 34: //Filter envelope sustain level
		envFlt.SetSusLvl(AmpValue(val));
		break;
	case 35: //Filter envelope release rate
		envFlt.SetRelRt(FrqValue(val));
		break;
	case 36: //Filter envelope final level
		envFlt.SetRelLvl(AmpValue(val));
		break;
	case 37: //Filter envelope curve type
		envFlt.SetType((EGSegType) (int) val);
		break;
	case 40: //LFO Frequency
		lfoGen.SetFrequency(FrqValue(val));
		break;
	case 41: //LFO wavetable index
		lfoGen.SetWavetable((int)val);
		break;
	case 42: //LFO envelope attack rate
		lfoGen.SetAttack(FrqValue(val));
		break;
	case 43: //LFO level
		lfoGen.SetLevel(AmpValue(val));
		break;
	case 44: // PB On
		pbOn = (int) val;
		break;
	case 45:
		pbGen.SetRate(0, FrqValue(val));
		break;
	case 46:
		pbGen.SetRate(1, FrqValue(val));
		break;
	case 47:
		pbGen.SetAmount(0, FrqValue(val));
		break;
	case 48:
		pbGen.SetAmount(1, FrqValue(val));
		break;
	case 49:
		pbGen.SetAmount(2, FrqValue(val));
		break;
	case 50:
		pbWT.SetLevel(AmpValue(val));
		break;
	case 51:
		pbWT.SetWavetable((int)val);
		break;
	case 52:
		pbWT.SetDuration(FrqValue(val));
		break;
	case 53:
		coefRate = val;
		break;
	case 54:
		pbWT.SetDelay(FrqValue(val));
		break;
	case 55:
		pbWT.SetMode((int)val);
		break;
	default:
		return 1;
	}
	return 0;
}

int SubSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_CHNL, (float) chnl);
	params->SetParam(P_VOLUME, (float) vol);
	params->SetParam(P_FREQ, (float) osc.GetFrequency());
	params->SetParam(16, (float) sigMix);
	params->SetParam(17, (float) osc.GetWavetable());
	params->SetParam(18, (float) fltType);
	params->SetParam(19, (float) fltGain);
	params->SetParam(20, (float) fltRes);
	params->SetParam(21, (float) envSig.GetStart());
	params->SetParam(22, (float) envSig.GetAtkRt());
	params->SetParam(23, (float) envSig.GetAtkLvl());
	params->SetParam(24, (float) envSig.GetDecRt());
	params->SetParam(25, (float) envSig.GetSusLvl());
	params->SetParam(26, (float) envSig.GetRelRt());
	params->SetParam(27, (float) envSig.GetRelLvl());
	params->SetParam(28, (float) envSig.GetType());
	params->SetParam(30, (float) envFlt.GetStart());
	params->SetParam(31, (float) envFlt.GetAtkRt());
	params->SetParam(32, (float) envFlt.GetAtkLvl());
	params->SetParam(33, (float) envFlt.GetDecRt());
	params->SetParam(34, (float) envFlt.GetSusLvl());
	params->SetParam(35, (float) envFlt.GetRelRt());
	params->SetParam(36, (float) envFlt.GetRelLvl());
	params->SetParam(37, (float) envFlt.GetType());
	params->SetParam(40, (float) lfoGen.GetFrequency());
	params->SetParam(41, (float) lfoGen.GetWavetable());
	params->SetParam(42, (float) lfoGen.GetAttack());
	params->SetParam(43, (float) lfoGen.GetLevel());
	params->SetParam(44, (float) pbOn);
	params->SetParam(45, (float) pbGen.GetRate(0));
	params->SetParam(46, (float) pbGen.GetRate(1));
	params->SetParam(47, (float) pbGen.GetAmount(0));
	params->SetParam(48, (float) pbGen.GetAmount(1));
	params->SetParam(49, (float) pbGen.GetAmount(2));
	params->SetParam(50, (float) pbWT.GetLevel());
	params->SetParam(51, (float) pbWT.GetWavetable());
	params->SetParam(52, (float) pbWT.GetDuration());
	params->SetParam(53, coefRate);
	params->SetParam(54, (float) pbWT.GetDelay());
	params->SetParam(55, (float) pbWT.GetMode());
	return 0;
}

int SubSynth::GetParam(bsInt16 idval, float *val)
{
	switch(idval)
	{
	case 16: *val = (float) sigMix; break;
	case 17: *val = (float) osc.GetWavetable(); break;
	case 18: *val = (float) fltType; break;
	case 19: *val = (float) fltGain; break;
	case 20: *val = (float) fltRes; break;
	case 21: *val = (float) envSig.GetStart(); break;
	case 22: *val = (float) envSig.GetAtkRt(); break;
	case 23: *val = (float) envSig.GetAtkLvl(); break;
	case 24: *val = (float) envSig.GetDecRt(); break;
	case 25: *val = (float) envSig.GetSusLvl(); break;
	case 26: *val = (float) envSig.GetRelRt(); break;
	case 27: *val = (float) envSig.GetRelLvl(); break;
	case 28: *val = (float) envSig.GetType(); break;
	case 30: *val = (float) envFlt.GetStart(); break;
	case 31: *val = (float) envFlt.GetAtkRt(); break;
	case 32: *val = (float) envFlt.GetAtkLvl(); break;
	case 33: *val = (float) envFlt.GetDecRt(); break;
	case 34: *val = (float) envFlt.GetSusLvl(); break;
	case 35: *val = (float) envFlt.GetRelRt(); break;
	case 36: *val = (float) envFlt.GetRelLvl(); break;
	case 37: *val = (float) envFlt.GetType(); break;
	case 40: *val = (float) lfoGen.GetFrequency(); break;
	case 41: *val = (float) lfoGen.GetWavetable(); break;
	case 42: *val = (float) lfoGen.GetAttack(); break;
	case 43: *val = (float) lfoGen.GetLevel(); break;
	case 44: *val = (float) pbOn; break;
	case 45: *val = (float) pbGen.GetRate(0); break;
	case 46: *val = (float) pbGen.GetRate(1); break;
	case 47: *val = (float) pbGen.GetAmount(0); break;
	case 48: *val = (float) pbGen.GetAmount(1); break;
	case 49: *val = (float) pbGen.GetAmount(2); break;
	case 50: *val = (float) pbWT.GetLevel(); break;
	case 51: *val = (float) pbWT.GetWavetable(); break;
	case 52: *val = (float) pbWT.GetDuration(); break;
	case 53: *val = (float) coefRate; break;
	case 54: *val = (float) pbWT.GetDelay(); break;
	case 55: *val = (float) pbWT.GetMode(); break;
	default:
		return 1;
	}
	return 0;
}

void SubSynth::Stop()
{
	envSig.Release();
	envFlt.Release();
}

void SubSynth::Tick()
{
	FrqValue phs = pwFrq;
	if (lfoGen.On())
		phs += lfoGen.Gen();
	if (pbOn)
		phs += pbGen.Gen();
	if (pbWT.On())
		phs += pbWT.Gen();
	osc.PhaseModWT(phs * synthParams.frqTI);
	AmpValue sigVal = osc.Gen();
	if (nzOn)
		sigVal = (sigVal * sigMix) + (nz.Gen() * nzMix);
	im->Output(chnl, filt->Sample(sigVal) * envSig.Gen() * vol);
}

int  SubSynth::IsFinished()
{
	return envSig.IsFinished();
}

void SubSynth::Destroy()
{
	delete this;
}

int SubSynth::Load(XmlSynthElem *parent)
{
	float dvals[7];
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
		{
			if (elem->GetAttribute("frq", dvals[0]) == 0)
				osc.SetFrequency(FrqValue(dvals[0]));
			if (elem->GetAttribute("wt", ival) == 0)
				osc.SetWavetable(ival);
			if (elem->GetAttribute("vol", dvals[0]) == 0)
				vol = AmpValue(dvals[0]);
			if (elem->GetAttribute("mix", dvals[0]) == 0)
				sigMix = AmpValue(dvals[0]);
			if (elem->GetAttribute("fg",  dvals[0]) == 0)
				fltGain = AmpValue(dvals[0]);
			if (elem->GetAttribute("ft",  ival) == 0)
				fltType = ival;
			if (elem->GetAttribute("fr",  dvals[0]) == 0)
				fltRes = AmpValue(dvals[0]);

			nzMix  = 1.0 - sigMix;
			nzOn = nzMix > 0;
			CreateFilter();
		}
		else if (elem->TagMatch("egs"))
		{
			elem->GetAttribute("st",  dvals[0]);
			elem->GetAttribute("atk", dvals[1]);
			elem->GetAttribute("pk",  dvals[2]);
			elem->GetAttribute("dec", dvals[3]);
			elem->GetAttribute("sus", dvals[4]);
			elem->GetAttribute("rel", dvals[5]);
			elem->GetAttribute("end", dvals[6]);
			elem->GetAttribute("ty", ival);
			envSig.InitADSR(AmpValue(dvals[0]), 
				FrqValue(dvals[1]), AmpValue(dvals[2]),
				FrqValue(dvals[3]), AmpValue(dvals[4]),
				FrqValue(dvals[5]), AmpValue(dvals[6]),
				(EGSegType)ival);
		}
		else if (elem->TagMatch("egf"))
		{
			elem->GetAttribute("st",  dvals[0]);
			elem->GetAttribute("atk", dvals[1]);
			elem->GetAttribute("pk",  dvals[2]);
			elem->GetAttribute("dec", dvals[3]);
			elem->GetAttribute("sus", dvals[4]);
			elem->GetAttribute("rel", dvals[5]);
			elem->GetAttribute("end", dvals[6]);
			elem->GetAttribute("ty", ival);
			envFlt.InitADSR(AmpValue(dvals[0]), 
				FrqValue(dvals[1]), AmpValue(dvals[2]),
				FrqValue(dvals[3]), AmpValue(dvals[4]),
				FrqValue(dvals[5]), AmpValue(dvals[6]),
				(EGSegType)ival);
			elem->GetAttribute("cr", coefRate);
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		else if (elem->TagMatch("pb"))
		{
			if (elem->GetAttribute("on", ival) == 0)
				pbOn = (int) ival;
			pbGen.Load(elem);
			pbWT.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int SubSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("osc");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("frq",  osc.GetFrequency());
	elem->SetAttribute("wt",  (short) osc.GetWavetable());
	elem->SetAttribute("vol",  vol);
	elem->SetAttribute("mix",  sigMix);
	elem->SetAttribute("fg",   fltGain);
	elem->SetAttribute("ft",   fltType);
	elem->SetAttribute("fr",   fltRes);
	delete elem;

	elem = parent->AddChild("egs");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st",  envSig.GetStart());
	elem->SetAttribute("atk", envSig.GetAtkRt());
	elem->SetAttribute("pk",  envSig.GetAtkLvl());
	elem->SetAttribute("dec", envSig.GetDecRt());
	elem->SetAttribute("sus", envSig.GetSusLvl());
	elem->SetAttribute("rel", envSig.GetRelRt());
	elem->SetAttribute("end", envSig.GetRelLvl());
	elem->SetAttribute("ty",  (short) envSig.GetType());
	delete elem;

	elem = parent->AddChild("egf");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("st",  envFlt.GetStart());
	elem->SetAttribute("atk", envFlt.GetAtkRt());
	elem->SetAttribute("pk",  envFlt.GetAtkLvl());
	elem->SetAttribute("dec", envFlt.GetDecRt());
	elem->SetAttribute("sus", envFlt.GetSusLvl());
	elem->SetAttribute("rel", envFlt.GetRelRt());
	elem->SetAttribute("end", envFlt.GetRelLvl());
	elem->SetAttribute("ty",  (short) envFlt.GetType());
	elem->SetAttribute("cr", coefRate);
	delete elem;

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	elem = parent->AddChild("pb");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("on", (short) pbOn);
	pbGen.Save(elem);
	pbWT.Save(elem);
	delete elem;

	return 0;
}

