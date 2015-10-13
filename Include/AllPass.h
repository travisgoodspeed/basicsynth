///////////////////////////////////////////////////////////
// BasicSynth - Allpass filter
//
/// @file AllPass.h all-pass filter class
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpFilter
//@{
#ifndef _ALLPASS_H_
#define _ALLPASS_H_

///////////////////////////////////////////////////////////
/// All-pass filter. The allpass filter passes all frequencies
/// without attenuation, but varies the phase of the signal.
///////////////////////////////////////////////////////////
class AllPassFilter : public GenUnit
{
private:
	AmpValue amp;
	AmpValue prevX;
	AmpValue prevY;

public:
	AllPassFilter()
	{
		amp = 0;
		prevX = 0;
		prevY = 0;
	}

	/// Initialize the filter. The first value in the input array is the 
	/// allpass delay.
	/// v[0] = allpass feedback value
	/// @param n number of values (always 1)
	/// @param v array of values
	void Init(int n, float *v)
	{
		if (n > 0)
			InitAP(v[0]);
	}

	/// Reset the filter.
	void Reset(float initPhs = 0)
	{
		prevX = 0;
		prevY = 0;
	}

	/// Initialize the filter. This calculates the coefficient from the equation:
	/// @code
	/// (1 - d) / (1 + d)
	/// @endcode
	/// The value of d specifies a portion of a sample time.
	/// @param d allpass delay
	void InitAP(float d)
	{
		amp = (1.0 - d) / (1.0 + d);
	}

	/// Process the current sample. The output sample is calculated using the allpass equation:
	/// @code
	/// y[n] = (g * x[n]) + x[n-1] - (g * y[n-1])
	/// @endcode
	/// @param val current sample
	AmpValue Sample(AmpValue val)
	{
		AmpValue out;
		out = (amp * val) + prevX - (amp * prevY);
		prevX = val;
		prevY = out;
		return out;
		// Alternate:
		//out = prevY;
		//prevY = val - (prevY * amp);
		//return out + (prevY * amp);
	}
};

//@}
#endif
