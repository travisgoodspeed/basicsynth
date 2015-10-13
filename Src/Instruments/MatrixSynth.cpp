//////////////////////////////////////////////////////////////////////
/// @file MatrixSynth.cpp Implementation of the MatSynth instrument.
//
// BasicSynth Matrix Synthesis instrument
//
// See _BasicSynth_ Chapter 24 for a full description
//
// The Matrix synth instrument combines 8 oscillators with 8 envelope
// generators in a configurable matrix. Any oscillator can function as
// a signal output and/or modulator. Separate amplitude scaling is
// configured for signal and modulation levels. Any envelope generator
// can be applied to any number of oscillators. Single LFO and pitch bend are
// built-in as well and can be applied individually to each oscillator.
// 
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "MatrixSynth.h"

Instrument *MatrixSynth::MatrixSynthFactory(InstrManager *m, Opaque tmplt)
{
	MatrixSynth *ip;
	if (tmplt)
		ip = new MatrixSynth((MatrixSynth*)tmplt);
	else
		ip = new MatrixSynth;
	if (ip)
		ip->im = m;
	return ip;
}

SeqEvent   *MatrixSynth::MatrixSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 16384;
	return (SeqEvent *) ep;
}

VarParamEvent *MatrixSynth::AllocParams()
{
	return (VarParamEvent *)MatrixSynthEventFactory(0);
}

// name = gen(on)param || env(en)seg(sn)param || frq || vol || lfofrq || lfowt || lfowt || lfoamp
static InstrParamMap genParams[] = 
{
	{ "eg",  6 },  { "fx1", 7 },  { "fx2", 8 },  { "fx3", 9 },  { "fx4", 10 },
	{ "lfo", 11 }, { "mnx", 4 },  { "mod", 1 },  { "mul", 3 },  { "on", 16 },
	{ "out", 0 },  { "pan", 14 }, { "pon", 15 }, 
	{ "sig", 17 }, { "trm", 13 }, { "vib", 12 }, { "vol", 5 },  { "wt",  2 }
};

static InstrParamMap envParams[] = 
{
	{"fix", 5}, {"fixed", 5}, {"level", 3}, {"lvl",     3}, {"rate", 2}, {"rt",   2}, {"son", 1},
	{"st",  0}, {"start", 0}, {"sus",   1}, {"sustain", 1}, {"ty",   4}, {"type", 4}
};

static InstrParamMap globParams[] = 
{
	{ "frq", P_FREQ }, 
	{ "lfoamp", 19 }, { "lfoatk", 18 }, { "lfofrq", 16 }, { "lfowt", 17 },
	{ "matfrq", P_FREQ }, { "matvol", P_VOLUME },
	{ "pba1", 22 },  { "pba2", 23 },  { "pba3", 24 },  
	{ "pbamp", 25 }, { "pbdly", 29 }, { "pbdur", 27 }, { "pbfrq", 27 }, 
	{ "pbmode", 28 }, { "pbr1", 20 },  { "pbr2", 21 },  { "pbwt", 26 },
	{ "vol", P_VOLUME }
};

bsInt16 MatrixSynth::MapParamID(const char *name, Opaque tmplt)
{
	int ty = 0;
	int gn = 0;
	int sn = 0;
	int pn = 0;
	const char *str = name;
	if (*str == 'o' || *str == 'g')
	{
		ty = 1;
		str = InstrParamMap::ParamNum(str+1, &gn);
		if (*str == '.')
			str++;
		pn = InstrParamMap::SearchParamID(str, genParams, sizeof(genParams)/sizeof(InstrParamMap));
	}
	else if (*name == 'e')
	{
		ty = 2;
		str = InstrParamMap::ParamNum(str+1, &gn);
		if (*str == 's')
			str = InstrParamMap::ParamNum(str+1, &sn);
		if (*str == '.')
			str++;
		pn = InstrParamMap::SearchParamID(str, envParams, sizeof(envParams)/sizeof(InstrParamMap));
	}
	else
	{
		return InstrParamMap::SearchParamID(name, globParams, sizeof(globParams)/sizeof(InstrParamMap));
	}
	if (pn >= 0)
		return (ty << 11) + (gn << 8) + (sn << 3) + pn;
	return -1;
}

const char *MatrixSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	static bsString paramNameBuf;

	bsInt16 ty = (id >> 11) & 3;
	if (ty == 0)
		return InstrParamMap::SearchParamName(id, globParams, sizeof(globParams)/sizeof(InstrParamMap));

	paramNameBuf = "";
	char dig[6];
	bsInt16 gn = (id >> 8) & 0xff;
	if (ty == 1)
	{
		dig[0] = 'g';
		InstrParamMap::FormatNum(gn, &dig[1]);
		paramNameBuf += dig;
		paramNameBuf += '.';
		paramNameBuf += InstrParamMap::SearchParamName(id & 0xff, genParams, sizeof(genParams)/sizeof(InstrParamMap));
	}
	else if (ty == 2)
	{
		dig[0] = 'e';
		InstrParamMap::FormatNum(gn, &dig[1]);
		paramNameBuf += dig;
		bsInt16 sn = (id >> 3) & 0x1f;
		bsInt16 vn = id & 7;
		if (vn > 1 && vn < 6)
		{
			dig[0] = 's';
			InstrParamMap::FormatNum(sn, &dig[1]);
			paramNameBuf += dig;
		}
		paramNameBuf += '.';
		paramNameBuf += InstrParamMap::SearchParamName(vn, envParams, sizeof(envParams)/sizeof(InstrParamMap));
	}

	return paramNameBuf;
}

static PhsAccum maxPhs;

MatrixSynth::MatrixSynth()
{
	im = NULL;
	frq = 440.0;
	vol = 1.0;
	chnl = 0;
	maxPhs = synthParams.ftableLength/2;
	lfoOn = 0;
	panOn = 0;
	pbOn = 0;
	pbWTOn = 0;
	fx1On = 0;
	fx2On = 0;
	fx3On = 0;
	fx4On = 0;
	panOn = 0;
	allFlags = 0;
	envUsed = 0;
	for (int n = 0; n < MATGEN; n++)
		envs[n].SetSegs(1);
}

MatrixSynth::MatrixSynth(MatrixSynth *tp)
{
	im = NULL;
	maxPhs = synthParams.ftableLength/2;
	Copy(tp);
}

MatrixSynth::~MatrixSynth()
{
}

void MatrixSynth::Copy(MatrixSynth *tp)
{
	chnl = tp->chnl;
	frq = tp->frq;
	vol = tp->vol;

	lfoOn = tp->lfoOn;
	pbOn  = tp->pbOn;
	pbWTOn = tp->pbWTOn;
	panOn = tp->panOn;
	fx1On = tp->fx1On;
	fx2On = tp->fx2On;
	fx3On = tp->fx3On;
	fx4On = tp->fx4On;
	panOn = tp->panOn;
	allFlags = tp->allFlags;
	envUsed = tp->envUsed;

	for (int n = 0; n < MATGEN; n++)
	{
		gens[n].Copy(&tp->gens[n]);
		envs[n].Copy(&tp->envs[n]);
	}

	lfoGen.Copy(&tp->lfoGen);
	pbGen.Copy(&tp->pbGen);
	pbWT.Copy(&tp->pbWT);
}

void MatrixSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);

	allFlags = 0;
	envUsed = 0;
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		allFlags |= tSig->toneFlags;
		if (tSig->toneFlags & TONE_ON)
		{
			envUsed |= (1 << tSig->envIndex);
			tSig->Start(frq);
		}
		tSig++;
	}
	bsUint16 flgs = envUsed;
	EnvGenSegSus *envPtr = envs;
	EnvGenSegSus *envEnd = &envs[MATGEN];
	while (envPtr < envEnd)
	{
		if (flgs & 1)
		{
			envPtr->SetDuration(evt->duration / synthParams.sampleRate);
			envPtr->Reset(0);
		}
		envPtr++;
		flgs >>= 1;
	}

	lfoOn = (allFlags & (TONE_LFOIN|TONE_TREM)) ? lfoGen.On() : 0;
	pbOn  = (allFlags & TONE_PBIN) ? 1 : 0;
	pbWTOn = pbOn && pbWT.On();
	fx1On = (allFlags & TONE_FX1OUT) ? 1 : 0;
	fx2On = (allFlags & TONE_FX2OUT) ? 1 : 0;
	fx3On = (allFlags & TONE_FX3OUT) ? 1 : 0;
	fx4On = (allFlags & TONE_FX4OUT) ? 1 : 0;
	panOn = (allFlags & TONE_PAN) ? 1 : 0;
	if (lfoOn)
		lfoGen.Reset(0);
	if (pbOn)
		pbGen.Reset(0);
	if (pbWTOn)
	{
		pbWT.SetDurationS(evt->duration);
		pbWT.Reset(0);
	}
}

void MatrixSynth::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
		return; // TODO: process controller changes
	SetParams((VarParamEvent*)evt);
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		if (tSig->toneFlags & TONE_ON)
			tSig->AlterFreq(frq);
		tSig++;
	}
	if (lfoOn)
		lfoGen.Reset(-1);
	if (pbOn)
		pbGen.Reset(-1);
	if (pbWTOn)
		pbWT.Reset(-1);
}

int MatrixSynth::SetParams(VarParamEvent *params)
{
	chnl = params->chnl;
	frq = params->frq;
	vol = params->vol;
	if (params->noteonvel > 0)
		vol *= ((float)params->noteonvel / 127.0);
	pbGen.SetFrequency(frq);
	lfoGen.SetSigFrq(frq);
	pbWT.SetSigFrq(frq);

	bsInt16 *id = params->idParam;
	float *vp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		SetParam(*id++, *vp++);
	return 0;
}

int MatrixSynth::SetParam(bsInt16 idval, float val)
{
	bsInt16 gn, vn, sn, ty;
	// id = 0|xx[3]|vn[8] (generic)
	// id = 1|on[3]|vn[8] (oscillator)
	// id = 2|en[3]|sn[5]|vn[3] (envelope)
	gn = (idval >> 8)  & 7;
	ty = (idval >> 11) & 3;
	if (ty == 0)
	{
		vn = idval & 0xFF;
		switch (vn)
		{
		case 16:
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 17:
			lfoGen.SetWavetable((int) val);
			break;
		case 18:
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 19:
			lfoGen.SetLevel(AmpValue(val));
			break;
		case 20:
			pbGen.SetRate(0, FrqValue(val));
			break;
		case 21:
			pbGen.SetRate(1, FrqValue(val));
			break;
		case 22:
			pbGen.SetAmount(0, FrqValue(val));
			break;
		case 23:
			pbGen.SetAmount(1, FrqValue(val));
			break;
		case 24:
			pbGen.SetAmount(2, FrqValue(val));
			break;
		case 25:
			pbWT.SetLevel(AmpValue(val));
			break;
		case 26:
			pbWT.SetWavetable((int)val);
			break;
		case 27:
			pbWT.SetDuration(FrqValue(val));
			break;
		case 28:
			pbWT.SetMode((int)val);
			break;
		case 29:
			pbWT.SetDelay(FrqValue(val));
			break;
		}
	}
	else if (ty == 1)
	{
		// oscillator
		vn = idval & 0xFF;
		MatrixTone *sig = &gens[gn];
		switch (vn)
		{
		case 0: // output flags
			sig->toneFlags = (sig->toneFlags & TONE_MOD_BITS) | (((bsUint32) val) & TONE_OUT_BITS);
			break;
		case 1: // modulator flags
			sig->toneFlags = (sig->toneFlags & TONE_OUT_BITS) | (((bsUint32) val) << 16);
			break;
		case 2:
			sig->osc.SetWavetable((int)val);
			break;
		case 3:
			sig->frqMult = FrqValue(val);
			break;
		case 4:
			sig->modLvl = AmpValue(val);
			break;
		case 5:
			sig->volLvl = AmpValue(val);
			break;
		case 6:
			sig->envIndex = (bsUint16) val;
			break;
		case 7:
			sig->fx1Lvl = AmpValue(val);
			break;
		case 8:
			sig->fx2Lvl = AmpValue(val);
			break;
		case 9:
			sig->fx3Lvl = AmpValue(val);
			break;
		case 10:
			sig->fx4Lvl = AmpValue(val);
			break;
		case 11:
			sig->lfoLvl = AmpValue(val);
			break;
		case 12: // vibrato
			if (val)
				sig->toneFlags |= TONE_LFOIN;
			else
				sig->toneFlags &= ~TONE_LFOIN;
			break;
		case 13: // tremolo
			if (val)
				sig->toneFlags |= TONE_TREM;
			else
				sig->toneFlags &= ~TONE_TREM;
			break;
		case 14:
			sig->panSet.Set(panTrig, AmpValue(val));
			break;
		case 15: // pan on/off
			if (val)
				sig->toneFlags |= TONE_PAN;
			else
				sig->toneFlags &= ~TONE_PAN;
			break;
		case 16: // oscil on
			if (val)
				sig->toneFlags |= TONE_ON;
			else
				sig->toneFlags &= ~TONE_ON;
			break;
		case 17: // audio out
			if (val)
				sig->toneFlags |= TONE_OUT;
			else
				sig->toneFlags &= ~TONE_OUT;
			break;
		case 18: // Fx1 out
			if (val)
				sig->toneFlags |= TONE_FX1OUT;
			else
				sig->toneFlags &= ~TONE_FX2OUT;
			break;
		case 19: // Fx2 out
			if (val)
				sig->toneFlags |= TONE_FX2OUT;
			else
				sig->toneFlags &= ~TONE_FX2OUT;
			break;
		case 20: // Fx3 out
			if (val)
				sig->toneFlags |= TONE_FX3OUT;
			else
				sig->toneFlags &= ~TONE_FX3OUT;
			break;
		case 21: // Fx4 out
			if (val)
				sig->toneFlags |= TONE_FX4OUT;
			else
				sig->toneFlags &= ~TONE_FX4OUT;
			break;
		case 22: // pitch bend amount
			sig->pbLvl = AmpValue(val);
			break;
		case 23: // pitch bend on/off
			if (val)
				sig->toneFlags |= TONE_PBIN;
			else
				sig->toneFlags &= ~TONE_PBIN;
			break;
		}
	}
	else if (ty == 2)
	{
		// envelope
		EnvGenSegSus *env = &envs[gn];
		sn = (idval >> 3) & 0x1F;
		vn = idval & 7;
		switch (vn)
		{
		case 0:
			env->SetStart(AmpValue(val));
			break;
		case 1:
			env->SetSusOn((int)val);
			break;
		case 2:
			env->SetRate(sn, FrqValue(val));
			break;
		case 3:
			env->SetLevel(sn, AmpValue(val));
			break;
		case 4:
			env->SetType(sn, (EGSegType)(int)val);
			break;
		case 5:
			env->SetFixed(sn, (int)val);
			break;
		case 6:
			env->SetSegs((int)val);
			break;
		}
	}
	return 0;
}

int MatrixSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_CHNL, (float) chnl);
	params->SetParam(P_FREQ, (float) frq);
	params->SetParam(P_VOLUME, (float) vol);

	params->SetParam(16, (float)lfoGen.GetFrequency());
	params->SetParam(17, (float)lfoGen.GetWavetable());
	params->SetParam(18, (float)lfoGen.GetAttack());
	params->SetParam(19, (float)lfoGen.GetLevel());
	params->SetParam(20, (float)pbGen.GetRate(0));
	params->SetParam(21, (float)pbGen.GetRate(1));
	params->SetParam(22, (float)pbGen.GetAmount(0));
	params->SetParam(23, (float)pbGen.GetAmount(1));
	params->SetParam(24, (float)pbGen.GetAmount(2));
	params->SetParam(25, (float)pbWT.GetLevel());
	params->SetParam(26, (float)pbWT.GetWavetable());
	params->SetParam(27, (float)pbWT.GetDuration());
	params->SetParam(28, (float)pbWT.GetMode());
	params->SetParam(29, (float)pbWT.GetDelay());

		// id = 1|on[3]|vn[8] (oscillator)
		// id = 2|en[3]|sn[5]|vn[3] (envelope)

	int idval;
	MatrixTone *sig;
	int ndx;
	for (ndx = 0; ndx < MATGEN; ndx++)
	{
		sig = &gens[ndx];
		idval = (1 << 11) | (ndx << 8);
		params->SetParam(idval+0, (float) (sig->toneFlags & 0xFFFF));
		params->SetParam(idval+1, (float) (sig->toneFlags >> 16));
		params->SetParam(idval+2, (float) sig->osc.GetWavetable());
		params->SetParam(idval+3, (float) sig->frqMult);
		params->SetParam(idval+4, (float) sig->modLvl);
		params->SetParam(idval+5, (float) sig->volLvl);
		params->SetParam(idval+6, (float) sig->envIndex);
		params->SetParam(idval+7, (float) sig->fx1Lvl);
		params->SetParam(idval+8, (float) sig->fx2Lvl);
		params->SetParam(idval+9, (float) sig->fx3Lvl);
		params->SetParam(idval+10, (float) sig->fx4Lvl);
		params->SetParam(idval+11, (float) sig->lfoLvl);
		params->SetParam(idval+12, (float) ((sig->toneFlags & TONE_LFOIN) ? 1 : 0));
		params->SetParam(idval+13, (float) ((sig->toneFlags & TONE_TREM) ? 1 : 0));
		params->SetParam(idval+14, (float) sig->panSet.panval);
		params->SetParam(idval+15, (float) ((sig->toneFlags & TONE_PAN) ? 1 : 0));
		params->SetParam(idval+16, (float) ((sig->toneFlags & TONE_ON) ? 1 : 0));
		params->SetParam(idval+17, (float) ((sig->toneFlags & TONE_OUT) ? 1 : 0));
		params->SetParam(idval+18, (float) ((sig->toneFlags & TONE_FX1OUT) ? 1 : 0));
		params->SetParam(idval+19, (float) ((sig->toneFlags & TONE_FX2OUT) ? 1 : 0));
		params->SetParam(idval+20, (float) ((sig->toneFlags & TONE_FX3OUT) ? 1 : 0));
		params->SetParam(idval+21, (float) ((sig->toneFlags & TONE_FX4OUT) ? 1 : 0));
		params->SetParam(idval+22, (float) sig->pbLvl);
		params->SetParam(idval+23, (float) ((sig->toneFlags & TONE_PBIN) ? 1 : 0));
	}
	for (ndx = 0; ndx < MATGEN; ndx++)
	{
		EnvGenSegSus *env = &envs[ndx];
		idval = (2 << 11) | (ndx << 8);
		params->SetParam(idval,   (float)env->GetStart());
		params->SetParam(idval+1, (float)env->GetSusOn());
		params->SetParam(idval+6, (float)env->GetSegs());
		for (int sn = 0; sn < env->GetSegs(); sn++)
		{
			idval = (2 << 11) | (ndx << 8) | (sn << 3);
			params->SetParam(idval+2, (float)env->GetRate(sn));
			params->SetParam(idval+3, (float)env->GetLevel(sn));
			params->SetParam(idval+4, (float)env->GetType(sn));
			params->SetParam(idval+5, (float)env->GetFixed(sn));
		}
	}
	return 0;
}

int MatrixSynth::GetParam(bsInt16 idval, float *val)
{
	bsInt16 gn, vn, sn, ty;
	// id = 0|xx[3]|vn[8] (generic)
	// id = 1|on[3]|vn[8] (oscillator)
	// id = 2|en[3]|sn[5]|vn[3] (envelope)
	gn = (idval >> 8)  & 7;
	ty = (idval >> 11) & 3;
	if (ty == 0)
	{
		vn = idval & 0xFF;
		switch (vn)
		{
		case 16:
			*val = (float) lfoGen.GetFrequency();
			break;
		case 17:
			*val = (float) lfoGen.GetWavetable();
			break;
		case 18:
			*val = (float) lfoGen.GetAttack();
			break;
		case 19:
			*val = (float) lfoGen.GetLevel();
			break;
		case 20:
			*val = (float) pbGen.GetRate(0);
			break;
		case 21:
			*val = (float) pbGen.GetRate(1);
			break;
		case 22:
			*val = (float) pbGen.GetAmount(0);
			break;
		case 23:
			*val = (float) pbGen.GetAmount(1);
			break;
		case 24:
			*val = (float) pbGen.GetAmount(2);
			break;
		case 25:
			*val = (float) pbWT.GetLevel();
			break;
		case 26:
			*val = (float) pbWT.GetWavetable();
			break;
		case 27:
			*val = (float) pbWT.GetDuration();
			break;
		case 28:
			*val = (float) pbWT.GetMode();
			break;
		case 29:
			*val = (float) pbWT.GetDelay();
			break;
		}
	}
	else if (ty == 1)
	{
		// oscillator
		vn = idval & 0xFF;
		MatrixTone *sig = &gens[gn];
		switch (vn)
		{
		case 0: // output flags
			*val = (float) (sig->toneFlags & TONE_MOD_BITS);
			break;
		case 1: // modulator flags
			*val = (float) (sig->toneFlags & TONE_OUT_BITS);
			break;
		case 2:
			*val = (float) sig->osc.GetWavetable();
			break;
		case 3:
			*val = (float) sig->frqMult;
			break;
		case 4:
			*val = (float) sig->modLvl;
			break;
		case 5:
			*val = (float) sig->volLvl;
			break;
		case 6:
			*val = (float) sig->envIndex;
			break;
		case 7:
			*val = (float) sig->fx1Lvl;
			break;
		case 8:
			*val = (float) sig->fx2Lvl;
			break;
		case 9:
			*val = (float) sig->fx3Lvl;
			break;
		case 10:
			*val = (float) sig->fx4Lvl;
			break;
		case 11:
			*val = (float) sig->lfoLvl;
			break;
		case 12: // vibrato
			*val = (sig->toneFlags & TONE_LFOIN) ? 1.0f : 0.0f;
			break;
		case 13: // tremolo
			*val = (sig->toneFlags & TONE_TREM) ? 1.0f : 0.0f;
			break;
		case 14:
			*val = (float) sig->panSet.panval;
			break;
		case 15: // pan on/off
			*val = (sig->toneFlags & TONE_PAN) ? 1.0f : 0.0f;
			break;
		case 16: // oscil on
			*val = (sig->toneFlags & TONE_ON) ? 1.0f : 0.0f;
			break;
		case 17: // audio out
			*val = (sig->toneFlags & TONE_OUT) ? 1.0f : 0.0f;
			break;
		case 18: // Fx1 out
			*val = (sig->toneFlags & TONE_FX1OUT) ? 1.0f : 0.0f;
			break;
		case 19: // Fx2 out
			*val = (sig->toneFlags & TONE_FX2OUT) ? 1.0f : 0.0f;
			break;
		case 20: // Fx3 out
			*val = (sig->toneFlags & TONE_FX3OUT) ? 1.0f : 0.0f;
			break;
		case 21: // Fx4 out
			*val = (sig->toneFlags & TONE_FX4OUT) ? 1.0f : 0.0f;
			break;
		case 22: // pitch bend amount
			*val = (float) sig->pbLvl;
			break;
		case 23: // pitch bend on/off
			*val = (sig->toneFlags & TONE_PBIN) ? 1.0f : 0.0f;
			break;
		}
	}
	else if (ty == 2)
	{
		// envelope
		EnvGenSegSus *env = &envs[gn];
		sn = (idval >> 3) & 0x1F;
		vn = idval & 7;
		switch (vn)
		{
		case 0:
			*val = (float) env->GetStart();
			break;
		case 1:
			*val = (float) env->GetSusOn();
			break;
		case 2:
			*val = (float) env->GetRate(sn);
			break;
		case 3:
			*val = (float) env->GetLevel(sn);
			break;
		case 4:
			*val = (float) env->GetType(sn);
			break;
		case 5:
			*val = (float) env->GetFixed(sn);
			break;
		case 6:
			*val = (float) env->GetSegs();
			break;
		}
	}
	return 0;
}


void MatrixSynth::Stop()
{
	bsUint16 flg = envUsed;
	EnvGenSegSus *envPtr = envs;
	EnvGenSegSus *envEnd = &envs[MATGEN];
	while (envPtr < envEnd)
	{
		if (flg & 1)
			envPtr->Release();
		flg >>= 1;
		envPtr++;
	}
}

void MatrixSynth::Tick()
{
	bsUint32 flgs;
	AmpValue sigOut = 0;
	AmpValue sigLft = 0;
	AmpValue sigRgt = 0;
	AmpValue lfoRad = 0;
	AmpValue lfoAmp = 0;
	AmpValue pbRad  = 0;
	AmpValue fx1Out = 0;
	AmpValue fx2Out = 0;
	AmpValue fx3Out = 0;
	AmpValue fx4Out = 0;
	AmpValue outVal[MATGEN];
	AmpValue *out = outVal;
	MatrixTone *tMod;
	MatrixTone *tSig;
	MatrixTone *tEnd;
	EnvGenSegSus *envPtr;
	EnvGenSegSus *envEnd;
	AmpValue egVal[MATGEN];
	AmpValue *eg = egVal;
	bsUint16 envFlgs;

	if (lfoOn)
	{
		lfoAmp = lfoGen.Gen();
		lfoRad = lfoAmp * synthParams.frqTI;
	}
	if (pbOn)
		pbRad = pbGen.Gen() * synthParams.frqTI;
	if (pbWTOn)
		pbRad = pbWT.Gen() * synthParams.frqTI;

	// Run the envelope generators
	envFlgs = envUsed;
	envEnd = &envs[MATGEN];
	envPtr = envs; 
	while (envPtr < envEnd)
	{
		if (envFlgs & 1)
			*eg = envPtr->Gen();
		else
			*eg = 0;
		eg++;
		envFlgs >>= 1;
		envPtr++;
	}

	// Collect samples from all active generators,
	// and sum the output signals.
	tEnd = &gens[MATGEN];
	for (tSig = gens; tSig < tEnd; tSig++)
	{
		flgs = tSig->toneFlags;
		if (flgs  & TONE_ON)
		{
			AmpValue sig = tSig->osc.Gen() * egVal[tSig->envIndex];
			if (flgs & TONE_TREM)
				sig += lfoAmp;
			*out = sig;
			if (flgs & TONE_OUT)
			{
				sig *= tSig->volLvl;
				if (flgs & TONE_PAN)
				{
					sigLft += sig * tSig->panSet.panlft;
					sigRgt += sig * tSig->panSet.panrgt;
				}
				else
					sigOut += sig;
				if (flgs & TONE_FX1OUT)
					fx1Out += sig * tSig->fx1Lvl;
				if (flgs & TONE_FX2OUT)
					fx2Out += sig * tSig->fx2Lvl;
				if (flgs & TONE_FX3OUT)
					fx3Out += sig * tSig->fx3Lvl;
				if (flgs & TONE_FX4OUT)
					fx4Out += sig * tSig->fx4Lvl;
			}
		}
		else
			*out = 0; // justin case...
		out++;
	}

	// Apply modulators
	if (allFlags & TONE_MODANY)
	{
		PhsAccum phs;
		bsUint32 mask;
		for (tSig = gens; tSig < tEnd; tSig++)
		{
			flgs = tSig->toneFlags;
			if (flgs & TONE_MODANY)
			{
				if (flgs & TONE_LFOIN)
					phs = lfoRad * tSig->lfoLvl;
				else
					phs = 0;
				if (flgs & TONE_PBIN)
					phs += pbRad * tSig->frqMult;
				out = outVal;
				mask = TONE_MOD1IN;
				for (tMod = gens; tMod < tEnd; tMod++)
				{
					if (flgs & mask)
						phs += *out * tMod->modRad;
					out++;
					mask <<= 1;
				}
				tSig->PhaseModWT(phs);
			}
		}
	}
	if (fx1On)
		im->FxSend(0, fx1Out);
	if (fx2On)
		im->FxSend(1, fx2Out);
	if (fx3On)
		im->FxSend(2, fx3Out);
	if (fx4On)
		im->FxSend(3, fx4Out);

	if (panOn)
		im->Output2(chnl, sigLft * vol, sigRgt * vol);
	im->Output(chnl, sigOut * vol);
}

int  MatrixSynth::IsFinished()
{
	// Test envelope generators on signal outputs...
	MatrixTone *tSig = gens;
	MatrixTone *tEnd = &gens[MATGEN];
	while (tSig < tEnd)
	{
		if ((tSig->toneFlags & (TONE_ON|TONE_OUT)) == (TONE_ON|TONE_OUT))
			if (!envs[tSig->envIndex].IsFinished())
				return 0;
		tSig++;
	}
	return 1;
}

void MatrixSynth::Destroy()
{
	delete this;
}

int MatrixSynth::LoadEnv(XmlSynthElem *elem)
{
	short en = -1;
	if (elem->GetAttribute("en", en) != 0)
		return -1;
	if (en < 0 || en >= MATGEN)
		return -1;

	short segs = 2;
	if (elem->GetAttribute("segs", segs) != 0)
		return -1;
	envs[en].SetSegs(segs);

	short ival;
	float dval;

	dval = 0;
	elem->GetAttribute("st", dval);
	envs[en].SetStart(dval);

	ival = 1;
	elem->GetAttribute("sus", ival);
	envs[en].SetSusOn((int)ival);
	ival = 0;

	XmlSynthElem *elemEG;
	XmlSynthElem *next = elem->FirstChild();
	while ((elemEG = next) != NULL)
	{
		if (elemEG->TagMatch("seg"))
		{
			elemEG->GetAttribute("sn", ival);
			if (ival >= 0 && ival < segs)
			{
				int sn = ival;
				if (elemEG->GetAttribute("rt",  dval) == 0)
					envs[en].SetRate(sn, FrqValue(dval));
				if (elemEG->GetAttribute("lvl",  dval) == 0)
					envs[en].SetLevel(sn, AmpValue(dval));
				if (elemEG->GetAttribute("ty", ival) == 0)
					envs[en].SetType(sn, (EGSegType)ival);
				if (elemEG->GetAttribute("fix", ival) == 0)
					envs[en].SetFixed(sn, (int)ival);
			}
		}
		next = elemEG->NextSibling();
		delete elemEG;
	}

	return 0;
}

int MatrixSynth::SaveEnv(XmlSynthElem *elem, int en)
{
	short nsegs = envs[en].GetSegs();
	elem->SetAttribute("en", (short) en);
	elem->SetAttribute("segs", nsegs);
	elem->SetAttribute("st", envs[en].GetStart());
	elem->SetAttribute("sus", (short)envs[en].GetSusOn());

	XmlSynthElem *elemEG;
	short sn;
	for (sn = 0; sn < nsegs; sn++)
	{
		elemEG = elem->AddChild("seg");
		if (elemEG == NULL)
			return -1;
		elemEG->SetAttribute("sn", sn);
		elemEG->SetAttribute("rt",  envs[en].GetRate(sn));
		elemEG->SetAttribute("lvl",  envs[en].GetLevel(sn));
		elemEG->SetAttribute("ty", (short) envs[en].GetType(sn));
		elemEG->SetAttribute("fix", (short) envs[en].GetFixed(sn));
		delete elemEG;
	}

	return 0;
}

int MatrixSynth::Load(XmlSynthElem *parent)
{
	float dval;
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("mat"))
		{
			if (elem->GetAttribute("frq", dval) == 0)
				frq = FrqValue(dval);
			if (elem->GetAttribute("vol", dval) == 0)
				vol = AmpValue(dval);
		}
		else if (elem->TagMatch("gen"))
		{
			if (elem->GetAttribute("gn",  ival) == 0)
			{
				if (ival >= 0 && ival < MATGEN)
					gens[ival].Load(elem);
			}
		}
		else if (elem->TagMatch("env"))
		{
			LoadEnv(elem);
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		else if (elem->TagMatch("pb"))
		{
			pbGen.Load(elem);
			pbWT.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int MatrixSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem = parent->AddChild("mat");
	if (elem == NULL)
		return -1;
	int err;
	err = elem->SetAttribute("frq", frq);
	err |= elem->SetAttribute("vol", vol);
	delete elem;

	long n;
	for (n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("gen");
		if (elem == NULL)
			return -1;
		err |= elem->SetAttribute("gn", n);
		gens[n].Save(elem);
		delete elem;
	}

	for (n = 0; n < MATGEN; n++)
	{
		elem = parent->AddChild("env");
		if (elem == NULL)
			return -1;
		SaveEnv(elem, n);
		delete elem;
	}

	elem = parent->AddChild("lfo");
	if (elem == NULL)
		return -1;
	err |= lfoGen.Save(elem);
	delete elem;

	elem = parent->AddChild("pb");
	if (elem == NULL)
		return -1;
	err |= pbGen.Save(elem);
	err |= pbWT.Save(elem);
	delete elem;

	return err;
}

/////////////////////////////////////////////////


MatrixTone::MatrixTone()
{
	toneFlags = 0;
	frqMult = 1.0;
	modLvl = 1.0;
	volLvl = 1.0;
	fx1Lvl = 0;
	fx2Lvl = 0;
	fx3Lvl = 0;
	fx4Lvl = 0;
	panSet.Set(panOff, 0);
	lfoLvl = 0;
	pbLvl = 0;
	envIndex = 0;
}

MatrixTone::~MatrixTone()
{
}

void MatrixTone::Copy(MatrixTone *tp)
{
	toneFlags = tp->toneFlags;
	frqMult = tp->frqMult;
	modLvl = tp->modLvl;
	modRad = tp->modRad;
	volLvl = tp->volLvl;
	fx1Lvl = tp->fx1Lvl;
	fx2Lvl = tp->fx2Lvl;
	fx3Lvl = tp->fx3Lvl;
	lfoLvl = tp->lfoLvl;
	pbLvl = tp->pbLvl;
	panSet.panlft = tp->panSet.panlft;
	panSet.panrgt = tp->panSet.panrgt;
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	envIndex = tp->envIndex;
}

inline void MatrixTone::Start(FrqValue frqBase)
{
	FrqValue f = frqBase * frqMult;
	modRad = f * modLvl * synthParams.frqTI;
	//if (modRad > maxPhs)
	//	modRad = maxPhs;
	osc.SetFrequency(f);
	osc.Reset(0);
}

inline void MatrixTone::AlterFreq(FrqValue frqBase)
{
	FrqValue f = frqBase * frqMult;
	modRad = f * modLvl * synthParams.frqTI;
	osc.SetFrequency(f);
	osc.Reset(-1);
}

inline void MatrixTone::PhaseModWT(PhsAccum phs)
{
	osc.PhaseModWT(phs);
}

int MatrixTone::Load(XmlSynthElem *elem)
{
	float dval;
	short ival;
	toneFlags = 0;

	if (elem->GetAttribute("wt",  ival) == 0)
		osc.SetWavetable(ival);
	if (elem->GetAttribute("mul", dval) == 0)
		frqMult = FrqValue(dval);
	if (elem->GetAttribute("mnx", dval) == 0)
		modLvl = AmpValue(dval);
	char *bits = NULL;
	char *bp;
	bsUint32 mask;
	int b;
	if (elem->GetAttribute("out", &bits) == 0)
	{
		bp = bits;
		mask = 1;
		for (b = 0; b < 16 && *bp; b++, bp++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
		}
		delete bits;
	}

	if (elem->GetAttribute("mod", &bits) == 0)
	{
		bp = bits;
		mask = TONE_MOD1IN;
		for (b = 0; b < MATGEN && *bp; b++, bp++)
		{
			if (*bp == '1')
				toneFlags |= mask;
			mask <<= 1;
		}
		delete bits;
	}

	if (elem->GetAttribute("vol", dval) == 0)
		volLvl = AmpValue(dval);
	if (elem->GetAttribute("eg", ival) == 0)
		envIndex = (bsUint16) ival;
	if (elem->GetAttribute("lfo", dval) == 0)
		lfoLvl = AmpValue(dval);
	if (elem->GetAttribute("fx1", dval) == 0)
		fx1Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx2", dval) == 0)
		fx2Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx3", dval) == 0)
		fx3Lvl = AmpValue(dval);
	if (elem->GetAttribute("fx4", dval) == 0)
		fx4Lvl = AmpValue(dval);
	if (elem->GetAttribute("pan", dval) == 0)
		panSet.Set(panTrig, AmpValue(dval));

	return 0;
}

int MatrixTone::Save(XmlSynthElem *elem)
{
	int err;
	err  = elem->SetAttribute("wt",  (short) osc.GetWavetable());
	err |= elem->SetAttribute("mul", frqMult);
	err |= elem->SetAttribute("mnx", modLvl);
	err |= elem->SetAttribute("eg",  (short) envIndex);
	char bits[33];
	bsUint32 mask = 1;
	int b;
	for (b = 0; b < 16; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask <<= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("out", bits);

	mask = TONE_MOD1IN;
	for (b = 0; b < MATGEN; b++)
	{
		bits[b] = toneFlags & mask ? '1' : '0';
		mask <<= 1;
	}
	bits[b] = 0;
	err |= elem->SetAttribute("mod", bits);

	err |= elem->SetAttribute("vol", volLvl);
	err |= elem->SetAttribute("fx1", fx1Lvl);
	err |= elem->SetAttribute("fx2", fx1Lvl);
	err |= elem->SetAttribute("fx3", fx3Lvl);
	err |= elem->SetAttribute("fx4", fx4Lvl);
	err |= elem->SetAttribute("pan", panSet.panval);
	err |= elem->SetAttribute("lfo", lfoLvl);
	err |= elem->SetAttribute("pb", pbLvl);

	return err;
}
