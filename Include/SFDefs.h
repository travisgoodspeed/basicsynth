///////////////////////////////////////////////////////////
// BasicSynth - SoundFont definitions
//
/// @file SFDefs.h SoundFont(R) file "generator" types
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

#ifndef SFDEFS_H
#define SFDEFS_H

// Generator opcodes
#define sfgStartAddrsOffset 0
#define sfgEndAddrsOffset 1
#define sfgStartloopAddrsOffset 2
#define sfgEndloopAddrsOffset 3
#define sfgStartAddrsCoarseOffset 4
#define sfgModLfoToPitch 5
#define sfgVibLfoToPitch 6
#define sfgModEnvToPitch 7
#define sfgInitialFilterFc 8
#define sfgInitialFilterQ 9
#define sfgModLfoToFilterFc 10
#define sfgModEnvToFilterFc 11
#define sfgEndAddrsCoarseOffset 12
#define sfgModLfoToVolume 13
#define sfgUnused1 14
#define sfgChorusEffectsSend 15
#define sfgReverbEffectsSend 16
#define sfgPan 17
#define sfgUnused2 18
#define sfgUnused3 19
#define sfgUnused4 20
#define sfgDelayModLFO 21
#define sfgFreqModLFO 22
#define sfgDelayVibLFO 23
#define sfgFreqVibLFO 24
#define sfgDelayModEnv 25
#define sfgAttackModEnv 26
#define sfgHoldModEnv 27
#define sfgDecayModEnv 28
#define sfgSustainModEnv 29
#define sfgReleaseModEnv 30
#define sfgKeynumToModEnvHold 31
#define sfgKeynumToModEnvDecay 32
#define sfgDelayVolEnv 33
#define sfgAttackVolEnv 34
#define sfgHoldVolEnv 35
#define sfgDecayVolEnv 36
#define sfgSustainVolEnv 37
#define sfgReleaseVolEnv 38
#define sfgKeynumToVolEnvHold 39
#define sfgKeynumToVolEnvDecay 40
#define sfgInstrument 41
#define sfgReserved1 42
#define sfgKeyRange 43
#define sfgVelRange 44
#define sfgStartloopAddrsCoarseOffset 45
#define sfgKeynum 46
#define sfgVelocity 47
#define sfgInitialAttenuation 48
#define sfgReserved2 49
#define sfgEndloopAddrsCoarseOffset 50
#define sfgCoarseTune 51
#define sfgFineTune 52
#define sfgSampleID 53
#define sfgSampleModes 54
#define sfgReserved3 55
#define sfgScaleTuning 56
#define sfgExclusiveClass 57
#define sfgOverridingRootKey 58
#define sfgUnused5 59
#define sfgEndOper 60

//values for wSampleLink
#define sfgMonoSample 1
#define sfgRightSample 2
#define sfgLeftSample 4
#define sfgLinkedSample 8
#define sfgRomBit 0x8000

//values for sample modes
#define sfgNoLoop   0  // don't loop
#define sfgLoopRel  1  // loop through release
#define sfgLoopKey  3  // only loop when key down

/////////////////// BEGIN RIFF STRUCTURES //////////////////////////
#ifndef FOURCC
#if BS_BIG_ENDIAN
#define FOURCC(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)
#else
#define FOURCC(a,b,c,d) (a|(b<<8)|(c<<16)|(d<<24))
#endif
#endif

// chunk header ids found in the SF2 file
#define SF_RIFF_CHUNK FOURCC('R','I','F','F')
#define SF_LIST_CHUNK FOURCC('L','I','S','T')
#define SF_IFIL_CHUNK FOURCC('i','f','i','l')
#define SF_ISNG_CHUNK FOURCC('i','s','n','g')
#define SF_IROM_CHUNK FOURCC('i','r','o','m')
#define SF_IVER_CHUNK FOURCC('i','v','e','r')
#define SF_INAM_CHUNK FOURCC('I','N','A','M')
#define SF_ICRD_CHUNK FOURCC('I','C','R','D')
#define SF_IENG_CHUNK FOURCC('I','E','N','G')
#define SF_IPRD_CHUNK FOURCC('I','P','R','D')
#define SF_ICOP_CHUNK FOURCC('I','C','O','P')
#define SF_ICMT_CHUNK FOURCC('I','C','M','T')
#define SF_ISFT_CHUNK FOURCC('I','S','F','T')
#define SF_SMPL_CHUNK FOURCC('s','m','p','l')
#define SF_SM24_CHUNK FOURCC('s','m','2','4')
#define SF_PHDR_CHUNK FOURCC('p','h','d','r')
#define SF_PBAG_CHUNK FOURCC('p','b','a','g')
#define SF_PMOD_CHUNK FOURCC('p','m','o','d')
#define SF_PGEN_CHUNK FOURCC('p','g','e','n')
#define SF_INST_CHUNK FOURCC('i','n','s','t')
#define SF_IBAG_CHUNK FOURCC('i','b','a','g')
#define SF_IMOD_CHUNK FOURCC('i','m','o','d')
#define SF_IGEN_CHUNK FOURCC('i','g','e','n')
#define SF_SHDR_CHUNK FOURCC('s','h','d','r')

// Chunk format values found in a sf2 file
#define SF_SFBK_FORMAT FOURCC('s','f','b','k')
#define SF_INFO_FORMAT FOURCC('I','N','F','O')
#define SF_SDTA_FORMAT FOURCC('s','d','t','a')
#define SF_PDTA_FORMAT FOURCC('p','d','t','a')

#pragma pack(push, 2)

/// SF2 RIFF chunk header
struct sfChunk
{
	bsUint32 ckid; //char ckid[4];
	bsUint32 cksz;
	// data[cksz]
};

#define sfChunkSize 8

/// Two bytes 
struct rangesType
{
	bsUint8 byLo;
	bsUint8 byHi;
};

/// SF2 generator or modulator value.
/// Union of two bytes, signed and unsigned words
union  genAmountType
{
	rangesType ranges;
	bsInt16 shAmount;
	bsUint16 wAmount;
};

/// SF2 Preset header record layout.
struct sfPresetHeader
{
	char achPresetName[20];
	bsUint16 wPreset;
	bsUint16 wBank;
	bsUint16 wPresetBagNdx;
	bsUint32 dwLibrary;
	bsUint32 dwGenre;
	bsUint32 dwMorphology;
};

#define sfPresetHeaderSize 38

/// SF2 Preset or Instrument bag record layout.
struct sfBag
{
	bsUint16 wGenNdx;
	bsUint16 wModNdx;
};

#define sfBagSize 4

/// SF2 Modulator record layout.
struct sfModList
{
	bsInt16 sfModSrcOper;
	bsInt16 sfModDestOper;
	bsInt16 modAmount;
	bsInt16 sfModAmtSrcOper;
	bsInt16 sfModTransOper;
};

#define sfModListSize 10

/// SF2 Generator record layout.
struct sfGenList
{
	bsInt16 sfGenOper;
	genAmountType genAmount;
};

#define sfGenListSize 4

/// SF2 Instrument record layout
struct sfInst
{
	char achInstName[20];
	bsUint16 wInstBagNdx;
};

#define sfInstSize 22

/// SF2 Sample record layout.
struct sfSample
{
	char achSampleName[20];
	bsUint32 dwStart;
	bsUint32 dwEnd;
	bsUint32 dwStartloop;
	bsUint32 dwEndloop;
	bsUint32 dwSampleRate;
	bsUint8  byOriginalKey;
	bsInt8   chCorrection;
	bsUint16 wSampleLink;
	bsInt16  sfSampleType;
};

#define sfSampleSize 46

#pragma pack(pop)
/////////////////// END RIFF STRUCTURES //////////////////////////
//@}
#endif
