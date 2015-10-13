//////////////////////////////////////////////////////////////////////
/// @file LFO.h BasicSynth LFO: Low Frequency Oscillator
//
// Used for vibrato and similar effects.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_LFO_H_)
#define _LFO_H_

class LFO : public GenUnit
{
private:
	GenWave32 osc;
	EnvSegExp atk;
	AmpValue depth;
	AmpValue ampLvl;
	FrqValue sigFrq;
	FrqValue atkRt;
	int lfoOn;

public:
	LFO();
	virtual ~LFO();
	void Copy(LFO *tp);
	
	void SetFrequency(FrqValue frq) { osc.SetFrequency(frq); }
	void SetWavetable(int wt)       { osc.SetWavetable(wt); }
	void SetAttack(FrqValue val)    { atkRt = val; }
	void SetLevel(AmpValue val)     { depth = val; lfoOn = depth > 0; }
	void SetSigFrq(FrqValue val)    { sigFrq = val; }
	FrqValue GetFrequency() { return osc.GetFrequency(); }
	int GetWavetable() { return osc.GetWavetable(); }
	FrqValue GetAttack() { return atkRt; }
	AmpValue GetLevel() { return depth; }

	void Init(int n, float *f);
	void InitLFO(FrqValue frq, int wvf, FrqValue rt, AmpValue amp, FrqValue sig);
	void GetSettings(FrqValue &frq, int &wvf, FrqValue& rt, AmpValue& amp);
	void Reset(float initPhs = 0);

	AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	AmpValue Gen()
	{
		return atk.Gen() * osc.Gen() * ampLvl;
	}


	int On() { return lfoOn; }
	int Load(XmlSynthElem *elem);
	int Save(XmlSynthElem *elem);
};
//@}
#endif
