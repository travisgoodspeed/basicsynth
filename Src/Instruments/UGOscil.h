//////////////////////////////////////////////////////////////////////
/// @file UGOscil.h Oscillators (and noise generators) for ModSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef UG_OSCIL_H
#define UG_OSCIL_H

extern UGParam oscilParams[];
extern UGParam oscil2Params[];
extern UGParam buzzParams[];
extern UGParam noiseParams[];

#define UGOSC_AMP  0
#define UGOSC_FRQ  1
#define UGOSC_MUL  2
#define UGOSC_MOD  3
#define UGOSC_WVT  4
#define UGOSC_IOM  5

class UGOscil : public ModSynthUGImpl<UGOscil, GenWaveWT, 5>
{
public:
	typedef ModSynthUGImpl<UGOscil, GenWaveWT, 5> base;

	const UGParam *GetParamList()
	{
		return oscilParams;
	}

	const char *GetType()
	{
		return "OSCIL";
	}

	UGOscil()
	{
	}

	void InitDefault()
	{
		inputs[UGOSC_AMP] = 1.0;    // amp
		inputs[UGOSC_FRQ] = 440.0;  // frq
		inputs[UGOSC_MUL] = 1.0;    // mul
		inputs[UGOSC_MOD] = 0.0;    // mod
		inputs[UGOSC_WVT] = 0.0;    // wt
	}

	void Start()
	{
		gen.InitWT(FrqValue(inputs[UGOSC_FRQ] * inputs[UGOSC_MUL]), (int)inputs[UGOSC_WVT]);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange & ((1<<UGOSC_FRQ)|(1<<UGOSC_MUL)))
		{
			gen.SetFrequency(inputs[UGOSC_FRQ] * inputs[UGOSC_MUL]);
			gen.Reset(-1);
		}
		if (anyChange & ((1<<UGOSC_MUL)|(1<<UGOSC_MOD)))
			gen.PhaseModWT(synthParams.frqTI * inputs[UGOSC_MOD] * inputs[UGOSC_MUL]);
		anyChange = 0;
		base::Tick();
	}
};

class UGOscilI : public ModSynthUGImpl<UGOscilI, GenWaveI, 5>
{
public:
	typedef ModSynthUGImpl<UGOscilI, GenWaveI, 5> base;

	const UGParam *GetParamList()
	{
		return oscilParams;
	}

	const char *GetType()
	{
		return "OSCILI";
	}

	UGOscilI()
	{
	}

	void InitDefault()
	{
		inputs[UGOSC_AMP] = 1.0;    // amp
		inputs[UGOSC_FRQ] = 440.0;  // frq
		inputs[UGOSC_MUL] = 1.0;    // mul
		inputs[UGOSC_MOD] = 0.0;    // mod
		inputs[UGOSC_WVT] = 0.0;    // wt
	}

	void Start()
	{
		gen.InitWT(FrqValue(inputs[UGOSC_FRQ] * inputs[UGOSC_MUL]), (int)inputs[UGOSC_WVT]);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange & ((1<<UGOSC_FRQ)|(1<<UGOSC_MUL)))
		{
			gen.SetFrequency(inputs[UGOSC_FRQ] * inputs[UGOSC_MUL]);
			gen.Reset(-1);
		}
		if (anyChange & ((1<<UGOSC_MUL)|(1<<UGOSC_MOD)))
			gen.PhaseModWT(synthParams.frqTI * inputs[UGOSC_MOD] * inputs[UGOSC_MUL]);
		anyChange = 0;
		base::Tick();
	}
};

class UGOscilFM : public ModSynthUGImpl<UGOscilFM, GenWaveFM, 6>
{
public:
	typedef ModSynthUGImpl<UGOscilFM, GenWaveFM, 6> base;

	const UGParam *GetParamList()
	{
		return oscil2Params;
	}

	const char *GetType()
	{
		return "OSCILFM";
	}

	UGOscilFM()
	{
	}

	void InitDefault()
	{
		inputs[UGOSC_AMP] = 1.0;    // amp
		inputs[UGOSC_FRQ] = 440.0;  // frq
		inputs[UGOSC_MUL] = 1.0;    // mul
		inputs[UGOSC_MOD] = 0.0;    // mod
		inputs[UGOSC_WVT] = 0.0;    // wt
		inputs[UGOSC_IOM] = 1.0;    // index of modulation
	}

	void Start()
	{
		gen.InitFM(inputs[UGOSC_FRQ], inputs[UGOSC_MUL], inputs[UGOSC_IOM], (int)inputs[UGOSC_WVT]);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange)
		{
			gen.SetModMultiple(inputs[UGOSC_MUL]);
			gen.SetFrequency(inputs[UGOSC_FRQ]+inputs[UGOSC_MOD]);
			gen.SetModIndex(inputs[UGOSC_IOM]);
			gen.Reset(-1);
		}
		anyChange = 0;
		base::Tick();
	}
};

class UGOscilAM : public ModSynthUGImpl<UGOscilAM, GenWaveAM, 5>
{
public:
	typedef ModSynthUGImpl<UGOscilAM, GenWaveAM, 5> base;

	const UGParam *GetParamList()
	{
		return oscilParams;
	}

	const char *GetType()
	{
		return "OSCILAM";
	}

	UGOscilAM()
	{
		inputs[UGOSC_AMP] = 1.0;
	}

	void InitDefault()
	{
		inputs[UGOSC_AMP] = 1.0;    // amp
		inputs[UGOSC_FRQ] = 440.0;  // frq
		inputs[UGOSC_MUL] = 1.0;    // mul
		inputs[UGOSC_MOD] = 0.0;    // mod
		inputs[UGOSC_WVT] = 0.0;    // wt
	}

	void Start()
	{
		gen.InitAM(inputs[UGOSC_FRQ],
			       inputs[UGOSC_FRQ]*inputs[UGOSC_MUL],
				   inputs[UGOSC_MOD],
				   (int)inputs[UGOSC_WVT]);
		anyChange = 0;
	}
};

class UGBuzz : public ModSynthUGImpl<UGBuzz, GenWaveBuzz, 4>
{
public:
	typedef ModSynthUGImpl<UGBuzz, GenWaveBuzz, 4> base;

	const UGParam *GetParamList()
	{
		return buzzParams;
	}

	const char *GetType()
	{
		return "BUZZ";
	}

	UGBuzz()
	{
	}

	void InitDefault()
	{
		inputs[UGOSC_AMP] = 1.0;    // amp
		inputs[UGOSC_FRQ] = 1.0;  // frq
		inputs[UGOSC_MUL] = 10.0;    // num harmonics
		inputs[UGOSC_MOD] = 0.0;    // mod
	}

	void Start()
	{
		gen.InitBuzz(FrqValue(inputs[UGOSC_FRQ]), (bsInt32)inputs[UGOSC_MUL]);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange & ((1<<UGOSC_FRQ)|(1<<UGOSC_MUL)))
		{
			gen.SetFrequency(inputs[UGOSC_FRQ]);
			gen.SetHarmonics((bsInt32)inputs[UGOSC_MUL]);
			gen.Reset(-1);
		}
		if (anyChange & (1<<UGOSC_MOD))
			gen.Modulate(inputs[UGOSC_MOD]);
		anyChange = 0;
		out = gen.Sample(inputs[0]);
		Send(out, UGP_GEN);
	}
};

#define UGNZ_AMP  0
#define UGNZ_RTE  1

class UGRandH : public ModSynthUGImpl<UGRandH, GenNoiseH, 2>
{
public:
	typedef ModSynthUGImpl<UGRandH, GenNoiseH, 2> base;

	const UGParam *GetParamList()
	{
		return noiseParams;
	}

	const char *GetType()
	{
		return "RANDH";
	}

	UGRandH()
	{
	}

	void InitDefault()
	{
		inputs[UGNZ_AMP] = 1.0;  // amp
		inputs[UGNZ_RTE] = 1.0;  // rate
	}

	void Start()
	{
		gen.InitH(inputs[UGNZ_RTE]*synthParams.sampleRate);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange & (1<<UGNZ_AMP))
		{
			gen.InitH(inputs[UGNZ_AMP]*synthParams.sampleRate);
			anyChange = 0;
		}
		base::Tick();
	}
};

class UGRandI : public ModSynthUGImpl<UGRandI, GenNoiseI, 2>
{
public:
	typedef ModSynthUGImpl<UGRandI, GenNoiseI, 2> base;

	const UGParam *GetParamList()
	{
		return noiseParams;
	}

	const char *GetType()
	{
		return "RANDI";
	}

	UGRandI()
	{
	}

	void InitDefault()
	{
		inputs[UGNZ_AMP] = 1.0;  // amp
		inputs[UGNZ_RTE] = 1.0;  // rate
	}

	void Start()
	{
		gen.InitH(inputs[UGNZ_RTE]*synthParams.sampleRate);
		anyChange = 0;
	}

	void Tick()
	{
		if (anyChange & (1<<UGNZ_AMP))
		{
			gen.InitH(inputs[UGNZ_AMP]*synthParams.sampleRate);
			anyChange = 0;
		}
		base::Tick();
	}
};
//@}
#endif
