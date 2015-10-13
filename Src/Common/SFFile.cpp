/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file SFFile.cpp SoundFont(TM) loader implementation.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <MIDIDefs.h>
#include <SFFile.h>

SFFile::SFFile()
{
	preload = 1;

	file.SetBufSize(0x10000);
	npresets = 0;
	phdr = 0;
	npbags = 0;
	pbag = 0;
	npmods = 0;
	pmod = 0;
	npgens = 0;
	pgen = 0;
	ninsts = 0;
	inst = 0;
	nibags = 0;
	ibag = 0;
	nimods = 0;
	imod = 0;
	nigens = 0;
	igen = 0;
	nshdrs = 0;
	shdr = 0;
	sfbnk = 0;
	samples = 0;
	sampleSize = 0;
	sampleFileOffs1 = 0;
	sampleFileOffs2 = 0;
}

SFFile::~SFFile()
{
	delete phdr;
	delete pbag;
	delete pmod;
	delete pgen;
	delete inst;
	delete ibag;
	delete imod;
	delete igen;
	delete shdr;
}

int SFFile::IsSF2File(const char *fname)
{
	FileReadBuf file;
	int isSF2 = 0;
	if (file.FileOpen(fname) == 0)
	{
		// read the RIFF chunk.
		sfChunk rchk;
		file.FileRead(&rchk, 8);
		if (rchk.ckid == SF_RIFF_CHUNK)
		{
			// Read the format
			bsInt32 id = 0;
			file.FileRead(&id, 4);
			if (id == SF_SFBK_FORMAT)
				isSF2 = 1;
		}
		file.FileClose();
	}
	return isSF2;
}

SoundBank *SFFile::LoadSoundBank(const char *fname, int pre)
{
	preload = pre;

	if (file.FileOpen(fname))
		return 0;

	sfChunk rchk;
	// read the RIFF chunk.
	ReadChunk(rchk);
	if (rchk.ckid != SF_RIFF_CHUNK)
	{
		file.FileClose();
		return 0;
	}

	// Read the sfbk chunk
	bsInt32 id = ReadLong();
	if (id != SF_SFBK_FORMAT)
	{
		file.FileClose();
		return 0;
	}

	sfbnk = new SoundBank;
	sfbnk->file = fname;
	sampleFileOffs1 = 0;
	sampleFileOffs2 = 0;

	sfChunk chk;
	int err = 0;
	long riffSize = rchk.cksz - 4;
	while (riffSize > 0)
	{
		// read the next chunk header
		if (ReadChunk(chk))
			break;
		riffSize -= sfChunkSize;
		if (chk.ckid == SF_LIST_CHUNK)
		{
			if (ListChunk(chk))
			{
				err = -1;
				break;
			}
		}
		else
			file.FileSkip(chk.cksz);
		riffSize -= chk.cksz;
	}

	if (err)
	{
		delete sfbnk;
		return 0;
	}

	BuildSoundBank();
	file.FileClose();

	sfbnk->Optimize();

	return sfbnk;
}


bsUint16 SFFile::Swap(bsUint16 n)
{
#if SYNTH_BIG_ENDIAN
	return ((n << 8) & 0xff00) | ((n >> 8) & 0x00ff);
#else
	return n;
#endif
}

bsUint32 SFFile::Swap(bsUint32 n)
{
#if SYNTH_BIG_ENDIAN
	return ((n << 24) & 0xff000000)
	     | ((n << 8)  & 0x00ff0000)
	     | ((n >> 8)  & 0x0000ff00)
	     | ((n >> 24) & 0x000000ff);
#else
	return n;
#endif
}

int SFFile::ReadChunk(sfChunk& chk)
{
	if (file.FileRead(&chk, sfChunkSize) != sfChunkSize)
		return -1;
	chk.cksz = Swap(chk.cksz);
	return 0;
}

int SFFile::ReadData(void **pdata, long cksz)
{
	if (*pdata)
		free(*pdata);
	*pdata = malloc(cksz);
	if (*pdata == 0)
		return -1;
	if (file.FileRead(*pdata, cksz) != cksz)
		return -1;
	return 0;
}

bsUint16 SFFile::ReadShort()
{
	bsUint16 val = 0;
	file.FileRead(&val, 2);
	return Swap(val);
}

bsUint32 SFFile::ReadLong()
{
	bsUint32 val = 0;
	file.FileRead(&val, 4);
	return Swap(val);
}

void SFFile::ReadString(long len, bsString *str)
{
	char *s = (char *)malloc(len+1);
	file.FileRead(s, len);
	str->Attach(s);
}

int SFFile::InfoChunk(long cksz)
{
	if (sfbnk == 0)
	{
		file.FileSkip(cksz);
		return 0;
	}

	sfChunk chk;
	while (cksz > 0)
	{
		if (ReadChunk(chk))
			return -1;
		cksz -= sfChunkSize;
		switch (chk.ckid)
		{
		case SF_IFIL_CHUNK:
			// File Version
			sfbnk->info.wMajorFile = ReadShort();
			sfbnk->info.wMinorFile = ReadShort();
			break;
		case SF_ISNG_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szSoundEngine);
			break;
		//case SF_IROM_CHUNK:
		//	ReadString(chk.cksz, &sfbnk->info.szROM);
		//	break;
		case SF_IVER_CHUNK:
			// Version
			sfbnk->info.wMajorVer = ReadShort();
			sfbnk->info.wMinorVer = ReadShort();
			break;
		case SF_INAM_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szName);
			break;
		case SF_ICRD_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szDate);
			break;
		case SF_IENG_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szEng);
			break;
		case SF_IPRD_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szProduct);
			break;
		case SF_ICOP_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szCopyright);
			break;
		case SF_ICMT_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szComment);
			break;
		case SF_ISFT_CHUNK:
			ReadString(chk.cksz, &sfbnk->info.szTools);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		cksz -= chk.cksz;
	}
	return 0;
}

int SFFile::SDTAChunk(long cksz)
{
	sfChunk chk;
	while (cksz > 0)
	{
		if (ReadChunk(chk))
			return -1;
		cksz -= 8;
		if (chk.ckid == SF_SMPL_CHUNK)
		{
			sampleSize = chk.cksz / sizeof(short);
			sampleFileOffs1 = file.FilePosition();
			file.FileSkip(chk.cksz);
		}
		else if (chk.ckid == SF_SM24_CHUNK)
		{
			sampleFileOffs2 = file.FilePosition();
			file.FileSkip(chk.cksz);
		}
		cksz -= chk.cksz;
	}

	return cksz != 0;
}

int SFFile::PDTAChunk(long cksz)
{
	int err = 0;
	sfChunk chk;
	while (cksz > 0)
	{
		if (ReadChunk(chk))
			return -1;
		cksz -= 8;
		err = 0;
		switch (chk.ckid)
		{
		case SF_PHDR_CHUNK:
			npresets = (chk.cksz / sfPresetHeaderSize);
			if (npresets > 0)
				err = ReadData((void**)&phdr, chk.cksz);
			break;
		case SF_PBAG_CHUNK:
			npbags = chk.cksz / sfBagSize;
			if (npbags > 0)
				err = ReadData((void**)&pbag, chk.cksz);
			break;
		case SF_PMOD_CHUNK:
			npmods = chk.cksz / sfModListSize;
			if (npmods > 0)
				err = ReadData((void**)&pmod, chk.cksz);
			break;
		case SF_PGEN_CHUNK:
			npgens = chk.cksz / sfGenListSize;
			if (npgens > 0)
				err = ReadData((void**)&pgen, chk.cksz);
			break;
		case SF_INST_CHUNK:
			ninsts = chk.cksz / sfInstSize;
			if (ninsts > 0)
				err = ReadData((void**)&inst, chk.cksz);
			break;
		case SF_IBAG_CHUNK:
			nibags = chk.cksz / sfBagSize;
			if (nibags > 0)
				err = ReadData((void**)&ibag, chk.cksz);
			break;
		case SF_IMOD_CHUNK:
			nimods = chk.cksz / sfModListSize;
			if (nimods > 0)
				err = ReadData((void**)&imod, chk.cksz);
			break;
		case SF_IGEN_CHUNK:
			nigens = chk.cksz / sfGenListSize;
			if (nigens > 0)
				err = ReadData((void**)&igen, chk.cksz);
			break;
		case SF_SHDR_CHUNK:
			nshdrs = chk.cksz / sfSampleSize;
			if (nshdrs > 0)
				err = ReadData((void**)&shdr, chk.cksz);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		if (err != 0)
			return -1;
		cksz -= chk.cksz;
	}
	return 0;
}

int SFFile::ListChunk(sfChunk& chk)
{
	int err = 0;
	bsInt32 id = ReadLong();
	switch (id)
	{
	case SF_INFO_FORMAT:
		err = InfoChunk(chk.cksz - 4);
		break;
	case SF_SDTA_FORMAT:
		err = SDTAChunk(chk.cksz - 4);
		break;
	case SF_PDTA_FORMAT:
		err = PDTAChunk(chk.cksz - 4);
		break;
	default:
		file.FileSkip(chk.cksz - 4);
		break;
	}
	return err;
}

SBSample *SFFile::CreateSample(sfSample *shdr, short id)
{
	SBSample *samp = sfbnk->GetSample(id, 0);
	if (samp != 0)
		return samp;
	samp = sfbnk->AddSample(id);
	samp->channels = 1;
	samp->sampleLen = (shdr->dwEnd - shdr->dwStart);
	samp->filepos = sampleFileOffs1 + (shdr->dwStart * 2);
	if (sampleFileOffs2 != 0)
	{
		samp->format = 3;
		samp->filepos = sampleFileOffs2 + shdr->dwStart;
	}
	else
		samp->format = 1;

	if (preload)
		sfbnk->LoadSample(samp, file);

	if (shdr->wSampleLink)
	{
		samp->linkSamp = sfbnk->GetSample(shdr->wSampleLink, 0);
		if (samp->linkSamp)
		{
			samp->linkSamp->linkSamp = samp;
			if (preload)
				sfbnk->LoadSample(samp->linkSamp, file);
		}
	}
	return samp;
}

// We have to check each generator because some sound font
// files contain "garbage" that needs to be ignored...

// Determine if the gen can apply at instrument level.
int SFFile::ValidInstrGen(short gen)
{
	switch (gen)
	{
	case sfgInstrument:
	case sfgUnused1:
	case sfgUnused2:
	case sfgUnused3:
	case sfgUnused4:
	case sfgUnused5:
	case sfgEndOper:
		return 0;
	}
	return 1;
}


// SF2 8.1.1: Sample Generators are generators which directly 
//            affect a sample’s properties. These generators are undefined
//            at the preset level. The currently defined Sample Generators 
//            are the eight address offset generators, the sampleModes generator,
//            the Overriding Root Key generator and the Exclusive Class generator.
//            
//            Substitution Generators are generators which substitute a value
//            for a note-on parameter. Two Substitution Generators are currently
//            defined, overridingKeyNumber and overridingVelocity.
//
// SF2 8.1.2:
//            ExclusiveClass: The exclusive class generator can only appear at the instrument level.
//            Instrument: This generator can only appear at the instrument level
//            SampleID: ...should only appear in the IGEN sub-chunk.
int SFFile::ValidPresetGen(short gen)
{
	switch (gen)
	{
	case sfgKeynum:
	case sfgVelocity:
	case sfgSampleID:
	case sfgStartAddrsOffset:
	case sfgStartAddrsCoarseOffset:
	case sfgEndAddrsOffset:
	case sfgEndAddrsCoarseOffset:
	case sfgStartloopAddrsOffset:
	case sfgStartloopAddrsCoarseOffset:
	case sfgEndloopAddrsOffset:
	case sfgEndloopAddrsCoarseOffset:
	case sfgSampleModes:
	case sfgOverridingRootKey:
	case sfgExclusiveClass:
	case sfgUnused1:
	case sfgUnused2:
	case sfgUnused3:
	case sfgUnused4:
	case sfgUnused5:
	case sfgEndOper:
		return 0;
	}
	return 1;
}

// List of preset generators that are additive
int SFFile::AddPresetGen(short gen)
{
	switch (gen)
	{
	// Sample generators
	case sfgStartAddrsOffset:
	case sfgEndAddrsOffset:
	case sfgStartloopAddrsOffset:
	case sfgEndloopAddrsOffset:
	case sfgStartAddrsCoarseOffset:
	case sfgEndAddrsCoarseOffset:
	case sfgStartloopAddrsCoarseOffset:
	case sfgEndloopAddrsCoarseOffset:
	case sfgSampleModes:

	// Substitution generators
	case sfgExclusiveClass:
	case sfgOverridingRootKey:

	// Range generators
	case sfgKeyRange:
	case sfgVelRange:

	// Index generators
	case sfgInstrument:
	case sfgSampleID:

	case sfgKeynum:
	case sfgVelocity:

	case sfgUnused1:
	case sfgUnused2:
	case sfgUnused3:
	case sfgUnused4:
	case sfgUnused5:
	case sfgReserved1:
	case sfgReserved2:
	case sfgReserved3:
		// not applicable at PHDR level
		return 0;
	}
	return 1;
}

SBModInfo *SFFile::AddModulator(SBModList *zone, short src, short dst, short ctl, short amnt, short flags)
{
	if (!(src & 0x80) && (src & 0x7f) >= SBGEN_SIZE)
	{
		return NULL;
	}
	if ((dst & 0x7f) >= SBGEN_SIZE)
	{
		return NULL;
	}
	SBModInfo *mi = zone->GetModInfo(src, ctl, dst);
	if (mi)
	{
		if (SBGEN_PREFLG & (flags | mi->trnOp))
		{
			mi->scale += (float) amnt;
		}
		else
		{
			if (amnt == 0)
			{
				mi->Remove();
				delete mi;
			}
			else
			{
				mi->scale = (float)amnt;
				mi->trnOp &= ~SBGEN_PREFLG;
			}
		}
	}
	else if (amnt != 0)
	{
		mi = zone->AddModInfo();
		mi->srcOp = src & 0xff;
		mi->srcNf = ((src >> 10) & 0x0f) | ((src >> 3) & 0x20) | ((src >> 5) & 0x10);
		mi->ctlOp = ctl & 0xff;
		mi->ctlNf = ((ctl >> 10) & 0x0f) | ((ctl >> 3) & 0x20) | ((ctl >> 5) & 0x10);
		mi->dstOp = dst;
		mi->trnOp = flags;
		mi->scale = (float) amnt;
	}
	return mi;
}


void SFFile::ApplyModulator(SBZone *zone, sfModList *mp, int flags)
{
	short sbDst = MapDestination(mp->sfModDestOper);
	short sbSrc = MapSource(mp->sfModSrcOper);
	short sbCtl = MapSource(mp->sfModAmtSrcOper);
	if (sbSrc == -1 || sbCtl == -1)
		return; // SF2 8.2.1
//	int addMod = 0;
	switch(sbSrc)
	{
	case SBGEN_NONE:    // Constant
	case SBGEN_EG1:    // only volume
		return;
	case SBGEN_VELN:    // Note-on velocity
		if (sbDst == SBGEN_VOLUME)// && sbCtl == SBGEN_NONE)
			zone->velScale = mp->modAmount;
		break;
	case SBGEN_LFO1:
		zone->genFlags |= SBGEN_LFO1X;
		if (sbDst == SBGEN_PITCH)
		{
			if (sbCtl == SBGEN_NONE)
				zone->vibLfoFrq = mp->modAmount;
			else if (sbCtl == 0x81) // mod wheel
				zone->vibLfoMwFrq = mp->modAmount;
		}
		break;
	case SBGEN_LFO2:
		zone->genFlags |= SBGEN_LFO2X;
		switch (sbDst)
		{
		case SBGEN_PITCH:
			switch (sbCtl & 0xff)
			{
			case SBGEN_NONE:
				zone->modLfoFrq = mp->modAmount;
				break;
			case 0x81: // mod wheel
				zone->modLfoMwFrq = mp->modAmount;
				break;
			}
			break;
		case SBGEN_FILTER:
			if (mp->modAmount != 0)
				zone->genFlags |= SBGEN_FILTERX;
			switch (sbCtl & 0xff)
			{
			case SBGEN_NONE:
				zone->modLfoFlt = mp->modAmount;
				break;
			case 0x81: // mod wheel
				zone->modLfoMwFlt = mp->modAmount;
				break;
			}
			break;
		case SBGEN_VOLUME:
			switch (sbCtl & 0xff)
			{
			case SBGEN_NONE:
				zone->modLfoVol = mp->modAmount;
				break;
			case 0x81: // mod wheel
				zone->modLfoMwVol = mp->modAmount;
				break;
			}
			break;
		}
		break;
	case SBGEN_EG2:
		zone->genFlags |= SBGEN_EG2X;
		switch (sbDst & 0xff)
		{
		case SBGEN_PITCH:
			if (sbCtl == SBGEN_NONE)
				zone->modEnvFrq = mp->modAmount;
			break;
		case SBGEN_FILTER:
			if (mp->modAmount != 0)
				zone->genFlags |= SBGEN_FILTERX;
			if (sbCtl == SBGEN_NONE)
				zone->modEnvFlt = mp->modAmount;
			break;
		}
		break;
	}
	// catch-all
	//if (addMod)
	//	AddModulator(zone, sbSrc, sbDst, sbCtl, mp->modAmount, mp->sfModTransOper | flags);
}

short SFFile::MapDestination(short destination)
{
	switch (destination)
	{
	case sfgModLfoToPitch:
		return SBGEN_LFO2PC;
	case sfgVibLfoToPitch:
		return SBGEN_LFO1PC;
	case sfgModEnvToPitch:
		return SBGEN_EG2PC;
	case sfgModLfoToVolume:
		return SBGEN_LFO2CB;
	case sfgInitialAttenuation:
		return SBGEN_VOLUME;
	case sfgModLfoToFilterFc:
		return SBGEN_LFO2FC;
	case sfgModEnvToFilterFc:
		return SBGEN_EG2FC;
	case sfgInitialFilterFc:
		return SBGEN_FILTER;
	case sfgInitialFilterQ:
		return SBGEN_FILTQ;
	case sfgChorusEffectsSend:
		return SBGEN_CHORUS;
	case sfgReverbEffectsSend:
		return SBGEN_REVERB;
	case sfgPan:
		return SBGEN_PAN;
	case sfgDelayModLFO:
		return SBGEN_LFO2DLY;
	case sfgFreqModLFO:
		return SBGEN_LFO2FRQ;
	case sfgDelayVibLFO:
		return SBGEN_LFO1DLY;
	case sfgFreqVibLFO:
		return SBGEN_LFO1FRQ;
	case sfgDelayModEnv:
		return SBGEN_EG2DLY;
	case sfgAttackModEnv:
		return SBGEN_EG2ATK;
	case sfgHoldModEnv:
		return SBGEN_EG2HOLD;
	case sfgDecayModEnv:
		return SBGEN_EG2DEC;
	case sfgSustainModEnv:
		return SBGEN_EG2SUS;
	case sfgReleaseModEnv:
		return SBGEN_EG2REL;
	case sfgKeynumToModEnvHold:
		return SBGEN_EG2KEYH;
	case sfgKeynumToModEnvDecay:
		return SBGEN_EG2KEYD;
	case sfgDelayVolEnv:
		return SBGEN_EG1DLY;
	case sfgAttackVolEnv:
		return SBGEN_EG1ATK;
	case sfgHoldVolEnv:
		return SBGEN_EG1HOLD;
	case sfgDecayVolEnv:
		return SBGEN_EG1DEC;
	case sfgSustainVolEnv:
		return SBGEN_EG1SUS;
	case sfgReleaseVolEnv:
		return SBGEN_EG1REL;
	case sfgKeynumToVolEnvHold:
		return SBGEN_EG1KEYH;
	case sfgKeynumToVolEnvDecay:
		return SBGEN_EG1KEYD;
	}
	return -1;
}

short SFFile::MapSource(short source)
{
	short flags = source & 0x0f00;
	switch(source & 0xff)
	{
	case 0:   // No Controller
		return SBGEN_NONE;
	case 2:   // Note-on velocity
		return SBGEN_VELN | flags;
	case 3:   // Note-on key
		return SBGEN_KEYN | flags;
	case 10:  // Poly Pressure
		return SBGEN_PKEYAT | flags;
	case 13:  // Channel Pressure
		return SBGEN_CHNLAT | flags;
	case 14:  // Pitch Wheel
		return SBGEN_PITCHWHL | flags;
	case 16:  // Pitch Wheel Sensitivity
		return SBGEN_RPN0 | flags;
	case 127:
		return -1; // links not supported
	default:
		if (source & 0x80)
		{
			source &= 0x7F;
			if (source == 0
			 || source == 6
			 || source == 32
			 || source == 38
			 || (source >= 33 && source <= 63)
			 || (source >= 98 && source <= 101)
			 || (source >= 120 && source <= 127))
			{
				// SF2 8.2.1 - ignore illegal MIDI CC#
				return -1;
			}
			return SBGEN_CHCTL | source | flags;
		}
		break;
	}
	// Not defined in SF2 spec
	return -1;
}


void SFFile::BuildInstrument(SBZoneGroup *grp, int n, int pbagNdx, int gbagNdx)
{
	if (n < 0 || n >= ninsts)
		return;

	SBInstr *in = grp->instr;

	InitGenVals(globZone);
	globZone[sfgKeyRange].shAmount = loclPreset[sfgKeyRange].shAmount;
	globZone[sfgVelRange].shAmount = loclPreset[sfgVelRange].shAmount;

	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;
	short globBag = -1;
	sfSample *sh;
	sfGenList *ig;
	unsigned short sampID;

	int first = 1;
	int inszone = 0;
	SBZone *zone;
	SBSample *samp;
	bagNdx1 = inst[n].wInstBagNdx;
	bagNdx2 = inst[n+1].wInstBagNdx;
	while (bagNdx1 < bagNdx2)
	{
		memcpy(loclZone, globZone, sizeof(loclZone));

		inszone = 0;
		genNdx1 = ibag[bagNdx1].wGenNdx;
		genNdx2 = ibag[bagNdx1+1].wGenNdx;
		while (genNdx1 < genNdx2)
		{
			ig = &igen[genNdx1];
			if (ValidInstrGen(ig->sfGenOper))
			{
				// SF2 9.4 - A generator in a local instrument zone...replaces that generator.
				loclZone[ig->sfGenOper].shAmount = ig->genAmount.shAmount;
				if (ig->sfGenOper == sfgSampleID)
				{
					// SF2 8.1.2: The sampleID enumerator is the terminal generator
					//            for IGEN zones. ...it must appear as the last generator
					//            enumerator in all but the global zone.
					inszone = 1;

					// SF2 7.9 - If any generators follow a sampleID generator, they will be ignored.
					break;
				}
			}
			genNdx1++;
		}

		int inrange = grp->InRange(loclZone[sfgKeyRange].ranges.byLo,
								   loclZone[sfgKeyRange].ranges.byHi,
								   loclZone[sfgVelRange].ranges.byLo,
								   loclZone[sfgVelRange].ranges.byHi);
		if (inszone && inrange)
		{
			zone = in->AddZone();
			zone->zoneNdx = bagNdx1;
			zone->genFlags |= SBGEN_SF2;

			// SF2 9.4 - A generator at the preset level adds to generator at the instrument level.
			int g;
			for (g = 0; g < sfgEndOper; g++)
			{
				if (AddPresetGen(g))
					loclZone[g].shAmount += loclPreset[g].shAmount;
			}

			zone->volEg.delay = loclZone[sfgDelayVolEnv].shAmount;
			zone->volEg.attack = loclZone[sfgAttackVolEnv].shAmount;
			zone->volEg.hold = loclZone[sfgHoldVolEnv].shAmount;
			zone->volEg.decay = loclZone[sfgDecayVolEnv].shAmount;
			zone->volEg.sustain = loclZone[sfgSustainVolEnv].shAmount;
			zone->volEg.release = loclZone[sfgReleaseVolEnv].shAmount;
			zone->volEg.keyDecay = loclZone[sfgKeynumToVolEnvDecay].shAmount;
			zone->volEg.keyHold = loclZone[sfgKeynumToVolEnvHold].shAmount;
			zone->modEg.delay = loclZone[sfgDelayModEnv].shAmount;
			zone->modEg.attack = loclZone[sfgAttackModEnv].shAmount;
			zone->modEg.hold = loclZone[sfgHoldModEnv].shAmount;
			zone->modEg.decay = loclZone[sfgDecayModEnv].shAmount;
			zone->modEg.sustain = loclZone[sfgSustainModEnv].shAmount;
			zone->modEg.release = loclZone[sfgReleaseModEnv].shAmount;
			zone->modEg.keyDecay = loclZone[sfgKeynumToModEnvDecay].shAmount;
			zone->modEg.keyHold = loclZone[sfgKeynumToModEnvHold].shAmount;
			zone->vibLfo.rate = loclZone[sfgFreqVibLFO].shAmount;
			zone->vibLfo.delay = loclZone[sfgDelayVibLFO].shAmount;
			zone->modLfo.rate = loclZone[sfgFreqModLFO].shAmount;
			zone->modLfo.delay = loclZone[sfgDelayModLFO].shAmount;
			zone->filtFreq = loclZone[sfgInitialFilterFc].shAmount;
			zone->filtQ = loclZone[sfgInitialFilterQ].shAmount;
			zone->initAtten = loclZone[sfgInitialAttenuation].shAmount;
			zone->pan = loclZone[sfgPan].shAmount;
			zone->chorus = loclZone[sfgChorusEffectsSend].shAmount;
			zone->reverb = loclZone[sfgReverbEffectsSend].shAmount;
			zone->scaleTune = loclZone[sfgScaleTuning].wAmount;
			zone->fineTune = loclZone[sfgFineTune].wAmount;
			zone->coarseTune = loclZone[sfgCoarseTune].wAmount;

			if (zone->vibLfo.rate < -12000)
				zone->vibLfo.rate = -12000;
			if (zone->modLfo.rate < -12000)
				zone->modLfo.rate = -12000;

			sampID = loclZone[sfgSampleID].wAmount;
			sh = &shdr[sampID];
			zone->name = CopyName(nameTemp, sh->achSampleName);
			zone->cents = (bsInt16) -sh->chCorrection;
			zone->keyNum = sh->byOriginalKey;
			zone->rate = (FrqValue) sh->dwSampleRate;
			if (sh->sfSampleType == 4)
				zone->chan = 1;
			else if (sh->sfSampleType == 2)
				zone->chan = 0;
			else if (sh->sfSampleType == 8)
			{
				// hmmm... "not fully defined in SF2" (I'm guessing here...)
				sfSample *sh2 = &shdr[sh->wSampleLink];
				if (sh2->sfSampleType & 4)
					zone->chan = 1;
				else
					zone->chan = 0;
			}
			else
				zone->chan = 0; // mono sample
			samp = CreateSample(sh, sampID);
			zone->sample = samp;
			zone->sampleNdx = samp->index;

			zone->lowKey = loclZone[sfgKeyRange].ranges.byLo;
			zone->highKey = loclZone[sfgKeyRange].ranges.byHi;
			zone->lowVel = loclZone[sfgVelRange].ranges.byLo;
			zone->highVel = loclZone[sfgVelRange].ranges.byHi;
			zone->fixedKey = loclZone[sfgKeynum].shAmount;
			zone->fixedVel = loclZone[sfgVelocity].shAmount;

			if (loclZone[sfgOverridingRootKey].shAmount != -1)
				zone->keyNum = loclZone[sfgOverridingRootKey].shAmount;
			zone->exclNote = loclZone[sfgExclusiveClass].shAmount;
			zone->recCents = (zone->keyNum * 100) + zone->cents;
			zone->recFreq = 440.0 * SoundBank::GetPow2n1200(zone->recCents - 6900);
			zone->recPeriod = (bsInt32) (zone->rate / zone->recFreq);

			zone->mode = loclZone[sfgSampleModes].shAmount;
			zone->tableStart = (bsInt32)loclZone[sfgStartAddrsOffset].shAmount 
			                 + ((bsInt32)loclZone[sfgStartAddrsCoarseOffset].shAmount * 32768);
			zone->tableEnd = (bsInt32)(sh->dwEnd - sh->dwStart)
						   + (bsInt32)loclZone[sfgEndAddrsOffset].shAmount
						   + (bsInt32)(loclZone[sfgEndAddrsCoarseOffset].shAmount * 32768);
			zone->loopStart = (bsInt32)(sh->dwStartloop - sh->dwStart)
							+ (bsInt32)loclZone[sfgStartloopAddrsOffset].shAmount
							+ ((bsInt32)loclZone[sfgStartloopAddrsCoarseOffset].shAmount * 32768);
			zone->loopEnd = (bsInt32)(sh->dwEndloop - sh->dwStart)
						  + (bsInt32)loclZone[sfgEndloopAddrsOffset].shAmount
						  + ((bsInt32)loclZone[sfgEndloopAddrsCoarseOffset].shAmount * 32768);

			// Watch out for bogus files. We should probably 
			// throw the zone away, but for now, just make 
			// sure it won't crash.
			if (zone->tableStart < 0)
				zone->tableStart = 0;
			if (zone->tableEnd < 0)
				zone->tableEnd = 0;
			if (zone->loopStart < zone->tableStart)
				zone->loopStart = zone->tableStart;
			else if (zone->loopStart > zone->tableEnd)
				zone->loopStart = zone->tableEnd;
			if (zone->loopEnd < zone->loopStart)
				zone->loopEnd = zone->loopStart;
			else if (zone->loopEnd > zone->tableEnd)
				zone->loopEnd = zone->tableEnd;

			short val;
			// Add implied modulators from SF2 IGEN records.
			// Since these are IGEN records, the modulator replaces 
			// an identical modulator. When the scale value is zero
			// the modulator cannot have any effect on the output. 
			// An identical PGEN record will add to these values,
			// but since the value is zero, the PGEN record can
			// be applied directly to the summing node at playback.

			// SF2 8.4.4 Vibrato
			if ((val = loclZone[sfgVibLfoToPitch].shAmount) != 0)
			{
				zone->vibLfoFrq = val;
				zone->genFlags |= SBGEN_LFO1X;
			}

			// Modulator LFO
			if ((val = loclZone[sfgModLfoToVolume].shAmount) != 0)
			{
				zone->modLfoVol = val;
				zone->genFlags |= SBGEN_LFO2X;
			}

			if ((val = loclZone[sfgModLfoToPitch].shAmount) != 0)
			{
				zone->modLfoFrq = val;
				zone->genFlags |= SBGEN_LFO2X;
			}

			if ((val = loclZone[sfgModLfoToFilterFc].shAmount) != 0)
			{
				zone->modLfoFlt = val;
				zone->genFlags |= SBGEN_LFO2X;
			}

			// Modulator Envelope
			if ((val = loclZone[sfgModEnvToPitch].shAmount) != 0)
			{
				zone->modEnvFrq = val;
				zone->genFlags |= SBGEN_EG2X;
			}

			if ((val = loclZone[sfgModEnvToFilterFc].shAmount) != 0)
			{
				zone->modEnvFlt = val;
				zone->genFlags |= SBGEN_EG2X;
			}

			// Apply global zone mods
			if (globBag != -1)
			{
				genNdx1 = ibag[globBag].wModNdx;
				genNdx2 = ibag[globBag+1].wModNdx;
				while (genNdx1 < genNdx2)
				{
					ApplyModulator(zone, &imod[genNdx1]);
					genNdx1++;
				}
			}

			// Apply zone modulators (will replace globals if identical)
			genNdx1 = ibag[bagNdx1].wModNdx;
			genNdx2 = ibag[bagNdx1+1].wModNdx;
			while (genNdx1 < genNdx2)
			{
				ApplyModulator(zone, &imod[genNdx1]);
				genNdx1++;
			}

			// Apply preset modulators (will add if identical)
			genNdx1 = pbag[pbagNdx].wModNdx;
			genNdx2 = pbag[pbagNdx+1].wModNdx;
			while (genNdx1 < genNdx2)
			{
				ApplyModulator(zone, &pmod[genNdx1], SBGEN_PREFLG);
				genNdx1++;
			}

			// Apply global preset modulators (will add if identical)
			if (gbagNdx != -1)
			{
				genNdx1 = pbag[gbagNdx].wModNdx;
				genNdx2 = pbag[gbagNdx+1].wModNdx;
				while (genNdx1 < genNdx2)
				{
					ApplyModulator(zone, &pmod[genNdx1], SBGEN_PREFLG);
					genNdx1++;
				}
			}

			// Don't call AddZone until everything setup
			grp->AddZone(zone);
		}
		if (!inszone && first)
		{
			// SF2 7.7: If an instrument has more than one zone, the first zone may be
			//          a global zone. A global zone is determined by the fact that the
			//          last generator in the list is not a sampleID generator.
            //
			//          If a zone other than the first zone lacks a sampleID generator
			//          as its last generator, that zone should be ignored.
			globBag = bagNdx1;
			memcpy(globZone, loclZone, sizeof(globZone));
		}
		bagNdx1++;
		first = 0;
	}
}


void SFFile::BuildPreset(int n)
{
	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;

	memset(globPreset, 0, sizeof(globPreset));
	globPreset[sfgKeyRange].ranges.byLo = 0;
	globPreset[sfgKeyRange].ranges.byHi = 127;
	globPreset[sfgVelRange].ranges.byLo = 0;
	globPreset[sfgVelRange].ranges.byHi = 127;

	SBInstr *instr = sfbnk->AddInstr(phdr[n].wBank, phdr[n].wPreset);
	if (instr == 0)
		return;
	instr->instrNdx = n;
	instr->instrName = CopyName(nameTemp, phdr[n].achPresetName);

	int gbagNdx = -1;
	bagNdx1 = phdr[n].wPresetBagNdx;
	bagNdx2 = phdr[n+1].wPresetBagNdx;
	while (bagNdx1 < bagNdx2)
	{
		memcpy(loclPreset, globPreset, sizeof(loclPreset));

		int globValid = 1;
		int velRangeValid = 1;
		int instrIndex = 0;
		genNdx1 = pbag[bagNdx1].wGenNdx;
		genNdx2 = pbag[bagNdx1+1].wGenNdx;
		for ( ; genNdx1 < genNdx2; genNdx1++)
		{
			sfGenList *pg = &pgen[genNdx1];
			if (!ValidPresetGen(pg->sfGenOper))
				continue;

			// SF2 9.4: If a key range generator is present and not the first generator,
			//          it should be ignored. If a velocity range generator is present,
			//          and is preceded by a generator other than a key range generator,
			//          it should be ignored.
			if (pg->sfGenOper == sfgKeyRange)
			{
				if (genNdx1 != pbag[bagNdx1].wGenNdx)
					continue;
			}
			else if (pg->sfGenOper != sfgVelRange)
				velRangeValid = 0;
			if (pg->sfGenOper == sfgVelRange)
			{
				if (!velRangeValid)
					continue;
			}

			// SF2 9.4 - "A generator in a local preset zone that is 
			//            identical to a generator in a global preset zone
			//            supersedes or replaces that generator..."
			loclPreset[pg->sfGenOper].shAmount = pg->genAmount.shAmount;
			if (pg->sfGenOper == sfgInstrument)
			{
				// SF2 8.1.2 - The instrument enumerator is the terminal generator
				//             for PGEN zones. As such, it should only appear in the
				//             PGEN sub-chunk, and it must appear as the last generator
				//
				// SF2 7.5:    Unless the zone is a global zone, the last generator in 
				//             the list is an "Instrument" generator, whose value is a
				//             pointer to the instrument associated with that zone.
				instrIndex = 1;
				SBZoneGroup *grp = instr->AddGroup();
				if (grp == 0)
					break;
				grp->index = bagNdx1;
				grp->lowKey = loclPreset[sfgKeyRange].ranges.byLo;
				grp->highKey = loclPreset[sfgKeyRange].ranges.byHi;
				grp->lowVel = loclPreset[sfgVelRange].ranges.byLo;
				grp->highVel = loclPreset[sfgVelRange].ranges.byHi;
				BuildInstrument(grp, pg->genAmount.shAmount, bagNdx1, gbagNdx);

				// SF2 7.5: If any generators follow an Instrument generator, they will be ignored.
				break;
			}
		}
		// SF2 7.3: If a preset has more than one zone, the first zone may be a global zone.
		//          A global zone is determined by the fact that the last generator in the
		//          list is not an Instrument generator.
		//
        //          If a zone other than the first zone lacks an Instrument generator as its
		//          last generator, that zone should be ignored. 
		if (!instrIndex && globValid)
		{
			gbagNdx = bagNdx1;
			memcpy(globPreset, loclPreset, sizeof(globPreset));
		}
		globValid = 0;
		bagNdx1++;
	}
	instr->loaded = preload;
}

void SFFile::BuildSoundBank()
{
	int n, m;

	m = npresets-1;
	for (n = 0; n < m; n++)
		BuildPreset(n);
}

AmpValue SFFile::SFAttenuation(short amt)
{
	if (amt <= 0)
		return 0;
	if (amt >= 1440.0)
		return 1440.0;
	return (AmpValue) amt;
}

char *SFFile::CopyName(char *dst, char *src)
{
	memcpy(dst, src, 20);
	dst[20] = '\0';
	int ndx;
	for (ndx = 19; ndx >= 0 && dst[ndx] == ' '; ndx--)
		dst[ndx] = '\0';
	return dst;
}


void SFFile::InitGenVals(genAmountType *genVals)
{
	// SF2 8.1 - Generator summary
	memset(genVals, 0, sizeof(genAmountType)*sfgEndOper);
	genVals[sfgInitialFilterFc].shAmount = 13500;
	genVals[sfgInitialFilterQ].shAmount = 0;
	genVals[sfgScaleTuning].shAmount = 100;

	// for some of the rates, spec says '-12000', < 1ms
	// which translates to about 0.9 ms.
	// For true 0 - use the DLS convetion: 0x8000 == 0
	genVals[sfgDelayVibLFO].shAmount = -32768; 
	genVals[sfgDelayModLFO].shAmount = -32768;

	genVals[sfgDelayModEnv].shAmount = -32768;
	genVals[sfgAttackModEnv].shAmount = -12000;
	genVals[sfgHoldModEnv].shAmount = -32768;
	genVals[sfgDecayModEnv].shAmount = -12000;
	genVals[sfgSustainModEnv].shAmount = 0;
	genVals[sfgReleaseModEnv].shAmount = -12000;

	genVals[sfgDelayVolEnv].shAmount = -32768;
	genVals[sfgAttackVolEnv].shAmount = -12000;
	genVals[sfgHoldVolEnv].shAmount = -32768;
	genVals[sfgDecayVolEnv].shAmount = -12000;
	genVals[sfgSustainVolEnv].shAmount = 0;
	genVals[sfgReleaseVolEnv].shAmount = -12000;

	genVals[sfgVelRange].ranges.byHi = 127;
	genVals[sfgKeyRange].ranges.byHi = 127;
	genVals[sfgKeynum].shAmount = -1;
	genVals[sfgVelocity].shAmount = -1;
	genVals[sfgOverridingRootKey].shAmount = -1;
}
