//////////////////////////////////////////////////////////////////////
/// @file BuzzSynth.h BasicSynth Instrument using GenWaveBuzz
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_BUZZSYNTHINSTR_H_)
#define _BUZZSYNTHINSTR_H_

#include "LFO.h"
#include "PitchBend.h"

class BuzzSynth;

#define BUZZ_MFILT  0x001
#define BUZZ_MHARM  0x002
#define BUZZ_FILTER 0x010
#define BUZZ_RELFRQ 0x020
#define BUZZ_ENABLE 0x100

///////////////////////////////////////////////////////////////
/// @brief One generator for the BuzzSynth class.
/// @details BuzzPart aggregates a variable spectrum oscillator,
/// lowpass filter, modulation envelope and amplitude
/// envelope. Envelopes are A3SR types.
///
/// The spectrum is controlled by three factors:
/// 1) Number of harmonics
/// 2) Frequency ratio between harmonics
/// 3) Amplitude scale of harmonics
/// The cumulative amplitude scale must be 0 < a < 1.
///
/// The modulation envelope can be applied to the amplitude scale and/or filter.
/// Its output is normalized to the range [0,1], scaled, and added to
/// a base value. For filter frequency, base and scale are specified as cents above
/// the oscillator frequency (relative mode) or above the lowest pitch (absolute mode). 
/// @code
///    fc = fo * 2 ^ (base + (env * scale))/1200
/// @endcode
/// For amplitude scaling:
/// @code
///    amp = base + (env * scale)
/// @endcode
/// @sa GenWaveDSB
///////////////////////////////////////////////////////////////
class BuzzPart
{
protected:
	GenWaveDSB osc;
	FilterIIR2p flt;
	EnvGenSegSus envSig;
	EnvGenSegSus envMod;

	FrqValue fltBase;
	FrqValue fltScl;
	FrqValue fltQ;
	FrqValue fltFreq;
	FrqValue fltLast;

	FrqValue frqBase;
	FrqValue frqScl;
	FrqValue frqMult;
	FrqValue frqRatio;
	AmpValue volume;
	AmpValue ampBase;
	AmpValue ampScl;
	AmpValue ampLast;
	bsInt32  harmMax;
	unsigned int modOn;

	BuzzPart()
	{
		volume = 1.0;
		frqBase = 440.0;
		frqScl = 0.0;
		frqMult = 1.0;
		frqRatio = 1.0;
		fltBase = 0.0;
		fltScl = 0.0;
		fltFreq = 2400.0;
		fltQ = 1.0;
		fltLast = 0;
		ampBase = 0.5;
		ampScl = 0.0;
		ampLast = 0;
		harmMax = 100;
		modOn = BUZZ_ENABLE|BUZZ_FILTER|BUZZ_RELFRQ;
		envSig.SetSegs(4);
		envSig.SetSusOn(1);
		envSig.SetLevel(1, 1.0);
		envSig.SetLevel(2, 1.0);
		envMod.SetSegs(4);
	}

	void Copy(BuzzPart *tp)
	{
		fltBase = tp->fltBase;
		fltScl = tp->fltScl;
		fltQ = tp->fltQ;
		fltFreq = tp->fltFreq;
		fltLast = 0;
		frqBase = tp->frqBase;
		frqScl = tp->frqScl;
		frqRatio = tp->frqRatio;
		volume = tp->volume;
		ampBase = tp->ampBase;
		ampScl = tp->ampScl;
		ampLast = 0;
		harmMax = tp->harmMax;
		modOn = tp->modOn;
		envSig.Copy(&tp->envSig);
		envMod.Copy(&tp->envMod);
	}

	void Start()
	{
		frqMult = synthParams.GetCentsMult((int)frqScl);
		osc.InitDSB(frqBase * frqMult, frqRatio, harmMax, ampBase);
		if (modOn & BUZZ_RELFRQ)
			fltFreq = osc.GetFrequency();
		else
			fltFreq = synthParams.GetFrequency(0);
		fltLast = fltFreq * synthParams.GetCentsMult((int)fltBase);
		flt.CalcCoef(fltLast, fltQ);
		flt.Reset();
		envSig.Reset();
		envMod.Reset();
	}

	void Stop()
	{
		envSig.Release();
		envMod.Release();
	}

	inline void Modulate(FrqValue f)
	{
		osc.Modulate(frqMult * f);
	}

	inline void Reset()
	{
		frqMult = synthParams.GetCentsMult((int)frqScl);
		osc.SetFrequency(frqBase * frqMult);
		if (!(modOn & BUZZ_MHARM))
			osc.SetRatio(ampBase);
		osc.Reset(-1);
		fltLast = 0.0;
		ampLast = 0.0;
	}

	AmpValue Gen()
	{
		if (!(modOn & BUZZ_ENABLE))
			return 0.0;

		AmpValue modVal = envMod.Gen();
		if (modOn & BUZZ_MFILT)
		{
			FrqValue fc = fltFreq * synthParams.GetCentsMult((int)(fltBase + (modVal * fltScl)));
			if (fc != fltLast)
				flt.CalcCoef(fltLast = fc, fltQ);
		}
		if (modOn & BUZZ_MHARM)
		{
			AmpValue amp = ampBase + (modVal * ampScl);
			if (amp != ampLast)
				osc.ModRatio(ampLast = amp);
		}
		AmpValue out = (AmpValue) osc.Gen2();
		if (modOn & BUZZ_FILTER)
			out = flt.Sample(out);
		return out * envSig.Gen() * volume;
	}

	inline int IsFinished()
	{
		if (!(modOn & BUZZ_ENABLE))
			return 1;
		return envSig.IsFinished();
	}

	int Save(XmlSynthElem *elem);
	int Load(XmlSynthElem *elem);
	int SaveEnv(XmlSynthElem *elem, EnvGenSegSus *env);
	int LoadEnv(XmlSynthElem *elem, EnvGenSegSus *env);

	friend class BuzzSynth;
};

// Define for two generators. 
#define BUZZ_NGEN 2

////////////////////////////////////////////////////////////////////////////////////
/// @brief BuzzSynth implements a two-oscillator subtractive synthesis instrument.
/// @details This instrument contains two bandwidth-limited pulse wave oscillators,
/// each with its own filter, modulation envelope, amplitude envelope, and
/// LFO and pitch bend.
///
/// The number of generators is currently hard-coded but can be made variable 
/// by dynamic allocation of the buzz member and adjusting the code appropriately.
/// Typically, two generators are good enough. One generator produces the main signal
/// with the second adding transients, swell, or doubling.
////////////////////////////////////////////////////////////////////////////////////
class BuzzSynth  : public InstrumentVP
{
private:
	BuzzPart buzz[BUZZ_NGEN];
	LFO lfoGen;
	PitchBendWT pbWT;
	AmpValue vol;
	FrqValue frq;
	FrqValue pwFrq;
	int chnl;
	int modOn;
	InstrManager *im;

public:
	BuzzSynth();
	BuzzSynth(BuzzSynth *tp);
	virtual ~BuzzSynth();
	static Instrument *InstrFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *EventFactory(Opaque tmplt);
	static bsInt16    MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(BuzzSynth *tp);
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
