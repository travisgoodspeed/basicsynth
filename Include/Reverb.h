///////////////////////////////////////////////////////////////
// BasicSynth Reverb classes
//
/// @file Reverb.h Reverberation units
//
// Reverb1 - Single resonator with LP filter
// Reverb2 - Schroeder type reverb unit
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpMix
//@{
#ifndef _REVERB_H_
#define _REVERB_H_

//#include "DelayLine.h"

/// Single resonator reverb.
/// Includes a LP filter in the feedback loop.
class Reverb1 : public DelayLineR
{
private:
	AmpValue atten;
	AmpValue prev;
public:
	Reverb1()
	{
		atten = 1.0;
		prev = 0.0;
	}

	/// Initialize the reverb.
	/// @param n number of values (3)
	/// @param v values, v[0] = atten v[1] = LT, v[2] = RT
	void Init(int n, float *v)
	{
		if (n > 2)
			InitReverb(AmpValue(v[0]), FrqValue(v[1]), FrqValue(v[2]));
	}

	/// Initialize the reverb.
	/// @param a input attenuation
	/// @param lt loop time
	/// @param rt reverb time
	void InitReverb(AmpValue a, FrqValue lt, FrqValue rt)
	{
		atten = a;
		DelayLineR::InitDLR(lt, rt, 0.001);
		prev = 0.0;
	}

	/// Process the current sample.
	/// The input value is passed to the reverb.
	/// The reverberated sample is returned.
	/// @param vin current sample
	AmpValue Sample(AmpValue vin)
	{
		AmpValue out = *delayPos * decayFactor;
		AmpValue vlp = (out + prev) / 2;
		prev = vlp;
		*delayPos = (vin * atten) + vlp;
		if (++delayPos >= delayEnd)
			delayPos = delayBuf;
		return out;
	}
};

/// Schroeder reverb.
/// This reverb uses four parallel comb filters + two series allpass filters
class Reverb2 : public GenUnit
{
private:
	DelayLineR dlr[4];
	AllPassDelay ap[2];
	AmpValue atten;
public:
	Reverb2()
	{
		atten = 1.0;
	}

	/// Initialize the reverb.
	/// @param n number of values (2)
	/// @param v values, v[0] = atten v[1] = RT
	void Init(int n, float *v)
	{
		if (n > 1)
			InitReverb(AmpValue(v[0]), FrqValue(v[1]));
	}

	/// Reset the reverb
	/// Clears the comb filters to zero.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		dlr[0].Reset(initPhs);
		dlr[1].Reset(initPhs);
		dlr[2].Reset(initPhs);
		dlr[3].Reset(initPhs);
		ap[0].Reset(initPhs);
		ap[1].Reset(initPhs);
	}

	/// Initialize the reverb.
	/// InitReverb sets the typical LT values for a Scroeder unit.
	/// The RT value is 1-2 seconds, typical.
	/// The attenuation value controls how much of the signal
	/// will be sent through the reverb. Usually it is set to
	/// 1.0, but can be lowered if the reverb distorts. If no
	/// output attenuation is applied, set the internal attenuation
	/// to 0.1 to 0.25 to balance the summation of the four comb filters.
	/// @param a attenuation value for input
	/// @param rt reverb time
	void InitReverb(AmpValue a, FrqValue rt)
	{
		atten = a;
		dlr[0].InitDLR(0.0437, rt, 0.001);
		dlr[1].InitDLR(0.0411, rt, 0.001);
		dlr[2].InitDLR(0.0371, rt, 0.001);
		dlr[3].InitDLR(0.0297, rt, 0.001);
		ap[0].InitDLR(0.09683, 0.0050, 0.001);
		ap[1].InitDLR(0.03292, 0.0017, 0.001);
	}

	/// Initialize Reverb.
	/// Set the LT and RT values individually
	/// The first four index values (0-3) are the comb
	/// filters while index 4-5 are the allpass units.
	/// @param n comb filter index
	/// @param lt loop time
	/// @param rt reverb time
	void InitDelay(int n, FrqValue lt, FrqValue rt)
	{
		if (n >= 0 && n < 4)
			dlr[n].InitDLR(lt, rt, 0.001);
		else if (n >= 4 && n < 6)
			ap[n-4].InitDLR(lt, rt, 0.001);
	}

	/// Clear all delay lines to zero
	void Clear()
	{
		dlr[0].Clear();
		dlr[1].Clear();
		dlr[2].Clear();
		dlr[3].Clear();
		ap[0].Clear();
		ap[1].Clear();
	}

	/// Process the current sample.
	/// The input value is passed to the reverb.
	/// The reverberated sample is returned.
	/// @param vin current sample
	AmpValue Sample(AmpValue vin)
	{
		vin *= atten;
		AmpValue out = dlr[0].Sample(vin) + dlr[1].Sample(vin) + dlr[2].Sample(vin) + dlr[3].Sample(vin);
		return ap[1].Sample(ap[0].Sample(out));
	}
};

//@}
#endif
