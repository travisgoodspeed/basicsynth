/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 4 (Chapter 8)
//
//
// Waveform generators with wave tables and other stuff...
// 1 - sine wave
// 2 - sine wave using interpolation
// 3 - summation of sine waves
// 4 - summation of sine waves using interpolation
// 5 - 32 bit fixed-point index,
// 6 - 64 bit fixed-point index,
// 7 - square wave (summation)
// 8 - triangle wave (summation)
// 9 - FM
// 10 - AM
// 11 - RM
// 12 - Dynamic summing: sawtooth
// 13 - Dynamic summing: doubling at the fifth
// 14 - Dynamic summing: chorus effect, two notes at slightly different frequency
// 15 - Sine wave Direct
// 16 - Sawtooth Direct
// 17 - Triangle Direct
// 18 - Square Direct
// 19 - Square Direct Integer
// 20 - White Noise
// 21 - "Pink" Noise 1
// 22 - "Pink" Noise 2
// 23 - BUZZ
// 24 - BUZZA
// 25 - GenWaveDS
// 26 - GenWaveDSB
//
// use: Example04 [duration [pitch [wtlength]]]
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
#include "GenNoise.h"
#include "GenWaveWT.h"
#include "GenWaveX.h"
#include "GenWaveDSF.h"
#include "GenWave64.h"

WaveFile wvf;


AmpValue Generate(float duration, GenUnit *wv, EnvGen *eg, AmpValue in = 1.0)
{
	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	AmpValue volume;
	AmpValue value;
	AmpValue peak = 0.0;

	eg->Reset();

	for (long n = 0; n < totalSamples; n++)
	{
		volume = eg->Gen();
		value = wv->Sample(in);
		if (value > peak)
			peak = value;
		wvf.Output1(value * volume);
	}
	return peak;
}

void GenerateVib(FrqValue duration, GenWave *wv, EnvGen *eg, AmpValue lfoAmp)
{
	GenWave32 lfo;
	lfo.InitWT(3.5, WT_SIN);

	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	AmpValue volume;
	AmpValue value;

	eg->Reset();

	for (long n = 0; n < totalSamples; n++)
	{
		volume = eg->Gen();
		wv->Modulate(lfoAmp * lfo.Gen());
		value = wv->Sample(1.0);
		wvf.Output1(value * volume);
	}
}

void GeneratePhaseVib(FrqValue duration, GenWave *wv, EnvGen *eg, AmpValue lfoAmp)
{
	GenWave32 lfo;
	lfo.InitWT(3.5, WT_SIN);
	lfoAmp = (AmpValue) synthParams.frqRad * lfoAmp;

	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	AmpValue volume;
	AmpValue value;

	eg->Reset();

	for (long n = 0; n < totalSamples; n++)
	{
		volume = eg->Gen();
		wv->PhaseMod(lfoAmp * lfo.Gen());
		value = wv->Sample(1.0);
		wvf.Output1(value * volume);
	}
}

void Silence(FrqValue duration)
{
	long totalSamples = (long) ((duration * synthParams.sampleRate) + 0.5);
	for (long n = 0; n < totalSamples; n++)
		wvf.Output1(0);
}

int main(int argc, char *argv[])
{
	int pitch = 48;  // middle C
	FrqValue duration = 1.0;
	AmpValue lfoAmp = 10;
	bsInt32 tl = 16384;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		tl = atol(argv[3]);

	InitSynthesizer(44100, tl);

	FrqValue frequency = synthParams.GetFrequency(pitch);
	
	if (wvf.OpenWaveFile("example04.wav", 1) < 0)
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	//EnvGen eg;
	EnvGenExp eg;
	//EnvGenLog eg;
	eg.SetBias(0.2);
	eg.InitEG(0.7, duration, 0.2, 0.3);

	GenWaveWT wv;
	GenWaveI  wvi;
	GenWave32 wv32;
	//GenWave64 wv64;
	GenWaveFM wvfm;
	GenWaveAM wvam;
	GenWaveRM wvrm;

	/////////////////////////////////////////////////
	// 1 - sine wave
	/////////////////////////////////////////////////
	wv.InitWT(frequency, WT_SIN);
	Generate(duration, &wv, &eg);
	//GenerateVib(duration, &wv, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wv, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 2 - sine wave using interpolation
	/////////////////////////////////////////////////
	wvi.InitWT(frequency, WT_SIN);
	Generate(duration, &wvi, &eg);
	//GenerateVib(duration, &wvi, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvi, &eg, lfoAmp);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 3 - fixed summation of sine waves
	/////////////////////////////////////////////////
	wv.InitWT(frequency, WT_SAW);
	Generate(duration, &wv, &eg);
	//GenerateVib(duration, &wv, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wv, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 4 - fixed summation of sine waves using interpolation
	/////////////////////////////////////////////////
	wvi.InitWT(frequency, WT_SAW);
	Generate(duration, &wvi, &eg);
	//GenerateVib(duration, &wvi, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvi, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 5 - 32 bit fixed-point index,
	/////////////////////////////////////////////////
	wv32.InitWT(frequency, WT_SAW);
	Generate(duration, &wv32, &eg);
	//GenerateVib(duration, &wv32, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wv32, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 6 - 64 bit fixed-point index,
	/////////////////////////////////////////////////
	//wv64.InitWT(frequency, WT_SAW);
	//Generate(duration, &wv64, &eg);
	//GenerateVib(duration, &wv64, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wv64, &eg, lfoAmp);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 7 - square wave (summation)
	/////////////////////////////////////////////////
	wv.InitWT(frequency, WT_SQR);
	Generate(duration, &wv, &eg);
	//GenerateVib(duration, &wv, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wv, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 8 - triangle wave (summation)
	/////////////////////////////////////////////////
	wv.InitWT(frequency, WT_TRI);
	Generate(duration, &wv, &eg);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 9 - FM
	/////////////////////////////////////////////////
	wvfm.InitFM(frequency/2, 2.0, 1.0, WT_SIN);
	Generate(duration, &wvfm, &eg);
	wvfm.InitFM(frequency, 2.0, 1.0, WT_SIN);
	Generate(duration, &wvfm, &eg);
	//GenerateVib(duration, &wvfm, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvfm, &eg, lfoAmp);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 10 - Amplitude Modulation
	/////////////////////////////////////////////////
	wvam.InitAM(frequency, frequency*2.5, 1.0, WT_SIN);
	Generate(duration, &wvam, &eg);

	/////////////////////////////////////////////////
	// 11 - Ring Modulation
	/////////////////////////////////////////////////
	wvrm.InitAM(frequency, frequency*2.5, 1.0, WT_SIN);
	Generate(duration, &wvrm, &eg);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 12 - Dynamic summing: sawtooth
	/////////////////////////////////////////////////
	GenWaveSum wvs;
	AmpValue mult[8];
	AmpValue amps[8];
	for (int pnum = 0; pnum < 8; pnum++)
	{
		AmpValue x = (AmpValue)pnum + 1;
		mult[pnum] = x;
		amps[pnum] = 1.0/x;
		if (pnum & 1)
			amps[pnum] = -amps[pnum];
	}
	wvs.InitParts(8, mult, amps, 1);
	wvs.InitWT(frequency, WT_SIN);
	Generate(duration, &wvs, &eg);
	//GenerateVib(duration, &wvs, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvs, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 13 - Dynamic summing: doubling at the fifth
	/////////////////////////////////////////////////
	mult[0] = 1.0f;
	mult[1] = 1.5f;
	amps[0] = 0.5f;
	amps[1] = 0.5f;
	wvs.InitParts(2, mult, amps, 0);
	wvs.InitWT(frequency, WT_SAW);
	Generate(duration, &wvs, &eg);
	//GenerateVib(duration, &wvs, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 14 - Dynamic summing: chorus effect, two notes at slightly different frequency
	/////////////////////////////////////////////////
	mult[0] = 1.0;
	mult[1] = 1.001;
	amps[0] = 0.6;
	amps[1] = 0.4;
	wvs.InitParts(2, mult, amps, 0);
	wvs.InitWT(frequency, WT_SAW);
	Generate(duration, &wvs, &eg);
	//GenerateVib(duration, &wvs, &eg, lfoAmp);

	Silence(0.25);

	// For comparison:

	/////////////////////////////////////////////////
	// 15 - Sine wave Direct
	/////////////////////////////////////////////////
	GenWave wvsin;
	wvsin.Init(1, &frequency);
	Generate(duration, &wvsin, &eg);
	//GenerateVib(duration, &wvsin, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 16 - Sawtooth Direct
	/////////////////////////////////////////////////
	GenWaveSaw wvsaw;
	wvsaw.Init(1, &frequency);
	Generate(duration, &wvsaw, &eg);
	//GenerateVib(duration, &wvsaw, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvsaw, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 17 - Triangle Direct
	/////////////////////////////////////////////////
	GenWaveTri wvtri;
	wvtri.Init(1, &frequency);
	Generate(duration, &wvtri, &eg);
	//GenerateVib(duration, &wvtri, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvtri, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 18 - Square Direct
	/////////////////////////////////////////////////
	GenWaveSqr wvsqr;
	wvsqr.InitSqr(frequency, 50.0);
	Generate(duration, &wvsqr, &eg);
	//GenerateVib(duration, &wvsqr, &eg, lfoAmp);
	//GeneratePhaseVib(duration, &wvsqr, &eg, lfoAmp);

	/////////////////////////////////////////////////
	// 19 - Square Direct Integer
	/////////////////////////////////////////////////
	GenWaveSqr32 wvsqr32;
	wvsqr32.InitSqr(frequency, 50.0);
	Generate(duration, &wvsqr32, &eg);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 20 - White Noise
	/////////////////////////////////////////////////
	GenNoise wvn;
	Generate(duration, &wvn, &eg);

	/////////////////////////////////////////////////
	// 21 - 'H' White Noise (sampled)
	/////////////////////////////////////////////////
	GenNoiseH wvnh;
	float h;
	for (h = 2000.0; h < 10000.0; h += 2000.0)
	{
		wvnh.Init(1, &h);
		Generate(duration, &wvnh, &eg);
	}

	/////////////////////////////////////////////////
	// 21 - 'Interploated' White Noise
	/////////////////////////////////////////////////
	GenNoiseI wvni;
	for (h = 2000.0; h < 10000.0; h += 2000.0)
	{
		wvni.Init(1, &h);
		Generate(duration, &wvni, &eg);
	}

	Silence(0.25);


	/////////////////////////////////////////////////
	// 22 - "Pink-ish" Noise (FIR filter)
	/////////////////////////////////////////////////
	GenNoisePink1 wvp1;
	Generate(duration, &wvp1, &eg);

	/////////////////////////////////////////////////
	// 23 - "Pink-ish" Noise (IIR filter)
	/////////////////////////////////////////////////
	GenNoisePink2 wvp2;
	Generate(duration, &wvp2, &eg);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 23 - Pitched Noise (ring modulation of sine wave and noise)
	/////////////////////////////////////////////////
	GenWaveNZ wvnzp;
	wvnzp.InitNZ(500.0, 400.0, WT_SIN);
	Generate(duration, &wvnzp, &eg);
	wvnzp.InitNZ(800.0, 400.0, WT_SIN);
	Generate(duration, &wvnzp, &eg);

	Silence(0.25);

	/////////////////////////////////////////////////
	// 24 - BUZZ generator
	/////////////////////////////////////////////////
	GenWaveBuzz buzz;
	buzz.InitBuzz(frequency, 20);
	Generate(duration, &buzz, &eg);
	buzz.InitBuzz(frequency, 100);
	Generate(duration, &buzz, &eg);

//	buzz.InitBuzz(frequency, 100);
//	GenerateVib(duration, &buzz, &eg, lfoAmp);
//	GeneratePhaseVib(duration, &buzz, &eg, lfoAmp);

	Silence(0.25);

	GenWaveBuzz2 buzz2;
	buzz2.InitBuzz(frequency, 20);
	Generate(duration, &buzz2, &eg);
	buzz2.InitBuzz(frequency, 100);
	Generate(duration, &buzz2, &eg);

	Silence(0.25);

	GenWaveBuzzA buzza;
//	buzza.SetOscillatorA(new GenWaveI);
//	buzza.SetOscillatorB(new GenWaveI);
	buzza.InitBuzz(frequency, 20);
	Generate(duration, &buzza, &eg);
	buzza.InitBuzz(frequency, 100);
	Generate(duration, &buzza, &eg);

	Silence(0.25);
	
	GenWaveDS ds;
	ds.InitDS(frequency, 0.5);
	Generate(duration, &ds, &eg);
	ds.InitDS(frequency, 0.95);
	Generate(duration, &ds, &eg);

	Silence(0.25);

	GenWaveDSB gbz;
	gbz.InitDSB(frequency, 1, 100, 0.5);
	Generate(duration, &gbz, &eg);
	gbz.InitDSB(frequency, 1, 100, 0.95);
	Generate(duration, &gbz, &eg);

	// Odd harmonics, like square wave
	gbz.InitDSB(frequency, 2, 100, 0.5);
	Generate(duration, &gbz, &eg);
	gbz.InitDSB(frequency, 2, 100, 0.95);
	Generate(duration, &gbz, &eg);

	/////////////////////////////////////////////////
	if (wvf.CloseWaveFile())
		perror("Close");

	return 0;
}
