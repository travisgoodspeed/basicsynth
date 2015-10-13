//////////////////////////////////////////////////////////////////////
/// @file SFPlayer.h BasicSynth SoundFont(R) Player
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_SFPLAYER_H_)
#define _SFPLAYER_H_

#include "LFO.h"
#include "PitchBend.h"
#include <SoundBank.h>
#include <SFGen.h>

#define SFPLAYER_BANK 16
#define SFPLAYER_PROG 17

/// One generator for SFPlayer.
class SFGen : public SynthList<SFGen>
{
public:
	//GenWaveWTLoop osc;
	GenWaveSB osc;
	FrqValue phsPC;
	Panner pan;

	void CalcPhsIncr(int pit, SBZone *zone);
};

/// BasicSynth Instrument to play SoundBank sounds.
///
/// This instrument provides playback of a SoundFont or DLS instrument,
/// specified using a SoundBank alias, bank number and preset number.
/// Bank numbers are 0-128, with bank 128 being the precussion sets. Preset
/// number is 0-127, equivalent to MIDI program change value. 
///
/// The articulation/modulation information from the sound bank is not used.
/// Since the instrument is for use with Notelist, it expects parameter information
/// to be set through SetParam or the SeqEvent object.
///
/// This instrument will respond to real-time frequency changes through the Param()
/// function, changing zones as needed. In order to avoid discontinuities, it
/// performs a cross-fade between samples. It does not support portamento, however.
class SFPlayerInstr : public InstrumentVP
{
protected:
	int chnl;           ///< output mixer channel
	int pitch;          ///< played pitch
	bsInt16 keyLo;      ///< lowest key for current zone
	bsInt16 keyHi;      ///< highest key for current zone
	AmpValue vol;       ///< volume level
	FrqValue frq;       ///< playback frequency
	FrqValue pwFrq;     ///< pitch wheel (pitch cents)
	SFGen *genList;     ///< primary generator
	SFGen *xfdList;     ///< cross-fade generator
	EnvSegLin fadeEG;   ///< cross-fade interpolator
	bsInt32 xfade;      ///< flag indicating we are cross-fading
	EnvGenADSR volEnv;  ///< volume envelope
	LFO vibLFO;         ///< vibrato LFO
	PitchBendWT pbWT;   ///< pitch bend (wavetable)

	bsInt16 monoSet;    ///< force monophonic even if sample is stereo, turn internal pan off

	GenWaveWT oscfm;    ///< FM 'boost' oscillator
	EnvGenADSR modEnv;  ///< envelope for FM
	FrqValue fmCM;      ///< c:m ratio for 'fm'
	AmpValue fmIM;      ///< index of mod for 'fm'
	AmpValue fmAmp;     ///< fm amplitude
	int fmOn;           ///< quick flag to see if FM is on

	bsString sndFile;   ///< sound bank name (alias or file)
	bsString insName;   ///< instr name (for reference)

	SoundBank *sndbnk;  ///< sound bank object
	SBInstr *instr;     ///< instrument object
	bsInt16 bnkNum;     ///< bank number, 0-128, 128=drum kit
	bsInt16 insNum;     ///< instrument number 0-127

	InstrManager *im;

	int LoadEnv(XmlSynthElem *elem, EnvGenADSR& env);
	int SaveEnv(XmlSynthElem *elem, EnvGenADSR& env);

	void FindInstr();
	SFGen *BuildZoneList(int pit, int vel);
	void ClearZoneList(SFGen *list);

public:
	static Instrument *SFPlayerInstrFactory(InstrManager *m, Opaque tmplt);
	static SeqEvent *SFPlayerEventFactory(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	SFPlayerInstr();
	SFPlayerInstr(SFPlayerInstr *tp);
	virtual ~SFPlayerInstr();
	virtual void Copy(SFPlayerInstr *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	void SetSoundBank(SoundBank *b);
	SoundBank *GetSoundBank()       { return sndbnk; }

	const char *GetSoundFile()        { return sndFile; }
	const char *GetInstrName()       { return insName; }

	void SetSoundFile(const char *str) 
	{
		sndFile = str;
		if (sndbnk)
			sndbnk->Unlock();
		sndbnk = 0;
		instr = 0;
	}
	void SetInstrName(const char *str) 
	{ 
		insName = str; 
		instr = 0;
	}

	virtual int Load(XmlSynthElem *parent);
	virtual int Save(XmlSynthElem *parent);

	virtual VarParamEvent *AllocParams();
	virtual int GetParams(VarParamEvent *params);
	virtual int GetParam(bsInt16 id, float* val);
	virtual int SetParams(VarParamEvent *params);
	virtual int SetParam(bsInt16 id, float val);
};
//@}
#endif
