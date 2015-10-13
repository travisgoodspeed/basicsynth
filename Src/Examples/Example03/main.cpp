///////////////////////////////////////////////////////////
// BasicSynth - Example 3 (Chapter 6)
//
// Program to calculate complex waveforms
// 1 - summation of first 8 partials
// 2 - sawtooth wave
// 2a - inverse saw wave
// 3 - triangle wave
// 4 - square wave
// 4a - 25% pulse wave
// 5 - frequency modulation
// 6 - phase modulation
// 7 - amplituded modulation
// 8 - ring modulation
// 9 - noise
// 10 - pulse wave
// 11 - discrete summation forumla
// 12 - bandwidth version of #11
// 13 - waveshaping
//
// use: Example03 [duration [pitch [volume]]]
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "EnvGen.h"

#define twoDivPI (2.0/PI)

int main(int argc, char *argv[])
{
	int pitch = 48;  // Middle C
	FrqValue duration = 1.0;
	AmpValue peakAmp = 0.707;
	long n;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		peakAmp = atof(argv[3]);

	InitSynthesizer();
	FrqValue frequency = synthParams.GetFrequency(pitch);
	int maxHarm = (int) (synthParams.sampleRate / (2.0 * frequency)) - 1;

	WaveFile wf;
	if (wf.OpenWaveFile("example03.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	long numSounds = 9;

	EnvGen eg;
	eg.InitEG(peakAmp, duration, 0.1, 0.2);

	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);

	/////////////////////////////////////////////////
	// 1 - summation of sine waves
	/////////////////////////////////////////////////
#define NUMPARTS 8
	PhsAccum phase[NUMPARTS];
	AmpValue ampPartial[NUMPARTS];
	FrqValue partNums[NUMPARTS] = {1, 2, 3, 4, 5, 6, 7, 8}; // sawtooth wave
//	FrqValue partNums[NUMPARTS] = {1, 3, 5, 7, 9, 11, 13, 15}; // "Square" wave
	PhsAccum phsIncr[NUMPARTS];
	int partMax = NUMPARTS;

	// limit to 2-4 partials to produce fewer upper
	// harmonics, a more "mellow" sound
	//partMax = 4;

	phsIncr[0] = synthParams.frqRad * frequency;
	phase[0] = 0.0f;
	ampPartial[0] = 1.0f;

	// Bandwidth limiting code -
	int partCount = 1;
	while (partCount < partMax)
	{
		PhsAccum tmp = phsIncr[0] * partNums[partCount];
		if (tmp >= PI)
			break;
		phsIncr[partCount] = tmp;
		phase[partCount] = 0;
		ampPartial[partCount] = 1 / partNums[partCount];
		partCount++;
	}


	// scale determined "empirically" - 
	// we can just add the amplitudes of partials, but 
	// usually won't get maximum amp depending on phase
	// of harmonics.
	// choose the maximum absolute amp value for scale
	AmpValue posMax = 0.f;
	AmpValue scale = 1.43f;   // sawtooth, 8 parts
	//scale = 1.53; // sawtooth, 4 parts, no gibbs
	//scale = 0.9274f; // Square wave

	// Negating the amplitude of even numbered overtones
	// will invert a sawtooth into a ramp...
	for (n = 1; n < partMax; n += 2)
		ampPartial[n] = -ampPartial[n];

	// Increasing amplitudes of overtones creates narrower "pulse"
	//for (n = 1; n < partMax;  n++)
	//	ampPartial[n] *= 2.0f;
	//scale = 3.02f;

	// sigma calculation to correct for Gibbs phenonmenon
	AmpValue sigmaK = PI / (float) partCount;
	AmpValue sigmaN = 0;
	AmpValue ampN;

	eg.Reset();
	AmpValue value = 0;
	for (n = 0; n < totalSamples; n++)
	{
		value = 0;
		sigmaN = 0;
		for (int p = 0; p < partCount; p++)
		{
			ampN = ampPartial[p];
			if (p > 0)
				ampN *= sinf(sigmaN) / sigmaN;
			sigmaN += sigmaK;
			value = value + (sinf(phase[p]) * ampN);
			phase[p] = phase[p] + phsIncr[p];
			if (phase[p] >= twoPI)
				phase[p] = phase[p] - twoPI;
		}
		if (fabs(value) > posMax)
			posMax = value;
		wf.Output1(eg.Gen() * (value / scale));
	}
	//printf("max amp=%1.8f, %i\n", posMax, wf.GetOOR());

	/////////////////////////////////////////////////
	// 2 - sawtooth wave by summation
	/////////////////////////////////////////////////
	PhsAccum phs1 = 0;
	PhsAccum phsIncr1 = synthParams.frqRad * frequency;
	PhsAccum phsN;
	scale = 1.5;

	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = 0;
		phsN = phs1;
		for (int p = 1; p <= maxHarm; p++) 
		{
			//phsN = fmod(phs1 * (double)p, twoPI);
			value += sin(phsN) / (double)p;
			if ((phsN += phs1) >= twoPI)
				phsN -= twoPI;
		}
		if ((phs1 += phsIncr1) >= twoPI)
			phs1 -= twoPI;
		wf.Output1(eg.Gen() * value / scale);
	}

	/////////////////////////////////////////////////
	// 2a - sawtooth wave by direct calculation
	/////////////////////////////////////////////////

	AmpValue sawIncr = (2 * frequency) / synthParams.sampleRate;
	AmpValue sawValue = -1;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sawValue);
		if ((sawValue += sawIncr) >= 1)
			sawValue -= 2;
	}

	/////////////////////////////////////////////////
	// 2b - inverse sawtooth wave
	/////////////////////////////////////////////////
	sawValue = 1;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sawValue);
		if ((sawValue -= sawIncr) <= -1)
			sawValue += 2;
	}

	/////////////////////////////////////////////////
	// 3 - triangle wave
	/////////////////////////////////////////////////
	AmpValue triValue = 0;
	PhsAccum triPhase = 0;
	PhsAccum triIncr  = synthParams.frqRad * frequency;
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		triValue = (AmpValue)(triPhase * twoDivPI);
		if (triValue < 0)
			triValue = 1.0f + triValue;
		else
			triValue = 1.0f - triValue;
		wf.Output1(eg.Gen() * triValue);
		if ((triPhase += triIncr) >= PI)
			triPhase -= twoPI;
	}

	triIncr = 4 * frequency / synthParams.sampleRate;
	triValue = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * triValue);
		if ((triValue += triIncr) > 1.0)
		{
			triValue -= triIncr;
			triIncr = -triIncr;
		}
		else if (triValue < -1.0)
		{
			triValue -= triIncr;
			triIncr = -triIncr;
		}
	}
	
	/////////////////////////////////////////////////
	// 4 - square waves
	/////////////////////////////////////////////////
	phs1 = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = 0;
		phsN = phs1;
		for (int p = 1; p <= maxHarm; p += 2) 
		{
			value += sin(phsN) / (double)p;
			if ((phsN += (phs1+phs1)) >= twoPI)
				phsN -= twoPI;
		}
		if ((phs1 += phsIncr1) >= twoPI)
			phs1 -= twoPI;
		wf.Output1(eg.Gen() * value);
	}

	PhsAccum sqPhase;
	PhsAccum sqMidPoint;
	PhsAccum sqPeriod;
	
	// Calculate using radians
	PhsAccum sqPhsIncr;
	sqPhase = 0;
	sqPhsIncr = synthParams.frqRad * frequency;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if ((sqPhase += sqPhsIncr) >= PI)
			sqPhase -= twoPI;
	}

	// Calculate using time
	sqPhsIncr = 1.0 / synthParams.sampleRate;
	sqPeriod = 1.0 / frequency;
	sqMidPoint = sqPeriod / 2;
	sqPhase = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if ((sqPhase += sqPhsIncr) >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	// Calculate using samples
	sqPeriod = synthParams.sampleRate / frequency;
	sqMidPoint = sqPeriod / 2;
	sqPhase = 0;
	//long sqPeriodi = (long) ((synthParams.sampleRate / frequency) + 0.5);
	//long sqMidPointi = sqPeriod / 2;
	//long sqPhasei = 0;
	eg.Reset();
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if (++sqPhase >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	/////////////////////////////////////////////////
	// 4a - 25% pulse wave
	/////////////////////////////////////////////////
	sqPhase = 0;
	sqMidPoint = sqPeriod / 4; // = (sqPeriod * dutyCycle) / 100
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen();
		if (sqPhase >= 0)
			wf.Output1(value);
		else
			wf.Output1(-value);
		if (++sqPhase >= sqMidPoint)
			sqPhase -= sqPeriod;
	}

	/////////////////////////////////////////////////
	// 5 - Frequency Modulation (FM)
	/////////////////////////////////////////////////
	FrqValue modFrequency = frequency * 3;
	PhsAccum modIncr = synthParams.frqRad * modFrequency;
	PhsAccum modPhase = 0;
	PhsAccum carIncr = 0;
	PhsAccum carPhase = 0;
	AmpValue modAmp = 2 * modFrequency;
	AmpValue modValue = 0;

	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase));
		modValue = modAmp * sinf(modPhase);
		carIncr = synthParams.frqRad * (frequency + modValue);
		carPhase = carPhase + carIncr;
		modPhase = modPhase + modIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		else if (carPhase < 0)
			carPhase += twoPI;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 6 - Phase Modulation (PM)
	/////////////////////////////////////////////////
	carIncr = synthParams.frqRad * frequency;
	modAmp = synthParams.frqRad * (2 * modFrequency); // 2000.0f; // convert frequency to radians/sec
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase));
		modValue = modAmp * sinf(modPhase);
		carPhase = carPhase + carIncr + modValue;
		modPhase = modPhase + modIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		else if (carPhase < 0)
			carPhase += twoPI;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 7 - Amplitude Modulation (AM)
	/////////////////////////////////////////////////
	modFrequency = frequency * 2.5;
	modIncr = synthParams.frqRad * modFrequency;
	modAmp = 1.0;
	carPhase = 0;
	modPhase = 0;
	AmpValue modScale = 1 / (1 + modAmp);
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		modValue = 1.0 + (modAmp * sinf(modPhase));
		wf.Output1(eg.Gen() * (sinf(carPhase) * modValue) * modScale);
		carPhase += carIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		modPhase += modIncr;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 8 - Ring Modulation (AM)
	/////////////////////////////////////////////////
	modAmp = 1.0;
	carPhase = 0;
	modPhase = 0;
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(eg.Gen() * sinf(carPhase) * modAmp * sinf(modPhase));
		carPhase += carIncr;
		if (carPhase >= twoPI)
			carPhase -= twoPI;
		modPhase += modIncr;
		if (modPhase >= twoPI)
			modPhase -= twoPI;
	}

	/////////////////////////////////////////////////
	// 9 - White noise
	/////////////////////////////////////////////////
	eg.Reset();

	for (n = 0; n < totalSamples; n++)
	{
		value = ((AmpValue) rand() - (RAND_MAX/2)) / (RAND_MAX/2);
		wf.Output1(eg.Gen() * value);
	}

	/////////////////////////////////////////////////
	// 10 - Pulse wave (BUZZ)
	/////////////////////////////////////////////////
	PhsAccum maxN = floor(synthParams.sampleRate / (2.0 * frequency) - 0.5) - 1;
	PhsAccum phsIncrDen = (PI / synthParams.sampleRate) * frequency;
	PhsAccum phsIncrNum = phsIncrDen * ((2.0 * maxN) + 1);
	PhsAccum phsDen = 0.0;
	PhsAccum phsNum = 0.0;
	AmpValue2 ampScl = 1.0 / (2.0 * maxN);
	AmpValue2 twoNp1 = (2.0 * maxN) + 1.0;

	eg.Reset();
	for (n = 0; n < totalSamples; n++) 
	{
		AmpValue den = sin(phsDen);
		if (den == 0.0)
			value = 1.0; // = ampScl * (((twoNp1 * cos(phsNum)) / cos(phsDen)) - 1);
		else
			value = ampScl * ((sin(phsNum) / den) - 1);
		wf.Output1(eg.Gen() * value);
		phsDen += phsIncrDen;
		if (phsDen >= twoPI)
			phsDen -= twoPI;
		phsNum += phsIncrNum;
		if (phsNum >= twoPI)
			phsNum -= twoPI;
	}

	/////////////////////////////////////////////////
	// 10a - Pulse wave (BUZZ)
	// Variation of BUZZ that produces positive and negative peaks.
	// The timbre is the same, and this is difficult to scale
	// correctly, making the code above the preferred version.
	/////////////////////////////////////////////////
	phsDen = 0;
	ampScl = 1.0 / maxN;
	eg.Reset();
	for (n = 0; n < totalSamples; n++) 
	{
		AmpValue den = sin(phsDen);
		if (den != 0.0)
			value = ampScl * sin(maxN * phsDen) * sin((maxN+1) * phsDen) / den;
		else
			value = 1.0;
		wf.Output1(eg.Gen() * value);
		phsDen += phsIncrDen;
		if (phsDen >= twoPI)
			phsDen -= twoPI;
	}

	/////////////////////////////////////////////////
	// 11 - Dynamic spectrum using closed form with (N = INF)
	/////////////////////////////////////////////////
	AmpValue2 ampRatio;
	for (ampRatio = 0.3; ampRatio < 1.0; ampRatio += 0.11)
	{
		PhsAccum phsIncr = synthParams.frqRad * frequency;
		PhsAccum phsN = 0.0;
		AmpValue2 ampTwo = ampRatio + ampRatio;
		AmpValue2 ampSqrP1 = 1.0 + (ampRatio * ampRatio);
		AmpValue2 ampSqrM1 = 1.0 - (ampRatio * ampRatio);

		eg.Reset();
		for (n = 0; n < totalSamples; n++) 
		{
			value = ampSqrM1 * sin(phsN) / (ampSqrP1 - (ampTwo * cos(phsN)));
			wf.Output1(eg.Gen() * value);
			phsN += phsIncr;
			if (phsN >= twoPI)
				phsN -= twoPI;
		}
	}

	/////////////////////////////////////////////////
	// 12 - Bandwidth limited version of 11
	/////////////////////////////////////////////////
	PhsAccum harm = floor(synthParams.sampleRate / (2.0 * frequency)) - 1;
	FrqValue beta = frequency; // beta = 2*frequency for odd harmonics
	for (ampRatio = 0.3; ampRatio < 1.0; ampRatio += 0.11)
	{
		PhsAccum phsIncr[5];
		phsIncr[0] = synthParams.frqRad * frequency;
		phsIncr[1] = synthParams.frqRad * (frequency-beta);
		phsIncr[2] = synthParams.frqRad * (frequency+((harm+1)*beta));
		phsIncr[3] = synthParams.frqRad * (frequency+(harm*beta));
		phsIncr[4] = synthParams.frqRad * beta;

		PhsAccum phsN[5];
		phsN[0] = 0.0;
		phsN[1] = 0.0;
		phsN[2] = 0.0;
		phsN[3] = 0.0;
		phsN[4] = 0.0;

		AmpValue2 ampTwo = ampRatio * 2;
		AmpValue2 ampSqrP1 = 1.0 + (ampRatio * ampRatio);
		AmpValue2 ampPowN = pow(ampRatio, harm+1);
		ampScl = (1.0 - pow(ampRatio, 2.0)) / (1.0 - pow(ampRatio, (2*harm)+2));

		eg.Reset();
		for (n = 0; n < totalSamples; n++) 
		{
			AmpValue2 den = ampSqrP1 - (ampTwo * cos(phsN[4]));
			if (den != 0.0)
			{
				AmpValue2 num = sin(phsN[0]) 
				              - (ampRatio * sin(phsN[1]))
				              - (ampPowN * (sin(phsN[2]) - (ampRatio * sin(phsN[3]))));
				value = (AmpValue) (ampScl * num / den);
			}
			else
				value = 1.0;
			
			wf.Output1(eg.Gen() * value);
			for (int i = 0; i < 5; i++)
			{
				phsN[i] += phsIncr[i];
				if (phsN[i] >= twoPI)
					phsN[i] -= twoPI;
			}
		}
	}

	/////////////////////////////////////////////////
	// 13 - Waveshaping
	/////////////////////////////////////////////////
	PhsAccum wsPhase = 0;
	PhsAccum wsPhaseIncr = synthParams.frqRad * frequency;

	AmpValue posClip;
	AmpValue negClip;
	AmpValue wsScale;

	for (posClip = 0.3; posClip < 1.0; posClip += 0.2)
	{
		negClip = -posClip;
		wsScale = 1.0 / posClip;
		eg.Reset();
		for (n = 0; n < totalSamples; n++) 
		{
			value = sinv(wsPhase);
			if (value >= posClip)
				value = 1.0;
			else if (value <= negClip)
				value = -1.0;
			else
				value *= wsScale;
			if ((wsPhase += wsPhaseIncr) >= twoPI)
				wsPhase -= twoPI;
			wf.Output1(eg.Gen() * value);
		}
	}
	wf.CloseWaveFile();

	return 0;
}
