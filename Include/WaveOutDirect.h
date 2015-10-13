///////////////////////////////////////////////////////////
// BasicSynth -
//
/// @file WaveOutDirect.h Send samples to the sound card using DirectSound
/// @note Microsoft does NOT include the DirectX SDK with the compiler by
/// default. You have to download the separate (and huge) SDK in order to
/// get the dsound.h and dsound.lib bits.
//
// We have two options:
// 1. Indirect - samples are put into a local buffer and then
//             copied to the direct sound buffer when the local
//             buffer is filled.
// 2. Direct - samples are written into the direct sound buffer.
//
// The direct method is slightly faster, but the indirect
// method is safer since it always has a valid buffer.
//
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{

#ifndef _WAVEOUTDIRECT_H
#define _WAVEOUTDIRECT_H 1
#include <dsoundintf.h>


/// Sound output to DAC.
/// Ths class uses uses the Windows DirectSound buffer
/// as the sample buffer. The buffer is divided into
/// multiple short blocks. While one block is playing,
/// other blocks can be filled in preparation. For
/// interactive playing the lead time (block size) should
/// be short (ca. 20-50ms) and the number of blocks kept
/// small (3-6) to avoid latency problems.
/// If playing a sequence, the time can be longer
/// to allow for more lead time.
/// NOTE: This class is not recommeded if there is
/// any chance of a buffer lost condition. It assumes
/// it can always lock some portion of the buffer!
class WaveOutDirect : public WaveOutBuf
{
protected:
	IDirectSoundBuffer *dirSndBuf;

	float latency;
	DWORD numBlk;
	DWORD nextWrite;
	DWORD blkLen;
	DWORD bufLen;
	DWORD lastBlk;
	void *startLock;
	DWORD sizeLock;
	DWORD pauseTime;
	int outState;

	int CreateSoundBuffer(HWND w, GUID *dev);
	void ClearBuffer();

public:

	WaveOutDirect();
	virtual ~WaveOutDirect();
	/// Setup the sound output buffer.
	/// @param wnd window handle to pass to DirectSound for collaboration
	/// @param leadtm block length in seconds
	/// @param nb number of blocks (>=3)
	/// @param dev GUID for device, NULL to use primary device
	virtual int Setup(HWND wnd, float leadtm, int nb = 4, GUID *dev = 0);
	/// Stop the sound output.
	virtual void Stop();
	/// Restart sound output.
	virtual void Restart();
	/// Destroy the DirectSound buffer.
	virtual void Shutdown();

	/// Flush output.
	/// This overrides the base class method and is the point
	/// where we need to switch to another block in the sound
	/// buffer. We don't actually flush anything since we are writing directly
	/// to the DirectSoundBuf data. We move the pointer forward
	/// to the next block of samples in the buffer.
	/// This has the side-effect of waiting on playback
	/// to move out of the way, and causes output to be
	/// synchronized with sample rate.
	virtual int FlushOutput();
};

/// Ths class uses an indirect buffer write.
/// Samples are put into a local buffer then
/// copied during FlushOutput. This is safer
/// than WaveOutDirect since we always have
/// a valid buffer to write into. In practie,
/// the performance is as good as the WaveOutDirect
/// class, and this should be used in most cases.
class WaveOutDirectI : public WaveOutDirect
{
public:
	WaveOutDirectI();
	virtual ~WaveOutDirectI() { }
	/// @copydoc WaveOutDirect::Setup
	virtual int Setup(HWND wnd, float leadtm, int nb = 4, GUID *dev = 0);
	/// @copydoc WaveOutDirect::Stop
	virtual void Stop();
	/// @copydoc WaveOutDirect::Restart
	virtual void Restart();
	/// Flush output.
	/// This overrides the base class method and is called at the point
	/// where we need to dump the output buffer to the
	/// DirectSound buffer. If the DirectSound buffer cannot
	/// receive more data, we block waiting on available space.
	/// This has the side-effect of waiting on playback
	/// to move out of the way, and causes output to be
	/// synchronized with sample rate.
	virtual int FlushOutput();
};

//@}
#endif
