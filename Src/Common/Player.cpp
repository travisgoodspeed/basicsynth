/////////////////////////////////////////////////////////////////////////////
/// @file Player.cpp Implementation of live playback.
//
// BasicSynth - Player object
//
// See Player.h for an explanation...
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
#if UNIX
#include <sys/types.h>
#endif
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
#include <Player.h>

void Player::AddEvent(SeqEvent *evt)
{
	if (evt == 0)
		return;

	EnterCritical();
	evtTail->InsertBefore(evt);
	LeaveCritical();
}

void Player::Play(InstrManager& instMgr)
{
	bsInt16 typ;
	SeqEvent *evt;
	ActiveEvent *act;
	ActiveEvent *actHead = new ActiveEvent;
	ActiveEvent *actTail = new ActiveEvent;
	actHead->ip = NULL;
	actTail->ip = NULL;
	actHead->evid = -1;
	actTail->evid = -2;
	actHead->ison = SEQ_AE_OFF;
	actTail->ison = SEQ_AE_OFF;
	actHead->Insert(actTail);

	instMgr.Start();
	playing = true;
	while (playing)
	{
		// in-line message peek/get
		EnterCritical();
		evt = evtHead->next;
		if (evt != evtTail)
			evt->Remove();
		else
			evt = NULL;
		LeaveCritical();
		if (evt)
		{
			if ((typ = evt->type) != SEQEVT_START)
			{
				// try to match this event to a prior event
				for (act = actHead->next; act != actTail; act = act->next)
				{
					if (act->evid == evt->evid)
					{
						if (typ == SEQEVT_PARAM)
						{
							act->ip->Param(evt);
						}
						else if (typ == SEQEVT_STOP)
						{
							act->ip->Stop();
							act->ison = SEQ_AE_REL;
						}
						break;
					}
				}
			}
			else
			{
				// Start an instrument. The instrument manager should
				// return a valid instance of something. We initialize
				// the instrument by passing the event structure. 
				if ((act = new ActiveEvent) != NULL)
				{
					if (evt->im)
						act->ip = instMgr.Allocate(evt->im);
					else
						act->ip = instMgr.Allocate(evt->inum);
					if (act->ip)
					{
						actTail->InsertBefore(act);
						act->count = evt->duration;
						act->evid = evt->evid;
						act->ison = SEQ_AE_ON;
						act->ip->Start(evt);
					}
					else
						delete act;
				}
			}
			delete evt;
		}

		// Cycle all active events (Tick)
		// This is "IT" - where we actually generate samples...
		act = actHead->next;
		while (act != actTail)
		{
			act->ip->Tick();
			if (act->ison == SEQ_AE_REL && act->ip->IsFinished())
			{
				instMgr.Deallocate(act->ip);
				ActiveEvent *p = act->Remove();
				delete act;
				act = p;
			}
			else
				act = act->next;
		}
		instMgr.Tick();
	}
	instMgr.Stop();

	// Since it is possible to halt the sequence while events are still
	// active, we do clean-up here. We don't bother with Stop or IsFinished
	// since we are no longer generating samples.
	while ((act = actHead->next) != actTail)
	{
		instMgr.Deallocate(act->ip);
		act->Remove();
		delete act;
	}
	
	delete actTail;
	delete actHead;
}

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

void Player::CreateMutex()
{
	CRITICAL_SECTION *cs = new CRITICAL_SECTION;
	::InitializeCriticalSection(cs);
	mutex = (void*)cs;
}

void Player::DestroyMutex()
{
	CRITICAL_SECTION *cs = (CRITICAL_SECTION*)mutex;
	if (cs)
	{
		::DeleteCriticalSection(cs);
		delete cs;
	}
}

inline void Player::EnterCritical()
{
	EnterCriticalSection((CRITICAL_SECTION*)mutex);
}

inline void Player::LeaveCritical()
{
	LeaveCriticalSection((CRITICAL_SECTION*)mutex);
}
#endif

#if UNIX
#include <pthread.h>

void Player::CreateMutex()
{
	pthread_mutex_t *cs = new pthread_mutex_t;
	pthread_mutex_init(cs, NULL);
	mutex = (void*)cs;
}

void Player::DestroyMutex()
{
	pthread_mutex_t *cs = (pthread_mutex_t*)mutex;
	if (cs)
	{
		pthread_mutex_destroy(cs);
		delete cs;
		mutex = 0;
	}
}

inline void Player::EnterCritical()
{
	pthread_mutex_lock((pthread_mutex_t*)mutex);
}

inline void Player::LeaveCritical()
{
	pthread_mutex_unlock((pthread_mutex_t*)mutex);
}
#endif
