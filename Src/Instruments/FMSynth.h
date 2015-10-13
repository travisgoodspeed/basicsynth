//////////////////////////////////////////////////////////////////////
/// @file FMSynth.h BasicSynth FM Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_FMSYNTH_H_)
#define _FMSYNTH_H_

#include "LFO.h"
#include "PitchBend.h"

#define ALG_STACK 1
#define ALG_STACK2 2
#define ALG_WYE 3
#define ALG_DELTA 4

class FMSynth : public InstrumentVP
{
private:
#ifdef USE_OSCILI
	GenWaveI gen1Osc;
#else
	GenWaveWT gen1Osc;
#endif
	EnvGenADSR gen1EG;
	EnvDef     gen1EnvDef;
	FrqValue   gen1Mult;
	AmpValue   fmMix;
	AmpValue   fmDly;
	long algorithm;
	int gen1Wt;

#ifdef USE_OSCILI
	GenWaveI gen2Osc;
#else
	GenWaveWT gen2Osc;
#endif
	EnvGenADSR gen2EG;
	EnvDef     gen2EnvDef;
	FrqValue   gen2Mult;
	int gen2Wt;

#ifdef USE_OSCILI
	GenWaveI gen3Osc;
#else
	GenWaveWT gen3Osc;
#endif
	EnvGenADSR gen3EG;
	EnvDef     gen3EnvDef;
	FrqValue   gen3Mult;
	int gen3Wt;

	GenNoiseI  nzi;
	GenWaveWT  nzo;
	EnvGenADSR nzEG;
	EnvDef     nzEnvDef;
	FrqValue   nzFrqh;
	FrqValue   nzFrqo;
	AmpValue   nzMix;
	AmpValue   nzDly;
	Panner   panSet;

	AllPassDelay apd;
	FrqValue   dlyTim;
	FrqValue   dlyDec;
	AmpValue   dlyMix;
	long dlySamps;

	LFO lfoGen;
	PitchBend pbGen;
	PitchBendWT pbWT;

	int panOn;
	int nzOn;
	int dlyOn;
	int pbOn;

	int chnl;
	FrqValue frq;
	AmpValue vol;
	AmpValue maxPhs;

	InstrManager *im;

	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult);
	void LoadEG(XmlSynthElem *elem, EnvDef& eg);
	XmlSynthElem *SaveEG(XmlSynthElem *parent, const char *tag, EnvDef& eg);

public:
	FMSynth();
	FMSynth(FMSynth* tp);
	virtual ~FMSynth();
	static Instrument *FMSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *FMSynthEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(FMSynth *tp);
	void Start(SeqEvent *evt);
	void Param(SeqEvent *evt);
	void Stop();
	void Tick();
	int  IsFinished();
	void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	VarParamEvent *AllocParams();
	int GetParams(VarParamEvent *params);
	int GetParam(bsInt16 id, float *val);
	int SetParams(VarParamEvent *params);
	int SetParam(bsInt16 id, float val);
};
//@}
#endif
