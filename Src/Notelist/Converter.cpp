//////////////////////////////////////////////////////////////////////
/// @file Converter.cpp Implementation of the Notelist converter.
//
// The converter class is the container for the interpreter. This is
// the class that aggregates the lexer, parser, generator and other
// relevant bits and pieces, and provides the interface to Notelist. 
// Callers must provide a sequencer, instrument manager, and message
// output class to the converter. After setup, call Convert and then
// Generate. It is not wise to call Generate if Convert returns
// a value > 0. The important stuff happens in MakeEvent.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"

nlConverter::nlConverter()
{
	debugLevel = 0;
	maxError = 5;
	lexin = NULL;
	curVoice = NULL;
	eout = NULL;
	ownLexIn = 0;
	mgr = NULL;
	seq = NULL;
	eng = NULL;
	evtID1 = 0;
	evtID2 = 0;
	sampleRate = 44100.0;
	mapList = 0;
	curMap = 0;
	symbList = NULL;
	mixInstr = -1;
	canceled = 0;
}

nlConverter::~nlConverter()
{
	if (ownLexIn)
		delete lexin;
	nlSymbol *sym;
	while ((sym = symbList) != NULL)
	{
		symbList = sym->next;
		delete sym;
	}
}

int nlConverter::Convert(const char *filename, nlLexIn *in)
{
	nlLex lexer;
	if (in == NULL)
	{
		lexin = new nlLexFileIn(filename);
		in = lexin;
		ownLexIn = 1;
	}

	lexer.Open(in);

	parser.SetFile(filename);
	parser.SetConverter(this);
	parser.SetGen(&gen);
	parser.SetLex(&lexer);

	int rv = parser.Parse();

	lexer.Close();
	return rv;
}

int nlConverter::Generate()
{
	if (seq == NULL || mgr == NULL)
		return -1;

	mixInstr = FindInstrNum("[mixer]");
	gen.SetConverter(this);
	return gen.Run();
}

void nlConverter::Reset()
{
	gen.Reset();
}

void nlConverter::BeginNotelist()
{
	curVoice = NULL;
	curMap = NULL;
}

void nlConverter::EndNotelist()
{
}

void nlConverter::BeginVoice(nlVoice *vp)
{
	if ((curVoice = vp) != NULL)
	{
		curMap = mapList;
		while (curMap)
		{
			if (curMap->Match(vp->instr))
				break;
			curMap = curMap->next;
		}
	}
	else
		curMap = NULL;
}

void nlConverter::EndVoice(nlVoice *vp)
{
	curVoice = NULL;
}

void nlConverter::BeginInstr()
{
	if (curVoice == NULL)
	{
		curMap = NULL;
		return;
	}

	curMap = mapList;
	while (curMap)
	{
		if (curMap->Match(curVoice->instr))
			break;
		curMap = curMap->next;
	}
}

void nlConverter::BeginNote(double start, double dur)
{
	MakeEvent(SEQEVT_START, start, dur);
}

void nlConverter::RestartNote(double start, double dur)
{
	MakeEvent(SEQEVT_RESTART, start, dur);
}

void nlConverter::ContinueNote(double start)
{
	MakeEvent(SEQEVT_PARAM, start, 0);
}

void nlConverter::MakeEvent(int evtType, double start, double dur)
{
	if (curVoice == NULL)
		return;

	// Insert new event or alter the current note
	SeqEvent *evt = mgr->ManufEvent(curVoice->instr);
	if (evt == NULL)
		return;
	if (evtType == SEQEVT_START)
		evtID1 = seq->NextEventID();
	evt->SetID(evtID1);
	evt->SetType(evtType);
	evt->SetParam(P_TRACK, (float) curVoice->track);
	evt->SetParam(P_INUM, (float) curVoice->instr);
	evt->SetParam(P_CHNL, (float) curVoice->chnl);
	evt->SetParam(P_START, (float) start);
	evt->SetParam(P_DUR, (float) dur);

	double amp = curVoice->lastVol;
	if (gen.GetVersion() < 1.0)
		amp /= 327.67;

	// in MIDI Velocity mode, per-note amplitude
	// is passed via note-on velocity with volume
	// multipler set in the volume. Otherwise,
	// we calculate total attenuation here.
	double vel;
	if (gen.GetVelocityOn())
	{
		vel = amp;
		amp = curVoice->volMul * 100.0;
	}
	else
	{
		vel = 127.0;
		amp *= curVoice->volMul;
	}
	evt->SetParam(P_NOTEONVEL, (float) vel);

	if (amp > 0)
	{
		if (gen.GetVoldbMode())
			amp = pow(10.0, (amp - 100.0) / 20.0); 
		else
			amp *= 0.01;
	}
	evt->SetParam(P_VOLUME, (float) amp);

	double pit;
	if (gen.GetFrequencyMode())
	{
		pit = curVoice->lastPit * pow(2.0, (double)curVoice->transpose/12.0);
		evt->SetParam(P_FREQ, pit);
	}
	else // P_PITCH automatically calculates P_FREQ.
	{
		pit = curVoice->lastPit + curVoice->transpose;
		evt->SetParam(P_PITCH, (long) pit);
	}

	double val;
	long pn;
	long pc = curVoice->cntParam;
	for (pn = 0; pn < pc; pn++)
	{
		val = curVoice->lastParam[pn];
		if (curMap)
			curMap->MapParam(evt, pn, val);
		else
			evt->SetParam(P_USER+pn, (float) val);
	}

	seq->AddEvent(evt);

	if (curVoice->doublex != 0)
	{
		SeqEvent *evt2 = mgr->ManufEvent(curVoice->instr);
		if (evt != NULL)
		{
			evt2->CopyEvent(evt);
			if (evtType == SEQEVT_START)
				evtID2 = seq->NextEventID();
			evt2->SetID(evtID2);
			evt2->SetParam(P_NOTEONVEL, vel);
			evt2->SetParam(P_VOLUME, amp * curVoice->doublev);
			if (gen.GetFrequencyMode())
				evt2->SetParam(P_FREQ, pit * pow(2.0, (double)curVoice->doublex/12.0));
			else
				evt->SetParam(P_PITCH, (long) pit + curVoice->doublex);
			seq->AddEvent(evt2);
		}
	}
}

void nlConverter::MixerEvent(int fn, double *params)
{
	if (mixInstr < 0)
		return;

	SeqEvent *evt = mgr->ManufEvent(mixInstr);
	evt->SetID(seq->NextEventID());
	evt->SetType(SEQEVT_START);
	evt->SetParam(P_TRACK, curVoice->track);
	evt->SetParam(P_INUM, (long) mixInstr);
	evt->SetParam(P_CHNL, (long) curVoice->chnl);
	evt->SetParam(P_START, (float) curVoice->curTime);
	evt->SetParam(P_DUR, (float) 0.0);
	evt->SetParam(P_MIX_FUNC, (float) fn);
	if (fn & mixFx)
		evt->SetParam(P_MIX_FX, (float) params[0]);
	evt->SetParam(P_MIX_FROM, (float) params[1]);
	if (fn & (mixRamp|mixOsc))
	{
		evt->SetParam(P_MIX_TO, (float) params[2]);
		evt->SetParam(P_MIX_TIME, (float) params[3]);
		if (fn & mixOsc)
		{
			evt->SetParam(P_MIX_FRQ, (float) params[4]);
			evt->SetParam(P_MIX_WT, (float) params[5]);
		}
	}
	seq->AddEvent(evt);
}

void nlConverter::MidiEvent(short mmsg, short val1, short val2)
{
	if (!curVoice)
		return;
	ControlEvent *evt = new ControlEvent;
	evt->SetID(seq->NextEventID());
	evt->SetType(SEQEVT_CONTROL);
	evt->SetParam(P_START, curVoice->curTime);
	evt->SetParam(P_CHNL, curVoice->chnl);
	evt->SetParam(P_TRACK, curVoice->track);
	evt->SetParam(P_MMSG, mmsg);
	evt->SetParam(P_CTRL, val1);
	evt->SetParam(P_CVAL, val2);
	seq->AddEvent(evt);
}

void nlConverter::TrackOp(int op, int trk, int cnt)
{
	if (!curVoice)
		return;
	TrackEvent *evt = new TrackEvent;
	evt->SetID(seq->NextEventID());
	evt->SetType(op ? SEQEVT_STARTTRACK : SEQEVT_STOPTRACK);
	evt->SetParam(P_START, curVoice->curTime);
	evt->SetParam(P_CHNL, curVoice->chnl);
	evt->SetParam(P_TRACK, curVoice->track);
	evt->SetParam(P_TRKNO, trk);
	evt->SetParam(P_LOOP, cnt);
	seq->AddEvent(evt);
}

int nlConverter::FindInstrNum(const char *name)
{
	if (name == NULL || *name == 0)
		return -1;

	InstrConfig *ip;
	if ((ip = mgr->FindInstr(name)) != NULL)
		return ip->inum;

	return -1;
}

int nlConverter::GetParamID(int inum, const char *name)
{
	InstrConfig *ip;
	if ((ip = mgr->FindInstr(inum)) != NULL)
		return (int) ip->GetParamID(name);
	return -1;
}

void nlConverter::InitParamMap(int inum)
{
	nlParamMap *mp = mapList;
	while (mp)
	{
		if (mp->Match(inum))
		{
			mp->Clear();
			break;
		}
		mp = mp->next;
	}

	if (curVoice)
	{
		curVoice->cntParam = 0;
		if (inum == curVoice->instr)
			curMap = mp;
	}
}

void nlConverter::SetParamMap(int inum, int pn, int mn, double scl)
{
	nlParamMap *mp = mapList;
	while (mp)
	{
		if (mp->Match(inum))
			break;
		mp = mp->next;
	}

	if (mp == NULL)
	{
		mp = new nlParamMap;
		mp->SetInstr(inum);
		// push_front - so that the new node will be
		// seen first on the next call.
		if (mapList != NULL)
			mapList->InsertBefore(mp);
		mapList = mp;
	}

	mp->AddEntry(pn, mn, (float) scl);
	if (curVoice && curVoice->instr == inum)
		curMap = mp;
}

nlSymbol *nlConverter::Lookup(const char *name)
{
	nlSymbol *sym = symbList;
	while (sym != NULL)
	{
		if (CompareToken(sym->name, name) == 0)
			return sym;
		sym = sym->next;
	}
	return NULL;
}

nlSymbol *nlConverter::AddSymbol(const char *name)
{
	nlSymbol *sym = new nlSymbol(name);
	sym->next = symbList;
	symbList = sym;
	return sym;
}

void nlConverter::Write(char *txt)
{
	// Output some information for the user and/or to an output file as appropriate
	if (eout)
		eout->OutputMessage(txt);
}
// it appears that strcmpi, strdup, itoa are "deprecated" (pffffttt...)

int CompareToken(const char *s1, const char *s2)
{
	int c1, c2;
	while (*s1 && *s2)
	{
		c1 = ((int)*s1++) & 0xFF;
		c2 = ((int)*s2++) & 0xFF;
		if (c1 >= 'a' && c1 <= 'z')
			c1 = 'A' + c1 - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 = 'A' + c2 - 'a';
		if (c1 != c2)
			return c2 - c1;
	}
	return *s2 - *s1;
}

char *StrMakeCopy(const char *s)
{
	char *snew = NULL;
	if (s)
	{
		if ((snew = new char[strlen(s)+1]) != NULL)
			strcpy(snew, s);
	}
	return snew;
}

char *StrPaste(const char *s1, const char *s2)
{
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char *s3 = new char[len1+len2+1];
	if (s3 != NULL)
	{
		strcpy(s3, s1);
		strcpy(s3+len1, s2);
	}
	return s3;
}

