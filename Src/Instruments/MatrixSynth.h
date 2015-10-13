//////////////////////////////////////////////////////////////////////
/// @file MatrixSynth.h BasicSynth Matrix Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{

#if !defined(_MATRIXSYNTH_H_)
#define _MATRIXSYNTH_H_

#include "LFO.h"
#include "PitchBend.h"

// MATGEN sets the size of the matrix. We can go up to 16 oscillators because 
// we have 16 available bits in the toneFlags member. Eight is usually enough.
#define MATGEN     8
#define TONE_OUT_BITS 0x0000FFFF
#define TONE_ON      0x00000001
#define TONE_OUT     0x00000002
#define TONE_LFOIN   0x00000004
#define TONE_PBIN    0x00000008
#define TONE_FX1OUT  0x00000010
#define TONE_FX2OUT  0x00000020
#define TONE_FX3OUT  0x00000040
#define TONE_FX4OUT  0x00000080
//#define TONE_XXX   0x00003F00 <= available
#define TONE_TREM    0x00004000
#define TONE_PAN     0x00008000
#define TONE_OUTANY  0x000000F2

#define TONE_MOD_BITS 0xFFFF0000
#define TONE_MOD1IN  0x00010000
#define TONE_MOD2IN  0x00020000
#define TONE_MOD3IN  0x00040000
#define TONE_MOD4IN  0x00080000
#define TONE_MOD5IN  0x00100000
#define TONE_MOD6IN  0x00200000
#define TONE_MOD7IN  0x00400000
#define TONE_MOD8IN  0x00800000
//#define TONE_MODXX   0xFF000000 <= available
#define TONE_MODANY  0x00FF000C

/// @brief MatrixTone is one tone generator for the MatSynth Instrument
/// @details MatrixTone aggregates an oscillator and contains
/// members for each parameter for one tone generator.
class MatrixTone
{
private:
	GenWaveI osc;      //!< signal oscillator
//	GenWaveWT osc;     //!< signal oscillator
	FrqValue frqMult;  //!< Frequency multiplier
	AmpValue volLvl;   //!< Signal out level
	AmpValue modLvl;   //!< Modulation in level
	PhsAccum modRad;   //!< Modulation in radians
	AmpValue lfoLvl;   //!< LFO input level
	AmpValue pbLvl;    //!< Pitch Bend level
	Panner   panSet;   //!< Pan setting -1,+1
	AmpValue fx1Lvl;   //!< Fx1 send level
	AmpValue fx2Lvl;   //!< Fx2 send level
	AmpValue fx3Lvl;   //!< Fx3 send level
	AmpValue fx4Lvl;   //!< fx4 send level
	bsUint32 toneFlags;  //!< TONE_* bits
	bsUint16 envIndex; //!< envelope index 0-7

public:
	MatrixTone();
	~MatrixTone();

	/// Make a copy of this generator
	void Copy(MatrixTone *tp);
	/// Start output
	void Start(FrqValue frqBase);
	/// Change the frequency
	void AlterFreq(FrqValue frqBase);
	/// Modulate the phase
	void PhaseModWT(PhsAccum phs);
	/// Generate the next sample
	AmpValue Gen();

	/// Load parameters from project file
	int Load(XmlSynthElem *elem);
	/// Save parameters to project file
	int Save(XmlSynthElem *elem);

	friend class MatrixSynth;
};

/// @brief MatrixSynth implements an eight-tone matrix instrument
/// @details MatrixSynth aggregates eight oscillators and
/// eight envelope generators in a variable configuration.
class MatrixSynth : public InstrumentVP
{
private:
	int chnl;
	FrqValue frq;
	AmpValue vol;
	MatrixTone   gens[MATGEN];
	EnvGenSegSus envs[MATGEN];    // envelope A,D1,D2,S,R
	bsUint16 envUsed;

	LFO lfoGen;
	PitchBend pbGen;
	PitchBendWT pbWT;

	int lfoOn;
	int panOn;
	int pbOn;
	int pbWTOn;
	int fx1On;
	int fx2On;
	int fx3On;
	int fx4On;
	bsUint32 allFlags;

	InstrManager *im;

	int LoadEnv(XmlSynthElem *elem);
	int SaveEnv(XmlSynthElem *elem, int en);

public:
	MatrixSynth();
	MatrixSynth(MatrixSynth *tp);
	virtual ~MatrixSynth();
	/// Create an instance of a MatSynth instrument
	static Instrument *MatrixSynthFactory(InstrManager *, Opaque tmplt);
	/// Create an instance of an event for the MatSynth instrument
	static SeqEvent   *MatrixSynthEventFactory(Opaque tmplt);
	/// Map a parameter name to an ID
	static bsInt16     MapParamID(const char *name, Opaque tmplt);
	/// Map a parameter ID to a name
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	/// Initialize parameters from a template
	void Copy(MatrixSynth *tp);
	/// Start output, setting parameters from the event
	void Start(SeqEvent *evt);
	/// Modify parameters while playing
	void Param(SeqEvent *evt);
	/// Stop output, release all EG
	void Stop();
	/// Generate the next sample sending output to the instrument manager
	void Tick();
	/// Return true if all envelopes are complete
	int  IsFinished();
	/// Destroy this instance
	void Destroy();

	/// Load parameters from the project file
	int Load(XmlSynthElem *parent);
	/// Save parameters to the project file
	int Save(XmlSynthElem *parent);
	/// Allocate an event structure large enough for all parameters
	VarParamEvent *AllocParams();
	/// Get the parameters into the event structure
	int GetParams(VarParamEvent *params);
	/// Get one parameter by ID
	int GetParam(bsInt16 idval, float *val);
	/// Set the parameters from the event structure
	int SetParams(VarParamEvent *params);
	/// Set one parameter by ID
	int SetParam(bsInt16 idval, float val);

	/// Set the number of segments for an EG
	void SetSegs(int gn, int sn)
	{
		if (gn < MATGEN)
			envs[gn].SetSegs(sn);
	}

	/// Get the number of segments for an EG
	int GetSegs(int gn)
	{
		if (gn < MATGEN)
			return envs[gn].GetSegs();
		return 0;
	}
};
//@}
#endif
