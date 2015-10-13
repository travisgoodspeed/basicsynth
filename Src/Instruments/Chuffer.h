//////////////////////////////////////////////////////////////////////
/// @file Chuffer.h BasicSynth Noise synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#if !defined(_CHUFFER_H_)
#define _CHUFFER_H_

class Chuffer  : public InstrumentVP
{
private:
	int chnl;
	AmpValue vol;
	FrqValue frq;
	FrqValue dur;

	GenNoiseH nz;
	FrqValue nzSampl;

	EnvGenAR envSig;
	bsInt16  envTrackDur;
	bsInt16  envSusOn;
	FrqValue envAtk;
	FrqValue envRel;

	GenWave32 modOsc;
	bsInt16  modOn;
	FrqValue modFrq;

	GenWave32 swpOsc;
	bsInt16  swpOn;
	FrqValue swpFrq;
	AmpValue swpAmp;
	int      swpWT;

	GenWave32 chpOsc;
	bsInt16  chpOn;
	FrqValue chpFrq;
	AmpValue chpAmp;
	int      chpWT;

	FilterIIR2p filt;
	bsInt16  fltOn;
	bsInt16  fltFixed;
	bsInt16  fltTrackFrq;
	bsInt16  fltTrackDur;
	bsInt16  fltTrackMul;
	bsInt16  resFixed;
	bsInt16  resTrackDur;
	bsInt16  resTrackMul;

	EnvSegLin envFlt;
	AmpValue fltStart;
	AmpValue fltEnd;
	FrqValue fltDur;
	float    fltMul;

	EnvSegLin envRes;
	AmpValue resStart;
	AmpValue resEnd;
	AmpValue resDur;
	float    resMul;

	bsInt32  coefRate;
	bsInt32  coefCount;
	AmpValue lastFc;
	AmpValue lastQ;

	InstrManager *im;

	void SetDefaults();

public:
	Chuffer();
	Chuffer(Chuffer *tp);
	virtual ~Chuffer();
	static Instrument *ChufferFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *ChufferEventFactory(Opaque tmplt);
	static bsInt16    MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(Chuffer *tp);
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
	int SetParams(VarParamEvent *params);
	int SetParam(bsInt16 idval, float val);
	int GetParam(bsInt16 idval, float *val);
};
//@}
#endif
