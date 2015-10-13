//////////////////////////////////////////////////////////////////////
/// @file AddSynth.cpp Implementation for Additive Synthesis instrument
//
// See _BasicSynth_ Chapter 20 for a full explanation
//
// Maintains an array of "tone generators" and sums the outputs.
// Each tone generator consists of a wavetable oscillator and envelope
// generator. A single LFO generator applies optional vibrato to
// all tone generators.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "AddSynth.h"

Instrument *AddSynth::AddSynthFactory(InstrManager *m, Opaque tmplt)
{
	AddSynth *ip;
	if (tmplt)
		ip = new AddSynth((AddSynth*)tmplt);
	else
		ip = new AddSynth;
	if (ip)
		ip->im = m;
	return ip;
}

SeqEvent *AddSynth::AddSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->frq = 440.0;
	ep->vol = 1.0;
	ep->maxParam = 0x3FFF; // max 63 partials, 16 envelope segments
	return (SeqEvent *) ep;
}

VarParamEvent *AddSynth::AllocParams()
{
	return (VarParamEvent *) AddSynthEventFactory(0);
}

static InstrParamMap addsynthParams[] = 
{
	{ "fix", 8 },
	{ "frq", 1 },
	{ "lfoamp", 19 },
	{ "lfoatk", 18 },
	{ "lfofrq", 16 },
	{ "lfowt", 17 },
	{ "lvl", 6 },
	{ "mul", 0 },
	{ "rt",  5 },
	{ "son", 4 },
	{ "st",  3 },
	{ "sus", 4 },
	{ "ty",  7 },
	{ "wt",  2 }
};

bsInt16 AddSynth::MapParamID(const char *name, Opaque tmplt)
{
	int pn = 0;
	int sn = 0;
	int vn = 0;
	const char *str = name;
	if (*str == 'p')
	{
		str = InstrParamMap::ParamNum(str+1, &pn);
		pn++;
		if (*str == '.')
			str++;
		if (*str == 's')
			str = InstrParamMap::ParamNum(str+1, &sn);
		if (*str == '.')
			str++;
	}
	vn = InstrParamMap::SearchParamID(str, addsynthParams, sizeof(addsynthParams)/sizeof(InstrParamMap));
	if (vn >= 0)
		return (pn << 8) + (sn << 4) + vn;
	return -1;
}

const char *AddSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	static bsString paramNameBuf;
	paramNameBuf = "";
	char dig[6];
	bsInt16 n = (id >> 8) & 0xff;
	if (n)
	{
		dig[0] = 'p';
		InstrParamMap::FormatNum(n, &dig[1]);
		paramNameBuf += dig;
		n = (id >> 4) & 0x0f;
		dig[0] = 's';
		InstrParamMap::FormatNum(n, &dig[1]);
		paramNameBuf += dig;
	}

	int count = sizeof(addsynthParams)/sizeof(InstrParamMap);
	for (int index = 0; index < count; index++)
	{
		if (addsynthParams[index].id == id)
		{
			paramNameBuf += '.';
			paramNameBuf += addsynthParams[index].name;
			break;
		}
	}
	return paramNameBuf;
}

AddSynth::AddSynth()
{
	im = 0;
	chnl = 0;
	frq = 440.0;
	vol = 1.0;
	numParts = 0;
	parts = NULL;
}

AddSynth::AddSynth(AddSynth *tp)
{
	im = 0;
	parts = 0;
	numParts = 0;
	Copy(tp);
}

AddSynth::~AddSynth()
{
	delete[] parts;
}

int AddSynth::SetNumParts(int n)
{
	if (n < 1)
		return -1;
	AddSynthPart *newParts = new AddSynthPart[n];
	if (newParts == NULL)
		return -1;
	int ndx;
	for (ndx = 0; ndx < numParts && ndx < n; ndx++)
		newParts[ndx].Copy(&parts[ndx]);

	delete[] parts;
	parts = newParts;
	numParts = n;
	return 0;
}

int AddSynth::GetNumParts()
{
	return numParts;
}

AddSynthPart *AddSynth::GetPart(int n)
{
	if (n < numParts)
		return &parts[n];
	return NULL;
}

void AddSynth::Copy(AddSynth *tp)
{
	frq = tp->frq;
	vol = tp->vol;
	chnl = tp->chnl;
	SetNumParts(tp->GetNumParts());
	for (int n = 0; n < numParts; n++)
		parts[n].Copy(&tp->parts[n]);
	lfoGen.Copy(&tp->lfoGen);
}

void AddSynth::Start(SeqEvent *evt)
{
	UpdateParams(evt, 0);
}

void AddSynth::Param(SeqEvent *evt)
{
	if (evt->type == SEQEVT_CONTROL)
		return; // TODO: process controller changes
	UpdateParams(evt, -1);
}

void AddSynth::UpdateParams(SeqEvent *evt, float initPhs)
{
	SetParams((VarParamEvent *)evt);

	FrqValue nyquist = synthParams.sampleRate / 2;
	AddSynthPart *pEnd = &parts[numParts];
	AddSynthPart *pSig = parts;
	for (pSig = parts; pSig < pEnd; pSig++)
	{
		FrqValue f = frq * pSig->mul;
		if (f >= nyquist)
			f = 0;
		pSig->osc.SetFrequency(f);
		pSig->osc.Reset(initPhs);
		pSig->env.Reset(initPhs);
	}
	lfoGen.SetSigFrq(frq);
	lfoGen.Reset(initPhs);
}

void AddSynth::Stop()
{
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		pSig->env.Release();
		pSig++;
	}
}

void AddSynth::Tick()
{
	PhsAccum phs = 0;
	int lfoOn = lfoGen.On();
	if (lfoOn)
		phs = lfoGen.Gen() * synthParams.frqTI;

	AmpValue sigVal = 0;
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		if (lfoOn)
			pSig->osc.PhaseModWT(phs * pSig->mul);
		sigVal += pSig->env.Gen() * pSig->osc.Gen();
		pSig++;
	}

	im->Output(chnl, sigVal * vol);
}

int  AddSynth::IsFinished()
{
	AddSynthPart *pSig = parts;
	AddSynthPart *pEnd = &parts[numParts];
	while (pSig < pEnd)
	{
		if (!pSig->env.IsFinished())
			return 0;
		pSig++;
	}

	return 1;
}

void AddSynth::Destroy()
{
	delete this;
}

/*************
<instr parts="n">
 <part pn="n"  mul="n" frq="n" wt="n" />
   <env segs="n" st="n" son="n">
    <seg sn="n" rt="n" lvl="n" ty="t" />
   </env>
 </part>
 <lfo frq="" wt="" atk="" lvl="" />
</instr>
***************/

int AddSynth::Load(XmlSynthElem *parent)
{
	float dval;
	float lvl;
	float rt;
	long ival;

	if (parent->GetAttribute("parts", ival) == 0)
		SetNumParts(ival);

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("part"))
		{
			long pno = -1;
			if (elem->GetAttribute("pn", pno) == 0 && pno < numParts)
			{
				AddSynthPart *pn = &parts[pno];
				if (elem->GetAttribute("mul", dval) == 0)
					pn->mul = FrqValue(dval);
				if (elem->GetAttribute("frq", dval) == 0)
					pn->osc.SetFrequency(FrqValue(dval));
				if (elem->GetAttribute("wt", ival) == 0)
					pn->osc.SetWavetable((int) ival);
				XmlSynthElem *partElem = elem->FirstChild();
				while (partElem != NULL)
				{
					if (partElem->TagMatch("env"))
					{
						EnvGenSeg *pe = &pn->env;
						long nseg = 0;
						if (partElem->GetAttribute("segs", nseg) == 0)
							pe->SetSegs((int)nseg);
						if (partElem->GetAttribute("st", lvl) == 0)
							pe->SetStart(AmpValue(lvl));
						if (partElem->GetAttribute("sus", ival) == 0)
							pe->SetSusOn((int)ival);
						XmlSynthElem *segElem = partElem->FirstChild();
						while (segElem != NULL)
						{
							if (segElem->TagMatch("seg"))
							{
								if (segElem->GetAttribute("sn", nseg) == 0)
								{
									if (segElem->GetAttribute("rt", rt) == 0)
										pe->SetRate((int)nseg, FrqValue(rt));
									if (segElem->GetAttribute("lvl", lvl) == 0)
										pe->SetLevel((int)nseg, AmpValue(lvl));
									if (segElem->GetAttribute("ty", ival) == 0)
										pe->SetType((int)nseg, (EGSegType)ival);
									if (segElem->GetAttribute("fix", ival) == 0)
										pe->SetFixed((int)nseg, (int)ival);
								}
							}
							next = segElem->NextSibling();
							delete segElem;
							segElem = next;
						}
					}
					next = partElem->NextSibling();
					delete partElem;
					partElem = next;
				}
			}
		}
		else if (elem->TagMatch("lfo"))
		{
			lfoGen.Load(elem);
		}
		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int AddSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *partElem;
	XmlSynthElem *subElem;
	XmlSynthElem *segElem;

	parent->SetAttribute("parts", (short)numParts);
	AddSynthPart *p = parts;
	for (int n = 0; n < numParts; n++, p++)
	{
		partElem = parent->AddChild("part");
		if (partElem == NULL)
			return -1;
		partElem->SetAttribute("pn", (short) n);
		partElem->SetAttribute("mul", p->mul);
		partElem->SetAttribute("frq", p->osc.GetFrequency());
		partElem->SetAttribute("wt", (short) p->osc.GetWavetable());

		subElem = partElem->AddChild("env");
		if (subElem == NULL)
			return -1;
		EnvGenSeg *pe = &p->env;
		int segs = pe->GetSegs();
		subElem->SetAttribute("segs", (short) segs);
		subElem->SetAttribute("st", pe->GetStart());
		subElem->SetAttribute("sus", (short) pe->GetSusOn());
		for (int sn = 0; sn < segs; sn++)
		{
			segElem = subElem->AddChild("seg");
			if (segElem == NULL)
				return -1;
			segElem->SetAttribute("sn", (short) sn);
			segElem->SetAttribute("rt", pe->GetRate(sn));
			segElem->SetAttribute("lvl", pe->GetLevel(sn));
			segElem->SetAttribute("ty", (short) pe->GetType(sn));
			segElem->SetAttribute("fix", (short) pe->GetFixed(sn));
			delete segElem;
		}
		delete subElem;
		delete partElem;
	}

	XmlSynthElem *lfoElem = parent->AddChild("lfo");
	if (lfoElem == NULL)
		return -1;
	lfoGen.Save(lfoElem);
	delete lfoElem;

	return 0;
}

// [pn(6)][sn(4)][val(4)]
// PN = (partial number + 1) * 256
// SN = segment number * 16
// PN+0	Frequency multiplier. 
// PN+1	Initial frequency for this oscillator. 
// PN+2	Wave table index.
// PN+3	Starting value for the envelope.
// PN+4 Sustain-on flag, 1 or 0.
// PN+SN+5	segment rate
// PN+SN+6	Level at the end of the segment.
// PN+SN+7	Segment curve type: 1=linear 2=exponential 3=log.
// PN+SN+8  Fixed (1) or relative (0) rate
int AddSynth::SetParams(VarParamEvent *params)
{
	int err = 0;

	chnl = params->chnl;
	vol = params->vol;
	if (params->noteonvel > 0)
		vol *= ((float)params->noteonvel / 127.0);
	frq = params->frq;
	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int AddSynth::SetParam(bsInt16 idval, float val)
{
	AddSynthPart *pSig;
	bsInt16 pn, sn;
	pn = (idval & 0x7F00) >> 8;
	if (pn == 0)
	{
		switch (idval)
		{
		case 16:
			lfoGen.SetFrequency(FrqValue(val));
			break;
		case 17:
			lfoGen.SetWavetable((int)val);
			break;
		case 18:
			lfoGen.SetAttack(FrqValue(val));
			break;
		case 19:
			lfoGen.SetLevel(AmpValue(val));
			break;
		case 20:
			// num parts not settable
			//break;
		default:
			return 1;
		}
	}
	else if (pn <= numParts)
	{
		sn = (idval & 0xF0) >> 4;
		pSig = &parts[pn-1];
		switch (idval & 0x0F)
		{
		case 0:
			pSig->mul = FrqValue(val);
			break;
		case 1:
			pSig->osc.SetFrequency(FrqValue(val));
			break;
		case 2:
			pSig->osc.SetWavetable((int)val);
			break;
		case 3:
			pSig->env.SetStart(AmpValue(val));
			break;
		case 4:
			pSig->env.SetSusOn((int)val);
			break;
		case 5:
			pSig->env.SetRate(sn, FrqValue(val));
			break;
		case 6:
			pSig->env.SetLevel(sn, AmpValue(val));
			break;
		case 7:
			pSig->env.SetType(sn, (EGSegType) (int) val);
			break;
		case 8:
			pSig->env.SetFixed(sn, (int) val);
			break;
		case 9:
			pSig->env.SetSegs((int) val);
			break;
		default:
			return 1;
		}
	}
	else
		return 1;
	return 0;
}

int AddSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_FREQ, (float)frq);
	params->SetParam(P_VOLUME, (float)vol);
	params->SetParam(16, (float)lfoGen.GetFrequency());
	params->SetParam(17, (float)lfoGen.GetWavetable());
	params->SetParam(18, (float)lfoGen.GetAttack());
	params->SetParam(19, (float)lfoGen.GetLevel());
	params->SetParam(20, (float)numParts);

	int pn, sn, segs, idval;
	for (pn = 0; pn < numParts; pn++)
	{
		AddSynthPart *pSig = &parts[pn];
		idval = (pn+1) << 8;
		params->SetParam(idval, (float) pSig->mul);
		params->SetParam(idval+1, (float) pSig->osc.GetFrequency());
		params->SetParam(idval+2, (float) pSig->osc.GetWavetable());
		params->SetParam(idval+3, (float) pSig->env.GetStart());
		params->SetParam(idval+4, (float) pSig->env.GetSusOn());
		segs = pSig->env.GetSegs();
		params->SetParam(idval+9, (float) segs);
		for (sn = 0; sn < segs; sn++)
		{
			idval = ((pn+1) << 8) + (sn << 4);
			params->SetParam(idval+5, (float) pSig->env.GetRate(sn));
			params->SetParam(idval+6, (float) pSig->env.GetLevel(sn));
			params->SetParam(idval+7, (float) pSig->env.GetType(sn));
			params->SetParam(idval+8, (float) pSig->env.GetFixed(sn));
		}
	}
	return 0;
}

int AddSynth::GetParam(bsInt16 idval, float* val)
{
	AddSynthPart *pSig;
	bsInt16 pn, sn;
	pn = (idval & 0x7F00) >> 8;
	if (pn == 0)
	{
		switch (idval)
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
			*val = (float) numParts;
			break;
		default:
			return 1;
		}
	}
	else if (pn <= numParts)
	{
		sn = (idval & 0xF0) >> 4;
		pSig = &parts[pn-1];
		switch (idval & 0x0F)
		{
		case 0:
			*val = (float) pSig->mul;
			break;
		case 1:
			*val = (float) pSig->osc.GetFrequency();
			break;
		case 2:
			*val = (float) pSig->osc.GetWavetable();
			break;
		case 3:
			*val = (float) pSig->env.GetStart();
			break;
		case 4:
			*val = (float) pSig->env.GetSusOn();
			break;
		case 5:
			*val = (float) pSig->env.GetRate(sn);
			break;
		case 6:
			*val = (float) pSig->env.GetLevel(sn);
			break;
		case 7:
			*val = (float) pSig->env.GetType(sn);
			break;
		case 8:
			*val = (float) pSig->env.GetFixed(sn);
			break;
		case 9:
			*val = (float) pSig->env.GetSegs();
			break;
		default:
			return 1;
		}
	}
	else
		return 1;
	return 0;
}
