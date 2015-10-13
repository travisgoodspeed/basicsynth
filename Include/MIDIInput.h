//////////////////////////////////////////////////////////////////////
/// @file MIDIInput.h MIDI keyboard input
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef MIDIINPUT_H
#define MIDIINPUT_H

#define MIDIIN_MSG_BIT(msg) (1<<((msg>>4)&7))
#define MIDIIN_MSG_ENABLE(bits,msg) bits |= MIDI_MSG_BIT(msg)
#define MIDIIN_MSG_DISABLE(bist,msg) bits &= ~MIDI_MSG_BIT(msg)

#define MIDIIN_CHNL_BIT(ch) (1<<(ch&0xf))
#define MIDIIN_CHNL_ENABLE(bits,ch) bits |= MIDI_CHNL_BIT(ch)
#define MIDIIN_CHNL_DISABLE(bits,ch) bits &= ~MIDI_CHNL_BIT(ch)

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
#endif

#ifdef UNIX
#include <pthread.h>
#include <alsa/asoundlib.h>
#endif

/// Read input from a MIDI device.
/// MIDIInput is a generic MIDI input class. When enabled, it opens
/// the device and waits for input. When a message is recieved the
/// appropriate sequencer event is created and added to the real-time
/// event queue of the sequencer.
/// Messages can be filtered by message and channel. In addition,
/// system common messages (0xf0 - 0xff) can be individually filtered.
/// The default is to process all channel voice messages on all 16
/// channels and to discard all system common messages.
class MIDIInput
{
protected:
	bsUint8 noteOn[128];
	bsUint8 noteOff[128];
	int devNum;
	bsString devName;
	Sequencer *seq;
	InstrManager *inmgr;
	bsInt16 inum;
	bsUint16 chnlMask;
	bsUint16 msgMask;
	bsUint16 sysMask;
	int midiOn;
	bsUint32 seMax;

#ifdef _WIN32
	struct MEvent
	{
		bsUint16 mmsg;
		bsUint16 val1;
		bsUint16 val2;
		bsUint32 ts;
	};

	HMIDIIN midiIn;
	HANDLE midiThreadID;
	MIDIHDR seData;
	MEvent eventBuf[500];
	int ebWrite;
	int ebRead;
	void MidiInWin(UINT uMsg, DWORD dwParam1, DWORD dwParam2);
	static void CALLBACK MidiCB(HMIDIIN hMidiIn, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
#endif

#ifdef UNIX
	bsUint8 *seData;
	pthread_t midiThreadID;
	snd_rawmidi_t *midiIn;
	void MidiInALSA();
	static void *MidiInputRaw(void *param);
#endif

public:
	MIDIInput();
	virtual ~MIDIInput();

	/// Check to see if input is active.
	virtual int  IsOn();

	/// Set the device number and/or name.
	/// Under Windows, the name is not used. Typically 
	/// a value of 0 for the device will work correctly.
	/// For ALSA, the name is used unless it is null, in
	/// which case a name of the form "hw:n" is used.
	/// @param device device number
	/// @param name device name
	virtual void SetDevice(int device, const char *name);

	/// Set the sequencer information.
	virtual void SetSequenceInfo(Sequencer *s, InstrManager *im);

	/// Set the MIDI synthesizer instrument number.
	/// This number is set in the sequencer event structure.
	/// Typically it is the instrument number for a GMManager
	/// based instrument.
	virtual void SetInstrument(bsInt16 in);

	/// Set the size of the SYSEX buffer.
	/// If the size is not set, or set to 0, no SYSEX messages
	/// will be recorded.
	virtual void SysExBuffer(bsUint32 len);

	/// @name Message filters
	/// Message filters determine which MIDI messages
	/// are processed. When a bit is set, the corresponding
	/// message is processed. When a bit is clear, the
	/// message is discarded.
	/// @{
	/// Set the message filter.
	/// Each bit represents one of the 7 channel voice messages,
	/// e.g., bit 0 represents NOTEOFF, bit 1 NOTEON, etc.
	virtual void MessageFilter(bsUint16 bits);
	/// Get the current message filter.
	virtual bsUint16 MessageFilter();
	/// Set the filter for a single message.
	virtual void FilterMessage(bsUint16 mmsg, bsUint16 enable);

	/// Set the channel filter.
	/// Each bit represents one of the 16 channels,
	/// e.g., bit 0 represents channel 1, bit 1 channel 2, etc.
	virtual void ChannelFilter(bsUint16 bits);
	/// Get the current channel filter.
	virtual bsUint16 ChannelFilter();
	/// Set the filter for a specific channel.
	virtual void FilterChannel(bsUint16 mmsg, bsUint16 enable);

	/// Set the system message filter.
	/// Each bit represents one of the 16 system common message,
	/// e.g., bit 0 represents 0xf0, bit 1 0xf1, etc.
	virtual void SystemFilter(bsUint16 bits);
	/// Get the current system filter.
	virtual bsUint16 SystemFilter();
	/// Set the filter for a specific system message.
	virtual void FilterSystem(bsUint16 mmsg, bsUint16 enable);
	/// @}

	/// Start MIDI input.
	virtual int Start();

	/// Stop MIDI input.
	virtual int Stop();

	/// Process a received MIDI message.
	/// If sequencer information is set, the ReceiveMessage
	/// function will automatically add an event to the sequencer.
	/// If the sequencer is not set, a derived class must overload
	/// the ReceiveMessage and handle the message.
	virtual void ReceiveMessage(bsUint16 mmsg, bsUint16 val1, bsUint16 val2, bsUint32 ts);

	/// Process a received SYSEX message.
	/// The default is to ignore SYSEX messages except for universal messages.
	virtual void ReceiveSysEx(const bsUint8 *data, bsUint32 size, bsUint32 xrun);
};
//@}
#endif
