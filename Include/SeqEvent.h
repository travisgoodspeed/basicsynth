///////////////////////////////////////////////////////////
// BasicSynth Sequencer events
//
// SeqEvent - sequencer event base class.
// NoteEvent - instrument event base class
// VarParamEvent - variable parameters class
//
/// @file SeqEvent.h Sequencer events
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
////////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{
#ifndef _SEQEVENT_H
#define _SEQEVENT_H

// Sequencer event ID values.
// The negative IDS are events internally generated
// by the sequencer. Obviously, these cannot be sent
// to the sequencer in an event since they control
// when events are processed. It is also possible
// to poll the sequencer's state to determine 
// if it is running, stopped, or paused.
#define SEQEVT_SEQSTART -1  ///< Sequencer started
#define SEQEVT_SEQSTOP  -2  ///< Sequencer stopped
#define SEQEVT_SEQPAUSE -4  ///< Sequencer paused
#define SEQEVT_SEQRESUME  -5  ///< Sequencer resumed

#define SEQEVT_START 0      ///< Start a sound
#define SEQEVT_STOP  1      ///< Stop a sound
#define SEQEVT_PARAM 2      ///< Change instrument parameters without restart
#define SEQEVT_RESTART 3    ///< Restart the sound with new parameters
#define SEQEVT_STARTTRACK 4 ///< Start a sequencer track
#define SEQEVT_STOPTRACK 5  ///< Stop a sequencer track
#define SEQEVT_CONTROL 6    ///< MIDI (or other) Control change
#define SEQEVT_CANCEL  7    ///< Cancel all notes

/// User defined events can be placed in the sequence with values
/// SEQEVT_USER or greater. The sequencer will ignore these;
/// they must be handled by setting an event callback.
#define SEQEVT_USER   1024  

// Paramater index
#define P_INUM  0           ///< Instrument number
#define P_CHNL  1           ///< Mixer channel number
#define P_START 2           ///< Start time in samples
#define P_DUR   3           ///< Duration in samples
#define P_XTRA  4           ///< First id for user params

class InstrConfig;

///////////////////////////////////////////////////////////
/// A sequencer event.
/// SeqEvent defines the minimum information needed by the
/// sequencer class. This can be used alone in cases where
/// a control function is needed. In that case, the
/// "instrument" only needs to know to start/stop operation
/// and does not need pitch, volume, duration, etc.
///
/// Events provide the coupling between the sequencer, instrument
/// manager and active instruments. The base class defines the
/// information needed by the sequencer. Each instrument derives
/// an event structure from SeqEvent, NoteEvent or VarParamEvent
/// and can add whatever additional parameters or methods 
/// the instrument needs. The event is passed to the instrument
/// for the START or PARAM signals.
///
/// A parameter ID value is the value used by the instrument
/// to identify the parameter. The first sixteen parameter IDs
/// are reserved for the sequencer.
///////////////////////////////////////////////////////////
class SeqEvent : public SynthList<SeqEvent>
{
public:
	bsInt16 type;     ///< event type, (see SEQEVT_* above)
	bsInt16 inum;     ///< instrument number
	bsInt16 chnl;     ///< channel number (usually mixer input)
	bsInt16 track;    ///< track number (formerly known as extra)
	bsInt32 evid;     ///< event ID or reference to earlier event
	bsInt32 start;    ///< start time in samples
	bsInt32 duration; ///< duration in samples
	InstrConfig *im;  ///< instrument object

	SeqEvent()
	{
		type = -1;
		inum = -1;
		evid = -1;
		track = 0;
		chnl = 0;
		start = 0;
		duration = 0;
		im = 0;
	}

	virtual ~SeqEvent() { }

	/// Destroy the event.
	/// The instrument manager and sequencer will call Destroy
	/// rather than the desctructor. Derived classes can use
	/// this feature to recycle events if desired.
	virtual void Destroy() { delete this; }

	/// Allocate space for variable parameters.
	/// @param n the number of parameters needed.
	virtual int AllocParam(bsInt16 n) { return 0; }

	/// Get the maximum parameter ID.
	virtual bsInt16 MaxParam() { return P_DUR; }

	virtual void SetInum(bsInt16 i) { inum = i; }
	virtual void SetInCfg(InstrConfig *i) { im = i; }
	virtual void SetType(bsInt16 t) { type = t; }
	virtual void SetID(bsInt16 id) { evid = id; }
	virtual void SetChannel(bsInt16 c) { chnl = c; }
	virtual void SetTrack(bsInt16 t) { track = t; }
	virtual void SetStart(bsInt32 s) { start = s; }
	virtual void SetDuration(bsInt32 d) { duration = d; }

	/// Set a parameter.
	/// @param id unique id number for this value
	/// @param v the parameter value
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_INUM:
			SetInum((bsInt16) v);
			break;
		case P_CHNL:
			SetChannel((bsInt16) v);
			break;
		case P_START: // in seconds
			SetStart((bsInt32) (synthParams.sampleRate * v));
			break;
		case P_DUR: // in seconds
			SetDuration((bsInt32) (synthParams.sampleRate * v));
			break;
		}
	}

	/// Set parameter from a string. This is a convenience
	/// for sequencer file readers.
	/// @param id unique id number for this value
	/// @param s the parameter value
	virtual void SetParam(bsInt16 id, char *s)
	{
		SetParam(id, (float) bsString::StrToFlp(s));
	}

	/// Get a parameter.
	/// @param id unique id number for this value
	/// @return the parameter value
	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_INUM:
			return (float) inum;
		case P_CHNL:
			return (float) chnl;
		case P_START:
			return (float) start / synthParams.sampleRate;
		case P_DUR:
			return (float) duration / synthParams.sampleRate;
		}
		return 0;
	}

	/// Reset values to defaults.
	virtual void Reset()
	{
		inum = 0;
		chnl = 0;
		start = 0;
		duration = 0;
		track = 0;
	}

	virtual void CopyEvent(SeqEvent *evt)
	{
		inum = evt->inum;
		chnl = evt->chnl;
		start = evt->start;
		duration = evt->duration;
		track = evt->track;
	}
};

#define P_PITCH  4       ///< The pitch value (0-127)
#define P_FREQ   5       ///< The frequency value (0-SR/2)
#define P_VOLUME 6       ///< The volume (0-1)
#define P_TRACK  7       ///< Track number
#define P_NOTEONVEL 8    ///< Note-on velocity (if MIDI)
#define P_PITCHBEND 9    ///< MIDI Pitch bend
#define P_CHNLPRESS 10   ///< MIDI channel pressure
#define P_BANK   11
#define P_PATCH  12
#define P_USER   16      ///< First instrument specific parameter

///////////////////////////////////////////////////////////
/// A note event.
/// The NoteEvent structure adds pitch, frequency and volume
/// to the pre-defined event parameters. Instruments that 
/// are to be used with Notelist should derive their events
/// from the NoteEvent class instead of SeqEvent.
/// ID numbers up to P_USER are reserved for future use.
/// @sa SeqEvent
///////////////////////////////////////////////////////////
class NoteEvent : public SeqEvent
{
public:
	bsInt16 pitch;
	bsInt16 noteonvel;
	bsInt16 bank;
	bsInt16 patch;
	FrqValue pitchbend;
	AmpValue chnlpress;
	FrqValue frq;
	AmpValue vol;

	NoteEvent()
	{
		pitch = 0;
		noteonvel = 0;
		bank = 0;
		patch = 0;
		pitchbend = 0.0;
		chnlpress = 0.0;
		frq = 0.0;
		vol = 1.0;
	}

	/// @copydoc SeqEvent::MaxParam
	virtual bsInt16 MaxParam() { return P_USER-1; }
	virtual void SetFrequency(FrqValue f) { frq = f; }
	virtual void SetPitch(bsInt16 p) 
	{
		SetFrequency(synthParams.GetFrequency(pitch = p));
	}
	virtual void SetVolume(AmpValue v) { vol = v; }
	virtual void SetVelocity(bsInt16 v) { noteonvel = v; }
	virtual void SetBank(bsInt16 b) { bank = b; }
	virtual void SetPatch(bsInt16 p) { patch = p; }
	virtual void SetPitchbend(FrqValue v) { pitchbend = v; }
	virtual void SetChannelPressure(AmpValue v) { chnlpress = v; }

	/// @copydoc SeqEvent::SetParam
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_PITCH:
			SetPitch((bsInt16)v);
			break;
		case P_FREQ:
			SetFrequency(FrqValue(v));
			break;
		case P_VOLUME:
			SetVolume(AmpValue(v));
			break;
		case P_NOTEONVEL:
			SetVelocity((bsInt16)v);
			break;
		case P_TRACK:
			SetTrack((bsInt16)v);
			break;
		case P_BANK:
			SetBank((bsInt16)v);
			break;
		case P_PATCH:
			SetPatch((bsInt16)v);
			break;
		case P_PITCHBEND:
			SetPitchbend(v);
			break;
		case P_CHNLPRESS:
			SetChannelPressure(v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}

	/// @copydoc SeqEvent::GetParam
	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_PITCH:
			return (float) pitch;
		case P_FREQ:
			return (float) frq;
		case P_VOLUME:
			return (float) vol;
		case P_TRACK:
			return (float) track;
		case P_NOTEONVEL:
			return (float) noteonvel;
		case P_BANK:
			return (float) bank;
		case P_PATCH:
			return (float) patch;
		case P_PITCHBEND:
			return (float) pitchbend;
		case P_CHNLPRESS:
			return (float) chnlpress;
		}
		return SeqEvent::GetParam(id);
	}

	virtual void CopyEvent(SeqEvent *evt)
	{
		SeqEvent::CopyEvent(evt);

		NoteEvent *ne = (NoteEvent *) evt;
		pitch = ne->pitch;
		frq = ne->frq;
		vol = ne->vol;
		track = ne->track;
		noteonvel = ne->noteonvel;
		patch = ne->patch;
		bank = ne->bank;
		pitchbend = ne->pitchbend;
		chnlpress = ne->chnlpress;
	}
};


///////////////////////////////////////////////////////////
/// Variable parameter event.
/// Instruments that have variable number of parameters,
/// or have a very large number of parameters, can use
/// VarParamEvent as a class for events. The event
/// factory should set the maxParam member to the largest
/// possible ID value. 
/// @sa SeqEvent
///////////////////////////////////////////////////////////
class VarParamEvent : public NoteEvent
{
public:
	bsInt16 maxParam;
	bsInt16 allParam;
	bsInt16 numParam;
	bsInt16 *idParam;
	float *valParam;

	VarParamEvent()
	{
		maxParam = 0;
		allParam = 0;
		numParam = 0;
		idParam  = 0;
		valParam = 0;
	}

	~VarParamEvent()
	{
		if (allParam > 0)
		{
			delete idParam;
			delete valParam;
		}
	}

	/// @copydoc SeqEvent::AllocParam
	int AllocParam(bsInt16 n)
	{
		bsInt16 *ndx = new bsInt16[n];
		if (ndx == 0)
			return 0;
		float *val = new float[n];
		if (val == 0)
		{
			delete ndx;
			return 0;
		}
		if (idParam != NULL)
		{
			memcpy(ndx, idParam, allParam*sizeof(bsInt16));
			delete idParam;
		}
		if (valParam != NULL)
		{
			memcpy(val, valParam, allParam*sizeof(float));
			delete valParam;
		}
		idParam = ndx;
		valParam = val;
		allParam = n;
		return n;
	}

	/// @copydoc SeqEvent::MaxParam
	virtual bsInt16 MaxParam() 
	{ 
		return maxParam;
	}

	/// @copydoc SeqEvent::SetParam
	void SetParam(bsInt16 id, float v)
	{
		if (id <= NoteEvent::MaxParam())
			NoteEvent::SetParam(id, v);
		else if (id <= maxParam)
		{
			if (numParam >= allParam)
			{
				if (AllocParam(allParam+5) == 0)
					return;
			}
			idParam[numParam] = id;
			valParam[numParam] = v;
			numParam++;
		}
	}

	/// Reset the index count.
	void Reset()
	{
		numParam = 0;
	}

	/// @copydoc SeqEvent::GetParam
	float GetParam(bsInt16 id)
	{
		bsInt16 *pp = idParam;
		for (int n = 0; n < numParam; n++, pp++)
		{
			if (*pp == id)
				return valParam[n];
		}
		return NoteEvent::GetParam(id);
	}

	/// Replace or Set the value of a parameter
	/// @param id parameter id
	/// @param val parameter value
	void UpdateParam(bsInt16 id, float val)
	{
		if (id < P_USER)
		{
			NoteEvent::SetParam(id, val);
			return;
		}

		bsInt16 *pp = idParam;
		for (int n = 0; n < numParam; n++, pp++)
		{
			if (*pp == id)
			{
				valParam[n] = val;
				return;
			}
		}
		SetParam(id, val);
	}

	virtual void CopyEvent(SeqEvent *evt)
	{
		NoteEvent::CopyEvent(evt);

		VarParamEvent *ve = (VarParamEvent *)evt;
		bsInt16 n = ve->allParam;
		AllocParam(n);
		memcpy(valParam, ve->valParam, n*sizeof(float));
		memcpy(idParam, ve->idParam, n*sizeof(bsInt16));
		numParam = ve->numParam;
	}
};

#define P_MMSG  4         ///< The MIDI message
#define P_CTRL  5         ///< The controller or data1
#define P_CVAL  6         ///< Controller value or data2

/// A ControlEvent is used for global synthsizer control.
/// For the most part, this is used to implement MIDI
/// channel voice messages. It could be used for other
/// things as well. These events are handled by the
/// channel manager object of the Sequencer.
class ControlEvent : public SeqEvent
{
public:
	bsInt16 mmsg;
	bsInt16 ctrl;
	bsInt16 cval;

	ControlEvent()
	{
		mmsg = 0;
		ctrl = 0;
		cval = 0;
	}

	/// Get the maximum parameter ID.
	virtual bsInt16 MaxParam() { return P_CVAL; }
	virtual void SetMessage(bsInt16 m) { mmsg = m; }
	virtual void SetControl(bsInt16 c) { ctrl = c; }
	virtual void SetValue(bsInt16 v)   { cval = v; }

	/// @copydoc SeqEvent::SetParam
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_MMSG:
			SetMessage((bsInt16) v);
			break;
		case P_CTRL:
			SetControl((bsInt16) v);
			break;
		case P_CVAL:
			SetValue((bsInt16) v);
			break;
		case P_TRACK:
			SetTrack((bsInt16) v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}

	/// @copydoc SeqEvent::GetParam
	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_MMSG:
			return (float) mmsg;
		case P_CTRL:
			return (float) ctrl;
		case P_CVAL:
			return (float) cval;
		case P_TRACK:
			return (float) track;
		}
		return SeqEvent::GetParam(id);
	}

	virtual void CopyEvent(SeqEvent *evt)
	{
		SeqEvent::CopyEvent(evt);
		ControlEvent *ce = (ControlEvent*)evt;
		mmsg = ce->mmsg;
		ctrl = ce->ctrl;
		cval = ce->cval;
	}

};

#define P_TRKNO 5      ///< ID for track number
#define P_LOOP  6      ///< ID for track loop count

/// A track event starts or stops a track.
/// The loopCount member indicates how many times
/// the track is played. A value of -1 will play the
/// track until it receives a STOPTRACK event.
class TrackEvent : public SeqEvent
{
public:
	bsInt16 trkNo;
	bsInt16 loopCount;

	TrackEvent()
	{
		trkNo = -1;
		loopCount = 0;
	}

	virtual void SetTrkNo(bsInt16 tk) { trkNo = tk; }
	virtual void SetLoop(bsInt16 lc) { loopCount = lc; }

	/// Get the maximum number of parameters.
	virtual bsInt16 MaxParam() { return P_LOOP; }

	/// @copydoc SeqEvent::SetParam
	virtual void SetParam(bsInt16 id, float v)
	{
		switch (id)
		{
		case P_TRKNO:
			SetTrkNo((bsInt16) v);
			break;
		case P_LOOP:
			SetLoop((bsInt16) v);
			break;
		default:
			SeqEvent::SetParam(id, v);
			break;
		}
	}

	/// @copydoc SeqEvent::GetParam
	virtual float GetParam(bsInt16 id)
	{
		switch (id)
		{
		case P_TRKNO:
			return (float) trkNo;
		case P_LOOP:
			return (float) loopCount;
		}
		return SeqEvent::GetParam(id);
	}

	virtual void CopyEvent(SeqEvent *evt)
	{
		SeqEvent::CopyEvent(evt);
		TrackEvent *te = (TrackEvent *)evt;
		trkNo = te->trkNo;
		loopCount = te->loopCount;
	}
};

//@}
#endif
