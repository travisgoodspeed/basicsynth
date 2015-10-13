///////////////////////////////////////////////////////////
// BasicSynth - SoundFont File
//
/// @file SFFile.h SoundFont(R) file loader.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{

#ifndef SFFILE_H
#define SFFILE_H

#include <SFDefs.h>
#include <SoundBank.h>

/// A sound font file.
/// SFFile is used to load a SF2 file into a SoundBank object.
class SFFile
{
public:
	int npresets;
	sfPresetHeader *phdr;

	int npbags;
	sfBag *pbag;

	int npmods;
	sfModList *pmod;

	int npgens;
	sfGenList *pgen;

	int ninsts;
	sfInst *inst;

	int nibags;
	sfBag *ibag;

	int nimods;
	sfModList *imod;

	int nigens;
	sfGenList *igen;

	int nshdrs;
	sfSample *shdr;

	int preload;

	FileReadBuf file;
	SoundBank *sfbnk;
	// generator amounts at different levels
	genAmountType defaultVal[sfgEndOper];
	genAmountType globPreset[sfgEndOper];
	genAmountType loclPreset[sfgEndOper];
	genAmountType globZone[sfgEndOper];
	genAmountType loclZone[sfgEndOper];

	short *samples;
	bsUint32 sampleSize;
	bsUint32 sampleFileOffs1;
	bsUint32 sampleFileOffs2;

	int ReadChunk(sfChunk& chk);
	int ReadData(void **pdata, long cksz);
	void ReadString(long len, bsString *str);
	bsUint16 ReadShort();
	bsUint32 ReadLong();
	bsUint16 Swap(bsUint16 n);
	bsUint32 Swap(bsUint32 n);
	bsInt16 Swap(bsInt16 n) { return (bsInt16)Swap((bsUint16)n); }
	bsInt32 Swap(bsInt32 n) { return (bsInt32)Swap((bsUint32)n); }

	int InfoChunk(long cksz);
	int SDTAChunk(long cksz);
	int PDTAChunk(long cksz);
	int ListChunk(sfChunk& chk);

	SBSample *CreateSample(sfSample *shdr, short id);

	int ValidInstrGen(short gen);
	int ValidPresetGen(short gen);
	int AddPresetGen(short gen);
	short MapDestination(short destination);
	short MapSource(short source);
	void ApplyModulator(SBZone *zone, sfModList *mp, int preset = 0);
	SBModInfo *AddGenerator(SBModList *zone, short src, short dst, short ctl, short amnt);
	SBModInfo *AddModulator(SBModList *zone, short src, short dst, short ctl, short amnt, short flags = 0);
	SBModInfo *AddInitializer(SBModList *zone, short src, short dst, short ctl, short amnt, short flags = 0);
	void BuildInstrument(SBZoneGroup *grp, int n, int pbagNdx, int gbagNdx);
	void BuildPreset(int n);
	void BuildSoundBank();

	AmpValue SFAttenuation(short amt);
	char *CopyName(char *dst, char *src);
	void InitGenVals(genAmountType *genVals);

public:
	SFFile();
	~SFFile();

	/// Determine if the file is a SF2 format file.
	/// @param fname path to the file
	/// @returns true if the file is SF2 format
	static int IsSF2File(const char *fname);

	/// Load the sound found file into a SoundBank.
	/// When pre is true, the sample data for all instruments is loaded
	/// into memory. If only a few sounds are to be used
	/// it is better to load each instrument's sample data explicitly.
	/// The caller is responsible for deleteing the returned object.
	/// @param fname path to the file
	/// @param pre preload all samples
	/// @returns pointer to SoundBank object.
	SoundBank *LoadSoundBank(const char *fname, int pre = 1);
};
//@}
#endif
