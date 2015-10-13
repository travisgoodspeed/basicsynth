//////////////////////////////////////////////////////////////////////
/// @file AddSynth.h BasicSynth Additive Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_ADDSYNTH_H_)
#define _ADDSYNTH_H_

#include "LFO.h"

/// @brief One part of an AddSynth instrument.
/// @details Each part contains an oscillator and envelope generator.
/// The frequency is typically set as a multiple of the base frequency for
/// the instrument. However, it is possible to set the frequency directly
/// for each part.
struct AddSynthPart
{
#ifdef USE_OSCILI
	GenWaveI osc; //<-- if you prefer interpolation
#else
	GenWaveWT osc;
#endif
	EnvGenSegSus env;
	FrqValue  mul; // frequency multiplier

	AddSynthPart()
	{
		mul = 1.0;
	}

	/// Make a copy of the settings.
	/// @param p object to copy from.
	void Copy(AddSynthPart *p)
	{
		mul = p->mul;
		osc.SetFrequency(p->osc.GetFrequency());
		osc.SetWavetable(p->osc.GetWavetable());
		env.Copy(&p->env);
	}
};

/// @brief Implements an additive synthesis instrument.
/// @details AddSynth contains a dynamic array of AddSynthPart
/// objects, each of which functions as a semi-independent
/// instrument. The output of AddSynth is the sum of the
/// output of all partials. Typically each part is set
/// to an integer frequency multiple to create a sum of
/// harmonics. However, this instrument can also be used
/// to cross-fade between waveforms, implementing a form
/// of wavetable synthesis.
///
/// Parameter ids for AddSynth contain 3 fields:
/// @code
/// [pn(6)][sn(4)][val(4)]
/// PN      (partial number + 1) * 256
/// SN      segment number * 16
/// PN+0    Frequency multiplier. 
/// PN+1    Frequency for oscillator.
/// PN+2    Wave table index.
/// PN+3    Starting value for the envelope.
/// PN+4    Sustain-on flag, 1 or 0.
/// PN+SN+5	Envelope segment time
/// PN+SN+6	Level at the end of the segment.
/// PN+SN+7	Segment curve type: 1=linear 2=exponential 3=log 4=squared.
/// PN+SN+8 Fixed (1) or relative (0) time
/// @endcode
/// For example, to set the wavetable on oscillator 2, use
/// @code
/// SetParam((2 << 8)+2, index)
/// @endcode
class AddSynth : public InstrumentVP
{
private:
	int chnl;
	AmpValue vol;
	FrqValue frq;
	AddSynthPart *parts;
	int numParts;
	LFO lfoGen;

	InstrManager *im;

	/// Internal function used to update instrument parameters during playback.
	void UpdateParams(SeqEvent *evt, float initPhs);

public:
	AddSynth();
	AddSynth(AddSynth *tp);
	virtual ~AddSynth();

	/// @copydoc InstrFactory
	static Instrument *AddSynthFactory(InstrManager *, Opaque tmplt);
	/// @copydoc EventFactory
	static SeqEvent   *AddSynthEventFactory(Opaque tmplt);
	/// @copydoc ParamID
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	/// @copydoc ParamName
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	/// Copy the settings from the template.
	/// @param tp template object
	void Copy(AddSynth *tp);
	/// @copydoc Instrument::Start
	virtual void Start(SeqEvent *evt);
	/// @copydoc Instrument::Param
	virtual void Param(SeqEvent *evt);
	/// @copydoc Instrument::Stop
	virtual void Stop();
	/// @copydoc Instrument::Tick
	virtual void Tick();
	/// @copydoc Instrument::IsFinished
	virtual int  IsFinished();
	/// @copydoc Instrument::Destroy
	virtual void Destroy();

	/// @copydoc Instrument::Load
	int Load(XmlSynthElem *parent);
	/// @copydoc Instrument::Save
	int Save(XmlSynthElem *parent);
	/// @copydoc InstrumentVP::AllocParams
	VarParamEvent *AllocParams();
	/// @copydoc InstrumentVP::GetParams
	int GetParams(VarParamEvent *params);
	/// @copydoc InstrumentVP::SetParams
	int SetParams(VarParamEvent *params);
	/// @copydoc InstrumentVP::SetParam
	int SetParam(bsInt16 idval, float val);
	/// @copydoc InstrumentVP::GetParam
	int GetParam(bsInt16 idval, float *val);

	/// Set the number of parts.
	/// The parts are accessed via SetParam()
	/// @param n number of parts to allocate
	int SetNumParts(int n);
	/// Get the number of parts
	int GetNumParts();
	/// Get direct access to a part.
	AddSynthPart *GetPart(int n);
};
//@}
#endif
