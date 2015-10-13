//////////////////////////////////////////////////////////////////////
/// @file BuzzSynth.cpp Implementation of the BuzzSynth instrument.
//
// BasicSynth Subtractive synthesis instrument based on BUZZ generator
//
// This instrument contains two bandwidth-limited pulse wave oscillators,
// each with its own filter, modulation envelope, amplitude envelope, 
// LFO and pitch bend.
// The modulation envelope can be applied to the pulse width and filter.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "BuzzSynth.h"

Instrument *BuzzSynth::InstrFactory(InstrManager *m, Opaque tmplt)
{
	BuzzSynth *ip;
	if (tmplt)
		ip = new BuzzSynth((BuzzSynth*)tmplt);
	else
		ip = new BuzzSynth;
	if (ip)
		ip->im = m;
	return ip;
}

SeqEvent *BuzzSynth::EventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 0x2ff;
	return (SeqEvent *) ep;
}

VarParamEvent *BuzzSynth::AllocParams()
{
	return (VarParamEvent *) EventFactory(0);
}

static InstrParamMap BuzzSynthParams[] = 
{
//	{"frq", 0}, 

	{"lfoamp", 19}, 
	{"lfoatk", 18}, 
	{"lfofrq", 16}, 
	{"lfowt", 17},
	
	{"pbamp", 20}, 
	{"pbdly", 23},
	{"pbdur", 22},
	{"pbmode", 24},
	{"pbwt", 21},
};

static InstrParamMap BuzzSynthPartParams[] = 
{
	{"amp", 2},
	{"ascl", 3 },

	{"egml0", 21}, 
	{"egml1", 23}, 
	{"egml2", 26}, 
	{"egmon", 27}, 
	{"egmr0", 20}, 
	{"egmr1", 22}, 
	{"egmr2", 24}, 
	{"egmr3", 25}, 
	{"egmty", 28}, 

	{"egvl0", 11}, 
	{"egvl1", 13}, 
	{"egvl2", 16}, 
	{"egvon", 17}, 
	{"egvr0", 10}, 
	{"egvr1", 12}, 
	{"egvr2", 14}, 
	{"egvr3", 15}, 
	{"egvty", 18}, 

	{"fc", 5}, 
	{"fcmod", 33},
	{"fcon", 31},
	{"fcrel", 32},
	{"fcscl", 6},

	{"fr", 9},
	{"fscl", 1},

	{"harm", 8},
	{"hmod", 34},
	{"hr", 9},

	{"mod", 30 },
	{"on", 35 },

	{"q", 7 },

	{"volume", 0}
};

// gen(1|2).param || param
bsInt16 BuzzSynth::MapParamID(const char *name, Opaque tmplt)
{
	int gn = 0;
	int vn = 0;
	const char *str = name;
	if (*str == 'g')
	{
		str = InstrParamMap::ParamNum(str+1, &gn);
		if (*str == '.')
			str++;
		vn = InstrParamMap::SearchParamID(str, BuzzSynthPartParams, sizeof(BuzzSynthPartParams)/sizeof(InstrParamMap));
		if (gn > 0 && gn <= BUZZ_NGEN && vn >= 0)
			return (gn << 8) + vn;
		return -1;
	}
	return InstrParamMap::SearchParamID(str, BuzzSynthParams, sizeof(BuzzSynthParams)/sizeof(InstrParamMap));
}

const char *BuzzSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	static bsString paramNameBuf;

	int gn = id >> 8;
	if (gn == 0)
		return InstrParamMap::SearchParamName(id, BuzzSynthParams, sizeof(BuzzSynthParams)/sizeof(InstrParamMap));
	paramNameBuf = "";
	if (gn > 0 && gn <= BUZZ_NGEN)
	{
		paramNameBuf = "gen";
		paramNameBuf += (char) (gn + '0');
		paramNameBuf += '.';
		paramNameBuf += InstrParamMap::SearchParamName(id & 0xff, BuzzSynthPartParams, sizeof(BuzzSynthPartParams)/sizeof(InstrParamMap));
	}
	return paramNameBuf;
}

BuzzSynth::BuzzSynth()
{
	im = NULL;
	vol = 1.0;
	chnl = 0;
	modOn = 0;
	frq = 440.0;
	pwFrq = 0;
}

BuzzSynth::BuzzSynth(BuzzSynth *tp)
{
	im = NULL;
	Copy(tp);
}

BuzzSynth::~BuzzSynth()
{
}

void BuzzSynth::Copy(BuzzSynth *tp)
{
	vol = tp->vol;
	chnl = tp->chnl;
	buzz[0].Copy(&tp->buzz[0]);
	buzz[1].Copy(&tp->buzz[1]);
	lfoGen.Copy(&tp->lfoGen);
	pbWT.Copy(&tp->pbWT);
}

void BuzzSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent *)evt);
	buzz[0].Start();
	buzz[1].Start();
	modOn = 0;
	if (lfoGen.On())
	{
		modOn |= 1;
		lfoGen.Reset(0);
	}
	if (pbWT.On())
	{
		modOn |= 2;
		pbWT.SetDurationS(evt->duration);
		pbWT.Reset(0);
	}
	pwFrq = (frq * synthParams.GetCentsMult((int)im->GetPitchbendC(chnl))) - frq;
}

void BuzzSynth::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
	{
		ControlEvent *cevt = (ControlEvent *)evt;
		if ((cevt->mmsg & MIDI_EVTMSK) == MIDI_PWCHG)
			pwFrq = (frq * synthParams.GetCentsMult((int)im->GetPitchbendC(chnl))) - frq;
		return; // TODO: process controller changes
	}
	SetParams((VarParamEvent *)evt);
	buzz[0].Reset();
	buzz[1].Reset();
	if (modOn & 1)
		lfoGen.Reset(-1);
	if (modOn & 2)
		pbWT.Reset(-1);
}


void BuzzSynth::Stop()
{
	buzz[0].Stop();
	buzz[1].Stop();
}

void BuzzSynth::Tick()
{
	FrqValue f = pwFrq;
	if (modOn || f != 0.0)
	{
		if (modOn & 1)
			f += lfoGen.Gen();
		if (modOn & 2)
			f += pbWT.Gen();
		buzz[0].Modulate(f);
		buzz[1].Modulate(f);
	}

	im->Output(chnl, (buzz[0].Gen() + buzz[1].Gen()) * vol);
}

int  BuzzSynth::IsFinished()
{
	return buzz[0].IsFinished() && buzz[1].IsFinished();
}

void BuzzSynth::Destroy()
{
	delete this;
}

int BuzzSynth::SetParams(VarParamEvent *evt)
{
	int err = 0;
	chnl = evt->chnl;
	vol = evt->vol;
	if (evt->noteonvel > 0)
		vol *= ((float)evt->noteonvel / 127.0);
	frq = evt->frq;
	buzz[0].frqBase = frq;
	buzz[1].frqBase = frq;
	lfoGen.SetSigFrq(frq);
	pbWT.SetSigFrq(frq);

	bsInt16 *id = evt->idParam;
	float *valp = evt->valParam;
	for (int n = evt->numParam; n > 0; n--)
		err += SetParam(*id++, *valp++);
	return err;
}

int BuzzSynth::SetParam(bsInt16 id, float val)
{
	EGSegType et;
	int segn;
	short err = 0;
	short ndx = (id >> 8) & 0xff;
	if (ndx == 0)
	{
		switch (id)
		{
		case 16: //LFO Frequency
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 17: //LFO wavetable index
			lfoGen.SetWavetable((int)val);
			break;
		case 18: //LFO envelope attack rate
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 19: //LFO level
			lfoGen.SetLevel(AmpValue(val));
			break;
		case 20:
			pbWT.SetLevel(AmpValue(val));
			break;
		case 21:
			pbWT.SetWavetable((int)val);
			break;
		case 22:
			pbWT.SetDuration(FrqValue(val));
			break;
		case 23:
			pbWT.SetDelay(FrqValue(val));
			break;
		case 24:
			pbWT.SetMode((int)val);
			break;
		default:
			err = 1;
			break;
		}
	}
	else if (ndx > 0 && ndx <= BUZZ_NGEN)
	{
		BuzzPart *bz = &buzz[ndx-1];
		switch (id & 0xff)
		{
		case 0: //	Oscillator volume
			bz->volume = AmpValue(val);
			break;
		case 1: //	Oscillator frequency multiplier
			bz->frqScl = FrqValue(val);
			break;
		case 2: // Harmonics base
			bz->ampBase = AmpValue(val);
			break;
		case 3: // Harmonics multiplier
			bz->ampScl = AmpValue(val);
			break;
		case 5: // Filter base (cents above fundamental frequency)
			bz->fltBase = FrqValue(val);
			break;
		case 6: //Filter envelope scale
			bz->fltScl = FrqValue(val);
			break;
		case 7: // Filter 'Q'
			bz->fltQ = AmpValue(val);
			break;
		case 8: // Harmonics frequency ratio
			bz->frqRatio = FrqValue(val);
			break;
		case 9:
			bz->harmMax = (bsInt32)val;
			break;

		case 10: // Amplitude envelope attack rate 1
			bz->envSig.SetRate(0, FrqValue(val));
			break;
		case 11:
			bz->envSig.SetLevel(0, AmpValue(val));
			break;
		case 12: // Amplitude envelope attack rate 2
			bz->envSig.SetRate(1, FrqValue(val));
			break;
		case 13:
			bz->envSig.SetLevel(1, AmpValue(val));
			break;
		case 14: // Amplitude envelope attack rate 3
			bz->envSig.SetRate(2, FrqValue(val));
			break;
		case 15: // Amplitude  envelope sustain level
			bz->envSig.SetLevel(2, AmpValue(val));
			break;
		case 16: // Amplitude envelope release rate
			bz->envSig.SetRate(3, FrqValue(val));
			break;
		case 17: // Amplitude envelope curve type
			et = (EGSegType) (int) val;
			for (segn = 0; segn < 4; segn++)
				bz->envSig.SetType(segn, et);
			break;
		case 18:
			bz->envSig.SetSusOn((int)val);
			break;

		case 20: // Modulation envelope attack rate 1
			bz->envMod.SetRate(0, FrqValue(val));
			break;
		case 21:
			bz->envMod.SetLevel(0, AmpValue(val));
			break;
		case 22: // Modulation envelope attack rate 2
			bz->envMod.SetRate(1, FrqValue(val));
			break;
		case 23:
			bz->envMod.SetLevel(1, AmpValue(val));
			break;
		case 24: // Modulation envelope attack rate 3
			bz->envMod.SetRate(2, FrqValue(val));
			break;
		case 25: // Modulation  envelope sustain level
			bz->envMod.SetLevel(2, AmpValue(val));
			break;
		case 26: // Modulation envelope release rate
			bz->envMod.SetRate(3, FrqValue(val));
			break;
		case 27: // Modulation envelope curve type
			et = (EGSegType) (int) val;
			for (segn = 0; segn < 4; segn++)
				bz->envMod.SetType(segn, et);
			break;
		case 28:
			bz->envMod.SetSusOn((int)val);
			break;

		case 30:
			bz->modOn = (int) val;
			break;
		case 31: // Filter on/off
			if (val != 0)
				bz->modOn |= BUZZ_FILTER;
			else
				bz->modOn &= ~BUZZ_FILTER;
			break;
		case 32: // Filter frequency relative
			if (val != 0)
				bz->modOn |= BUZZ_RELFRQ;
			else
				bz->modOn &= ~BUZZ_RELFRQ;
			break;
		case 33: // Filter modulation on/off
			if (val != 0)
				bz->modOn |= BUZZ_MFILT;
			else
				bz->modOn &= ~BUZZ_MFILT;
			break;
		case 34: // Harmonics modulation on/off
			if (val != 0)
				bz->modOn |= BUZZ_MHARM;
			else
				bz->modOn &= ~BUZZ_MHARM;
			break;
		case 35: // Generator on/off
			if (val != 0)
				bz->modOn |= BUZZ_ENABLE;
			else
				bz->modOn &= ~BUZZ_ENABLE;
			break;

		default:
			err = 1;
			break;
		}
	}
	else
		err = 1;

	return err;
}

int BuzzSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_CHNL, (float) chnl);
	params->SetParam(P_VOLUME, (float) vol);

	params->SetParam(16, (float) lfoGen.GetFrequency());
	params->SetParam(17, (float) lfoGen.GetWavetable());
	params->SetParam(18, (float) lfoGen.GetAttack());
	params->SetParam(19, (float) lfoGen.GetLevel());
	params->SetParam(20, (float) pbWT.GetLevel());
	params->SetParam(21, (float) pbWT.GetWavetable());
	params->SetParam(22, (float) pbWT.GetDuration());
	params->SetParam(23, (float) pbWT.GetDelay());
	params->SetParam(24, (float) pbWT.GetMode());

	int gn;
	int ndx;
	for (ndx = 0; ndx < BUZZ_NGEN; ndx++)
	{
		gn = (ndx+1) << 8;
		BuzzPart *bz = &buzz[ndx];
		params->SetParam(gn|0, (float) bz->volume);
		params->SetParam(gn|1, (float) bz->frqScl);
		params->SetParam(gn|2, (float) bz->ampBase);
		params->SetParam(gn|3, (float) bz->ampScl);
		params->SetParam(gn|5, (float) bz->fltBase);
		params->SetParam(gn|6, (float) bz->fltScl);
		params->SetParam(gn|7, (float) bz->fltQ);
		params->SetParam(gn|8, (float) bz->frqRatio);
		params->SetParam(gn|9, (float) bz->harmMax);

		params->SetParam(gn|10, (float) bz->envSig.GetRate(0));
		params->SetParam(gn|11, (float) bz->envSig.GetLevel(0));
		params->SetParam(gn|12, (float) bz->envSig.GetRate(1));
		params->SetParam(gn|13, (float) bz->envSig.GetLevel(1));
		params->SetParam(gn|14, (float) bz->envSig.GetRate(2));
		params->SetParam(gn|15, (float) bz->envSig.GetLevel(2));
		params->SetParam(gn|16, (float) bz->envSig.GetRate(3));
		params->SetParam(gn|17, (float) bz->envSig.GetType(0));
		params->SetParam(gn|18, (float) bz->envSig.GetSusOn());

		params->SetParam(gn|20, (float) bz->envMod.GetRate(0));
		params->SetParam(gn|21, (float) bz->envMod.GetLevel(0));
		params->SetParam(gn|22, (float) bz->envMod.GetRate(1));
		params->SetParam(gn|23, (float) bz->envMod.GetLevel(1));
		params->SetParam(gn|24, (float) bz->envMod.GetRate(2));
		params->SetParam(gn|25, (float) bz->envMod.GetLevel(2));
		params->SetParam(gn|26, (float) bz->envMod.GetRate(3));
		params->SetParam(gn|27, (float) bz->envMod.GetType(0));
		params->SetParam(gn|28, (float) bz->envMod.GetSusOn());

		params->SetParam(gn|30, (float) bz->modOn);
		params->SetParam(gn|31, bz->modOn & BUZZ_FILTER ? 1.0 : 0.0);
		params->SetParam(gn|32, bz->modOn & BUZZ_RELFRQ ? 1.0 : 0.0);
		params->SetParam(gn|33, bz->modOn & BUZZ_MFILT ? 1.0 : 0.0);
		params->SetParam(gn|34, bz->modOn & BUZZ_MHARM ? 1.0 : 0.0);
		params->SetParam(gn|35, bz->modOn & BUZZ_ENABLE ? 1.0 : 0.0);
	}
	return 0;
}

int BuzzSynth::GetParam(bsInt16 idval, float *val)
{
	bsInt16 gn = (idval >> 8) & 0xFF;
	bsInt16 pn = idval & 0xff;
	if (gn == 0)
	{
		switch (pn)
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
			*val = (float) pbWT.GetLevel();
			break;
		case 21:
			*val = (float) pbWT.GetWavetable();
			break;
		case 22:
			*val = (float) pbWT.GetDuration();
			break;
		case 23:
			*val = (float) pbWT.GetDelay();
			break;
		case 24:
			*val = (float) pbWT.GetMode();
			break;
		default:
			return 1;
		}
	}
	else if (gn > 0 && gn <= BUZZ_NGEN)
	{
		BuzzPart *bz = &buzz[gn-1];
		switch (pn & 0xff)
		{
		case 0: //	Oscillator volume
			*val = (float) bz->volume;
			break;
		case 1: //	Oscillator frequency multiplier (cents)
			*val = (float) bz->frqScl;
			break;
		case 2: // Harmonics amplitude base
			*val = (float) bz->ampBase;
			break;
		case 3: // Harmonics amplitude multiplier 
			*val = (float) bz->ampScl;
			break;
		case 5: // Filter base (cents above fundamental frequency)
			*val = (float) bz->fltBase;
			break;
		case 6: //Filter envelope scale (cents)
			*val = (float) bz->fltScl;
			break;
		case 7: // Filter 'Q'
			*val = (float) bz->fltQ;
			break;
		case 8: // Harmonics ratio
			*val = (float) bz->frqRatio;
			break;
		case 9:
			*val = (float) bz->harmMax;
			break;

		case 10: // Amplitude envelope attack rate 1
			*val = (float) bz->envSig.GetRate(0);
			break;
		case 11:
			*val = (float) bz->envSig.GetLevel(0);
			break;
		case 12: // Amplitude envelope attack rate 2
			*val = (float) bz->envSig.GetRate(1);
			break;
		case 13:
			*val = (float) bz->envSig.GetLevel(1);
			break;
		case 14: // Amplitude envelope attack rate 3
			*val = (float) bz->envSig.GetRate(2);
			break;
		case 15: // Amplitude  envelope sustain level
			*val = (float) bz->envSig.GetLevel(2);
			break;
		case 16: // Amplitude envelope release rate
			*val = (float) bz->envSig.GetRate(3);
			break;
		case 17: // Amplitude envelope curve type
			*val = (float) bz->envSig.GetType(0);
			break;
		case 18:
			*val = (float) bz->envSig.GetSusOn();
			break;

		case 20: // Amplitude envelope attack rate 1
			*val = (float) bz->envMod.GetRate(0);
			break;
		case 21:
			*val = (float) bz->envMod.GetLevel(0);
			break;
		case 22: // Amplitude envelope attack rate 2
			*val = (float) bz->envMod.GetRate(1);
			break;
		case 23:
			*val = (float) bz->envMod.GetLevel(1);
			break;
		case 24: // Amplitude envelope attack rate 3
			*val = (float) bz->envMod.GetRate(2);
			break;
		case 25: // Amplitude  envelope sustain level
			*val = (float) bz->envMod.GetLevel(2);
			break;
		case 26: // Amplitude envelope release rate
			*val = (float) bz->envMod.GetRate(3);
			break;
		case 27: // Amplitude envelope curve type
			*val = (float) bz->envMod.GetType(0);
			break;
		case 28:
			*val = (float) bz->envMod.GetSusOn();
			break;

		case 30:
			*val = (float) bz->modOn;
			break;
		case 31: // Filter On/Off
			*val = (bz->modOn & BUZZ_FILTER) ? 1.0 : 0.0;
			break;
		case 32: // Filter cutoff relative to pitch
			*val = bz->modOn & BUZZ_RELFRQ ? 1.0 : 0.0;
			break;
		case 33: // Filter cutoff modulation on/off
			*val = (bz->modOn & BUZZ_MFILT) ? 1.0 : 0.0;
			break;
		case 34: // Harmonics modulation on/off
			*val = (bz->modOn & BUZZ_MHARM) ? 1.0 : 0.0;
			break;
		case 35: // Harmonics modulation on/off
			*val = (bz->modOn & BUZZ_ENABLE) ? 1.0 : 0.0;
			break;

		default:
			return 1;
		}
	}
	else
		return 1;
	return 0;
}

int BuzzSynth::Load(XmlSynthElem *parent)
{
	short ival;

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("gen"))
		{
			if (elem->GetAttribute("gn", ival) == 0)
			{
				if (ival >= 0 && ival < BUZZ_NGEN)
					buzz[ival].Load(elem);
			}
		}
		else if (elem->TagMatch("lfo"))
			lfoGen.Load(elem);
		else if (elem->TagMatch("pb"))
			pbWT.Load(elem);
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int BuzzSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;

	for (short ndx = 0; ndx < BUZZ_NGEN; ndx++)
	{
		if ((elem = parent->AddChild("gen")) == NULL)
			return -1;
		elem->SetAttribute("gn", ndx);
		buzz[ndx].Save(elem);
		delete elem;
	}

	if ((elem = parent->AddChild("lfo")) == NULL)
		return -1;
	lfoGen.Save(elem);
	delete elem;

	if ((elem = parent->AddChild("pb")) == NULL)
		return -1;
	pbWT.Save(elem);
	delete elem;

	return 0;
}

int BuzzPart::Load(XmlSynthElem *parent)
{
	bsInt16 ival;
	float dval;

	if (parent->GetAttribute("mod", ival) == 0)
		modOn = ival;
	if (parent->GetAttribute("vol", dval) == 0)
		volume = AmpValue(dval);

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("osc"))
		{
			if (elem->GetAttribute("frq", dval) == 0)
				frqBase = FrqValue(dval);
			if (elem->GetAttribute("fscl", dval) == 0)
				frqScl = FrqValue(dval);
			if (elem->GetAttribute("fr", dval) == 0)
				frqRatio = dval;
			if (elem->GetAttribute("harm", ival) == 0)
				harmMax = ival;
			if (elem->GetAttribute("amp", dval) == 0)
				ampBase = AmpValue(dval);
			if (elem->GetAttribute("ascl", dval) == 0)
				ampScl = AmpValue(dval);
		}
		else if (elem->TagMatch("flt"))
		{
			if (elem->GetAttribute("fc",  dval) == 0)
				fltBase = FrqValue(dval);
			if (elem->GetAttribute("fcscl",  dval) == 0)
				fltScl = AmpValue(dval);
			if (elem->GetAttribute("q",  dval) == 0)
				fltQ = AmpValue(dval);
		}
		else if (elem->TagMatch("egv"))
			LoadEnv(elem, &envSig);
		else if (elem->TagMatch("egm"))
			LoadEnv(elem, &envMod);
		next = elem->NextSibling();
		delete elem;
	}

	return 0;
}

int BuzzPart::LoadEnv(XmlSynthElem *elem, EnvGenSegSus *env)
{
	float dval;
	short ival;

	if (elem->GetAttribute("rt0", dval) == 0)
		env->SetRate(0, FrqValue(dval));
	if (elem->GetAttribute("lv0", dval) == 0)
		env->SetLevel(0, AmpValue(dval));
	if (elem->GetAttribute("rt1", dval) == 0)
		env->SetRate(1, FrqValue(dval));
	if (elem->GetAttribute("lv1", dval) == 0)
		env->SetLevel(1, AmpValue(dval));
	if (elem->GetAttribute("rt2", dval) == 0)
		env->SetRate(2, FrqValue(dval));
	if (elem->GetAttribute("lv2", dval) == 0)
		env->SetLevel(2, AmpValue(dval));
	if (elem->GetAttribute("rt3", dval) == 0)
		env->SetRate(3, FrqValue(dval));
	if (elem->GetAttribute("ty", ival) == 0)
	{
		env->SetType(0, (EGSegType)ival);
		env->SetType(1, (EGSegType)ival);
		env->SetType(2, (EGSegType)ival);
		env->SetType(3, (EGSegType)ival);
	}
	if (elem->GetAttribute("son", ival) == 0)
		env->SetSusOn(ival);
	return 0;
}

int BuzzPart::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;
	
	parent->SetAttribute("mod", (long)modOn);
	parent->SetAttribute("vol",  (float)volume);

	if ((elem = parent->AddChild("osc")) == NULL)
		return -1;
	elem->SetAttribute("frq",  (float)frqBase);
	elem->SetAttribute("fscl", (float)frqScl);
	elem->SetAttribute("fr", (short)frqRatio);
	elem->SetAttribute("harm", (short)harmMax);
	elem->SetAttribute("amp", (float)ampBase);
	elem->SetAttribute("ascl", (float)ampScl);
	delete elem;

	if ((elem = parent->AddChild("flt")) == NULL)
		return -1;
	elem->SetAttribute("fc",  (float)fltBase);
	elem->SetAttribute("fscl", (float)fltScl);
	elem->SetAttribute("q",  (float)fltQ);
	delete elem;

	if ((elem = parent->AddChild("egv")) == NULL)
		return -1;
	SaveEnv(elem, &envSig);
	delete elem;

	if ((elem = parent->AddChild("egm")) == NULL)
		return -1;
	SaveEnv(elem, &envMod);
	delete elem;

	return 0;
}

int BuzzPart::SaveEnv(XmlSynthElem *elem, EnvGenSegSus *env)
{
	elem->SetAttribute("rt0", (float) env->GetRate(0));
	elem->SetAttribute("lv0", (float) env->GetLevel(0));
	elem->SetAttribute("rt1", (float) env->GetRate(1));
	elem->SetAttribute("lv1", (float) env->GetLevel(1));
	elem->SetAttribute("rt2", (float) env->GetRate(2));
	elem->SetAttribute("lv2", (float) env->GetLevel(2));
	elem->SetAttribute("rt3", (float) env->GetRate(3));
	elem->SetAttribute("ty", (short) env->GetType(0));
	elem->SetAttribute("son", (short) env->GetSusOn());
	return 0;
}
