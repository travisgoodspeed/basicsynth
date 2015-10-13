///////////////////////////////////////////////////////////////
//
// BasicSynth - GenWaveDSF
//
/// @file GenWaveDSF.h Various complex spectrum waveform generators based on
/// closed form of discrete summation forumlae.
//
// GenWaveDSB - sum of sines, bandwidth limited
// GenWaveDS - sum of sines, "infinite" N
// GenWaveBuzz - pulse generator
// GenWaveBuzzA - pulse generator (aggregation)
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpOscil
//@{
#ifndef _GENWAVEDSF_H_
#define _GENWAVEDSF_H_

///////////////////////////////////////////////////////////
/// @brief Discrete summation wave generator (bandwidth limited)
/// @details Uses a closed form of the series:
/// @code
/// Sn = A * SUM(k=0...N, a^k * sin(Fo-k*Fm))
/// 
///        (1 - a^2)       sin(Fo) - a*sin(Fo-Fm) - a^N+1*(sin(Fo+(N+1)Fm) - a*sin(Fo+N*Fm))
/// Sn = --------------  * -----------------------------------------------------------------
///      (1 - a^(2n+2))                (1 + a^2) - 2a * cos(Fm)
///
/// Fo = 2*PI*f*t = radians at sample 't'
/// @endcode
/// Fo is the fundamental, Fm the first partial above Fo, 'N' the number of harmonics
/// and 'a' the amplitude scaling ratio. When a<>1, the amplitudes of partials follow
/// an exponential curve. When a=1 a pulse wave is produced. The value for 'a' can be
/// dynamic, producing a time-varying spectrum. The first factor provides peak amplitude
/// normalization.
///
/// Fm is specified by a ratio (R=Fm/Fo) so that the spectrum remains consistent as the 
/// fundamental frequency changes. When Fm=Fo, R=1 and produces a natural harmonic series.
/// R=2 produces odd harmonics only. Making R an irrational number (e.g., sqrt(2))
/// produces an inharmonic spectrum.
///
/// The value for 'N' is clamped to the maximum partial less than the Nyquist limit (SR/2)
/// and thus the output is always bandwidth limited, i.e., does not produce alias frequencies. 
/// N can be set to a value for the lowest frequency desired and the generator will adjust 
/// as needed when the frequency changes.
///
/// This equation (and other closed forms of the Fourier series) is highly sensitive to 
/// round-off erros. A higher precision amplitude value (AmpValue2) helps keep the peak
/// amplitude consistent as the amplitude ratio (a) and number of harmonics (N) varies.
/// When a wavetable is used, we should use interpolation of some kind. In addition, 
/// round-off of the phase increment and wavetable values may cause the amplitude to never
/// reach zero. We test for amplitude of the denomerator close to zero instead
/// of attempting to match zero exactly. 
///
/// GenWaveDSB is implemented by aggregating 5 oscillators. The base class is
/// used as the cos() oscillator. The 4 members in osc[n] are the sin() oscillators.
/// Best quality is with the high-precision interpolating wavetable, GenWaveI::Gen2().
/// GenWaveWT also gives good results and provides slightly higher performance.
/// Fastest is to use the 32-bit or 64-bit integer accumulator GenWave32 or GenWave64. 
/// 
/// GenWaveDSB is similar to the "GBUZZ" unit generator in CSound. The
/// main difference is this allows non-integer values for 'k'.
///
/// See: Moorer, "The Synthesis of Complex Audio Spectra by Means of Discrete Summation Formulae."
/// Journal of the Audio Engineering Society, Volume 24, Number 9, November 1976, pp717-727
///
/// @sa GenWave GenWaveWT GenWaveDS GenWaveBuzz
///////////////////////////////////////////////////////////
class GenWaveDSB : public GenWaveI
{
private:
	GenWaveI osc[4];
	FrqValue frqRatio;
	FrqValue partN;
	FrqValue beta;
	AmpValue2 ampRatio;
	AmpValue2 ampTwo;
	AmpValue2 ampSqrP1;
	AmpValue2 ampPowN;
	AmpValue2 ampScale;
	bsInt32  harmNum;

public:
	GenWaveDSB()
	{
		frqRatio = 1.0;
		partN = 1.0;
		harmNum = 1;
		for (int n = 0; n < 4; n++)
			osc[n].SetWavetable(WT_SIN);
		SetWavetable(WT_SIN);
	}

	/// Set the range and spacing of harmonics.
	/// Rational values for fr produce harmonic overtones.
	/// Irrational values produce inharmonic overtones.
	/// CalcRatio() should be called after this is set.
	/// @param fr frequency ratio (Fm/Fc)
	/// @param nh the highest partial (n > 0)
	void SetHarmonics(FrqValue fr, bsInt32 nh)
	{
		if (fr == 0.0)
			fr = 1.0;
		frqRatio = fr;
		harmNum = nh;
	}

	/// Set the ratio of amplitudes for partials.
	/// @param a amplitude ratio, partial[n] = a^n
	void SetRatio(AmpValue a)
	{
		ampRatio = a;
	}

	/// Modulate the ratio.
	/// This should be used for dynamic spectrum 
	/// since it is more efficient than
	/// SetRatio() followed by Reset(-1).
	/// @param a amplitude ratio
	void ModRatio(AmpValue a)
	{
		ampRatio = a;
		CalcRatio();
	}

	/// Calculate the amplitude ratio values.
	/// For internal use, but can be called directly after
	/// a direct modification of frequency, ampRatio or partN.
	void CalcRatio()
	{
		beta = frq * frqRatio;
		partN = FrqValue(harmNum);
		FrqValue maxN = floor((synthParams.sampleRate / (2.0*beta)) - (1.0/frqRatio) - 1.0);
		if (partN < 1 || partN > maxN)
			partN = maxN;
		ampTwo = ampRatio + ampRatio;
		ampSqrP1 = ampRatio * ampRatio + 1.0;
		if (ampRatio == 1.0)
		{
			ampPowN = 1.0;
			ampScale = 1.0 / AmpValue2(partN+1);
		}
		else
		{
			ampPowN = pow(ampRatio, (double)(partN+1));
			ampScale = (1.0 - (ampRatio * ampRatio)) / (1.0 - pow(ampRatio, (double)(2*partN)+2));
		}
	}

	/// Initialize the generator from an array of values.
	/// @param n number of values (4)
	/// @param v array of values, v[0] = Fo, v[1] = Fm/Fo, v[2] = N, v[3] = a
	void Init(int n, float *v)
	{
		if (n >= 4)
			InitDSB(FrqValue(v[0]), FrqValue(v[1]), (bsInt32)v[2], AmpValue(v[3]));
	}

	/// Initialize the generator from explicit values.
	/// @param f frequency (Fo)
	/// @param fr frequency ratio (Fm/Fo)
	/// @param nh number of harmonics (N)
	/// @param a amplitude ratio
	void InitDSB(FrqValue f, FrqValue fr, bsInt32 nh, AmpValue a)
	{
		SetFrequency(f);
		SetHarmonics(fr, nh);
		SetRatio(a);
		Reset(0);
	}

	/// Reset the generator after a parameter change.
	/// @param initPhs (initial oscillator phase)
	void Reset(float initPhs)
	{
		CalcRatio();
		osc[0].SetFrequency(frq);
		osc[1].SetFrequency(frq - beta);
		osc[2].SetFrequency(frq + ((partN + 1)*beta));
		osc[3].SetFrequency(frq + (partN * beta));

		indexIncr = synthParams.frqTI * beta;
		if (initPhs >= 0)
		{
			index = (initPhs / twoPI) * synthParams.ftableLength;
			index += synthParams.ftableLength / 4; // phase shift to make a cosine
		}
		osc[0].Reset(initPhs);
		osc[1].Reset(initPhs);
		osc[2].Reset(initPhs);
		osc[3].Reset(initPhs);
	}

	/// Modulate the frequency.
	/// @param d amount to add or subtract from the base frequency
	void Modulate(FrqValue d)
	{
		PhsAccum fo = frq + d;
		PhsAccum fm = beta + d;
		indexIncr = synthParams.frqTI * fm;
		osc[0].indexIncr = synthParams.frqTI * fo;
		osc[1].indexIncr = synthParams.frqTI * (fo - fm);
		osc[2].indexIncr = synthParams.frqTI * (fo + ((partN + 1)*fm));
		osc[3].indexIncr = synthParams.frqTI * (fo + (partN * fm));
	}

	/// Modulate the phase.
	/// @param phs amount to add or subtract from the phase
	void PhaseModWT(FrqValue phs)
	{
		index += phs;
		osc[0].index += phs;
		osc[1].index += phs * (osc[1].indexIncr / indexIncr);
		osc[2].index += phs * (osc[2].indexIncr / indexIncr);
		osc[3].index += phs * (osc[3].indexIncr / indexIncr);
	}

	/// Generate the next sample value (default precision).
	/// @return amplitude of the current sample.
	virtual AmpValue Gen()
	{
		return (AmpValue) Gen2();
	}

	/// Generate the next sample value (high precision).
	/// @return amplitude of the current sample.
	virtual PhsAccum Gen2()
	{
		AmpValue2 den = ampSqrP1 - (ampTwo * GenWaveI::Gen2());
		if (den != 0.0)
		{
			AmpValue2 num = osc[0].Gen2() - (ampRatio * osc[1].Gen2())
			              - (ampPowN * (osc[2].Gen2() - (ampRatio * osc[3].Gen2())));
			return ampScale * num / den;
		}
		// since we skipped calling Gen(), increment the "phase" directly.
		osc[0].index += osc[0].indexIncr;
		osc[1].index += osc[1].indexIncr;
		osc[2].index += osc[2].indexIncr;
		osc[3].index += osc[3].indexIncr;
		return 1.0;
	}
};


///////////////////////////////////////////////////////////
/// @brief Discrete summation wave generator.
/// @details Uses a closed form of the series:
/// @code
/// Sn = A * SUM(k=0...INF, a^k * sin(Fo-k*Fm))
/// 
///                   sin(Fo) - a * sin(Fo-Fm)
/// Sn = (1 - a^2) * --------------------------
///                    1 + a^2 - 2a * cos(Fm)
/// @endcode
/// In contrast to GenWaveDSB, the series is "infinite" for N. This simplifies
/// the computation, but 'a' MUST be 0 < a < 1 and chosen carefully to avoid audible alias
/// frequencies. Using a higher sample rate, or applying a low-pass filter, is often
/// enough to avoid problems. The multiplier (1-a^2) normalizes peak amplitude.
///
/// In this implementation, Fo = Fm, and the second term of the 
/// numerator becomes a * sin(0) = 0.  This constraint allows implementation 
/// with a single phase increment where the cos() phase is calculated by an 
/// offset of the sin() phase. Table lookup and phase increment is performed in-line,
/// resulting in a fast, dynamic spectrum generator.
///
/// See: Moorer, "The Synthesis of Complex Audio Spectra by Means of Discrete Summation Formulae."
/// Journal of the Audio Engineering Society, Volume 24, Number 9, November 1976, pp717-727
/// Also, Computer Music, Dodge&Jerse, chapter 5.3b.
///
/// @sa GenWave GenWaveWT GenWaveDSB
///////////////////////////////////////////////////////////
class GenWaveDS : public GenWaveWT
{
private:
	PhsAccum index2;  ///< table lookup for cos()
	AmpValue2 ampRatio;  ///< value for 'a'
	AmpValue2 amp1pSqr;  ///< 1 + a^2
	AmpValue2 amp1mSqr;  ///< 1 - a^2
	AmpValue2 ampTwo;    ///< a*2

public:
	GenWaveDS()
	{
		ampRatio = 0.0;
		amp1pSqr = 1.0;
		amp1mSqr = 1.0;
		ampTwo = 0.0;
		index2 = 0.0;
	}

	/// Initialize the oscillator. 
	/// The values are passed in the v array
	/// with v[0] = frequency and v[1] = ratio of Fm/Fc and v[2] = amplitude ratio.
	/// @param n number of values (2)
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n > 0)
			SetFrequency(v[0]);
		if (n > 1)
			SetRatio(v[2]);
		Reset();
	}

	/// Set the amplitude ratio for harmonics.
	/// Higher values produce narrower pulse.
	/// We pre-calculate 1 +/- a^2 and 2a.
	/// @param a amplitude value for harmonics (0 < a < 1)
	void SetRatio(AmpValue a)
	{
		if (a >= 0.9999)
			a = 0.9999;
		ampRatio = (AmpValue2) a;
		AmpValue2 as = ampRatio * ampRatio;
		amp1mSqr = 1.0 - as;
		amp1pSqr = 1.0 + as;
		ampTwo = ampRatio + ampRatio;
	}

	/// Initialize the generator
	/// @param f frequency
	/// @param a amplitude scaling of harmonics (a < 1)
	void InitDS(FrqValue f, AmpValue a)
	{
		SetFrequency(f);
		SetRatio(a);
		Reset();
	}

	/// @copydoc GenWave::Reset
	virtual void Reset(float initPhs = 0.0)
	{
		GenWaveWT::Reset(initPhs);
		if (initPhs >= 0.0)
			index2 = index + (synthParams.ftableLength / 4.0);
	}

	/// @copydoc GenWaveWT::PhaseModWT()
	virtual void PhaseModWT(PhsAccum phs)
	{
		index  += phs;
		index2 += phs;
	}

	/// @copydoc GenWave::Gen()
	virtual AmpValue Gen()
	{
		return (AmpValue) Gen2();
	}

	/// @copydoc GenWave::Gen2()
	virtual AmpValue2 Gen2()
	{
		index  = PhaseWrapWT(index);
		index2 = PhaseWrapWT(index2);
		AmpValue2 out = amp1mSqr * wtSet.SinWT(index) / (amp1pSqr - (ampTwo * wtSet.SinWT(index2)));
		index  += indexIncr;
		index2 += indexIncr;
		return out;
	}
};


///////////////////////////////////////////////////////////
/// @brief Pulse wave generator.
/// @details Buzz uses a closed form of the Fourier series to produce a pulse wave.
/// There is a sin() form and a cos() form.
/// @code
///        A       sin((2N+1)*x)
/// Sn = ---- * ((-----------------) - 1)
///       2N          sin(x)
/// ============================================
///        A      (2N+1)*cos((2N+1)*x)
/// Sn = ---- * ((------------------------) - 1)
///       2N            cos(x)
///
/// x = PI*f*t = radians at time 't'
/// @endcode
/// This produces a bandwidth limited pulse wave with peak amplitude A.
/// In this implementation, A is always 1.
/// The number of harmonics (N) is a settable parameter. 
/// A higher number of harmonics produces a narrower pulse.
///
/// As the sin() form has one less multiply, it is the prefered form.
/// However, when the denominator sin(x) is 0, we can't use the sin() form
/// as it would result in divide by zero. But we can use the cos() form
/// in that case. We know that when sin(x) = 0, then cos(x) = 1. 
/// In addition, sin(x) = 0 only when the phase is some multiple of PI.
/// Since phase of the numerator is an integer multiple of the phase
/// of the denominator, we know the numerator phase must be 0 or some 
/// multiple of PI also. Furthermore, |cos(x)| is the same for all multiples
/// of PI. Thus:
/// @code
/// x = {0, PI}
/// S = A/2N * (((2N+1)*cos(x)/cos(x)) - 1)
/// S = A/2N * (((2N+1)*1) - 1)
/// S = A/2N * 2N
/// S = A
/// @endcode
/// When the denominator = 0, we simply substitue A=1.0
///
/// See: Computer Music, Dodge&Jerse, chapter 5.3a
/// and http://www.cs.sfu.ca/~tamaras/.
///
/// @sa GenWave GenWaveDSB
///////////////////////////////////////////////////////////
class GenWaveBuzz : public GenWaveWT
{
private:
	bsInt32 numHarm;
	PhsAccum index2;
	PhsAccum indexIncr2;
	AmpValue2 ampScale;
	AmpValue2 num2p1;
	PhsAccum frqTI;

	/// Calcuate the phase increment.
	/// @param f frequency
	void CalcIncr(FrqValue f)
	{
		indexIncr  = frqTI * PhsAccum(f);
		indexIncr2 = indexIncr * num2p1;
	}

public:
	GenWaveBuzz()
	{
		numHarm = 1;
		ampScale = 0.0;
		index2 = 0.0;
		indexIncr2 = 0.0;
		frqTI = synthParams.ftableLength / (synthParams.sampleRate * 2.0);
	}

	/// @copydoc GenWave::Init
	void Init(int n, float *v)
	{
		if (n > 1)
			InitBuzz(v[0], (int)v[1]);
	}

	/// Set the number of harmonics.
	/// The higher n, the narowwer the pulse.
	/// @param n number of harmonics (0 < n < (SR/2Fo))
	void SetHarmonics(bsInt32 n)
	{
		numHarm = n;
	}

	/// Initialize the Buzz generator
	/// @param f frequency
	/// @param n number of harmonics (0 = maximum)
	void InitBuzz(FrqValue f, bsInt32 n)
	{
		SetFrequency(f);
		SetHarmonics(n);
		Reset();
	}

	/// @copydoc GenWave::Reset
	virtual void Reset(float initPhs = 0.0)
	{
		bsInt32 maxN = (bsInt32) (synthParams.sampleRate / (2.0 * frq)) - 1;
		bsInt32 N = (numHarm > maxN) ? maxN : numHarm;
		if (N <= 0)
		{
			ampScale = 1.0;
			num2p1 = 1.0;
		}
		else
		{
			ampScale = 0.5 / PhsAccum(N); // 1.0 / 2N
			num2p1 = PhsAccum(N + N + 1); // 2N + 1
		}
		CalcIncr(frq);
		if (initPhs >= 0.0)
			index  = synthParams.radTI * initPhs;
		index2 = index * num2p1;
	}

	/// @copydoc GenWave::Modulate
	virtual void Modulate(FrqValue d)
	{
		CalcIncr(frq+d);
	}

	virtual void PhaseModWT(PhsAccum phs)
	{
		phs = phs * 0.5;
		indexIncr += phs;
		indexIncr2 += phs * num2p1;
	}

	/// @copydoc GenWave::Gen
	AmpValue Gen()
	{
		return (AmpValue) Gen2();
	}

	PhsAccum Gen2()
	{
		index = PhaseWrapWT(index);
		index2 = PhaseWrapWT(index2);
		AmpValue2 out = wtSet.SinWT(index);
		if (out != 0.0)
			out = ampScale * ((wtSet.SinWT(index2) / out) - 1.0);
		else
			out = 1.0;
		index  += indexIncr;
		index2 += indexIncr2;
		return out;
	}
};

///////////////////////////////////////////////////////////
/// @brief Pulse wave generator #2
/// @details Buzz uses a closed form of a series to produce a pulse wave.
/// @code
///        1     sin(N*x/2) * sin((N+1)*x/2)
/// Sn = ----- * -----------------------------
///       N+1             sin(x/2)
/// @endcode
/// Unlike the  more common BUZZ, it produces a waveform with positive and negative peaks. 
/// See: http://mathworld.wolfram.com/Sine.html for the derivation.
///
///
/// @sa GenWave GenWaveDSB
///////////////////////////////////////////////////////////
class GenWaveBuzz2 : public GenWaveWT
{
private:
	bsInt32 numHarm;
	PhsAccum index1;
	PhsAccum indexIncr1;
	PhsAccum index2;
	PhsAccum indexIncr2;
	AmpValue2 ampScale;
	AmpValue2 num;
	AmpValue2 nump1;
	PhsAccum frqTI;

	/// Calcuate the phase increment.
	/// @param f frequency
	void CalcIncr(FrqValue f)
	{
		indexIncr  = frqTI * PhsAccum(f);
		indexIncr1 = indexIncr * numHarm;
		indexIncr2 = indexIncr * nump1;
	}

public:
	GenWaveBuzz2()
	{
		numHarm = 1;
		ampScale = 0.0;
		index1 = 0.0;
		indexIncr1 = 0.0;
		index2 = 0.0;
		indexIncr2 = 0.0;
		frqTI = synthParams.ftableLength / (synthParams.sampleRate * 2.0);
	}

	/// @copydoc GenWave::Init
	void Init(int n, float *v)
	{
		if (n > 1)
			InitBuzz(v[0], (int)v[1]);
	}

	/// Set the number of harmonics.
	/// The higher n, the narowwer the pulse.
	/// @param n number of harmonics (0 < n < (SR/2Fo))
	void SetHarmonics(bsInt32 n)
	{
		numHarm = n;
	}

	/// Initialize the Buzz generator
	/// @param f frequency
	/// @param n number of harmonics (0 = maximum)
	void InitBuzz(FrqValue f, bsInt32 n)
	{
		SetFrequency(f);
		SetHarmonics(n);
		Reset();
	}

	/// @copydoc GenWave::Reset
	virtual void Reset(float initPhs = 0.0)
	{
		bsInt32 maxN = (bsInt32) (synthParams.sampleRate / (2.0 * frq)) - 1;
		bsInt32 N = (numHarm > maxN) ? maxN : numHarm;
		if (N <= 1)
		{
			ampScale = 1.0;
			num = 0.0;
		}
		else
		{
			ampScale = 1.0 / (AmpValue2) (N+1);
			num = N;
		}
		nump1 = num + 1.0;
		CalcIncr(frq);
		if (initPhs >= 0.0)
			index  = synthParams.radTI * initPhs;
		index1 = index * num;
		index2 = index * nump1;
	}

	/// @copydoc GenWave::Modulate
	virtual void Modulate(FrqValue d)
	{
		CalcIncr(frq+d);
	}

	virtual void PhaseModWT(PhsAccum phs)
	{
		phs = phs * 0.5;
		indexIncr += phs;
		indexIncr1 += phs * num;
		indexIncr2 += phs * nump1;
	}

	/// @copydoc GenWave::Gen
	AmpValue Gen()
	{
		return (AmpValue) Gen2();
	}

	PhsAccum Gen2()
	{
		index = PhaseWrapWT(index);
		index1 = PhaseWrapWT(index1);
		index2 = PhaseWrapWT(index2);
		AmpValue2 out = wtSet.SinWT(index);
		if (out != 0.0)
			out = ampScale * wtSet.SinWT(index1) * wtSet.SinWT(index2) / out;
		else
			out = 1.0;
		index  += indexIncr;
		index1 += indexIncr1;
		index2 += indexIncr2;
		return out;
	}
};

///////////////////////////////////////////////////////////
/// @brief Pulse wave generator (using aggregation).
/// @details Variation of BUZZ implemented by aggregation of two oscillators.
/// This has less inline code, and thus slightly slower, 
/// but is flexible, allowing either a faster or more precise oscillator.
/// For example, use GenWave32 for fastest operation,
/// GenWaveI for interpolation, or your own custom oscillator
/// derived from GenWaveWT.
/// By default, AmpValue is single precision float and GenWaveWT
/// rounds the phase accumulator, producing slight distortion
/// and spikes in the waveform.
/// @sa GenWaveBuzz
///////////////////////////////////////////////////////////
class GenWaveBuzzA : public GenWaveWT
{
private:
	bsInt32 numHarm;
	GenWaveWT *osca;
	GenWaveWT *oscb;
	AmpValue ampScale;
	AmpValue a0;
	AmpValue a1;
	FrqValue num2p1;

public:
	GenWaveBuzzA()
	{
		numHarm = 1;
		ampScale = 0.5;
		num2p1 = 2;
		osca = 0;
		oscb = 0;
		bsInt32 midpt = synthParams.itableLength/2;
		a0 = wtSet.wavSin[midpt-1];
		a1 = wtSet.wavSin[midpt+1];
	}

	~GenWaveBuzzA()
	{
		delete osca;
		delete oscb;
	}

	/// Set the numerator oscillator.
	void SetOscillatorA(GenWaveWT *o)
	{
		delete osca;
		osca = o;
	}

	/// Set the denomarator oscillator.
	void SetOscillatorB(GenWaveWT *o)
	{
		delete oscb;
		oscb = o;
	}


	/// @copydoc GenWave::Init
	void Init(int n, float *v)
	{
		if (n > 1)
			InitBuzz(v[0], (int)v[1]);
	}

	/// Set the number of harmonics.
	/// The higher n, the narowwer the pulse.
	/// @param n number of harmonics (0 < n < (SR-Fo)/2Fo)
	void SetHarmonics(bsInt32 n)
	{
		numHarm = n;
	}

	/// Initialize the Buzz generator
	/// @param f frequency
	/// @param n number of harmonics (0 = maximum)
	void InitBuzz(FrqValue f, bsInt32 n)
	{
		SetFrequency(f);
		SetHarmonics(n);
		Reset();
	}

	/// @copydoc GenWave::Reset
	void Reset(float initPhs = 0.0)
	{
		bsInt32 maxN = (bsInt32) ((synthParams.sampleRate - frq) / (2.0 * frq)) - 1;
		bsInt32 N = (numHarm > maxN) ? maxN : numHarm;
		if (N <= 0)
		{
			ampScale = 1.0;
			num2p1 = 1.0;
		}
		else
		{
			ampScale = 0.5 / AmpValue(N); // 1.0 / 2N
			num2p1 = FrqValue(N + N + 1); // 2N + 1
		}
		if (osca == NULL)
		{
			osca = new GenWaveWT;
			osca->SetWavetable(WT_SIN);
		}
		if (oscb == NULL)
		{
			oscb = new GenWaveWT;
			oscb->SetWavetable(WT_SIN);
		}
		FrqValue f = frq * 0.5;
		osca->SetFrequency(f * num2p1);
		osca->Reset(initPhs);
		oscb->SetFrequency(f);
		oscb->Reset(initPhs);
	}

	/// @copydoc GenWave::Modulate
	void Modulate(FrqValue d)
	{
		FrqValue f = (frq + d) * 0.5;
		osca->SetFrequency(f * num2p1);
		oscb->SetFrequency(f);
		osca->Reset(-1);
		oscb->Reset(-1);
	}

	/// @copydoc GenWaveWT::PhaseModWT
	virtual void PhaseModWT(PhsAccum phs)
	{
		phs *= 0.5;
		osca->PhaseModWT(phs*num2p1);
		oscb->PhaseModWT(phs);
	}

	/// @copydoc GenWave::Gen
	AmpValue Gen()
	{
		AmpValue b = oscb->Gen();
		AmpValue a = osca->Gen();
		//if (b != 0.0)
		if (b > a0 || b < a1)
			return ampScale * ((a / b) - 1.0);
		return 1.0;
	}
};

//@}
#endif

