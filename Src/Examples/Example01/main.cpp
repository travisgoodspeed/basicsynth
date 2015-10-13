///////////////////////////////////////////////////////////
// BasicSynth - Example 1 (Chapter 5)
//
// Generate a sound by calling the sin() function.
//
// use: example01 [duration [pitch [volume]]]
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

int main(int argc, char *argv[])
{
	int pitch = 48;  // Middle C
	FrqValue duration = 1;
	AmpValue volume = 1;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		volume = atof(argv[3]);

	InitSynthesizer();
	FrqValue frequency = synthParams.GetFrequency(pitch);

	PhsAccum phaseIncr = synthParams.frqRad * frequency;
	PhsAccum phase = 0;

	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);

	WaveFile wf;
	if (wf.OpenWaveFile("example01.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	for (long n = 0; n < totalSamples; n++)
	{
		wf.Output1(volume * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	wf.CloseWaveFile();

	return 0;
}
