//////////////////////////////////////////////////////////////////////
/// @file WFSynth.cpp Implementation of the WFSynth instrument.
//
// BasicSynth WaveFile playback instrument
//
// See _BasicSynth_ Chapter 23 for a full explanation
//
// This instrument plays a wave file. The sound is loaded into
// a cache and can be shared by multiple instrument instances.
// The sound can be played once or looped, and a simple AR 
// envelope can be applied to fade in/out.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "WFSynth.h"

static WaveFileIn wfCache[WFSYNTH_MAX_WAVEFILES];
static int wfCacheCount = 0;
static AmpValue dummy;

int WFSynth::GetCacheCount()
{
	return wfCacheCount;
}

WaveFileIn *WFSynth::GetCacheEntry(int n)
{
	return &wfCache[n];
}

void WFSynth::ClearCache()
{
	int n;
	for (n = 0; n < wfCacheCount; n++)
		wfCache[n].Clear();
	wfCacheCount = 0;
}

int WFSynth::AddToCache(const char *filename, bsInt16 id)
{
	int n;
	for (n = 0; n < wfCacheCount; n++)
	{
		const char *wfname = wfCache[n].GetFilename();
		if (wfname && strcmp(filename, wfname) == 0
		 && wfCache[n].GetFileID() == id)
		{
			return n;
		}
	}
	if (wfCacheCount >= WFSYNTH_MAX_WAVEFILES)
		return -1;
	if (wfCache[n].LoadWaveFile(filename, id) != 0)
		return -1;
	wfCacheCount++;
	return n;
}

Instrument *WFSynth::WFSynthFactory(InstrManager *m, Opaque tmplt)
{
	WFSynth *ip = new WFSynth;
	ip->im = m;
	if (tmplt)
		ip->Copy((WFSynth *) tmplt);
	return ip;
}

SeqEvent *WFSynth::WFSynthEventFactory(Opaque tmplt)
{
	VarParamEvent *evt = new VarParamEvent;
	evt->maxParam = 20;
	return (SeqEvent*)evt;
}

VarParamEvent *WFSynth::AllocParams()
{
	return (VarParamEvent *)WFSynthEventFactory(0);
}

static InstrParamMap wfsynthParams[] =
{
	{"envar", 19},
	{"envrr", 20},
	{"wvfid", 16},
	{"wvflp", 17},
	{"wvfpa", 18},
};

bsInt16 WFSynth::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, wfsynthParams, sizeof(wfsynthParams)/sizeof(InstrParamMap));
}

const char *WFSynth::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, wfsynthParams, sizeof(wfsynthParams)/sizeof(InstrParamMap));
}

WFSynth::WFSynth()
{
	im = NULL;
	samples = &dummy;
	sampleNumber = 0;
	sampleTotal = 0;
	sampleIncr = 1.0;
	sampleRel = 0;
	looping = 0;
	playAll = 0;
	fileID = -1;
	eg.SetAtkRt(0.0);
	eg.SetRelRt(0.0);
	eg.SetSus(1.0);
	eg.SetSusOn(1);
	memset(wfUsed, 0, sizeof(wfUsed));
}

WFSynth::~WFSynth()
{
}

void WFSynth::Copy(WFSynth *tp)
{
	fileID = tp->fileID;
	sampleTotal = tp->sampleTotal;
	sampleNumber = tp->sampleNumber;
	sampleIncr = tp->sampleIncr;
	sampleRel = tp->sampleRel;
	samples = tp->samples;
	looping = tp->looping;
	playAll = tp->playAll;
	eg.Copy(&tp->eg);
}

void WFSynth::Start(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
	
	samples = &dummy;
	sampleNumber = 0;
	sampleIncr = 1;
	sampleTotal = 0;

	WaveFileIn *wfp = &wfCache[0];
	WaveFileIn *wfe = &wfCache[WFSYNTH_MAX_WAVEFILES];
	while (wfp < wfe)
	{
		if (wfp->GetFileID() == fileID)
		{
			samples = wfp->GetSampleBuffer();
			sampleTotal = wfp->GetInputLength();
			sampleIncr = (PhsAccum) wfp->GetSampleRate() / (PhsAccum) synthParams.sampleRate;
			break;
		}
		wfp++;
	}
	
	if (sampleTotal > 0)
		sampleRel = sampleTotal - PhsAccum(eg.GetRelRt() * synthParams.sampleRate);

	eg.Reset(0);
}

void WFSynth::Param(SeqEvent *evt)
{
	SetParams((VarParamEvent*)evt);
}

int WFSynth::SetParams(VarParamEvent *params)
{
	int err = 0;

	chnl = params->chnl;
	eg.SetSus(params->vol);

	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int WFSynth::SetParam(bsInt16 id, float v)
{
	switch (id)
	{
	case 16:
		fileID = (bsInt16) v;
		break;
	case 17:
		looping = (bsInt16) v;
		break;
	case 18:
		playAll = (bsInt16) v;
		break;
	case 19:
		eg.SetAtkRt(FrqValue(v));
		break;
	case 20:
		eg.SetRelRt(FrqValue(v));
		break;
	default:
		return 1;
	}
	return 0;
}

int WFSynth::GetParams(VarParamEvent *params)
{
	params->SetParam(P_VOLUME, (float)eg.GetSus());
	params->SetParam(16, (float) fileID);
	params->SetParam(17, (float) looping);
	params->SetParam(18, (float) playAll);
	params->SetParam(19, (float) eg.GetAtkRt());
	params->SetParam(20, (float) eg.GetRelRt());
	return 0;
}

int WFSynth::GetParam(bsInt16 id, float *val)
{
	switch (id)
	{
	case 16:
		*val = (float) fileID;
		break;
	case 17:
		*val = (float) looping;
		break;
	case 18:
		*val = (float) playAll;
		break;
	case 19:
		*val = (float) eg.GetAtkRt();
		break;
	case 20:
		*val = (float) eg.GetRelRt(); 
		break;
	default:
		return 1;
	}
	return 0;
}

void WFSynth::Stop()
{
	if (looping || !playAll)
		eg.Release();
}

void WFSynth::Tick()
{
	if (sampleNumber >= sampleTotal)
	{
		if (!looping)
			return;
		sampleNumber -= sampleTotal;
	}
	im->Output(chnl, samples[(int)sampleNumber] * eg.Gen());
	sampleNumber += sampleIncr;
	if (!looping && playAll && sampleNumber > sampleRel)
		eg.Release();
}

int  WFSynth::IsFinished()
{
	if (!looping && sampleNumber >= sampleTotal)
		return 1;
	return eg.IsFinished();
}

void WFSynth::Destroy()
{
	delete this;
}

int WFSynth::Load(XmlSynthElem *parent)
{
	float atk;
	float rel;
	short ival;

	memset(wfUsed, 0, sizeof(wfUsed));

	XmlSynthElem *elem;
	XmlSynthElem *next = parent->FirstChild();
	while ((elem = next) != NULL)
	{
		if (elem->TagMatch("wvf"))
		{
			if (elem->GetAttribute("fn", ival) == 0)
				fileID = (bsInt16) ival;
			if (elem->GetAttribute("lp", ival) == 0)
				looping = (bsInt16) ival;
			if (elem->GetAttribute("pa", ival) == 0)
				playAll = (bsInt16) ival;
		}
		else if (elem->TagMatch("env"))
		{
			elem->GetAttribute("ar", atk);
			elem->GetAttribute("rr", rel);
			eg.InitAR(atk, 1.0, rel, 1, linSeg);
		}
		else if (elem->TagMatch("file"))
		{
			char *filename = 0;
			if (elem->GetAttribute("name", &filename) == 0)
			{
				if (elem->GetAttribute("id", ival) == 0)
				{
					ival = AddToCache(filename, (bsInt16) ival);
					if (ival >= 0)
						wfUsed[ival] = 1;
				}
				delete filename;
			}
		}

		next = elem->NextSibling();
		delete elem;
	}
	return 0;
}

int WFSynth::Save(XmlSynthElem *parent)
{
	XmlSynthElem *elem;

	elem = parent->AddChild("wvf");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("fn", (long) fileID);
	elem->SetAttribute("lp", (short) looping);
	elem->SetAttribute("pa", (short) playAll);
	delete elem;

	elem = parent->AddChild("env");
	if (elem == NULL)
		return -1;
	elem->SetAttribute("ar", eg.GetAtkRt());
	elem->SetAttribute("rr", eg.GetRelRt());
	delete elem;

	for (int n = 0; n < WFSYNTH_MAX_WAVEFILES; n++)
	{
		short id = (short) wfCache[n].GetFileID();
		if (id >= 0 && wfUsed[n])
		{
			elem = parent->AddChild("file");
			if (elem == NULL)
				return -1;
			elem->SetAttribute("name", wfCache[n].GetFilename());
			elem->SetAttribute("id", id);
			delete elem;
		}
	}
	return 0;
}
