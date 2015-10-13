///////////////////////////////////////////////////////////
/// @file WaveOutDirect.cpp Sample output using DirectSound
//
// BasicSynth - send samples to the sound card using DirectSound
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////

#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <WaveOutDirect.h>

#if !USE_SDK_DSOUND
CLSID CLSID_DirectSound = {0x47d4d946, 0x62e8, 0x11cf, {0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0} };
IID IID_IDirectSound = {0x279AFA83, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60} };
#endif
typedef HRESULT (WINAPI *tDirectSoundCreate)(const GUID *pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT (WINAPI *tDirectSoundEnumerate)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

/// Manage the DirectSound object.
/// This class creates a "singleton" DirectSound object
/// shared by all WaveOutDirect instances. One instance
/// of this class exists and is created at init time
/// and destroyed at program exit.
class ManageDirectSound
{
public:
	tDirectSoundCreate pDirectSoundCreate;
	tDirectSoundEnumerate pDirectSoundEnumerate;
	IDirectSound *dirSndObj;
	GUID *lastDev;

	ManageDirectSound()
	{
		dirSndObj = NULL;
		lastDev = NULL;
		HMODULE h = LoadLibrary("dsound.dll");
		if (h)
		{
			pDirectSoundCreate = (tDirectSoundCreate)GetProcAddress(h, "DirectSoundCreate");
			pDirectSoundEnumerate = (tDirectSoundEnumerate)GetProcAddress(h, "DirectSoundEnumerateA");
		}
	}

	~ManageDirectSound()
	{
		// This doesn't work. The dsound lib can get
		// unloaded before this destructor is called
		// and the dirSndObj will no longer be valid.
		//if (dirSndObj != NULL)
		//{
		//	dirSndObj->Release();
		//	dirSndObj = NULL;
		//}
		//if (lastDev)
		//	delete lastDev;
	}

	/// Add a reference to the DirectSound object.
	ULONG AddRef()
	{
		ULONG rc = 0;
		if (dirSndObj != NULL)
			rc = dirSndObj->AddRef();
		return rc;
	}

	/// Release a reference to the DirectSound object.
	ULONG Release()
	{
		ULONG rc = 0;
		if (dirSndObj != NULL)
		{
			if ((rc = dirSndObj->Release()) == 0)
				dirSndObj = NULL;
		}
		return rc;
	}

	/// Create the DirectSound object if it does not exist.
	/// If the device GUID is changed, we re-init.
	int Create(HWND w, GUID *dev)
	{
		int newDev = 1;
		if (dirSndObj)
		{
			if (lastDev && dev)
			{
				if (memcmp(lastDev, dev, sizeof(GUID)) == 0)
					newDev = 0;
			}
			else if (!lastDev && !dev)
				newDev = 0;
			if (newDev)
			{
				dirSndObj->Release();
				dirSndObj = NULL;
			}
		}
		if (dirSndObj == NULL)
		{
			if (dev)
			{
				if (!lastDev)
					lastDev = new GUID;
				memcpy(lastDev, dev, sizeof(GUID));
			}
			else
			{
				if (lastDev)
					delete lastDev;
				lastDev = 0;
			}

			HRESULT hr;
			hr = pDirectSoundCreate(dev, &dirSndObj, NULL);
			if (hr != S_OK)
				return -1;
			hr = dirSndObj->SetCooperativeLevel(w, DSSCL_PRIORITY);
			if (hr != S_OK)
			{
				dirSndObj->Release();
				dirSndObj = NULL;
				return -1;
			}
		}
		return 0;
	}
};

static ManageDirectSound dsObj;

WaveOutDirect::WaveOutDirect()
{
	dirSndBuf = 0;
	nextWrite = 0;
	latency = 0.02;
	numBlk = 4;
	blkLen = 0;
	bufLen = 0;
	lastBlk = 0;
	startLock = 0;
	sizeLock = 0;
	pauseTime = 20;
	outState = 0;
	channels = 2;
}

WaveOutDirect::~WaveOutDirect()
{
	Shutdown();
}

void WaveOutDirect::ClearBuffer()
{
	if (dirSndBuf)
	{
		if (dirSndBuf->Lock(0, bufLen, &startLock, &sizeLock, NULL, NULL, 0) == S_OK)
		{
			memset(startLock, 0, sizeLock);
			dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		}
	}
}

int WaveOutDirect::CreateSoundBuffer(HWND w, GUID *dev)
{
	// Release the old buffer if needed
	Shutdown();

	if (dsObj.Create(w, dev))
		return -1;

	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 2;
    wf.nSamplesPerSec = synthParams.isampleRate;
	wf.nBlockAlign = wf.nChannels * 2;
    wf.wBitsPerSample = 16;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;

	sampleMax = (bsInt32) ((synthParams.sampleRate * latency) * (FrqValue)wf.nChannels);
	if (sampleMax & 1)
		sampleMax++;
	blkLen = sampleMax * 2; // two bytes per sample
	bufLen = blkLen * numBlk;
	lastBlk = bufLen - blkLen;

	DSBUFFERDESC dsbd;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	dsbd.dwBufferBytes = bufLen;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = &wf;

	if (dsObj.dirSndObj->CreateSoundBuffer(&dsbd, &dirSndBuf, NULL) != S_OK)
		return -1;

	ClearBuffer();

	nextWrite = 0;
	// when we must wait, we wait 1/4 of a block length
	// ms = latency * 0.25 * 1000
	pauseTime = (DWORD) (latency * 250.0f);

	dsObj.AddRef();

	return 0;
}

int WaveOutDirect::Setup(HWND w, float leadtm, int nb, GUID *dev)
{
	if (leadtm == 0.0)
	{
		latency = 0.02f;
		numBlk = 3;
	}
	else
	{
		latency = leadtm;
		if ((numBlk = nb) < 3)
			numBlk = 3;
	}

	if (CreateSoundBuffer(w, dev))
		return -1;

	// Lock the first block
	if (dirSndBuf->Lock(0, blkLen, &startLock, &sizeLock, NULL, NULL, 0) != S_OK)
		return -1;
	outState = 1;
	channels = 2;
	samples = (SampleValue *) startLock;
	nxtSamp = samples;
	endSamp = nxtSamp + sampleMax;
	ownBuf = 0;

	return 0;
}

void WaveOutDirect::Stop()
{
	if (dirSndBuf && outState != 0)
	{
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->Stop();
	}
	outState = 0;
}

void WaveOutDirect::Restart()
{
	if (dirSndBuf)
	{
		// Lock the first block
		ClearBuffer();
		dirSndBuf->SetCurrentPosition(0);
		dirSndBuf->Lock(0, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		nxtSamp = (SampleValue*)startLock;
		endSamp = nxtSamp + sampleMax;
		nextWrite = 0;
		outState = 1;
	}
}

void WaveOutDirect::Shutdown()
{
	if (dirSndBuf)
	{
		Stop();
		dirSndBuf->Release();
		dirSndBuf = 0;
		dsObj.Release();
	}
	outState = 0;
}

// NB: we always leave this function with a portion of the
// buffer locked. Since samples are going directly to the buffer,
// it is imperative to do that.
// This next flag controls whether the code sleeps or spins when the
// buffer is full. Depending on the CPU load, it might be better to
// spin because we might not get the CPU back when we want it.
#define SLEEP_WHEN_BLOCKED 1
int WaveOutDirect::FlushOutput()
{
	DWORD m, pl;
	switch (outState)
	{
	case 0:
		// suspended state...
		Sleep(pauseTime);
		break;
	case 1:
		// Initial write. Start playback
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->SetCurrentPosition(0);
		dirSndBuf->Play(0, 0, DSBPLAY_LOOPING);
		nextWrite = blkLen;
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		outState = 2;
		break;
	case 2:
		// filling buffer first time through
		nextWrite += blkLen;
		if (nextWrite < bufLen)
		{
			dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
			dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
			break;
		}
		nextWrite = 0;
		outState = 3;
		// FALLTHROUGH
	case 3:
		// Writing behind playback; wait for play position in next block
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		m = nextWrite + blkLen;
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		while (pl < m)
		{
#if SLEEP_WHEN_BLOCKED
			Sleep(pauseTime);
#endif
			dirSndBuf->GetCurrentPosition(&pl, NULL);
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		nextWrite = m;
		if (nextWrite >= lastBlk)
			outState = 4;
		break;
	case 4:
		// Writing to last block; wait for play position in first block
		dirSndBuf->Unlock(startLock, sizeLock, 0, 0);
		dirSndBuf->GetCurrentPosition(&pl, NULL);
		while (pl > blkLen)
		{
#if SLEEP_WHEN_BLOCKED
			Sleep(pauseTime);
#endif
			dirSndBuf->GetCurrentPosition(&pl, NULL);
		}
		dirSndBuf->Lock(nextWrite, blkLen, &startLock, &sizeLock, NULL, NULL, 0);
		nextWrite = 0;
		outState = 3;
		break;
	}
	nxtSamp = (SampleValue*)startLock;
	endSamp = nxtSamp + sampleMax;
	return 0;
}

/////////////////////////////////////////////////////////

WaveOutDirectI::WaveOutDirectI()
{
	latency = 0.015f;
	numBlk = 3;
}

int WaveOutDirectI::Setup(HWND w, float leadtm, int nb, GUID *dev)
{
	// the minimum latency varies with processor speed
	// for Pentium 4 on XP, 20ms is about as small as you
	// should go.
	if (leadtm == 0.0f)
		latency = 0.020f;
	else
		latency = leadtm;
	if (nb < 3)
		numBlk = 3;
	else
		numBlk = nb;
	if (CreateSoundBuffer(w, dev))
		return -1;
	AllocBuf(sampleMax, 2);

	dirSndBuf->SetCurrentPosition(0);
	dirSndBuf->Play(0, 0, DSBPLAY_LOOPING);

	return 0;
}

void WaveOutDirectI::Stop()
{
	if (dirSndBuf)
		dirSndBuf->Stop();
}

void WaveOutDirectI::Restart()
{
	if (dirSndBuf)
	{
		ClearBuffer();
		dirSndBuf->SetCurrentPosition(0);
		dirSndBuf->Play(0, 0, DSBPLAY_LOOPING);
		nextWrite = 0;
		nxtSamp = samples;
	}
}

int WaveOutDirectI::FlushOutput()
{
	DWORD pl, wr;

	BYTE *wrBuf = (BYTE*)samples;
	DWORD wrPos = nextWrite;
	DWORD toWrite = blkLen;
	DWORD writeNow;
	DWORD toSleep = 4;
	DWORD bufstat = 0;
	while (dirSndBuf)
	{
		dirSndBuf->GetStatus(&bufstat);
		if (!(bufstat & DSBSTATUS_PLAYING))
			break;
		if (bufstat & DSBSTATUS_BUFFERLOST)
		{
			if (dirSndBuf->Restore())
				break;
			ClearBuffer();
		}
		dirSndBuf->GetCurrentPosition(&pl, &wr);
		if (pl < wrPos)
			writeNow = (pl + bufLen) - wrPos;
		else
			writeNow = pl - wrPos;
		if (writeNow > toWrite)
			writeNow = toWrite;
		if (writeNow > 0)
		{
			void *start1 = 0;
			void *start2 = 0;
			DWORD size1 = 0;
			DWORD size2 = 0;
			if (dirSndBuf->Lock(wrPos, writeNow, &start1, &size1, &start2, &size2, 0) != S_OK)
				break;
			memcpy(start1, wrBuf, size1);
			toWrite -= size1;
			wrBuf += size1;
			wrPos += size1;
			if (start2 != NULL)
			{
				memcpy(start2, wrBuf, size2);
				toWrite -= size2;
				wrBuf += size2;
				wrPos += size2;
			}
			dirSndBuf->Unlock(start1, size1, start2, size2);
			if (wrPos >= bufLen)
				wrPos -= bufLen;
		}
		if (toWrite > 0)
			Sleep(toSleep);
		else
			break;
	}
	nextWrite += blkLen;
	if (nextWrite >= bufLen)
		nextWrite = 0;
	nxtSamp = samples;
	return 0;
}
