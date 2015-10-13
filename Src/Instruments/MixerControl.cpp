//////////////////////////////////////////////////////////////////////
/// @file MixerControl.cpp Implementation of the MixerControl instrument.
//
// BasicSynth - Dynamic Mixer Control instrument
//
// Performs dynamic control of the mixer. The following functions
// are defined:
// 1. Set input channel level
// 2. Set input channel pan position
// 3. Incrementally vary channel level
// 4. Incrementally vary pan position
// 5. Oscillate input level
// 6. Oscillate pan position
// 7. Set input channel send level
// 8. Set effects channel receive level
// 9. Set effects channel pan level
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "MixerControl.h"

MixerControl::MixerControl()
{
	im = 0;
	mix = 0;
	func = mixNoFunc;
	tickCount = 0;
	toLvl = 0;
	frLvl = 0;
	tmSec = 0;
	fxUnit = -1;
	inChnl = -1;
	swpRng = 0;
	swpOffs = 0;
	oscOn = 0;
	panType = panTrig;
}

MixerControl::~MixerControl()
{
}

Instrument *MixerControl::MixerControlFactory(InstrManager *i, Opaque tmplt)
{
	MixerControl *mc = new MixerControl;
	if (mc)
		mc->im = i;
	return (Instrument *) mc;
}

SeqEvent *MixerControl::MixerControlEventFactory(Opaque tmplt)
{
	VarParamEvent *evt = new VarParamEvent;
	evt->maxParam = P_MIX_PANT;
	return (SeqEvent*) evt;
}

VarParamEvent *MixerControl::AllocParams()
{
	return (VarParamEvent *) MixerControlEventFactory(0);
}

static InstrParamMap mixControlParams[] =
{
	{ "in",   P_CHNL },
	{ "from", P_MIX_FROM },
	{ "frq",  P_MIX_FRQ },
	{ "func", P_MIX_FUNC },
	{ "fxu",  P_MIX_FX },
	{ "pant", P_MIX_PANT },
	{ "tm",   P_MIX_TIME },
	{ "to",   P_MIX_TO },
	{ "wt",   P_MIX_WT }
};

bsInt16 MixerControl::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, mixControlParams, sizeof(mixControlParams)/sizeof(InstrParamMap));
}

const char *MixerControl::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, mixControlParams, sizeof(mixControlParams)/sizeof(InstrParamMap));
}

void MixerControl::Copy(MixerControl *tp)
{
	func = tp->func;
	toLvl = tp->toLvl;
	frLvl = tp->frLvl;
	tmSec = tp->tmSec;
	fxUnit = tp->fxUnit;
	inChnl = tp->inChnl;
	panType = tp->panType;
	oscOn = tp->oscOn;
	swpRng = tp->swpRng;
	swpOffs = tp->swpOffs;
	tickCount = tp->tickCount;
	osc.SetFrequency(tp->osc.GetFrequency());
	osc.SetWavetable(tp->osc.GetWavetable());
	rmp.Copy(&tp->rmp);
}

void MixerControl::Start(SeqEvent *evt)
{
	oscOn = 0;
	SetParams((VarParamEvent *)evt);

	if (im == 0)
		return;

	mix = im->GetMixer();
	if (mix == 0)
		return;

	switch (func)
	{
	case mixSetInpLvl:
		mix->ChannelVolume(inChnl, frLvl);
		break;
	case mixSetPanPos:
		mix->ChannelPan(inChnl, panType, frLvl);
		break;
	case mixSetSendLvl:
		mix->FxLevel(fxUnit, inChnl, frLvl);
		break;
	case mixSetFxLvl:
		mix->FxReceive(fxUnit, frLvl);
		break;
	case mixSetFxPan:
		mix->FxPan(fxUnit, panTrig, frLvl);
		break;
	case mixRampInpLvl:
	case mixRampPanPos:
	case mixRampSendLvl:
	case mixRampFxLvl:
	case mixRampFxPan:
		rmp.InitSegTick(tickCount, frLvl, toLvl);
		break;
	case mixOscilInpLvl:
	case mixOscilPanPos:
	case mixOscilSendLvl:
	case mixOscilFxLvl:
	case mixOscilFxPan:
		oscOn = 1;
		if (frLvl < toLvl)
		{
			swpRng = toLvl - frLvl;
			swpOffs = frLvl;
		}
		else
		{
			swpRng = frLvl - toLvl;
			swpOffs = toLvl;
		}
		osc.Reset(0);
		break;
	}
}

void MixerControl::Param(SeqEvent *evt)
{
}

void MixerControl::Stop()
{
}

void MixerControl::Tick()
{
	if (tickCount < 0 || mix == 0)
		return;
	tickCount--;
	float oscVal = 0;
	if (oscOn)
		oscVal = (((osc.Gen() + 1.0) / 2) * swpRng) + swpOffs;
	switch (func)
	{
	case mixRampInpLvl:
		mix->ChannelVolume(inChnl, rmp.Gen());
		break;
	case mixRampPanPos:
		mix->ChannelPan(inChnl, panType, rmp.Gen());
		break;
	case mixRampSendLvl:
		mix->FxLevel(fxUnit, inChnl, rmp.Gen());
		break;
	case mixRampFxLvl:
		mix->FxReceive(fxUnit, rmp.Gen());
		break;
	case mixRampFxPan:
		mix->FxPan(fxUnit, panType, rmp.Gen());
		break;
	case mixOscilInpLvl:
		mix->ChannelIn(inChnl, oscVal);
		break;
	case mixOscilPanPos:
		mix->ChannelPan(inChnl, panType, oscVal);
		break;
	case mixOscilSendLvl:
		mix->FxLevel(fxUnit, inChnl, oscVal);
		break;
	case mixOscilFxLvl:
		mix->FxReceive(fxUnit, oscVal);
		break;
	case mixOscilFxPan:
		mix->FxPan(fxUnit, panType, oscVal);
		break;
	}
}

int  MixerControl::IsFinished()
{
	return tickCount <= 0;
}

void MixerControl::Destroy()
{
	delete this;
}

int MixerControl::Load(XmlSynthElem *node)
{
	float fval;
	short sval;

	if (node->GetAttribute("in", sval) == 0)
		inChnl = sval;
	if (node->GetAttribute("fn", sval) == 0)
		func = sval;
	if (node->GetAttribute("from", fval) == 0)
		frLvl = fval;
	if (node->GetAttribute("to", fval) == 0)
		toLvl = fval;
	if (node->GetAttribute("tm", fval) == 0)
		tmSec = fval;
	if (node->GetAttribute("frq", fval) == 0)
		osc.SetFrequency(fval);
	if (node->GetAttribute("wt", sval) == 0)
		osc.SetWavetable((int)sval);
	if (node->GetAttribute("fxu", sval) == 0)
		fxUnit = sval;
	if (node->GetAttribute("pant", sval) == 0)
		panType = sval;

	return 0;
}

int MixerControl::Save(XmlSynthElem *node)
{
	node->SetAttribute("in", (short) inChnl);
	node->SetAttribute("fn", (short) func);
	node->SetAttribute("from", frLvl);
	node->SetAttribute("to", toLvl);
	node->SetAttribute("tm", tmSec);
	node->SetAttribute("frq", (float) osc.GetFrequency());
	node->SetAttribute("wt", (short) osc.GetWavetable());
	node->SetAttribute("fxu", fxUnit);
	node->SetAttribute("pant", panType);

	return 0;
}

int MixerControl::GetParams(VarParamEvent *params)
{
	params->SetParam(P_DUR,      (float) synthParams.sampleRate * tmSec);
	params->SetParam(P_CHNL,     (float) inChnl);
	params->SetParam(P_MIX_FUNC, (float) func);
	params->SetParam(P_MIX_FRQ,  (float) osc.GetFrequency());
	params->SetParam(P_MIX_FROM, (float) frLvl);
	params->SetParam(P_MIX_TO,   (float) toLvl);
	params->SetParam(P_MIX_TIME, (float) tmSec);
	params->SetParam(P_MIX_FX,   (float) fxUnit);
	params->SetParam(P_MIX_WT,   (float) osc.GetWavetable());
	params->SetParam(P_MIX_PANT, (float) panType);
	return 0;
}

int MixerControl::SetParams(VarParamEvent *params)
{
	inChnl = params->chnl;
	tickCount = params->duration;

	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	int err = 0;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int MixerControl::SetParam(bsInt16 idval, float val)
{
	switch (idval)
	{
	case P_MIX_FUNC:
		func = (int) val;
		break;
	case P_MIX_TIME:
		tmSec = FrqValue(val);
		tickCount = (bsInt32) (synthParams.sampleRate * val);
		break;
	case P_MIX_FROM:
		frLvl = AmpValue(val);
		break;
	case P_MIX_TO:
		toLvl = AmpValue(val);
		break;
	case P_MIX_FX:
		fxUnit = (bsInt16) val;
		break;
	case P_MIX_WT:
		osc.SetWavetable((int) val);
		break;
	case P_MIX_PANT:
		panType = (int) val;
		break;
	default:
		return -1;
	}
	return 0;
}

int MixerControl::GetParam(bsInt16 idval, float* val)
{
	switch (idval)
	{
	case P_MIX_FUNC:
		*val = (float) func;
		break;
	case P_MIX_TIME:
		*val = (float) tmSec;
		break;
	case P_MIX_FROM:
		*val = (float) frLvl;
		break;
	case P_MIX_TO:
		*val = (float) toLvl;
		break;
	case P_MIX_FX:
		*val = (float) fxUnit;
		break;
	case P_MIX_WT:
		*val = (float) osc.GetWavetable();
		break;
	case P_MIX_PANT:
		*val = (float) panType;
		break;
	default:
		return -1;
	}
	return 0;
}
