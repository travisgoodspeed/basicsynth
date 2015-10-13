
/////////////////////////////////////////////////////////////////////////////
// Platform-specific project functions
/////////////////////////////////////////////////////////////////////////////
#ifdef OLD_STUFF
#include "globinc.h"
#include "MainFrm.h"
#include <MIDIDefs.h>
#include <MIDIControl.h>

#ifdef _WIN32
static WaveOutDirect *wop = 0;
static HANDLE genThreadH;
static DWORD  genThreadID;
#endif
#ifdef UNIX
static WaveOutALSA *wop = 0;
static pthread_t genThreadID;
#endif

int SynthProject::Generate(int todisk, long from, long to)
{
	SeqState oldState = seq.GetState();
	if (oldState != seqOff)
		Stop();

	mixInfo->InitMixer();
	if (todisk)
		return GenerateToFile(from, to);
	long nbuf = 4;
	if (prjOptions.playBuf < 0.01)
		prjOptions.playBuf = 0.01;
#ifdef _WIN32
	WaveOutDirect wvd;
	if (wvd.Setup(fl_xid(mainWnd), prjOptions.playBuf, nbuf))
		return -1;
#endif
#ifdef UNIX
	WaveOutALSA wvd;
	if (wvd.Setup(prjOptions.waveDevice, prjOptions.playBuf, nbuf))
		return -1;
#endif
	mgr.Init(&mix, &wvd);

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");

	if (theProject->prjMidiIn.IsOn())
		oldState |= seqPlay;

	// Generate the output...
	wop = &wvd;
	bsInt32 fromSamp = from*synthParams.isampleRate;
	bsInt32 toSamp = to*synthParams.isampleRate;
	if (seq.GetTrackCount() > 1 || oldState & seqPlay)
		seq.SequenceMulti(mgr, fromSamp, toSamp, seqSeqOnce | (oldState & seqPlay));
	else
		seq.Sequence(mgr, fromSamp, toSamp);
	wop = 0;

	AmpValue lv, rv;
	long pad = (long) (synthParams.sampleRate * prjOptions.playBuf) * nbuf;
	while (pad-- > 0)
	{
		mix.Out(&lv, &rv);
		wvd.Output2(lv, rv);
	}
	wvd.Shutdown();

	// re-initialize in case of dynamic mixer control changes
	mixInfo->InitMixer();

	return 0;
}

int SynthProject::Play()
{
#ifdef _WIN32
	WaveOutDirect wvd;
	if (wvd.Setup(fl_xid(mainWnd), 0.02, 3))
		return 0;
#endif
#ifdef UNIX
	WaveOutALSA wvd;
	if (wvd.Setup(prjOptions.waveDevice, 0.02, 3))
		return 0;
#endif
	mix.Reset();
	mgr.Init(&mix, &wvd);
	wop = &wvd;
	seq.Play(mgr);
	wop = 0;
	wvd.Stop();
	return 1;
}

#ifdef _WIN32
static DWORD WINAPI PlayerProc(LPVOID param)
{
	return theProject->Play();
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
#endif

#ifdef UNIX
#include <pthread.h>

static void *PlayerProc(void *param)
{
	theProject->Play();
	return 0;
}

int SynthProject::Start()
{
	return pthread_create(&genThreadID, NULL, PlayerProc, 0);
}

int SynthProject::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (wasRunning)
	{
		seq.Halt();
		pthread_join(genThreadID, NULL); 
	}
	return wasRunning != seqOff;
}
#endif

int SynthProject::Pause()
{
	if (seq.GetState() != seqPaused)
	{
		seq.Pause();
		while (seq.GetState() != seqPaused)
		{
#ifdef _WIN32
			Sleep(0);
#endif
#ifdef UNIX
			usleep(1000);
#endif
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
