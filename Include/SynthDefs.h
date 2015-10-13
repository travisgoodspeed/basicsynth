///////////////////////////////////////////////////////////////
//
// BasicSynth - SynthDefs
//
/// @file SynthDefs.h Global synthesizer definitions
///
/// This file must be included in all programs that use the BasicSynth library.
//
// Copyright 2008,2009 Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpGeneral
//@{


#ifndef _SYNTHDEFS_H_
#define _SYNTHDEFS_H_

#include <SynthString.h>

// This pragma gets rid of the irritating MSVC warning about 
// "double truncation to float" because the little 'f' is not 
// at the end of the constant. You might leave the warning on
// during initial code development.
#ifdef _MSC_VER
#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#endif

/// sample output type (16-bit for PCM)
typedef short SampleValue;
/// 8-bit data type
typedef char  bsInt8;
/// 16-bit data type 
typedef short bsInt16;
/// 32-bit data type 
typedef int   bsInt32;
/// 8-bit unsigned type
typedef unsigned char  bsUint8;
/// 16-bit unsigned type
typedef unsigned short bsUint16;
/// 32-bit unsigned type
typedef unsigned int   bsUint32;
/// transparent data type
typedef void* Opaque;

// Choose one of these as the oscillator phase accumulator type.
// double gives less noise with slightly more calculation time.
/// Type for a phase accumulator
typedef double PhsAccum;
#define sinv sin
//typedef float PhsAccum;
//#define sinv sinf

/// Type for an amplitude value
typedef float AmpValue;
/// Type for a high precision amplitude value
typedef double AmpValue2;
/// Type for a frequency or time value
typedef float FrqValue;
/// Type for a generic parameter
typedef float ParamValue;

#define PI 3.14159265358979
#define twoPI 6.28318530717958

#define PANTBLLEN 4096
#define MAX_AMPCB 1440

/// Global parameters for the synthesizer. SynthConfig holds global information for the synthesizer.
/// This includes sample rate, wave table size, phase increment calculation constants, 
/// and the tuning table for conversion of pitch to frequency. Exactly one instance of 
/// this class exists and is named \ref synthParams. Library classes utilize this object rather
/// than store sample rate and other parameters internally. 
class SynthConfig
{
public:
	/// Sample rate
	FrqValue sampleRate;
	/// Maximum frequency (Nyquist limit)
	FrqValue nyquist;
	/// Sample rate as integer
	bsInt32 isampleRate;
	/// multiplier to convert internal sample values into output values
	AmpValue sampleScale;
	/// pre-calculated multipler for frequency to radians (twoPI/sampleRate)
	PhsAccum frqRad;
	/// pre-calculated multipler for frequency to table index (tableLength/sampleRate)
	PhsAccum frqTI;
	/// pre-calculated multipler for radians to table index (tableLength/twoPI)
	PhsAccum radTI;
	/// wave table length
	PhsAccum ftableLength;
	/// wave table length as integer
	bsInt32  itableLength;
	/// maximum phase increment for wavetables (ftableLength/2)
	PhsAccum maxIncrWT;
	/// table to convert pitch index into frequency
	FrqValue tuning[128];
	/// table to convert cents +/-1200 into frequency multiplier
	FrqValue cents[2401];
	/// first quadrant of a sine wave (used by Panner)
	AmpValue sinquad[PANTBLLEN];
	/// square roots of 0-1 (used by Panner)
	AmpValue sqrttbl[PANTBLLEN];
	/// pan table index scaling
	bsInt32  sqNdx;
	/// wave file path (semi-colon separated)
	bsString wvPath;
	/// cB value table
	AmpValue cbVals[MAX_AMPCB];

	/// Constructor. The constructor for \p SynthConfig initializes
	/// member variables to default values by calling \p Init().
	/// In addition, the pitch-to-frequency table is filled with values
	/// for an equal-tempered scale.
	SynthConfig()
	{
		Init();

		size_t sampleBits = (sizeof(SampleValue) * 8) - 1; // -1 because a sample is a signed value.
		sampleScale = (AmpValue) ((1 << sampleBits) - 1);

		int i;
		// Equal tempered tuning system at A4=440 (Western standard)
		// Middle C = C4 = index 48
		double frq = 13.75 * pow(2.0, 0.25); // C1 = A0*2^(3/12) = 16.35159...
		double two12 = pow(2.0, 1.0/12.0); // 2^(1/12) = 1.059463094...
		for (i = 0; i < 128; i++)
		{
			tuning[i] = (FrqValue) frq;
			frq *= two12;
			//printf("%d = %f\n", i, tuning[i]);
		}

		frq = -1200;
		for (i = 0; i <= 2400; i++)
		{
			cents[i] = (FrqValue) pow(2.0, frq/1200.0);
			frq += 1.0;
		}

		// lookup tables for non-linear panning
		sqNdx = PANTBLLEN-1;
		double scl = sqrt(2.0) / 2.0;
		double phs = 0;
		double phsInc = (PI/2) / PANTBLLEN;
		double sqInc = 1.0 / PANTBLLEN;
		double sq = 0.0;
		for (i = 0; i < PANTBLLEN; i++)
		{
			sqrttbl[i] = sqrt(sq) * scl;
			sinquad[i] = sin(phs) * scl;
			phs += phsInc;
			sq += sqInc;
		}

		// lookup table for centibels attenuation to amplitude
		double lvl = 0;
		for (i = 0; i < MAX_AMPCB; i++)
		{
			cbVals[i] = (AmpValue) pow(10.0, (double) lvl / -200.0);
			lvl += 1.0;
		}
	}

	/// Initialize values based on sample rate.
	/// Init is called automatically from the constructor but may also be called
	/// directly to change the sample rate and default wavetable length.
	/// @note All unit generators should be deleted or reset after calling this
	/// function as they may have internal values calculated based on sample rate
	/// or table length. In addition, the wavetables must be reinitialized if
	/// the wavetable length changes.
	/// @param sr sample rate, default 44.1K
	/// @param tl wavetable length, default 16K
	void Init(bsInt32 sr = 44100, bsInt32 tl = 16384)
	{
		sampleRate = (FrqValue) sr;
		nyquist = sampleRate * 0.5;
		isampleRate = sr;
		itableLength = tl;
		ftableLength = (PhsAccum) tl;
		maxIncrWT = ftableLength * 0.5;
		frqRad = twoPI / (PhsAccum) sampleRate;
		frqTI = ftableLength / (PhsAccum) sampleRate;
		radTI = ftableLength / twoPI;
	}

	/// Convert pitch index to frequency.
	/// The frequency table is built in the constructor based on an equal-tempered
	/// scale with middle C at index 48. Since the tuning array is a public member, it is
	/// possible to overwrite the values from anywhere in the system by updating the array
	/// directly. Negative pitch values are allowed but are calculated directly using
	/// the equation for equal-tempered tuning (f * 2^n/12)
	/// @param pitch pitch index
	/// @returns frequency in Hz
	FrqValue GetFrequency(int pitch)
	{
		if (pitch < 0 || pitch > 127)
			return tuning[0] * pow(2.0, (double) pitch / 12.0);
		return tuning[pitch];
	}

	/// Convert cents to frequency multiplier.
	/// Cents represent a frequency variation of 1/100 semitone
	/// and should range +/- one octave [-1200,+1200]. Values outside
	/// that range result in a direct calculation. The returned
	/// value can be multiplied by a base frequency to get the detuned
	/// frequency.
	/// @param c deviation in pitch in 1/100 of a semitone.
	/// @returns frequency multiplier
	FrqValue GetCentsMult(int c)
	{
		if (c < -1200 || c > 1200)
			return pow(2.0, (double) c / 1200);
		return cents[c + 1200];
	}

	/// Convert cB (centibel) of attenuation into amplitude level.
	/// cb = -200 log10(amp). Each bit in the sample represents ~60cB. 
	/// For 16-bits resolution the useful range is 0-960, 
	/// 24 bits 0-1440, 32 bits 0-1920, etc.
	/// @param cb centi-bels of attenuation
	/// @returns linear amplitude value
	AmpValue AttenCB(int cb)
	{
		if (cb <= 0)
			return 1.0;
		if (cb >= MAX_AMPCB)
			return 0.0;
		return cbVals[cb];
	}

	/// Function to locate a file on the wave file path (synthParams.wvPath).
	/// @param fullPath output string where the full path is placed
	/// @param fname file name to search for
	/// @returns non-zero if the file was located, zero otherwise
	int FindOnPath(bsString& fullPath, const char *fname);

};

/// Global synthesizer parameteres object
/// This global must be defined somewhere in the main code.
/// The simplest way is to include the common library. 
/// It initializes automatically in the constructor to default values,
/// but typically InitSynthesizer() is called to initialize the values.
extern SynthConfig synthParams;

/// Initialize the global synthesizer parameters and wavetables.
/// Both synthParams and wtSet must be initialized before using any other
/// class or function in the library. This is easily accomplished by calling InitSynthesizer 
/// during program startup or after synthesizer settings have been configured.
/// @note InitSynthesizer can be called multiple times. However, you MUST delete all
/// unit generators first. Many oscillators hold pointers into the wave table set
/// or calculate local values once based on the current sample rate.
extern int InitSynthesizer(bsInt32 sampleRate = 44100, bsInt32 wtLen = 16384, bsInt32 wtUsr = 0);

/// A block of samples.
/// A SampleBlock structure is used to buffer a block of samples.
/// The size member defines the size of the in and out blocks.
class SampleBlock
{
public:
	/// size of the sample block in frames
	int size;
	/// input buffer containing current values
	AmpValue *in;
	/// output buffer containing generated or processed values
	AmpValue *out;
	SampleBlock()
	{
		size = 0;
		in = NULL;
		out = NULL;
	}
};

/// Unit generator.
/// A unit generator is any class that can generate samples or control signals.
/// GenUnit is the base class for oscillators, envelope generators, filters, and delay lines,
/// and is generally not used directly. However, it can be instantiated to produce a null generator.
/// (A null generator is sometimes useful as a place-holder for a dynamically set unit generator
/// and avoids repeatedly testing the variable to see if it is a null pointer.)
/// Derived classes should implement the Init, Reset, and Sample methods.
class GenUnit
{
public:
	virtual ~GenUnit() { }

	/// Initialize the generator.
	/// The Init method sets initial values for the object. The count argument
	/// indicates the number of values in the values array. This method provides
	/// an opaque mechanism for initializing unit generators. The caller can load 
	/// values from a file or other source and pass them to the generator unit without
	/// interpretation. Most generator units will supply additional initialization
	/// and access methods where each value is passed as an explicit argument. 
	/// The latter form of initialization is preferred as it allows for effective type and range checking.
	/// @param count number of values
	/// @param values array of values
	virtual void Init(int count, float *values) { }

	/// Reset the generator.
	/// The Reset method is called to force the generator unit to update any internal
	/// values that are dependent on properties or initial values and enter a known state.
	/// For example, an oscillator would calculate the phase increment based on the last set 
	/// frequency value, while an envelope generator would calculate the slope of the initial segment. 
	///
	/// The initPhs argument indicates the reset condition. The range varies based on the 
	/// type of object. For an oscillator, phase has a range of [0,2π], while for an 
	/// envelope generator, the phase might indicate a time in seconds. A value of 0 
	/// indicates the object should reset to a condition prior to any samples being generated. 
	/// A value greater than 0 indicates a condition after some samples have been generated. 
	/// A value of -1 indicates the object should apply any changed parameter values, 
	/// but should not otherwise change the current phase. Not all generator units utilize the initPhs argument.
	/// @param initPhs initial phase
	virtual void Reset(float initPhs = 0) { }

	/// Return the next sample.
	/// The Sample method is invoked to generate one sample. The \e in argument represents
	/// the current amplitude of the current sample. The use of \e in varies with the unit generator.
	/// Objects that modify the current sample (e.g., filters, delay lines) may store the value
	/// and return a filtered value. Objects that generate new samples (e.g., oscillators, envelope generators)
	/// will typically treat the value as an amplitude multiplier.
	/// @param in current sample amplitude
	/// @returns generated or processed sample vaule
	virtual AmpValue Sample(AmpValue in) { return 0; }

	/// Return a block of samples.
	/// A convienience function that will call Sample to return a block of values
	/// @param block structure to hold a block of samples, initialized by the caller.
	virtual void Samples(SampleBlock *block)
	{
		int n = block->size;
		AmpValue *in = block->in;
		AmpValue *out = block->out;
		while (--n >= 0)
			*out++ = Sample(*in++);
	}

	/// Determine if the unit generator can stop.
	/// IsFinished() is called to determine when a generator has produced all the valid sample 
	/// values it can produce. Oscillators are always considered finished and always return true.
	/// Envelope generators return true when the end of the envelope cycle is reached. 
	/// Other unit generator return true or false based on their individual requirements, 
	/// but for the most part, should always return true.
	virtual int IsFinished()
	{
		return 1;
	}
};

//@}

#endif
