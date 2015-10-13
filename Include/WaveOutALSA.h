///////////////////////////////////////////////////////////
// BasicSynth - 
//
/// @file WaveOutALSA.h Send samples to the sound card using the ALSA driver
/// @note You must install ALSA development libraries to compile this file.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef WAVEOUTALSA_H
#define WAVEOUTALSA_H
#include <alsa/asoundlib.h>

/// @brief Output samples to the sound card using ALSA
/// @details WaveOutALSA provides immediate sound output
/// for Linux versions of BasicSynth. Typically this class
/// should be created on a background thread and passed
/// to the instrument manager as the sample output object.
/// Consequently, there is nothing very sophisticated here.
///
/// When the output buffer is full, we pass the samples
/// to the driver and assume the driver will block us when
/// we need to wait for prior output to complete. 
///
/// To use this class, create an instance and then call
/// Setup(...) to open and prepare the device using the
/// current synthParams sample rate. We always do two
/// output channels. The "leadtm" value indicates how
/// much output is generated before calling the driver
/// and thus represents the synthesizer's latency. Typical
/// lead times are in the range 200-500ms for sequencing
/// and 20-50ms when playing a keyboard or other interactive device.
/// The number of blocks (nb) indicates a multiplier
/// that determines the latency value to snd_pcm_open.
/// For example, leadtm=0.5 and nb=2 means we generate
/// 1/2 second output before calling the driver and the
/// driver will maintain a 1 second buffer.
class WaveOutALSA : public WaveOutBuf
{
private:
	snd_pcm_t *handle;

public:
	WaveOutALSA()
	{
		handle = 0;
	}
	
	~WaveOutALSA()
	{
		if (handle)
			snd_pcm_close(handle);
	}
	
	/// Setup for output.
	/// @param device name of the sound device ("default" usually works fine.)
	/// @param leadtm lead time in seconds (latency)
	/// @param nb number of blocks to allocate in the driver (>0)
	/// @returns 0 on success, < 0 for error
	int Setup(char *device, float leadtm, int nb=1)
	{
		if (leadtm == 0.0)
		{
			leadtm = 0.005;
			nb = 1;
		}
		else if (nb < 1)
			nb = 1;
		if (AllocBuf((long)(leadtm * synthParams.sampleRate) * 2, 2))
			return -1;
		
		int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0);
		if (err == 0)
		{
			err = snd_pcm_set_params(handle, 
					SND_PCM_FORMAT_S16, 
					SND_PCM_ACCESS_RW_INTERLEAVED,
					2, // channels
					synthParams.isampleRate,
					1, // allow resample
					(unsigned int) (leadtm * 1000000.0) * nb);
			if (err == 0)
				err = snd_pcm_prepare(handle);
			if (err < 0)
			{
				snd_pcm_close(handle);
				handle = 0;
			}
		}
		return err;
	}
	
	/// Stop the sound output.
	void Stop()
	{
		if (handle && snd_pcm_state(handle) == SND_PCM_STATE_RUNNING)
			snd_pcm_drop(handle);
	}
	
	/// Restart sound output.
	void Restart()
	{
		if (!handle)
			return;
		snd_pcm_state_t st = snd_pcm_state(handle);
		if (st == SND_PCM_STATE_SETUP)
			snd_pcm_prepare(handle);
		if (st == SND_PCM_STATE_PREPARED)
			snd_pcm_start(handle);
		nxtSamp = samples;
	}

	/// Wait for output to finish.
	void Drain()
	{
		if (!handle)
			return;
		if (nxtSamp > samples)
			FlushOutput();
		snd_pcm_drain(handle);
	}

	/// Drain the output and close the device.
	/// This waits for output to complete. If you just want
	/// to stop all output immediately, call Stop() then
	/// delete this object.
	void Shutdown()
	{
		if (handle)
		{
			Drain();
			snd_pcm_close(handle);
			handle = 0;
		}
	}
	
	/// Flush output.
	/// This overrides the base class method and copies the output
	/// buffer to the ALSA driver. We use a fairly simple strategy
	/// that blocks in the driver until the output can be completed.
	/// That causes output to synchronize to the sample rate. This is
	/// OK since this code should probably run in a background thread
	/// anyway. If something bad happens during a write, we throw away
	/// this block and blaze onward.
	int FlushOutput()
	{
		SampleValue *ptr = samples;
		snd_pcm_sframes_t towrite = (int)(nxtSamp - samples) / channels;
		snd_pcm_sframes_t frames = 0;
		while (towrite > 0) 
		{
			frames = snd_pcm_writei(handle, ptr, towrite);
			if (frames < 0) 
			{
				if (frames == -EPIPE)
				{	// underrun - try to recover
					frames = snd_pcm_prepare(handle);
					if (frames < 0)
						break;
				}
				// we could be suspended, or some other funky state...
				// all we can do is throw away the output and assume the 
				// higher level code knows what is going on.
				break;
			}
			ptr += frames * channels;
			towrite -= frames;
		}
		nxtSamp = samples;
		return 0;
	}
};

//@}
#endif
