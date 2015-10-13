///////////////////////////////////////////////////////////////
//
// BasicSynth - BiQuad
//
/// @file BiQuad.h BiQuad filters.
//
/// LowPass, High-pass and Band-pass Butterworth, and resonant filters.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpFilter
//@{
#ifndef _BIQUAD_H_
#define _BIQUAD_H_

#define sqr2 1.414213562

///////////////////////////////////////////////////////////
/// Bi-quad filter base class. This class defines the 
/// coefficient and sample history buffer members but
/// does not implement a filter.
///////////////////////////////////////////////////////////
class BiQuadFilter : public GenUnit
{
public:
	double rad;
	AmpValue gain;
	AmpValue ampIn0;
	AmpValue ampIn1;
	AmpValue ampIn2;
	AmpValue ampOut1;
	AmpValue ampOut2;
	AmpValue dlyIn1;
	AmpValue dlyIn2;
	AmpValue dlyOut1;
	AmpValue dlyOut2;
	FrqValue cutoff;
	FrqValue fq;

	BiQuadFilter()
	{
		rad = PI / synthParams.sampleRate;
		cutoff = 1;
		fq = 1.0;
		gain = 0;
		ampIn0 = 0;
		ampIn1 = 0;
		ampIn2 = 0;
		ampOut1 = 0;
		ampOut2 = 0;
		dlyIn1 = 0;
		dlyIn2 = 0;
		dlyOut1 = 0;
		dlyOut2 = 0;
	}

	/// Initialize with a copy. 
	/// Settings, coefficients are copied from the filt object.
	/// @param filt filter to copy from
	virtual void Copy(BiQuadFilter *filt)
	{
		cutoff = filt->cutoff;
		gain = filt->gain;
		fq = filt->fq;
		ampIn0 = filt->ampIn0;
		ampIn1 = filt->ampIn1;
		ampIn2 = filt->ampIn2;
		ampOut1 = filt->ampOut1;
		ampOut2 = filt->ampOut2;
		dlyIn1 = filt->dlyIn1;
		dlyIn2 = filt->dlyIn2;
		dlyOut1 = filt->dlyOut1;
		dlyOut2 = filt->dlyOut2;
	}

	/// Set the cutoff frequency.
	inline void SetFrequency(FrqValue fc)
	{
		cutoff = fc;
	}

	/// Set the filter Q.
	inline void SetQ(FrqValue q)
	{
		fq = q;
	}

	/// Set the overall gain.
	inline void SetGain(AmpValue g)
	{
		gain = g;
	}

	/// Initialize the filter. The input array holds the cutoff frequency and gain.
	/// @param n number of values (1 or 2)
	/// @param v values: v[0] = cutoff, v[1] = gain
	virtual void  Init(int n, float *v)
	{
		if (n > 1)
			Init((FrqValue) v[0], (AmpValue) v[1]);
		else if (n > 0)
			Init((FrqValue) v[0], 1);
	}

	/// Initialize cutoff frequency and gain.
	/// @param cu cutoff frequency
	/// @param g overall filter gain
	virtual void Init(FrqValue cu, AmpValue g)
	{
		SetFrequency(cu);
		SetGain(g);
		CalcCoef();
		Reset(0.0);
	}

	/// Initialize cutoff frequency, Q, and gain.
	/// @param cu cutoff frequency
	/// @param q filter 'Q'
	/// @param g overall filter gain
	virtual void Init(FrqValue cu, FrqValue q, AmpValue g)
	{
		SetQ(q);
		Init(cu, g);
	}

	/// Calculate the coefficients.
	/// The derived class must implement this method.
	virtual void CalcCoef() { }

	/// Reset the filter. 
	/// The history buffer is cleared to zero. 
	/// @param initPhs (not used)
	virtual void Reset(float initPhs)
	{
		dlyIn1 = 0;
		dlyIn2 = 0;
		dlyOut1 = 0;
		dlyOut2 = 0;
	}

	/// Process the current sample. 
	/// The output sample is calculated from
	/// the coefficients and delayed samples.
	/// @param vin current sample amplitude
	virtual AmpValue Sample(AmpValue vin)
	{
		// Direct Form I
		AmpValue out = (ampIn0 * vin) + (ampIn1 * dlyIn1) + (ampIn2 * dlyIn2)
		             - (ampOut1 * dlyOut1) - (ampOut2 * dlyOut2);
		dlyOut2 = dlyOut1;
		dlyOut1 = out;
		dlyIn2 = dlyIn1;
		dlyIn1 = vin;

		// Direct Form II
		//AmpValue tmp = vin - (ampOut1 * dlyOut1) - (ampOut2 * dlyOut2);
		//AmpValue out = (ampIn0 * tmp) + (ampIn1 * dlyOut1) + (ampIn2 * dlyOut2);
		//dlyOut2 = dlyOut1;
		//dlyOut1 = tmp;
		return out * gain;
	}
};

/// BiQuadFilter optimized for Bandpass.
/// For bandpass filters, we know that ampIn1 == 0
/// and can avoid the useless multiply by 0.
/// Likewise, ampIn2 is -ampIn0, and we can
/// refactor to eliminate the multiply/add of ampIn2.
class BiQuadFilterBP : public BiQuadFilter
{
public:
	AmpValue Sample(AmpValue vin)
	{
		AmpValue out = (ampIn0 * (vin - dlyIn2))
		             - (ampOut1 * dlyOut1) - (ampOut2 * dlyOut2);
		dlyOut2 = dlyOut1;
		dlyOut1 = out;
		dlyIn2 = dlyIn1;
		dlyIn1 = vin;
		return out * gain;
	}
};

///////////////////////////////////////////////////////////
/// Low-pass filter. This class extends BiQuadFilter adding
/// code to calculate the coefficients for a 2nd order
/// Butterworth low-pass filter.
///////////////////////////////////////////////////////////
class FilterLP : public BiQuadFilter
{
public:
	void CalcCoef()
	{
		if (cutoff < 1)
			cutoff = 1;
		if (fq < 0.5)
			fq = 0.5;
		double c = 1 / tan(rad * cutoff);
		double c2 = c * c;
		double csqr2 = sqr2 * c;
		double oned = 1.0 / (c2 + csqr2 + 1.0);

		ampIn0 = oned;
		ampIn1 = oned + oned;
		ampIn2 = oned;
		ampOut1 = (2.0 * (1.0 - c2)) * oned;
		ampOut2 = (c2 - csqr2 + 1.0) * oned;
	}
};

///////////////////////////////////////////////////////////
/// High-pass filter. This class extends BiQuadFilter adding 
/// code to calculate the coefficients for a 2nd order
/// Butterworth high-pass filter.
///////////////////////////////////////////////////////////
class FilterHP : public BiQuadFilter
{
public:
	void CalcCoef()
	{
		double c = tan(rad * cutoff);
		double c2 = c * c;
		double csqr2 = sqr2 * c;
		double oned = 1.0 / (1.0 + c2 + csqr2);

		ampIn0 = (AmpValue) oned;     // 1/d
		ampIn1 = -(ampIn0 + ampIn0);  // -2/d
		ampIn2 = ampIn0;              // 1/d
		ampOut1 = (AmpValue) ((2.0 * (c2 - 1.0)) * oned);
		ampOut2 = (AmpValue) ((1.0 - csqr2 + c2) * oned);
	}
};

///////////////////////////////////////////////////////////
/// Band-pass filter. This class extends BiQuadFilter adding
/// code to calculate the coefficients for a 2nd order
/// Butterworth band-pass filter.
///////////////////////////////////////////////////////////
class FilterBP : public BiQuadFilterBP
{
public:
	void CalcCoef()
	{
		if (fq < 0.5)
			fq = 0.5;
		//double c = 1.0 / tan((PI * cutoff) / (fq * synthParams.sampleRate));
		double c = 1.0 / tan(rad * cutoff / fq);
		//double d = 2.0 * cos(2.0 * PI * cutoff / synthParams.sampleRate);
		double d = 2.0 * cos(synthParams.frqRad * cutoff);
		double oned = 1.0 / (1.0 + c);

		ampIn0 = oned;
		ampIn1 = 0;
		ampIn2 = -oned;
		ampOut1 = -c * d * oned;
		ampOut2 = (c - 1.0) * oned;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// Lowpass filter.
/// The RBJ filters are normalized for unity gain and are easier
/// to use because of that. They also have 'Q' for all forms.
///
/// See: _Cookbook Forumlae for audio EQ biquad filter coefficients_,
/// Robert Bristow-Johnson (http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt)
///////////////////////////////////////////////////////////////////////////////
class FilterLP2 : public BiQuadFilter
{
public:
	void CalcCoef()
	{
		if (fq < 0.5)
			fq = 0.5;
		double w0 = synthParams.frqRad * cutoff;
		double cw0 = cos(w0);
		double alpha = sin(w0) / (2.0*fq);
//		double w0 = synthParams.frqTI * cutoff;
//		double cw0 = wtSet.CosWT(w0);
//		double alpha = wtSet.SinWT(w0) / (2.0*fq);

		double b0 = (1.0 - cw0) / 2.0;
		double b1 = 1.0 - cw0;
		double b2 = b0;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * cw0;
		double a2 = 1.0 - alpha;
		ampIn0 = b0/a0;
		ampIn1 = b1/a0;
		ampIn2 = b2/a0;
		ampOut1 = a1/a0;
		ampOut2 = a2/a0;
	}
};

/// Highpass filter.
class FilterHP2 : public BiQuadFilter
{
public:
	void CalcCoef()
	{
		if (fq < 0.5)
			fq = 0.5;
		double w0 = synthParams.frqRad * cutoff;
		double cw0 = cos(w0);
		double alpha = sin(w0) / (2.0*fq);

		double b0 = (1.0 + cw0) / 2.0;
		double b1 = -(1.0 + cw0);
		double b2 = b0;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * cw0;
		double a2 = 1.0 - alpha;
		ampIn0 = b0/a0;
		ampIn1 = b1/a0;
		ampIn2 = b2/a0;
		ampOut1 = a1/a0;
		ampOut2 = a2/a0;
	}
};

/// Bandpass Filter (0dB peak gain).
class FilterBP2 : public BiQuadFilterBP
{
public:
	void CalcCoef()
	{
		if (fq < 0.5)
			fq = 0.5;
		double w0 = synthParams.frqRad * cutoff;
		double cw0 = cos(w0);
		double alpha = sin(w0) / (2.0*fq);

		double b0 = alpha;
		double b1 = 0.0;
		double b2 = -alpha;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * cw0;
		double a2 = 1.0 - alpha;
		ampIn0 = b0/a0;
		ampIn1 = b1/a0;
		ampIn2 = b2/a0;
		ampOut1 = a1/a0;
		ampOut2 = a2/a0;
	}

};

///////////////////////////////////////////////////////////
/// Constant gain Resonantor. This class extends BiQuadFilter
/// adding code to calculate the coefficients for a constant
/// gain resonant band-pass filter. R must be 0 < r < 1.
/// See J. Smith, "Introduction to Digital Filters", Appendix B
/// and Perry Cook, "Real Sound Synthesis", Chapter 3
///////////////////////////////////////////////////////////
class Reson : public BiQuadFilterBP
{
private:
	FrqValue res;
public:
	Reson()
	{
		res = 0.9;
	}

	/// Set the resonance directly.
	/// By default, res is calculated from 'Q'.
	/// @param r resonance 0 < r < 1
	inline void SetRes(FrqValue r)
	{
		res = r;
	}

	virtual void CalcCoef()
	{
		ampOut1 = -(res + res) * cos(synthParams.frqRad * cutoff);
		ampOut2 = res * res;
		ampIn0 = (1.0 - ampOut2) * 0.5;
		// alternate scaling:
		//ampIn0 = sqrt(1.0 - ampOut2) * 0.5;
		ampIn1 = 0;
		ampIn2 = -ampIn0;
	}

	virtual void Init(FrqValue cu, FrqValue q, AmpValue g)
	{
		if (q > 0.0)
			res = exp(-PI / (q * synthParams.sampleRate));
		else
			res = exp(-PI / synthParams.sampleRate);
		if (res > 0.99999)
			res = 0.99999;
		BiQuadFilterBP::Init(cu, g);
	}
};


//@}
#endif
