///////////////////////////////////////////////////////////
// BasicSynth - Example 2a (Chapter 6)
//
// Envelope Generators
// 1 - interpolate linear, convex, log, exponential
// 2 - state machine
// 3 - interpolate multiple segments ADSR
// 4 - multiple segments, state machine
//
// This example uses the BasicSynth library classes. For inline code example, see Example02
//
// use: Example02a [duration [pitch [volume]]]
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
#include "EnvGenSeg.h"

int main(int argc, char *argv[])
{
	int pitch = 48;  // Middle C
	FrqValue duration = 1.0;
	AmpValue peakAmp = 1.0;
	int i;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		peakAmp = atof(argv[3]);

	InitSynthesizer();
	FrqValue frequency = synthParams.GetFrequency(pitch);

	PhsAccum phaseIncr = synthParams.frqRad * frequency;
	PhsAccum phase = PI/2; //0.0;

	long silence = (long) (synthParams.sampleRate * 0.1);
	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);
	long n;

	WaveFile wf;
	if (wf.OpenWaveFile("example02a.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	/////////////////////////////////////////////////
	// AR linear
	/////////////////////////////////////////////////
	EnvGen eglin;
	eglin.InitEG(peakAmp, duration, 0.2, 0.4);
	while (!eglin.IsFinished())
	{
		wf.Output1(eglin.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	/////////////////////////////////////////////////
	// AR convex (x^2)
	/////////////////////////////////////////////////
	EnvGenSqr egsqr;
	egsqr.InitEG(peakAmp, duration, 0.2, 0.4);
	while (!egsqr.IsFinished())
	{
		wf.Output1(egsqr.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	/////////////////////////////////////////////////
	// AR - exponential
	/////////////////////////////////////////////////
	EnvGenExp egexp;
	egexp.InitEG(peakAmp, duration, 0.2, 0.4);
	while (!egexp.IsFinished())
	{
		wf.Output1(egexp.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	/////////////////////////////////////////////////
	// AR - log
	/////////////////////////////////////////////////
	EnvGenLog eglog;
	eglog.InitEG(peakAmp, duration, 0.2, 0.4);
	while (!eglog.IsFinished())
	{
		wf.Output1(eglog.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// AR w/sustain - lin, exp, log
	/////////////////////////////////////////////////
	totalSamples = (long) (synthParams.sampleRate * (duration - 0.4));
	EGSegType egTypes[] = { linSeg, sqrSeg, expSeg, logSeg };
	int numEgTypes = sizeof(egTypes) / sizeof(EGSegType);

	EnvGenAR egar;
	for (i = 0; i < numEgTypes; i++)
	{
		egar.InitAR(0.2, peakAmp, 0.4, 1, egTypes[i]);
		for (n = 0; n < totalSamples; n++)
		{
			wf.Output1(egar.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
		egar.Release();
		while (!egar.IsFinished())
		{
			wf.Output1(egar.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// ADSR - lin, sqr, exp, log
	/////////////////////////////////////////////////
	EnvGenADSR adsr;
	for (i = 0; i < numEgTypes; i++)
	{
		adsr.InitADSR(0, 0.1, 1.0, 0.1, 0.707, 0.4, 0.0, egTypes[i]);
		for (n = 0; n < totalSamples; n++)
		{
			wf.Output1(adsr.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
		adsr.Release();
		while (!adsr.IsFinished())
		{
			wf.Output1(adsr.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// A3SR - lin, exp, log
	/////////////////////////////////////////////////
	totalSamples = (long) (synthParams.sampleRate * (duration - 0.3));
	EnvGenSegSus a3sr;
	a3sr.SetSegs(4);
	a3sr.SetStart(0);
	a3sr.SetSusOn(1);
	for (i = 0; i < numEgTypes; i++)
	{
		a3sr.SetSegN(0, 0.2, 1.0, egTypes[i]);
		a3sr.SetSegN(1, 0.1, 0.5, egTypes[i]);
		a3sr.SetSegN(2, 0.2, 0.8, egTypes[i]);
		a3sr.SetSegN(3, 0.3, 0.0, egTypes[i]);
		a3sr.Reset();
		for (n = 0; n < totalSamples; n++)
		{
			wf.Output1(a3sr.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
		a3sr.Release();
		while (!a3sr.IsFinished())
		{
			wf.Output1(a3sr.Gen() * sinv(phase));
			if ((phase += phaseIncr) >= twoPI)
				phase -= twoPI;
		}
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// Multi-seg, no sustain
	/////////////////////////////////////////////////
	EnvGenSeg egseg;
	egseg.SetStart(0);
	egseg.SetSegs(5);
	egseg.SetSegN(0, 0.1, 0.5, expSeg);
	egseg.SetSegN(1, 0.2, 1.0, logSeg);
	egseg.SetSegN(2, 0.2, 0.7, linSeg);
	egseg.SetSegN(3, 0.2, 0.7, susSeg);
	egseg.SetSegN(4, 0.3, 0.0, expSeg);
	egseg.Reset();
	while (!egseg.IsFinished())
	{
		wf.Output1(egseg.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}
	
	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// Multi-atk, multi-dec, w/sustain 
	/////////////////////////////////////////////////
	EnvGenMulSus egsegs;
	egsegs.SetStart(0);
	egsegs.SetSegs(3, 2);
	egsegs.SetAtkN(0, 0.1, 1.0, expSeg);
	egsegs.SetAtkN(1, 0.1, 0.5, expSeg);
	egsegs.SetAtkN(2, 0.2, 0.7, logSeg);
	egsegs.SetDecN(0, 0.2, 0.5, logSeg);
	egsegs.SetDecN(1, 0.2, 0.0, expSeg);
	egsegs.Reset();
	totalSamples = (long) (synthParams.sampleRate * (duration - 0.4));
	for (n = 0; n < totalSamples; n++)
	{
		wf.Output1(egsegs.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}
	egsegs.Release();
	while (!egsegs.IsFinished())
	{
		wf.Output1(egsegs.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	for (n = 0; n < silence; n++)
		wf.Output1(0.0);

	/////////////////////////////////////////////////
	// EG Table
	/////////////////////////////////////////////////
	EnvGenTable egtbl;
	FrqValue tLen[5] = { 0.1, 0.2, 0.2, 0.2, 0.3 };
	AmpValue tAmp[5] = { 0.5, 1.0, 0.7, 0.7, 0.0 };
	EGSegType tTyp[5] = { expSeg, logSeg, linSeg, susSeg, expSeg };
	egtbl.InitSegs(5, 0.0, tLen, tAmp, tTyp);
	egtbl.Reset();
	while (!egtbl.IsFinished())
	{
		wf.Output1(egtbl.Gen() * sinv(phase));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	wf.CloseWaveFile();

	int oor = wf.GetOOR();
	if (oor)
		printf("%d Samples out of range...\n", oor);

	return 0;
}
