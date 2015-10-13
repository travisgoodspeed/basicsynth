///////////////////////////////////////////////////////////
// BasicSynth - Example 2a (Chapter 6)
//
// Amplitude Envelope using Bresenham midpoint algorithm
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

long dx2, dy2, linErr, linY, linMax, linDir;
int slopeFlag;

#ifdef USE_SWITCH_DDA
/* The obvious implementation... */
long CalcLine()
{
	long mult;
	switch (slopeFlag)
	{
	case 0:
		linY = linMax;
		break;
	case 1:
		linY++;
		break;
	case 2:
		if (linErr >= 0)
		{
			linErr -= dx2;
			linY++;
		}
		linErr += dy2;
		break;
	case 3:
	  /*while (linErr <= 0)
		{
			linY++;
			linErr += dx2;
		}*/
		// ASSERT: linErr is always negative on entry
		mult = (-linErr / dx2) + 1;
		linY += mult;
		linErr += (dx2 * mult);
		linErr -= dy2;
		break;
	}
	if (linDir < 0)
		return linMax - linY;
	return linY;
}
#else
/*************************************************
 * For ultimate speed freaks:
 * Set CalcLine to one of the following in InitLine()
 *************************************************/

long CalcLine0()
{
	return (linDir > 0) ? linMax : 0;
}

long CalcLine1()
{
	linY++;
	return (linDir > 0) ? linY : linMax - linY;
}

long CalcLine2()
{
	if (linErr >= 0)
	{
		linErr -= dx2;
		linY++;
	}
	linErr += dy2;
	return (linDir > 0) ? linY : linMax - linY;
}

long CalcLine3()
{
	long mult = (-linErr / dx2) + 1;
	linY += mult;
	linErr += (dx2 * mult);
	linErr -= dy2;
	return (linDir > 0) ? linY : linMax - linY;
}

long (*CalcLine)();

long (*LineFuncs[4])() = { CalcLine0, CalcLine1, CalcLine2, CalcLine3 };

#endif

void InitLine(long dx, long dy, int dir)
{
	linDir = dir;
	linMax = dy;
	linY = 0;

	if (dx == 0 || dy == 0)
	{
		slopeFlag = 0;
		dx2 = dy2 = linErr = 0;
	}
	else if (dx == dy)
	{
		slopeFlag = 1;
		dx2 = dy2 = linErr = 0;
	}
	else 
	{
		dx2 = dx * 2;
		dy2 = dy * 2;
		if (dx2 > dy2)
		{
			slopeFlag = 2;
			linErr = dy2 - dx;
		}
		else
		{
			slopeFlag = 3;
			linErr = dx2 - dy;
		}
	}
#ifndef USE_SWITCH_DDA
	CalcLine = LineFuncs[slopeFlag];
#endif
}

int main(int argc, char *argv[])
{
	InitSynthesizer();
	int pitch = 48; // Middle C
	FrqValue duration = 1;
	long volume = (long) synthParams.sampleScale;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		volume = atol(argv[3]);

	FrqValue frequency = synthParams.GetFrequency(pitch);
	PhsAccum phaseIncr = synthParams.frqRad * frequency;
	PhsAccum phase = 0;

	long totalSamples = (long) ((synthParams.sampleRate * duration) + 0.5);
	long n;

	WaveFile wf;
	if (wf.OpenWaveFile("example02b.wav", 1))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	long attackTime = (long) (0.2 * synthParams.sampleRate);
	long decayTime  = (long) (0.3 * synthParams.sampleRate);
	long sustainTime = totalSamples - (attackTime + decayTime);
	long decayStart = totalSamples - decayTime;

	InitLine(attackTime, volume, 1);

	for (n = 0; n < totalSamples; n++)
	{
		if (n < attackTime || n > decayStart)
			volume = CalcLine();
		else if (n == decayStart)
			InitLine(decayTime, volume, -1);
		wf.OutS((SampleValue) ((AmpValue) volume * sinv(phase)));
		if ((phase += phaseIncr) >= twoPI)
			phase -= twoPI;
	}

	wf.CloseWaveFile();

	return 0;
}
