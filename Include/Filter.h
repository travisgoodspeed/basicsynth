///////////////////////////////////////////////////////////
// BasicSynth filter classes #1
//
/// @file Filter.h FIR and IIR filter classes.
//
// This file includes classes for:
//  FilterFIR - 1st order (one-zero) FIR filter
//  FilterIIR - 1st order (one-pole) IIR filter
//  FilterIIR2 - one-pole, two-zero filter
//  FilterIIR2p - two-pole filter
//  FilterFIRn - n-order FIR filter using convolution
//  FilterAVGn - n-delay running average filter
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpFilter
//@{
#ifndef _FILTER_H_
#define _FILTER_H_


///////////////////////////////////////////////////////////
/// FIR, one-zero filter. This filter implements the equation:
/// @code
/// y[n] = b * x[n] + a * x[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterFIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (b)
	AmpValue dlyAmp; // delay coefficient (a)
public:
	FilterFIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the two coefficients from the 
	/// value array.
	/// @param n number of values (2)
	/// @param v array of values, v[0] = b, v[1] = a
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	/// Initialize the filter. The two arguments are the coefficients. 
	/// @param in input sample coefficient (a)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample
	AmpValue Sample(AmpValue val)
	{
		AmpValue out = (val * inAmp) + (delay * dlyAmp);
		delay = val;
		return out;
	}
};

///////////////////////////////////////////////////////////
/// IIR, one-pole filter. This filter implements the equation:
/// @code
/// y[n] = b * x[n] - a * y[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterIIR : public GenUnit
{
protected:
	AmpValue delay;  // one sample delay
	AmpValue inAmp;  // input coefficient (b)
	AmpValue dlyAmp; // delay coefficient (a)
public:
	FilterIIR()
	{
		delay = 0;
		inAmp = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the two coefficients from the 
	/// value array.
	/// @param n number of values (2)
	/// @param v array of values, v[0] = b, v[1] = a
	void Init(int n, float *v)
	{
		if (n > 1)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delay = 0;
	}

	/// Initialize the filter. The two arguments are the coefficients 
	/// @param in input sample coefficient (a)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in, AmpValue out)
	{
		inAmp = in;
		dlyAmp = out;
	}

	/// Calculate coefficients. The coefficients are calculate to produce the indicated
	/// cutoff frequency for either a low-pass or high-pass frequency response.
	/// @param fc cutoff frequency
	/// @param hp when true, produce a high-passs
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		if (fc > synthParams.nyquist)
			fc = synthParams.nyquist;

		double x = exp(-twoPI * (fc/synthParams.sampleRate));
		if (hp)
		{
			inAmp = x;
			dlyAmp = AmpValue(1.0 - x);
		}
		else
		{
			inAmp = AmpValue(1.0 - x);
			dlyAmp = AmpValue(-x);
		}
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample value
	AmpValue Sample(AmpValue val)
	{
		return delay = (val * inAmp) - (delay * dlyAmp);
	}
};

///////////////////////////////////////////////////////////
/// One-pole, one-zero filter. This filter implements the equation
/// @code
/// y[n] = a0 * x[n] + a1 * x[n-1] + b1 * y[n-1]
/// @endcode
///////////////////////////////////////////////////////////
class FilterIIR2 : public GenUnit
{
protected:
	AmpValue delayY;  // one sample delay
	AmpValue delayX;
	AmpValue inAmp0; // a0
	AmpValue inAmp1; // a1
	AmpValue dlyAmp; // b1
public:
	FilterIIR2()
	{
		delayX = 0;
		delayY = 0;
		inAmp0 = 0;
		inAmp1 = 0;
		dlyAmp = 0;
	}

	/// Initialize the filter. Set the three coefficients from the 
	/// value array.
	/// @param n number of values (3)
	/// @param v values, v[0] = a0, v[1] = a1, v[2] = b
	void Init(int n, float *v)
	{
		if (n > 2)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delayX = 0;
		delayY = 0;
	}

	/// Initialize the filter. The three arguments are the coefficients 
	/// @param in0 input sample coefficient (a0)
	/// @param in1 input sample coefficient (a1)
	/// @param out delayed sample coefficient (b)
	void InitFilter(AmpValue in0, AmpValue in1, AmpValue out)
	{
		inAmp0 = in0;
		inAmp1 = in1;
		dlyAmp = out;
	}

	/// Calculate coefficients. The coefficients are calculate to produce the indicated
	/// cutoff frequency for either a low-pass or high-pass frequency response.
	/// @param fc cutoff frequency
	/// @param hp when true, produce a high-passs
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		if (fc > synthParams.nyquist)
			fc = synthParams.nyquist;

		double x = exp(-twoPI * (fc/synthParams.sampleRate));
		if (hp)
		{
			inAmp0 = AmpValue((1.0 + x) / 2.0);
			inAmp1 = -inAmp0;
		}
		else
		{
			inAmp0 = 1.0 - x;
			inAmp1 = 0.0;
		}
		dlyAmp = AmpValue(x);
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample value
	AmpValue Sample(AmpValue val)
	{
		delayY = (val * inAmp0) + (inAmp1 * delayX) + (delayY * dlyAmp);
		delayX = val;
		return delayY;

	}
};

///////////////////////////////////////////////////////////
/// Two-pole recursive filter. This filter implements the equation
/// @code
/// y[n] = a0 * x[n] - b1 * y[n-1] - b2 * y[n-2]
/// @endcode
///////////////////////////////////////////////////////////
class FilterIIR2p : public GenUnit
{
protected:
	AmpValue delayY1; // one sample delay
	AmpValue delayY2; // two sample delay
	AmpValue inAmp0;  // a0
	AmpValue dlyAmp1; // b1
	AmpValue dlyAmp2; // b2
public:
	FilterIIR2p()
	{
		delayY1 = 0;
		delayY2 = 0;
		inAmp0 = 0;
		dlyAmp1 = 0;
		dlyAmp2 = 0;
	}

	/// Initialize the filter. Set the three coefficients from the 
	/// value array.
	/// @param n number of values (3)
	/// @param v values, v[0] = a0, v[1] = b0, v[2] = b1
	void Init(int n, float *v)
	{
		if (n > 2)
			InitFilter(AmpValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	/// Reset the filter. This merely clears the delay buffer. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		delayY1 = 0;
		delayY2 = 0;
	}

	/// Initialize the filter. The three arguments are the coefficients 
	/// @param in0 input sample coefficient (a0)
	/// @param out1 delayed sample coefficient (b1)
	/// @param out2 delayed sample coefficient (b2)
	void InitFilter(AmpValue in0, AmpValue out1, AmpValue out2)
	{
		inAmp0 = in0;
		dlyAmp1 = out1;
		dlyAmp2 = out2;
	}

	/// Calculate coefficients. The coefficients are calculate to produce the indicated
	/// cutoff frequency for a band-pass filter with resonance Q
	/// @param fc cutoff frequency
	/// @param q  1/bandwidth
	void CalcCoef(FrqValue fc, FrqValue q)
	{
		if (q == 0 || fc == 0)
		{
			dlyAmp1 = 0;
			dlyAmp2 = 0;
			inAmp0 = 0;
			return;
		}
		if (fc > synthParams.nyquist)
			fc = synthParams.nyquist;
		if (q < 0.5)
			q = 0.5;

		// Hal Chamberlin, Musical Applications of Microprocessors
		double r = exp(-PI * fc / (q * synthParams.sampleRate));
		dlyAmp1 = -2.0 * r * cos(synthParams.frqRad * fc);
		dlyAmp2 = r * r;
		inAmp0 = 1.0 + dlyAmp1 + dlyAmp2;
	}

	/// Process the current sample. The input sample is stored in the delay
	/// buffer and the filtered sample is calculated and returned.
	/// @param val current sample value
	AmpValue Sample(AmpValue val)
	{
		AmpValue out = (inAmp0 * val) - (dlyAmp1 * delayY1) - (dlyAmp2 * delayY2);
		delayY2 = delayY1;
		delayY1 = out;
		return out;
	}
};

///////////////////////////////////////////////////////////
/// FIR impulse response filter. This filter implements
/// convolution of the input with an impulse response:
/// @code
/// y[n] = h[0] * x[0] + h[1] * x[n-1] ... + h[m] * x[n-m]
/// @endcode
/// For a windowed sinc filter kernel of length M,
/// the impulse is calculated:
/// @code
/// h[k] = sin(2*PI*f*(k-M/2))/(k-(M/2) * W(k)
/// W(k) = 0.42 - 0.5 * cos(2*PI*(k/M))
/// f = fc / fs, i.e. fraction of the sample rate
/// @endcode
/// See: _The Scientist and Engineer's Guide to Digital Signal Processing_
/// Steven Smith, Chapter 6, 14, 16.
///////////////////////////////////////////////////////////
class FilterFIRn : public GenUnit
{
protected:
	AmpValue *val; // input sample values
	AmpValue *imp; // impulse response
	int length;
public:
	FilterFIRn()
	{
		val = NULL;
		imp = NULL;
		length = 0;
	}

	~FilterFIRn()
	{
		delete val;
		delete imp;
	}

	/// Allocate impulse response. 
	/// The impulse responce array holds the coefficients for convolution. 
	/// This array is allocated automatically when Init is called.
	/// If coefficients are to be set individulally using SetCoef() this function
	/// must be called first to create the buffer.
	/// @param n number of coefficients
	void AllocImpResp(int n)
	{
		if (val)
		{
			delete val;
			val = NULL;
		}
		if (imp)
		{
			delete imp;
			imp = NULL;
		}
		if ((length = n) > 0)
		{
			val = new AmpValue[length];
			imp = new AmpValue[length];
		}
	}

	/// Initialize the filter. 
	/// The first member of the value array contains the number of samples.
	/// The remaining values contain the amplitudes for each sample.
	/// @param n number of values
	/// @param v values, v[0] = number of coefficients (n), v[1..n] = array of coefficient values
	void Init(int n, float *v)
	{
		AllocImpResp(n);
		for (int i = 0; i < n; i++)
		{
			val[i] = 0;
			if (v)
				imp[i] = AmpValue(v[i]);
			else
				imp[i] = 0;
		}
	}

	/// Set the impulse coefficients. The array must be of the same
	/// length set with AllocImpResp()
	/// @param v impulse response
	void SetCoef(float *v)
	{
		for (int i = 0; i < length; i++)
			imp[i] = AmpValue(v[i]);
	}
	
	/// Calculate coefficients. 
	/// The impulse responce coefficients are calculated for a low-pass
	/// filter using the windowed sinc equation with a Hamming window.
	/// High-pass is also possible by "spectral inversion" of the
	/// coefficients.
	/// @param fc cutoff frequency
	/// @param hp 0 = low-pass, 1 = high-pass
	void CalcCoef(FrqValue fc, int hp = 0)
	{
		if (!(length & 1))
			return;
		if (fc > synthParams.nyquist)
			fc = synthParams.nyquist;
		int n2 = length/2;
		int k;
		// g is the sum of the coefficients and used
		// to normalize gain.
		double g = 0;
		// ti1 is the sin() phase increment for sinc() calculation.
		PhsAccum ti1 = fc * synthParams.frqTI;
		// ti2 is the cos() phase increment for Hamming window.
		PhsAccum ti2 = synthParams.ftableLength / (PhsAccum) (length - 1);
		// Initial phase is at the right lobe of the function.
		PhsAccum tph1 = ti1;
		PhsAccum tph2 = ti2 + (synthParams.ftableLength / 4);
		AmpValue div = 1;
		int ndx1 = n2 + 1;
		int ndx2 = n2 - 1;
		g = twoPI * (fc / synthParams.sampleRate);
		imp[n2] = g;

		AmpValue v;
		for (k = 0; k < n2; k++)
		{
			v = (wtSet.SinWT(tph1) / div) * (0.54 + (0.46 * wtSet.SinWT(tph2)));
			g += v + v;
			imp[ndx1++] = v;
			imp[ndx2--] = v;
			if ((tph1 += ti1) >= synthParams.ftableLength)
				tph1 -= synthParams.ftableLength;
			if ((tph2 += ti2) >= synthParams.ftableLength)
				tph2 -= synthParams.ftableLength;
			div += 1.0;
		}

		// normalize filter gain for unity at DC
		// and optionally convert to high-pass
		for (k = 0; k < length; k++)
		{
			imp[k] /= g;
			if (hp)
				imp[k] = -imp[k];
		}
		if (hp)
			imp[n2] += 1.0;

		/**** Direct calculation (for reference) ******
		double m = (double) length - 1;
		double f = fc / synthParams.sampleRate;
		for (k = 0; k < length; k++)
		{
			double n = (double)k - (m / 2);
			if (n == 0)
				imp[k] = twoPI * f; //2.0 * f;
			else
			{
				imp[k] = sin(twoPI*f*n) / n;
				imp[k] *= 0.54 + (0.46 * cos(twoPI * n / m));
			}
			g += imp[k];
		}
		**********************************************/
	}

	/// Reset the filter. This clears the history values to 0. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		AmpValue *v = val;
		for (int n = length; n > 0; n--)
			*v++ = 0;
	}

	/// Process the current sample. The current sample is pushed into the
	/// history buffer and then convolved with the impulse response.
	/// @param inval current sample value.
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = imp[0] * inval;
		int m = length-1;
		AmpValue *vp = &val[m];
		AmpValue *ip = &imp[m];
		if (m > 0)
		{
			AmpValue tmp;
			do
			{
				tmp = *(vp-1);
				*vp-- = tmp;
				out += *ip-- * tmp;
			} while (vp > val);
		}
		*vp = inval;
		return out;
		/***** indexing (for ref) *****
		for (n = length-1; n > 0; n--)
		{
			val[n] = val[n-1];
			out += imp[n] * val[n];
		}
		val[0] = inval;
		*******************************/
	}
};

///////////////////////////////////////////////////////////
/// Running average filter. This filter sums a series of
/// samples using the equation:
/// @code
/// y[n] = (x[n] + x[n-1] ... + x[n-M]) / M
/// @endcode
///////////////////////////////////////////////////////////
class FilterAvgN : public GenUnit
{
protected:
	AmpValue *prev;
	int length;
public:
	FilterAvgN()
	{
		prev = NULL;
		length = 0;
	}

	~FilterAvgN()
	{
		delete prev;
	}

	/// Initialize the filter. The first value in the array is the number of samples to average.
	/// @param n number of values (1)
	/// @param v values, v[0] = number of samples to average
	void  Init(int n, float *v)
	{
		if (n > 0)
			InitFilter((int)v[0]);
	}

	/// Reset the filter. This clears the previous values. The phase argument is ignored.
	/// @param initPhs not used
	void Reset(float initPhs = 0)
	{
		if (length > 0)
		{
			AmpValue *p = &prev[length];
			while (--p >= prev)
				*p = 0;
		}
	}

	/// Initialize the filter. A history buffer is allocated to the indicated length
	/// and set to zero. 
	/// @param n number of samples to average.
	void InitFilter(int n)
	{
		if (prev)
		{
			delete prev;
			prev = NULL;
		}
		if ((length = n) < 2)
			length = 2;
		prev = new AmpValue[length];
		Reset();
	}

	/// Process the current sample. The sample is added to the history buffer
	/// and then the average is returned.
	/// @param inval current sample value
	AmpValue Sample(AmpValue inval)
	{
		AmpValue out = inval;
		AmpValue tmp;
		AmpValue *p = &prev[length-1];
		do
		{
			tmp = *(p-1);
			*p-- = tmp;
			out += tmp;
		} while (p > prev);
		*p = inval;

		return out / (AmpValue) length;
	}
};

/// State variable filter.
/// The state variable filter produces simultaneous low pass, high pass
/// and band pass outputs from a single second order filter.
/// The output of FilterSV is the sum of the three outputs.
/// By varying the relative gain of each output a wide variety
/// of filters can be realized.
/// The three outputs are also available separately.
/// For dynamic changes, call CalcCoef() directly.
/// Unfortunately, this nice little filter becomes unstable
/// at frequencies above 1/6 of the sample rate. It's good for
/// a lot of subtractive synthesis, but not a good choice for
/// an anti-alias or upper frequency filter.
/// See Hal Chamberlin, Musical Applications of Microprocessors
class FilterSV : public GenUnit
{
protected:
	AmpValue lowPass;
	AmpValue hiPass;
	AmpValue bandPass;

	AmpValue a;
	AmpValue b;

	AmpValue lpOut;
	AmpValue hpOut;
	AmpValue bpOut;

	AmpValue maxFc;

public:
	FilterSV()
	{
		lowPass = 0;
		hiPass = 0;
		bandPass = 0;
		lpOut = 0;
		hpOut = 0;
		bpOut = 0;
		a = 0;
		b = 0;
		maxFc = synthParams.sampleRate / 6.0;
	}

	/// Standard initializer.
	void Init(int n, float *v)
	{
		if (n >= 5)
			InitFilter(v[0], v[1], v[2], v[3], v[4]);
	}

	void CalcCoef(FrqValue fc, AmpValue q)
	{
		if (fc > maxFc)
			fc = maxFc;
		a = 2.0 * wtSet.SinWT(synthParams.maxIncrWT * fc / synthParams.sampleRate);
		if (q > 0)
			b = 1.0 / q;
		else
			b = 0;
	}

	void SetRatios(AmpValue lp, AmpValue hp, AmpValue bp)
	{
		lpOut = lp;
		hpOut = hp;
		bpOut = bp;
	}

	/// Initialize the filter.
	/// @param fc cutoff frequency
	/// @param q filter Q, 0.5 < Q < INF
	/// @param lp volume for low pass output
	/// @param hp volume for high pass output
	/// @param bp volume for band pass output
	void InitFilter(FrqValue fc, AmpValue q, AmpValue lp, AmpValue hp, AmpValue bp)
	{
		CalcCoef(fc, q);
		SetRatios(lp, hp, bp);
		Reset(0);
	}

	/// Clear the filter history buffers.
	/// @param initPhs ignored
	void Reset(float initPhs)
	{
		lowPass = 0;
		hiPass = 0;
		bandPass = 0;
	}

	/// Return the next sample.
	/// The output is the sum of low pass, high pass and band pass
	/// each adjusted by the respective volume level.
	/// @param in current sample.
	/// @returns filtered sample.
	AmpValue Sample(AmpValue in)
	{
		lowPass += a * bandPass;
		hiPass = in - (lowPass + (b * bandPass));
		bandPass += a * hiPass;
		// notch = hiPass + lowPass;

		return (lowPass * lpOut) + (hiPass * hpOut) + (bandPass * bpOut) /*+ notch * brOut) */;
	}

	/// Return the low pass output alone.
	inline AmpValue LowPass()  { return lowPass; }
	/// Return the high pass output alone.
	inline AmpValue HighPass() { return hiPass; }
	/// Return the band pass output alone.
	inline AmpValue BandPass() { return bandPass; }
	/// Return the band reject output alone.
	inline AmpValue BandReject() { return hiPass + lowPass; }
};


/// Resonant lowpass filter based on state variable filter.
/// This is a simplified version, optimized for use as 
/// a resonant lowpass filter.
/// We don't need (and don't store) the high-pass output.
class FilterSVLP : public FilterSV
{
public:
	void Init(int n, float *v)
	{
		InitFilter(v[0], v[1]);
	}

	/// Initialize the filter.
	/// @param fc cutoff frequency
	/// @param q filter Q, 0.5 < Q < INF
	void InitFilter(FrqValue fc, AmpValue q)
	{
		CalcCoef(fc, q);
		Reset(0);
	}

	/// Return the next sample.
	/// The output is the low pass tap.
	/// @param in current sample.
	/// @returns filtered sample.
	AmpValue Sample(AmpValue in)
	{
		lowPass += a * bandPass;
		bandPass += a * (in - (lowPass + (b * bandPass)));
		return lowPass;
	}
};

//@}

#endif
