///////////////////////////////////////////////////////////
/// @file SynthThread.cpp Very simple thread class
//
// Copyright 2010 Daniel R. Mitchell, All Rights Reserved
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#include <SynthThread.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOGDI                           // don't need graphics
#define NOCRYPT
#include <windows.h>

class ThreadInfo
{
public:
	HANDLE thrdH;
	DWORD  thrdID;
	ThreadInfo()
	{
		thrdH = 0;
		thrdID = 0;
	}
};

static DWORD WINAPI Startup(LPVOID param)
{
	return (DWORD) ((SynthThread *)param)->ThreadProc();
}

int SynthThread::StartThread(int pri)
{
	info->thrdH = CreateThread(NULL, 0, Startup, this, CREATE_SUSPENDED, &info->thrdID);
	if (info->thrdH != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(info->thrdH, pri);
		ResumeThread(info->thrdH);
		Sleep(0);
		return 0;
	}
	return -1;
}

int SynthThread::StopThread()
{
	if (info->thrdH != INVALID_HANDLE_VALUE)
	{
		TerminateThread(info->thrdH, 0);
		return 0;
	}
	return -1;
}

int SynthThread::WaitThread()
{
	if (info->thrdH == INVALID_HANDLE_VALUE)
		return -1;

	try
	{
		WaitForSingleObject(info->thrdH, 10000);
	}
	catch(...)
	{
	}
	info->thrdH = INVALID_HANDLE_VALUE;
	info->thrdID = 0;
	return 0;
}

void SynthThread::ShortWait()
{
	Sleep(1);
}

#endif

#ifdef UNIX
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

class ThreadInfo
{
public:
	pthread_mutex_t thrdGuard;
	pthread_t thrdID;

	ThreadInfo()
	{
		thrdID = 0;
	}
};

static void *Startup(void *param)
{
	SynthThread *thrd = (SynthThread *)param;
	// synchronize with the main thread...
	pthread_mutex_lock(&thrd->info->thrdGuard);
	pthread_mutex_unlock(&thrd->info->thrdGuard);
	thrd->ThreadProc();
	pthread_exit((void*)0);
	return 0;
}

int SynthThread::StartThread(int pri)
{
	pthread_attr_t tattr;
	sched_param param;
	pthread_attr_init(&tattr);
	param.sched_priority = pri;
	pthread_attr_setschedparam(&tattr, &param);

	pthread_mutex_init(&info->thrdGuard, NULL);
	pthread_mutex_lock(&info->thrdGuard);
	int err = pthread_create(&info->thrdID, NULL, Startup, this);
	pthread_mutex_unlock(&info->thrdGuard);
	pthread_attr_destroy(&tattr);
	return err;
}

int SynthThread::StopThread()
{
	pthread_cancel(info->thrdID);
	return 0;
}

int SynthThread::WaitThread()
{
	try
	{
		pthread_join(info->thrdID, NULL);
		pthread_mutex_destroy(&info->thrdGuard);
	}
	catch (...)
	{
	}
	info->thrdID = 0;
	return 0;
}

void SynthThread::ShortWait()
{
	usleep(1000);
}

#endif

SynthThread::SynthThread()
{
	info = new ThreadInfo;
}

SynthThread::~SynthThread()
{
	delete info;
}

int SynthThread::ThreadProc()
{
	return -1;
}
