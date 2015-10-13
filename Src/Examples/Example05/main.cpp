/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 5 (Chapter 10)
//
// Various Filters
//
// Bi-Quad Filters (LP,HP,BP) and allpass applied to noise and a sawtooth wave.
// FIR and IIR averaging filters.
// FIR Convolution
//
// use: Example05 [duration [pitch [cutoff]]]
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
#include "GenNoise.h"
#include "BiQuad.h"
#include "AllPass.h"
#include "Filter.h"
#include "DynFilter.h"

WaveFile wvf;

void Generate(FrqValue duration, GenUnit *wv, EnvGen *eg, GenUnit *fp, AmpValue vol = 1.0)
{
	long totalSamples = (long) (duration * synthParams.sampleRate);
	AmpValue sig;

	eg->Reset();

	for (long n = 0; n < totalSamples; n++)
	{
		sig = eg->Gen() * wv->Sample(1.0);
		if (fp)
			sig = fp->Sample(sig);
		wvf.Output1(sig * vol);
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
	int pitch = 48;
	FrqValue duration = 1.0;
	FrqValue cutoff = 1000.0;

	if (argc > 1)
		duration = atof(argv[1]);
	if (argc > 2)
		pitch = atoi(argv[2]);
	if (argc > 3)
		cutoff = atof(argv[3]);

	InitSynthesizer();

	FrqValue frequency = synthParams.GetFrequency(pitch);

	if (wvf.OpenWaveFile("example05.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	EnvGen eg;
	eg.InitEG(0.707, duration, 0.1, 0.2);

	FrqValue q = 1;
	FrqValue cu;
	long nf;
	int fn;

	long fstep = 50;
	FrqValue custep = 5000.0 / (float) fstep;
	long sn = (long) (5.0 * synthParams.sampleRate) / fstep;

	GenNoise nz;
	FilterLP2 lpf;
	FilterHP2 hpf;
	FilterBP2 bpf;
	AllPassFilter apf;
	Reson resf;
	FilterAvgN avgn;
	DynFilterLP dynfilt;
	FilterFIRn firn;
	FilterIIR2 iir2;
	FilterIIR2p iir2p;
	FilterSV fsv;
	GenWaveI wv;
	wv.InitWT(frequency, WT_SAW); //WT_PLS);

	/////////////////////////////////////////////////
	// 1 - reference white noise
	/////////////////////////////////////////////////
	lpf.Init(0.0, 1.0);
	Generate(duration, &nz, &eg, NULL);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 2 - low pass filter
	/////////////////////////////////////////////////
	//lpf.Init(cutoff, 3.0);
	lpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &nz, &eg, &lpf);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 3 - high pass filter
	/////////////////////////////////////////////////
	hpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &nz, &eg, &hpf);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 4 - band pass filter
	/////////////////////////////////////////////////
	bpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &nz, &eg, &bpf);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 5 - allpass filter
	/////////////////////////////////////////////////
	apf.InitAP(0.5);
	Generate(duration, &nz, &eg, &apf);

	Silence(0.5);

	cutoff = 3*frequency;
	/////////////////////////////////////////////////
	// 6 - reference wave
	/////////////////////////////////////////////////
	Generate(duration, &wv, &eg, NULL);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 7 - low pass filter
	/////////////////////////////////////////////////
	lpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &wv, &eg, &lpf);
	lpf.Init(cutoff, 12.0, 0.7);
	Generate(duration, &wv, &eg, &lpf);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 8 - high pass filter
	/////////////////////////////////////////////////
	hpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &wv, &eg, &hpf);
	Silence(0.1);

	/////////////////////////////////////////////////
	// 9-10 - band pass filters
	/////////////////////////////////////////////////
	bpf.Init(cutoff, 0.0, 1.0);
	Generate(duration, &wv, &eg, &bpf);
	bpf.Init(cutoff, 12.0, 1.0);
	Generate(duration, &wv, &eg, &bpf);

	Silence(0.5);

	/////////////////////////////////////////////////
	// 11-14 - reson filters
	/////////////////////////////////////////////////
	resf.SetGain(1.0);
	resf.SetFrequency(cutoff);
	resf.SetRes(0.8);
	resf.CalcCoef();
	Generate(duration, &wv, &eg, &resf);
	resf.SetRes(0.9);
	resf.CalcCoef();
	resf.Reset(0);
	Generate(duration, &wv, &eg, &resf);
	resf.SetRes(0.99);
	resf.CalcCoef();
	resf.Reset(0);
	Generate(duration, &wv, &eg, &resf);
	resf.SetRes(0.999);
	resf.CalcCoef();
	resf.Reset(0);
	Generate(duration, &wv, &eg, &resf);

	Silence(0.5);

	/////////////////////////////////////////////////
	// 15 - Allpass filter
	/////////////////////////////////////////////////
	apf.InitAP(0.5);
	Generate(duration, &wv, &eg, &apf);

	/////////////////////////////////////////////////
	// 16 - Running average filter
	/////////////////////////////////////////////////
	avgn.InitFilter(4);
	Generate(duration, &wv, &eg, &avgn);

	Silence(0.5);

	/////////////////////////////////////////////////
	// 17 - Dynamic lowpass filter
	/////////////////////////////////////////////////
	dynfilt.InitFilter(100.0, 0.2, 4000.0, 0.4, 1000.0, 0.2, 100.0);
	Generate(duration, &wv, &eg, &dynfilt);

	Silence(0.5);

	/////////////////////////////////////////////////
	// 18 - Dynamic calculation of FIR LP filter
	/////////////////////////////////////////////////
	cu = 100.0;
	//firn.Init(129, NULL); // <== very sharp roll-off
	firn.Init(49, NULL);  // <== a good compromise
	//firn.Init(17, NULL);    // <== much faster to calculate
	eg.InitEG(1.0, 5.0, 0.1, 0.1);
	for (fn = 0; fn < fstep; fn++)
	{
		firn.CalcCoef(cu, 0);
		for (nf = 0; nf < sn; nf++)
			wvf.Output1(eg.Gen() * firn.Sample(nz.Gen()));
		cu += custep;
	}

	/////////////////////////////////////////////////
	// 19 - Dynamic calculation of FIR HP filter
	/////////////////////////////////////////////////
	cu = 100.0;
	//firn.Reset();
	eg.Reset();
	for (fn = 0; fn < fstep; fn++)
	{
		firn.CalcCoef(cu, 1);
		for (nf = 0; nf < sn; nf++)
			wvf.Output1(eg.Gen() * firn.Sample(nz.Gen()));
		cu += custep;
	}

	Silence(0.5);

	/////////////////////////////////////////////////
	// 20 - Dynamic calculation of IIR LP filter
	/////////////////////////////////////////////////
	cu = 100.0;
	eg.Reset();
	for (fn = 0; fn < fstep; fn++)
	{
		iir2.CalcCoef(cu, 0);
		for (nf = 0; nf < sn; nf++)
			wvf.Output1(eg.Gen() * iir2.Sample(nz.Gen()));
		cu += custep;
	}

	/////////////////////////////////////////////////
	// 21 - Dynamic calculation of IIR LP filter, vary Q
	/////////////////////////////////////////////////
	cu = 100.0;
	q = 1.0;
	eg.Reset();
	for (fn = 0; fn < fstep; fn++)
	{
		iir2p.CalcCoef(cu, q);
		for (nf = 0; nf < sn; nf++)
			wvf.Output1(eg.Gen() * iir2p.Sample(nz.Gen()));
		cu += custep;
		q += 0.1;
	}

	/////////////////////////////////////////////////
	// 22 - Dynamic calculation of IIR HP filter
	/////////////////////////////////////////////////
	cu = 100.0;
	eg.Reset();
	for (fn = 0; fn < fstep; fn++)
	{
		iir2.CalcCoef(cu, 1);
		for (nf = 0; nf < sn; nf++)
			wvf.Output1(eg.Gen() * iir2.Sample(nz.Gen()));
		cu += custep;
	}

	Silence(0.5);

	/////////////////////////////////////////////////
	// 23 - State variable filter
	/////////////////////////////////////////////////
	eg.InitEG(0.707, duration, 0.1, 0.2);
	fsv.InitFilter(250, 2, 0.3, 0.1, 0.5);
	Generate(duration, &nz, &eg, &fsv);
	fsv.InitFilter(250, 2, 0.1, 0.3, 0.5);
	Generate(duration, &nz, &eg, &fsv);

	fsv.InitFilter(500, 2, 0.3, 0.1, 0.5);
	Generate(duration, &nz, &eg, &fsv);
	fsv.InitFilter(500, 2, 0.1, 0.3, 0.5);
	Generate(duration, &nz, &eg, &fsv);

	fsv.InitFilter(500, 20, 0.2, 0.2, 0.3);
	Generate(duration, &wv, &eg, &fsv);

	fsv.InitFilter(500, 20, 0.3, 0.2, 0.2);
	Generate(duration, &wv, &eg, &fsv);

	Silence(0.5);
	fsv.InitFilter(2000, 10, 1.0, 0.0, 0.0);
	Generate(duration, &wv, &eg, &fsv);


/****************************************************
	Silence(0.5);
	/////////////////////////////////////////////////
	// 15 - Pre-calculated values for FIR lowpass filter
	// fc = 1000hz, (-60db at 10khz) 
	// generated by Digital Filter Analyzer (http://www.digitalfilter.com)
	/////////////////////////////////////////////////
	float h[32]; // FIR filter coeffcients 
	h[  0] =  0.027961015;
	h[  1] =  0.083693340;
	h[  2] =  0.155213352;
	h[  3] =  0.206230444;
	h[  4] =  0.206230444;
	h[  5] =  0.155213352;
	h[  6] =  0.083693340;
	h[  7] =  0.027961015;
	firn.Init(8, h);
	Generate(duration, &wv, &eg, &firn);

	h[  0] = -0.002747550;
	h[  1] = -0.008972243;
	h[  2] = -0.013909771;
	h[  3] = -0.004676988;
	h[  4] =  0.032612163;
	h[  5] =  0.099181436;
	h[  6] =  0.174930924;
	h[  7] =  0.225961565;
	h[  8] =  0.225961565;
	h[  9] =  0.174930924;
	h[ 10] =  0.099181436;
	h[ 11] =  0.032612163;
	h[ 12] = -0.004676988;
	h[ 13] = -0.013909771;
	h[ 14] = -0.008972243;
	h[ 15] = -0.002747550;
	firn.Init(16, h);
	Generate(duration, &wv, &eg, &firn);

	h[  0] =  0.000000406;
	h[  1] =  0.000039607;
	h[  2] =  0.000259015;
	h[  3] =  0.000893486;
	h[  4] =  0.001996369;
	h[  5] =  0.002887092;
	h[  6] =  0.001801127;
	h[  7] = -0.003386830;
	h[  8] = -0.012931337;
	h[  9] = -0.022509999;
	h[ 10] = -0.022554891;
	h[ 11] = -0.001852118;
	h[ 12] =  0.045174841;
	h[ 13] =  0.111774641;
	h[ 14] =  0.178305011;
	h[ 15] =  0.220106530;
	h[ 16] =  0.220106530;
	h[ 17] =  0.178305011;
	h[ 18] =  0.111774641;
	h[ 19] =  0.045174841;
	h[ 20] = -0.001852118;
	h[ 21] = -0.022554891;
	h[ 22] = -0.022509999;
	h[ 23] = -0.012931337;
	h[ 24] = -0.003386830;
	h[ 25] =  0.001801127;
	h[ 26] =  0.002887092;
	h[ 27] =  0.001996369;
	h[ 28] =  0.000893486;
	h[ 29] =  0.000259015;
	h[ 30] =  0.000039607;
	h[ 31] =  0.000000406;
	firn.Init(32, h);
	Generate(duration, &wv, &eg, &firn);

	// fc = 1000hz, (-60db at 5khz) 
	h[  0] =  0.048904450;
	h[  1] =  0.041711461;
	h[  2] =  0.051772913;
	h[  3] =  0.057376838;
	h[  4] =  0.057376838;
	h[  5] =  0.051772913;
	h[  6] =  0.041711461;
	h[  7] =  0.048904450;
	firn.Init(8, h);
	Generate(duration, &wv, &eg, &firn);

	h[  0] =  0.011450321;
	h[  1] =  0.020925663;
	h[  2] =  0.036206042;
	h[  3] =  0.054620473;
	h[  4] =  0.074191105;
	h[  5] =  0.092331533;
	h[  6] =  0.106342885;
	h[  7] =  0.113981762;
	h[  8] =  0.113981762;
	h[  9] =  0.106342885;
	h[ 10] =  0.092331533;
	h[ 11] =  0.074191105;
	h[ 12] =  0.054620473;
	h[ 13] =  0.036206042;
	h[ 14] =  0.020925663;
	h[ 15] =  0.011450321;
	firn.Init(16, h);
	Generate(duration, &wv, &eg, &firn);

	h[  0] = -0.001100185;
	h[  1] = -0.002450162;
	h[  2] = -0.004547048;
	h[  3] = -0.007005208;
	h[  4] = -0.009181706;
	h[  5] = -0.010099613;
	h[  6] = -0.008568940;
	h[  7] = -0.003421262;
	h[  8] =  0.006198307;
	h[  9] =  0.020534175;
	h[ 10] =  0.039007830;
	h[ 11] =  0.060163755;
	h[ 12] =  0.081801173;
	h[ 13] =  0.101297845;
	h[ 14] =  0.116063661;
	h[ 15] =  0.124023204;
	h[ 16] =  0.124023204;
	h[ 17] =  0.116063661;
	h[ 18] =  0.101297845;
	h[ 19] =  0.081801173;
	h[ 20] =  0.060163755;
	h[ 21] =  0.039007830;
	h[ 22] =  0.020534175;
	h[ 23] =  0.006198307;
	h[ 24] = -0.003421262;
	h[ 25] = -0.008568940;
	h[ 26] = -0.010099613;
	h[ 27] = -0.009181706;
	h[ 28] = -0.007005208;
	h[ 29] = -0.004547048;
	h[ 30] = -0.002450162;
	h[ 31] = -0.001100185;
	
	firn.Init(32, h);
	Generate(duration, &wv, &eg, &firn);

****************************************************/

	wvf.CloseWaveFile();
	return 0;
}
