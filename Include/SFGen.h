///////////////////////////////////////////////////////////
// BasicSynth - SoundFont unit generators
//
/// @file SFGen.h SoundFont(R) unit generators
//
// These class derive from standard ugens but are initialized from SoundFont file data.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

#ifndef SFGEN_H
#define SFGEN_H

/// Oscillator that initializes directly from a SBZone.
class GenWaveSB : public GenWaveWTLoop
{
public:
	/// Init the wavetable oscillator.
	/// @param zone sample information.
	/// @param pi frequency in phase increment
	/// @param skipAttack when true, start at the loop section.
	void InitSB(SBZone *zone, PhsAccum pi, int skipAttack = 0)
	{
		//frq = fo;
		phsIncr = pi;
		recFrq = zone->recFreq;
		rateRatio = zone->rate / synthParams.sampleRate;
		piMult = rateRatio / recFrq; // pre-calculate for Modulate code
		period = zone->rate / recFrq;
		phase = (PhsAccum) zone->tableStart;
		tableEnd = (PhsAccum) zone->tableEnd;
		loopStart = (PhsAccum) zone->loopStart;
		loopEnd = (PhsAccum) zone->loopEnd;
		loopLen = loopEnd - loopStart;
		loopMode = zone->mode;
		if (skipAttack && loopMode == 1)
			phase = loopStart;
		wavetable = zone->sample->sample;
	}

	inline void UpdatePhaseIncr(PhsAccum p)
	{
		phsIncr = p;
	}

};

/// Envelope generator for sound founts.
/// A sound bank (SF2 or DLS) uses a six segment envelope --
/// delay, attack, hold, decay, sustain, release.
/// This EG differs from the typical BasicSynth EG in the following ways:
/// 1) Start, peak and end levels are always normalized to [0,1,0]
/// 2) Sustain is a percentage of peak.
/// 3) Attack, decay and release are constant-rate calculations.
class EnvGenSB : public GenUnit
{
private:
	AmpValue curLevel;
	AmpValue susLevel;
	AmpValue atkIncr;
	AmpValue decIncr;
	AmpValue relIncr;
	bsInt32  delayCount;
	bsInt32  holdCount;
	int      segNdx;
public:
	EnvGenSB()
	{
		curLevel = 0.0;
		susLevel = 0.0;
		atkIncr = 1.0;
		decIncr = 1.0;
		relIncr = 1.0;
		delayCount = 0;
		holdCount = 0;
		segNdx = 6;
	}

	/// Initialize from an array of floats.
	void Init(int n, float *v)
	{
		if (n >= 6)
		{
			SetDelay(v[0]);
			SetAttack(v[1]);
			SetHold(v[2]);
			SetDecay(v[3]);
			SetSustain(v[4]);
			SetRelease(v[5]);
			Reset(0);
		}
	}

	inline void Stop()
	{
		segNdx = 6;
		curLevel = 0;
	}

	inline void SetDelay(FrqValue rt)
	{
		delayCount = (bsInt32) (synthParams.sampleRate * rt);
	}

	inline void SetAttack(FrqValue rt)
	{
		FrqValue count = floor(synthParams.sampleRate * rt);
		if (count > 0)
			atkIncr = 1.0 / count;
		else
			atkIncr = 1.0;
	}

	inline void SetHold(FrqValue rt)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * rt);
	}

	inline void SetDecay(FrqValue rt)
	{
		FrqValue count = floor(synthParams.sampleRate * rt);
		if (count > 0)
			decIncr = 1.0 / count;
		else
			decIncr = 1.0;
	}

	inline void SetSustain(AmpValue n)
	{ 
		susLevel = n; 
	}

	inline void SetRelease(FrqValue rt)
	{
		FrqValue count = floor(synthParams.sampleRate * rt);
		if (count > 0)
			relIncr = 1.0 / count;
		else
			relIncr = 1.0;
	}

	inline void SetSegment(int n)
	{
		segNdx = n; 
	}

	inline int GetSegment()
	{
		return segNdx;
	}

	inline void Reset(float initPhs)
	{
		if (initPhs >= 0)
		{
			curLevel = 0;
			segNdx = 0;
		}
	}

	inline int IsFinished()
	{
		return segNdx > 5;
	}

	inline void AdjustRelease()
	{
		// only for volume envelope
		if (segNdx < 2)
		{
			// still in attack.
			curLevel = 1.0 - (-200.0/960.0 * log10(curLevel));
			segNdx = 5;
		}
		else if (segNdx < 5)
			segNdx = 5;
	}

	inline void Release()
	{
		if (segNdx < 5)
			segNdx = 5;
	}

	AmpValue Gen()
	{
		switch (segNdx)
		{
		case 0: // delay
			if (--delayCount >= 0)
				break;
			segNdx++;
		case 1: // attack
			curLevel += atkIncr;
			if (curLevel >= 1.0)
			{
				segNdx++;
				return curLevel = 1.0;
			}
			return curLevel;
		case 2: // hold
			if (--holdCount >= 0)
				break;
			segNdx++;
		case 3: // decay
			curLevel -= decIncr;
			if (curLevel > susLevel)
				break;
			segNdx++;
			curLevel = susLevel;
		case 4: // sustain
			break;
		case 5: // release
			curLevel -= relIncr;
			if (curLevel > 0.0)
				break;
			segNdx++;
			curLevel = 0.0;
			break;
		case 6:
		//default:
			return 0.0;
		}
		return curLevel;
	}
};


//@}
#endif
