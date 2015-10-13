///////////////////////////////////////////////////////////
// BasicSynth - Example 10 (Chapters 18-23)
// Instruments test
//
// ToneInstr
// SubSynth
// AddSynth
// FMSynth
// MatixSynth
// WFSynth
// Chuffer
// ModSynth
// BuzzSynth
//
// use: example10 [instrlib.xml]
//
// If no instrlib file is indicated, the file data.xml must exist in the current directory.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include <Instruments.h>

#pragma warning(disable : 4996)

// Uncomment the next line to add reverb to sounds.
//#define ADD_REVERB 1
// Uncomment the next line to dump instrument definitions back to disk.
// This will validate that the internal state is the same as the
// input file contents. Note that attributes may get rearranged
// and additional nodes or attributes may appear in the output file.
//#define TEST_SAVE_INSTR 1

long evidcount = 1;
float startTime = 0;
InstrManager inMgr;
Sequencer seq;
Mixer mix;
Reverb2 rvrb;
WaveFile wvf;


static void AddEvent(bsInt16 inum, int pit, float dur)
{
	SeqEvent *evt;
	evt = inMgr.ManufEvent(inum);
	evt->evid = evidcount++;
	evt->type = SEQEVT_START;
	evt->track = 0;
	evt->SetParam(P_INUM, (float)inum);
	evt->SetParam(P_CHNL, 0.0);
	evt->SetParam(P_START, startTime);
	evt->SetParam(P_DUR, dur);
	evt->SetParam(P_PITCH, (float)pit);
	evt->SetParam(P_VOLUME, 0.7);
	startTime += dur;
	seq.AddEvent(evt);
}

static void AddSequence(bsInt16 inum, float dur)
{
	for (int pit = 48; pit <= 53; pit++)
		AddEvent(inum, pit, dur);
	startTime += 0.1;
}

static void DestroyTemplate(Opaque tp)
{
	Instrument *ip = (Instrument *)tp;
	delete ip;
}

int main(int argc, char *argv[])
{
#if defined(USE_MSXML)
	CoInitialize(0);
#endif

	const char *fname = "data.xml";
	if (argc > 1)
		fname = argv[1];

	InitSynthesizer();

	mix.SetChannels(2);
	mix.MasterVolume(1.0, 1.0);
	mix.ChannelOn(0, 1);
	mix.ChannelOn(1, 1);
	mix.ChannelVolume(0, 1.0);
	mix.ChannelVolume(1, 1.0);
#ifdef ADD_REVERB
	mix.SetFxChannels(1);
	mix.FxInit(0, &rvrb, 0.1);
	mix.FxLevel(0, 0, 0.2);
	mix.FxLevel(0, 1, 0.2);
	rvrb.InitReverb(1.0, 2.0);
#endif

	inMgr.Init(&mix, &wvf);

	inMgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
	inMgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
	inMgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
	inMgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
	inMgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
	inMgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
	inMgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
	inMgr.AddType("Chuffer", Chuffer::ChufferFactory, Chuffer::ChufferEventFactory);
	inMgr.AddType("ModSynth", ModSynth::ModSynthFactory, ModSynth::ModSynthEventFactory);
	inMgr.AddType("BuzzSynth", BuzzSynth::InstrFactory, BuzzSynth::EventFactory);
	InstrMapEntry *ime = 0;
	while ((ime = inMgr.EnumType(ime)) != 0)
		ime->dumpTmplt = DestroyTemplate;

	XmlSynthDoc doc;
	XmlSynthElem rootNode(&doc);
	if (!doc.Open(fname, &rootNode))
	{
		printf("Cannot open file %s\n", fname);
		exit(1);
	}

	// Optional: use LoadInstrLib(inMgr, fname)
	// but we want to discover the inum values
	// and add sequences programaticaly...

	XmlSynthElem elem(&doc);
	XmlSynthElem *inst = rootNode.FirstChild(&elem);
	while (inst != NULL)
	{
		if (inst->TagMatch("instr"))
		{
			InstrConfig *ent = inMgr.LoadInstr(inst);
			if (strcmp(ent->instrType->GetType(), "WFSynth") == 0)
				AddEvent(ent->inum, 48, 1.0);
			else
				AddSequence(ent->inum, 0.25);
		}

		inst = elem.NextSibling(&elem);
	}
	doc.Close();

	if (wvf.OpenWaveFile("example10.wav", 2))
	{
		printf("Cannot open wavefile for output\n");
		exit(1);
	}
	seq.Sequence(inMgr);

#ifdef ADD_REVERB
	// drain the reverb...
	AmpValue lv;
	AmpValue rv;
	long n = synthParams.isampleRate;
	while (n-- > 0)
	{
		mix.Out(&lv, &rv);
		wvf.Output2(lv, rv);
	}
#endif

	wvf.CloseWaveFile();

	///////////////////////////////////////////////////////////////
	// Code to test instrument save functions...
#define TEST_SAVE_INSTR 1
#ifdef TEST_SAVE_INSTR
	doc.NewDoc("instrlib", &rootNode);
	InstrConfig *inc = inMgr.EnumInstr(0);
	while (inc)
	{
		InstrMapEntry *ime = inc->instrType;
		Instrument *ip = (Instrument *)inc->instrTmplt;
		if (ip)
		{
			rootNode.AddChild("instr", &elem);
			elem.SetAttribute("id", inc->inum);
			elem.SetAttribute("type", ime->itype);
			elem.SetAttribute("name", inc->GetName());
			elem.SetAttribute("desc", inc->GetDesc());
			ip->Save(&elem);
		}
		inc = inMgr.EnumInstr(inc);
	}
	bsString outxml(fname);
	bsString outbase;
	bsString outfile;
	outxml.SplitPath(outbase, outfile, 1);
	outxml = outbase;
	outxml += '_';
	outxml += outfile;
	doc.Save(outxml);
#endif
	///////////////////////////////////////////////////////////////

	return 0;
}
