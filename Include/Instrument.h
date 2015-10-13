///////////////////////////////////////////////////////////
// BasicSynth
//
/// @file Instrument.h Instrument and Instrument manager classes.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef _INSTRDEF_
#define _INSTRDEF_

class InstrManager; // forward reference for type defs below
class Sequencer;

///////////////////////////////////////////////////////////
/// Base class for instruments. This class defines
/// the common methods needed by the instrument manager and
/// sequencer. It also functions as a dummy instrument that
/// can be allocated in place of invalid instrument numbers.
/// @sa InstrFactory EventFactory TmpltFactory
///////////////////////////////////////////////////////////
class Instrument
{
public:
	virtual ~Instrument() { }
	
	/// Start output.
	/// This method is called when the current playback time
	/// matches the event start time. The instrument
	/// uses the event structure to get the per-note
	/// parameters for note initialization.
	/// @param evt the start event
	virtual void Start(SeqEvent *evt) { }
	
	/// Change parameters. 
	/// Used to alter parameters while playing.
	/// @param evt the change event
	virtual void Param(SeqEvent *evt) { }

	/// Stop output.
	/// This method is called when the duration has completed.
	/// The instrument can stop immediately, or begin
	/// the release phase of its envelope if appropriate.
	/// The instrument remains active until IsFinished
	/// returns true.
	virtual void Stop() { }

	/// Cancel note.
	/// This is a "hack" for MIDI. When EOT is reached,
	/// or we see a ALL NOTES OFF, we cancel all notes.
	/// The instrument should shutdown ASAP.
	/// By default this is just like a stop, but some
	/// instruments respond differently to a Cancel.
	virtual void Cancel() { Stop(); }

	/// Produce next sample.
	/// This method is called to generate the current sample. 
	/// Sample output is done through the instrument manager, 
	/// set by the instrument factory function.
	virtual void Tick() { }

	/// Test for output complete.
	/// IsFinished is called for each sample after Stop has been sent
	/// to determine if the instrument can be removed from
	/// the active list. 
	virtual int  IsFinished() { return 1; }

	/// Destroy the instance.
	/// By default, this deletes the instance. However, an
	/// instrument may cache instrument instances, or
	/// may implement a singleton where only one instance
	/// of the instrument ever exists. In those cases,
	/// this method would not actually delete the instance.
	virtual void Destroy() { delete this; }

	/// Load instrument settings.
	/// Called to load the instrument configuration from an XML file
	virtual int Load(XmlSynthElem *parent) { return -1; }
	
	/// Save instrument settings.
	/// Called to save the instrument configuration to an XML file
	virtual int Save(XmlSynthElem *parent) { return -1; }
};

///////////////////////////////////////////////////////////
/// Instruments that support variable parameters.
///////////////////////////////////////////////////////////
class InstrumentVP : public Instrument
{
public:
	/// Allocate a VarParamEvent object.
	virtual VarParamEvent *AllocParams() { return 0; }
	/// Get all the parameters
	virtual int GetParams(VarParamEvent *params) { return 0; }
	/// Set all the parameters
	virtual int SetParams(VarParamEvent *params) { return 0; }
	/// Set one parameter by id
	virtual int SetParam(bsInt16 idval, float val) { return 0; }
	/// Get one parameter by id
	virtual int GetParam(bsInt16 idval, float *val) { return 0; }
};

///////////////////////////////////////////////////////////
/// The InstrFactory is a static method or non-class function
/// used to instantiate the instrument. The template parameter
/// contains default settings for the instrument and is
/// opaque to the sequencer and instrument manager.
/// Typically, only this function should create new instances
/// of instruments. There are exceptions, of course.
///////////////////////////////////////////////////////////
typedef Instrument *(*InstrFactory)(InstrManager *, Opaque tmplt);

///////////////////////////////////////////////////////////
/// The EventFactory is a static method or non-class function
/// used to instantiate an event object specific to an instrument.
///////////////////////////////////////////////////////////
typedef SeqEvent *(*EventFactory)(Opaque tmplt);

///////////////////////////////////////////////////////////
/// The TmplFactory is a static method or non-class function
/// used to instantiate a template specific to an instrument.
/// This is optional. If not used, the template will be
/// created as an instance of the instrument.
///////////////////////////////////////////////////////////
typedef Opaque (*TmpltFactory)(XmlSynthElem *tmplt);

///////////////////////////////////////////////////////////
/// The TmplDump is a static method or non-class function
/// used to delete a template specific to an instrument.
/// By default, the same dump is used for all templates.
/// It casts the template to an Instrument pointer and
/// calls the destructor. If an instrument implements
/// the template factory it should also supply a companion
/// dump to get rid of templates.
///////////////////////////////////////////////////////////
typedef void (*TmpltDump)(Opaque tmplt);

///////////////////////////////////////////////////////////
/// The ParamID function is a static method or non-class function
/// used to translate a parameter name into a numeric ID.
///////////////////////////////////////////////////////////
typedef bsInt16 (*ParamID)(const char *name, Opaque tmplt);

///////////////////////////////////////////////////////////
/// The ParamName function is a static method or non-class function
/// used to translate a parameter ID into a character string.
///////////////////////////////////////////////////////////
typedef const char *(*ParamName)(bsInt16, Opaque tmplt);

///////////////////////////////////////////////////////////
/// An instrument type.
/// This class is used by the instrument manager
/// to manage one type of instrument.
///////////////////////////////////////////////////////////
class InstrMapEntry : public SynthList<InstrMapEntry>
{
public:
	bsString itype;     ///< Instrument type name.
	bsString idesc;     ///< Instrument type description.
	InstrFactory manufInstr;  ///< Create an instrument instance.
	EventFactory manufEvent;  ///< Create an instruement event.
	TmpltFactory manufTmplt;  ///< Create an instrument template.
	TmpltDump dumpTmplt;      ///< Destroy an instrument template.
	ParamID paramToID;        ///< Convert parameter name to id.
	ParamName paramToName;    ///< Convert parameter id to name.

	/// Construct a blank instrument map
	InstrMapEntry()
	{
		manufInstr = 0;
		manufEvent = 0;
		manufTmplt = 0;
		paramToID = 0;
		paramToName = 0;
	}

	/// Construct an entry with a template (used for instrument definitions)
	InstrMapEntry(const char *type, InstrFactory in, EventFactory ev)
	{
		itype = type;
		manufInstr = in;
		manufEvent = ev;
		manufTmplt = 0;
		paramToID = 0;
	}

	/// Construct an entry with factory functions (used for type definitions)
	InstrMapEntry(const char *type, InstrFactory in, EventFactory ev, TmpltFactory tf, TmpltDump td)
	{
		itype = type;
		manufInstr = in;
		manufEvent = ev;
		manufTmplt = tf;
		dumpTmplt = td;
		paramToID = 0;
	}

	~InstrMapEntry()
	{
	}

	/// Get the type value for the instrument 
	const char *GetType()
	{
		return itype;
	}

	/// Set the type value for the instrument 
	void SetType(const char *str)
	{
		itype = str;
	}

	/// Get the description value for the instrument map entry.
	const char *GetDesc()
	{
		return idesc;
	}

	/// Set the name value for the instrument map entry.
	void SetDesc(const char *str)
	{
		idesc = str;
	}

	/// Get the parameter ID for a parameter name.
	bsInt16 GetParamID(const char *name, Opaque tmplt)
	{
		if (paramToID)
			return paramToID(name, tmplt);
		return -1;
	}

	/// Get the parameter name for a parameter ID.
	const char *GetParamName(bsInt16 id, Opaque tmplt)
	{
		if (paramToName)
			return paramToName(id, tmplt);
		return "";
	}
};

///////////////////////////////////////////////////////////
/// An instrment configuration.
/// This class is used by the instrument manager
/// to manage a specific configuration of an instrument
/// type. The instance entry contains references to the
/// type and a template used to construct instrument
/// instances for playback.
///////////////////////////////////////////////////////////
class InstrConfig : public SynthList<InstrConfig>
{
public:
	
	bsInt16 inum;   ///< Instrument id number
	bsString name;  ///< Instrument name
	bsString desc;  ///< Instrument description
	Opaque instrTmplt; ///< Template to create new instance.
	InstrMapEntry *instrType; ///< Instrument type entry

	InstrConfig()
	{
		inum = -1;
		instrType = 0;
		instrTmplt = 0;
	}

	InstrConfig(bsInt16 in, InstrMapEntry *type, Opaque tmplt)
	{
		inum = in;
		instrType = type;
		instrTmplt = tmplt;
	}

	~InstrConfig()
	{
		if (instrType && instrType->dumpTmplt && instrTmplt)
			instrType->dumpTmplt(instrTmplt);
	}

	/// Create an instance object of this instrument configuration.
	/// @param im instrument manager
	/// @return pointer to the instrument
	Instrument *MakeInstance(InstrManager *im)
	{
		if (instrType)
			return instrType->manufInstr(im, instrTmplt);
		return new Instrument;
	}

	/// Create an event object for this instrument.
	/// @return pointer to the event.
	SeqEvent *MakeEvent()
	{
		if (instrType)
			return instrType->manufEvent(instrTmplt);
		return new NoteEvent;
	}

	/// Get the name value for the instrument map entry.
	const char *GetName()
	{
		return name;
	}

	/// Set the name value for the instrument map entry.
	void SetName(const char *str)
	{
		name = str;
	}

	/// Get the description value for the instrument map entry.
	const char *GetDesc()
	{
		return desc;
	}

	/// Set the name value for the instrument map entry.
	void SetDesc(const char *str)
	{
		desc = str;
	}

	/// Get the parameter ID for a parameter name.
	bsInt16 GetParamID(const char *name)
	{
		if (instrType)
			return instrType->GetParamID(name, instrTmplt);
		return -1;
	}

	/// Get the parameter name for a parameter ID.
	const char *GetParamID(bsInt16 id)
	{
		if (instrType)
			return instrType->GetParamName(id, instrTmplt);
		return "";
	}
};

///////////////////////////////////////////////////////////
/// Instrument manager class.
//
/// This class maintains lists of instrument types and 
/// instrument configurations. It is called by the sequencer
/// to allocate a new instance of the instrument when a note
/// is started, and to deallocate an instance when the note
/// is finished. 
///
/// Before playback is started, the manager must be initialized
/// with mixer and output buffer objects. 
/// The instance of the instrument manager is passed to each
/// instrument instance. Instruments output samples through
/// a method on the instrument manager, not directly to the
/// mixer or output device.
///
/// The "tmplt" argument is stored in the instrument entry
/// and passed to the instrument instance during construction.
/// Typically the "tmplt" is an instance of the instrument
/// that should be used to initialize a copy for playback,
/// but can be any data the instrument needs to use for
/// initialization.
///
/// Each instrument is identified by a unique number.
/// If a duplicate is found,
/// the instrument number is automatically adjusted.
/// When an instrument is added, callers should check the
/// return object if specific instrument numbers are needed.
/// In addition, the caller should set the name on the returned
/// object if it is desired to locate instruments by name.
///
/// The instrument manager also stores the current MIDI
/// values. Instruments that are MIDI-aware can use the
/// functions on the MIDIControl class to retrieve the
/// current values.
/// @sa MIDIControl
///////////////////////////////////////////////////////////
class InstrManager : public MIDIControl
{
protected:
	InstrConfig *instList;    ///< List of instrument configurations (templates)
	InstrMapEntry *typeList;  ///< List of instrument types
	Mixer *mix;               ///< Mixer - accumulator for instrument output
	WaveOut *wvf;             ///< Audio endpoint
	Sequencer *seq;           ///< The sequencer (when appropriate)
	bsInt16 internalID;       ///< Counter for next auto instrument ID
	Instrument *exclNotes[16*16]; ///< SF2/DLS exclusive notes 16 channels, 16 groups each

public:
	InstrManager()
	{
		instList = 0;
		typeList = 0;
		mix = 0;
		wvf = 0;
		seq = 0;
		internalID = 16384;
		for (int ch = 0; ch < 16; ch++)
			channel[ch].Reset();
		channel[9].bank = 128;
		memset(exclNotes, 0, sizeof(exclNotes));
	}

	virtual ~InstrManager() 
	{
		Clear();
		InstrMapEntry *ime;
		while ((ime = typeList) != 0)
		{
			typeList = ime->next;
			delete ime;
		}
	}

	/// Clear the instrument list. This removes all current
	/// instrument definitions. It does not remove the instrument
	/// types.
	virtual void Clear()
	{
		InstrConfig *ic;
		while ((ic = instList) != 0)
		{
			instList = ic->next;
			delete ic;
		}
		internalID = 16384;
	}

	/// Initialize the mixer and output buffer.
	/// Init MUST be called first. If you forget, 
	/// things will go very wrong very quickly.
	/// @param m mixer object
	/// @param w wave output buffer
	virtual void Init(Mixer *m, WaveOut *w)
	{
		mix =  m;
		wvf = w;
	}

	inline void SetSequencer(Sequencer *s) { seq = s; }
	inline void SetMixer(Mixer *m) { mix = m; }
	inline Mixer *GetMixer() { return mix; }
	inline void SetWaveOut(WaveOut *w) { wvf = w; }
	inline WaveOut *GetWaveOut() { return wvf; }

	/// Add an entry to the instrument type list.
	/// This method constructs the instrument type object
	/// from the supplied arguments.
	/// @param type name for this instrument type
	/// @param in instrument factory
	/// @param ev event factory
	/// @param tf template factory
	/// @return new instrument map entry for this type
	virtual InstrMapEntry *AddType(const char *type, InstrFactory in, EventFactory ev, TmpltFactory tf = 0)
	{
		InstrMapEntry *ent = new InstrMapEntry(type, in, ev, tf, 0);
		if (typeList)
			typeList->Insert(ent);
		else
			typeList = ent;
		return ent;
	}

	/// Enumerate instrument types.
	/// The first call should pass NULL for the argument. Subsequent calls
	/// should pass the last returned value.
	/// @param p previous entry
	/// @return pointer to next instrument map entry
	InstrMapEntry *EnumType(InstrMapEntry *p)
	{
		if (p)
			return p->next;
		return typeList;
	}

	/// Find the instrument map entry for a specific type.
	/// @param type name for the instrument type
	/// @return pointer to instrument map entry for this type
	virtual InstrMapEntry *FindType(const char *type)
	{
		InstrMapEntry *ent;
		for (ent = typeList; ent; ent = ent->next)
		{
			if (ent->itype.Compare(type) == 0)
				break;
		}
		return ent;
	}

	/// Add an instrument configuration.
	/// @param inum instrument number
	/// @param type instrument type
	/// @param tmplt template for instrument initialization
	/// @return pointer to new instrument configuration object
	virtual InstrConfig* AddInstrument(bsInt16 inum, InstrMapEntry *type, Opaque tmplt = 0)
	{
		if (inum < 0)
			inum = internalID++;
		InstrConfig *ent = new InstrConfig(inum, type, tmplt);
		if (ent == 0)
			return 0;

		InstrConfig *pos = instList;
		if (pos == 0)
		{
			instList = ent;
			return ent;
		}

		InstrConfig *last = 0;
		while (pos)
		{
			if (pos->inum > ent->inum)
			{
				pos->InsertBefore(ent);
				if (pos == instList)
					instList = ent;
				return ent;
			}
			if (pos->inum == ent->inum)
				ent->inum = internalID++;
			last = pos;
			pos = pos->next;
		}
		if (last)
			last->Insert(ent); 
		return ent;
	}

	/// Load instrument library file.
	/// This method opens the file, reads all instrument conifgurations
	/// and adds each instrument configuration to this instrument
	/// manager. The file must be an XML file.
	/// @param fname path to the file
	/// @returns 0 on success, -1 on error
	int LoadInstrLib(const char *fname);

	/// Load instrument library file.
	/// This method reads all instrument conifgurations from the
	/// XML tree and adds each instrument configuration to this instrument
	/// manager. The argument must be the root node with tag instrlib.
	/// @param inst root node for the instrument library
	/// @returns 0 on success, -1 on error
	int LoadInstrLib(XmlSynthElem *inst);

	/// Load instrument configuration.
	/// This method reads one instrument conifguration from the
	/// XML tree and adds the instrument configuration to this instrument
	/// manager. The argument must be the node with tag instr.
	/// @param inst node for the instrument configuration
	/// @returns pointer to the new instrument config object
	InstrConfig *LoadInstr(XmlSynthElem *inst);

	/// Load a wavetable
	/// This loads a wavetable from a configuration file.
	/// @param wvnode root node for the wavetable definition
	/// @returns 0 on success, -1 on error
	int LoadWavetable(XmlSynthElem *wvnode);

	/// Enumerate instruments. The first call should
	/// pass NULL as an argument. Subsequent calls should
	/// pass the previous entry.
	/// @param p instrument map entry
	/// @returns next instrument
	InstrConfig *EnumInstr(InstrConfig *p)
	{
		if (p)
			return p->next;
		return instList;
	}

	/// Find instrument by number.
	/// @param inum instrument number
	/// @return pointer to the instrument map entry or NULL
	virtual InstrConfig *FindInstr(bsInt16 inum)
	{
		InstrConfig *in;
		for (in = instList; in; in = in->next)
		{
			if (in->inum == inum)
				return in;
		}
		return in;
	}

	/// Find instrument by name
	/// @param iname instrument name
	/// @return pointer to the instrument map entry or NULL
	virtual InstrConfig *FindInstr(const char *iname)
	{
		InstrConfig *in;
		for (in = instList; in; in = in->next)
		{
			if (in->name.CompareNC(iname) == 0)
				break;
		}
		return in;
	}

	/// Allocate an instrument instance.
	/// @param inum instrument number
	virtual Instrument *Allocate(bsInt16 inum)
	{
		return Allocate(FindInstr(inum));
	}

	/// Allocate an instrument instance.
	/// @param in instrument map entry for the instrument
	/// @return pointer to the instrument instance.
	virtual Instrument *Allocate(InstrConfig *in)
	{
		if (in)
			return in->MakeInstance(this);
		return new Instrument;
	}

	/// Allocate an instrument instance from event.
	/// @param evt sequencer event
	virtual Instrument *Allocate(SeqEvent *evt)
	{
		if (evt->im)
			return evt->im->MakeInstance(this);
		return Allocate(FindInstr(evt->inum));
	}

	/// Deallocate an instrument instance.
	/// @param ip pointer to the instrument object
	virtual void Deallocate(Instrument *ip)
	{
		ip->Destroy();
	}

	/// Allocate a new event for an instrument.
	/// @param inum instrument number
	virtual SeqEvent *ManufEvent(bsInt16 inum)
	{
		return ManufEvent(FindInstr(inum));
	}

	/// Allocate a new event for an instrument.
	/// @param in instrument map entry
	/// @return pointer to the event object
	virtual SeqEvent *ManufEvent(InstrConfig *in)
	{
		if (in)
		{
			SeqEvent *evt = in->MakeEvent();
			if (evt)
			{
				evt->inum = in->inum;
				evt->im = in;
				return evt;
			}
		}
		return new NoteEvent;
	}

	/// Start is called by the sequencer when the sequence starts.
	virtual void Start()
	{
		// clear the mixer
		if (mix)
			mix->Reset();
		memset(exclNotes, 0, sizeof(exclNotes));
	}
	
	/// Stop is called by the sequencer when the sequence stops.
	virtual void Stop()
	{
		memset(exclNotes, 0, sizeof(exclNotes));
	}

	/// Tick is called by the sequencer on each sample. This is
	/// the point where all active instruments have produced a
	/// value for the current sample. We can now output the sample
	/// to the output buffer. Notice that this only supports 
	/// two-channel output. However, the buffer can be set for
	/// single channel and merge the two outputs together. For
	/// more that two channels, derive a class from this one
	/// and override Tick.
	virtual void Tick()
	{
		AmpValue outLft, outRgt;
		mix->Out(&outLft, &outRgt);
		wvf->Output2(outLft, outRgt);
	}

	/// Direct output to effects units. This bypasses the
	/// normal input channel volume, pan, and fx send values.
	/// @param unit effects unit number
	/// @param val amplitude value to send
	virtual void FxSend(int unit, AmpValue val)
	{
		mix->FxIn(unit, val);
	}

	/// Output a sample on the indicated channel.
	/// The value is passed through the panning and
	/// effects processing for the channel.
	/// @param ch mixer input channel
	/// @param val amplitude value
	virtual void Output(int ch, AmpValue val)
	{
		mix->ChannelIn(ch, val);
	}

	/// Output a left/right sample on the indicated channel.
	/// This is used by instruments that have internal
	/// panning capability as it will bypass the mixer 
	/// panning functions.
	/// @param ch mixer input channel
	/// @param lft left output amplitude value
	/// @param rgt right output amplitude value
	virtual void Output2(int ch, AmpValue lft, AmpValue rgt)
	{
		mix->ChannelIn2(ch, lft, rgt);
	}

	/// Controller change (MIDI).
	/// Sets the current controller value.
	/// @param chnl channel number
	/// @param ccx controller number
	/// @param val value (0-127)
	virtual void ControlChange(int chnl, int ccx, int val);

	/// Pitch wheel change (MIDI).
	/// Sets the current pitch wheel value. This is a
	/// 14-bit value divided into two bytes. Final value is
	/// (msb<<7)|lsb. The value should be scaled by RPN0,
	/// with a default range of +/-2 semitones.
	/// @param chnl channel number
	/// @param v1 message byte 1 (LSB)
	/// @param v2 message byte 2 (MSB)
	virtual void PitchbendChange(int chnl, int v1, int v2);

	/// Channel pressure change (MIDI).
	/// @param chnl channel number
	/// @param val message value (0-127)
	virtual void AftertouchChange(int chnl, int val);

	/// Exclusive note on.
	/// SF2 and DLS files allow indicating a note
	/// as "exclusive" - no other note in the same
	/// group should sound simultaneously. If a sounding
	/// note in the same group and channel is found,
	/// it is sent a Cancel signal. Typically
	/// this is used for percussion instruments.
	/// The index is (channel<<4)|group.
	/// @param index exclusive note group + channel.
	/// @param ip instrument that is currently sounding.
	virtual void ExclNoteOn(int index, Instrument *ip);

	/// Exclusive note off.
	/// See ExclNoteOn().
	/// @param index exclusive note group + channel.
	/// @param ip instrument that is currently sounding.
	virtual void ExclNoteOff(int index, Instrument *ip);
};
//@}
#endif
