//////////////////////////////////////////////////////////////////////
/// @file SubSynth.h BasicSynth Subtractive synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_SUBSYNTHINSTR_H_)
#define _SUBSYNTHINSTR_H_

#include "LFO.h"
#include "PitchBend.h"

class SubFilt
{
protected:
	EnvGenADSR *egFilt;
	AmpValue res;
	AmpValue gain;
	bsInt32 coefRate;
	bsInt32 coefCount;
public:
	SubFilt()
	{
		egFilt = 0;
		res = 0;
		gain = 1.0;
		coefRate = 0;
		coefCount = 0;
	}
	virtual ~SubFilt() { }
	virtual AmpValue Sample(AmpValue in) { return in; }
	virtual void Init(EnvGenADSR *eg, AmpValue g, AmpValue r)
	{
		egFilt = eg;
		gain = g;
		res = r;
	}
	virtual void Reset(float ) { }
	virtual void Copy(SubFilt *tp) 
	{ 
		if (tp)
		{
			gain = tp->gain;
			res = tp->res;
			coefRate = tp->coefRate; 
		}
	}
	virtual void SetCalcRate(float f)
	{
		coefRate = (bsInt32) (f * 0.001 * synthParams.sampleRate);
	}
};

class SubFiltLP : public SubFilt
{
private:
	FilterLP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.SetFrequency(f);
			filt.CalcCoef();
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltLP*)tp)->filt);
	}
};

class SubFiltHP : public SubFilt
{
private:
	FilterHP filt;
public:
	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.SetFrequency(f);
			filt.CalcCoef();
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), gain);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltHP*)tp)->filt);
	}
};

class SubFiltBP : public SubFilt
{
private:
	FilterBP filt;
public:
	SubFiltBP()
	{
		res = 1000.0;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.SetFrequency(f);
			filt.CalcCoef();
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (initPhs >= 0)
			filt.Init(egFilt->GetStart(), res, gain);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltBP*)tp)->filt);
	}
};

class SubFiltRES : public SubFilt
{
private:
	Reson filt;
public:
	SubFiltRES()
	{
		res = 0.5;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			filt.SetFrequency(f);
			filt.CalcCoef();
			coefCount = coefRate;
		}
		return filt.Sample(in);
	}
	virtual void Reset(float initPhs)
	{
		if (res >= 1.0)
			res = 0.99999999;
		if (initPhs >= 0)
		{
			filt.SetRes(res);
			filt.SetFrequency(egFilt->GetStart());
			filt.SetGain(gain);
			filt.CalcCoef();
		}
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
		filt.Copy(&((SubFiltRES*)tp)->filt);
	}
};

class SubFiltLPR : public SubFilt
{
private:
	FilterIIR2p filt;

public:
	SubFiltLPR()
	{
		res = 1;
	}

	virtual AmpValue Sample(AmpValue in)
	{
		AmpValue f = egFilt->Gen();
		if (--coefCount <= 0)
		{
			float q = res;
			if (q < 1)
				q = 1;
			filt.CalcCoef(f, res);
			coefCount = coefRate;
		}
		return filt.Sample(in) * gain;
	}

	virtual void Reset(float initPhs)
	{
		float q = res;
		if (q < 1)
			q = 1;
		if (initPhs >= 0)
			filt.CalcCoef(egFilt->GetStart(), q);
		coefCount = coefRate;
		filt.Reset(initPhs);
	}
	virtual void Copy(SubFilt *tp)
	{
		SubFilt::Copy(tp);
//		filt.Copy(&((SubFiltLPR*)tp)->filt);
	}
};

class SubSynth  : public InstrumentVP
{
private:
#ifdef USE_OSCILI
	GenWaveI osc;
#else
	GenWaveWT osc;
#endif
	GenNoise nz;
	EnvGenADSR   envSig;
	SubFilt *filt;
	EnvGenADSR   envFlt;
	LFO lfoGen;
	PitchBend pbGen;
	PitchBendWT pbWT;
	FrqValue frq;
	FrqValue pwFrq;
	AmpValue vol;
	AmpValue sigMix;
	AmpValue nzMix;
	AmpValue fltGain;
	AmpValue fltRes; // only if Reson filter
	short fltType; // 0=LP, 1=HP, 2=BP, 3=RES
	float coefRate;
	int chnl;
	int nzOn;
	int pbOn;
	InstrManager *im;

public:
	SubSynth();
	SubSynth(SubSynth *tp);
	virtual ~SubSynth();
	static Instrument *SubSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *SubSynthEventFactory(Opaque tmplt);
	static bsInt16    MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(SubSynth *tp);
	void CreateFilter();
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	VarParamEvent *AllocParams();
	int GetParams(VarParamEvent *params);
	int GetParam(bsInt16 id, float* val);
	int SetParams(VarParamEvent *params);
	int SetParam(bsInt16 id, float val);
};
//@}
#endif
