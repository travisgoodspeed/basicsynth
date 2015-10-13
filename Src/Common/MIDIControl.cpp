/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDIControl.cpp MIDI Control classes implementation
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <SynthMutex.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <GenWaveWT.h>

MIDIChannelStatus::MIDIChannelStatus()
{
	channel = 0;
	Reset();
}

void MIDIChannelStatus::Reset()
{
	memset(no, 0, sizeof(no));
	memset(cc, 0, sizeof(cc));
	memset(at, 0, sizeof(at));
	bank = channel == 9 ? 128 : 0;
	patch = 0;
	aftertouch = 0;
	pitchbend = 0;
	pbsemi = 2;
	pbcents = 0;
	mwsemi = 0;
	mwcents = 50;
	finetune = 0;
	coarsetune = 0;
	rpnnum = 0x7f7f;
	enable = 1;
	cc[MIDI_CTRL_BANK] = channel == 9 ? 0x78 : 0x79;
	cc[MIDI_CTRL_VOL] = 100;
	cc[MIDI_CTRL_EXPR] = 127;
	cc[MIDI_CTRL_PAN] = 64;
}

MIDIControl::MIDIControl()
{
	switchLevel = 64;
	for (int ch = 0; ch < 16; ch++)
		channel[ch].channel = ch;
	channel[9].cc[MIDI_CTRL_BANK] = 0x78;
	channel[9].bank = 128;
}

void MIDIControl::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	ControlEvent *cevt = (ControlEvent *)evt;
	ProcessMessage(cevt->mmsg, cevt->ctrl, cevt->cval);
}

void MIDIControl::ProcessMessage(short mm, short v1, short v2)
{
	short msg = mm & 0xf0;
	short chnl = mm & 0x0f;

	if (msg == 0xf0)
	{
		switch (mm)
		{
		case MIDI_SYSEX:
			// TODO: handle universal SYSEX messages
			break;
		}
	}
	else
	{
		MIDIChannelStatus *st = &channel[chnl];
		switch (msg)
		{
		case MIDI_NOTEON:
			if (v1 != 0)
			{
				st->no[v1]++;
				NoteOn(chnl, v1, v2);
				break;
			}
			/*FALLTHROUGH*/
		case MIDI_NOTEOFF:
			st->no[v1]--;
			NoteOff(chnl, v1, v2);
			break;
		case MIDI_CTLCHG:
			st->cc[v1] = v2;
			switch (v1)
			{
			case MIDI_CTRL_BANK:
			case MIDI_CTRL_BANK_LSB:
				if (st->cc[MIDI_CTRL_BANK] == 0x78) // GM2 percussion bank
					st->bank = 128;
				else if (st->cc[MIDI_CTRL_BANK] == 0x79) // GM2 melodic bank
					st->bank = st->cc[MIDI_CTRL_BANK_LSB];
				else if (chnl == 9)
					st->bank = 128;
				else
					st->bank = 0;
				break;
			case MIDI_CTRL_DATA:
				switch (st->rpnnum)
				{
				case 0:
					st->pbsemi = v2;
					break;
				case 1:
					st->finetune = (st->finetune & 0x007f) | (v2 << 7);
					break;
				case 2:
					st->coarsetune = 64 - v2;
					break;
				case 3:
					st->mwsemi = v2;
					break;
				}
				break;
			case MIDI_CTRL_DATA_LSB:
				switch (st->rpnnum)
				{
				case 0:
					st->pbcents = v2;
					break;
				case 1:
					st->finetune = (st->finetune & 0x007f) | (v2 << 7);
					break;
				case 3:
					st->mwcents = v2;
					break;
				}
				break;
			case MIDI_CTRL_RPNLSB:
				st->rpnnum = (st->rpnnum & 0x3F80) | v2;
				break;
			case MIDI_CTRL_RPNMSB:
				st->rpnnum = (st->rpnnum & 0x007F) | (v2 << 7);
				break;
			}
			ControlChange(chnl, v1, v2);
			break;
		case MIDI_PRGCHG:
			st->patch = v2;
			break;
		case MIDI_KEYAT:
			st->at[v1] = v2;
			break;
		case MIDI_CHNAT:
			st->aftertouch = v1;
			AftertouchChange(chnl, v1);
			break;
		case MIDI_PWCHG:
			st->pitchbend = (v2 << 7) | v1;
			PitchbendChange(chnl, v1, v2);
			break;
		}
	}
}

