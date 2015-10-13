//////////////////////////////////////////////////////////////////////
/// @file GMSynthDLL.cpp General MIDI Synthesizer
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef UNIX
typedef struct _GUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#endif

static const GUID GMSYNTH_MAGIC  =
{ 0x2d257162, 0x432b, 0x438d, { 0xbd, 0x5e, 0x86, 0x9a, 0xbb, 0x7, 0x3a, 0x3c } };

/// Structure to hold global information.
/// There is exactly one of these allocated.
class GMSynthDLL : public SynthThread
{
private:
	GMInstrManager inmgr;
	SequencerCB seq;
	SeqState seqMode;
	WaveFile wvf;
	SoundBank *sbnk;
	SMFFile midFile;
	MIDIInput kbd;
	GMSYNTHCB usrCB;
	void *usrArg;
	bsString outFileName;
	float ldTm;
	int live;
	bsInt32 stTime;
	bsInt32 endTime;

#ifdef _WIN32
	WaveOutDirectI wvd;
	HWND wavWnd;
	GUID wavDevice;
#endif

#ifdef UNIX
	WaveOutALSA wvd;
	char *wavDevice;
#endif

	static void eventCB(bsUint32 tick, bsInt16 evtID, const SeqEvent *evt, void *usr);
	static void tickCB(bsInt32 cnt, void *arg);

public:
	GUID magic;

	GMSynthDLL(Opaque w)
	{
		magic = GMSYNTH_MAGIC;
		sbnk = 0;
		seqMode = seqOff;
		seq.SetMaxNotes(32);
		kbd.SetSequenceInfo(&seq, &inmgr);
		wvf.SetBufSize(30);
		ldTm = 0.5;
		live = 1;
		stTime = 0;
		endTime = 0;

#ifdef _WIN32
		SetWaveDevice(NULL);
		wavWnd = (HWND)w;
#endif
#ifdef UNIX
		SetWaveDevice((void*)"hw:0");
#endif
	}

	~GMSynthDLL()
	{
		memset(&magic, 0, sizeof(GUID));
	}

	void SetTimes(float start, float end);
	void SetVolume(float db, float rv);
	void SetCallback(GMSYNTHCB cb, bsInt32 cbRate, void *arg);
	void OnTick(bsInt32 cnt);
	void OnEvent(bsInt16 evtid, const SeqEvent *evt);

	void SetWaveDevice(void *dev);
	void OpenWaveDevice();
	void CloseWaveDevice();
	int  ThreadProc();

	int Unload();
	int LoadSoundBank(const char *alias, const char *fileName, int preload, float scl);
	int LoadSequence(const char *fileName, const char *sbnkName, short mask);
	int GetMetaText(int id, char *txt, size_t len);
	int GetMetaData(int id, long *vals);
	int GetBanks(short *banks, size_t len);
	int GetPreset(short bank, short preset, char *txt, size_t len);

	int Start(int mode);
	int Stop();
	int Pause();
	int Resume();
	int Generate(const char *fileName);
	int MidiIn(int onoff, int device);
	void ImmediateEvent(short mmsg, short val1, short val2);
};

static GMSynthDLL *theSynth;
static int openCount = 0;

/////////////////////////////////////////////////////////////////////
// C++ Wrapper
////////////////////////////////////////////////////////////////////

GMSynth::GMSynth(Opaque w)
{
	if (theSynth == NULL)
		theSynth = new GMSynthDLL(w);
	openCount++;
}

GMSynth::~GMSynth()
{
	if (--openCount == 0)
	{
		delete theSynth;
		theSynth = 0;
	}
}

void GMSynth::SetVolume(float db, float rv)
{
	theSynth->SetVolume(db, rv);
}

void GMSynth::SetCallback(GMSYNTHCB cb, bsInt32 cbRate, void *arg)
{
	theSynth->SetCallback(cb, cbRate, arg);
}

void GMSynth::SetWaveDevice(void *dev)
{
	theSynth->SetWaveDevice(dev);
}

int GMSynth::Unload()
{
	return theSynth->Unload();
}

int GMSynth::LoadSoundBank(const char *alias, const char *fileName, int preload, float scl)
{
	return theSynth->LoadSoundBank(alias, fileName, preload, scl);
}

int GMSynth::GetMetaText(int id, char *txt, size_t len)
{
	return theSynth->GetMetaText(id, txt, len);
}

int GMSynth::GetMetaData(int id, long *vals)
{
	return theSynth->GetMetaData(id, vals);
}

int GMSynth::GetBanks(short *banks, size_t len)
{
	return theSynth->GetBanks(banks, len);
}

int GMSynth::GetPreset(short bank, short preset, char *txt, size_t len)
{
	return theSynth->GetPreset(bank, preset, txt, len);
}

int GMSynth::LoadSequence(const char *fileName, const char *sbnkName, unsigned short mask)
{
	return theSynth->LoadSequence(fileName, sbnkName, mask);
}

int GMSynth::Start(int mode, void *dev)
{
	theSynth->SetWaveDevice(dev);
	return theSynth->Start(mode);
}

int GMSynth::Stop()
{
	return theSynth->Stop();
}

int GMSynth::Pause()
{
	return theSynth->Pause();
}
int GMSynth::Resume()
{
	return theSynth->Resume();
}

int GMSynth::Generate(const char *fileName)
{
	return theSynth->Generate(fileName);
}

int GMSynth::MidiIn(int onoff, int device)
{
	return theSynth->MidiIn(onoff, device);
}

void GMSynth::ImmediateEvent(short mmsg, short val1, short val2)
{
	return theSynth->ImmediateEvent(mmsg, val1, val2);
}


/////////////////////////////////////////////////////////////////////////
// Function call API implementation
/////////////////////////////////////////////////////////////////////////

extern "C" {

static int CheckHandle()
{
	if (theSynth == NULL)
		return 1;
	return memcmp(&theSynth->magic, &GMSYNTH_MAGIC, sizeof(GUID));
}

int EXPORT GMSynthOpen(Opaque w, bsInt32 sr)
{
	if (++openCount == 1)
	{
		theSynth = new GMSynthDLL(w);
		if (theSynth == NULL)
		{
			openCount = 0;
			return GMSYNTH_ERR_BADHANDLE;
		}
		if (sr == 0)
			sr = 44100;
		InitSynthesizer(sr);
	}
	return GMSYNTH_NOERROR;
}


int EXPORT GMSynthClose()
{
	if (openCount < 1)
		return GMSYNTH_ERR_BADHANDLE;

	if (--openCount == 0)
	{
		delete theSynth;
		theSynth = 0;
	}
	return GMSYNTH_NOERROR;
}

int EXPORT GMSynthUnload()
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;
	return theSynth->Unload();
}

int EXPORT GMSynthLoadSoundBank(const char *fileName, const char *alias, int preload, float scl)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->LoadSoundBank(alias, fileName, preload, scl);
}

int EXPORT GMSynthLoadSequence(const char *fileName, const char *sbnkName, unsigned short mask)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->LoadSequence(fileName, sbnkName, mask);
}

int EXPORT GMSynthMetaText(int id, char *txt, size_t len)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->GetMetaText(id, txt, len);
}

int EXPORT GMSynthMetaData(int id, long *vals)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->GetMetaData(id, vals);
}

int EXPORT GMSynthStart(int mode, float start, float end, void *dev)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	theSynth->SetWaveDevice(dev);
	theSynth->SetTimes(start, end);
	return theSynth->Start(mode);
}

int EXPORT GMSynthStop()
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->Stop();
}

int EXPORT GMSynthPause()
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->Pause();
}

int EXPORT GMSynthResume()
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->Resume();
}

int EXPORT GMSynthGenerate(const char *fileName)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->Generate(fileName);
}

int EXPORT GMSynthSetCallback(GMSYNTHCB cb, bsUint32 cbRate, void *arg)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	theSynth->SetCallback(cb, cbRate, arg);
	return 0;
}

int EXPORT GMSynthMIDIKbdIn(int onoff, int device)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;

	return theSynth->MidiIn(onoff, device);
}

int EXPORT GMSynthMIDIEvent(short mmsg, short val1, short val2)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;
	theSynth->ImmediateEvent(mmsg, val1, val2);
	return 0;
}

int EXPORT GMSynthSetVolume(float db, float rv)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;
	theSynth->SetVolume(db, rv);
	return 0;
}

/// Return a list of valid bank numbers.
/// Pass in a NULL for banks to get the maximum number.
int EXPORT GMSynthGetBanks(short *banks, size_t len)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;
	return theSynth->GetBanks(banks, len);
}

int EXPORT GMSynthGetPreset(short bank, short preset, char *txt, size_t len)
{
	if (CheckHandle())
		return GMSYNTH_ERR_BADHANDLE;
	return theSynth->GetPreset(bank, preset, txt, len);
}

// TODO:
// Add to sequence
// Enumerate sequence
// Start/stop track

// end extern "C"
}

/////////////////////////////////////////////////////////////////////////
// Component implementation
/////////////////////////////////////////////////////////////////////////

void GMSynthDLL::tickCB(bsInt32 cnt, void *arg)
{
	((GMSynthDLL*)arg)->OnTick(cnt);
}

void GMSynthDLL::eventCB(bsUint32 tick, bsInt16 evtID, const SeqEvent *evt, void *usr)
{
	((GMSynthDLL*)usr)->OnEvent(evtID, evt);
}

void GMSynthDLL::OnTick(bsInt32 cnt)
{
	if (usrCB)
		usrCB(GMSYNTH_EVENT_TICK, cnt, usrArg);
}

void GMSynthDLL::OnEvent(bsInt16 evtid, const SeqEvent *evt)
{
	if (usrCB)
	{
		NoteEvent *nevt;
//		ControlEvent *cevt;
		TrackEvent *tevt;
		bsInt16 uevt = 0;
		bsInt32 val = 0;
		switch (evtid)
		{
		case SEQEVT_SEQSTART:
			uevt = GMSYNTH_EVENT_START;
			break;
		case SEQEVT_SEQSTOP:
			uevt = GMSYNTH_EVENT_STOP;
			break;
		case SEQEVT_SEQPAUSE:
			uevt = GMSYNTH_EVENT_PAUSE;
			break;
		case SEQEVT_SEQRESUME:
			uevt = GMSYNTH_EVENT_RESUME;
			break;
		case SEQEVT_START:
			nevt = (NoteEvent *)evt;
			uevt = GMSYNTH_EVENT_NOTEON;
			val = 0x900000 | (nevt->chnl << 16) | ((nevt->pitch+12)<<8) | nevt->noteonvel;
			break;
		case SEQEVT_STOP:
			nevt = (NoteEvent *)evt;
			uevt = GMSYNTH_EVENT_NOTEOFF;
			val = 0x800000 | (nevt->chnl << 16) | ((nevt->pitch+12)<<8);
			break;
		case SEQEVT_STARTTRACK:
			tevt = (TrackEvent *)evt;
			uevt = GMSYNTH_EVENT_TRKON;
			val = tevt->trkNo;
			break;
		case SEQEVT_STOPTRACK:
			tevt = (TrackEvent *)evt;
			uevt = GMSYNTH_EVENT_TRKOFF;
			val = tevt->trkNo;
			break;
		//case SEQEVT_CONTROL:
		//	uevt = GMSYNTH_EVENT_CTLCHG;
		//	cevt = (ControlEvent *)evt;
		//	val = (cevt->mmsg | cevt->chnl) << 16) | (cevt->ctrl << 8) | cevt->cval;
		//	break;
		case SEQEVT_PARAM:
		case SEQEVT_RESTART:
			return;
        default:
            return;
		}
		usrCB(uevt, val, usrArg);
	}
}

int GMSynthDLL::Unload()
{
	midFile.Reset();
	SoundBank::SoundBankList.DeleteBankList();
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::LoadSoundBank(const char *alias, const char *fileName, int preload, float scl)
{
	SoundBank *sb = 0;
	sb = SoundBank::SoundBankList.FindBank(alias);
	if (sb != NULL)
	{
		sb->Unlock();
		sb = 0;
	}
	if (SFFile::IsSF2File(fileName))
	{
		SFFile file;
		sb = file.LoadSoundBank(fileName, preload);
	}
	else if (DLSFile::IsDLSFile(fileName))
	{
		DLSFile file;
		sb = file.LoadSoundBank(fileName, preload);
	}
	else
		return GMSYNTH_ERR_FILETYPE;

	if (sb == NULL)
		return GMSYNTH_ERR_FILEOPEN;

	sb->Lock();
	sb->name = alias;
	SoundBank::SoundBankList.Insert(sb);
	inmgr.SetSoundBank(sb, scl);
	sbnk = sb;
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::LoadSequence(const char *fileName, const char *sbnkName, short mask)
{
	if (sbnkName)
	{
		SoundBank *sb = SoundBank::SoundBankList.FindBank(sbnkName);
		if (sb == 0)
			return GMSYNTH_ERR_SOUNDBANK;
		sbnk = sb;
	}
	else if (sbnk == 0)
		return GMSYNTH_ERR_SOUNDBANK;
	//inmgr.SetSoundBank(sbnk, 1.0);

	Stop();
	seq.Reset();
	midFile.Reset();
	if (midFile.LoadFile(fileName))
		return GMSYNTH_ERR_FILEOPEN;


	InstrConfig *inc = inmgr.FindInstr((bsInt16)1);
	SMFInstrMap map[16];
	for (int i = 0; i < 16; i++)
	{
		map[i].inc = inc;
		map[i].bnkParam = -1;
		map[i].preParam = -1;
	}

	if (midFile.GenerateSeq(&seq, map, sbnk, mask))
		return GMSYNTH_ERR_GENERATE;

	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Start(int mode)
{
	Stop();
	switch (mode)
	{
	case GMSYNTH_MODE_PLAY:
		seqMode = seqPlay;
		break;
	case GMSYNTH_MODE_SEQUENCE:
		seqMode = seqSeqOnce;
		break;
	case GMSYNTH_MODE_SEQPLAY:
		seqMode = seqPlaySeqOnce;
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}
	live = 1;
	return StartThread();
}

int GMSynthDLL::Stop()
{
	SeqState wasRunning = seq.GetState();
	//if (wasRunning != seqOff)
		seq.Halt();
	WaitThread();
	seqMode = seqOff;
	return wasRunning != seqOff;
}

int GMSynthDLL::Pause()
{
	SeqState st = seq.GetState();
	if (st == seqOff)
		return GMSYNTH_ERR_BADID;

	if (st != seqPaused)
	{
		seq.Pause();
		// Don't stop the wave output until the sequencer is paused
		// or the sequencer can get stuck attempting to write
		// to the DirectSound/ALSA buffer!
		while (seq.GetState() != seqPaused)
			ShortWait();
		wvd.Stop();
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Resume()
{
	SeqState st = seq.GetState();
	if (st == seqOff)
		return GMSYNTH_ERR_BADID;
	if (st == seqPaused)
	{
		wvd.Restart();
		seq.Resume();
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::ThreadProc()
{
	if (live)
	{
		if (seqMode & seqPlay)
			ldTm = 0.02;
		else
			ldTm = 0.20;
		OpenWaveDevice();
		inmgr.SetWaveOut(&wvd);
	}
	else
	{
		if (wvf.OpenWaveFile(outFileName, 2))
		{
			OnEvent(SEQEVT_SEQSTOP, NULL);
			return GMSYNTH_ERR_FILEOPEN;
		}
		inmgr.SetWaveOut(&wvf);
	}
	inmgr.Reset();
	seq.SequenceMulti(inmgr, stTime, endTime, seqMode);
	if (live)
	{
		bsInt32 drain = (bsInt32) (synthParams.sampleRate * (ldTm * 4));
		while (--drain > 0)
			inmgr.Tick();
		CloseWaveDevice();
	}
	else
		wvf.CloseWaveFile();
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::Generate(const char *fileName)
{
	if (fileName == NULL)
		return GMSYNTH_ERR_FILEOPEN;
	outFileName = fileName;
	Stop();
	seqMode = seqSeqOnce;
	live = 0;
	return StartThread();
}

int GMSynthDLL::GetMetaText(int id, char *txt, size_t len)
{
	*txt = '\0';

	switch (id)
	{
	case GMSYNTH_META_SEQTEXT:
		strncpy(txt, midFile.MetaText(), len);
		break;
	case GMSYNTH_META_SEQCPYR:
		strncpy(txt, midFile.Copyright(), len);
		break;
	case GMSYNTH_META_SEQNAME:
		strncpy(txt, midFile.SeqName(), len);
		break;
	case GMSYNTH_META_TIMESIG:
		strncpy(txt, midFile.TimeSignature(), len);
		break;
	case GMSYNTH_META_KEYSIG:
		strncpy(txt, midFile.KeySignature(), len);
		break;
	case GMSYNTH_META_SBKNAME:
		if (sbnk)
			strncpy(txt, sbnk->info.szName, len);
		break;
	case GMSYNTH_META_SBKCPYR:
		if (sbnk)
			strncpy(txt, sbnk->info.szCopyright, len);
		break;
	case GMSYNTH_META_SBKCMNT:
		if (sbnk)
			strncpy(txt, sbnk->info.szComment, len);
		break;
	case GMSYNTH_META_SBKVER:
		if (sbnk)
			snprintf(txt, len, "%d.%d.%d.%d",
				sbnk->info.wMajorFile, sbnk->info.wMinorFile,
				sbnk->info.wMajorVer, sbnk->info.wMinorVer);
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::GetMetaData(int id, long *vals)
{
	switch (id)
	{
	case GMSYNTH_META_TIMESIG:
		vals[0] = (long) midFile.timeSigNum;
		vals[1] = (long) midFile.timeSigDiv;
		vals[2] = (long) midFile.timeSigBeat;
		break;
	case GMSYNTH_META_KEYSIG:
		vals[0] = (long) midFile.keySigKey;
		vals[1] = (long) midFile.keySigMaj;
		break;
	case GMSYNTH_META_SBKVER:
		if (sbnk)
		{
			vals[0] = (long) sbnk->info.wMajorFile;
			vals[1] = (long) sbnk->info.wMinorFile;
			vals[2] = (long) sbnk->info.wMajorVer;
			vals[3] = (long) sbnk->info.wMinorVer;
		}
		else
			return GMSYNTH_ERR_SOUNDBANK;
		break;
	default:
		return GMSYNTH_ERR_BADID;
	}
	return GMSYNTH_NOERROR;
}

int GMSynthDLL::GetBanks(short *banks, size_t len)
{
	if (sbnk == 0)
		return 0;
	size_t count = 0;
	int index;
	for (index = 0; index < 129; index++)
	{
		if (sbnk->instrBank[index] != NULL)
		{
			if (banks)
			{
				if (count < len)
					banks[count] = index;
			}
			count++;
		}
	}
	return (int)count;
}

int GMSynthDLL::GetPreset(short bank, short preset, char *txt, size_t len)
{
	if (txt == NULL || len < 1)
		return GMSYNTH_ERR_BADID;
	*txt = '\0';
	if (sbnk == 0)
		return GMSYNTH_ERR_SOUNDBANK;
	SBInstr *in = sbnk->GetInstr(bank, preset, 0);
	if (in == NULL)
		return GMSYNTH_ERR_BADID;
	strncpy(txt, in->instrName, len-1);
	txt[len-1] = 0;
	return GMSYNTH_NOERROR;
}

void GMSynthDLL::ImmediateEvent(short mmsg, short val1, short val2)
{
	if ((mmsg & 0xf0) == MIDI_PRGCHG && sbnk != 0)
		sbnk->GetInstr(inmgr.GetBank(mmsg&0x0f), val1, 1);
	if (seq.GetState() != seqOff)
		kbd.MIDIInput::ReceiveMessage(mmsg, val1, val2, 0);
	else if (mmsg <= MIDI_SYSEX && (mmsg & 0xF0) > MIDI_NOTEON)
		inmgr.ProcessMessage(mmsg, val1, val2);
}

int GMSynthDLL::MidiIn(int onoff, int device)
{
	if (onoff)
	{
		kbd.SetDevice(device, 0);
		kbd.Start();
	}
	else
		kbd.Stop();
	return 0;
}
void GMSynthDLL::SetTimes(float start, float end)
{
	stTime = (bsInt32) (synthParams.sampleRate * start);
	endTime = (bsInt32) (synthParams.sampleRate * end);
}

void GMSynthDLL::SetVolume(float db, float rv)
{
	inmgr.SetVolume(pow(10, (double)db/20.0), rv);
}

void GMSynthDLL::SetCallback(GMSYNTHCB cb, bsInt32 cbRate, void *arg)
{
	usrArg = arg;
	usrCB = cb;
	if (cb != 0)
	{
		if (cbRate > 0)
			seq.SetCB(tickCB, (cbRate * synthParams.isampleRate) / 1000, this);
		else
			seq.SetCB(0, 0, 0);
		seq.SetEventCB(eventCB, this);
	}
}


////////////////////////////////////////////////////////////////

#ifdef _WIN32
void GMSynthDLL::SetWaveDevice(void *w)
{
	if (w)
		memcpy(&wavDevice, w, sizeof(GUID));
	else
		memset(&wavDevice, 0, sizeof(GUID));
}


void GMSynthDLL::OpenWaveDevice()
{
	wvd.Setup(wavWnd, ldTm, 4, &wavDevice);
}

void GMSynthDLL::CloseWaveDevice()
{
	wvd.Shutdown();
}
#endif

#ifdef UNIX

void GMSynthDLL::SetWaveDevice(void *w)
{
	if (wavDevice)
		free(wavDevice);
	if (w)
		wavDevice = strdup((char *)w);
	else
		wavDevice = 0;
}

void GMSynthDLL::OpenWaveDevice()
{
	wvd.Setup(wavDevice, ldTm, 4);
}

void GMSynthDLL::CloseWaveDevice()
{
	wvd.Shutdown();
}

#endif
