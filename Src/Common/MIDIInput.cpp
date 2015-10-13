//////////////////////////////////////////////////////////////////////
/// @file MIDIInput.cpp Midi keyboard (or other) input handler.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <SynthList.h>
#include <SynthMutex.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Mixer.h>
#include <WaveFile.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <MIDIInput.h>

MIDIInput::MIDIInput()
{
	midiOn = 0;
	midiIn = 0;
	midiThreadID = 0;
	memset(noteOn, 0, sizeof(noteOn));
	memset(noteOff, 0, sizeof(noteOff));
	devNum = 0;
	seq = 0;
	inmgr = 0;
	inum = 0;
	chnlMask = 0xffff;
	msgMask = 0x0007;
	sysMask = 0;
	seMax = 0;

#ifdef _WIN32
	ebWrite = 0;
	ebRead = 0;
	memset(eventBuf, 0, sizeof(eventBuf));
	memset(&seData, 0, sizeof(seData));
#endif
#ifdef UNIX
	seData = 0;
#endif
}

MIDIInput::~MIDIInput()
{
	Stop();
}

int MIDIInput::IsOn()
{
	return midiOn;
}

void MIDIInput::SetDevice(int device, const char *name)
{
	devNum = device;
	devName = name;
}

void MIDIInput::SetSequenceInfo(Sequencer *s, InstrManager *im)
{
	seq = s;
	inmgr = im;
}

void MIDIInput::SetInstrument(bsInt16 in)
{
	inum = in;
}

void MIDIInput::MessageFilter(bsUint16 bits)
{
	msgMask = bits;
}

void MIDIInput::ChannelFilter(bsUint16 bits)
{
	chnlMask = bits;
}

void MIDIInput::SystemFilter(bsUint16 bits)
{
	sysMask = bits;
}

bsUint16 MIDIInput::MessageFilter()
{
	return msgMask;
}

bsUint16 MIDIInput::ChannelFilter()
{
	return chnlMask;
}

bsUint16 MIDIInput::SystemFilter()
{
	return sysMask;
}

void MIDIInput::FilterMessage(bsUint16 mmsg, bsUint16 enable)
{
	if (enable)
		msgMask |= MIDIIN_MSG_BIT(mmsg);
	else
		msgMask &= ~MIDIIN_MSG_BIT(mmsg);
}

void MIDIInput::FilterChannel(bsUint16 chnl, bsUint16 enable)
{
	if (enable)
		chnlMask |= MIDIIN_CHNL_BIT(chnl);
	else
		chnlMask &= ~MIDIIN_CHNL_BIT(chnl);
}

void MIDIInput::FilterSystem(bsUint16 mmsg, bsUint16 enable)
{
	if (enable)
		sysMask |= MIDIIN_CHNL_BIT(mmsg);
	else
		sysMask &= ~MIDIIN_CHNL_BIT(mmsg);
}

void MIDIInput::SysExBuffer(bsUint32 len)
{
	seMax = len;
}

// The event IDS are calculated to allow overlapping note-on/note-off events.
// The first note off will always match the first note on for a key+channel
// value. The MSB is set on the id to distinguish it from a normal sequencer
// event ID.
void MIDIInput::ReceiveMessage(bsUint16 mmsg, bsUint16 val1, bsUint16 val2, bsUint32 ts)
{
	if (!seq || !(seq->GetState() & seqPlay) || !inmgr)
		return;

	NoteEvent *nevt;
	ControlEvent *cevt;
	bsInt16 type;
	bsInt32 id;
	int cmd = (int) (mmsg & 0xf0);
	int chnl = (int) (mmsg & 0x0f);
	switch (cmd)
	{
	case MIDI_NOTEON:
		if (val2 != 0)
		{
			type = SEQEVT_START;
			id = ++noteOn[val1];
		}
		else
		{
	case MIDI_NOTEOFF:
			type = SEQEVT_STOP;
			id = ++noteOff[val1];
		}
		id = (id & 0xff) | 0x80000000 | (chnl << 16) | (val1 << 8);
		nevt = (NoteEvent *)inmgr->ManufEvent(inum);
		nevt->SetType(type);
		nevt->SetID(id);
		nevt->SetChannel(chnl);
		nevt->SetStart(0);
		nevt->SetDuration(0);
		nevt->SetVolume((float)val2 / 127.0);
		nevt->SetPitch(val1-12);
		nevt->SetVelocity(val2);
		nevt->SetInum(inum);
		seq->AddImmediate(nevt);
		break;
	case MIDI_CTLCHG:
	case MIDI_PRGCHG:
	case MIDI_CHNAT:
	case MIDI_PWCHG:
		cevt = new ControlEvent;
		cevt->SetType(SEQEVT_CONTROL);
		cevt->SetStart(0);
		cevt->SetDuration(0);
		cevt->SetID(seq->NextEventID());
		cevt->SetChannel(chnl);
		cevt->SetMessage(cmd);
		if (cmd == MIDI_CTLCHG)
		{
			cevt->SetControl(val1);
			cevt->SetValue(val2);
		}
		else
		{
			cevt->SetControl(-1);
			cevt->SetValue(val1);
		}
		seq->AddImmediate(cevt);
		break;
	default:
		return;
	}
}

void MIDIInput::ReceiveSysEx(const bsUint8 *data, bsUint32 size, bsUint32 xrun)
{
}

#ifdef _WIN32

void CALLBACK MIDIInput::MidiCB(HMIDIIN hMidiIn, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	((MIDIInput *) dwInstance)->MidiInWin(uMsg, dwParam1, dwParam2);
}

void MIDIInput::MidiInWin(UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == MIM_LONGDATA)
	{
		MIDIHDR *mhdr = (MIDIHDR*) dwParam1;
		if (sysMask & MIDIIN_MSG_BIT(MIDI_SYSEX))
			ReceiveSysEx((bsUint8*)mhdr->lpData, mhdr->dwBytesRecorded, 0);
		if (mhdr->dwFlags & MHDR_DONE)
		{
			midiInPrepareHeader(midiIn, mhdr, sizeof(MIDIHDR));
			midiInAddBuffer(midiIn, mhdr, sizeof(MIDIHDR));
		}
		return;
	}

	if (uMsg == MIM_DATA)
	{
		// process buffered input
		while (ebRead != ebWrite)
		{
			MEvent *p = &eventBuf[ebRead++];
			if (ebRead >= 500)
				ebRead = 0;
			ReceiveMessage(p->mmsg, p->val1, p->val2, p->ts);
		}
	}

	bsUint16 mmsg = dwParam1 & 0xFF;
	if (mmsg == MIDI_ACTSNS)
		return;

	int enable;
	if ((mmsg & 0xf0) == 0xf0)
		enable = sysMask & MIDIIN_CHNL_BIT(mmsg);
	else
		enable = (msgMask & MIDIIN_MSG_BIT(mmsg)) && (chnlMask & MIDIIN_CHNL_BIT(mmsg));

	if (enable)
	{
		bsUint16 val1 = (dwParam1 >> 8) & 0x7f;
		bsUint16 val2 = (dwParam1 >> 16) & 0x7f;
		if (uMsg == MIM_DATA)
		{
			ReceiveMessage(mmsg, val1, val2, dwParam2);
		}
		else if (uMsg == MIM_MOREDATA)
		{
			// buffer this event and return immediately.
			// on the next MIM_DATA, process the buffered events
			MEvent *p = &eventBuf[ebWrite++];
			if (ebWrite >= 500)
				ebWrite = 0;
			p->mmsg = mmsg;
			p->val1 = val1;
			p->val2 = val2;
			p->ts = dwParam2;
		}
	}
}

int MIDIInput::Start()
{
	if (midiOn)
		return 0;

	midiOn = 1;
	ebWrite = 0;
	ebRead = 0;
	memset(eventBuf, 0, sizeof(eventBuf));
	memset(noteOn, 0, sizeof(noteOn));
	memset(noteOff, 0, sizeof(noteOff));
	MMRESULT err = midiInOpen(&midiIn, devNum, (DWORD_PTR) MidiCB, (DWORD_PTR) this, CALLBACK_FUNCTION|MIDI_IO_STATUS);
	if (err != MMSYSERR_NOERROR)
		return -1;
	memset(&seData, 0, sizeof(seData));
	if (seMax > 0)
	{
		seData.dwBufferLength = seMax;
		seData.lpData = new char[seMax];
		midiInPrepareHeader(midiIn, &seData, sizeof(seData));
		midiInAddBuffer(midiIn, &seData, sizeof(seData));
	}
	midiInStart(midiIn);
	return 0;
}

int MIDIInput::Stop()
{
	if (!midiOn)
		return 0;
	if (seData.dwFlags & MHDR_PREPARED)
	{
		midiInUnprepareHeader(midiIn, &seData, sizeof(MIDIHDR));
		delete seData.lpData;
		memset(&seData, 0, sizeof(MIDIHDR));
	}
	midiInStop(midiIn);
	midiInClose(midiIn);
	midiIn = 0;
	midiOn = 0;
	return 0;
}

#endif

#ifdef UNIX

void *MIDIInput::MidiInputRaw(void *param)
{
	((MIDIInput *) param)->MidiInALSA();
	return 0;
}

void MIDIInput::MidiInALSA()
{
	snd_rawmidi_drain(midiIn);

	unsigned char inb;
	unsigned char mmsg = 0;
	unsigned char val[2];
	int valCount = 0;
	bsUint32 seCnt;
	bsUint32 seXrun;

	while (1)
	{
		if (snd_rawmidi_read(midiIn, &inb, 1) < 0)
			break;
		else if (inb & 0x80)
		{
			// MIDI command byte
			if ((inb & 0xf0) == 0xf0)
			{
				unsigned char val = 0;
				switch (inb)
				{
				case MIDI_SYSEX:  //  0xF0
					seCnt = 0;
					seXrun = 0;
					if (seCnt < seMax)
						seData[seCnt++] = inb;
					while (snd_rawmidi_read(midiIn, &val, 1) > 0)
					{
						if (seCnt < seMax)
							seData[seCnt++] = val;
						else
							seXrun++;
						if (val == MIDI_ENDEX)
							break;
					}
					ReceiveSysEx(seData, seCnt, seXrun);
					break;
				case MIDI_SNGPOS: // 0xF2
					snd_rawmidi_read(midiIn, &val, 1);
				case MIDI_SNGSEL: // 0xF3
					snd_rawmidi_read(midiIn, &val, 1);
					break;
				case MIDI_TMCODE: // 0xF1
				case MIDI_TUNREQ: // 0xF6
				case MIDI_ENDEX:  // 0xF7
				case MIDI_TMCLK:  // 0xF8
				case MIDI_START:  // 0xFA
				case MIDI_CONT:   // 0xFB
				case MIDI_STOP:   // 0xFC
				case MIDI_ACTSNS: // 0xFE
				case MIDI_META:   // 0xFF
					break;
				}
				if (sysMask & MIDIIN_CHNL_BIT(inb))
					ReceiveMessage(mmsg, val, 0, 0);
			}
			else
			{
				mmsg = inb;
				valCount = 0;
			}
		}
		else if (mmsg)
		{
			val[valCount++] = inb;
			if (valCount == 2)
			{
				if ((chnlMask & MIDIIN_CHNL_BIT(mmsg)) && (msgMask & MIDIIN_MSG_BIT(mmsg)))
					ReceiveMessage(mmsg, val[0], val[1], 0);
				valCount = 0;
			}
		}
	}
}

int MIDIInput::Start()
{
	if (midiOn)
		return 0;
	memset(noteOn, 0, sizeof(noteOn));
	memset(noteOff, 0, sizeof(noteOff));
	char name[80];
	if (devName.Length() == 0)
		sprintf(name, "hw:%d", devNum);
	else
		strncpy(name, devName, 80);
	if (snd_rawmidi_open(&midiIn, NULL, name, 0) < 0)
		return -1;
	pthread_create(&midiThreadID, NULL, MidiInputRaw, this);
	midiOn = 1;
	return 0;
}

int MIDIInput::Stop()
{
	if (!midiOn)
		return 0;
	pthread_cancel(midiThreadID);
	pthread_join(midiThreadID, NULL);
	snd_rawmidi_drain(midiIn);
	snd_rawmidi_close(midiIn);
	midiIn = 0;
	midiOn = 0;
	return 0;
}

#endif
