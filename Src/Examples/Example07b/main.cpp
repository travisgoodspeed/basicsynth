/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 7b (Chapter 13)
//
// Delay Lines III - Flanger/Chorus
//
// use: Example07b [duration [pitch]]
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "GenNoise.h"
#include "GenWaveWT.h"
#include "GenWaveX.h"
#include "EnvGen.h"
#include "Mixer.h"
#include "DelayLine.h"
#include "AllPass.h"
#include "Flanger.h"

WaveFile wvf;
Mixer theMix;

void Silence(float t)
{
	long n = (long) (t * synthParams.sampleRate);
	while (n-- > 0)
		wvf.Output1(0);
}

int main(int argc, char *argv[])
{
	InitSynthesizer();

	long n;
	int pitch = 48;
	FrqValue duration = 2.75;
	AmpValue value1, value2;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);

	FrqValue frequency = synthParams.GetFrequency(pitch);

	if (wvf.OpenWaveFile("example07b.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	GenWaveFM wv;
	wv.InitFM(frequency, 1, 2, WT_SIN);

	EnvGen eg;
	eg.InitEG(0.5f, duration, 0.5f, 0.5f);

	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);

	// reference sound. 
	for (n = 0; n < totalSamples; n++)
	{
		value2 = (eg.Gen() * wv.Gen());
		wvf.Output1(value2);
	}
	Silence(0.25);


	// Flanger #1 varies from 0 to 5ms
	Flanger flng1;
	flng1.InitFlanger(0.5, 0.5, 0, 0.0025, 0.005, 0.15);

	// Flanger #2 varies from 45 to 50ms
	Flanger flng2;
	flng2.InitFlanger(0.5, 0.5, 0, 0.0042, 0.005, 0.15);

	// Flanger #3 is set for a chorus effect
	Flanger flng3;
	flng3.InitFlanger(0.5, 0.5, 0.5, 0.100, 0.001, 0.8);

	for (float snd = 0.5; snd <= 1; snd += 0.5)
	{
		wv.InitFM(frequency*snd, 1, 2, WT_SIN);
		eg.Reset();
		flng1.Clear();
		for (n = 0; n < totalSamples; n++)
		{
			value1 = (eg.Gen() * wv.Gen());
			value2 = flng1.Sample(value1);
			wvf.Output2(value2, value2);
		}
		Silence(0.25);

		eg.Reset();
		flng2.Clear();
		for (n = 0; n < totalSamples; n++)
		{
			value1 = (eg.Gen() * wv.Gen());
			value2 = flng2.Sample(value1);
			wvf.Output2(value2, value2);
		}
		Silence(0.25);

		eg.Reset();
		flng3.Clear();
		for (n = 0; n < totalSamples; n++)
		{
			value1 = (eg.Gen() * wv.Gen());
			value2 = flng3.Sample(value1);
			wvf.Output2(value2, value2);
		}
		Silence(0.25);
	}

	wvf.CloseWaveFile();

	return 0;
}
