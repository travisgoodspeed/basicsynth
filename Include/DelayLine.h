///////////////////////////////////////////////////////////////
// BasicSynth - DelayLine
//
/// @file DelayLine.h Various forms of delay lines
//
//  - DelayLine, basic delay line with attenuation of the output
//  - DelayLineR, recirculating delay line (resonator)
//  - DelayLineV, variable delay
//  - AllPassDelay, delay line with all pass decay value
//  - DelayLineT, multi-tap delay line
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpDelay
//@{
#ifndef _DELAYLINE_H_
#define _DELAYLINE_H_

/// Basic delay line. Delay lines store the input sample and return
/// a sample delayed by some number of sample times. The decay
/// setting applies an attenuator to the output of the delay line.
/// This simulates the loss of energy during the delay time.
/// @code
/// y[n] = x[n-M] * g, M = delay time, g = decay
/// @endcode
class DelayLine : public GenUnit
{
protected:
	AmpValue *delayBuf;    // buffer to store samples
	AmpValue decayFactor;  // output attenuation
	AmpValue *delayPos;    // current in/out position
	AmpValue *delayEnd;    // last writable position
	FrqValue delayTime;    // length of delayBuf in secs.
	int delayLen;          // length of delayBuf in samps.

	virtual void ReleaseBuffers()
	{
		if (delayBuf)
		{
			delete delayBuf;
			delayBuf = NULL;
			delayPos = NULL;
			delayEnd = NULL;
			delayLen = 0;
		}
	}

public:
	DelayLine()
	{
		delayLen = 0;
		delayBuf = NULL;
		delayPos = NULL;
		delayEnd = NULL;
		decayFactor = 1;
	}

	virtual ~DelayLine()
	{
		ReleaseBuffers();
	}

	/// Initialize the delay line from a source object.
	/// @param dp source object.
	virtual void Copy(DelayLine *dp)
	{
		InitDL(dp->delayTime, dp->decayFactor);
	}

	/// Get the delay line settings.
	/// @param dly delay time
	/// @param dec decay time
	void GetSettings(FrqValue& dly, FrqValue& dec)
	{
		dly = delayTime;
		dec = decayFactor;
	}

	/// Initialize the delay line from an array of values
	/// dlytm, decay
	virtual void Init(int n, float *v)
	{
		if (n > 1)
			InitDL(v[0], v[1]);
	}

	/// Reset the delay line. This merely moves the current position
	/// to the beginning of the buffer.
	/// @param initPhs not used
	virtual void Reset(float initPhs = 0)
	{
		delayPos = delayBuf;
	}

	/// Clear the buffer to zeros
	void Clear()
	{
		delayPos = delayBuf;
		while (delayPos < delayEnd)
			*delayPos++ = 0;
		delayPos = delayBuf;
	}

	/// Initialize the delay line.
	/// @param dlyTm delay time
	/// @param decay amplitude attenuation
	virtual void InitDL(FrqValue dlyTm, AmpValue decay = 1)
	{
		ReleaseBuffers();
		delayTime = dlyTm;
		decayFactor = decay;
		delayLen = (int) (delayTime * synthParams.sampleRate);
		if (delayLen <= 0)
			delayLen = 1;
		delayBuf = new AmpValue[delayLen];
		delayEnd = delayBuf + delayLen;
		Clear();
	}

	/// Read the value at time offset d
	/// @param d time offset in seconds
	AmpValue TapT(PhsAccum d)
	{
		return Tap(d * synthParams.sampleRate);
	}

	/// Read the value at sample offset s
	/// @param s time offset in samples
	AmpValue Tap(PhsAccum s)
	{
		AmpValue *tp = delayPos - (int) s;
		if (tp < delayBuf)
			tp += delayLen;
		return *tp;
	}

	/// Get the oldest value
	inline AmpValue GetOut()
	{
		return *delayPos * decayFactor;
	}

	/// Set the newest value
	/// @param inval value to store in the delay line
	inline void SetIn(AmpValue inval)
	{
		*delayPos = inval;
		if (++delayPos >= delayEnd)
			delayPos = delayBuf;
	}

	/// Store a new value and return the oldest value
	/// @param inval value to store in the delay line
	virtual AmpValue Sample(AmpValue inval)
	{
		AmpValue out = GetOut();
		SetIn(inval);
		return out;
	}
};

/// Re-circulating delay line.
/// Also called feedback delay line, IIR comb filter, resonator.
/// @sa DelayLine
class DelayLineR : public DelayLine
{
private:
	AmpValue final;
	AmpValue peak;
	FrqValue decayTime;

public:
	DelayLineR()
	{
		final = 0.001;
		peak = 1.0;
		decayTime = 1.0;
	}

	/// @copydoc DelayLine::Copy
	virtual void Copy(DelayLineR *dp)
	{
		InitDLR(dp->delayTime, dp->decayTime, dp->final, dp->peak);
	}

	/// Get the delay line settings
	/// @param dlyTm delay time
	/// @param decTm decay time
	/// @param fin amplitude attentaion after decay time
	/// @param pk peak amplitude
	void GetSettings(FrqValue& dlyTm, FrqValue& decTm, AmpValue& fin, AmpValue& pk)
	{
		dlyTm = delayTime;
		decTm = decayTime;
		fin = final;
		pk = peak;
	}

	/// Initialize the delay line.
	/// This method calculates an exponential decay based on delayTime and
	/// final amplitude level after decayTime. Use InitDL directly if the
	/// caller calculates decayFactor by some other means.
	/// @param dlyTm delay time
	/// @param decTm decay time
	/// @param fin amplitude attentaion after decay time
	/// @param pk peak amplitude
	void InitDLR(FrqValue dlyTm, FrqValue decTm, AmpValue fin, AmpValue pk = 1.0)
	{
		final = fin;
		peak = pk;
		decayTime = decTm;
		DelayLine::InitDL(dlyTm, pow(peak * final, dlyTm/decTm));
	}

	/// Process the current sample. The new value is stored in the
	/// delay line and the delayed sample is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = GetOut();
		SetIn(inval + out);
		return out;
	}
};

/// Variable delay tap delay line.
/// This delay line can shift the delay time smoothly from
/// one value to another by interpolating between adjacent
/// delayed values.
/// @sa DelayLine
class DelayLineV : public DelayLine
{
protected:
	PhsAccum dlyTime;
	PhsAccum rdFract;
	int      rdInt;

public:
	DelayLineV()
	{
		dlyTime = 0;
		rdFract = 0;
		rdInt = 0;
	}

	/// Set delay in seconds.
	/// The delay time must be less than the configured delay length
	/// @param d delay time
	void SetDelayT(PhsAccum d)
	{
		SetDelay(d * synthParams.sampleRate);
	}

	/// Set variable delay in samples
	/// @param d delay time
	void SetDelay(PhsAccum d)
	{
		dlyTime = d;
#ifdef _DEBUG
		if (dlyTime < 0)
			dlyTime = 0; // sorry, can't predict the future!
		else if (dlyTime >= delayLen)
			dlyTime = delayLen-1;
#endif
		rdInt = (int) dlyTime;
		rdFract = dlyTime - (PhsAccum) rdInt;
	}

	/// Process the current sample. The current sample is stored
	/// in the delay line buffer and the delayed sample is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue *rdPos = delayPos - rdInt;
		if (rdPos < delayBuf)
			rdPos += delayLen;
		if (rdInt == 0)
			SetIn(inval);
		// *delayPos now contains the current sample if dlyTime < 1 sample
		// or the maximum delay sample otherwise.
		// assert: 
		//         0 <= delayOffs <= delayLen
		//         0 < dlyTime < 1 will combine previous and current samples
		//         dlyTime = 0 will return current sample
		//         delayLen-1 <= dlyTime < delayLen will combine maxdelay sample with maxdelay-1
		//         dlyTime = delayLen will return maxdelay
		//         all other cases return interpolation of rdPos[0] and rdPos[-1]
		AmpValue out = *rdPos;
		if (--rdPos < delayBuf)
			rdPos += delayLen;
		out += (*rdPos - out) * rdFract;
		if (rdInt > 0) // safe to overwrite maxdelay sample
			SetIn(inval);
		return out * decayFactor;
	}
};

/// Delay line with all-pass feedback value.
/// The feedback value creates a combination high-pass + low-pass
/// comb filter that has a flat frequency response.
/// @sa DelayLineR
class AllPassDelay : public DelayLineR
{
public:
	/// Process the current sample. The input value is stored and the delayed
	/// sample is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue vm = *delayPos;
		AmpValue vn = inval - (vm * decayFactor);
		SetIn(vn);
		return vm + (vn * decayFactor);
	}
};

/// All-pass delay line (2).
/// This uses direct-form I, two separate delay lines.
class AllPassDelay2 : public GenUnit
{
private:
	DelayLine dlx;
	DelayLine dly;
	AmpValue apg;

public:
	AllPassDelay2()
	{
		apg = 0;
	}

	// dlyTm, decay
	void Init(int n, float *v)
	{
		if (n > 1)
			InitDL(v[0], v[1]);
	}

	void Reset(float initPhs = 0)
	{
		dlx.Reset(initPhs);
		dly.Reset(initPhs);
	}

	void InitDL(FrqValue delayTime, AmpValue decay)
	{
		apg = decay;
		dlx.InitDL(delayTime, 1);
		dly.InitDL(delayTime, apg);
	}

	/// Initialize the delay line length and feedback value
	void InitDLR(FrqValue delayTime, FrqValue decayTime, float atten, float peak = 1.0)
	{
		InitDL(delayTime, pow(peak * atten, delayTime/decayTime));
	}

	/// Process the current sample. The input value is stored and the delayed
	/// sample is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = (inval * apg) + dlx.GetOut() - dly.GetOut();
		dly.SetIn(out);
		dlx.SetIn(inval);
		return out;
	}
};

/// Multi-tap delay line.
/// @sa DelayLine
class DelayLineT : public DelayLine
{
protected:
	int numTaps;          // number of delay taps
	AmpValue **delayTaps; // position of each tap;
	AmpValue *decayTaps;  // decay value for each tap

public:
	DelayLineT()
	{
		numTaps = 0;
		delayTaps = NULL;
		decayTaps = NULL;
	}

	virtual ~DelayLineT()
	{
		ReleaseBuffers();
	}

	virtual void ReleaseBuffers()
	{
		DelayLine::ReleaseBuffers();
		if (delayTaps)
		{
			delete delayTaps;
			delete decayTaps;
			delayTaps = NULL;
			decayTaps = NULL;
		}
		numTaps = 0;
	}

	// dlytm, taps, [tapn]*
	void Init(int n, float *v)
	{
		if (n > 0)
		{
			int taps = 0;
			if (n > 1)
				taps = (int) v[1];
			InitDLT(v[0], taps);
			int i, t;
			for (i = 2, t = 0; i < n && t < taps; i++, t++)
				SetTap(t, v[i]);
		}
	}

	/// Initialize the delay line length and number of taps
	void InitDLT(FrqValue dlyTm, int taps, AmpValue decay = 1)
	{
		DelayLine::InitDL(dlyTm, decay);

		numTaps = taps;
		if (taps > 0)
		{
			delayTaps = new AmpValue*[taps];
			decayTaps = new AmpValue[taps];
			for (int n = 0; n < taps; n++)
			{
				delayTaps[n] = delayBuf;
				decayTaps[n] = 1.0;
			}
		}
	}

	/// Set the time offset for tap number n
	void SetTap(int n, AmpValue dlyTm, AmpValue decay = 1)
	{
		long position = (int) (dlyTm * synthParams.sampleRate);
		if (position < delayLen)
		{
			delayTaps[n] = delayBuf + (delayLen - position);
			decayTaps[n] = decay;
		}
	}

	/// Read the value for tap n
	AmpValue Tap(int n)
	{
		if (n < numTaps)
			return *(delayTaps[n]) * decayTaps[n];
		return 0;
	}

	/// Store a new value and return the oldest value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = DelayLine::Sample(inval);
		for (int  n = 0; n < numTaps; n++)
		{
			if (++(delayTaps[n]) >= delayEnd)
				delayTaps[n] = delayBuf;
		}
		return out;
	}
};

//@}

#endif
