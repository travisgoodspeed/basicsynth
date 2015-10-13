//////////////////////////////////////////////////////////////////
/// @file MIDISequencer.cpp Example sequencer for MIDI files.
//
// BasicSynth
//
// MIDI Sequencer code that is not inline
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>
#include <Instrument.h>
//#include <Sequencer.h>
#include <MIDISequencer.h>

void MIDIChannel::NoteOn(short trk, short key, short vel)
{
	MIDINote *np = new MIDINote;
	head->Insert(np);
	np->key = key;
	np->chnl = chnlNum;
	np->ison = 1;
	np->ip = Allocate(trk);
	np->ip->NoteOn(key, vel);
	if (exprval != 0)
		np->ip->ControlChange(MIDI_CTRL_EXPR, exprval);
	if (modval != 0)
		np->ip->ControlChange(MIDI_CTRL_MOD, exprval);
	if (pbval != 0)
		np->ip->PitchBend(pbval);
}

void MIDIChannel::NoteOff(short key, short vel)
{
	MIDINote *np = head->next;
	while (np != NULL)
	{
		if (np->key == key)
		{
			np->ison = 0;
			np->ip->NoteOff(key, vel);
			//break;
		}
		np = np->next;
	}
}

void MIDIChannel::ControlChange(short ctl, short val)
{
	switch (ctl)
	{
	case MIDI_CTRL_MOD:
		modval = val;
		break;
	case MIDI_CTRL_VOL:
		volval = val;
		break;
	case MIDI_CTRL_PAN:
		panval = val;
		break;
	case MIDI_CTRL_EXPR:
		exprval = val;
		break;
	}

	MIDINote *np = head->next;
	while (np != NULL)
	{
		np->ip->ControlChange(ctl, val);
		np = np->next;
	}
}

int MIDITrack::Start(MIDISequence *seq)
{
	evtLast = evtHead->next;
	if (evtLast == evtTail)
		return 0;
	deltaTime = evtLast->deltat;
	eot = false;
	loopCount = loopSet;
	return 1;
}

int MIDITrack::Play(MIDISequence *seq)
{
	if (eot)
		return 0;
	while (deltaTime == 0)
	{
		short chan = evtLast->chan;
		short val1 = evtLast->val1;
		short val2 = evtLast->val2;
		switch (evtLast->event)
		{
		case MIDI_NOTEON:
			if (evtLast->val2 != 0)
			{
				seq->NoteOn(trkNum, chan, val1, val2);
				break;
			}
			// FALLTHROUGH
		case MIDI_NOTEOFF:
			seq->NoteOff(chan, val1, val2);
			break;
		case MIDI_KEYAT:
			seq->KeyAfterTouch(chan, val1, val2);
			break;
		case MIDI_CTLCHG:
			seq->ControlChange(chan, val1, val2);
			break;
		case MIDI_PRGCHG:
			seq->ProgChange(chan, val1);
			break;
		case MIDI_CHNAT:
			seq->ChannelAfterTouch(chan, val1);
			break;
		case MIDI_PWCHG:
			seq->PitchBend(chan, val1);
			break;
		case MIDI_META:
			if (chan == MIDI_META_EOT)
			{
				if (--loopCount <= 0)
				{
					eot = true;
					return 0;
				}
				evtLast = evtHead;
			}
			else if (chan == MIDI_META_TMPO)
				seq->Tempo(evtLast->lval);
			break;
		}
		evtLast = evtLast->next;
		deltaTime = evtLast->deltat;
	}
	deltaTime--;
	return 1;
}

void MIDIChannel::AfterTouch(short key, short val)
{
	MIDINote *np = head->next;
	while (np != NULL)
	{
		if (key == -1 || key == np->key)
			np->ip->AfterTouch(val);
		np = np->next;
	}
}

void MIDIChannel::PitchBend(short val)
{
	pbval = val;
	MIDINote *np = head->next;
	while (np != NULL)
	{
		np->ip->PitchBend(val);
		np = np->next;
	}
}

int MIDIChannel::Tick(Mixer *mix)
{
	int count = 0;
	MIDINote *np = head->next;
	while (np != NULL)
	{
		np->ip->Tick(mix, chnlNum);
		if (!np->ison)
		{
			if (np->ip->IsFinished())
			{
				//printf("%ld: Remove %d\n", theTick, np->key);
				MIDINote *tmp = np;
				np = np->Remove();
				delete tmp->ip;
				delete tmp;
			}
			else
			{
				count++;
				np = np->next;
			}
		}
		else
		{
			count++;
			np = np->next;
		}
	}
	return count;
}

int MIDIFileLoad::LoadFile(char *file)
{
	if (seq == NULL)
		return -1;

	CloseFile();

	if (fp.FileOpen(file))
		return -1;

	bsUint32 trkSize;
	if (fp.FileRead(hdr.chnkID, 4) != 4
	 || memcmp(hdr.chnkID, "MThd", 4) != 0)
	{
		CloseFile();
		return -1;
	}

	hdr.size = ReadLong();
	if (hdr.size != 6)
	{
		CloseFile();
		return -1;
	}

	hdr.format = ReadShort();
	hdr.numTrk = ReadShort();
	hdr.tmDiv = ReadShort();
	if (hdr.tmDiv & 0x8000)
	{
		// SMTPE - punt for now
		CloseFile();
		return -1;
	}

	seq->SetPPQN(hdr.tmDiv);

	bsUint8 chnkID[4];
	int err = 0;
	trkNum = 0;
	while (trkNum < hdr.numTrk)
	{
		if (fp.FileRead(chnkID, 4) != 4)
		{
			err = -1;
			break;
		}
		trkSize = ReadLong();
		if (memcmp(chnkID, "MTrk", 4) != 0)
		{
			// Can this happen?
			fp.FileSkip(trkSize);
			continue;
		}

		//printf("Read Track %d of size %ld\n", trkNum, trkSize);
		inpBuf = new bsUint8[trkSize];
		if (inpBuf == NULL)
		{
			err = -1;
			break;
		}
		if ((bsUint32)fp.FileRead(inpBuf, trkSize) != trkSize)
		{
			err = -1;
			break;
		}
		inpPos = inpBuf;
		inpEnd = inpBuf + trkSize;
		lastMsg = 0;
		trackObj = seq->AddTrack(trkNum);

		while (inpPos < inpEnd)
		{
			deltaT = GetVarLen();
			bsUint16 msg = *inpPos;
			if ((msg & MIDI_EVTMSK) == MIDI_SYSEX)
			{
				inpPos++;
				if (msg == MIDI_META)
					MetaEvent();
				else
					SysCommon(msg);
			}
			else
			{
				if (msg & MIDI_MSGBIT)
				{
					inpPos++;
					lastMsg = msg;
				}
				else
					msg = lastMsg;
				ChnlMessage(msg);
			}
		}

		delete inpBuf;
		inpBuf = NULL;
		inpPos = NULL;
		inpEnd = NULL;
		trkNum++;
	}
	seq->SetChannels(chnlMask);

	CloseFile();
	return err;
}

void MIDIFileLoad::MetaEvent()
{
	static const char *flats[]  = {"C", "F", "Bb", "Eb", "Ab", "Db", "Gb", "Cb"};
	static const char *sharps[] = {"C", "G", "D",  "A",  "E",  "B",  "F#", "C#"};
	MIDIEvent *evt;

	bsUint16 meta = *inpPos++;
	bsUint32 metaLen = GetVarLen();

	bsUint16 val, val2, val3, val4;
	char *fmt;

	//printf("Meta event %02x, len=%d on track %d\n", meta, metaLen, trkNum);

	switch (meta)
	{
	case MIDI_META_TEXT:
		metaText = CopyString(metaLen, metaText);
		//printf("%s\n", metaText);
		break;
	case MIDI_META_CPYR:
		metaCpyr = CopyString(metaLen, metaCpyr);
		//printf("Copyright: %s\n", metaCpyr);
		break;
	case MIDI_META_TRK:
		metaSeqName = CopyString(metaLen, metaSeqName);
		//printf("Track: %s\n", metaSeqName);
		break;
	case MIDI_META_TMSG:
		val  = *inpPos++;
		val2 = *inpPos++;
		val3 = *inpPos++;
		val4 = *inpPos++;
		fmt = IntToStr(val, timeSig);
		*fmt++ = '/';
		fmt = IntToStr(val2, fmt);
		*fmt++ = ' ';
		*fmt++ = '(';
		fmt = IntToStr(val3, fmt);
		*fmt++ = ')';
		*fmt++ = ' ';
		fmt = IntToStr(val4, fmt);
		*fmt = '\0';
		//printf("Time Signature: %s\n", timeSig);
		break;
	case MIDI_META_KYSG:
		val  = *inpPos++;
		val2 = *inpPos++;
		strcpy(keySig, (val & 0x80) ? flats[(256-val)&7] : sharps[val&7]);
		strcat(keySig, " ");
		strcat(keySig, val2 ? "min." : "maj.");
		//printf("Key Signature: %s\n", keySig);
		break;
	case MIDI_META_EOT:
		// The TAIL of the event list already has the EOT meta event.
		evt = new MIDIEvent;
		evt->deltat = deltaT;
		evt->event = MIDI_META;
		evt->chan = MIDI_META_EOT;
		trackObj->AddEvent(evt);
		break;
	case MIDI_META_TMPO:
		evt = new MIDIEvent;
		evt->deltat = (bsUint32) deltaT;
		evt->event = MIDI_META;
		evt->chan = MIDI_META_TMPO;
		evt->lval = *inpPos++;
		evt->lval = (evt->lval << 8) + *inpPos++;
		evt->lval = (evt->lval << 8) + *inpPos++;
		trackObj->AddEvent(evt);
		break;
	case MIDI_META_INST:
	case MIDI_META_LYRK:
	case MIDI_META_MRKR:
	case MIDI_META_CUE:
	case MIDI_META_CHNL:
	default:
		//printf("Meta event %02x, len=%d on track %d\n", meta, metaLen, trkNum);

		inpPos += metaLen;
		break;
	}
}

void MIDIFileLoad::SysCommon(bsUint16 msg)
{
	//printf("Sys Common %02x on track %d\n", msg, trkNum);

	bsUint32 dataLen;

	switch (msg)
	{
	case MIDI_SYSEX:
		dataLen = GetVarLen();
		inpPos += dataLen;
		break;
	case MIDI_SNGPOS:
		inpPos += 2;
		break;
	case MIDI_TMCODE:
	case MIDI_SNGSEL:
		inpPos += 1;
		break;
	case MIDI_TUNREQ:
	case MIDI_ENDEX:
	case MIDI_TMCLK:
	case MIDI_START:
	case MIDI_CONT:
	case MIDI_STOP:
	case MIDI_ACTSNS:
		break;
	default:
		//printf("SysCommon: %02x\n", msg);
		break;
	}
}

void MIDIFileLoad::ChnlMessage(bsUint16 msg)
{
	MIDIEvent *evt = new MIDIEvent;
	evt->deltat = deltaT;
	evt->event = msg & MIDI_EVTMSK;
	evt->chan  = msg & MIDI_CHNMSK;

	chnlMask |= (1 << evt->chan);

	//printf("%ld: Channel message %02x on track %d\n", deltaT, msg, trkNum);

	switch (msg & MIDI_EVTMSK)
	{
	case MIDI_NOTEOFF:
	case MIDI_NOTEON:
	case MIDI_KEYAT:
	case MIDI_CTLCHG:
		evt->val1 = *inpPos++;
		evt->val2 = *inpPos++;
		//printf(" values %d %d\n", evt->val1, evt->val2);
		break;
	case MIDI_PRGCHG:
	case MIDI_CHNAT:
		evt->val1 = *inpPos++;
		//printf(" value %d\n", evt->val1);
		break;
	case MIDI_PWCHG:
		evt->val1 = *inpPos++;
		evt->val2 = *inpPos++;
		evt->val1 += evt->val2 << 7;
		evt->val1 -= 8192;
		//printf(" value %d\n", evt->val1);
		break;
	default:
		delete evt;
		return;
	}

	trackObj->AddEvent(evt);
}

char *MIDIFileLoad::CopyString(bsUint32 len, char *text)
{
	bsUint32 oldlen = 0;
	if (text != NULL)
		oldlen = (bsUint32) strlen(text) + 1;
	char *str = (char *)realloc(text, oldlen+len+1);
	if (str != NULL)
	{
		if (oldlen)
			str[oldlen-1] = '\n';
		memcpy(str+oldlen, inpPos, len);
		str[len+oldlen] = 0;
	}
	else
		str = text;
	inpPos += len;
	return str;
}



void MIDISequence::Sequence(WaveOutBuf *out)
{
	AmpValue lft;
	AmpValue rgt;

	int trkNum = 0;
	int chNum;
	MIDITrack *tp;

	for (tp = trcks; tp != NULL; tp = tp->next)
	{
		if (tp->Start(this))
			trkNum++;
	}

	double ticksToPlay = 0;

	theTick = 0;
	bool playing = true;
	while (playing)
	{
		if (trkNum)
		{
			trkNum = 0;
			for (tp = trcks; tp != NULL; tp = tp->next)
			{
				if (tp->Play(this))
					trkNum++;
			}
		}
		int count = 0;
		ticksToPlay += srTicks;
		while (ticksToPlay >= 1.0)
		{
			for (chNum = 0; chNum < MAX_MIDI_CHNL; chNum++)
			{
				if (instr[chNum] != NULL)
					count += instr[chNum]->Tick(&mix);
			}
			mix.Out(&lft, &rgt);
			out->Output2(lft, rgt);
			ticksToPlay -= 1.0;
		}
		playing = count > 0 ||trkNum > 0;
		theTick++;
	}
}

char *MIDIFileLoad::IntToStr(int val, char *s)
{
	if (val >= 10)
		s = IntToStr(val/10, s);
	*s++ = (val % 10) + '0';
	*s = '\0';
	return s;
}
