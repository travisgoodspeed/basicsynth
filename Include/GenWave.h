///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWave
//
/// @file GenWave.h Various waveform generators using direct calculation
//
///  - GenWave - sin wave generator using sin() lib function 
///  - GenWaveSaw - sawtooth wave generator
///  - GenWaveSqr - square wave generator
///  - GenWaveSqr32 - square wave generator, integers
///  - GenWaveTri - triangle wave generator
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpOscil
//@{
#ifndef _GENWAVE_H_
#define _GENWAVE_H_

/// Direct calculation of Sin wave. 
/// Implements the equation for a sinusoid:
/// @code
/// y = A[n] * sin(phs[n])
/// @endcode
/// This is slower than other methods but as accurate as we can get.
/// This class is the base class for all other waveform generators
/// @sa GenUnit
class GenWave : public GenUnit
{
public:
	PhsAccum indexIncr;
	PhsAccum index;
	FrqValue frq;

	GenWave()
	{
		indexIncr = 0;
		index = 0;
		frq = 440;
	}

	/// Initialize the oscillator.
	/// @param n number of values (1)
	/// @param v frequency in v[0]
	virtual void Init(int n, float *v)
	{
		if (n > 0)
			SetFrequency(FrqValue(*v));
		Reset(0);
	}

	/// Return the next sample. The generated value is multiplied by the
	/// supplied input value.
	/// @param in sample peak amplitude
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Set the Frequency. The caller must invoke Reset() to apply the new frequency
	/// @param f frequency in Hz
	inline void SetFrequency(FrqValue f)
	{
		frq = f;
	}

	/// Get the frequency.
	/// @return frequency in Hz
	inline FrqValue GetFrequency()
	{
		return frq;
	}

	/// Reset the oscillator. The phase increment is calculated based on the 
	/// last set frequency value. The phase argument indicates the next phase.
	/// When set to 0, the oscillator is reset to the initial conditions. Values
	/// greater than 0 cause calculation of the appropriate starting sample.
	/// Values less than zero cause the phase to remain unchanged.
	/// @param initPhs phase in radians
	virtual void Reset(float initPhs = 0)
	{
		indexIncr = (PhsAccum)frq * synthParams.frqRad;
		if (indexIncr > PI)
			indexIncr = PI;
		if (initPhs >= 0)
			index = initPhs;
	}

	/// Modulate the oscillator frequency.  This forces recalculation of the
	/// phase increment by adding the argument to the last set frequency value.
	/// @param d delta frequency in Hz
	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqRad;
		if (indexIncr > PI)
			indexIncr = PI;
	}

	/// Modulate the oscillator phase. This changes the oscillator frequency by
	/// directly altering the current oscillator phase. This is faster than
	/// Modulate() if the caller has pre-calculated the phase range.
	/// @param phs delta phase in radians
	virtual void PhaseMod(PhsAccum phs)
	{
		index += phs;
	}

	inline PhsAccum PhaseWrap(PhsAccum index)
	{
		while (index >= twoPI)
			index -= twoPI;
		while (index < 0)
			index += twoPI;
		return index;
	}

	/// Generate the next sample. The sample amplitude is normalized to [-1,+1]
	/// range. The caller must apply any amplitude peak level multiplier.
	/// @return sample value for the current phase
	virtual AmpValue Gen()
	{
		index = PhaseWrap(index);
		AmpValue out = sinv(index);
		index += indexIncr;
		return out;
	}

	/// Generate next sample (high-precision).
	/// Generators that can produce a higher precision
	/// output should override this method.
	/// @return sample value for the current phase
	virtual AmpValue2 Gen2() { return (AmpValue2) Gen(); }
};

/// Like GenWave, but high-precision.
class GenWave2 : public GenWave
{
public:
	virtual AmpValue2 Gen2()
	{
		index = PhaseWrap(index);
		PhsAccum out = sin(index);
		index += indexIncr;
		return out;
	}
};

#define oneDivPI (1.0/PI)

/// Sawtooth wave by direct calculation. This is fast but is not bandwidth
/// limited. It should only be used for LFO effects, not audio.
/// @sa GenWave
class GenWaveSaw : public GenWave
{
public:
	/// @copydoc GenWave::Modulate()
	virtual void Modulate(FrqValue d)
	{
		PhsAccum f = (PhsAccum)(frq + d);
		if (f < 0)
			f = -f;
		indexIncr = (2 * f) / synthParams.sampleRate;
	}

	/// @copydoc GenWave::PhaseMod()
	virtual void PhaseMod(PhsAccum phs)
	{
		// phase modulation works, "mostly"
		//index += 2 * (phs / twoPI);
		index += phs * oneDivPI;
		if (index >= 1)
			index -= 2;
		else if (index < -1)
			index += 2;
	}

	/// @copydoc GenWave::Reset()
	virtual void Reset(float initPhs = 0)
	{
		indexIncr = (PhsAccum)((2 * frq) / synthParams.sampleRate);
		if (initPhs >= 0)
		{
			index = (initPhs * oneDivPI) - 1;
			while (index >= 1)
				index -= 2;
		}
	}

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
		AmpValue v = index;
		if ((index += indexIncr) >= 1)
			index = -1;
		return v;
		/* Alternate calculation, slower
		AmpValue v = (index * oneDivPI) - 1;
		if ((index += indexIncr) >= twoPI)
			index -= twoPI;
		return v;
		*/
	}
};

#define twoDivPI (2.0/PI)

/// Triangle wave by direct calculation. This is very fast, but is not
/// bandwidth limited. 
class GenWaveTri : public GenWave
{
public:
	/// @copydoc GenWave::Modulate()
	virtual void Modulate(FrqValue d)
	{
		indexIncr = (PhsAccum)(frq + d) * synthParams.frqRad;
		if (indexIncr >= PI)
			indexIncr -= twoPI;
		else if (indexIncr < -PI)
			indexIncr += twoPI;
	}

	/// @copydoc GenWave::PhaseMod()
	virtual void PhaseMod(PhsAccum phs)
	{
		index += phs;
		if (index >= PI)
			index -= twoPI;
		else if (index < -PI)
			index += twoPI;
	}

	/// @copydoc GenWave::Gen()
	/// @note phase index varies from [-PI, PI] not [0, 2PI]
	virtual AmpValue Gen()
	{
		//AmpValue triValue = (AmpValue)(1 + (2 * fabs(index - PI) / PI);
		AmpValue triValue = (AmpValue)(index * twoDivPI);
		if (triValue < 0)
			triValue = 1.0 + triValue;
		else
			triValue = 1.0 - triValue;
		if ((index += indexIncr) >= PI)
			index -= twoPI;
		return triValue;
	}
};

/// Square wave by direct calculation. This is fast but not
/// bandwidth limited and should only be used for LFO effects,
/// not audio. This has a settable min/max so that it can toggle
/// from 0/1 as well as -1/+1, or any other pair of values
class GenWaveSqr : public GenWave
{
private:
	PhsAccum midPoint;
	PhsAccum dutyCycle;
	AmpValue ampMax;
	AmpValue ampMin;

public:
	GenWaveSqr()
	{
		midPoint = PI;
		dutyCycle = 50.0;
		ampMax = 1.0;
		ampMin = -1.0;
	}

	/// Set the duty cycle. The duty cycle is specified in a percent
	/// of the period (e.g. 50 = half of period is on).
	/// @param d duty cycle (0-100)
	void inline SetDutyCycle(float d)
	{
		dutyCycle = (PhsAccum)d;
	}

	/// Set the min/max amplitudes. Typically an oscillator is normalized
	/// to the [-1,+1] range. The square wave can be set to toggle between
	/// any two values so that it functions as a gate signal.
	/// @param amin minimum amplitude (off value)
	/// @param amax maximum amplitude (on value)
	void inline SetMinMax(AmpValue amin, AmpValue amax)
	{
		ampMin = amin;
		ampMax = amax;
	}

	// Fo, Duty%
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			SetFrequency(FrqValue(v[0]));
			if (n > 1)
				SetDutyCycle(v[1]);
		}
		Reset();
	}

	/// Initialize the square wave. 
	/// @param f frequency in Hz
	/// @param duty duty cycle (0-100)
	void InitSqr(FrqValue f, float duty)
	{
		SetDutyCycle(duty);
		SetFrequency(f);
		Reset();
	}

	/// @copydoc GenWave::Reset()
	virtual void Reset(float initPhs = 0)
	{
		GenWave::Reset(initPhs);
		midPoint = twoPI * (dutyCycle / 100.0);
	}

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
		AmpValue v = (index > midPoint) ? ampMin : ampMax;
		if ((index += indexIncr) >= twoPI)
			index -= twoPI;
		return v;
	}
};

/// Square waves using integer values.
/// This discards the fractional part of the phase increment
/// and thus avoids phase jitter, but produces slight
/// frequency error that gets worse at higher frequencies.
/// Modulation doesn't work very well either.
/// Makes a very efficient gate signal and not much else.
/// This has a settable min/max so that it can toggle
/// from 0/1 as well as -1/+1, or any other pair of values
class GenWaveSqr32 : public GenWave
{
private:
	bsInt32 sqPeriod;
	bsInt32 sqMidPoint;
	bsInt32 sqPhase;
	float dutyCycle;
	AmpValue ampMax;
	AmpValue ampMin;

	void inline CalcPeriod(FrqValue f)
	{
		sqPeriod = (bsInt32) ((synthParams.sampleRate / f) + 0.5);
		sqMidPoint = (bsInt32) (((float)sqPeriod * dutyCycle) / 100.0);
	}

public:
	GenWaveSqr32()
	{
		sqPeriod = 2;
		sqMidPoint = 1;
		sqPhase = 0;
		dutyCycle = 50.0;
		ampMax = 1.0;
		ampMin = -1.0;
	}

	/// @copydoc GenWaveSqr::SetDutyCycle()
	void inline SetDutyCycle(float d)
	{
		dutyCycle = d;
	}

	/// @copydoc GenWaveSqr::SetMinMax()
	void inline SetMinMax(AmpValue amin, AmpValue amax)
	{
		ampMin = amin;
		ampMax = amax;
	}

	/// @copydoc GenWaveSqr::Init()
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			SetFrequency(FrqValue(v[0]));
			if (n > 1)
				SetDutyCycle(v[1]);
		}
		Reset();
	}

	/// @copydoc GenWaveSqr::InitSqr()
	void InitSqr(FrqValue f, float duty)
	{
		SetDutyCycle(duty);
		SetFrequency(f);
		Reset();
	}

	/// @copydoc GenWave::Reset()
	virtual void Reset(float initPhs = 0)
	{
		CalcPeriod(frq);
		sqPhase = (bsInt32) ((initPhs / twoPI) * (float)sqPeriod);
	}

	/// @copydoc GenWave::Modulate()
	/// Modulate is OK at lower values for d
	virtual void Modulate(FrqValue d)
	{
		FrqValue f = frq + d;
		if (f < 0)
			f = -f;
		CalcPeriod(f);
	}

	/// @copydoc GenWave::PhaseMod()
	/// Doesn't really work because calculated phase
	/// offset will discard fractional portion.
	virtual void PhaseMod(PhsAccum phs)
	{
		sqPhase += (long) ((phs / twoPI) * (float)sqPeriod);
		if (sqPhase >= sqPeriod)
			sqPhase -= sqPeriod;
		else if (sqPhase < 0)
			sqPhase += sqPeriod;
	}

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
		AmpValue v = (sqPhase < sqMidPoint) ? ampMax : ampMin;
		if (++sqPhase >= sqPeriod)
			sqPhase = 0;
		return v;
	}
};

/// Normalized phase integrator. 
/// The phase counts up from 0 to 1-incr
class Phasor : public GenUnit
{
protected:
	PhsAccum phase;
	PhsAccum phsIncr;
	FrqValue frq;

public:
	Phasor()
	{
		phase = 0;
		phsIncr = 0;
		frq = 0;
	}

	/// @copydoc GenWave::SetFrequency
	inline void SetFrequency(FrqValue f)
	{
		frq = f;
	}

	/// Initialize the phasor.
	/// @param n number of value (1)
	/// @param v v[0] contains the frequency
	void Init(int n, float *v)
	{
		if (n > 0)
		{
			SetFrequency(FrqValue(v[0]));
			Reset(0);
		}
	}

	void Reset(float initPhs = 0)
	{
		phsIncr = (PhsAccum)frq / synthParams.sampleRate;
		if (initPhs >= 0)
		{
			phase = (initPhs / twoPI) * phsIncr;
			while (phase >= 1)
				phase -= 1;
		}
	}

	/// Generate the next value.
	/// @param in the scale value, e.g., table length.
	AmpValue Sample(AmpValue in)
	{
		in *= (AmpValue) phase;
		if ((phase += phsIncr) >= 1)
			phase -= 1;
		return in;
	}
};

/// Normalized phase integrator. 
/// The phase counts down from 1 to 0
class PhasorR : public Phasor
{
public:
	void Reset(float initPhs = 0)
	{
		Phasor::Reset(initPhs);
		phase = 1.0 - phase;
	}

	/// Generate the next value.
	/// Set 'in' to the max value, e.g., table length.
	AmpValue Sample(AmpValue in)
	{
		in *= (AmpValue) phase;
		if ((phase -= phsIncr) < 0)
			phase += 1;
		return in;
	}
};
//@}
#endif

