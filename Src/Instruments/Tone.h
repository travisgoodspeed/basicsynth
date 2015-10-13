//////////////////////////////////////////////////////////////////////
/// @file Tone.h BasicSynth Tone instruments
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_TONE_H_)
#define _TONE_H_

#include "LFO.h"
#include "PitchBend.h"

class ToneBase : public InstrumentVP
{
protected:
	int chnl;          ///< output channel
	int pbOn;          ///< pitch modification is enabled
	AmpValue vol;      ///< overall volume level
	FrqValue frq;      ///< base frequency
	FrqValue pwFrq;    ///< MIDI pitch wheel frequency delta
	GenWaveWT *osc;    ///< oscillator
	EnvGenADSR env;    ///< envelope
	LFO lfoGen;        ///< LFO (vibrato)
	PitchBend pbGen;   ///< Pitch bend generator
	PitchBendWT pbWT;  ///< Pitch bend wavetable

	InstrManager *im;

public:
	ToneBase();
	ToneBase(ToneBase *tp);
	virtual ~ToneBase();
	virtual void Copy(ToneBase *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);
	virtual int GetParams(VarParamEvent *params);

	virtual int GetParam(bsInt16 id, float* val);
	virtual int SetParams(VarParamEvent *params);
	virtual int SetParam(bsInt16 id, float val);

	virtual int LoadOscil(XmlSynthElem *elem);
	virtual int LoadEnv(XmlSynthElem *elem);
	virtual int SaveOscil(XmlSynthElem *elem);
	virtual int SaveEnv(XmlSynthElem *elem);
};

class ToneInstr : public ToneBase
{
public:
	static Instrument *ToneFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	ToneInstr();
	ToneInstr(ToneInstr *tp);
	virtual ~ToneInstr();
	VarParamEvent *AllocParams();
};

class ToneFM : public ToneBase
{
public:
	static Instrument *ToneFMFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ToneFMEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	ToneFM();
	ToneFM(ToneFM *tp);
	virtual ~ToneFM();
	virtual void Copy(ToneFM *tp);
	virtual int LoadOscil(XmlSynthElem *elem);
	virtual int SaveOscil(XmlSynthElem *elem);
	virtual int SetParam(bsInt16 id, float val);
	virtual int GetParam(bsInt16 id, float* val);
	VarParamEvent *AllocParams();

	int GetParams(VarParamEvent *params);
};
//@}
#endif
