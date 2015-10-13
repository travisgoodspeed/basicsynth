/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDIControl.h MIDI Control classes
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef MIDICONTROL_H
#define MIDICONTROL_H

/// MIDI Channel status information.
/// This class stores information for one MIDI channel.
/// It has members to hold the raw MIDI values for
/// channel voice messages and channel control messages.
class MIDIChannelStatus
{
public:
	bsInt16 no[128];      ///< note on
	bsInt16 cc[128];      ///< controller values
	bsInt16 at[128];      ///< polyphonic after touch
	bsInt16 bank;         ///< bank number
	bsInt16 patch;        ///< patch (aka program)
	bsInt16 aftertouch;   ///< channel pressure
	bsInt16 pitchbend;    ///< pitch bend
	bsInt16 pbsemi;       ///< pitch bend resolution, semi-tones
	bsInt16 pbcents;      ///< pitch bend resolution, cents
	bsInt16 mwsemi;       ///< mod wheel sensitivity, semi-tones
	bsInt16 mwcents;      ///< mod wheel sensitivity, cents
	bsInt16 finetune;     ///< fine tuning, cents
	bsInt16 coarsetune;   ///< course tuning, semi-tones
	bsInt16 rpnnum;       ///< currently selected RPN parameter
	bsInt16 enable;       ///< channel is enabled to process messages
	bsInt16 channel;      ///< channel number, for reference

	MIDIChannelStatus();

	/// Reset to initial values.
	/// Note: this is not the same as MIDI reset.
	void Reset();
};

/// MIDI channel control information.
/// This class aggregates 16 MIDIChannelStatus objects
/// and provides processing of MIDI input. 
/// This class does not handle Note-on and note-off
/// since those messages are typically processed by a 
/// Sequencer object. However, it is possible to derive a class
/// from MIDIControl and make it function like a sequencer.
///
/// The channel status objects are public and can be accessed 
/// directly. However, the preferred method is to call
/// one of the convenience functions on the MIDIControl
/// object.
///
/// Functions with 'N' return normalized values in the
/// range [0,1]. Functions with 'CB' return centibels
/// of attenation, range [0,960] where 0 is no attenuation and
/// 960 is silence. Functions with 'C' return pitch cents.
///
/// A MIDI-aware instrument should be passed a reference
/// to a MIDIControl object when then instrument is created.
/// During playback, the instrument can retrieve various
/// performance controller parameters at start time
///
/// The instrument manager inherits from this class and implements
/// the VolumeChange(), PitchbendChange() and ControlChange() functions
/// to send changes to the mixer and active voices.
class MIDIControl
{
public:
	MIDIChannelStatus channel[16];  ///< Channel status information
	bsInt16 switchLevel;

	MIDIControl();

	/// Process an event from the sequencer.
	virtual void ProcessEvent(SeqEvent *evt, bsInt16 flags);
	/// Process a MIDI message.
	virtual void ProcessMessage(short mm, short v1, short v2);
	/// Process a MIDI control change.
	virtual void ControlChange(int chnl, int ccx, int val) { }
	/// Process a MIDI pitch wheel change.
	virtual void PitchbendChange(int chnl, int v1, int v2) { }
	/// Process a MIDI channel pressure change.
	virtual void AftertouchChange(int chnl, int val) { }
	/// Process a MIDI note on message.
	virtual void NoteOn(short chnl, int key, int vel) { }
	/// Process a MIDI note off message.
	virtual void NoteOff(short chnl, int key, int vel) { }

	/// Reset all channels to initial values.
	void Reset()
	{
		for (int ch = 0; ch < 16; ch++)
		{
			channel[ch].channel = ch;
			channel[ch].Reset();
		}
		channel[9].bank = 128;
	}

	void EnableChannel(int ch, int enable)
	{
		channel[ch].enable = enable;
	}

	/// Convenience functions

	/// Get pitchbend (a/k/a pitch wheel) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns current pitch bend [0,16384]
	inline FrqValue GetPitchbend(int chnl)
	{
		return channel[chnl].pitchbend;
	}

	/// Get normalized pitchbend.
	/// @param chnl MIDI channel number (0-15)
	/// @returns current pitch bend normalized to [-1,+1]
	inline FrqValue GetPitchbendN(int chnl)
	{
		return FrqValue(channel[chnl].pitchbend - 8192) / 8192.0;
	}

	/// Get pitchbend in cents. 
	/// Scales normalized pitchbend with RPN0.
	/// @param chnl MIDI channel number (0-15)
	/// @returns pitchbend in cents
	inline FrqValue GetPitchbendC(int chnl)
	{
		return GetPitchbendN(chnl) * GetPitchbendRange(chnl);
	}

	/// Get pitchbend range in cents. 
	/// @param chnl MIDI channel number (0-15)
	/// @returns pitch bend range (RPN0)
	inline FrqValue GetPitchbendRange(int chnl)
	{
		return FrqValue((channel[chnl].pbsemi * 100) + channel[chnl].pbcents);
	}

	/// Get volume.
	/// @param chnl MIDI channel number (0-15)
	/// @returns CC7 value
	inline bsInt16 GetVolume(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_VOL];
	}

	/// Set volume (CC7) value.
	/// @param chnl MIDI channel number (0-15)
	/// @param val volume level [0,127]
	inline void SetVolume(int chnl, int val)
	{
		channel[chnl].cc[MIDI_CTRL_VOL] = val;
		ControlChange(chnl, MIDI_CTRL_VOL, val);
	}

	/// Get scaled volume.
	/// @param chnl MIDI channel number (0-15)
	/// @returns CC7 value normalized to [0,1]
	inline AmpValue GetVolumeN(int chnl)
	{
		return AmpValue(GetVolume(chnl)) / 127.0;
	}

	/// Get scaled volume.
	/// @param chnl MIDI channel number (0-15)
	/// @returns CC7 value in centibels [0,960]
	inline AmpValue GetVolumeCB(int chnl)
	{
		return GetVolumeN(chnl) * 960.0;
	}

	/// Get modulation wheel (CC1) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns current mod wheel [0,127]
	inline bsInt16 GetModwheel(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_MOD];
	}

	/// Get normalized modulation wheel (CC1) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns current mod wheel [0,1]
	inline AmpValue GetModwheelN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_MOD]) / 127.0;
	}

	/// Get scaled modulation wheel (CC1) value.
	/// This scales the mod wheel by RPN3
	/// @param chnl MIDI channel number (0-15)
	/// @returns current mod wheel [0,RPN3]
	inline AmpValue GetModwheelCents(int chnl)
	{
		return GetModwheelN(chnl) * GetModwheelRange(chnl);
	}

	/// Get modulation wheel (CC1) range.
	/// The mod wheel range is set by RPN3.
	/// @param chnl MIDI channel number (0-15)
	/// @returns current mod wheel range (RPN3)
	inline AmpValue GetModwheelRange(int chnl)
	{
		return AmpValue((channel[chnl].mwsemi * 100) + channel[chnl].mwcents);
	}

	/// Get expression controller (CC11) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns expression controller [0,127]
	inline bsInt16 GetExpr(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_EXPR];
	}

	/// Get normalize expression controller (CC11).
	/// @param chnl MIDI channel number (0-15)
	/// @returns expression controller [0,1]
	inline AmpValue GetExprN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_EXPR]) / 127.0;
	}

	/// Get scaled expression controller (CC11) in centibels.
	/// @param chnl MIDI channel number (0-15)
	/// @returns expression controller in centibels [0,960]
	inline AmpValue GetExprCB(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_EXPR]) * 960.0;
	}

	/// Get key aftertouch (polyphonic key pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @param key MIDI key number
	/// @returns aftertouch [0,127]
	inline bsInt16 GetKeyAftertouch(int chnl, int key)
	{
		return channel[chnl].at[key&0x7f];
	}

	/// Get normalized key aftertouch (polyphonic key pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @param key MIDI key number
	/// @returns aftertouch [0,1]
	inline AmpValue GetKeyAftertouchN(int chnl, int key)
	{
		return AmpValue(GetKeyAftertouch(chnl, key)) / 127.0;
	}

	/// Get scaled key aftertouch (polyphonic key pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @param key MIDI key number
	/// @returns aftertouch in centibels [0,960]
	inline AmpValue GetKeyAftertouchCB(int chnl, int key)
	{
		return GetKeyAftertouchN(chnl, key) * 960.0;
	}

	/// Get aftertouch (channel pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @returns aftertouch [0,127]
	inline bsInt16 GetAftertouch(int chnl)
	{
		return channel[chnl].aftertouch;
	}

	/// Get normalized aftertouch (channel pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @returns aftertouch [0,1]
	inline AmpValue GetAftertouchN(int chnl)
	{
		return AmpValue(channel[chnl].aftertouch) / 127.0;
	}

	/// Get scaled aftertouch (channel pressure).
	/// @param chnl MIDI channel number (0-15)
	/// @returns aftertouch in centibels [0,960]
	inline AmpValue GetAftertouchCB(int chnl)
	{
		return GetAftertouchN(chnl) * 960.0;
	}

	/// Get current patch (program) number.
	/// @param chnl MIDI channel number (0-15)
	/// @returns patch number (0-127)
	inline bsInt16 GetPatch(int chnl)
	{
		return channel[chnl].patch;
	}

	/// Get current patch (program) number.
	/// @param chnl MIDI channel number (0-15)
	/// @param patch number (0-127)
	inline void SetPatch(int chnl, int patch)
	{
		channel[chnl].patch = patch;
	}

	/// Get the bank setting.
	/// This follows the GM/DLS spec of using
	/// either bank MSB or channel to determine
	/// the Sound bank number where 128 is the
	/// percussion bank and 0 is the primary
	/// melodic bank. To get the
	/// actual bank settings (for bank mapping)
	/// call GetCC() for MIDI_CTRL_BANK and
	/// MIDI_CTRL_BANK_LSB.
	/// @param chnl channel number
	/// @returns Sound bank number.
	inline bsInt16 GetBank(int chnl)
	{
		bsInt16 bmsb = channel[chnl].cc[MIDI_CTRL_BANK];
		if (bmsb == 0x78)  // GM percussion bank
			return 128;
		if (bmsb == 0x79) // GM melodic bank
			return channel[chnl].cc[MIDI_CTRL_BANK_LSB];
		if (chnl == 9)
			return 128;
		return 0;
	}

	/// Set the sound bank number.
	/// See GetBank().
	/// @param chnl channel number
	/// @param bank Sound bank number.
	inline void SetBank(int chnl, int bank)
	{
		if (bank == 128)
		{
			channel[chnl].cc[MIDI_CTRL_BANK] = 0x78;
			channel[chnl].cc[MIDI_CTRL_BANK_LSB] = 0;
		}
		else
		{
			channel[chnl].cc[MIDI_CTRL_BANK] = 0x79;
			channel[chnl].cc[MIDI_CTRL_BANK_LSB] = bank;
		}
		channel[chnl].bank = bank;
	}

	/// Get pan (CC10) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns pan value [0-127]
	inline bsInt16 GetPan(int chnl)
	{
		return channel[chnl].cc[MIDI_CTRL_PAN];
	}

	/// Set pan (CC10) value.
	/// @param chnl MIDI channel number (0-15)
	/// @param val value [0-127]
	inline void SetPan(int chnl, int val)
	{
		channel[chnl].cc[MIDI_CTRL_PAN] = val;
		ControlChange(chnl, MIDI_CTRL_PAN, val);
	}

	/// Get normalized pan (CC10) value.
	/// @param chnl MIDI channel number (0-15)
	/// @returns pan value [-1,+1]
	inline AmpValue GetPanN(int chnl)
	{
		return AmpValue(channel[chnl].cc[MIDI_CTRL_PAN] - 64) / 64.0;
	}

	/// Get coarse tune value (RPN2).
	/// @param chnl MIDI channel number (0-15)
	/// @returns fine tune value [0-127]
	inline bsInt32 GetFineTune(int chnl)
	{
		return channel[chnl].finetune;
	}

	/// Get coarse tune value (RPN1).
	/// @param chnl MIDI channel number (0-15)
	/// @returns coarse tune value [0-127]
	inline bsInt32 GetCoarseTune(int chnl)
	{
		return channel[chnl].coarsetune;
	}

	/// Get CC switch value.
	/// The controller value is compared against
	/// the switch threshold.
	/// @param chnl MIDI channel number (0-15)
	/// @param sw MIDI switch controller number
	/// @returns 1 if switch is on, 0 if off
	inline bsInt16 GetSwitch(int chnl, int sw)
	{
		return channel[chnl].cc[sw] > switchLevel;
	}

	/// Get CC value.
	/// @param chnl MIDI channel number (0-15)
	/// @param ccx MIDI controller number (0-127)
	/// @returns control value [0-127]
	inline bsInt16 GetCC(int chnl, int ccx)
	{
		return channel[chnl].cc[ccx];
	}

	/// Set CC value.
	/// @param chnl MIDI channel number (0-15)
	/// @param ccx MIDI controller number (0-127)
	/// @param val Controller value (0-127)
	inline void SetCC(int chnl, int ccx, int val)
	{
		ProcessMessage(MIDI_CTLCHG|chnl, ccx, val);
	}

	/// Get normalized CC value.
	/// @param chnl MIDI channel number (0-15)
	/// @param ccx MIDI controller number (0-127)
	/// @returns control value scaled to [0-1]
	inline AmpValue GetCCN(int chnl, int ccx)
	{
		return AmpValue(channel[chnl].cc[ccx]) / 127.0;
	}
};

//@}

#endif
