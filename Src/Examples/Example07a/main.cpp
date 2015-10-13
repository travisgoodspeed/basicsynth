/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 7a (Chapter 12)
//
// Delay Lines II - Reverberation
//
//  1 - allpass delay line
//	2 - low-pass filtered resonator
//  3 - Schroeder style reverb
//
// use: Example07a [duration [pitch]]
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "GenWaveWT.h"
#include "EnvGen.h"
#include "DelayLine.h"
#include "Reverb.h"

int main(int argc, char *argv[])
{
	InitSynthesizer();

	long n; // sample counter

	int pitch = 48;
	FrqValue duration = 0.5;
	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);

	FrqValue middleC = synthParams.GetFrequency(pitch);
	AmpValue v1, v2;

	GenWaveI wv;
	wv.InitWT(middleC, WT_SAW);

	EnvGen eg;
	eg.InitEG(0.5f, duration, 0.05f, 0.2f);

	WaveFile wvf;
	if (wvf.OpenWaveFile("example07a.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5f);
	long delaySamples = synthParams.isampleRate / 2;

	// Dry reference signal:
	for (n = 0; n < totalSamples; n++)
	{
		wvf.Output1(eg.Gen() * wv.Gen());
	}

	// put a little silence in here...
	for (n = 0; n < delaySamples; n++)
		wvf.Output1(0);


	FrqValue rt;
	AmpValue atten = 1.0;
	AmpValue mix = 0.5;
	FrqValue rvrbTime = 1.5f;
	FrqValue irTime = 0.05f;
	delaySamples = (long) ((rvrbTime - duration) * synthParams.sampleRate);

	/////////////////////////////////////////////
	// Allpass delay
	/////////////////////////////////////////////
	AllPassDelay apd;

	for (rt = 0.8f; rt <= 1.8; rt += 0.2f)
	{
		apd.InitDLR(irTime, rt, 0.001);
		eg.Reset();
		for (n = 0; n < totalSamples; n++)
		{
			v1 = eg.Gen() * wv.Gen();
			v2 = apd.Sample(v1 * atten);
			wvf.Output1((v1 * (1.0 - mix)) + (v2 * mix));
		}
		// wait for the reverb to decay

		for (n = 0; n < delaySamples; n++)
			wvf.Output1(apd.Sample(0.0)*mix);
	}

	/////////////////////////////////////////////
	// Reverb with LP filtered feedback comb filter
	/////////////////////////////////////////////
	Reverb1 rvb1;
	for (rt = 0.8f; rt <= 1.8; rt += 0.2f)
	{
		rvb1.InitReverb(atten, irTime, rt);
		eg.Reset();
		for (n = 0; n < totalSamples; n++)
		{
			v1 = eg.Gen() * wv.Gen();
			v2 = rvb1.Sample(v1);
			wvf.Output1((v1 * (1.0 - mix)) + (v2 * mix));
		}
		// wait for the reverb to decay

		for (n = 0; n < delaySamples; n++)
			wvf.Output1(rvb1.Sample(0.0)*mix);
	}

	/////////////////////////////////////////////
	// Reverb with multiple resonator delay lines
	// (Scroeder reverb)
	/////////////////////////////////////////////
	Reverb2 rvb2;
	for (rt = 0.8f; rt <= 1.8; rt += 0.2f)
	{
		rvb2.InitReverb(atten, rt);
		eg.Reset();
		for (n = 0; n < totalSamples; n++)
		{
			v1 = eg.Gen() * wv.Gen();
			v2 = rvb2.Sample(v1);
			wvf.Output1((v1 * (1.0 - mix)) + (v2 * mix));
		}
		// wait for the reverb to decay
		for (n = 0; n < delaySamples; n++)
			wvf.Output1(rvb2.Sample(0.0)*mix);
	}

	wvf.CloseWaveFile();

	return 0;
}
