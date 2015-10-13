//////////////////////////////////////////////////////////////////////
/// @file MixerControl.h BasicSynth dynamic mixer control
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef MIXER_CONTROL_H
#define MIXER_CONTROL_H

class MixerControl : public InstrumentVP
{
private:
	bsInt16 func;
	bsInt16 inChnl;
	bsInt16 fxUnit;
	bsInt16 panType;
	bsInt16 oscOn;
	bsInt32 tickCount;
	AmpValue frLvl;
	AmpValue toLvl;
	AmpValue swpRng;
	AmpValue swpOffs;
	FrqValue tmSec;
	EnvSegLin rmp;
	GenWave32 osc;

	InstrManager *im;
	Mixer *mix;

public:
	MixerControl();
	~MixerControl();

	static Instrument *MixerControlFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *MixerControlEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(MixerControl *tp);
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
	int GetParam(bsInt16 idval, float* val);
	int SetParams(VarParamEvent *params);
	int SetParam(bsInt16 idval, float val);
};
//@}
#endif
