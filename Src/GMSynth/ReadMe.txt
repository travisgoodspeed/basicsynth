========================================================================
    GMSynthDLL - plug-in synthesis modulue
========================================================================

GMSynth is a shared library (DLL) containing a complete MIDI-based synth module.
The DLL has a C++ wrapper, but can also be invoked through a function call 
interface from various programming languages, such as C, VB and C#.

See the API in GMSynthDLL.h for the function call list and C++ wrapper class.

Typical use to play a MIDI SMF file would be something like...

	GMSynthOpen(mainWindow, 44100);
	GMSynthLoadSoundBank("Sounds.sf2", 1, "Sounds");
	GMSynthLoadSequence("Song.mid", "Sounds", 0xffff);
	GMSynthGenerate("Song.wav");
	<< Wait for file complete - see below >>
	GMSynthClose(synth);

Typical use for live playing from a keyboard:

	StartSynth() {
		GMSynthOpen(mainWindow, 44100);
		GMSynthLoadSoundBank("Sounds.sf2", 1, "Sounds");
		GMSynthMIDIKbdIn(TRUE, deviceIn);
		GMSynthStart(GMSYNTH_MODE_SEQUENCE, deviceOut);
	}

	StopSynth() {
		GMSynthStop();
		GMSynthClose();
	}

The first argument to GMSynthOpen is used to set up the sound
output device. On Windows, this is a window handle to pass to
the DirectSound functions and is usually the main application window.
On Linux, this is not used. The device argument on GMSynthStart is
the platform specific device id. On Windows, this is a GUID, or NULL.
On Linux, it is the ALSA device name (e.g., "hw:0").

Sound generation is performed on a background thread.
A callback function is used to receive various events. 

	void SynthEvent(bsInt32 evtid, bsInt32 count, Opaque arg) {
		<< send message to main thread >>
	}

	GMSynthSetCallback(SynthEvent, ms, arg);

If 'ms' is non-zero, the callback is invoked every ms milliseconds
of output. This can be used to display the current time or to
synchronize screen updates with the sound playback.

The callback will be invoked from a background thread and must not
make calls to GMSynth functions. The callback function will
need to signal the main thread and return. On windows, the easiest
implementation is to post a message to the main thread's message queue.
Alternatively, and on UNIX, set an event, clear a semaphore, write to a pipe, etc.