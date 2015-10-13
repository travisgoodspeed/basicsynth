///////////////////////////////////////////////////////////////
//
// BasicSynth - Mixer
//
/// @file Mixer.h Mixer and associated classes.
//
// The Mixer and associated classes implement a multi-channel
// mixer with panning and effects send/recieve. The number of
// input channels and effects processors is settable at run-time.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpMix
//@{
#ifndef _MIXER_H_
#define _MIXER_H_

/// Pan method off
#define panOff 0
/// Pan method linear
#define panLin 1
/// Pan method sin(pan)
#define panTrig 2
/// Pan method sqrt(pan)
#define panSqr 3

////////////////////////////////////////////////////////////////////
/// Dynamic mixer functions
////////////////////////////////////////////////////////////////////
#define mixFx   0x100
#define mixRamp 0x200
#define mixOsc  0x400

#define mixNoFunc       0x00
#define mixSetInpLvl    0x01
#define mixSetPanPos    0x02
#define mixSetSendLvl   (0x04|mixFx)
#define mixSetFxLvl     (0x08|mixFx)
#define mixSetFxPan     (0x10|mixFx)
#define mixRampInpLvl   (mixSetInpLvl|mixRamp)
#define mixRampPanPos   (mixSetPanPos|mixRamp)
#define mixRampSendLvl  (mixSetSendLvl|mixRamp)
#define mixRampFxLvl    (mixSetFxLvl|mixRamp)
#define mixRampFxPan    (mixSetFxPan|mixRamp)
#define mixOscilInpLvl  (mixSetInpLvl|mixOsc)
#define mixOscilPanPos  (mixSetPanPos|mixOsc)
#define mixOscilSendLvl (mixSetSendLvl|mixOsc)
#define mixOscilFxLvl   (mixSetFxLvl|mixOsc)
#define mixOscilFxPan   (mixSetFxPan|mixOsc)

#define P_MIX_FUNC 16
#define P_MIX_FROM 17
#define P_MIX_TO   18
#define P_MIX_TIME 19
#define P_MIX_FX   20
#define P_MIX_FRQ  21
#define P_MIX_WT   22
#define P_MIX_PANT 23

///////////////////////////////////////////////////////////////
/// Pan sounds left to right.
/// The Panner class calculates the left and right multipliers
/// based on pan method and pan value (-1,+1). Lookup tables are
/// used for non-linear panning methods so that dynamic panning
/// is efficient.
///////////////////////////////////////////////////////////////
class Panner
{
public:
	AmpValue panval;
	AmpValue panlft;
	AmpValue panrgt;

	Panner()
	{
		panval = 0.0;
		panlft = 0.5;
		panrgt = 0.5;
	}

	/// Set the pan method and value.
	/// The pan value is -1 for full left, +1 for full right, and 0 for center.
	/// Pan methods are off, linear, trig and square-root.
	/// @param pm pan method
	/// @param pv pan setting value
	void Set(int pm, AmpValue pv)
	{
		if (pv > 1.0)
			pv = 1.0;
		else if (pv < -1.0)
			pv = -1.0;
		panval = pv;
		if (pm == panOff)
		{
			panlft = 0.5;
			panrgt = 0.5;
			return;
		}

		panlft = (1 - pv) / 2;
		panrgt = (1 + pv) / 2;
		// optional: range 0-1
		// panlft = 1 - pv;
		// panrgt = pv;

		if (pm == panTrig)
		{
			//panlft = sin(panlft * PI/2) * sqrt(2)/2;
			//panrgt = sin(panrgt * PI/2) * sqrt(2)/2;
			panlft = synthParams.sinquad[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sinquad[(int)(panrgt * synthParams.sqNdx)];
		}
		else if (pm == panSqr)
		{
			//panlft = sqrt(panlft) * sqrt(2)/2;
			//panrgt = sqrt(panrgt) * sqrt(2)/2;
			panlft = synthParams.sqrttbl[(int)(panlft * synthParams.sqNdx)];
			panrgt = synthParams.sqrttbl[(int)(panrgt * synthParams.sqNdx)];
		}
	}
};

///////////////////////////////////////////////////////////////
/// Effects channel.
/// FxChannel represents one effects channel. The effects 
/// processor is typically reverb, flanger, chorus, etc., but
/// can in theory be any unit generator. For example, if the
/// fx member is set to a LFO, this would apply tremolo to all
/// inputs.
///
/// This class includes an array of "send" values representing
/// the amount of each input channel that is passed through the
/// effects processor. This allows multiple inputs to share the
/// same reverb (for example) but with a different level applied
/// to each channel.
///
/// The FxChannel also has a built-in panner so that it can be
/// directed to each output channel independently. I.E. we can
/// have one amount of reverb in the left output, and a different
/// amount in the right output.
///
/// Note: the unit generator is owned by the caller and is not
/// deleted by the FxChannel. Be sure to set the unit generator
/// to null in this object before deleting it!
///////////////////////////////////////////////////////////////
class FxChannel
{
public:
	GenUnit *fx;
	AmpValue *fxlvl; // one for each input channel
	AmpValue value;  // total input
	AmpValue fxmix;  // output level
	Panner   pan;
	int init;

	FxChannel()
	{
		init = 0;
		fx = 0;
		fxlvl = 0;
		value = 0;
		fxmix = 0;
	}

	~FxChannel()
	{
		delete[] fxlvl;
	}

	/// Effects in from input channel.
	/// @param ch input channel sending the value
	/// @param val input amplitude value
	void FxIn(int ch, AmpValue val)
	{
		// NB no runtine check
		value += fxlvl[ch] * val;
	}

	/// Effects in direct.
	/// @param val input amplitude value
	void FxIn(AmpValue val)
	{
		value += val;
	}

	/// Effects output. Applies internall panning
	/// @param lft left output value
	/// @param rgt right output value
	void FxOut(AmpValue& lft, AmpValue& rgt)
	{
		if (fx)
		{
			AmpValue out = fx->Sample(value) * fxmix;
			lft += out * pan.panlft;
			rgt += out * pan.panrgt;
		}
		value = 0;
	}

	/// Effects initialization.
	/// @param p generator unit (e.g., reverb, chorus)
	/// @param ch number of input channel
	/// @param lvl output level
	void FxInit(GenUnit *p, int ch, AmpValue lvl)
	{
		if (fxlvl)
		{
			delete[] fxlvl;
			fxlvl = 0;
		}
		fx = 0;
		if (p && ch)
		{
			fxlvl = new AmpValue[ch];
			for (int n = 0; n < ch; n++)
				fxlvl[n] = 0;
			fx = p;
			init = 1;
		}
		fxmix = lvl;
	}

	/// Set effects level setting
	/// @param ch input channel
	/// @param lvl receive attenuator
	void FxSendSet(int ch, AmpValue lvl)
	{
		if (init)
			fxlvl[ch] = lvl;
	}

	/// Get effects level setting
	/// @param ch input channel
	/// @return receive attenuator
	AmpValue FxSendGet(int ch)
	{
		if (init)
			return fxlvl[ch];
		return 0;
	}

	/// Set effects output level.
	/// @param lvl output level
	void FxOutSet(AmpValue lvl)
	{
		fxmix = lvl;
	}

	/// Get effects output level
	AmpValue FxOutGet()
	{
		return fxmix;
	}

	/// Set effects pan values.
	/// @param pm pan method
	/// @param lvl pan setting (-1,+1)
	void FxPanSet(int pm, AmpValue lvl)
	{
		pan.Set(pm, lvl);
	}

	/// Clear the effects unit to zero.
	void Clear()
	{
		value = 0;
		if (fx)
			fx->Reset();
	}

	/// Get the unit generator
	/// @returns pointer to the GenUnit
	GenUnit *FxGenGet()
	{
		return fx;
	}

	/// Set the unit generator.
	/// The caller should delete the old unit if appropriate.
	/// @param newfx new unit generator for processing
	/// @returns previous generator
	GenUnit *FxGenSet(GenUnit *newfx)
	{
		GenUnit *oldfx = fx;
		fx = newfx;
		return oldfx;
	}
};

///////////////////////////////////////////////////////////////
/// Mixer input channel class. This accumulates values for one
/// input and applies panning to the value. In addition, a mono
/// value is accumulated so that it can be passed through to
/// additional FX processors. The In2 method bypasses the mixer
/// panning and FX so that instruments can implement dynamic panning
/// reverb, etc, on a note-by-note basis.
///////////////////////////////////////////////////////////////
class MixChannel 
{
private:
	AmpValue left;
	AmpValue right;
	AmpValue volume;
	AmpValue panset;
	Panner pan;
	int   method;
	int   on;

public:
	MixChannel()
	{
		volume = 0.5;
		on = false;
		left = 0;
		right = 0;
		panset = 0;
		method = 0;
	}

	/// Set channel on/off
	/// @param n on = 1, off = 0
	void SetOn(int n)
	{
		on = n;
	}

	/// Get channel on/off
	int IsOn()
	{
		return on;
	}

	/// Set channel level.
	/// @param v volume level
	void SetVolume(AmpValue v)
	{
		volume = v;
	}

	/// Get channel level.
	AmpValue GetVolume() 
	{
		return volume;
	}

	/// Set channel panning.
	/// @param pm pan method
	/// @param p pan setting (-1,+1)
	void SetPan(int pm, AmpValue p)
	{
		panset = p;
		method = pm;
		pan.Set(pm, p);
	}

	/// Get channel panning level.
	AmpValue GetPan()
	{
		return panset;
	}

	/// Add a value to the input buffer.
	/// Panning is applied here.
	/// @param val sample value
	void In(AmpValue val)
	{
		left  += val * pan.panlft;
		right += val * pan.panrgt;
	}

	/// Add a value to the input buffer directly.
	/// This bypasses internall panning.
	/// @param lft left amplitude value
	/// @param rgt right amplitude value
	void In2(AmpValue lft, AmpValue rgt)
	{
		// N.B. : bypass panning and effects!
		left += lft;
		right += rgt;
	}

	/// Get the current level as monophonic value
	AmpValue Level()
	{
		//return both * volume;
		return (left + right) * volume;
	}

	/// Get the current level as stereo values
	/// @param lval left channel value
	/// @param rval right channel value
	void Out(AmpValue &lval, AmpValue& rval)
	{
		lval += left * volume;
		rval += right * volume;
		left = 0;
		right = 0;
	}

	/// Clear the input buffer to zero.
	void Clear()
	{
		left = 0;
		right = 0;
	}
};

///////////////////////////////////////////////////////////////
/// Mix multiple inputs and apply panning and effects.
///
/// The Mixer class combines multiple input channels into a
/// two channel output, applying panning and effects. The number of
/// input channels and number of Fx units can be set dynamically
/// but typically are set once at program initialization.
/// Master left/right values are applied to the sum of all input
/// channels.
///
/// The ChannelIn() and Out() methods are the main interface to the
/// mixer. Each signal generator should pass its output to the
/// appropriate input channel. When all generators have been
/// invoked for the current sample, the Out method is called to
/// get the final output samples. The Out method combines inputs
/// and applies Fx units, before applying the final master output
/// level. 
///////////////////////////////////////////////////////////////
class Mixer
{
private:
	int mixInputs;
	int fxUnits;
	MixChannel *inBuf;
	FxChannel *fxBuf;
	AmpValue lvol;
	AmpValue rvol;
	AmpValue lpeak;
	AmpValue rpeak;

public:
	Mixer()
	{
		mixInputs = 0;
		fxUnits = 0;
		lvol = 1.0;
		rvol = 1.0;
		lpeak = 0.0;
		rpeak = 0.0;
		inBuf = 0;
		fxBuf = 0;
	}

	~Mixer()
	{
		if (inBuf)
			delete[] inBuf;
		if (fxBuf)
			delete[] fxBuf;
	}

	/// Set the master volume values.
	/// @param lv left channel output volume
	/// @param rv right channel output volume
	void MasterVolume(AmpValue lv, AmpValue rv)
	{
		lvol = lv;
		rvol = rv;
	}

	/// Set the number of channels.
	/// This must be called before the mixer is put into use.
	/// Calling this again clears the old buffers.
	/// Input channels are allocated, but not initialized.
	/// @param nchnl number of channels.
	void SetChannels(int nchnl)
	{
		if (inBuf)
		{
			delete[] inBuf;
			inBuf = 0;
		}
		mixInputs = nchnl;
		if (nchnl > 0)
			inBuf = new MixChannel[nchnl];
	}

	/// Get the number of input channels.
	int GetChannels()
	{
		return mixInputs;
	}

	/// Set an input channel on/off.
	/// @param ch input channel
	/// @param on 1 = on, 0 = off
	void ChannelOn(int ch, int on)
	{
		if (ch >= 0 && ch < mixInputs)
			inBuf[ch].SetOn(on);
	}

	/// Set an input channel level.
	/// @param ch input channel
	/// @param v input volume level
	void ChannelVolume(int ch, AmpValue v)
	{
		if (ch >= 0 && ch < mixInputs)
			inBuf[ch].SetVolume(v);
	}

	/// Set an input channel pan values.
	/// @param ch input channel
	/// @param pm pan method
	/// @param p pan level setting (-1,+1)
	void ChannelPan(int ch, int pm, AmpValue p)
	{
		if (ch >= 0 && ch < mixInputs)
			inBuf[ch].SetPan(pm, p);
	}

	/// Send a sample to an input channel.
	/// @param ch channel number
	/// @param val sample amplitude value
	void ChannelIn(int ch, AmpValue val)
	{
		// warning - no runtime range check here...
		inBuf[ch].In(val);
	}

	/// Send a sample to an input channel, direct.
	/// This bypasses channel panning.
	/// @param ch channel number
	/// @param lft left sample amplitude value
	/// @param rgt right sample amplitude value
	void ChannelIn2(int ch, AmpValue lft, AmpValue rgt)
	{
		// warning - no runtime range check here...
		inBuf[ch].In2(lft, rgt);
	}

	/// Set the number of effects channels.
	/// This should be called before beginning output.
	/// Unlike SetChannels, effects channels are optional.
	/// @param n number of effects channels
	void SetFxChannels(int n)
	{
		if (fxBuf)
		{
			delete[] fxBuf;
			fxBuf = 0;
		}
		fxUnits = n;
		if (n > 0)
			fxBuf = new FxChannel[n];
	}

	/// Get the number of effects channels.
	int GetFxChannels()
	{
		return fxUnits;
	}

	/// Initialise an effects channel.
	/// This can only be set affter the number of input channels is set.
	/// @param f effects channel
	/// @param fx effects object
	/// @param lvl output level
	void FxInit(int f, GenUnit *fx, AmpValue lvl)
	{
		// NB: Must set mixInputs first.
		if (mixInputs > 0 && f >= 0 && f < fxUnits)
			fxBuf[f].FxInit(fx, mixInputs, lvl);
	}

	/// Initialize the effects channel receive (output) level.
	/// This can only be set affter the number of input channels is set.
	/// @param f effects channel
	/// @param lvl output level
	void FxReceive(int f, AmpValue lvl)
	{
		if (f >= 0 && f < fxUnits)
			fxBuf[f].FxOutSet(lvl);
	}

	/// Initialize the effects channel send (input) level.
	/// @param f effects channel
	/// @param ch input channel
	/// @param lvl send level
	void FxLevel(int f, int ch, AmpValue lvl)
	{
		if (f >= 0 && f < fxUnits)
			fxBuf[f].FxSendSet(ch, lvl);
	}
	
	/// Initialize the effects panning.
	/// @param f effects channel
	/// @param pm pan method
	/// @param lvl pan setting
	void FxPan(int f, int pm, AmpValue lvl)
	{
		if (f >= 0 && f < fxUnits)
			fxBuf[f].FxPanSet(pm, lvl);
	}

	/// Effects input direct.
	/// Effects send, bypass input channel
	/// @param f effects channel
	/// @param val sample value
	void FxIn(int f, AmpValue val)
	{
		if (f >= 0 && f < fxUnits)
			fxBuf[f].FxIn(val);
	}

	/// Get the mixed output.
	/// This is the main output of the mixer. All input channels
	/// and effects are combined into the left and right values.
	/// @param lval left output
	/// @param rval right output
	void Out(AmpValue *lval, AmpValue *rval)
	{
		int n;
		AmpValue lvalOut = 0;
		AmpValue rvalOut = 0;
		FxChannel *fx, *fxe;
		MixChannel *pin = inBuf;
		// Add inputs and send to fx units.
		for (n = 0; n < mixInputs; n++)
		{
			if (pin->IsOn())
			{
				if ((fx = fxBuf) != 0)
				{
					AmpValue lvl = pin->Level();
					fxe = &fxBuf[fxUnits];
					while (fx < fxe)
					{
						fx->FxIn(n, lvl);
						fx++;
					}
				}
				pin->Out(lvalOut, rvalOut);
			}
			pin++;
		}

		// Add outputs from fx units
		if ((fx = fxBuf) != 0)
		{
			fxe = &fxBuf[fxUnits];
			while (fx < fxe)
			{
				fx->FxOut(lvalOut, rvalOut);
				fx++;
			}
		}

		*lval = lvalOut * lvol;
		*rval = rvalOut * rvol;
		if (*lval > lpeak)
			lpeak = *lval;
		if (*rval > rpeak)
			rpeak = *rval;
	}

	/// Get the peak value.
	/// The peak value is reset to zero
	/// @param lval left channel peak
	/// @param rval right channel peak
	void Peak(AmpValue& lval, AmpValue& rval)
	{
		lval = lpeak;
		rval = rpeak;
		lpeak = 0;
		rpeak = 0;
	}

	/// Reset the mixer. This does not delete channels and effects,
	/// only clears the input to zero.
	void Reset()
	{
		int n;
		for (n = 0; n < mixInputs; n++)
			inBuf[n].Clear();
		for (n = 0; n < fxUnits; n++)
			fxBuf[n].Clear();
		lpeak = 0.0;
		rpeak = 0.0;
	}

	/// Get a reference to an input channel object.
	/// This is made available for editors.
	/// This can be used for interactive mixer control
	/// but can cause clicks if values are changed
	/// during sound output.
	/// @param ch Channel number
	MixChannel *GetChannelPtr(int ch)
	{
		if (ch >= 0 && ch <= mixInputs)
			return &inBuf[ch];
		return 0;
	}

	/// Get a reference to an effects channel object.
	/// This is made available for editors.
	/// @param unit Effects unit number
	FxChannel *GetFxPtr(int unit)
	{
		if (unit >= 0 && unit <= fxUnits)
			return &fxBuf[unit];
		return 0;
	}
};
//@}
#endif
