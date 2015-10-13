//////////////////////////////////////////////////////////////////////
/// @file FMSynth.cpp Implementation of the FMSynth instrument
// BasicSynth FM Synthesis instrument
//
// See _BasicSynth_ Chapter 22 for a full explanation
//
// The FM Synth instrument implements a three oscillator FM synthesis
// method. The "algorithms" include:
//   - one modulator, one carrier stack
//   - two modulator, one carrier stack
//   - two modulator, one carrier "Y"
//   - one modulator, two carrier "Delta"
// LFO and pitch bend can be optionally applied to the signal
// A noise generator can be summed with the FM signal
// to produce noisy transient sounds.
// A delay line is available to add resonance.
// Panning can be done internally rather than through the mixer
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "FMSynth.h"

Instrument *FMSynth::FMSynthFactory(InstrManager *m, Opaque tmplt)
{
	FMSynth *ip;
	if (tmplt)
		ip = new FMSynth((FMSynth*)tmplt);
	else
		ip = new FMSynth;
	ip->im = m;
	return ip;
}

SeqEvent *FMSynth::FMSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *vpe = new VarParamEvent;
	vpe->maxParam = 110;
	return (SeqEvent *) vpe;
}

VarParamEvent *FMSynth::AllocParams()
{
	return (VarParamEvent *) FMSynthEventFactory(0);
}

static InstrParamMap fmsynthParams[] =
{
	{ "dlydec",  82 }, { "dlylen",  81 }, { "dlymix",  80 },

	{ "fmalg", 18 },   { "fmdly", 17 },   { "fmmix", 16 },   { "fmpan", 20 },  { "fmpon", 19 },

	{ "gen1atk", 32 }, { "gen1dec", 34 }, { "gen1end", 37 }, { "gen1mul", 30 }, { "gen1pk",  33 },
	{ "gen1rel", 36 }, { "gen1st",  31 }, { "gen1sus", 35 }, { "gen1ty",  38 }, { "gen1wt",  39 },

	{ "gen2atk", 42 }, { "gen2dec", 44 }, { "gen2end", 47 }, { "gen2mul", 40 }, { "gen2pk",  43 },
	{ "gen2rel", 46 }, { "gen2st",  41 }, { "gen2sus", 45 }, { "gen2ty",  48 }, { "gen2wt",  49 },

	{ "gen3atk", 52 }, { "gen3dec", 54 }, { "gen3end", 57 }, { "gen3mul", 50 }, { "gen3pk",  53 },
	{ "gen3rel", 56 }, { "gen3st",  51 }, { "gen3sus", 55 }, { "gen3ty",  58 }, { "gen3wt",  59 },

	{ "lfoamp",  93 }, { "lfoatk",  92 }, { "lfofrq",  90 }, { "lfowt",   91 },

	{ "nzatk",   65 }, { "nzdec",   67 }, { "nzdly",   61 }, { "nzend",   70 }, { "nzfo",    63 },
	{ "nzfr",    62 }, { "nzmix",   60 }, { "nzpk",    66 }, { "nzrel",   69 }, { "nzst",    64 },
	{ "nzsus",   68 }, { "nzty",    71 },

	{ "pba1",   103 }, { "pba2",   104 }, { "pba3",   105 }, { "pbamp",  106 }, 
	{ "pbdly",  109 }, { "pbdur",  108 }, { "pbfrq",  108 }, { "pbmode", 110 }, 
	{ "pbon",   100 }, { "pbr1",   101 }, { "pbr2",   102 }, { "pbwt",   107 }
};

bsInt16 FMSynth::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, fmsynthParams, sizeof(fmsynthParams)/sizeof(InstrParamMap));
}
const char *FMSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, fmsynthParams, sizeof(fmsynthParams)/sizeof(InstrParamMap));
}

FMSynth::FMSynth()
{
	chnl = 0;
	frq = 440.0;
	vol = 1.0;
	gen1EnvDef.Alloc(3, 0, 1);
	gen2EnvDef.Alloc(3, 0, 1);
	gen3EnvDef.Alloc(3, 0, 1);
	nzEnvDef.Alloc(3, 0, 1);
	int i;
	for (i = 0; i < 3; i++)
	{
		gen1EnvDef.Set(i, 0, 0, linSeg);
		gen2EnvDef.Set(i, 0, 0, linSeg);
		gen3EnvDef.Set(i, 0, 0, linSeg);
		nzEnvDef.Set(i, 0, 0, linSeg);
	}
	maxPhs = synthParams.ftableLength / 2;
	gen1Mult = 1.0;
	gen2Mult = 1.0;
	gen3Mult = 2.0;
	gen1Wt = 0;
	gen2Wt = 0;
	gen3Wt = 0;
	fmMix = 1.0;
	fmDly = 0.0;
	nzMix = 0.0;
	nzDly = 0.0;
	nzFrqh = 400.0;
	nzFrqo = 400.0;
	nzOn = 0;
	dlyMix = 0.0;
	dlyOn = 0;
	dlyTim = 0.01;
	dlyDec = 0.1;
	dlySamps = 0;
	panOn  = 0;
	pbOn = 0;
}

FMSynth::FMSynth(FMSynth *tp)
{
	im = 0;
	maxPhs = synthParams.ftableLength / 2;
	Copy(tp);
}

FMSynth::~FMSynth()
{
	gen1EnvDef.Clear();
	gen2EnvDef.Clear();
	gen3EnvDef.Clear();
	nzEnvDef.Clear();
}

void FMSynth::Copy(FMSynth *ip)
{
	gen1Wt = ip->gen1Wt;
	gen1Mult = ip->gen1Mult;
	gen1EnvDef.Copy(&ip->gen1EnvDef);
	fmMix = ip->fmMix;
	fmDly = ip->fmDly;
	algorithm = ip->algorithm;

	gen2Wt = ip->gen2Wt;
	gen2Mult = ip->gen2Mult;
	gen2EnvDef.Copy(&ip->gen2EnvDef);

	gen3Wt = ip->gen3Wt;
	gen3Mult = ip->gen3Mult;
	gen3EnvDef.Copy(&ip->gen3EnvDef);

	nzEnvDef.Copy(&ip->nzEnvDef);
	nzMix = ip->nzMix;
	nzFrqh = ip->nzFrqh;
	nzFrqo = ip->nzFrqo;
	nzOn = ip->nzOn;
	nzDly = ip->nzDly;

	dlyTim = ip->dlyTim;
	dlyDec = ip->dlyDec;
	dlyMix = ip->dlyMix;
	dlySamps = ip->dlySamps;
	dlyOn = ip->dlyOn;

	lfoGen.Copy(&ip->lfoGen);
	pbGen.Copy(&ip->pbGen);
	pbOn = ip->pbOn;
	pbWT.Copy(&ip->pbWT);
	panSet.Set(panTrig, ip->panSet.panval);
	panOn = ip->panOn;

	chnl = ip->chnl;
	frq = ip->frq;
	vol = ip->vol;
}

AmpValue FMSynth::CalcPhaseMod(AmpValue amp, FrqValue mult)
{
	amp = (amp * mult) * synthParams.frqTI;
	if (amp > maxPhs)
		amp = maxPhs;
	return amp;
}

void FMSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	FrqValue mul1 = gen2Mult * frq;
	FrqValue mul2 = gen3Mult * frq;
	gen1Osc.InitWT(frq*gen1Mult, gen1Wt);
	gen2Osc.InitWT(mul1, gen2Wt);
	gen3Osc.InitWT(mul2, gen3Wt);
	gen1EG.SetEnvDef(&gen1EnvDef);
	gen1EG.Reset(0);
	if (algorithm != ALG_DELTA)
	{
		gen2EG.InitADSR(CalcPhaseMod(gen2EnvDef.start, mul1),
			gen2EnvDef.GetRate(0), CalcPhaseMod(gen2EnvDef.GetLevel(0),  mul1),
			gen2EnvDef.GetRate(1), CalcPhaseMod(gen2EnvDef.GetLevel(1),  mul1),
			gen2EnvDef.GetRate(2), CalcPhaseMod(gen2EnvDef.GetLevel(2),  mul1),
			gen2EnvDef.GetType(0));
	}
	else
	{
		// double carrier
		gen2EG.SetEnvDef(&gen2EnvDef);
		gen2EG.Reset(0);
	}
	if (algorithm != ALG_STACK)
	{
		gen3EG.InitADSR(CalcPhaseMod(gen3EnvDef.start, mul2),
			gen3EnvDef.GetRate(0), CalcPhaseMod(gen3EnvDef.GetLevel(0),  mul2),
			gen3EnvDef.GetRate(1), CalcPhaseMod(gen3EnvDef.GetLevel(1),  mul2),
			gen3EnvDef.GetRate(2), CalcPhaseMod(gen3EnvDef.GetLevel(2),  mul2),
			gen3EnvDef.GetType(0));
	}

	nzOn = nzMix > 0;
	if (nzOn)
	{
		nzi.InitH(nzFrqh * synthParams.sampleRate);
		nzo.InitWT(nzFrqo, WT_SIN);
		nzEG.SetEnvDef(&nzEnvDef);
		nzEG.Reset(0);
	}
	dlyOn = dlyMix > 0 && (fmDly > 0 || nzDly > 0);
	if (dlyOn)
		apd.InitDLR(dlyTim, dlyDec, 0.001);
	if (lfoGen.On())
	{
		lfoGen.SetSigFrq(frq);
		lfoGen.Reset();
	}
	if (pbOn)
	{
		pbGen.SetFrequency(frq);
		pbGen.Reset();
	}
	if (pbWT.On())
	{
		pbWT.SetDurationS(evt->duration);
		pbWT.Reset();
	}
}

void FMSynth::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
		return; // TODO: process controller changes
	SetParams((VarParamEvent*)evt);
	// The only changeable things are the oscillators, i.e. pitch
	// and signal/noise/delay mixture.
	// Envelope rates and levels are not reset while playing.
	// changing the 'algorithm' while playing is an "interseting" idea...
	// most likely will produce unpredictable behavior.
	gen1Osc.SetWavetable(gen1Wt);
	gen2Osc.SetWavetable(gen2Wt);
	gen3Osc.SetWavetable(gen3Wt);
	gen1Osc.SetFrequency(frq*gen1Mult);
	gen2Osc.SetFrequency(frq*gen2Mult);
	gen3Osc.SetFrequency(frq*gen3Mult);
	gen1Osc.Reset(-1);
	gen2Osc.Reset(-1);
	gen3Osc.Reset(-1);
	lfoGen.Reset(-1);
	pbWT.Reset(-1);
	if (nzOn)
	{
		nzi.Reset(-1);
		nzo.Reset(-1);
	}
}

int FMSynth::SetParams(VarParamEvent *evt)
{
	int err = 0;
	vol = evt->vol;
	if (evt->noteonvel > 0)
		vol *= ((float)evt->noteonvel / 127.0);
	frq = evt->frq;
	chnl = evt->chnl;
	lfoGen.SetSigFrq(frq);
	pbWT.SetSigFrq(frq);

	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	int n = evt->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int FMSynth::SetParam(bsInt16 id, float val)
{
	EGSegType segType;
	switch (id)
	{
	case 16: //mix
		fmMix = val;
		break;
	case 17: //dly
		fmDly = val;
		break;
	case 18: //alg
		algorithm = (bsInt16) val;
		break;
	case 19:
		panOn = (bsInt16) val;
		break;
	case 20:
		panSet.Set(panTrig, AmpValue(val));
		break;
	case 30: //mul
		gen1Mult = val;
		break;
	case 31: //st
		gen1EnvDef.SetStart(AmpValue(val));
		break;
	case 32: //atk
		gen1EnvDef.SetRate(0, FrqValue(val));
		break;
	case 33: //pk
		gen1EnvDef.SetLevel(0, AmpValue(val));
		break;
	case 34: //dec
		gen1EnvDef.SetRate(1, FrqValue(val));
		break;
	case 35: //sus
		gen1EnvDef.SetLevel(1, AmpValue(val));
		break;
	case 36: //rel
		gen1EnvDef.SetRate(2, FrqValue(val));
		break;
	case 37: //end
		gen1EnvDef.SetLevel(2, AmpValue(val));
		break;
	case 38: //ty
		segType = (EGSegType)(int)val;
		gen1EnvDef.SetType(0, segType);
		gen1EnvDef.SetType(1, segType);
		gen1EnvDef.SetType(2, segType);
		break;
	case 39: // wt
		gen1Wt = (int) val;
		break;
	//gen2:
	case 40: //mul
		gen2Mult = val;
		break;
	case 41: //st
		gen2EnvDef.SetStart(AmpValue(val));
		break;
	case 42: //atk
		gen2EnvDef.SetRate(0, FrqValue(val));
		break;
	case 43: //pk
		gen2EnvDef.SetLevel(0, AmpValue(val));
		break;
	case 44: //dec
		gen2EnvDef.SetRate(1, FrqValue(val));
		break;
	case 45: //sus
		gen2EnvDef.SetLevel(1, AmpValue(val));
		break;
	case 46: //rel
		gen2EnvDef.SetRate(2, FrqValue(val));
		break;
	case 47: //end
		gen2EnvDef.SetLevel(2, AmpValue(val));
		break;
	case 48: //ty
		segType = (EGSegType)(int)val;
		gen2EnvDef.SetType(0, segType);
		gen2EnvDef.SetType(1, segType);
		gen2EnvDef.SetType(2, segType);
		break;
	case 49: // wt
		gen2Wt = (int) val;
		break;
	// gen3:
	case 50: //mul
		gen3Mult = val;
		break;
	case 51: //st
		gen3EnvDef.SetStart(AmpValue(val));
		break;
	case 52: //atk
		gen3EnvDef.SetRate(0, FrqValue(val));
		break;
	case 53: //pk
		gen3EnvDef.SetLevel(0, AmpValue(val));
		break;
	case 54: //dec
		gen3EnvDef.SetRate(1, FrqValue(val));
		break;
	case 55: //sus
		gen3EnvDef.SetLevel(1, AmpValue(val));
		break;
	case 56: //rel
		gen3EnvDef.SetRate(2, FrqValue(val));
		break;
	case 57: //end
		gen3EnvDef.SetLevel(2, AmpValue(val));
		break;
	case 58: //ty
		segType = (EGSegType)(int)val;
		gen3EnvDef.SetType(0, segType);
		gen3EnvDef.SetType(1, segType);
		gen3EnvDef.SetType(2, segType);
		break;
	case 59: // wt
		gen3Wt = (int) val;
		break;
	//nz:
	case 60: //mix
		nzMix = val;
		break;
	case 61: //dly
		nzDly = val;
		break;
	case 62: //nz frq
		nzFrqh = FrqValue(val);
		break;
	case 63: //osc frq
		nzFrqo = FrqValue(val);
		break;
	case 64: //st
		nzEnvDef.SetStart(AmpValue(val));
		break;
	case 65: //atk
		nzEnvDef.SetRate(0, FrqValue(val));
		break;
	case 66: //pk
		nzEnvDef.SetLevel(0, AmpValue(val));
		break;
	case 67: //dec
		nzEnvDef.SetRate(1, FrqValue(val));
		break;
	case 68: //sus
		nzEnvDef.SetLevel(1, AmpValue(val));
		break;
	case 69: //rel
		nzEnvDef.SetRate(2, FrqValue(val));
		break;
	case 70: //end
		nzEnvDef.SetLevel(2, AmpValue(val));
		break;
	case 71: //ty
		segType = (EGSegType)(int)val;
		nzEnvDef.SetType(0, segType);
		nzEnvDef.SetType(1, segType);
		nzEnvDef.SetType(2, segType);
		break;
	//dlyn:
	case 80: //mix
		dlyMix = val;
		break;
	case 81: //dly
		dlyTim = val;
		break;
	case 82: //dec
		dlyDec = val;
		break;
	//lfo:
	case 90: //frq
		lfoGen.SetFrequency(FrqValue(val));
		break;
	case 91: //wt
		lfoGen.SetWavetable((int)val);
		break;
	case 92: //rt
		lfoGen.SetAttack(FrqValue(val));
		break;
	case 93: //amp
		lfoGen.SetLevel(AmpValue(val));
		break;
	// pitchbend
	case 100:
		pbOn = (int) val;
		break;
	case 101:
		pbGen.SetRate(0, FrqValue(val));
		break;
	case 102:
		pbGen.SetRate(1, FrqValue(val));
		break;
	case 103:
		pbGen.SetAmount(0, FrqValue(val));
		break;
	case 104:
		pbGen.SetAmount(1, FrqValue(val));
		break;
	case 105:
		pbGen.SetAmount(2, FrqValue(val));
		break;
	case 106:
		pbWT.SetLevel(AmpValue(val));
		break;
	case 107:
		pbWT.SetWavetable((int)val);
		break;
	case 108:
		pbWT.SetDuration(FrqValue(val));
		break;
	case 109:
		pbWT.SetDelay(FrqValue(val));
		break;
	case 110:
		pbWT.SetMode((int)val);
		break;
	default:
		return 1;
	}
	return 0;
}

int FMSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_CHNL, chnl);
	params->SetParam(P_VOLUME, vol);
	params->SetParam(P_FREQ, frq);
	params->SetParam(16, (float) fmMix);
	params->SetParam(17, (float) fmDly);
	params->SetParam(18, (float) algorithm);
	params->SetParam(19, (float) panOn);
	params->SetParam(20, (float) panSet.panval);
	params->SetParam(30, (float) gen1Mult);
	params->SetParam(31, (float) gen1EnvDef.GetStart());
	params->SetParam(32, (float) gen1EnvDef.GetRate(0));
	params->SetParam(33, (float) gen1EnvDef.GetLevel(0));
	params->SetParam(34, (float) gen1EnvDef.GetRate(1));
	params->SetParam(35, (float) gen1EnvDef.GetLevel(1));
	params->SetParam(36, (float) gen1EnvDef.GetRate(2));
	params->SetParam(37, (float) gen1EnvDef.GetLevel(2));
	params->SetParam(38, (float) gen1EnvDef.GetType(0));
	params->SetParam(39, (float) gen1Wt);
	params->SetParam(40, (float) gen2Mult);
	params->SetParam(41, (float) gen2EnvDef.GetStart());
	params->SetParam(42, (float) gen2EnvDef.GetRate(0));
	params->SetParam(43, (float) gen2EnvDef.GetLevel(0));
	params->SetParam(44, (float) gen2EnvDef.GetRate(1));
	params->SetParam(45, (float) gen2EnvDef.GetLevel(1));
	params->SetParam(46, (float) gen2EnvDef.GetRate(2));
	params->SetParam(47, (float) gen2EnvDef.GetLevel(2));
	params->SetParam(48, (float) gen2EnvDef.GetType(0));
	params->SetParam(49, (float) gen2Wt);
	params->SetParam(50, (float) gen3Mult);
	params->SetParam(51, (float) gen3EnvDef.GetStart());
	params->SetParam(52, (float) gen3EnvDef.GetRate(0));
	params->SetParam(53, (float) gen3EnvDef.GetLevel(0));
	params->SetParam(54, (float) gen3EnvDef.GetRate(1));
	params->SetParam(55, (float) gen3EnvDef.GetLevel(1));
	params->SetParam(56, (float) gen3EnvDef.GetRate(2));
	params->SetParam(57, (float) gen3EnvDef.GetLevel(2));
	params->SetParam(58, (float) gen3EnvDef.GetType(0));
	params->SetParam(59, (float) gen3Wt);
	params->SetParam(60, (float) nzMix);
	params->SetParam(61, (float) nzDly);
	params->SetParam(62, (float) nzFrqh);
	params->SetParam(63, (float) nzFrqo);
	params->SetParam(64, (float) nzEnvDef.GetStart());
	params->SetParam(65, (float) nzEnvDef.GetRate(0));
	params->SetParam(66, (float) nzEnvDef.GetLevel(0));
	params->SetParam(67, (float) nzEnvDef.GetRate(1));
	params->SetParam(68, (float) nzEnvDef.GetLevel(1));
	params->SetParam(69, (float) nzEnvDef.GetRate(2));
	params->SetParam(70, (float) nzEnvDef.GetLevel(2));
	params->SetParam(71, (float) nzEnvDef.GetType(0));
	params->SetParam(80, (float) dlyMix);
	params->SetParam(81, (float) dlyTim);
	params->SetParam(82, (float) dlyDec);
	params->SetParam(90, (float) lfoGen.GetFrequency());
	params->SetParam(91, (float) lfoGen.GetWavetable());
	params->SetParam(92, (float) lfoGen.GetAttack());
	params->SetParam(93, (float) lfoGen.GetLevel());
	params->SetParam(100, (float) pbOn);
	params->SetParam(101, (float) pbGen.GetRate(0));
	params->SetParam(102, (float) pbGen.GetRate(1));
	params->SetParam(103, (float) pbGen.GetAmount(0));
	params->SetParam(104, (float) pbGen.GetAmount(1));
	params->SetParam(105, (float) pbGen.GetAmount(2));
	params->SetParam(106, (float) pbWT.GetLevel());
	params->SetParam(107, (float) pbWT.GetWavetable());
	params->SetParam(108, (float) pbWT.GetDuration());
	params->SetParam(109, (float) pbWT.GetDelay());
	params->SetParam(110, (float) pbWT.GetMode());

	return 0;
}

int FMSynth::GetParam(bsInt16 id, float *val)
{
//	EGSegType segType;
	switch (id)
	{
	case 16: //mix
		*val = (float) fmMix;
		break;
	case 17: //dly
		*val = (float) fmDly;
		break;
	case 18: //alg
		*val = (float) algorithm;
		break;
	case 19:
		*val = (float) panOn;
		break;
	case 20:
		*val = (float) panSet.panval;
		break;
	case 30: //mul
		*val = (float) gen1Mult;
		break;
	case 31: //st
		*val = (float) gen1EnvDef.GetStart();
		break;
	case 32: //atk
		*val = (float) gen1EnvDef.GetRate(0);
		break;
	case 33: //pk
		*val = (float) gen1EnvDef.GetLevel(0);
		break;
	case 34: //dec
		*val = (float) gen1EnvDef.GetRate(1);
		break;
	case 35: //sus
		*val = (float) gen1EnvDef.GetLevel(1);
		break;
	case 36: //rel
		*val = (float) gen1EnvDef.GetRate(2);
		break;
	case 37: //end
		*val = (float) gen1EnvDef.GetLevel(2);
		break;
	case 38: //ty
		*val = (float) (int) gen1EnvDef.GetType(0);
		break;
	case 39: // wt
		*val = (float) gen1Wt;
		break;
	//gen2:
	case 40: //mul
		*val = (float) gen2Mult;
		break;
	case 41: //st
		*val = (float) gen2EnvDef.GetStart();
		break;
	case 42: //atk
		*val = (float) gen2EnvDef.GetRate(0);
		break;
	case 43: //pk
		*val = (float) gen2EnvDef.GetLevel(0);
		break;
	case 44: //dec
		*val = (float) gen2EnvDef.GetRate(1);
		break;
	case 45: //sus
		*val = (float) gen2EnvDef.GetLevel(1);
		break;
	case 46: //rel
		*val = (float) gen2EnvDef.GetRate(2);
		break;
	case 47: //end
		*val = (float) gen2EnvDef.GetLevel(2);
		break;
	case 48: //ty
		*val = (float) (int) gen2EnvDef.GetType(0);
		break;
	case 49: // wt
		*val = (float) gen2Wt;
		break;
	// gen3:
	case 50: //mul
		*val = (float) gen3Mult;
		break;
	case 51: //st
		*val = (float) gen3EnvDef.GetStart();
		break;
	case 52: //atk
		*val = (float) gen3EnvDef.GetRate(0);
		break;
	case 53: //pk
		*val = (float) gen3EnvDef.GetLevel(0);
		break;
	case 54: //dec
		*val = (float) gen3EnvDef.GetRate(1);
		break;
	case 55: //sus
		*val = (float) gen3EnvDef.GetLevel(1);
		break;
	case 56: //rel
		*val = (float) gen3EnvDef.GetRate(2);
		break;
	case 57: //end
		*val = (float) gen3EnvDef.GetLevel(2);
		break;
	case 58: //ty
		*val = (float) (int) gen2EnvDef.GetType(0);
		break;
	case 59: // wt
		*val = (float) gen3Wt;
		break;
	//nz:
	case 60: //mix
		*val = (float) nzMix;
		break;
	case 61: //dly
		*val = (float) nzDly;
		break;
	case 62: //nz frq
		*val = (float) nzFrqh;
		break;
	case 63: //osc frq
		*val = (float) nzFrqo;
		break;
	case 64: //st
		*val = (float) nzEnvDef.GetStart();
		break;
	case 65: //atk
		*val = (float) nzEnvDef.GetRate(0);
		break;
	case 66: //pk
		*val = (float) nzEnvDef.GetLevel(0);
		break;
	case 67: //dec
		*val = (float) nzEnvDef.GetRate(1);
		break;
	case 68: //sus
		*val = (float) nzEnvDef.GetLevel(1);
		break;
	case 69: //rel
		*val = (float) nzEnvDef.GetRate(2);
		break;
	case 70: //end
		*val = (float) nzEnvDef.GetLevel(2);
		break;
	case 71: //ty
		*val = (float) (int) nzEnvDef.GetType(0);
		break;
	//dlyn:
	case 80: //mix
		*val = (float) dlyMix;
		break;
	case 81: //dly
		*val = (float) dlyTim;
		break;
	case 82: //dec
		*val = (float) dlyDec;
		break;
	//lfo:
	case 90: //frq
		*val = (float) lfoGen.GetFrequency();
		break;
	case 91: //wt
		*val = (float) lfoGen.GetWavetable();
		break;
	case 92: //rt
		*val = (float) lfoGen.GetAttack();
		break;
	case 93: //amp
		*val = (float) lfoGen.GetLevel();
		break;
	// pitchbend
	case 100:
		*val = (float) pbOn;
		break;
	case 101:
		*val = (float) pbGen.GetRate(0);
		break;
	case 102:
		*val = (float) pbGen.GetRate(1);
		break;
	case 103:
		*val = (float) pbGen.GetAmount(0);
		break;
	case 104:
		*val = (float) pbGen.GetAmount(1);
		break;
	case 105:
		*val = (float) pbGen.GetAmount(2);
		break;
	case 106:
		*val = (float) pbWT.GetLevel();
		break;
	case 107:
		*val = (float) pbWT.GetWavetable();
		break;
	case 108:
		*val = (float) pbWT.GetDuration();
		break;
	case 109:
		*val = (float) pbWT.GetDelay();
		break;
	case 110:
		*val = (float) pbWT.GetMode();
		break;
	default:
		return 1;
	}
	return 0;
}

void FMSynth::Stop()
{
	gen1EG.Release();
	gen2EG.Release();
	gen3EG.Release();
	if (nzOn)
		nzEG.Release();
}


int FMSynth::IsFinished()
{
	if (gen1EG.IsFinished())
	{
		if (!dlyOn || --dlySamps <= 0)
			return 1;
	}
	return 0;
}

void FMSynth::Tick()
{
	AmpValue sigOut;
	AmpValue gen1Out;
	AmpValue gen2Out;
	AmpValue gen3Out;
	AmpValue nzOut = 0;
	AmpValue dlyOut;
	AmpValue lfoOut = 0;
	AmpValue gen1Mod;
	AmpValue gen2Mod;
	AmpValue gen3Mod;

	if (lfoGen.On())
		lfoOut = lfoGen.Gen() * synthParams.frqTI;
	if (pbOn)
		lfoOut += pbGen.Gen() * synthParams.frqTI;
	if (pbWT.On())
		lfoOut += pbWT.Gen() * synthParams.frqTI;
	gen3Mod = lfoOut * gen3Mult;
	gen2Mod = lfoOut * gen2Mult;
	gen1Mod = lfoOut * gen1Mult;

	gen1Out = gen1Osc.Gen() * gen1EG.Gen();
	gen2Out = gen2Osc.Gen() * gen2EG.Gen();
	gen3Out = gen3Osc.Gen() * gen3EG.Gen();
	switch (algorithm)
	{
	case ALG_STACK2:
		gen2Mod += gen3Out;
		// fallthrough
	case ALG_STACK:
		gen1Mod += gen2Out;
		break;
	case ALG_WYE:
		gen1Mod += gen2Out + gen3Out;
		break;
	case ALG_DELTA:
		gen1Mod += gen3Out;
		gen2Mod += gen3Out;
		gen1Out += gen2Out;
		break;
	}
	gen1Osc.PhaseModWT(gen1Mod);
	gen2Osc.PhaseModWT(gen2Mod);
	gen3Osc.PhaseModWT(gen3Mod);

	sigOut = gen1Out * fmMix;

	if (nzOn)
	{
		nzOut = nzi.Gen() * nzEG.Gen();
		if (nzFrqo)
			nzOut *= nzo.Gen();
		sigOut += nzOut * nzMix;
	}

	if (dlyOn)
	{
		AmpValue dlyIn = gen1Out * fmDly;
		dlyIn += nzOut * nzDly;
		dlyOut = apd.Sample(dlyIn);
		sigOut += dlyOut * dlyMix;
	}

	sigOut *= vol;
	if (panOn)
		im->Output2(chnl, sigOut * panSet.panlft, sigOut * panSet.panrgt);
	else
		im->Output(chnl, sigOut);
}

void FMSynth::Destroy()
{
	delete this;
}

void FMSynth::LoadEG(XmlSynthElem *elem, EnvDef& eg)
{
	float rt = 0;
	float lvl = 0;
	short typ = 0;
	short fix = 1;

	elem->GetAttribute("st", lvl);
	elem->GetAttribute("ty", typ);
	eg.start = lvl;
	elem->GetAttribute("atk", rt);
	elem->GetAttribute("pk",  lvl);
	eg.Set(0, rt, lvl, (EGSegType)typ, fix);
	elem->GetAttribute("dec", rt);
	elem->GetAttribute("sus", lvl);
	eg.Set(1, rt, lvl, (EGSegType)typ, fix);
	elem->GetAttribute("rel", rt);
	elem->GetAttribute("end", lvl);
	eg.Set(2, rt, lvl, (EGSegType)typ, fix);
}

int FMSynth::Load(XmlSynthElem *parent)
{
	float dval;
	short ival;

	XmlSynthElem node(parent->Document());
	XmlSynthElem *elem = parent->FirstChild(&node);
	while (elem != NULL)
	{
		if (elem->TagMatch("fm"))
		{
			if (elem->GetAttribute("alg", ival) == 0)
				algorithm = (bsInt16) ival;
			if (elem->GetAttribute("mix", dval) == 0)
				fmMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				fmDly = dval;
			if (elem->GetAttribute("pon", ival) == 0)
				panOn = ival;
			if (elem->GetAttribute("pan", dval) == 0)
				panSet.Set(panTrig, AmpValue(dval));
		}
		if (elem->TagMatch("gen1"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen1Mult = dval;
			if (elem->GetAttribute("wt", ival) == 0)
				gen1Wt = ival;
			LoadEG(elem, gen1EnvDef);
		}
		else if (elem->TagMatch("gen2"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen2Mult = dval;
			if (elem->GetAttribute("wt", ival) == 0)
				gen2Wt = ival;
			LoadEG(elem, gen2EnvDef);
		}
		else if (elem->TagMatch("gen3"))
		{
			if (elem->GetAttribute("mul", dval) == 0)
				gen3Mult = dval;
			if (elem->GetAttribute("wt", ival) == 0)
				gen3Wt = ival;
			LoadEG(elem, gen3EnvDef);
		}
		else if (elem->TagMatch("nz"))
		{
			if (elem->GetAttribute("mix", dval) == 0)
				nzMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				nzDly = dval;
			if (elem->GetAttribute("fr", dval) == 0)
				nzFrqh = dval / synthParams.sampleRate;
			if (elem->GetAttribute("fh", dval) == 0)
				nzFrqh = dval;
			if (elem->GetAttribute("fo", dval) == 0)
				nzFrqo = dval;
			LoadEG(elem, nzEnvDef);
		}
		else if (elem->TagMatch("dln"))
		{
			if (elem->GetAttribute("mix", dval) == 0)
				dlyMix = dval;
			if (elem->GetAttribute("dly", dval) == 0)
				dlyTim = dval;
			if (elem->GetAttribute("dec", dval) == 0)
				dlyDec = dval;
			dlySamps = (long) (dlyDec * synthParams.sampleRate);
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
		elem = elem->NextSibling(&node);
	}
	return 0;
}

XmlSynthElem *FMSynth::SaveEG(XmlSynthElem *parent, const char *tag, EnvDef& eg)
{
	XmlSynthElem *elem = parent->AddChild(tag);
	if (elem != NULL)
	{
		elem->SetAttribute("st",  eg.start);
		elem->SetAttribute("atk", eg.segs[0].rate);
		elem->SetAttribute("pk",  eg.segs[0].level);
		elem->SetAttribute("dec", eg.segs[1].rate);
		elem->SetAttribute("sus", eg.segs[1].level);
		elem->SetAttribute("rel", eg.segs[2].rate);
		elem->SetAttribute("end", eg.segs[2].level);
		elem->SetAttribute("ty", (short) eg.segs[0].type);
	}
	return elem;
}

int FMSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;

	elem = parent->AddChild("fm");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", fmMix);
	elem->SetAttribute("dly", fmDly);
	elem->SetAttribute("alg", (short) algorithm);
	elem->SetAttribute("pon", (short)panOn);
	elem->SetAttribute("pan", panSet.panval);
	delete elem;

	elem = SaveEG(parent, "gen1", gen1EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen1Mult);
	elem->SetAttribute("wt",  (short)gen1Wt);
	delete elem;

	elem = SaveEG(parent, "gen2", gen2EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen2Mult);
	elem->SetAttribute("wt",  (short)gen2Wt);
	delete elem;

	elem = SaveEG(parent, "gen3", gen3EnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mul", gen3Mult);
	elem->SetAttribute("wt",  (short)gen3Wt);
	delete elem;

	elem = SaveEG(parent, "nz", nzEnvDef);
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", nzMix);
	elem->SetAttribute("dly", nzDly);
	elem->SetAttribute("fh", nzFrqh);
	elem->SetAttribute("fo", nzFrqo);
	delete elem;

	elem = parent->AddChild("dln");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("mix", dlyMix);
	elem->SetAttribute("dly", dlyTim);
	elem->SetAttribute("dec", dlyDec);
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

