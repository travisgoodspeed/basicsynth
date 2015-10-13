//////////////////////////////////////////////////////////////////////
/// @file GMInstrManager.h Instrument manager for GMSynth
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef GMINSTRMANAGER_H
#define GMINSTRMANAGER_H

/// Specialized instrument manager for GM.
/// GMInstrManager inherits from InstrManager and overrides
/// the generic instrument allocation functions, always
/// returning a GMPlayer instrument regardless of the value
/// set for the instrument in the SeqEvent structure.
///
/// We bypass the mixer because the volume levels are controlled
/// by the MIDI CC# 7 on a per-channel basis. The mixer inputs
/// are, therefore, redunant.
///
/// This implementation adds/destroys instruments on each
/// note. A possible improvement is to maintain a cache
/// of instruments, much the same way a keyboard synth
/// has a fixed number of voices.
class GMInstrManager : public InstrManager
{
protected:
	AmpValue outLft;
	AmpValue outRgt;
	AmpValue outRvrb;
	AmpValue masterVol;
	AmpValue reverbMix;
	Reverb2 reverb;
	GMPlayer *gm;

public:
	GMInstrManager()
	{
		masterVol = 1.0;
		reverbMix = 0.1;
		InstrMapEntry *ime = AddType("GMPlayer", GMPlayer::InstrFactory, GMPlayer::EventFactory);
		gm = new GMPlayer;
		AddInstrument(1, ime, gm);
		gm->SetParam(16, GMPLAYER_LOCAL_PAN|GMPLAYER_LOCAL_FX|GMPLAYER_LOCAL_VOL);
	}

	~GMInstrManager()
	{
	}

	void SetSoundBank(SoundBank *sb, float scl)
	{
		gm->SetSoundBank(sb);
		gm->SetParam(19, scl);
	}

	void SetVolume(AmpValue v, AmpValue r)
	{
		masterVol = v;
		reverbMix = r;
	}

	virtual void Clear()
	{
		// don't discard the GMManager object.
	}

	virtual Instrument *Allocate(bsInt16 inum)
	{
		return GMPlayer::InstrFactory(this, gm);
	}

	virtual Instrument *Allocate(InstrConfig *in)
	{
		return GMPlayer::InstrFactory(this, gm);
	}

	virtual void Deallocate(Instrument *ip)
	{
		ip->Destroy();
	}

	virtual SeqEvent *ManufEvent(bsInt16 inum)
	{
		return GMPlayer::EventFactory(gm);
	}

	virtual SeqEvent *ManufEvent(InstrConfig *in)
	{
		return GMPlayer::EventFactory(gm);
	}

	void ControlChange(int chnl, int ctl, int val)
	{
		InstrManager::ControlChange(chnl, ctl, val);
	}

	/// Start is called by the sequencer 
	/// when the sequence starts.
	virtual void Start()
	{
		InstrManager::Start();
		outLft = 0;
		outRgt = 0;
		outRvrb = 0;
		reverb.InitReverb(0.25, 1.0);
	}

	/// Tick outputs the current sample.
	virtual void Tick()
	{
		AmpValue rv = reverb.Sample(outRvrb) * reverbMix;
		wvf->Output2(masterVol * (outLft + rv), masterVol * (outRgt + rv));
		outLft = 0;
		outRgt = 0;
		outRvrb = 0;
	}

	/// FxSend sends values to an effects unit
	/// This only implements reverb (unit 0)
	virtual void FxSend(int unit, AmpValue val)
	{
		if (unit == 0)
			outRvrb += val;
	}

	/// Output a mono sample
	virtual void Output(int ch, AmpValue val)
	{
		val *= GetVolumeN(ch) * 0.5;
		outLft += val;
		outRgt += val;
	}

	/// Output a stereo sample
	virtual void Output2(int ch, AmpValue lft, AmpValue rgt)
	{
		outLft += lft;
		outRgt += rgt;
	}
};
#endif
//@}
