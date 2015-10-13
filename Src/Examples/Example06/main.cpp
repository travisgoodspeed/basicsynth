/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 6 (Chapter 9)
//
// Mixing and Panning
//
// use: Example06
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
#include "EnvGen.h"
#include "EnvGenSeg.h"
#include "GenWaveWT.h"
#include "Mixer.h"

int main(int argc, char *argv[])
{
	InitSynthesizer(44100, 16384, 1);
	double ampWv[] = { 1.0, 0.3, 0.1 };
	wtSet.SetWaveTable(WT_USR(0), 3, NULL, ampWv, NULL, 0);

	GenWaveI wv1;
	GenWaveI wv2;
	EnvGen eg1;
	EnvGen eg2;
	WaveFile wvf;
	Mixer theMix;

	if (wvf.OpenWaveFile("example06.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}
	theMix.SetChannels(2);
	theMix.MasterVolume(1.0, 1.0);
	theMix.ChannelOn(0, true);
	theMix.ChannelVolume(0, 0.8);

	AmpValue lftOut;
	AmpValue rgtOut;


	AmpValue volume;
	AmpValue value;

	FrqValue duration = 2.0;
	long totalSamples = (long) (duration * synthParams.sampleRate);
	long snd, n;
	int pm;

	eg1.InitEG(1.0, duration, 0.1, 0.1);
	wv1.InitWT(synthParams.GetFrequency(48), WT_USR(0));

	EnvSegLin pan;

	for (pm = panLin; pm <= panSqr; pm++)
	{
		eg1.Reset();
		pan.InitSeg(2.0, -1.0, 1.0);
		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelPan(0, pm, pan.Gen());
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	duration = 0.5;
	eg1.InitEG(1.0, duration, 0.1, 0.2);
	eg2.InitEG(1.0, duration, 0.2, 0.1);
	totalSamples = (long) (duration * synthParams.sampleRate);
	AmpValue panset[] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
	FrqValue frequency[5];
	long numSounds = 5;

	// make a little scale...
	frequency[0] = synthParams.GetFrequency(48);
	frequency[1] = synthParams.GetFrequency(50);
	frequency[2] = synthParams.GetFrequency(52);
	frequency[3] = synthParams.GetFrequency(53);
	frequency[4] = synthParams.GetFrequency(55);
	// linear
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_USR(0));
		eg1.Reset();

		theMix.ChannelPan(0, panLin, panset[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	// log
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_USR(0));
		eg1.Reset();

		theMix.ChannelPan(0, panTrig, panset[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	// square root
	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_USR(0));
		eg1.Reset();

		theMix.ChannelPan(0, panSqr, panset[snd]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	//////////////////////////////////////////
	// Cross-fade two sounds.
	//////////////////////////////////////////

	theMix.ChannelOn(1, true);
	theMix.ChannelVolume(1, 0.5);

	for (snd = 0; snd < numSounds; snd++)
	{
		wv1.InitWT(frequency[snd], WT_USR(0));
		wv2.InitWT(frequency[snd]/2, WT_USR(0));
		eg1.Reset();
		eg2.Reset();

		theMix.ChannelPan(0, panTrig, panset[snd]);
		theMix.ChannelPan(1, panTrig, panset[numSounds - snd - 1]);

		for (n = 0; n < totalSamples; n++)
		{
			volume = eg1.Gen();
			value = wv1.Gen();
			theMix.ChannelIn(0, value * volume);

			volume = eg2.Gen();
			value = wv2.Gen();
			theMix.ChannelIn(1, value * volume);
			theMix.Out(&lftOut, &rgtOut);
			wvf.Output2(lftOut, rgtOut);
		}
	}

	//////////////////////////////////////////
	// Dynamic panning using an oscillator
	//////////////////////////////////////////
	wv1.InitWT(frequency[0], WT_USR(0));
	wv2.InitWT(1.0, WT_SIN);
	eg1.InitEG(1.0, 2.0, 0.8, 0.8);
	totalSamples = synthParams.isampleRate*2;
	theMix.ChannelPan(0, panLin, 0);

	for (n = 0; n < totalSamples; n++)
	{
		theMix.ChannelPan(0, panTrig, wv2.Gen());
		theMix.ChannelIn(0, eg1.Gen() * wv1.Gen());
		theMix.Out(&lftOut, &rgtOut);
		wvf.Output2(lftOut, rgtOut);
	}

	wvf.CloseWaveFile();

	return 0;
}
