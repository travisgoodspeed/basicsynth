/////////////////////////////////////////////////////
/// @file SynthMutex.h Mutex and Signal
//
// Copyright 2010 Daniel R. Mitchell, All Rights Reserved
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////
/// @addtogroup grpGeneral
///@{
#ifndef _SYNTHMUTEX_H
#define _SYNTHMUTEX_H

/// Mutex (critical region) object.
/// A mutex is used to synchronize threads
/// by blocking entry to a section of code
/// by more than one thread.
class SynthMutex
{
private:
	void *mtx;
public:
	SynthMutex()
	{
		mtx = 0;
	}
	~SynthMutex()
	{
		Destroy();
	}

	/// Create the mutex.
	void Create();
	/// Destroy the mutex.
	void Destroy();
	/// Enter critical region.
	void Enter();
	/// Leave critical region.
	void Leave();
};

/// Signal object.
/// A signal object is used to signal another thread
/// to continue operation.
class SynthSignal
{
private:
	void *sig;
public:
	SynthSignal()
	{
		sig = 0;
	}

	~SynthSignal()
	{
		Destroy();
	}

	/// Create the signal.
	void Create();
	/// Destroy the signal.
	void Destroy();
	/// Wait for the signal.
	void Wait();
	/// Wakeup a thread waiting on a signal.
	void Wakeup();
};
#endif
///@}
