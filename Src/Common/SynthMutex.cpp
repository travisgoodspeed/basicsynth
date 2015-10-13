/////////////////////////////////////////////////////
/// @file SynthMutex.cpp Mutex and Signal
//
// Copyright 2010 Daniel R. Mitchell, All Rights Reserved
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////

#include <SynthMutex.h>


#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

void SynthMutex::Create()
{
	if (mtx == 0)
	{
		CRITICAL_SECTION *cs = new CRITICAL_SECTION;
		::InitializeCriticalSection(cs);
		mtx = (void*)cs;
	}
}

void SynthMutex::Destroy()
{
	if (mtx != 0)
	{
		CRITICAL_SECTION *cs = (CRITICAL_SECTION*)mtx;
		if (cs)
		{
			::DeleteCriticalSection(cs);
			delete cs;
		}
		mtx = 0;
	}
}

void SynthMutex::Enter()
{
	::EnterCriticalSection((CRITICAL_SECTION*)mtx);
}

void SynthMutex::Leave()
{
	::LeaveCriticalSection((CRITICAL_SECTION*)mtx);
}


void SynthSignal::Create()
{
	if (sig == 0)
		sig = (void*)CreateEvent(NULL, FALSE, 0, NULL);
}

void SynthSignal::Destroy()
{
	if (sig != 0)
	{
		CloseHandle((HANDLE)sig);
		sig = 0;
	}
}

void SynthSignal::Wait()
{
	if (sig != 0)
		::WaitForSingleObject((HANDLE)sig, (DWORD)-1);
}

void SynthSignal::Wakeup()
{
	if (sig != 0)
		::SetEvent((HANDLE)sig);
}
#endif

#if UNIX
#include <sys/types.h>
#include <pthread.h>

struct pthread_event
{
	pthread_mutex_t m;
	pthread_cond_t  c;
};

void SynthMutex::Create()
{
	if (mtx == 0)
	{
		pthread_mutex_t *cs = new pthread_mutex_t;
		pthread_mutex_init(cs, NULL);
		mtx = (void*)cs;
	}
}

void SynthMutex::Destroy()
{
	pthread_mutex_t *cs = (pthread_mutex_t*)mtx;
	if (cs)
	{
		pthread_mutex_destroy(cs);
		delete cs;
		mtx = 0;
	}
}

void SynthMutex::Enter()
{
	pthread_mutex_lock((pthread_mutex_t*)mtx);
}

void SynthMutex::Leave()
{
	pthread_mutex_unlock((pthread_mutex_t*)mtx);
}

void SynthSignal::Create()
{
	if (sig == 0)
	{
		pthread_event *e = new pthread_event;
		pthread_mutex_init(&e->m, NULL);
		pthread_cond_init(&e->c, NULL);
		sig = (void*)e;
	}
}

void SynthSignal::Destroy()
{
	pthread_event *e = (pthread_event*)sig;
	if (e)
	{
		pthread_cond_destroy(&e->c);
		pthread_mutex_destroy(&e->m);
		delete e;
		sig = 0;
	}
}

void SynthSignal::Wait()
{
	if (sig)
	{
		pthread_event *e = (pthread_event*)sig;
		pthread_mutex_lock(&e->m);
		pthread_cond_wait(&e->c, &e->m);
		pthread_mutex_unlock(&e->m);
	}
}

void SynthSignal::Wakeup()
{
	if (sig)
	{
		pthread_event *e = (pthread_event*)sig;
		pthread_mutex_lock(&e->m);
		pthread_cond_signal(&e->c);
		pthread_mutex_unlock(&e->m);
	}
}

#endif
