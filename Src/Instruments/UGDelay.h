//////////////////////////////////////////////////////////////////////
/// @file UGDelay.h BasicSynth Delay lines for ModSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#ifndef UG_DELAY_H
#define UG_DELAY_H

extern UGParam delayParams[];
extern UGParam delayParamsV[];
extern UGParam flangerParams[];
extern UGParam reverbParams[];

#define UGDLY_INP  0
#define UGDLY_DLY  1
#define UGDLY_DEC  2
#define UGDLY_VOL  3
#define UGDLY_VRT  4
#define UGDLY_MAX  4
#define UGDLY_MAXV 5

/// Template for delay lines
template <class DT, class UG, short IP = 1>
class ModSynthDlyImpl : public ModSynthUGImpl<DT, UG, IP>
{
protected:
	bsInt32 count;
	int stopped;
public:
	ModSynthDlyImpl()
	{
		stopped = 0;
	}

	int IsFinished()
	{
		return count <= 0;
	}

	void SetInput(short index, float value)
	{
		DT *pdt = (DT*)this;
		if (index == UGDLY_INP)
			pdt->inputs[UGDLY_INP] += value;
		else if (index < IP)
			pdt->inputs[index] = value;
		pdt->anyChange |= 1 << index;
	}

	void Stop()
	{
		stopped = 1;
	}

	void Tick()
	{
		DT *pdt = (DT*)this;
		pdt->out = pdt->gen.Sample(pdt->inputs[UGDLY_INP] * pdt->inputs[UGDLY_VOL]);
		pdt->inputs[UGDLY_INP] = 0.0;
		pdt->Send(pdt->out, UGP_GEN);
		count -= stopped;
	}
};

/// @brief Delay line
/// @details The Delay UG implements a plain delay line. The
/// decay parameter represents the attenuation (loss of amplitude)
/// for the whole delay time.
class UGDelay : public ModSynthDlyImpl<UGDelay, DelayLine, UGDLY_MAX>
{
public:
	const UGParam *GetParamList()
	{
		return delayParams;
	}

	const char *GetType()
	{
		return "DELAY";
	}

	UGDelay()
	{
	}

	void InitDefault()
	{
		inputs[UGDLY_DLY] = 0.001;
		inputs[UGDLY_DEC] = 1.0;
		inputs[UGDLY_VOL] = 1.0;
	}

	void Start()
	{
		stopped = 0;
		count = (bsInt32) (inputs[UGDLY_DLY] * synthParams.sampleRate);
		inputs[UGDLY_INP] = 0.0;
		gen.InitDL(inputs[UGDLY_DLY], inputs[UGDLY_DEC]);
		out = 0;
	}

};

/// @brief Feedback delay line
/// @details The DelayR UG implements a feedback delay line. The
/// decay parameter represents the time to decay to -60db
class UGDelayR : public ModSynthDlyImpl<UGDelayR, DelayLineR, UGDLY_MAX>
{
public:
	const UGParam *GetParamList()
	{
		return delayParams;
	}

	const char *GetType()
	{
		return "DELAYR";
	}

	UGDelayR()
	{
	}

	void InitDefault()
	{
		inputs[UGDLY_DLY] = 0.001;
		inputs[UGDLY_DEC] = 0.005;
		inputs[UGDLY_VOL] = 1.0;
	}

	void Start()
	{
		stopped = 0;
		count = (bsInt32) (inputs[UGDLY_DLY] * synthParams.sampleRate);
		inputs[UGDLY_INP] = 0.0f;
		gen.InitDLR(inputs[UGDLY_DLY], inputs[UGDLY_DEC], 0.001, inputs[UGDLY_VOL]);
		out = 0;
	}
};

/// @brief Variable delay line
/// @details The Variable delay UG implements a delay line with a 
/// varying delay time. The delay parameter sets the maximum delay
/// time in seconds. The decay parameter represents the time to
/// decay to -60db
class UGDelayV : public ModSynthDlyImpl<UGDelayV, DelayLineV, UGDLY_MAXV>
{
public:
	const UGParam *GetParamList()
	{
		return delayParamsV;
	}

	const char *GetType()
	{
		return "DELAYV";
	}

	UGDelayV()
	{
	}

	void InitDefault()
	{
		inputs[UGDLY_DLY] = 0.01;
		inputs[UGDLY_DEC] = 0.01;
		inputs[UGDLY_VOL] = 1.0;
		inputs[UGDLY_VRT] = 0.005;
	}

	void Start()
	{
		inputs[UGDLY_INP] = 0.0f;
		if (inputs[UGDLY_DLY] == 0) 
			inputs[UGDLY_DLY] = 0.01;
		if (inputs[UGDLY_DEC] == 0)
			inputs[UGDLY_DEC] = 1.0;
		gen.InitDL(inputs[UGDLY_DLY], pow(0.001f, inputs[UGDLY_DLY]/inputs[UGDLY_DEC]));
		anyChange |= (1<<UGDLY_VRT); // force setting variable delay on first tick
		out = 0;
		stopped = 0;
		count = (bsInt32) (inputs[UGDLY_DLY] * synthParams.sampleRate);
	}

	void Tick()
	{
		if (anyChange & (1<<UGDLY_VRT))
		{
			float d = inputs[UGDLY_VRT];
			if (d <= 0)
				d = 0.00001;
			else if (d > inputs[UGDLY_DLY])
				d = inputs[UGDLY_DLY];
			gen.SetDelayT(d);
			anyChange = 0;
		}
		out = gen.Sample(inputs[UGDLY_INP] * inputs[UGDLY_VOL]);
		inputs[UGDLY_INP] = 0.0f;
		Send(out, UGP_GEN);
		count -= stopped;
	}
};

#define UGRVB_INP 0
#define UGRVB_VOL 1
#define UGRVB_RVT 2
#define UGRVB_MAX 3

/// @brief Reverb implements a Schroeder reverb.
/// @details A Schroeder reverb contains four comb filters
/// and two allpass filters. This ug only allows the reverb time
/// and amplitude to be varied. The loop times are fixed.
class UGReverb : public ModSynthUGImpl<UGReverb, Reverb2, UGRVB_MAX>
{
private:
	bsInt32 count;
	int stopped;
public:
	typedef ModSynthUGImpl<UGReverb, Reverb2, UGRVB_MAX> base;

	const UGParam *GetParamList()
	{
		return reverbParams;
	}

	const char *GetType()
	{
		return "REVERB";
	}

	UGReverb()
	{
		count = 0;
		stopped = 0;
	}

	void InitDefault()
	{
		inputs[UGRVB_VOL] = 0.1;
		inputs[UGRVB_RVT] = 2.0;
	}

	void SetInput(short index, float val)
	{
		if (index == UGRVB_INP)
			inputs[UGRVB_INP] += val;
		else
			inputs[index] = val;
		anyChange |= 1 << index;
	}

	void Start()
	{
		gen.InitReverb(inputs[UGRVB_VOL], inputs[UGRVB_RVT]);
		anyChange = 0;
		out = 0;
		count = (bsInt32) (inputs[UGRVB_RVT] * synthParams.sampleRate);
		stopped = 0;
	}

	void Stop()
	{
		stopped = 1;
	}

	int IsFinished()
	{
		return count <= 0;
	}

	void Tick()
	{
		out = gen.Sample(inputs[UGRVB_INP]);
		inputs[UGRVB_INP] = 0.0f;
		Send(out, UGP_GEN);
		count -= stopped;
	}
};

#define UGFLNG_INP  0
#define UGFLNG_LVL  1
#define UGFLNG_MIX  2
#define UGFLNG_FB   3
#define UGFLNG_CTR  4
#define UGFLNG_DPTH 5
#define UGFLNG_SWP  6
#define UGFLNG_MAX  7

/// @brief Flanger unit
/// @details The Flanger UG provides an interface to the Flanger class.
class UGFlanger : public ModSynthUGImpl<UGFlanger, Flanger, UGFLNG_MAX>
{
private:
	bsInt32 count;
	bsInt32 stopped;
public:
	const UGParam *GetParamList()
	{
		return flangerParams;
	}

	const char *GetType()
	{
		return "FLANGER";
	}

	UGFlanger()
	{
		count = 0;
		stopped = 0;
	}

	void InitDefault()
	{
		inputs[UGFLNG_LVL] = 1.0;
		inputs[UGFLNG_MIX] = 0.5;
		inputs[UGFLNG_FB] = 0.5;
		inputs[UGFLNG_CTR] = 0.01;
		inputs[UGFLNG_DPTH] = 0.001;
		inputs[UGFLNG_SWP] = 0.1;
	}

	void Start()
	{
		inputs[UGFLNG_INP] = 0.0f;
		gen.InitFlanger(inputs[UGFLNG_LVL], 
			            inputs[UGFLNG_MIX], 
						inputs[UGFLNG_FB], 
						inputs[UGFLNG_CTR], 
						inputs[UGFLNG_DPTH], 
						inputs[UGFLNG_SWP]);
		anyChange = 0;
		out = 0;
		count = (bsInt32) (inputs[UGFLNG_CTR] * synthParams.sampleRate);
		stopped = 0;
	}

	void Stop()
	{
		stopped = 1;
	}

	void SetInput(short index, float val)
	{
		if (index == UGFLNG_INP)
			inputs[UGFLNG_INP] += val;
		else
			inputs[index] = val;
		anyChange |= 1 << index;
	}

	int IsFinished()
	{
		return count <= 0;
	}

	void Tick()
	{
		out = gen.Sample(inputs[UGFLNG_INP]);
		inputs[UGFLNG_INP] = 0.0f;
		Send(out, UGP_GEN);
		count -= stopped;
	}
};
//@}
#endif
