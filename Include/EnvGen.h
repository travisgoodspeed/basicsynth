///////////////////////////////////////////////////////////////
//
// BasicSynth - EnvGen
//
/// @file EnvGen.h Simple envelope generators.
/// Fixed duration AR type envelope generators, linear, exponential and log
/// All classes in this file vary output from 0 - peak - 0
///
///  - EnvGen - linear attack and decay, base class for other types
///  - EnvGenExp - exponential attack and decay
///  - EnvGenLog - log attack and decay
//
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpEnv
//@
#ifndef _ENVGEN_H_
#define _ENVGEN_H_

/// Fixed duration envelope generator. EnvGen implements a fixed duration
/// envelope generator with one attack segment and one release segment.
/// EnvGen has a linear attack and decay, and is the base class for other
/// simple fixed duration generators. 
/// @sa GenUnit
class EnvGen : public GenUnit
{
protected:
	bsUint32  index;
	bsUint32  totalSamples;
	bsUint32  attackTime;
	bsUint32  decayTime;
	bsUint32  sustainTime;
	bsUint32  decayStart;
	AmpValue envInc;
	AmpValue peakAmp;
	AmpValue volume;
	FrqValue attack;
	FrqValue decay;
	FrqValue duration;

public:
	EnvGen()
	{
		index = 0;
		volume = 0;
		attackTime = 1;
		decayTime = 1;
		sustainTime = 1;
		decayStart = 1;
		envInc = 1.0;
		peakAmp = 1.0;
		totalSamples = 0;
		attack = 0;
		decay = 0;
		duration = 0;
	}

	/// Initialize the envelope. Values are initialized from the v array. \n
	/// v[0] = peak amplitude \n
	/// v[1] = total duration \n
	/// v[2] = attack time \n
	/// v[3] = decay time
	/// @param n number of values (4)
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n >= 4)
			InitEG(AmpValue(v[0]), FrqValue(v[1]), FrqValue(v[2]), FrqValue(v[3]));
	}

	/// Initialize from specific values. 
	/// The total duration must be greater than or equal to the sum of the attack and decay. 
	/// If this is not the case, the attack and decay times are reduced in length proportionately.
	/// @param peak maximum amplitude and/or sustain level
	/// @param dur sets the total duration. 
	/// @param atk sets the attack time in seconds
	/// @param dec sets the decay (release) time in seconds
	virtual void InitEG(AmpValue peak, FrqValue dur, FrqValue atk, FrqValue dec)
	{
		duration = dur;
		attack = atk;
		decay = dec;

		totalSamples = (bsUint32) ((synthParams.sampleRate * duration) + 0.5);
		if (totalSamples < 3)
			totalSamples = 3;
		attackTime = (bsUint32) (attack * synthParams.sampleRate);
		if (attackTime < 1)
			attackTime = 1;
		decayTime  = (bsUint32) (decay * synthParams.sampleRate);
		if (decayTime < 1)
			decayTime = 1;
		while ((attackTime + decayTime) >= totalSamples)
		{
			if (attackTime > 1)
				attackTime--;
			if (decayTime > 1)
				decayTime--;
		}
		sustainTime = totalSamples - (attackTime + decayTime);
		decayStart = totalSamples - decayTime;
		peakAmp = peak;
		Reset();
	}

	/// Reset the envelope. This is called to start or continue
	/// generation of the envelope.
	/// 0 <= initPhs < 1 will set the volume to the appropriate level
	/// and can be used to re-trigger or cycle the envelope.
	/// initPhs < 0 will not change the current level or index
	/// @param initPhs phase in seconds
	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			index = (bsUint32) (initPhs * duration * synthParams.sampleRate);
			if (index < attackTime)
			{
				envInc = peakAmp / (AmpValue) attackTime;
				volume = (AmpValue) index * envInc;
			}
			else if (index >= decayStart)
			{
				envInc = -peakAmp / (AmpValue) decayTime;
				volume = peakAmp + ((AmpValue) (decayStart - index) * envInc);
			}
			else
				volume = peakAmp;
		}
	}

	/// Generate the next value.
	/// The next envelope value is multiplied by the argument and returned.
	/// @param inval current sample value
	virtual AmpValue Sample(AmpValue inval)
	{
		return Gen() * inval;
	}

	/// Generate the next value.
	/// Gen calculates and returns the next envelope value. 
	/// When the end of the envelope is reached, the last value is returned
	/// until Reset() is called.
	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
			volume += envInc;
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
			envInc = -volume / AmpValue(decayTime);
		index++;
		return volume;
	}

	/// Determine if envelope is finished.
	/// This method returns true if the envelope has reached the last value.
	virtual int IsFinished()
	{
		return index >= totalSamples;
	}
};

/// Convex exponential AR envelope.
/// This implements a fixed duration envelope generator with curved attack
/// and decay generated by taking the square of a normalized linear curve.
class EnvGenSqr : public EnvGen
{
private:
	AmpValue peakAmp;
public:
	EnvGenSqr()
	{
		peakAmp = 1.0;
	}

	virtual void InitEG(AmpValue peak, FrqValue dur, FrqValue atk, FrqValue dec)
	{
		EnvGen::InitEG(1.0, dur, atk, dec);
		peakAmp = peak;
	}

	virtual AmpValue Gen()
	{
		AmpValue out = EnvGen::Gen();
		return peakAmp * out * out;
	}
};

/// Exponential AR envelope. 
/// EnvGenExp implements a fixed duration envelope generator with one
/// attack segment and one release segment. EnvGenExp has an exponential attack and decay,
/// but is otherwise the same as EnvGen.
/// @sa EnvGen GenUnit
class EnvGenExp : public EnvGen
{
private:
	AmpValue expMin;
	AmpValue expFactor;
	AmpValue expCurrent;

public:
	EnvGenExp()
	{
		expMin = 0.2;
		expCurrent = 0;
		expFactor = 0;
	}

	/// Set the curve bias. 
	/// The curve is defined by the equation:
	/// @code
	/// (b/(1+b))^(1/t) or ((1+b)/b))^(1/t)
	/// @endcode
	/// where \e t is the attack or decay time as appropriate. 
	/// The bias must be greater than zero. Smaller values produce steeper curves. 
	/// The default bias value is 0.2.
	/// @param b bias
	virtual void SetBias(AmpValue b)
	{
		expMin = b;
	}

	/// @copydoc EnvGen::Reset()
	virtual void Reset(float initPhs = 0)
	{
		EnvGen::Reset(initPhs);
		if (initPhs >= 0)
		{
			if (index < decayStart)
			{
				expFactor = (AmpValue) pow((1+expMin)/expMin, (AmpValue)1.0 / (AmpValue)attackTime);
				if (index > 0)
					expCurrent = expMin * pow(expFactor, (AmpValue) index);
				else
					expCurrent = expMin;
			}
			else
			{
				expFactor = (AmpValue) pow(expMin/(1+expMin), (AmpValue)1.0 / (AmpValue) decayTime);
				if (index > decayStart)
					expCurrent = (1+expMin) * pow(expFactor, (AmpValue) (index - decayStart));
				else
					expCurrent = 1+expMin;
			}
		}
	}

	/// @copydoc EnvGen::Gen()
	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
		{
			volume = (expCurrent - expMin) * peakAmp;
			expCurrent *= expFactor;
		}
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
		{
			expCurrent = 1 + expMin;
			expFactor = (AmpValue) pow(expMin/expCurrent, (AmpValue)1.0 / (AmpValue) (decayTime-1));
		}
		index++;
		return volume;
	}
};

/// Logarithmic envelope generator. 
/// EnvGenLog implements a fixed duration envelope generator with one attack segment 
/// and one release segment. EnvGenLog has a logarithmic attack and decay, but is otherwise the same as EnvGenExp.
/// @sa EnvGenLog EnvGen GenUnit
class EnvGenLog : public EnvGen
{
private:
	AmpValue expMin;
	AmpValue expFactor;
	AmpValue expCurrent;

public:
	EnvGenLog()
	{
		expMin = 0.2f;
		expCurrent = 0.0f;
		expFactor = 0.0f;
	}

	/// @copydoc EnvGenExp::SetBias()
	virtual void SetBias(AmpValue b)
	{
		expMin = b;
	}

	/// @copydoc EnvGen::Reset()
	virtual void Reset(float initPhs = 0)
	{
		EnvGen::Reset(initPhs);
		if (initPhs >= 0)
		{
			if (index < decayStart)
			{
				expFactor = (AmpValue) pow(expMin/(1+expMin), (AmpValue)1.0 / (AmpValue) attackTime);
				if (index > 0)
					expCurrent = (1+expMin) * pow(expFactor, (AmpValue) index);
				else
					expCurrent = 1 + expMin;
			}
			else
			{
				expFactor = (AmpValue) pow((1+expMin)/expMin, (AmpValue) 1.0 / (AmpValue) decayTime);
				if (index > decayStart)
					expCurrent = expMin * pow(expFactor, (AmpValue) (index - decayStart));
				else
					expCurrent = expMin;
			}
		}
	}

	/// @copydoc EnvGen::Gen()
	virtual AmpValue Gen()
	{
		if (index >= totalSamples)
			return 0;
		if (index < attackTime || index > decayStart)
		{
			volume = (1.0 - (expCurrent - expMin)) * peakAmp;
			expCurrent *= expFactor;
		}
		else if (index == attackTime)
			volume = peakAmp;
		else if (index == decayStart)
		{
			expCurrent = expMin;
			expFactor = (AmpValue) pow((1+expMin)/expMin, 1.0f / (AmpValue) decayTime);
		}
		index++;
		return volume;
	}
};
//@}
#endif
