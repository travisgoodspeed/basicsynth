//////////////////////////////////////////////////////////////////////
/// @file GMPlayer.h BasicSynth General MIDI Player
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
/// @addtogroup grpMIDI
//@{

#if !defined(_GMPLAYER_H_)
#define _GMPLAYER_H_

#include "LFO.h"
#include <SoundBank.h>
#include <SFGen.h>

#define GMPLAYER_LOCAL_BANK 1
#define GMPLAYER_LOCAL_PROG 2
#define GMPLAYER_LOCAL_PAN  4
#define GMPLAYER_LOCAL_FX   8
#define GMPLAYER_LOCAL_VOL  16

#define GMPLAYER_FLAGS 16
#define GMPLAYER_BANK 17
#define GMPLAYER_PROG 18
#define GMPLAYER_ATTN 19

/// @brief GM sound player
/// @details The GM player implements playback of a SoundBank sample.
/// GMPlayer objects are created and managed by the GMManager object,
/// and should not be created directly.
///
/// Each player object recieves a MIDI bank and program number along
/// with a Soundbank object. The associated program (a/k/a patch) holds
/// a list of zones (regions) each covering a range of pitches. The
/// player locates the appropriate zone(s) using the pitch and then
/// initializes an oscillator, envelope and suitable modulators
/// based on the information in the zone. Control of volume, panning,
/// pitch bend and modulation level is done indirectly via the MIDIControl
/// object associated with the GMManager.
///
/// To optimize playback, a set of flags controls which unit generators
/// are executed. The flags are set when the SBZone object is loaded.
///
/// SoundFont (SF2) files will sometimes have multiple zones per note.
/// This is typically done to implement a stereo sample. GMPlayer uses
/// a list of zones to handle this situation.
class GMPlayer : public InstrumentVP
{
private:
	/// One zone player.
	class GMPlayerZone : public SynthList<GMPlayerZone>
	{
	public:
		InstrManager *im;
		GMPlayer *player;
		SBZone *zone;       ///< zone for this key
		GenWaveSB osc;      ///< wavetable oscillator
		EnvGenSB  volEnv;   ///< volume envelope (EG1)
		EnvGenSB  modEnv;   ///< modulation envelope (EG2)
		GenWaveWT vibLfo;   ///< LF pitch variation
		GenWaveWT modLfo;   ///< LF amplitude variation
		FilterIIR2p filt;   ///< Filter
		bsInt32 genFlags;   ///< map of generators that are operational
		bsInt32 vibDelay;   ///< delay before vibrato begins to affect output
		bsInt32 modDelay;   ///< delay before modulator begins to affect output
		bsInt32 fcFlt;     ///< Last filter fc in cents

		bsInt16 chnl;       ///< Playback channel
		bsInt16 noKey;      ///< Note-on key number
		bsInt16 noVel;      ///< Note-on velocity
		bsInt16 localPan;   ///< apply panning internally
		bsInt16 localVol;   ///< apply volume (CC7) internally
		FrqValue initPitch;
		AmpValue initAtten;
		FrqValue initFilter;
		FrqValue vibLfoFrq;
		FrqValue modLfoFrq;
		FrqValue modLfoFlt;
		FrqValue modLfoVol;
		AmpValue gainQ;
		AmpValue panLft;
		AmpValue panRgt;

		GMPlayerZone(SBZone *z, InstrManager *m, GMPlayer *p)
		{
			im = m;
			zone = z;
			player = p;
		}

		void Initialize(bsInt16 ch, bsInt16 key, bsInt16 vel);
		void Gen();
		void SetPanning();
		void SetLFO();
	};

	GMPlayerZone *zoneList;

	bsInt16 chnl;       ///< MIDI channel
	bsInt16 mkey;       ///< MIDI key number
	bsInt16 novel;      ///< note-on velocity
	bsInt16 exclNote;   ///< exclusive note
	bsInt16 sostenuto;  ///< true when sostenuto was on before note start
	bsInt16 pendingStop; ///< Sustain or sostenuto was on at note off
	FrqValue pitchBend;
	AmpValue ctrlAtten;
	AmpValue attnScale;
	FrqValue rvrbAmnt;
	SBInstr *instr;     ///< instrument patch
	InstrManager *im;
	SoundBank *sndbnk;
	bsInt16 localVals;   ///< use local bank/patch and pan values
	bsInt16 bankValue;   ///< Local bank override
	bsInt16 progValue;   ///< Local program override
	bsString sndFile;    ///< sound file name

	void ClearZones();
	void SetVolume();

	friend class GMPlayerZone;
public:
	static Instrument *InstrFactory(InstrManager *m, Opaque tmplt);
	static SeqEvent *EventFactory(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	GMPlayer();
	GMPlayer(GMPlayer *tmplt, InstrManager *m);
	virtual ~GMPlayer();

	void SetSoundFile(const char *f);
	void SetSoundBank(SoundBank *s);

	const char *GetSoundFile() { return sndFile; }
	SoundBank *GetSoundBank() { return sndbnk; }

	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Cancel();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	virtual VarParamEvent *AllocParams();
	virtual int GetParams(VarParamEvent *params);
	virtual int GetParam(bsInt16 id, float* val);
	virtual int SetParams(VarParamEvent *params);
	virtual int SetParam(bsInt16 id, float val);

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);
};

//@}
#endif
