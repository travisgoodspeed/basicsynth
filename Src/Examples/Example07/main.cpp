/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 7 (Chapter 11)
//
// Delay Lines part 1
//  1 - echo .3s later
//  2 - two taps to produce three echos, .3s apart
//
// use: Example07 [duration [pitch]]
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "GenWaveWT.h"
#include "EnvGen.h"
#include "Mixer.h"
#include "DelayLine.h"

int main(int argc, char *argv[])
{
	InitSynthesizer();

	long numSounds = 3;
	long n;

	int pitch = 48;
	FrqValue duration = 0.5;
	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);

	AmpValue value;
	AmpValue lft, rgt;

	WaveFile wvf;
	Mixer theMix;

	//GenWave32 wv;
	GenWaveI wv;
	EnvGen eg;
	DelayLineT dl;

	if (wvf.OpenWaveFile("example07.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}
	theMix.MasterVolume(1.0f, 1.0f);
	theMix.SetChannels(4);
	theMix.ChannelOn(0, true);
	theMix.ChannelOn(1, true);
	theMix.ChannelOn(2, true);
	theMix.ChannelOn(3, true);
	theMix.ChannelVolume(0, 0.7f);
	theMix.ChannelVolume(1, 0.5f);
	theMix.ChannelPan(0, panTrig, 0.8f);
	theMix.ChannelPan(1, panTrig, -0.8f);

	float dlTime = 0.3f;
	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	long delaySamples = (long) ((dlTime * synthParams.sampleRate) + 0.5);

	dl.InitDLT(dlTime, 0);

	wv.InitWT(synthParams.GetFrequency(pitch), WT_SAW);
	eg.InitEG(1.0f, duration, 0.1f, 0.2f);

	for (long snd = 0; snd < numSounds; snd++)
	{
		for (n = 0; n < totalSamples; n++)
		{
			value = eg.Gen() * wv.Gen();
			theMix.ChannelIn(0, value);

			value = dl.Sample(value);
			theMix.ChannelIn(1, value);
			theMix.Out(&lft, &rgt);
			wvf.Output2(lft, rgt);
		}
		pitch += 2;
		wv.SetFrequency(synthParams.GetFrequency(pitch));
		wv.Reset();
		eg.Reset();
	}

	for (n = 0; n < delaySamples; n++)
	{
		value = dl.Sample(0.0);
		theMix.ChannelIn(1, value);
		theMix.Out(&lft, &rgt);
		wvf.Output2(lft, rgt);
	}

	duration = 0.25;
	dlTime = 1.0;
	dl.InitDLT(dlTime, 2);
	dl.SetTap(0, 0.25f);
	dl.SetTap(1, 0.6f);
	wv.InitWT(synthParams.GetFrequency(pitch), WT_SAW);
	eg.InitEG(1.0f, duration, 0.05f, 0.05f);

	theMix.ChannelVolume(0, 0.5f);
	theMix.ChannelVolume(1, 0.4f);
	theMix.ChannelVolume(2, 0.3f);
	theMix.ChannelVolume(3, 0.2f);
	theMix.ChannelPan(0, panTrig, 0.0f);
	theMix.ChannelPan(1, panTrig, -0.8f);
	theMix.ChannelPan(2, panTrig, 0.0f);
	theMix.ChannelPan(3, panTrig,  0.8f);

	totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	for (n = 0; n < totalSamples; n++)
	{
		value = eg.Gen() * wv.Gen();
		theMix.ChannelIn(0, value);
		theMix.ChannelIn(3, dl.Sample(value));
		theMix.ChannelIn(1, dl.Tap(0));
		theMix.ChannelIn(2, dl.Tap(1));
		theMix.Out(&lft, &rgt);
		wvf.Output2(lft, rgt);
	}

	delaySamples = (long) (dlTime * synthParams.sampleRate);
	for (n = 0; n < delaySamples; n++)
	{
		theMix.ChannelIn(3, dl.Sample(0.0f));
		theMix.ChannelIn(1, dl.Tap(0));
		theMix.ChannelIn(2, dl.Tap(1));
		theMix.Out(&lft, &rgt);
		wvf.Output2(lft, rgt);
	}

	// frequency shift using varying delay line tap
	duration = 4.0;
	DelayLineV dlv;
	// the delay time must be greater than the max value set with SetOffs below
	dlv.InitDL(0.15, 0.9);
	GenWave32 wvoffs;
	wvoffs.InitWT(4/duration, WT_SIN);
	eg.InitEG(0.7, duration, 0.5, 0.5);
	delaySamples = (long) (0.5 * synthParams.sampleRate);
	for (n = 0; n < delaySamples; n++)
		dlv.SetIn(wv.Gen());

	theMix.ChannelOn(1, false);
	theMix.ChannelOn(2, false);
	theMix.ChannelOn(3, false);
	theMix.ChannelVolume(0, 1);
	theMix.ChannelPan(0, panTrig, 0);
	totalSamples = (long) ((duration * synthParams.sampleRate));
	for (n = 0; n < totalSamples; n++)
	{
		AmpValue v2 = wvoffs.Gen();
		theMix.ChannelPan(0, panTrig, v2);
		dlv.SetDelayT(0.05*(v2 + 1));
		value = wv.Gen();
		value = dlv.Sample(value);
		value = eg.Gen() * value;
		theMix.ChannelIn(0, value);
		theMix.Out(&lft, &rgt);
		wvf.Output2(lft, rgt);
	}

	for (n = 0; n < delaySamples; n++)
		wvf.Output1(0);

	wvf.CloseWaveFile();

	return 0;
}
