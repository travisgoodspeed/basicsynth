//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef _OLD_STUFF_
// This object is set when the keyboard player is active.
// it is accessible from multiple threads.
static HANDLE genThreadH = INVALID_HANDLE_VALUE;
static DWORD  genThreadID;
static WaveOutDirect *wop = 0;

int SynthProject::Generate(int todisk, long from, long to)
{
	SeqState oldState = seq.GetState();
	if (oldState != seqOff)
		Stop();

	mixInfo->InitMixer();
	if (todisk)
		return GenerateToFile(from, to);

	WaveOutDirect wvd;
	mgr.Init(&mix, &wvd);

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	long nbuf = 4;
	// because Windows locks 20ms of the buffer, we should use least 40ms total.
	if (prjOptions.playBuf < 0.01)
		prjOptions.playBuf = 0.01;
	if (wvd.Setup(_Module.mainWnd, prjOptions.playBuf, nbuf, _Module.waveID))
		return -1;

	if (theProject->prjMidiIn.IsOn())
		oldState |= seqPlay;

	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");
	seq.SetCB(SeqCallback, synthParams.isampleRate, (Opaque)this);

	// Generate the output...
	wop = &wvd;
	bsInt32 fromSamp = from*synthParams.isampleRate;
	bsInt32 toSamp = to*synthParams.isampleRate;
	if (seq.GetTrackCount() > 1 || oldState & seqPlay)
		seq.SequenceMulti(mgr, fromSamp, toSamp, seqSeqOnce | (oldState & seqPlay));
	else
		seq.Sequence(mgr, fromSamp, toSamp);
	seq.SetCB(0, 0, 0);
	wop = 0;

	AmpValue lv, rv;
	long pad = (long) (synthParams.sampleRate * prjOptions.playBuf) * nbuf;
	while (pad-- > 0)
	{
		mix.Out(&lv, &rv);
		wvd.Output2(lv, rv);
	}
	wvd.Shutdown();

	// re-initialize in case there were dynamic mixer control changes
	mixInfo->InitMixer();

	return 0;
}

static DWORD WINAPI PlayerProc(LPVOID param)
{
	return theProject->Play();
}

int SynthProject::Play()
{
	WaveOutDirect wvd;
	mix.Reset();
	mgr.Init(&mix, &wvd);
	if (prjOptions.playBuf < 0.01)
		prjOptions.playBuf = 0.01;
	if (wvd.Setup(_Module.mainWnd, prjOptions.playBuf, 3, _Module.waveID))
		return 0;
	//ATLTRACE("Starting live playback...\n");
	wop = &wvd;
	seq.SetCB(0,0,0);
	seq.Play(mgr);
	wop = 0;
	//ATLTRACE("Stopping live playback...");
	wvd.Stop();
	//ATLTRACE("Done\n");
	return 1;
}

int SynthProject::Start()
{
	if (seq.GetState() == seqOff)
	{
		genThreadH = CreateThread(NULL, 0, PlayerProc, NULL, CREATE_SUSPENDED, &genThreadID);
		if (genThreadH != INVALID_HANDLE_VALUE)
		{
			SetThreadPriority(genThreadH, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(genThreadH);
			return 1;
		}
	}
	return seq.GetState() != seqOff;
}

int SynthProject::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (genThreadH != INVALID_HANDLE_VALUE)
	{
		try
		{
			seq.Halt();
			WaitForSingleObject(genThreadH, 10000);
		}
		catch (...)
		{
		}
		genThreadH = INVALID_HANDLE_VALUE;
	}
	return wasRunning != seqOff;
}

int SynthProject::Pause()
{
	SeqState state = seq.GetState();
	if (state & (seqSequence|seqPlay))
	{
		seq.Pause();
		while ((state = seq.GetState()) != seqPaused)
		{
			if (state == seqOff)
				break;
			Sleep(0);
		}
		if (wop)
			wop->Stop();
		return 1;
	}
	return 0;
}

int SynthProject::Resume()
{
	if (seq.GetState() == seqPaused)
	{
		if (wop)
			wop->Restart();
		seq.Resume();
		return 1;
	}
	return 0;
}
#endif
