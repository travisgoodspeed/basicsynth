///////////////////////////////////////////////////////////
/// @file dsoundintf.h Reference DirectSound.
/// @details dsound.h and dsound.lib are only included with the DirectX SDK, which is huge.
/// to avoid requiring the SDK, the parts used in BasicSynth are defined here.
/// If the DirectX SDK is installed, and you want to use it, define USE_SDK_DSOUND=1
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef USE_SDK_DSOUND
#define USE_SDK_DSOUND 0
#endif
#if USE_SDK_DSOUND
#include <dsound.h>
#pragma comment(lib, "dsound.lib")
#else
#ifndef _DSOUNDINTF_H
#define _DSOUNDINTF_H

#include <objbase.h>

#define DSSCL_NORMAL                0x00000001
#define DSSCL_PRIORITY              0x00000002
#define DSSCL_EXCLUSIVE             0x00000003
#define DSSCL_WRITEPRIMARY          0x00000004

#define DSBPLAY_LOOPING             0x00000001
#define DSBCAPS_GLOBALFOCUS         0x00008000
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000

#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
#define DSBSTATUS_LOCHARDWARE       0x00000008
#define DSBSTATUS_LOCSOFTWARE       0x00000010
#define DSBSTATUS_TERMINATED        0x00000020

#define DSBLOCK_FROMWRITECURSOR     0x00000001
#define DSBLOCK_ENTIREBUFFER        0x00000002

typedef struct _DSCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwMinSecondarySampleRate;
    DWORD           dwMaxSecondarySampleRate;
    DWORD           dwPrimaryBuffers;
    DWORD           dwMaxHwMixingAllBuffers;
    DWORD           dwMaxHwMixingStaticBuffers;
    DWORD           dwMaxHwMixingStreamingBuffers;
    DWORD           dwFreeHwMixingAllBuffers;
    DWORD           dwFreeHwMixingStaticBuffers;
    DWORD           dwFreeHwMixingStreamingBuffers;
    DWORD           dwMaxHw3DAllBuffers;
    DWORD           dwMaxHw3DStaticBuffers;
    DWORD           dwMaxHw3DStreamingBuffers;
    DWORD           dwFreeHw3DAllBuffers;
    DWORD           dwFreeHw3DStaticBuffers;
    DWORD           dwFreeHw3DStreamingBuffers;
    DWORD           dwTotalHwMemBytes;
    DWORD           dwFreeHwMemBytes;
    DWORD           dwMaxContigFreeHwMemBytes;
    DWORD           dwUnlockTransferRateHwBuffers;
    DWORD           dwPlayCpuOverheadSwBuffers;
    DWORD           dwReserved1;
    DWORD           dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef const DSCAPS *LPCDSCAPS;

typedef struct _DSBCAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwUnlockTransferRate;
    DWORD           dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

typedef const DSBCAPS *LPCDSBCAPS;

typedef struct _DSBUFFERDESC
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
    GUID            guid3DAlgorithm;
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

struct IDirectSoundBuffer;
struct IDrectSound;

typedef struct IDirectSound                 *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer           *LPDIRECTSOUNDBUFFER;

struct IDirectSound
{
    // IUnknown methods
	virtual HRESULT __stdcall QueryInterface(REFIID, void **) = 0;
	virtual ULONG   __stdcall AddRef() = 0;
    virtual ULONG   __stdcall Release() = 0;

    // IDirectSound methods
    virtual HRESULT __stdcall CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter) = 0;
    virtual HRESULT __stdcall GetCaps(LPDSCAPS pDSCaps) = 0;
    virtual HRESULT __stdcall DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) = 0;
    virtual HRESULT __stdcall SetCooperativeLevel(HWND hwnd, DWORD dwLevel) = 0;
    virtual HRESULT __stdcall Compact() = 0;
    virtual HRESULT __stdcall GetSpeakerConfig(LPDWORD pdwSpeakerConfig) = 0;
    virtual HRESULT __stdcall SetSpeakerConfig(DWORD dwSpeakerConfig) = 0;
    virtual HRESULT __stdcall Initialize(const GUID *pcGuidDevice) = 0;
};

struct IDirectSoundBuffer
{
    // IUnknown methods
    virtual HRESULT __stdcall QueryInterface(REFIID, LPVOID *) = 0;
    virtual ULONG   __stdcall AddRef() = 0;
    virtual ULONG   __stdcall Release() = 0;

    // IDirectSoundBuffer methods
    virtual HRESULT __stdcall GetCaps(LPDSBCAPS pDSBufferCaps) = 0;
    virtual HRESULT __stdcall GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) = 0;
    virtual HRESULT __stdcall GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) = 0;
    virtual HRESULT __stdcall GetVolume(LPLONG plVolume) = 0;
    virtual HRESULT __stdcall GetPan(LPLONG plPan) = 0;
    virtual HRESULT __stdcall GetFrequency(LPDWORD pdwFrequency) = 0;
    virtual HRESULT __stdcall GetStatus(LPDWORD pdwStatus) = 0;
    virtual HRESULT __stdcall Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc) = 0;
    virtual HRESULT __stdcall Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1,
                                           LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) = 0;
    virtual HRESULT __stdcall Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) = 0;
    virtual HRESULT __stdcall SetCurrentPosition(DWORD dwNewPosition) = 0;
    virtual HRESULT __stdcall SetFormat(LPCWAVEFORMATEX pcfxFormat) = 0;
    virtual HRESULT __stdcall SetVolume(LONG lVolume) = 0;
    virtual HRESULT __stdcall SetPan(LONG lPan) = 0;
    virtual HRESULT __stdcall SetFrequency(DWORD dwFrequency) = 0;
    virtual HRESULT __stdcall Stop() = 0;
    virtual HRESULT __stdcall Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) = 0;
    virtual HRESULT __stdcall Restore() = 0;
};
#endif

#endif
//@}
