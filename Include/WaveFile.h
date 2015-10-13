///////////////////////////////////////////////////////////////
//
// BasicSynth - Wave file output
//
/// @file WaveFile.h Classes for wavefile managment
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpIO
//@{
#ifndef _WAVEFILE_H_
#define _WAVEFILE_H_

#include <SynthFile.h>

#ifdef BS_BIG_ENDIAN
extern SampleValue SwapSample(SampleValue x);
extern bsInt16 Swap16(bsInt16 x);
extern bsInt32 Swap32(bsInt32 x);
#else
#define SwapSample(x) x
#define Swap16(x) x
#define Swap32(x) x
#endif

/// Size of RIFF chunk
#define CHUNK_SIZE 8
/// Size of RIFF chunk ID
#define CHUNK_ID   4

#pragma pack(push, 2)

/// One chunk in a RIFF file.
struct RiffChunk
{
	/// The chunk ID
	bsInt8  chunkId[CHUNK_ID];
	/// The chunk size (little-endian byte order)
	bsInt32 chunkSize;
};

/// PCM file format data.
struct FmtData
{
	/// Format code 1 = PCM, 3 = IEEE float
	bsInt16 fmtCode;
	/// Number of channles, 1 = mono, 2 = stereo
	bsInt16 channels;
	/// Sample rate (44100Hz)
	bsInt32 sampleRate;
	/// Bytes per second (samplerate * align)
	bsInt32 avgbps;
	/// Sample alignment (channels*bits)/8
	bsInt16 align;
	/// Bits per sample
	bsInt16 bits;
};

/// Simplified WAVE file header. The chunks and format are combined
/// into a single structure.
/// @see RiffChunk FmtData
struct WavHDR
{
	RiffChunk riff;      // 'RIFF' chunk
	bsInt8  waveType[4]; // 'WAVE' type of file
	RiffChunk fmt;       // 'fmt ' chunk
	FmtData fmtdata;
	RiffChunk data;      // 'data' chunk
};

struct WavHDR32
{
	RiffChunk riff;      // 'RIFF' chunk
	bsInt8  waveType[4]; // 'WAVE' type of file
	RiffChunk fmt;       // 'fmt ' chunk
	FmtData fmtdata;
	bsInt16 cbsize;      // extra data = 0
	RiffChunk fact;      // 'fact' chunk
	bsInt32 sampleLength;
	RiffChunk data;      // 'data' chunk
};

#pragma pack(pop)

/// Interface for sample output.
/// WaveOut is a pure-virtual base class that defines the
/// methods all sample output classes must implement.
class WaveOut
{
public:
	virtual ~WaveOut() { }

	/// Put value directly to the output. This function bypasses
	/// range checking or scaling or duplication into mutiple channels.
	/// @param value the sample
	virtual void OutS(SampleValue value) = 0;

	/// Put one value into the buffer. The value is assumed normalized [-1,+1]
	/// and is checked for range then scaled to the output sample size.
	/// The value is placed directly into the buffer, not duplicated for
	/// each channel.
	/// @param value the sample
	virtual void Output(AmpValue value) = 0;

	/// Put one sample into the buffer. Write one value to all channels. This
	/// function will duplicate the sample for two channel output.
	/// Values are assumed normalized to [-1,+1] and are scaled appropriately
	/// @param value the sample
	virtual void Output1(AmpValue value) = 0;

	/// Put two values into the buffer. This function copies the values to
	/// left and right channels, or combines then into one channel as
	/// appropriate. Values are assumed normalized to [-1,+1] and are
	/// scaled appropriately.
	/// @param vleft left channel sample
	/// @param vright right channel sample
	virtual void Output2(AmpValue vleft, AmpValue vright) = 0;

	/// Get number of out-of-range samples
	virtual long GetOOR()  = 0;
	/// Reset number of out-of-range samples
	virtual void ClrOOR() = 0;

	/// Stop the sound output.
	virtual void Stop() = 0;
	/// Restart sound output.
	virtual void Restart() = 0;
	/// Shutdown the device buffer.
	virtual void Shutdown() = 0;
};

/// Base class for sample output. Samples are
/// stored in a buffer until the buffer is filled, at which time
/// a flush routine is called. For file output, the flush routine
/// dumps the samples to disk and resets the buffer. For a live
/// playback system, a derived class should send the filled buffer
/// to the DAC.
template<class ST> class WaveOutBufBase : public WaveOut
{
protected:
	bsInt32 sampleTotal;
	bsInt32 sampleNumber;
	bsInt32 sampleMax;
	bsInt32 sampleOOR;
	bsInt16 channels;
	ST *samples;
	ST *nxtSamp;
	ST *endSamp;
	int   ownBuf;

public:
	WaveOutBufBase()
	{
		channels = 0;
		sampleTotal = 0;
		sampleNumber = 0;
		sampleMax = 0;
		sampleOOR = 0;
		samples = 0;
		nxtSamp = 0;
		endSamp = 0;
		ownBuf = 0;
	}

	virtual ~WaveOutBufBase()
	{
		if (ownBuf)
			DeallocBuf();
	}

	/// Get a pointer to the output buffer. Although not declared as such,
	/// this should be treated as volatile since the buffer can get reallocated
	/// by derived classes.
	/// @return pointer to start of sample buffer
	ST *GetBuf()
	{
		return samples;
	}

	/// Allocate the sample buffer. You can invoke this directly, but
	/// usually a derived class will call this when it needs a new
	/// buffer.
	/// @note for multi-channel output, multiply the number of
	/// samples by the number of channels to set the length.
	/// @param length size of the buffer in frames
	/// @param ch number of channels
	virtual int AllocBuf(long length, bsInt16 ch)
	{
		DeallocBuf();
		samples = new ST[length];
		if (samples == NULL)
		{
			sampleMax = 0;
			return -1;
		}
		channels = ch;
		sampleMax = length;
		memset(samples, 0, length * sizeof(ST));
		sampleNumber = 0;
		sampleTotal = 0;
		nxtSamp = samples;
		endSamp = samples + length;
		ownBuf = 1;
		return 0;
	}

	/// De-allocate the sample buffer. If the buffer was set by
	/// passing a value to SetBuf() this will clear the pointer
	/// but not delete the memory for the buffer. If the buffer
	/// was allocated by AllocBuf() it is deleted.
	virtual void DeallocBuf()
	{
		if (ownBuf && samples)
			delete samples;
		samples = NULL;
		ownBuf = 0;
		sampleMax = 0;
	}

	/// Set the sample buffer. This can be used when you already
	/// have an appropriately sized buffer, for example one
	/// allocated through DirectSound.
	/// @param length size of the buffer in samples
	/// @param ch number of output channels
	/// @param bp pointer to the buffer
	virtual void SetBuf(long length, bsInt16 ch, ST *bp)
	{
		DeallocBuf();
		sampleMax = length;
		channels = ch;
		samples = bp;
		nxtSamp = samples;
		endSamp = samples + length;
	}

	/// Put one sample into the buffer. Write one value to all channels. This
	/// function will duplicate the sample for two channel output.
	/// Values are assumed normalized to [-1,+1] and are scaled appropriately
	/// @param value the sample
	virtual void Output1(AmpValue value)
	{
		Output(value);
		if (channels == 2)
			Output(value);
	}

	/// Put two values into the buffer. This function copies the values to
	/// left and right channels, or combines then into one channel as
	/// appropriate. Values are assumed normalized to [-1,+1] and are
	/// scaled appropriately.
	/// @param vleft left channel sample
	/// @param vright right channel sample
	virtual void Output2(AmpValue vleft, AmpValue vright)
	{
		if (channels == 1)
		{
			Output((vleft + vright) / 2);
		}
		else
		{
			Output(vleft);
			Output(vright);
		}
	}

	/// Flush buffer. When the sample buffer is filled this function is called.
	/// The base class does nothing but reset the next input position back
	/// to the beginning of the buffer. Derived classes must implement this
	/// and do something with the samples when the buffer is filled.
	virtual int FlushOutput()
	{
		nxtSamp = samples;
		//sampleNumber = 0;
		return 0;
	}

	/// Get the number of out-of-range samples
	virtual long GetOOR() { return sampleOOR; }
	virtual void ClrOOR() { sampleOOR = 0; }

	/// Stop the sound output.
	virtual void Stop() { }
	/// Restart sound output.
	virtual void Restart() { nxtSamp = samples; }
	/// Shutdown the device buffer.
	virtual void Shutdown() { }
};

/// Wave output class for 16-bit PCM sample output.
class WaveOutBuf : public WaveOutBufBase<SampleValue>
{
public:
	virtual void OutS(SampleValue value)
	{
	//	if (sampleNumber >= sampleMax)
	//		FlushOutput();
	//	samples[sampleNumber++] = SwapSample(value);
		if (nxtSamp >= endSamp)
			FlushOutput();
		*nxtSamp++ = SwapSample(value);
		sampleTotal++;
	}

	virtual void Output(AmpValue value)
	{
		// the out-of-range test can be removed to gain a
		// slight performance increase if you are sure
		// the values cannot go overrange, or just don't care...
		if (value > 1.0)
		{
			sampleOOR++;
			value = 1.0;
		}
		else if (value < -1.0)
		{
			value = -1.0;
			sampleOOR++;
		}
		if (nxtSamp >= endSamp)
			FlushOutput();
		*nxtSamp++ = SwapSample((SampleValue) (value * synthParams.sampleScale));
		sampleTotal++;
	}
};

/// Wave output class for 32-bit float sample output.
class WaveOutBufIEEE : public WaveOutBufBase<float>
{
public:
	virtual void OutS(SampleValue value)
	{
		Output((float)value / 32767.0);
	}

	virtual void Output(AmpValue value)
	{
		if (value > 1.0 || value < -1.0)
			sampleOOR++;
		if (nxtSamp >= endSamp)
			FlushOutput();
		*nxtSamp++ = (float)value;
		sampleTotal++;
	}
};

/// Wave file writer (PCM). WaveFile manages output to a WAV file.
/// The wave file header is automatically updated as needed.
/// A simplified model of a wave file is used. Only two chunks
/// are defined, fmt and data, and are always at the first of the
/// file. This allows us to treat the file as a fixed length
/// header followed by sample data. Sample data and is always PCM, 1 or 2 channel.
class WaveFile : public WaveOutBuf
{
private:
	FileWriteUnBuf wfp;
	WavHDR wh;
	int   bufSecs;

	void SetupWH(int ch);

public:
	WaveFile()
	{
		bufSecs = 5;
	}

	virtual ~WaveFile()
	{
		wfp.FileClose();
	}

	/// Set buffer size. The size of the buffer in specified in seconds, not samples.
	/// The buffer size must be set before the wave file is opened. Changing the size
	/// afterword has no effect.
	/// @param secs buffer size
	void SetBufSize(int secs)
	{
		bufSecs = secs;
	}

	/// Open wave output file. The file is created if it does not exist.
	/// Existing files are truncated.
	/// @param fname path to the output file
	/// @param chnls number of output channels, 1 or 2
	/// @returns 0 on success, negative value on error
	int OpenWaveFile(const char *fname, int chnls = 1);

	/// Close the wave file. Closing the file will flush remaining output, update
	/// the header and then close the file.
	/// @return 0 on success, negative on error
	int CloseWaveFile();

	/// Write output to WAVE file. This does not need to be called directly.
	/// The base class will call this method each time the buffer is filled.
	virtual int FlushOutput();
};

/// Wave file writer (32-bit float). WaveFileIEEE manages output to a WAV file.
/// The wave file header is automatically updated as needed.
/// A simplified model of a wave file is used. Only three chunks
/// are defined, fmt, fact and data, and are always at the first of the
/// file. This allows us to treat the file as a fixed length
/// header followed by sample data. Sample data and is always 32-bit float, 1 or 2 channel.
class WaveFileIEEE : public WaveOutBufIEEE
{
private:
	FileWriteUnBuf wfp;
	WavHDR32 wh;
	int   bufSecs;

	void SetupWH(short ch);

public:
	WaveFileIEEE()
	{
		bufSecs = 5;
	}

	virtual ~WaveFileIEEE()
	{
		wfp.FileClose();
	}

	/// Set buffer size. The size of the buffer in specified in seconds, not samples.
	/// The buffer size must be set before the wave file is opened. Changing the size
	/// afterword has no effect.
	/// @param secs buffer size
	void SetBufSize(int secs)
	{
		bufSecs = secs;
	}

	/// Open wave output file. The file is created if it does not exist.
	/// Existing files are truncated.
	/// @param fname path to the output file
	/// @param chnls number of output channels, 1 or 2
	/// @returns 0 on success, negative value on error
	int OpenWaveFile(char *fname, int chnls = 1);

	/// Close the wave file. Closing the file will flush remaining output, update
	/// the header and then close the file.
	/// @return 0 on success, negative on error
	int CloseWaveFile();

	/// Write output to WAVE file. This does not need to be called directly.
	/// The base class will call this method each time the buffer is filled.
	virtual int FlushOutput();
};

#define SMPL_BUFSIZE 8192

/// Wave file reader. WaveFileIn reads a WAV file into a buffer, converting samples
/// to the internal data format (i.e. AmpValue). The peak values are rescaled to
/// the maximum amplitude range. Only files in PCM format (16-bit samples, 44.1kHz)
/// are allowed. However, multiple channels are allowed, but only the first two
/// are used. These two are combined into a single channel. The resulting sample buffer is
/// always one-channel, normalized to [-1,+1].
class WaveFileIn
{
private:
	char *filename;
	bsInt16 fileID;
	AmpValue *samples;
	bsInt32 sampleTotal;
	bsInt32 loopStart;
	bsInt32 loopEnd;
	FmtData fmt;

public:
	WaveFileIn()
	{
		filename = NULL;
		samples = NULL;
		sampleTotal = 0;
		loopStart = 0;
		loopEnd = 0;
		fileID = -1;
		memset(&fmt, 0, sizeof(fmt));
	}

	~WaveFileIn()
	{
		if (samples)
			delete samples;
		if (filename)
			delete filename;
	}

	/// Get the filename for this wavefile.
	/// @returns pointer to filename member.
	const char *GetFilename()
	{
		return filename;
	}

	/// Get the file ID for this wavefile.
	/// @returns integer ID.
	bsInt16 GetFileID()
	{
		return fileID;
	}

	/// Set the file ID for this wavefile.
	/// @param newID new ID.
	/// @returns old ID.
	bsInt16 SetFileID(bsInt16 newID)
	{
		bsInt16 oldID = fileID;
		fileID = newID;
		return oldID;
	}

	/// Get the sample rate of the loaded file
	/// @returns sample rate
	bsInt32 GetSampleRate()
	{
		return fmt.sampleRate;
	}

	/// Get direct access to the samples.
	/// @return the sample buffer
	AmpValue *GetSampleBuffer()
	{
		return samples;
	}

	/// Get the number of samples in the wavefile.
	/// @return number of samples
	long GetInputLength()
	{
		return (long)sampleTotal;
	}

	/// Clear the filename, ID, and sample buffer.
	void Clear()
	{
		if (filename)
		{
			delete filename;
			filename = 0;
		}
		if (samples)
		{
			delete samples;
			samples = 0;
		}
		fileID = -1;
		sampleTotal = 0;
	}

	/// Set the loop points. This allows a user of the wavetable
	/// to associate start/end points for looping over the
	/// steady-state portion of the wavefile.
	/// @param st loop start point in samples
	/// @param end loop end point in samples
	void SetLoopPoints(bsInt32 st, bsInt32 end)
	{
		loopStart = st;
		loopEnd = end;
	}

	/// Get the loop points.
	/// @param st loop start point in samples
	/// @param end loop end point in samples
	void GetLoopPoints(bsInt32& st, bsInt32& end)
	{
		st = loopStart;
		end = loopEnd;
	}

	/// Load a wave file. This function loads the wave file and converts it
	/// to a one-channel sample buffer normalized to [-1,+1] amplitude range.
	/// @param fname path to the file
	/// @param id unique identifier for this file
	/// @return 0 on success or negative value on error
	int LoadWaveFile(const char *fname, bsInt16 id);
};

//@}
#endif
