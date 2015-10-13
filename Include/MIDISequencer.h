/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDISequencer.h MIDI Sequencer Classes
/// @deprecated The MIDISequencer and associated classes are no longer needed.
/// Since they are referenced in the BasicSynth book, they are included for reference.
/// The main Sequencer and Instrument Manager now handle MIDI functions.
/// Use SMFFile to load the sequence and a GMPlayer to play GM sound fonts.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef _MIDISEQUENCE_H_
#define _MIDISEQUENCE_H_

#include <MIDIDefs.h>

/// A MIDI event.
/// The MIDI event holds information needed
/// for each event. There are never more than
/// two parameters for an event.
class MIDIEvent : public SynthList<MIDIEvent>
{
public:
	bsUint32 deltat; // deltaTime
	bsUint16 event;
	bsUint16 chan;  // channel or meta
	bsUint16 val1;
	bsUint16 val2;
	bsUint32 lval;
};

/// MIDI instrument interface.
/// Similar to Instrument, but adds MIDI specific methods
/// to handle channel messages.
class MIDIInstrument
{
public:
	virtual ~MIDIInstrument() { }
	virtual void NoteOn(short key, short vel)  = 0;
	virtual void NoteOff(short key, short vel) = 0;
	virtual void ControlChange(short ctl, short val) = 0;
	virtual void AfterTouch(short val) = 0;
	virtual void PitchBend(short val) = 0;
	virtual int IsFinished() = 0;
	virtual void Tick(Mixer *mix, short chnl) = 0;
	virtual void Destroy() { delete this; }
};

/// Event for MIDI notes.
class MIDINote : public SynthList<MIDINote>
{
public:
	bsUint16 key;
	bsUint16 ison;
	bsUint16 chnl;
	MIDIInstrument *ip;
};

/// MIDI channel.
/// This class manages a set of sounding notes on an instrument.
class MIDIChannel
{
public:
	short patch;
	short chnlNum;
	short modval;
	short volval;
	short panval;
	short exprval;
	short pbval;
	MIDINote *head;

	MIDIChannel(short ch)
	{
		patch = 0;
		chnlNum = ch;
		volval = 64;
		panval = 0;
		modval = 0;
		exprval = 0;
		pbval = 0;
		head = new MIDINote;
	}

	virtual ~MIDIChannel()
	{
		delete head;
	}

	void SetTrack(int t)
	{
	}

	virtual MIDIInstrument *Allocate(int trk) = 0;
	virtual void Deallocate(MIDIInstrument *) = 0;

	void NoteOn(short trk, short key, short vel);
	void NoteOff(short key, short vel);
	void ProgChange(short val)
	{
		patch = val;
	}
	void ControlChange(short ctl, short val);
	void AfterTouch(short key, short val);
	void PitchBend(short val);
	int Tick(Mixer *mix);

};

class MIDISequence;

/// MIDI track.
/// This is the class that maintains a list of MIDI events for playback.
class MIDITrack : public SynthList<MIDITrack>
{
private:
	MIDIEvent *evtHead;
	MIDIEvent *evtTail;
	MIDIEvent *evtLast;
	int eot;
	int loopSet;
	int loopCount;
	int trkNum;

	bsUint32 deltaTime;

public:
	MIDITrack(int t)
	{
		evtHead = new MIDIEvent;
		evtTail = new MIDIEvent;
		evtHead->Insert(evtTail);
		evtLast = evtHead;
		evtHead->deltat = 0;
		evtHead->event = 0;
		evtTail->deltat = 0;
		evtTail->event = MIDI_META;
		evtTail->chan  = MIDI_META_EOT;
		eot = 1;
		loopSet = 0;
		loopCount = 0;
		trkNum = t;
	}

	// Add the event to the sequence. The caller is responsible for setting
	// valid values for inum, start, duration, type and eventid.
	void AddEvent(MIDIEvent *evt)
	{
		if (evtLast == NULL)
			evtLast = evtHead;
		if (evt)
		{
			evtLast->Insert(evt);
			evtLast = evt;
		}
	}

	void SetLoop(int count)
	{
		loopSet = count;
	}

	int Start(MIDISequence *seq);
	int Play(MIDISequence *seq);
};

/// MIDI Sequencer.
/// This is intended as a prototype/example class. A complete
/// implementation needs more sophisticated processing of CC#s
/// as well as SMPTE, standard MIDI instruments, META, etc.
class MIDISequence
{
private:
	Mixer mix;
	MIDIChannel *instr[MAX_MIDI_CHNL];
	MIDITrack   *trcks;
	int numTrk;
	int chnlMask;
	long theTick;
	double srTicks;
	double ppqn;

public:
	MIDISequence()
	{
		trcks = NULL;
		numTrk = 0;
		mix.SetChannels(MAX_MIDI_CHNL);
		for (int i = 0; i < MAX_MIDI_CHNL; i++)
		{
			mix.ChannelOn(i, 1);
			mix.ChannelVolume(i, 0.7);
			instr[i] = NULL;
		}
		mix.MasterVolume(0.5, 0.5);
		ppqn = 24.0e6;
		srTicks = (0.5 * synthParams.sampleRate) / 24.0;
	}

	~MIDISequence()
	{
	}

	MIDITrack *AddTrack(int trkN)
	{
		MIDITrack *newTrack = new MIDITrack(trkN);
		if (trcks == NULL)
			trcks = newTrack;
		else
		{
			MIDITrack *p = trcks;
			while (p->next)
				p = p->next;
			p->Insert(newTrack);
		}
		return newTrack;
	}

	void MasterVolume(AmpValue v)
	{
		mix.MasterVolume(v, v);
	}

	void SetChannels(bsUint32 chnlMask)
	{
		int cnt = 0;
		bsUint32 bit = 1;
		for (int i = 0; i < MAX_MIDI_CHNL; i++)
		{
			int on = (chnlMask & bit) ? 1 : 0;
			mix.ChannelOn(i, on);
			bit <<= 1;
			cnt += on;
		}
	}

	void SetChannelMgr(short chan, MIDIChannel *mgr)
	{
		instr[chan] = mgr;
	}

	void SetPPQN(bsUint16 val)
	{
		ppqn = ((double) val * 1.0e6);
	}

	void Sequence(WaveOutBuf *out);

	void NoteOn(short trk, short chnl, short key, short vel)
	{
	//	printf("%ld: Note ON: %d %d %d\n", theTick, chnl, key, vel);

		if (instr[chnl] != NULL)
			instr[chnl]->NoteOn(trk, key, vel);
	}

	void NoteOff(short chnl, short key, short vel)
	{
	//	printf("%ld: Note OFF: %d %d %d\n", theTick, chnl, key, vel);
		if (instr[chnl] != NULL)
			instr[chnl]->NoteOff(key, vel);
	}

	void Tempo(long val)
	{
		srTicks = ((FrqValue)val * synthParams.sampleRate) / ppqn;
	}

	void ProgChange(short chnl, short val)
	{
		if (instr[chnl] != NULL)
			instr[chnl]->ProgChange(val);
	}

	void ControlChange(short chnl, short ctl, short val)
	{
		switch(ctl)
		{
		case MIDI_CTRL_VOL:
		//	printf("%ld: Volume: %d %d\n", theTick, chnl, val);
			mix.ChannelVolume(chnl, (AmpValue) val / 127.0);
			break;
		case MIDI_CTRL_PAN:
		//	printf("%ld: Pan: %d %d\n", theTick, chnl, val);
			mix.ChannelPan(chnl, 0, (AmpValue) (val - 64) / 64.0);
			break;
		}
		if (instr[chnl] != NULL)
			instr[chnl]->ControlChange(ctl, val);
	}

	void KeyAfterTouch(short chnl, short key, short val)
	{
		if (instr[chnl] != NULL)
			instr[chnl]->AfterTouch(key, val);
	}

	void ChannelAfterTouch(short chnl, short val)
	{
		if (instr[chnl] != NULL)
			instr[chnl]->AfterTouch(-1, val);
	}

	void PitchBend(short chnl, short val)
	{
		//printf("%ld: Bend: %d %d\n", theTick, chnl, val);
		if (instr[chnl] != NULL)
			instr[chnl]->PitchBend(val);
	}
};


/// MIDI header chunk
struct MTHDchunk 
{
	bsUint8  chnkID[4]; // MThd
	bsUint32 size;
	bsUint16 format;
	bsUint16 numTrk;
	bsUint16 tmDiv;
};


/// Load a standard MIDI file for sequencing.
/// @deprecated The MIDIFileLoad class is superseded. Use SMFFIle instead.
class MIDIFileLoad
{
protected:
	MIDISequence *seq;
	FileReadBuf fp;
	MTHDchunk hdr;
	char* metaText;
	char* metaCpyr;
	char* metaSeqName;
	char timeSig[20];
	char keySig[20];
	bsUint8 *inpBuf;
	bsUint8 *inpPos;
	bsUint8 *inpEnd;
	bsUint16 lastMsg;
	bsUint16 trkNum;
	bsUint32 chnlMask;
	bsUint32 deltaT;
	MIDITrack *trackObj;

	char *IntToStr(int val, char *s);

public:
	MIDIFileLoad()
	{
		hdr.format = 1;
		hdr.numTrk = 0;
		hdr.tmDiv = 24;
		chnlMask = 0;
		seq = NULL;
		chnlMask = 0;
		metaText = NULL;
		metaCpyr = NULL;
		metaSeqName = NULL;
		timeSig[0] = 0;
		keySig[0] = 0;
		trackObj = NULL;
	}

	~MIDIFileLoad()
	{
		if (metaText)
			delete metaText;
		if (metaCpyr)
			delete metaCpyr;
		if (metaSeqName)
			delete metaSeqName;
	}

	const char *MetaText()
	{
		return metaText;
	}

	const char *Copyright()
	{
		return metaCpyr;
	}

	const char *SeqName()
	{
		return metaSeqName;
	}

	const char *TimeSignature()
	{
		return timeSig;
	}

	const char *KeySignature()
	{
		return keySig;
	}

	void SetSequencer(MIDISequence *p)
	{
		seq = p;
	}

	int LoadFile(char *file);

protected:
	void MetaEvent();
	void SysCommon(bsUint16 msg);
	void ChnlMessage(bsUint16 msg);
	char *CopyString(bsUint32 len, char *text = NULL);
	
	bsUint32 ReadLong()
	{
		return ((bsUint32) ReadShort() << 16) + (bsUint32) ReadShort();
	}

	bsUint16 ReadShort()
	{
		return (bsUint16) (fp.ReadCh() << 8) + fp.ReadCh();
	}

	bsUint32 GetVarLen()
	{
		bsUint32 value = 0;
		do
			value = (value << 7) + (bsUint32) (*inpPos & 0x7F);
		while (*inpPos++ & 0x80);
		return value;
	}

	void CloseFile()
	{
		fp.FileClose();
	}
};
//@}
#endif
