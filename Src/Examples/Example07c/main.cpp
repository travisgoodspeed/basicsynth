///////////////////////////////////////////////////////////
// BasicSynth - Example 7c
//
// Karplus-Strong
//
// Several varations are possible:
// 1 - use a NoiseH or NoiseI generator rathern than white noise
// 2 - change the coefficients of the LP filter or use the IIR form
// 3 - change the decay parameter to the delay line
//
// Daniel R. Mitchell
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "WaveFile.h"
#include "WaveTable.h"
#include "EnvGen.h"
#include "DelayLine.h"
#include "Filter.h"
#include "AllPass.h"
#include "GenNoise.h"

WaveFile wvf;

int main(int argc, char *argv[])
{
	InitSynthesizer();
	wvf.OpenWaveFile("Example07c.wav", 1);

	long n;
	FrqValue duration = 1.0;
	AmpValue value;
	FrqValue dlTime;
	FrqValue apTime;
	FrqValue freq;
	long noise;

	EnvGen eg;
	DelayLine dl;
	FilterFIR lp;
	//FilterIIR lp;
	AllPassFilter ap;
	GenNoise nz;
	//GenNoiseI nz;
	//nz.InitH(2000.0);

	lp.InitFilter(0.5, 0.5);
	//lp.InitFilter(0.5, 0.45);
	eg.InitEG(1.0, duration, 0.0, 0.01);

	int pitch[] = { 24, 28, 31, 36, 40, 43, 48, 52, 55, 60, 64, 67, 72 };
	int npitch = sizeof(pitch) / sizeof(int);

	long totalSamples = (long) (duration * synthParams.sampleRate);

	for (int i = 0; i < npitch; i++)
	{
		freq = synthParams.GetFrequency(pitch[i]);
		// Note: the dlTime is slightly inaccurate since delay length
		// should be (Fs/Fo)+0.5. The delay line class expects
		// time and will multiply by SR producing a length of (Fs/Fo)
		dlTime = 1.0 / freq;
		// The length of the sound can be altered by changing the decay
		// parameter to InitDL. Typically, this would be done to vary
		// the decay based on frequency.
		dl.InitDL(dlTime, 1.0);
		//dl.InitDL(dlTime, 0.98);
		apTime = synthParams.sampleRate / freq;
		ap.InitAP(apTime - floor(apTime));
		noise = (long) (dlTime * synthParams.sampleRate);
		for (n = 0; n < noise; n++)
			dl.SetIn(nz.Gen());
		value = 0;
		for (n = 0; n < totalSamples; n++)
		{
			value = ap.Sample(lp.Sample(dl.Sample(value)));
			wvf.Output1(value * eg.Gen());
		}
		eg.Reset();
	}

	wvf.CloseWaveFile();

	return 0;
}
