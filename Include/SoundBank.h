///////////////////////////////////////////////////////////
// BasicSynth - SoundFont sound bank
//
/// @file SoundBank.h Internal sound bank, loaded from SF2 or DLS files.
//
// These classes are for the in-memory sound bank.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

#ifndef SOUNDBANK_H
#define SOUNDBANK_H
#include <SynthFile.h>

#define SBGEN_NONE     0x0000 ///< Constant 1.0

// Generator destinations (summing nodes)
#define SBGEN_PITCH    0x0001
#define SBGEN_VOLUME   0x0002
#define SBGEN_FILTER   0x0003
#define SBGEN_FILTQ    0x0004
#define SBGEN_PAN      0x0005
#define SBGEN_CHORUS   0x0006
#define SBGEN_REVERB   0x0007

// Internal generator
#define SBGEN_LFO1PC   0x0008 ///< LFO1 - pitch
#define SBGEN_LFO2PC   0x0009 ///< LFO2 - pitch
#define SBGEN_LFO2CB   0x000A ///< LFO2 - attenuation
#define SBGEN_LFO2FC   0x000B ///< LFO2 - filter
#define SBGEN_EG2PC    0x000C ///< EG2 - pitch
#define SBGEN_EG2FC    0x000D ///< EG2 - filter

// Synth parameters
#define SBGEN_LFO1FRQ  0x000E
#define SBGEN_LFO1DLY  0x000F
#define SBGEN_LFO2FRQ  0x0010
#define SBGEN_LFO2DLY  0x0011

#define SBGEN_EG1DLY   0x0012
#define SBGEN_EG1ATK   0x0013
#define SBGEN_EG1HOLD  0x0014
#define SBGEN_EG1DEC   0x0015
#define SBGEN_EG1SUS   0x0016
#define SBGEN_EG1REL   0x0017
#define SBGEN_EG1KEYH  0x0018
#define SBGEN_EG1KEYD  0x0019
#define SBGEN_EG1VELA  0x001A
#define SBGEN_EG1SHTDN 0x001B

#define SBGEN_EG2DLY   0x001C
#define SBGEN_EG2ATK   0x001D
#define SBGEN_EG2HOLD  0x001E
#define SBGEN_EG2DEC   0x001F
#define SBGEN_EG2SUS   0x0020
#define SBGEN_EG2REL   0x0021
#define SBGEN_EG2KEYH  0x0022
#define SBGEN_EG2KEYD  0x0023
#define SBGEN_EG2VELA  0x0024

#define SBGEN_KEYN     0x0025
#define SBGEN_VELN     0x0026
#define SBGEN_PITCHWHL 0x0027
#define SBGEN_CHNLAT   0x0028
#define SBGEN_PKEYAT   0x0029
#define SBGEN_RPN0     0x002A
#define SBGEN_LFO1     0x002B
#define SBGEN_LFO2     0x002C
#define SBGEN_EG1      0x002D
#define SBGEN_EG2      0x002E

#define SBGEN_SIZE     0x002F

// Flags indicating internal generator connections.
#define SBGEN_LFO1X   0x00000001 ///< LFO1 is active
#define SBGEN_LFO2X   0x00000002 ///< LFO2 is active
#define SBGEN_EG2X    0x00000004 ///< MOD EG is active
#define SBGEN_FILTERX 0x00000010 ///< Filter is active
#define SBGEN_FILTERD 0x00000020 ///< Filter is dynamic
#define SBGEN_SF2     0x10000000 ///< Loaded from SF2 file
#define SBGEN_DLS     0x20000000 ///< Loaded from DLS file

// other MIDI Channel controller source (flag - or'd with CC#)
#define SBGEN_CHCTL  0x0080  ///< MIDI CC value

#define SBGEN_PREFLG  0x8000

/// Normalization index values.
/// These must match the normalize[] array setup.
#define SBGEN_LINEAR  0x00
#define SBGEN_CONCAVE 0x01
#define SBGEN_CONVEX  0x02
#define SBGEN_SWITCH  0x03
#define SBGEN_POLFLG  0x10 // bi-polar when set
#define SBGEN_NEGFLG  0x20 // invert when set

class SBInstr; // forward ref

/// @brief Soundbank file information
/// The strings are read from INFO chunks in the soundbank file.
/// Not all INFO possible chunk IDs are represented. This list
/// contains information useful to display to the user in a GUI
/// synthesizer.
struct SBInfo
{
	bsInt16 wMajorFile;
	bsInt16 wMinorFile;
	bsInt16 wMajorVer;
	bsInt16 wMinorVer;
	bsString szSoundEngine;
	bsString szName;
	bsString szDate;
	bsString szEng;
	bsString szProduct;
	bsString szCopyright;
	bsString szComment;
	bsString szTools;

	SBInfo()
	{
		wMajorFile = 0;
		wMinorFile = 0;
		wMajorVer = 0;
		wMinorVer = 0;
	}
};

/// @brief One SF2 modulator, or DLS connection block.
/// These can appear at the preset, Instrument or zone level.
class SBModInfo : public SynthList<SBModInfo>
{
public:
	short srcOp;  ///< Source
	short srcNf;  ///< Source normalize function
	short ctlOp;  ///< Control source
	short ctlNf;  ///< Control normalize function
	short dstOp;  ///< Destination
	short trnOp;  ///< Output transfer function
	float scale;  ///< Source scaling

	SBModInfo()
	{
		srcOp = SBGEN_NONE;
		dstOp = SBGEN_NONE;
		ctlOp = SBGEN_NONE;
		trnOp = 0;
		srcNf = 0;
		ctlNf = 0;
		scale = 0.0f;
	}
};

/// @brief A container of modulators for instrument, group or zone.
/// @details This aggregates two lists, one for initialization
/// and one for playback. The initialization list typically
/// has a source of NONE, KEY, or VEL and is only processed
/// during note-on.
class SBModList
{
public:
	SynthEnumList<SBModInfo> modList; ///< MIDI modulators (not currently used)
	
	/// @brief Get modulator information
	/// Locate any modulator info that connects 
	/// the given source generator to the given
	/// destination generator.
	/// @param src Source generator
	/// @param ctl Source Amount generator (controller)
	/// @param dst Destination generator
	SBModInfo *GetModInfo(short src, short ctl, short dst)
	{
		SBModInfo *mod = 0;
		while ((mod = modList.EnumItem(mod)) != 0)
		{
			if (mod->srcOp == src 
			 && mod->ctlOp == ctl 
			 && mod->dstOp == dst)
				return mod;
		}
		return 0;
	}

	/// Add a new, empty modulator.
	/// @returns new modulator object.
	inline SBModInfo *AddModInfo()
	{
		return modList.AddItem();
	}

	/// Enumerate all modulators.
	/// On the first call, mi should be set equal to NULL.
	/// @param mi previous modulator or NULL for first.
	/// @returns next modulator or NULL for no more modulators.
	SBModInfo *EnumModInfo(SBModInfo *mi)
	{
		return modList.EnumItem(mi);
	}

};

/// @brief SBSample contains a block of samples read from the file.
/// For SF2 files, there is only one block holding all
/// samples. However, we divide that one block into smaller blocks.
/// DLS files have multiple blocks, potentially one for each region.
/// The filepos member is included to allow for incremental loading of sample information.
/// (@sa SoundBank::GetSample())
class SBSample : public SynthList<SBSample>
{
public:
	AmpValue *sample;    ///< mono sample
	AmpValue *linked;    ///< second array for 2 channel
	SBSample *linkSamp;  ///< linked, phase-locked sample object
	bsUint32  filepos;   ///< file offset for samples
	bsUint32  filepos2;  ///< offset for LSB in SF2 24-bit format
	bsInt32   rate;      ///< recording sample rate
	bsInt32   sampleLen; ///< total length of 'samples'
	bsInt32   index;     ///< index/id number
	bsInt16   format;    ///< 0 = 8-bit, 1=16-bit, 2=IEEE float, 3=SF2 24-bit
	bsInt16   channels;  ///< 1 = mono, 2 = stero (others not supported)

	SBSample(int n = 0)
	{
		index = n;
		sample = 0;
		linked = 0;
		linkSamp = 0;
		sampleLen = 0;
		rate = 44100;
		filepos = 0;
		filepos2 = 0;
		format = -1;
		channels = 0;
	}

	~SBSample()
	{
		delete sample;
		delete linked;
	}
};


/// @brief Envelope definition for a soundbank instrument.
/// This is a 6-segment envelope: delay, attack, hold,
/// decay, sustain, release. Time values are in timecents.
class SBEnv
{
public:
	FrqValue  delay;        ///< delay time (timecents
	FrqValue  attack;       ///< attack rate (timecents)
	FrqValue  hold;         ///< hold time (timecents)
	FrqValue  decay;        ///< decay rate (timecents)
	FrqValue  release;      ///< release rate (timecents)
	FrqValue  shutdown;     ///< shutdown rate (secs)
	AmpValue  sustain;      ///< sustain level (percent)
	FrqValue  keyDecay;     ///< Key # to decay rate scale (timecents)
	FrqValue  keyHold;      ///< Key # to hold rate scale (timecents)
	FrqValue  velAttack;    ///< Velocity to attack rate scale (timecents)

	SBEnv()
	{
		delay = -32768.0;
		attack = -12000.0;
		hold = -32768.0;
		decay = -12000.0;
		release = -12000.0;
		shutdown = 0.001;
		sustain = 1000.0;
		keyHold = 0.0;
		keyDecay = 0.0;
		velAttack = 0.0;
	}
};

/// @brief LFO oscillator information.
class SBLfo
{
public:
	FrqValue  rate;   ///< LFO rate (frequency)
	FrqValue  delay;  ///< LFO onset delay

	SBLfo()
	{
		rate = 4.5;
		delay = 0;
	}
};

/// @brief SBZone represents a wavetable for a range of keys.
/// The zone includes a pointer to the wavetable, loop points,
/// frequency, and envelope information. The key and velocity
/// range values are used to associate the zone with a range
/// of pitches. Instruments have from 1 to 128 zones. 
/// Melodic instruments typically have zones that cover a small
/// range of pitches per zone.
/// Percussion instruments typically have one zone per sound,
/// each mapped to one key number.
class SBZone : 
	public SBModList,
	public SynthList<SBZone>
{
public:
	bsString  name;      ///< zone name
	bsInt16   zoneNdx;   ///< zone index (SF2 IBAG#)
	bsInt16   sampleNdx; ///< index into sample list
	SBSample *sample;    ///< the wavetable
	SBZone   *linkZone;  ///< link to another zone (stereo sample)
	AmpValue  pan;       ///< pan 
	FrqValue  rate;      ///< sample rate
	FrqValue  recCents;  ///< recording frequency in pitch cents
	FrqValue  recFreq;   ///< recording frequency in Hz
	bsInt32   tableStart;///< first playable sample (usually 0)
	bsInt32   tableEnd;  ///< one past last playable sample
	bsInt32   loopStart; ///< the phase where we start looping
	bsInt32   loopEnd;   ///< the phase where the loop ends (one sample past the last sample to loop)
	bsInt32   loopLen;   ///< loopEnd - loopStart
	bsInt32   recPeriod; ///< nominal waveform period in samples
	bsInt16   keyNum;    ///< 0-127, the nominal frequency is 440 * pow(2, (keyNum - 69)/12)
	bsInt16   cents;     ///< detune amount (multiplier for frequency) frq *= pow(2, cents / 1200);
	bsInt16   chan;      ///< 0 = right/mono, 1 = left;
	bsInt16   mode;      ///< 0 = no loop, 
						 ///< 1 = loop continuously
						 ///< 3 = loop during key, then play remainder
	bsInt16   lowKey;    ///< lowest pitch for this sample
	bsInt16   highKey;   ///< highest pitch for this sample
	bsInt16   lowVel;    ///< lowest MIDI note-on velocity
	bsInt16   highVel;   ///< highest MIDI note-on velocity
	bsInt16   exclNote;  ///< Exclusive note id
	bsInt16   exclSelf;  ///< Non-self exclusive (DLS2 1.4.4)
	bsInt16   fixedKey;
	bsInt16   fixedVel;
	bsInt16   fineTune;
	bsInt16   coarseTune;
	bsInt16   scaleTune;

	SBEnv     volEg;     ///< Volume envelope
	SBEnv     modEg;     ///< Modulator envelope
	SBLfo     vibLfo;    ///< LFO 1 (vibrato)
	SBLfo     modLfo;    ///< LFO 2 (modulation)

	AmpValue  velScale;  ///< Note-on velocity scaling (centibels)
	FrqValue  velFlt;    ///< Note-on velocity filter scaling (cents)
	AmpValue  initAtten; ///< Initial attenuation
	FrqValue  vibLfoFrq; ///< Vib LFO amount applied to frequency (cents)
	FrqValue  vibLfoMwFrq;
	AmpValue  modLfoVol; ///< Mod LFO amount appiled to volume (centibels)
	FrqValue  modLfoFlt; ///< Mod LFO amount applied to filter (cents)
	FrqValue  modLfoFrq; ///< Mod LFO amount applied to frequency (cents)
	FrqValue  modLfoMwFrq;
	FrqValue  modLfoMwFlt;
	AmpValue  modLfoMwVol;
	FrqValue  modEnvFrq; ///< Mod ENV amount applied to frequency (cents)
	FrqValue  modEnvFlt; ///< Mod ENV amount applied to filter (cents)

	FrqValue  filtFreq;
	AmpValue  filtQ;

	AmpValue  reverb;
	AmpValue  chorus;

	bsUint32  genFlags;  ///< flags indicating default modulator connections
	AmpValue  peak;      ///< peak amplitude of loop

	SBZone()
	{
		Init();
	}

	~SBZone()
	{
	}

	/// Initialize member variables.
	void Init()
	{
		zoneNdx = 0;
		sampleNdx = 0;
		sample = 0;
		linkZone = 0;
		peak = 0.0;
		pan = 0.0;
		rate = 44100;
		recCents = 6000;
		recFreq = 261.6255653;
		tableStart = 0;
		tableEnd = 0;
		loopStart = 0;
		loopEnd = 0;
		loopLen = 0;
		keyNum = 60;
		cents = 0;
		chan = 0;
		mode = 0;
		lowKey = 0;
		highKey = 127;
		lowVel = 0;
		highVel = 127;
		exclNote = 0;
		exclSelf = 1;
		fixedKey = -1;
		fixedVel = -1;
		fineTune = 0;
		coarseTune = 0;
		scaleTune = 100;
		velScale = 960.0;
		velFlt = 0;
		vibLfoFrq = 0;
		vibLfoMwFrq = 50;
		modLfoVol = 0;
		modLfoFlt = 0;
		modLfoFrq = 0;
		modLfoMwFrq = 0;
		modLfoMwFlt = 0;
		modLfoMwVol = 0;
		modEnvFrq = 0;
		modEnvFlt = 0;
		filtFreq = 13500;
		filtQ = 0;
		reverb = 0;
		chorus = 0;

		genFlags = 0;
	}

	/// @brief Check this zone for a match to key and velocity.
	inline int Match(int key, int vel)
	{
		return key >= lowKey && key <= highKey
			&& vel >= lowVel && vel <= highVel;
	}

	/// @brief Check this zone for a match to only the key.
	inline int MatchKey(int key)
	{
		return key >= lowKey && key <= highKey;
	}

	/// @brief Check this zone for a match to only the velocity.
	inline int MatchVel(int vel)
	{
		return vel >= lowVel && vel <= highVel;
	}
};

/// @brief A reference to a zone.
/// This is used to make up an optimized map of
/// key number to zone. Since a zone can apply
/// to multiple keys, we keep a list. This makes
/// multiple references to a zone.
class SBZoneRef : public SynthList<SBZoneRef>
{
public:
	SBZone *zone;
};

/// @brief A Group of zones.
/// This is roughly the same as the SF2 "Instrument"
/// and is needed to make layering work right.
/// Zones in the zone group are played only
/// if the current key and velocity are in the
/// range of the group and the zone.
/// DLS1 does not support layering, so there will be exactly
/// one group with all zones in it.
class SBZoneGroup : 
	public SBModList,
	public SynthList<SBZoneGroup>
{
public:
	SBZoneRef *map[128];
	bsInt16 lowKey;
	bsInt16 highKey;
	bsInt16 lowVel;
	bsInt16 highVel;
	bsInt16 index;
	bsInt16 global;       ///< true if this is "global" to groups in the instrument
	SBInstr *instr;
	SynthEnumList<SBModInfo> modList; ///< playback modulators
	SynthEnumList<SBModInfo> iniList; ///< initialization modulators

	SBZoneGroup()
	{
		lowKey = 0;
		highKey = 127;
		lowVel = 1;
		highVel = 127;
		instr = 0;
		index = 0;
		global = 0;
		memset(map, 0, sizeof(map));
	}

	~SBZoneGroup()
	{
		ClearMap();
	}

	void ClearMap()
	{
		for (int n = 0; n < 128; n++)
		{
			SBZoneRef *last;
			SBZoneRef *ref = map[n];
			while (ref)
			{
				last = ref->next;
				delete ref;
				ref = last;
			}
			map[n] = 0;
		}
	}

	int InRange(short zLowKey, short zHighKey, short zLowVel, short zHighVel)
	{
		//int lo = max(zLowKey, lowKey);
		int lo = zLowKey > lowKey ? zLowKey : lowKey;
		//int hi = min(zHighKey, highKey);
		int hi = zHighKey < highKey ? zHighKey : highKey;
		if (lo > hi)
			return 0;

		//lo = max(zLowVel, lowVel);
		lo = zLowVel > lowVel ? zLowVel : lowVel;
		//hi = min(zHighVel, highVel);
		hi = zHighVel < highVel ? zHighVel : highVel;
		if (lo > hi)
			return 0;

		return 1;
	}

	void AddZone(SBZone *zone)
	{
		//int lo = max(zone->lowKey, lowKey);
		int lo = zone->lowKey > lowKey ? zone->lowKey : lowKey;
		//int hi = min(zone->highKey, highKey);
		int hi = zone->highKey < highKey ? zone->highKey : highKey;
		for (int n = lo; n <= hi; n++)
		{
			SBZoneRef *ref = new SBZoneRef;
			ref->zone = zone;
			ref->next = map[n];
			map[n] = ref;
		}
	}

	/// @brief Check this group for a match to key and velocity.
	inline int Match(int key, int vel)
	{
		return key >= lowKey && key <= highKey
			&& vel >= lowVel && vel <= highVel;
	}

	/// @brief Return a zone matching key and velocity.
	/// @details This only returns one zone, and is probably
	/// useless because of that.
	SBZone *GetZone(int key, int vel)
	{
		SBZoneRef *ref = map[key];
		while (ref)
		{
			if (ref->zone->MatchVel(vel))
				return ref->zone;
			ref = ref->next;
		}
		return 0;
	}
};

/// @brief Soundbank Instrument
/// An instrument is a collection of zones and global modulators.
/// This is roughly equivalent to a SF2 "preset" or a DLS "instrument"
/// structure;
class SBInstr : 
	public SBModList,
	public SynthList<SBInstr>
{
public:
	SynthEnumList<SBZone> zoneList;
	SynthEnumList<SBZoneGroup> groupList;
	bsString instrName;
	bsInt16  instrNdx;
	bsInt16  bank;
	bsInt16  prog;
	bsInt16  loaded;

	SBInstr()
	{
		instrNdx = -1;
		bank = 0;
		prog = 0;
		loaded = 0;
	}

	~SBInstr()
	{
	}

	/// Add a new, empty zone to this list.
	/// @returns new zone object.
	SBZone *AddZone()
	{
		return zoneList.AddItem();
	}

	SBZoneGroup *AddGroup()
	{
		SBZoneGroup *grp = groupList.AddItem();
		grp->instr = this;
		return grp;
	}

	/// @brief Locate a zone
	/// GenZone finds the first zone that 
	/// matches the key and velocity.
	/// There may be multiple zones that match.
	/// Use EnumZones to find all matches.
	///
	/// @param key MIDI key number.
	/// @param vel MIDI note-on velocity.
	/// @returns matching zone or NULL for no match.
	SBZone *GetZone(int key, int vel)
	{
		SBZoneGroup *grp = 0;
		while ((grp = groupList.EnumItem(grp)) != 0)
		{
			if (grp->Match(key, vel))
			{
				SBZone *zone = grp->GetZone(key, vel);
				if (zone != NULL)
					return zone;
			}
		}
		return 0;
	}

	inline SBZoneGroup *EnumGroups(SBZoneGroup *grp)
	{
		return groupList.EnumItem(grp);
	}

	/// Enumerate all zones.
	/// On the first call, zone should be set equal to NULL.
	/// @param zone previous zone or NULL for first zone.
	/// @return next zone or NULL for no more zones.
	inline SBZone *EnumZones(SBZone *zone)
	{
		return zoneList.EnumItem(zone);
	}

};

//////////////////////////////////////////////////////////////
/// @brief SoundBank holds a collection of instruments.
/// A SoundBank is typically loaded from either a
/// SF2 file or a DLS file, but could be created directly 
/// in memory from individual WAVE files or oscillators.
///
/// Instruments are represented by bank and program (patch)
/// numbers, with 128 patch settings per bank. 
/// For MIDI, there are potentially 16k banks with the bank
/// number represented as a pair of values, CC# 0 (MSB) and CC# 32 (LSB).
/// A GM soundbank typically has two banks of importance,
/// a melody instruments bank, represented by MSB 0x78,
/// and a percussion bank represented by MSB of 0x79.
/// Technically, only banks with LSB of 0-9 are defined under GM.
/// This implementation allows the bank number to range 0-128, with
/// bank number 128 representing the percussion bank. The user of the 
/// sound bank must map the MIDI bank number into the range 0-128.	
///
/// Soundbank files typically encode pitch, time and
/// volume with a fixed point value, scaled appropriately.
/// Pitch is in cents; time is in timecents; volume
/// is in centibels. This class includes static methods 
/// to convert the integer representation
/// into the appropriate floating point value.
/// In addition, four tables are created to apply a normalization
/// transfer function to a raw MIDI value.
///
/// All soundbank objects should be attached to the static
/// SoundBankList member and located using FindBank.
//////////////////////////////////////////////////////////////
class SoundBank : public SynthList<SoundBank>
{
private:
	int lockCount;
public:
	bsInt32  id;
	bsUint16 chnls;
	bsString file;                 ///< file name (empty if constructed in memory)
	bsString name;                 ///< symbolic name
	SBInfo info;                   ///< INFO records (copyright, version, etc.)
	SBInstr **instrBank[129];      ///< array of instruments[bank][patch]
	SBSample *samples;             ///< list of sample blocks
	FileReadBuf sampleFile;        ///< file for on-demand loading of samples
	int sampleFileOpen;

	static SoundBank SoundBankList; ///< List of loaded soundbanks
	static void DeleteBankList();  ///< Remove all soundbanks
	static SoundBank *FindBank(const char *name); ///< Find soundbank by name
	static SoundBank *FindBankFile(const char *file); ///< Find soundbank by file path
	static SoundBank *DefaultBank();

	/// Pre-calculated powers of 2^(n/1200)
	static FrqValue *pow2Table;

	/// Pre-calculated powers of 10^(n/-200)
	static AmpValue *pow10Table;

	/// Maximum filter, pitch cents
	static FrqValue maxFilter;
	/// Minimum filter, pitch cents
	static FrqValue minFilter;

	/// normalization curve tables
	/// Use the transform and raw MIDI value as index: normalize[fn][value].
	/// The "negative" function is available as: normalize[fn][127 - value].
	/// Bipolar functions are a little more complicated.
	///@{
	static float posLinear[128];
	static float posConcave[128];
	static float posConvex[128];
	static float posSwitch[128];
	static float *normalize[4];
	static void InitTransform();
	static float Transform(short val, short tt);
	///}@

	static double log2(double f);

	/// Convert time cents to time
	/// Time cents is defined as tc = 1200 * log2(sec),
	/// where tc ranges from -12000 (1ms) to the maximum rate.
	/// Max rate varies, but is usually no more than 8000 (100s)
	/// @param tc time in time cents
	/// @return time in seconds
	static FrqValue EnvRate(FrqValue tc);

	/// Convert pitch cents to frequency.
	static FrqValue Frequency(FrqValue pc);

	/// Convert attenuation in centibels to linear amplitude.
	/// Centibels is defined as cb = 200 * log10(-amp), where
	/// cb ranges from 0 (no attenuation) to 960 (silence)
	/// @param cb attenuation in centibels
	/// @return linear amplitude (0-1)
	static AmpValue Attenuation(AmpValue cb);
	
	/// Convert gain in centibels to linear amplitude.
	/// Centibels is defined as cb = 200 * log10(amp).
	/// Linear gain is thus: g = pow(10, amp/200).
	/// Values less than zero will attenuate, greater
	/// than zero will multiply.
	/// @param cb gain in centibels
	/// @return linear amplitude
	static AmpValue Gain(AmpValue cb);

	/// Lookup 2^(n/1200).
	/// This function uses a table where possible.
	/// It has multiple uses since a lot of SF/DLS
	/// parameters are stored as 1200 * log2(x).
	/// To convert MIDI key to frequency use:
	/// @code
	/// freq = 440.0 * GetPow2n1200(100*(key-69));
	/// @endcode
	/// To add cents to a frequency:
	/// @code
	/// freq *= GetPow2n1200(cents);
	/// @endcode
	/// @return power of 2
	static FrqValue GetPow2n1200(FrqValue fc);
	static void InitPow2n1200();

	/// Lookup 10^(n/-200)
	static AmpValue GetPow10n200(AmpValue amp);
	static void InitPow10n200();

	SoundBank()
	{
		sampleFileOpen = 0;
		lockCount = 0;
		samples = 0;
		chnls = 0xffff;
		id = 0;
		memset(&instrBank[0], 0, sizeof(instrBank));
		InitPow2n1200();
		InitPow10n200();
		InitTransform();
		maxFilter = log2((synthParams.sampleRate/4)/8.175) * 1200;
		minFilter = log2(20.0/8.175) * 1200;
	}

	~SoundBank()
	{
		Remove(); // justin-case

		for (int b = 0; b < 129; b++)
		{
			SBInstr **instrList = instrBank[b];
			if (instrList != 0)
			{
				for (int n = 0; n < 128; n++)
					if (instrList[n] != 0)
						delete instrList[n];
				delete instrList;
				instrBank[b] = 0;
			}
		}
		SBSample *samp;
		while ((samp = samples) != 0)
		{
			samples = samp->next;
			delete samp;
		}
	}

	/// Optimize the zone lookup.
	void Optimize()
	{
		// no longer needed
	}

	/// @name SoundBank locking.
	/// A SoundBank object is likely shared by multiple instruments
	/// and editors. We do not want to delete until all instruments
	/// have released their reference. The lock count is
	/// used for this purpose.
	/// @{
	/// Lock the sound bank.
	int Lock() { return ++lockCount; }
	/// Unlock the sound bank.
	/// If this is the last reference, the object is deleted.
	int Unlock()
	{
		int cnt = --lockCount;
		if (lockCount <= 0)
		{
			Remove();
			delete this;
		}
		return cnt;
	}
	/// @}

	/// @brief Add an instrument to the soundbank.
	/// @param bank bank number
	/// @param prog program (patch) number
	/// @return pointer to empty instrument definition
	SBInstr *AddInstr(bsInt16 bank, bsInt16 prog)
	{
		if (prog < 0 || prog > 127)
			return 0;
		if (bank > 128)
		{
			if ((bank & 0x7F) == 0)
				bank >>= 7;
			if (bank > 128)
				return 0;
		}

		SBInstr **instrList = instrBank[bank];
		if (instrList == 0)
		{
			instrList = new SBInstr*[128];
			instrBank[bank] = instrList;
			memset(instrList, 0, sizeof(SBInstr*)*128);
		}
		SBInstr *ip = instrList[prog];
		if (ip == 0)
		{
			ip = new SBInstr;
			ip->bank = bank;
			ip->prog = prog;
			instrList[prog] = ip;
		}
		return ip;
	}

	/// @brief Locate an instrument.
	/// @param bank bank number
	/// @param prog patch number
	/// @param load pre-load the instrument sample if not loaded
	/// @return pointer to instrument definition, or null
	SBInstr *GetInstr(bsInt16 bank, bsInt16 prog, int load = 1)
	{
		if (bank < 0 || bank > 128 || prog < 0 || prog > 127)
			return 0;
		SBInstr **instrList;
		SBInstr *in = 0;
		if (bank == 128)
		{
			if ((instrList = instrBank[128]) != 0)
				 in = instrList[prog];
		}
		else
		{
			do
			{
				if ((instrList = instrBank[bank]) != 0)
				{
					if ((in = instrList[prog]) != 0)
						break;
				}
			} while (--bank >= 0);
		}
		if (in && load && !in->loaded)
			LoadInstr(in);
		return in;
	}

	/// @brief Add a sample block.
	/// @param ndx index (id) for this sample
	/// @return pointer to empty sample
	SBSample *AddSample(int ndx)
	{
		SBSample *samp = new SBSample(ndx);
		if (samples)
			samples->InsertBefore(samp);
		samples = samp;
		return samp;
	}

	/// @brief Locate a sample by index
	/// This function will attempt to dynamically
	/// load the sample data if the sample data is null.
	/// The sample object must already be initialized
	/// with the correct file offset, size and format.
	/// @param ndx index (id) of the sample
	/// @param load load the sample data if not loaded
	/// @return pointer to sample
	SBSample *GetSample(int ndx, int load = 1)
	{
		SBSample *samp = samples;
		while (samp)
		{
			if (samp->index == ndx)
			{
				if (samp->sample == 0 && load)
					LoadSample(samp);
				return samp;
			}
			samp = samp->next;
		}
		return 0;
	}

	/// @name Sample Loading
	/// Load sample data from the original file.
	/// If the sample cannot be loaded, a block of zeros is allocated.
	/// @param samp pointer to sample block object.
	/// @param f already open file
	/// @return 0 on success, non-zero on failure.
	/// @{
	int OpenSampleFile();
	int LoadSample(SBSample *samp);
	int LoadSample(SBSample *samp, FileReadBuf& f);
	int LoadInstr(SBInstr *instr);
	int LoadInstr(SBInstr *instr, FileReadBuf& f);
	int ReadSamples1(SBSample *samp, FileReadBuf& f);
	int ReadSamples2(SBSample *samp, FileReadBuf& f);
	/// @}
};

#endif
//@}
