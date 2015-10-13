///////////////////////////////////////////////////////////////
//
// BasicSynth Noise generators
//
/// @file GenNoise.h Noise generators
///
///  - GenNoise - white noise generator
///  - GenNoiseH - sampled white noise
///  - GenNoiseI - sampled/interpolated white noise
///  - GenPink1 - pink-ish noise generator using FIR comb filter
///  - GenPink2 - pink-ish noise generator using IIR comb filter
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpNoise
//@{
#ifndef _GENNOISE_H_
#define _GENNOISE_H_

/// White Noise. 
class GenNoise : public GenUnit
{
public:
	/// Initialize. This currently has no effect but is needed
	/// to implement all base class methods.
	virtual void Init(int n, float *v) { }
	/// Reset. This currently has no effect but is needed
	/// to implement all base class methods.
	virtual void Reset(float initPhs = 0) { }

	/// Generate the next sample. The noise signal is
	/// multiplied by the supplied amplitude level.
	/// @param in peak amplitude level
	/// @returns sample value
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Generate the next sample. The noise value is returned
	/// normalized to [-1,+1] amplitude range.
	/// @returns sample value
	virtual AmpValue Gen()
	{
		return ((AmpValue) rand() - (RAND_MAX/2)) / (RAND_MAX/2);
	}

};

/// Held noise. The frequency setting determines when a new random
/// value is generated. In between, the last value is reused. This
/// has the effect of resampling the noise, in effect a low-pass filter.
class GenNoiseH : public GenNoise
{
private:
	AmpValue lastVal;
	FrqValue freq;
	bsInt32 count;
	bsInt32 hcount;
public:
	GenNoiseH()
	{
		freq = synthParams.sampleRate;
		count = 0;
		hcount = 1;
		lastVal = 0;
	}

	/// Initialize the generator. The first value in the array sets the hold frequency.
	/// @param n number of values (1)
	/// @param v array of values (v[0] = frequency)
	virtual void Init(int n, float *v)
	{
		if (n > 0)
			InitH(*v);
	}

	/// Initialize the generator. The argument sets the hold frequency.
	/// @param f hold frequency
	void InitH(FrqValue f)
	{
		if ((freq = f) <= 0)
			freq = 1;
		Reset(0);
	}

	/// Reset the generator. The hold rate is recalculated from the last set frequency.
	/// @param initPhs not used
	virtual void Reset(float initPhs = 0)
	{
		hcount = (bsInt32) (synthParams.sampleRate / freq);
		count = 0;
	}

	/// @copydoc GenNoise::Gen()
	virtual AmpValue Gen()
	{
		if (--count <= 0)
		{
			count = hcount;
			lastVal = GenNoise::Gen();
		}
		return lastVal;
	}

};

/// Interpolated noise. The frequency setting determines when a new random
/// value is generated. In between, the value is interpolated from the last value.
/// This has the effect of resampling the noise, in effect a low-pass filter.
class GenNoiseI : public GenNoise
{
private:
	AmpValue lastVal;
	AmpValue nextVal;
	AmpValue incrVal;
	FrqValue freq;
	bsInt32 count;
	bsInt32 hcount;
public:
	GenNoiseI()
	{
		freq = synthParams.sampleRate;
		hcount = 1;
		count = 0;
		lastVal = 0;
		nextVal = 0;
		incrVal = 0;
	}

	/// @copydoc GenNoiseH::Init()
	virtual void Init(int n, float *v)
	{
		if (n > 0)
			InitH(FrqValue(*v));
	}

	/// @copydoc GenNoiseH::InitH()
	void InitH(FrqValue f)
	{
		if ((freq = f) <= 0)
			freq = 1;
		Reset(0);
	}

	/// Reset the generator. The hold rate is recalculated from the last set frequency.
	/// @param initPhs not used
	virtual void Reset(float initPhs = 0)
	{
		hcount = (bsInt32) (synthParams.sampleRate / freq);
		if (hcount < 1)
			hcount = 1;
		count = 0;
		lastVal = GenNoise::Gen();
		nextVal = GenNoise::Gen();
		incrVal = (nextVal - lastVal) / (AmpValue) hcount;
	}

	/// @copydoc GenNoise::Gen()
	virtual AmpValue Gen()
	{
		if (--count <= 0)
		{
			count = hcount;
			lastVal = nextVal;
			nextVal = GenNoise::Gen();
			incrVal = (nextVal - lastVal) / (AmpValue) count;
		}
		else
			lastVal += incrVal;
		return lastVal;
	}

};

/// "Pinkish" noise generator.
/// First order FIR LP filter applied to white noise.
class GenNoisePink1 : public GenNoise
{
private:
	AmpValue prev;
public:
	GenNoisePink1()
	{
		prev = 0;
	}

	/// @copydoc GenNoise::Gen()
	virtual AmpValue Gen()
	{
		AmpValue val = GenNoise::Gen();
		AmpValue out = (val + prev) / 2;
		prev = val;
		return out;
	}
};

/// "Pinkish" noise generator.
/// First order IIR LP filter applied to white noise.
class GenNoisePink2 : public GenNoise
{
private:
	AmpValue prev;
public:
	GenNoisePink2()
	{
		prev = 0;
	}

	/// @copydoc GenNoise::Gen()
	virtual AmpValue Gen()
	{
		//AmpValue val = GenWaveNoise::Gen();
		//AmpValue out = (val + prev) / 2;
		//prev = out;
		//return out;
		return prev = (GenNoise::Gen() + prev) / 2;
	}
};
//@}
#endif

