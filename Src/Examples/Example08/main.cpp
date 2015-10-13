/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 8 (Chapter 15)
//
// BasicSynth Sequencer
//
// use: Example08
//
// Copyright 2008, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>

#define P_FRQ   (P_XTRA)         // 4
#define P_WT    (P_XTRA+1)       // 5
#define P_AMP   (P_XTRA+2)       // 6
#define P_ATK   (P_XTRA+3)       // 7
#define P_REL   (P_XTRA+4)       // 8

struct Note : public SeqEvent
{
	FrqValue frq;
	AmpValue amp;
	FrqValue atk;
	FrqValue rel;
	int wt;

	Note()
	{
		frq = 440.0;
		amp = 1.0;
		atk = 0.0;
		rel = 0.0;
	}

	virtual ~Note() { }

	virtual bsInt16 MaxParam() 
	{ 
		return P_REL; 
	}

	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_FRQ:
			frq = synthParams.GetFrequency((int)v);
			break;
		case P_WT:
			wt = (int) v;
			break;
		case P_AMP:
			amp = v;
			break;
		case P_ATK:
			atk = v;
			break;
		case P_REL:
			rel = v;
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}
};


class Toner : public Instrument
{
private:
	GenWave32 wv;
	EnvGenAR  eg;
	InstrManager *inMgr;
	int channel;

public:
	static Instrument *Factory(InstrManager *p, Opaque tmplt)
	{
		return new Toner(p);
	}

	static SeqEvent *ManufEvent(Opaque tmplt)
	{
		return new Note;
	}

	Toner(InstrManager *p)
	{
		inMgr = p;
		channel = 0;
	}

	~Toner()
	{
	}

	void Start(SeqEvent *evt)
	{
		Note *np = (Note *)evt;
		channel = np->chnl;
		wv.InitWT(np->frq, np->wt);
		eg.InitAR(np->atk, np->amp, np->rel, 1, expSeg);
	}

	void Param(SeqEvent *evt)
	{
		wv.SetFrequency(((Note*)evt)->frq);
		wv.Reset(-1);
	}

	void Stop()
	{
		eg.Release();
	}

	void Tick()
	{
		inMgr->Output(channel, eg.Gen() * wv.Gen());
	}

	int IsFinished()
	{
		return eg.IsFinished();
	}
};

#define P_MODM  (P_REL+1)    // 9
#define P_MODI  (P_REL+2)    // 10

struct NoteFM : public Note
{
	FrqValue mul;
	AmpValue mi;

	NoteFM()
	{
		mul = 2.0;
		mi = 1.0;
	}

	virtual bsInt16 MaxParam() 
	{ 
		return P_MODI; 
	}

	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_MODM:
			mul = v;
			break;
		case P_MODI:
			mi = v;
			break;
		default:
			Note::SetParam(id, v);
			break;
		}
	}
};

class EffEm : public Instrument
{
private:
	GenWaveFM wv;
	EnvGenAR  eg;
	InstrManager *inMgr;
	int channel;

public:
	static Instrument *Factory(InstrManager *p, Opaque tmplt)
	{
		return new EffEm(p);
	}

	static SeqEvent *ManufEvent(Opaque tmplt)
	{
		NoteFM *p = new NoteFM;
		p->amp = 1.0;
		p->atk = 0.01;
		p->rel = 0.1;
		p->mul = 2.0;
		p->mi  = 1.0;
		return p;
	}

	EffEm(InstrManager *p)
	{
		inMgr = p;
		channel = 0;
	}

	~EffEm()
	{
	}

	void Start(SeqEvent *evt)
	{
		NoteFM *np = (NoteFM *)evt;
		channel = np->chnl;
		wv.InitFM(np->frq, np->mul, np->mi, np->wt);
		eg.InitAR(np->atk, np->amp, np->rel, 1, expSeg);
	}

	void Param(SeqEvent *evt)
	{
		wv.SetFrequency(((NoteFM*)evt)->frq);
		wv.Reset(-1);
	}

	void Stop()
	{
		eg.Release();
	}

	void Tick()
	{
		inMgr->Output(channel, eg.Gen() * wv.Gen());
	}

	int IsFinished()
	{
		return eg.IsFinished();
	}
};


int main(int argc, char *argv[])
{
	InitSynthesizer();

	WaveFile wvf;
	if (wvf.OpenWaveFile("example08.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}

	Mixer mix;
	mix.SetChannels(2);
	mix.MasterVolume(1.0, 1.0);
	mix.ChannelOn(0, 1);
	mix.ChannelVolume(0, 0.7);
	mix.ChannelPan(0, panTrig, 0.6);
	mix.ChannelOn(1, 1);
	mix.ChannelVolume(1, 0.7);
	mix.ChannelPan(1, panTrig, -0.4);

	InstrMapEntry *ime;
	InstrManager im;
	im.Init(&mix, &wvf);
	ime = im.AddType("Tone", Toner::Factory, Toner::ManufEvent);
	im.AddInstrument(1, ime, 0);
	ime = im.AddType("EffEm", EffEm::Factory, EffEm::ManufEvent);
	im.AddInstrument(2, ime, 0);

	Sequencer seq;
	SequenceFile infile;
	infile.Init(&im, &seq);

	if (argc > 1)
		infile.LoadFile(argv[1]);
	else
	{
		//               i ch start dur  pit wt amp  atk  rel [mul mi]
		infile.ParseMem("1 0  0.00  0.25 48  2  1.00 0.08 0.1");
		infile.ParseMem("1 0  0.25  0.25 50  2  1.00 0.07 0.1");
		infile.ParseMem("1 0  0.50  0.25 52  2  1.00 0.06 0.1");
		infile.ParseMem("1 0  0.75  0.25 53  2  1.00 0.05 0.1");
		infile.ParseMem("1 0  1.00  1.00 55  2  1.00 0.04 0.1");
		infile.ParseMem("1 0  2.00  1.00 48  2  1.00 0.10 0.1");
		infile.ParseMem("=2 4 5 6 8 9"); // pitch wt amp rel mul
		infile.ParseMem("2 1  0.00  1.00 36  0  1.00 0.20 2");
		infile.ParseMem("2 1  1.00  0.50 31  0  1.00 0.05 2");
		infile.ParseMem("2 1  1.50  0.50 35  0  1.00 0.05 4");
		infile.ParseMem("2 1  2.00  1.00 36  0  1.00 0.10 5");
	}

	seq.Sequence(im);
	seq.Reset();

	wvf.CloseWaveFile();

	return 0;
}
