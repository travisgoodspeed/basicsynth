/////////////////////////////////////////////////////////////////////
// BasicSynth Example09 (Chapter 16)
//
// MIDI file loading and playback
//
// NOTE: This is deprecated since the MIDI functionality has been
// moved into the main instrument manager and sequencer. The code
// is left here since it is referenced in the BasicSynth book.
// See example 9a for the new MIDI playback example.
//
// use: Example09 [-vn] infile.mid outfile.wav
// -v = master volume level
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SynthDefs.h"
#include "SynthMutex.h"
#include "SynthString.h"
#include "EnvGenSeg.h"
#include "WaveFile.h"
#include "GenWaveWT.h"
#include "GenNoise.h"
#include "BiQuad.h"
#include "DelayLine.h"
#include "Mixer.h"
#include "SynthList.h"
#include "XmlWrap.h"
#include "SeqEvent.h"
#include "MIDIDefs.h"
#include "MIDIControl.h"
#include "Instrument.h"
#include "Sequencer.h"
#include "MIDISequencer.h"

struct GMInstrParam
{
	int algorithm;
	AmpValue carStart;
	FrqValue carAtk;
	AmpValue carPk;
	FrqValue carDec;
	AmpValue carSus;
	FrqValue carRel;
	AmpValue carEnd;

	FrqValue mod1Mult;
	AmpValue mod1Start;
	FrqValue mod1Atk;
	AmpValue mod1Pk;
	FrqValue mod1Dec;
	AmpValue mod1Sus;
	FrqValue mod1Rel;
	AmpValue mod1End;

	FrqValue mod2Mult;
	AmpValue mod2Start;
	FrqValue mod2Atk;
	AmpValue mod2Pk;
	FrqValue mod2Dec;
	AmpValue mod2Sus;
	FrqValue mod2Rel;
	AmpValue mod2End;

} parmsGM[] = {
// PIANO group 0-7
// Acoustic Grand, Bright Acoustic, Electric Grand, Honky-Tonk,
// Electric Piano 1, Electric Piano 2, Harpsichord, Clavinet,
{    2,  0.000,  0.015,  1.000,  3.000,  0.000,  0.100,  0.000,
 2.996,  3.000,  0.010,  1.000,  2.000,  0.600,  0.150,  0.200,
 5.001,  2.000,  0.010,  1.500,  1.000,  0.200,  0.100,  0.100 },

// Chromatic Percussion
{    4,  0.000,  0.010,  1.000,  2.000,  0.000,  0.200,  0.000,
 1.414,  2.500,  0.020,  2.000,  1.600,  0.100,  0.200,  0.000,
 7.000,  1.000,  0.300,  0.400,  0.600,  0.000,  0.100,  0.000 },

// ORGANS 16-23
{    3,  0.000,  0.150,  1.000,  0.100,  0.950,  0.150,  0.000,
 2.001,  1.000,  0.150,  2.000,  0.000,  2.000,  0.050,  0.500,
 4.999,  1.000,  0.150,  1.500,  0.000,  1.500,  0.050,  0.500 },

 // Guitar group 24-31
{    3,  0.000,  0.015,  1.000,  2.000,  0.000,  0.100,  0.000,
 3.000,  1.500,  0.200,  1.200,  1.600,  0.500,  0.100,  0.000,
 8.000,  0.600,  0.010,  0.500,  0.430,  0.100,  0.100,  0.000 },

// Bass group 32-39
{    3,  0.000,  0.005,  1.000,  2.000,  0.000,  0.080,  0.000,
 1.000,  1.000,  0.200,  1.200,  1.600,  2.000,  0.100,  0.000,
 1.000,  1.000,  0.000,  1.000,  0.000,  1.000,  0.000,  1.000 },

 // String 40-47
{    2,  0.000,  0.150,  1.000,  0.100,  0.950,  0.150,  0.000,
 4.000,  2.000,  0.050,  1.500,  0.000,  1.500,  0.150,  1.000,
 3.000,  0.000,  0.010,  1.000,  0.000,  1.000,  0.150,  0.500 },

 // Ensemble 48-55
{    3,  0.000,  0.150,  1.000,  0.100,  0.950,  0.150,  0.000,
 2.001,  1.000,  0.150,  2.000,  0.000,  2.000,  0.050,  0.500,
 4.999,  1.000,  0.150,  1.500,  0.000,  1.500,  0.050,  0.500 },

// Brass 56-63
{    3,  0.000,  0.100,  1.000,  0.200,  0.800,  0.200,  0.000,
 1.000,  0.000,  0.080,  1.200,  0.200,  0.800,  0.200,  0.000,
 2.000,  0.500,  0.050,  1.500,  0.100,  0.500,  0.100,  0.300 },

// Reed 64-71
{    3,  0.000,  0.100,  1.000,  0.200,  0.900,  0.200,  0.000,
 2.000,  0.500,  0.030,  2.000,  0.100,  0.800,  0.200,  0.200,
 4.000,  0.200,  0.010,  0.600,  0.100,  0.400,  0.100,  0.000 },

// Pipe 56-63
{    3,  0.000,  0.200,  1.000,  0.200,  0.900,  0.200,  0.000,
 3.000,  0.000,  0.030,  0.700,  0.100,  0.150,  0.200,  0.000,
 1.000,  0.400,  0.010,  0.500,  0.200,  0.200,  0.150,  0.000 },

// Synth Lead
{    2,  0.000,  0.040,  1.000,  0.100,  0.700,  0.200,  0.000,
 1.000,  0.000,  0.040,  3.000,  0.150,  0.800,  0.200,  0.000,
 4.000,  0.000,  0.050,  2.000,  0.100,  0.200,  0.150,  0.000 },
// Synth Pad
{    2,  0.000,  0.040,  1.000,  0.300,  0.000,  0.200,  0.000,
 1.000,  0.000,  0.010,  4.000,  0.150,  0.000,  0.200,  0.000,
 4.000,  0.000,  0.010,  3.000,  0.100,  0.000,  0.150,  0.000 },
// Synth Effects
{    2,  0.000,  0.040,  1.000,  0.300,  0.000,  0.200,  0.000,
 1.000,  0.000,  0.010,  4.000,  0.150,  0.000,  0.200,  0.000,
 4.000,  0.000,  0.010,  3.000,  0.100,  0.000,  0.150,  0.000 },

	// Ethnic
{    2,  0.000,  0.010,  1.000,  0.500,  0.000,  0.200,  0.000,
 3.000,  3.000,  0.020,  2.500,  0.400,  2.000,  0.200,  1.000,
 4.000,  0.000,  0.010,  3.000,  0.200,  1.000,  0.150,  0.000 },
	// Percussive
{    2,  0.000,  0.010,  1.000,  0.500,  0.000,  0.200,  0.000,
 3.000,  3.000,  0.020,  2.500,  0.400,  2.000,  0.200,  1.000,
 4.000,  0.000,  0.010,  3.000,  0.200,  1.000,  0.150,  0.000 },
	// Sound Effects
};

//PIANO
// "Acoustic Grand", "Bright Acoustic", "Electric Grand", "Honky-Tonk", "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavinet",
// CHROMATIC PERCUSSION
// "Celesta","Glockenspiel","Music Box","Vibraphone","Marimba","Xylophone","Tubular Bells","Dulcimer",
//ORGAN
// "Drawbar Organ", "Percussive Organ", "Rock Organ","Church Organ","Reed Organ","Accoridan","Harmonica","Tango Accordian",
// GUITAR
//"Nylon String Guitar","Steel String Guitar","Electric Jazz Guitar","Electric Clean Guitar","Electric Muted Guitar","Overdriven Guitar","Distortion Guitar","Guitar Harmonics",
// BASS
//"Acoustic Bass","Electric Bass(finger)","Electric Bass(pick)","Fretless Bass","Slap Bass","Slap Bass","Synth Bass","Synth Bass",
// SOLO STRINGS
//"Violin","Viola","Cello","Contrabass","Tremolo Strings","Pizzicato Strings","Orchestral Strings","Timpani",
//  ENSEMBLE
//"String Ensemble 1","String Ensemble 2","SynthStrings 1","SynthStrings 2","Choir Aahs","Voice Oohs","Synth Voice","Orchestra Hit",
//BRASS
//"Trumpet","Trombone","Tuba","Muted Trumpet","French Horn","Brass Section","SynthBrass 1","SynthBrass 2",
//  REED
//"Soprano Sax","Alto Sax","Tenor Sax","Baritone Sax","Oboe","English Horn","Bassoon","Clarinet",
//PIPE
//"Piccolo","Flute","Recorder","Pan Flute","Blown Bottle","Skakuhachi","Whistle","Ocarina",
//  SYNTH LEAD
//"Lead 1 (square)","Lead 2 (sawtooth)","Lead 3 (calliope)","Lead 4 (chiff)","Lead 5 (charang)","Lead 6 (voice)","Lead 7 (fifths)","Lead 8 (bass+lead)",
//SYNTH PAD
//"Pad 1 (new age)","Pad 2 (warm)","Pad 3 (polysynth)","Pad 4 (choir)","Pad 5 (bowed)","Pad 6 (metallic)","Pad 7 (halo)","Pad 8 (sweep)",
//  SYNTH EFFECTS
//"FX 1 (rain)","FX 2 (soundtrack)","FX 3 (crystal)","FX 4 (atmosphere)","FX 5 (brightness)","FX 6 (goblins)","FX 7 (echoes)","FX 8 (sci-fi)",
 // ETHINIC
//"Sitar","Banjo","Shamisen","Koto","Kalimba","Bagpipe","Fiddle","Shanai",
//  PERCUSSIVE
//"Tinkle Bell","Agogo","Steel Drums","Woodblock","Taiko Drum","Melodic Tom","Synth Drum","Reverse Cymbal",
//SOUND EFFECTS
//"Guitar Fret Noise","Breath Noise","Seashore","Bird Tweet","Telephone Ring","Helicopter","Applause","Gunshot"

FrqValue pwDepth = 16384.0 / 1; // 2^14 / (semi * 2)
FrqValue bendFact = 1.122462048; // 2^(2/12)

#define ALG_STACK 1
#define ALG_STACK2 2
#define ALG_WYE 3
#define ALG_DELTA 4

class FMInstr : public MIDIInstrument
{
private:
	GenWaveWT carOsc;
	GenWaveWT mod1Osc;
	GenWaveWT mod2Osc;
	EnvGenADSR carEG;
	EnvGenADSR mod1EG;
	EnvGenADSR mod2EG;
	GenWave32 lfoOsc;
	AllPassDelay apd;

	int algorithm;
	long delaySamps;
	int patch;
	FrqValue frq;
	AmpValue vol;
	PhsAccum lfoRad;

	AmpValue CalcPhaseMod(AmpValue amp, FrqValue mult)
	{
		amp = (amp * mult) * synthParams.frqTI;
		AmpValue maxPhs = synthParams.ftableLength / 2;
		if (amp > maxPhs)
			amp = maxPhs;
		return amp;
	}

public:
	FMInstr()
	{
		frq = 440.0;
		vol = 1.0;
		patch = 0;
		lfoRad = 0;
	}

	void SetPatch(int p)
	{
		patch = p;
	}

	virtual void NoteOn(short key, short vel)
	{
		GMInstrParam *pp = &parmsGM[patch];
		algorithm = pp->algorithm;
		vol = (AmpValue) vel / 127.0;
		frq = synthParams.GetFrequency((int)key-12);
		carOsc.InitWT(frq, WT_SIN);
		FrqValue mul1 = frq * pp->mod1Mult;
		FrqValue mul2 = frq * pp->mod2Mult;
		mod1Osc.InitWT(mul1, WT_SIN);
		mod2Osc.InitWT(mul2, WT_SIN);
		carEG.InitADSR(pp->carStart, pp->carAtk, pp->carPk, pp->carDec, pp->carSus, pp->carRel, pp->carEnd, linSeg);
		mod1EG.InitADSR(CalcPhaseMod(pp->mod1Start, mul1),
			pp->mod1Atk, CalcPhaseMod(pp->mod1Pk,  mul1),
			pp->mod1Dec, CalcPhaseMod(pp->mod1Sus, mul1),
			pp->mod1Rel, CalcPhaseMod(pp->mod1End, mul1),
			linSeg);
		mod2EG.InitADSR(CalcPhaseMod(pp->mod2Start, mul2),
			pp->mod2Atk, CalcPhaseMod(pp->mod2Pk,  mul2),
			pp->mod2Dec, CalcPhaseMod(pp->mod2Sus, mul2),
			pp->mod2Rel, CalcPhaseMod(pp->mod2End, mul2),
			linSeg);
		//apd.InitDLR(0.02, 0.2, 0.001);
		//delaySamps = 0.2 * synthParams.sampleRate;
	}

	virtual void NoteOff(short key, short vel)
	{
		carEG.Release();
		mod1EG.Release();
		mod2EG.Release();
	}

	virtual void ControlChange(short ctl, short val)
	{
		if (ctl == MIDI_CTRL_EXPR)
			vol = (AmpValue) val / 127.0;
	}

	virtual void AfterTouch(short val)
	{
	}

	virtual void PitchBend(short val)
	{
		FrqValue bendVal;
		if (val != 0)
			bendVal = frq + (frq * (bendFact * ((FrqValue) val / pwDepth)));
		else
			bendVal = frq;
		carOsc.SetFrequency(bendVal);
		carOsc.Reset(-1);
	}

	virtual int IsFinished()
	{
		if (carEG.IsFinished())
		{
			if (--delaySamps <= 0)
				return 1;
		}
		return 0;
	}


	virtual void Tick(Mixer *mix, short chnl)
	{
		AmpValue v1, v2;
		switch (algorithm)
		{
		case ALG_STACK2:
			mod1Osc.PhaseModWT(mod2EG.Gen() * mod2Osc.Gen());
		case ALG_STACK:
			carOsc.PhaseModWT(mod1EG.Gen() * mod1Osc.Gen());
			break;
		case ALG_WYE:
			carOsc.PhaseModWT((mod1EG.Gen() * mod1Osc.Gen()) + (mod2EG.Gen() * mod2Osc.Gen()));
			break;
		case ALG_DELTA:
			v1 = mod2EG.Gen() * mod2Osc.Gen();
			mod1Osc.PhaseModWT(v1);
			v2 = mod1EG.Gen() * mod1Osc.Gen();
			carOsc.PhaseModWT(v1 + v2);
			break;
		}
		//v1 = vol * carEG.Gen() * carOsc.Gen();
		//v2 = apd.Sample(v1);
		//mix->ChannelIn(chnl, (v1 * 0.7) + (v2 * 0.3));
		mix->ChannelIn(chnl, vol * carEG.Gen() * carOsc.Gen());
	}
};

class SqrInstr : public MIDIInstrument
{
	// temporary
	GenWaveWT osc;
	EnvGenADSR  env;
	FrqValue  frq;

	GenWave32 lfo;
	AmpValue lfoAmp;
	AmpValue lfoScale;
	int lfoOn;

	AmpValue volLevel;

public:
	SqrInstr()
	{
		volLevel = 1.0;
		lfoOn = 0;
		lfo.InitWT(3.5, WT_SIN);
		lfoScale = 10.0; // max 10hz deflection
	}

	virtual void NoteOn(short key, short vel)
	{
		frq = synthParams.GetFrequency((int)key-12);
		osc.InitWT(frq, WT_SQR);
		AmpValue amp = (AmpValue) vel / 127.0;
		env.InitADSR(0.0, 0.01, amp, 1.0, amp * 0.1, 0.05, 0.0, linSeg);
		lfoAmp = 0;
		lfoOn = 0;
	}

	virtual void NoteOff(short key, short vel)
	{
		env.Release();
	}

	virtual void ControlChange(short ctl, short val)
	{
		if (ctl == MIDI_CTRL_MOD)
		{
			if (val != 0)
			{
				lfoAmp = ((AmpValue) val / 127.0) * lfoScale;
				lfoOn = 1;
			}
			else
				lfoOn = 0;
		}
		else if (ctl == MIDI_CTRL_EXPR)
			volLevel = (AmpValue) val / 127.0;
	}

	virtual void AfterTouch(short val)
	{
	}

	virtual void PitchBend(short val)
	{
		FrqValue bendVal;
		if (val != 0)
			bendVal = frq + (frq * (bendFact * ((FrqValue) val / pwDepth)));
		else
			bendVal = frq;
		osc.SetFrequency(bendVal);
		osc.Reset(-1);
	}

	virtual int IsFinished()
	{
		return env.IsFinished();
	}

	virtual void Tick(Mixer *mix, short chnl)
	{
		if (lfoOn)
			osc.Modulate(lfo.Gen() * lfoAmp);

		mix->ChannelIn(chnl, volLevel * env.Gen() * osc.Gen());
	}
};

class DrumInstr : public MIDIInstrument
{
	GenNoise nz;
	GenWave32 osc;
	EnvGenADSR eg;
	Reson flt;
	AmpValue vol;

public:
	virtual void NoteOn(short key, short vel)
	{
		flt.Init(200.0, 0.5, 0.5);
		osc.InitWT(synthParams.GetFrequency(key-12), WT_SIN);
		eg.InitADSR(0, 0.001, (AmpValue) vel / 127.0, 0.1, 0, 0.01, expSeg);
	}
	void NoteOff(short key, short vel)
	{
		eg.Release();
	}

	void ControlChange(short ctl, short val)
	{
	}

	void AfterTouch(short val)
	{}

	void PitchBend(short val)
	{}

	virtual int IsFinished()
	{
		return eg.IsFinished();
	}

	void Tick(Mixer *mix, short chnl)
	{
		AmpValue a = eg.Gen()/2;
		AmpValue s = a * (nz.Gen() + osc.Gen());
		s = flt.Sample(s);
		mix->ChannelIn(chnl, s);
	}
};

class SqrManager : public MIDIChannel
{
private:

public:
	SqrManager(short ch) : MIDIChannel(ch)
	{
	}

	MIDIInstrument *Allocate(int trk)
	{
		if (chnlNum == 9)
			return new DrumInstr;
		FMInstr *pNew = new FMInstr;
		pNew->SetPatch(patch/8);
		return pNew;
	}

	void Deallocate(MIDIInstrument *p)
	{
		delete p;
	}
};

class Playback : public WaveFile
{
public:
	bsInt32 secCount;
	bsInt32 secs;

	Playback()
	{
		secCount = synthParams.isampleRate;
		secs = 0;
	}

	void Output2(AmpValue vleft, AmpValue vright)
	{
		WaveFile::Output2(vleft, vright);
		if (--secCount <= 0)
		{
			secCount = synthParams.isampleRate;
			secs++;
			printf("%d seconds\r", secs);
		}
	}
};

void PlayNote(MIDITrack *trk, int pit, int t1, int t2)
{
	MIDIEvent *evt;

	evt = new MIDIEvent;
	evt->chan = 0;
	evt->deltat = 6;
	evt->event = 0x90;
	evt->val1 = pit;
	evt->val2 = 120;
	trk->AddEvent(evt);
	evt = new MIDIEvent;
	evt->chan = 0;
	evt->deltat = 24;
	evt->event = 0x90;
	evt->val1 = pit;
	evt->val2 = 0;
	trk->AddEvent(evt);
}

int main(int argc, char *argv[])
{
	InitSynthesizer();

	char *inFile = NULL;
	const char *outFile = "example09.wav";
	AmpValue vol = 1.0;
	if (argc > 1)
	{
		int i = 1;
		while (i < argc && argv[i][0] == '-')
		{
			if (argv[i][1] == 'v')
				vol = atof(&argv[i][2]);
			i++;
		}
		if (i < argc)
			inFile = argv[i++];
		if (i < argc)
			outFile = argv[i];
	}

	MIDISequence seq;
	MIDIFileLoad seqFile;

	for (short ch = 0; ch < 16; ch++)
		seq.SetChannelMgr(ch, new SqrManager(ch));

	seqFile.SetSequencer(&seq);
	if (inFile)
	{
		if (seqFile.LoadFile(inFile))
			exit(1);
		const char *str;
		if ((str = seqFile.MetaText()) != NULL)
			printf("%s\n", str);
		if ((str = seqFile.SeqName()) != NULL)
			printf("%s\n", str);
		if ((str = seqFile.Copyright()) != NULL)
			printf("Copyright: %s\n", str);
		if ((str = seqFile.KeySignature()) != NULL && *str)
			printf("Key: %s\n", str);
		if ((str = seqFile.TimeSignature()) != NULL && *str)
			printf("Time: %s\n", str);

	}
	else
	{
		// Test GM sounds
		seq.SetPPQN(24);
		MIDIEvent *evt;
		MIDITrack *trk;
		trk = seq.AddTrack(0);
		for (int pat = 0; pat < 128; pat += 8)
		{
			evt = new MIDIEvent;
			evt->deltat = 24;
			evt->chan = 0;
			evt->event = MIDI_PRGCHG;
			evt->val1 = pat;
			trk->AddEvent(evt);
			for (int pit = 36; pit <= 72; pit += 12)
				PlayNote(trk, pit, 6, 24);
		}
		evt = new MIDIEvent;
		evt->deltat = 6;
		evt->event = MIDI_META;
		evt->chan = MIDI_META_EOT;
		trk->AddEvent(evt);
	}

	Playback wvf;
	if (wvf.OpenWaveFile(outFile, 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	seq.MasterVolume(vol);
	seq.Sequence(&wvf);

	wvf.CloseWaveFile();

	return 0;
}
