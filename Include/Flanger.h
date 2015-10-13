/////////////////////////////////////////////////////////////////////
// BasicSynth Flanger/Chorus unit. 
//
/// @file Flanger.h Flanger/Chorus unit
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////
/// @addtogroup grpMix
//@{
#ifndef _FLANGER_H_
#define _FLANGER_H_

/// Flanger class
/// Uses a variable tap delay line
/// to modulate the delay. Allows feedback to provide for allpass filtering. 
///
/// See: Jon Dattorro, "Effect Design", Journal of the Audio Engineering
///      Society, Vol 45, No. 10, 1997 October, p. 764
class Flanger : public GenUnit
{
protected:
	DelayLineV dlv;       // delay line
	GenWave32 wv;         // modulation oscillator
	AmpValue dlyLvl;      // input signal level (0-1)
	AmpValue dlyMix;      // mix (blend) value (0-1)
	AmpValue dlyFeedback; // feedback level (0-1)
	PhsAccum dlyRange;    // delay depth in sec.
	PhsAccum dlyCenter;   // delay center in sec.

public:
	Flanger()
	{
		dlyRange = 0;
		dlyCenter = 0;
		dlyLvl = 0;
		dlyMix = 0;
		dlyFeedback = 0;
	}

	/// Clear the delay line buffer to zero
	void Clear()
	{
		dlv.Clear();
	}

	/// Copy the settings
	void Copy(Flanger *fp)
	{
		dlyLvl = fp->dlyLvl;
		dlyMix = fp->dlyMix;
		dlyFeedback = fp->dlyFeedback;
		dlyRange = fp->dlyRange;
		dlyCenter = fp->dlyCenter;
		wv.SetFrequency(fp->wv.GetFrequency());
		dlv.Copy(&fp->dlv);
	}

	/// Return the flanger settings
	/// @param inlvl input lelvel
	/// @param mix mix of dry/wet signal
	/// @param fb feedback value
	/// @param center nominal delay time
	/// @param depth variation in delay time
	/// @param sweep sweep oscillator frequency
	void GetSettings(AmpValue& inlvl, AmpValue& mix, AmpValue& fb, FrqValue& center, FrqValue& depth, FrqValue& sweep)
	{
		inlvl = dlyLvl;
		mix = dlyMix;
		fb = dlyFeedback;
		center = dlyCenter / synthParams.sampleRate;
		depth = (dlyRange * 2) / synthParams.sampleRate;
		sweep = wv.GetFrequency();
	}

	/// Initialize the flanger from an array of values.
	void Init(int n, float *v)
	{
		if (n >= 5)
			InitFlanger(AmpValue(v[0]), AmpValue(v[1]), AmpValue(v[2]), FrqValue(v[3]), FrqValue(v[4]), FrqValue(v[5]));
	}

	/// Reset to an initial state
	void Reset(float initPhs = 0)
	{
		dlv.Reset(initPhs);
		wv.Reset(initPhs);
	}

	/// Initialize the flanger
	/// @param inlvl input lelvel
	/// @param mix mix of dry/wet signal
	/// @param fb feedback value
	/// @param center nominal delay time
	/// @param depth variation in delay time
	/// @param sweep sweep oscillator frequency
	void InitFlanger(AmpValue inlvl, AmpValue mix, AmpValue fb, FrqValue center, FrqValue depth, FrqValue sweep)
	{
		wv.InitWT(sweep, WT_SIN);
		if ((center - depth) < 0)
			center = depth / 2;
		dlv.InitDL(center + (depth/2), mix);
		dlyLvl = inlvl;
		dlyMix = mix;
		dlyFeedback = fb;
		dlyRange = (depth * synthParams.sampleRate) / 2;
		dlyCenter = (center * synthParams.sampleRate);
	}

	/// Pass the sample through the flanger unit.
	/// @param inval current sample
	/// @returns processed sample
	AmpValue Sample(AmpValue inval)
	{
		if (dlyFeedback != 0)
			inval -= dlv.Tap(dlyCenter) * dlyFeedback;
		dlv.SetDelay(dlyCenter + (dlyRange * wv.Gen()));
		return (inval * dlyLvl) + dlv.Sample(inval);
	}
};
//@}
#endif
