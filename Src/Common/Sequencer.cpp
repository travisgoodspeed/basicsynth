//////////////////////////////////////////////////////////////////
/// @file Sequencer.cpp Implementation of the sequencer.
//
// BasicSynth
//
// Sequencer code that is not inline
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
#include <SynthMutex.h>
#include <WaveTable.h>
#include <WaveFile.h>
#include <Mixer.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>
#include <Instrument.h>
#include <Sequencer.h>


//////////////////////////// TRACK ////////////////////////////

void SeqTrack::Reset()
{
	SeqEvent *evt;
	while ((evt = evtHead->next) != evtTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	evtLast = evtHead;
	seqLength = 0;
}

void SeqTrack::AddEvent(SeqEvent *evt)
{
	//printf("Add Event %d at time %d\n", evt->evid, evt->start);
	if (evtLast == NULL)
		evtLast = evtHead;
	SeqEvent *inspt;
	if (evt->start >= evtLast->start)
	{
		do
		{
			inspt = evtLast;
			evtLast = evtLast->next;
		} while (evt->start >= evtLast->start);
		inspt->Insert(evt);
		evtLast = evt;
	}
	else
	{
		do
		{
			inspt = evtLast;
			evtLast = evtLast->prev;
		} while (evt->start < evtLast->start);
		evtLast->Insert(evt);
		evtLast = evt;
	}
	bsInt32 e = evt->start + evt->duration;
	if (e >= seqLength)
		seqLength = e+1;
}

//////////////////////////// SEQUENCER ////////////////////////////

Sequencer::Sequencer()
{
	state = seqOff;
	playing = false;
	pausing = false;
	seqLength = 0;
	seqTick = 0;
	tickCB = 0;
	tickCount = 0;
	tickWrap = 0;
	tickArg = 0;
	tickRes = (bsInt32) (synthParams.sampleRate * 0.0005);
	wrapCount = 0;
	//cntrlMgr = 0;
	instMgr = 0;
	globEventID = 0;
	maxNote = 1000;
	trkActive = 0;
	evtActive = 0;

	track = new SeqTrack(0);

	immHead = new SeqEvent;
	immTail = new SeqEvent;
	immHead->Insert(immTail);
	immHead->start = 0;
	immTail->start = 0x7FFFFFFFL;
	immHead->evid = -1;
	immTail->evid = -2;

	actHead = new ActiveEvent;
	actTail = new ActiveEvent;
	actHead->ip = NULL;
	actTail->ip = NULL;
	actHead->evid = -1;
	actTail->evid = -2;
	actHead->ison = SEQ_AE_OFF;
	actTail->ison = SEQ_AE_OFF;
	actHead->flags = SEQ_AE_KEEP;
	actTail->flags = SEQ_AE_KEEP;
	actHead->Insert(actTail);

	critMutex.Create();
	pauseSignal.Create();
}

Sequencer::~Sequencer()
{
	Reset();
	immHead->Destroy();
	immTail->Destroy();
	delete actHead;
	delete actTail;
	delete track;
	globEventID = 0;
}

// Add the event to the sequence sorted by start time.
// The caller is responsible for setting valid values
// for inum, start, duration, type and eventid.
void Sequencer::AddEvent(SeqEvent *evt)
{
	if (evt == 0)
		return;

	SeqTrack *pp = 0;
	SeqTrack *tp = track;
	while (tp != 0)
	{
		if (tp->Track() == evt->track)
			break;
		pp = tp;
		tp = tp->next;
	}
	if (!tp)
	{
		tp = new SeqTrack(evt->track);
		if (pp)
			pp->Insert(tp);
		else
			track = tp;
	}
	tp->AddEvent(evt);
}


void Sequencer::AddImmediate(SeqEvent *evt)
{
	if (evt == 0)
		return;

	critMutex.Enter();
	immTail->InsertBefore(evt);
	critMutex.Leave();
}

// Multi-mode sequencer can play live, sequence, loop tracks or any combination.
bsUint32 Sequencer::SequenceMulti(InstrManager& im, bsUint32 startTime, bsUint32 endTime, SeqState st)
{
	if (state != seqOff)
		return 0;

	if (track == 0)
		return 0;

	if (tickRes < 1)
		tickRes = 1;

	im.SetSequencer(this);

	SeqEvent *imm = 0;
	SeqEvent *evt = 0;
	SeqTrack *tp = 0;
	trkActive = 0;
	evtActive = 0;

	seqTick = startTime;
	track->LoopCount(1);
	track->Start(seqTick, tickRes);

	instMgr = &im;
	instMgr->Start();

	state = st;
	int live = st & seqPlay;
	int sequenced = st & seqSequence;
	int once = st & seqOnce;

	tickCount = 0;
	wrapCount = 0;
	if (tickCB)
		tickCB(0, tickArg);

	playing = true;
	while (playing)
	{
		if (live)
		{
			// in-line message peek/get
			critMutex.Enter();
			imm = immHead->next;
			if (imm != immTail)
				imm->Remove();
			else
				imm = NULL;
			critMutex.Leave();
			if (imm)
			{
				ProcessEvent(imm, 0);
				imm->Destroy();
			}
		}

		if (sequenced)
		{
			// find any events that are ready to activate
			trkActive = 0;
			for (tp = track; tp; tp = tp->next)
			{
				while ((evt = tp->NextEvent()) != 0)
					ProcessEvent(evt, SEQ_AE_TM);
				trkActive |= tp->Tick();
			}
		}

		// invoke all active instruments for tickRes samples
		evtActive = Tick();

		// When we have reached the end of the sequence
		// AND all events have finished,
		// OR, we have hit the last time caller wanted,
		// we can quit. N.B. this can leave active events!
		if (once)
		{
			if ((!trkActive && !evtActive)
			  || (endTime > 0 && seqTick >= endTime))
				playing = false;
		}
	}
	instMgr->Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	ClearActive();

	state = seqOff;

	if (tickCB)
		tickCB(wrapCount, tickArg);

	im.SetSequencer(NULL);

	return seqTick; // in case the caller wants to know how long we played...
}

/// Optimal sequencing - no live events are checked and loop tracks are not played.
bsUint32 Sequencer::Sequence(InstrManager& im, bsUint32 startTime, bsUint32 endTime)
{
	if (state != seqOff)
		return 0;

	if (track == 0)
		return 0;

	if (tickRes < 1)
		tickRes = 1;

	im.SetSequencer(this);

	SeqEvent *evt = 0;
	trkActive = 0;
	evtActive = 0;

	seqTick = startTime;
	track->LoopCount(1);
	track->Start(seqTick, tickRes);

	instMgr = &im;
	instMgr->Start();

	state = seqSeqOnce;

	tickCount = 0;
	wrapCount = 0;
	if (tickCB)
		tickCB(0, tickArg);

	playing = true;
	while (playing)
	{
		// find any events that are ready to activate
		while ((evt = track->NextEvent()) != 0)
			ProcessEvent(evt, SEQ_AE_TM);
		trkActive = track->Tick();

		// invoke all active instruments for tickRes samples
		evtActive = Tick();

		// When we have reached the end of the sequence
		// AND all events have finished,
		// OR, we have hit the last time caller wanted,
		// we can quit. N.B. this can leave active events!
		if ((!trkActive && !evtActive)
		  || (endTime != 0 && seqTick >= endTime))
			playing = false;
	}
	instMgr->Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	ClearActive();

	state = seqOff;

	if (tickCB)
		tickCB(wrapCount, tickArg);

	im.SetSequencer(NULL);

	return seqTick; // in case the caller wants to know how long we played...
}

/// Optimal live playback - no tracks are checked, only immediate input.
bsUint32 Sequencer::Play(InstrManager& im)
{
	if (state != seqOff)
		return 0;

	if (tickRes < 1)
		tickRes = 1;

	instMgr = &im;
	im.SetSequencer(this);

	SeqEvent *evt;

	ClearActive();

	critMutex.Enter();
	while ((evt = immHead->next) != immTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	critMutex.Leave();

	state = seqPlay;
	seqTick = 0;

	instMgr->Start();
	playing = true;
	while (playing)
	{
		// in-line message peek/get
		critMutex.Enter();
		evt = immHead->next;
		if (evt != immTail)
			evt->Remove();
		else
			evt = NULL;
		critMutex.Leave();
		if (evt)
		{
			ProcessEvent(evt, 0);
			evt->Destroy();
		}

		Tick();
	}
	instMgr->Stop();

	ClearActive();

	state = seqOff;

	im.SetSequencer(NULL);

	return seqTick;
}

// Reset the sequencer.
// Reset should be called to clean up any memory before filling in a new sequence.
void Sequencer::Reset()
{
	SeqEvent *evt;
	ClearActive();

	critMutex.Enter();
	while ((evt = immHead->next) != immTail)
	{
		evt->Remove();
		evt->Destroy();
	}
	critMutex.Leave();

	track->Reset();
	SeqTrack *tp;
	while ((tp = track->next) != 0)
	{
		tp->Remove();
		delete tp;
	}
	seqLength = 0;
	seqTick = 0;
	globEventID = 0;
}

// Discard any active events. No "Stop" or "IsFinished"
void Sequencer::ClearActive()
{
	ActiveEvent *act;

	while ((act = actHead->next) != actTail)
	{
		instMgr->Deallocate(act->ip);
		act->Remove();
		delete act;
	}
}

// Cycle all active events (Tick)
// This is "IT" - where we actually generate samples...
int Sequencer::Tick()
{
	if (pausing)
	{
		Wait();
		if (!playing)
			return 0;
	}

	int actCount;
	bsInt32 tickBlk = tickRes;
	do
	{
		actCount = 0;
		Instrument *ins;
		ActiveEvent *act = actHead->next;
		while (act != actTail)
		{
			actCount++;
			ins = act->ip;
			if (act->ison == SEQ_AE_ON)
			{
				// duration not yet reached or note-off not signaled
				ins->Tick();
				if ((act->flags & SEQ_AE_TM) && --act->count == 0)
				{
					// duration finished
					ins->Stop();
					act->ison = SEQ_AE_REL;
					//printf("Stop Note for event %d\n", act->evid);
				}
				act = act->next;
			}
			else if (act->ison == SEQ_AE_REL)
			{
				// in release
				if (ins->IsFinished())
				{
					//printf("Remove Note for event %d\n", act->evid);
					instMgr->Deallocate(ins);
					ActiveEvent *p = act->Remove();
					delete act;
					act = p;
					actCount--;
				}
				else
				{
					ins->Tick();
					act = act->next;
				}
			}
		}
		instMgr->Tick();

		seqTick++;
		if (tickCB && ++tickCount >= tickWrap)
		{
//#if _MSC_VER && _DEBUG
//			char buf[80];
//			sprintf_s(buf, 80, "Tick %d active = %d\r\n", seqTick, actCount);
//			OutputDebugString(buf);
//#endif
			tickCB(++wrapCount, tickArg);
			tickCount = 0;
		}
	} while (--tickBlk > 0);

	return actCount;
}

void Sequencer::Broadcast(SeqEvent *evt)
{
	ActiveEvent *act;
	for (act = actHead->next; act != actTail; act = act->next)
	{
		if (act->chnl == evt->chnl)
			act->ip->Param(evt);
	}
}

void Sequencer::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	bsInt16 typ = evt->type;
	ActiveEvent *act;
	SeqTrack *tp;
	TrackEvent *tevt;

	switch (typ)
	{
	case SEQEVT_RESTART:
	case SEQEVT_PARAM:
	case SEQEVT_STOP:
		// try to match this event to a prior event
		for (act = actHead->next; act != actTail; act = act->next)
		{
			if (act->evid == evt->evid)
			{
				if (typ == SEQEVT_PARAM)
					act->ip->Param(evt);
				else if (typ == SEQEVT_STOP)
				{
					act->ip->Stop();
					act->ison = SEQ_AE_REL;
				}
				else if (typ == SEQEVT_RESTART)
				{
					act->ip->Start(evt);
					act->count = evt->duration;
					act->ison = SEQ_AE_ON;
				}
				return;
			}
		}
		if (typ != SEQEVT_RESTART)
			break;
		/// FALTHROUGH on RESTART event no longer playing
	case SEQEVT_START:
		if (++evtActive > maxNote)
		{
			// This is for MIDI, or other live playback,
			// where the instruments are not "well behaved."
			for (act = actHead->next; act != actTail; act = act->next)
			{
				if (act->ison == SEQ_AE_REL)
					break;
			}
			// if no notes in release, remove oldest
			if (act == actTail)
				act = actHead->next;
			if (act != actTail) // sanity check
			{
				instMgr->Deallocate(act->ip);
				act->Remove();
				delete act;
				evtActive--;
			}
		}
		// Start an instrument. The instrument manager must
		// locate the instrument by id (inum) and return
		// a valid instance. We then initialize the instrument
		// by passing the event structure.
		if ((act = new ActiveEvent) == NULL)
		{
			playing = false;
			return;
		}
		actTail->InsertBefore(act);
		act->evid = evt->evid;
		act->ison = SEQ_AE_ON;
		act->count = evt->duration;
		act->chnl = evt->chnl;
		if ((flags & SEQ_AE_TM) && act->count == 0)
			act->count = 1;
		act->flags = flags;

		// assume: allocate should not fail, even if inum is invalid...
		act->ip = instMgr->Allocate(evt);
		if (act->ip != 0)
			act->ip->Start(evt);
		else	// ...except if we are out of memory, so give up now.
			playing = false;
		break;
	case SEQEVT_STARTTRACK:
		tevt = (TrackEvent *)evt;
		for (tp = track; tp; tp = tp->next)
		{
			if (tp->Track() == tevt->trkNo)
			{
				tp->LoopCount(tevt->loopCount);
				tp->Start(0, tickRes);
				break;
			}
		}
		break;
	case SEQEVT_STOPTRACK:
		tevt = (TrackEvent *)evt;
		for (tp = track->next; tp; tp = tp->next)
		{
			if (tp->Track() == tevt->trkNo)
			{
				tp->Stop();
				break;
			}
		}
		break;
	case SEQEVT_CONTROL:
		instMgr->ProcessEvent(evt, flags);
		break;
	case SEQEVT_CANCEL:
		for (act = actHead->next; act != actTail; act = act->next)
		{
			if (act->chnl == evt->chnl)
				act->ip->Cancel();
		}
		break;
	}
}

void Sequencer::Wait()
{
	SeqState was = state;
	state = seqPaused;
	pauseSignal.Wait();
	state = was;
}

void Sequencer::Halt()
{
	playing = false;
	if (pausing)
	{
		pausing = false;
		pauseSignal.Wakeup();
	}
}

/////////// Sequencer with event callbacks //////////////////////

void SequencerCB::ProcessEvent(SeqEvent *evt, bsInt16 flags)
{
	Sequencer::ProcessEvent(evt, flags);
	if (flags)
		Notify(evt->type, evt);
}

void SequencerCB::Wait()
{
	Notify(SEQEVT_SEQPAUSE, 0);
	Sequencer::Wait();
	Notify(SEQEVT_SEQRESUME, 0);
}

bsUint32 SequencerCB::SequenceMulti(InstrManager &im, bsUint32 startTime, bsUint32 endTime, SeqState st)
{
	Notify(SEQEVT_SEQSTART, 0);
	Sequencer::SequenceMulti(im, startTime, endTime, st);
	Notify(SEQEVT_SEQSTOP, 0);
	return seqTick;
}

bsUint32 SequencerCB::Sequence(InstrManager& im, bsUint32 startTime, bsUint32 endTime)
{
	Notify(SEQEVT_SEQSTART, 0);
	Sequencer::Sequence(im, startTime, endTime);
	Notify(SEQEVT_SEQSTOP, 0);
	return seqTick;
}

bsUint32 SequencerCB::Play(InstrManager& im)
{
	Notify(SEQEVT_SEQSTART, 0);
	Play(im);
	Notify(SEQEVT_SEQSTOP, 0);
	return seqTick;
}
