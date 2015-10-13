/////////////////////////////////////////////////////////////////////////////
// BasicSynth - Player object
//
/// @file Player.h Interactive sound playback
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _PLAYER_H_
#define _PLAYER_H_

#pragma once

/////////////////////////////////////////////////////////////////////////////
/// Interactive sound playback.
///
/// This class manages immediate playback of sounds. It waits for
/// a start event, then allocates an instrument and adds the instrument
/// to the list of active sounds. The instrument remains active until
/// a corresponding stop event is received. When no instruments are
/// active, the Player will continue to call the instrument manager
/// which must output zeros to the DAC buffer.
///
/// To play a sound, allocate a sequencer event and call the AddEvent
/// method from another thread. Unlike the Sequencer, events are removed from
/// the event list as soon as they are discovered. For every start event, 
/// a matching stop event is required. The event id field of the SeqEvent 
/// object is used to match events. The caller should NOT free the event
/// object; it will be deleted by the Player after it is played.
///
/// Timing of sample output is under control of the instrument manager.
/// In a typical setup, the instrument manager sends samples to
/// a wave output buffer. The wave output buffer will synchronize
/// sample output to the sample rate by blocking on the sound card
/// driver output routine.
///
/// The player should be run as a separate thread. New events are appended
/// to the event list by one thread and removed by the playback thread. 
/// In short, this is a very simple message queue between threads. 
/// The critical section mutex is used to synchronize access to the
/// event list. Optionally, we could use a non-blocking pipe or message queue
/// in place of directly adding to the event object list.
///
/// @note
/// Beginning with version 1.2 the Sequencer class can do everything the
/// Player can do plus run sequences and is the prefered way of doing
/// this now. The Player is still useful for small, interactive synth programs, however.
/////////////////////////////////////////////////////////////////////////////
class Player
{
private:
	SeqEvent *evtHead;
	SeqEvent *evtTail;
	SeqEvent *evtLast;
	bool playing;
	void *mutex;

	void CreateMutex();
	void DestroyMutex();
	inline void EnterCritical();
	inline void LeaveCritical();

public:
	Player()
	{ 
		evtHead = new SeqEvent;
		evtTail = new SeqEvent;
		evtHead->Insert(evtTail);
		evtLast = evtHead;
		evtHead->start = 0;
		evtTail->start = 0x7FFFFFFFL;
		evtHead->evid = -1;
		evtTail->evid = -2;
		playing = false;
		CreateMutex();
	}

	~Player()
	{
		Reset();
		evtHead->Destroy();
		evtTail->Destroy();
		DestroyMutex();
	}

	/// Reset should be called to clean up any old
	/// events before playback is started or after
	/// playback ends. Usually, there is nothing in the list.
	void Reset()
	{
		EnterCritical();
		while ((evtLast = evtHead->next) != evtTail)
		{
			evtLast->Remove();
			evtLast->Destroy();
		}
		evtLast = evtHead;
		LeaveCritical();
	}

	/// Halt is called to stop any further sequencing.
	/// This is not protected by a mutex since it should
	/// be an atomic assignment anyway. The player only
	/// reads this value, never sets it, after start.
	void Halt()
	{
		playing = false;
	}

	/// IsPlaying is called to determine if the Player is running
	/// the event handling loop.
	/// Because the Player does not set this value true until
	/// it enters the playback loop, IsPlaying can be used to
	/// wait for the player to start as well as waiting to see
	/// if it has been stopped.
	/// @return true if the player is running.
	int IsPlaying()
	{
		return playing;
	}

	/// Add the event to the sequence. The caller is responsible for setting
	/// valid values for inum or im, type and eventid. The available types
	/// are start, stop, and change parameters.
	/// @param evt the event to send for playback
	void AddEvent(SeqEvent *evt);

	/// The playback loop. 
	/// @param instMgr reference to an instrument manager object
	void Play(InstrManager& instMgr);
};
//@}
#endif
