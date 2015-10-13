/////////////////////////////////////////////////////////////////////////
// BasicSynth - Example 4a
//
// Soundfont dump/player
//
// use: example4a soundfont -bbank -ppreset -m -d(1|2|3) -wprefix -n[0|1] -l[0|1]
// -d1 = print internal representation
// -d2 = print raw file contents
// -d3 = print both
// -m  = mono
// -n  = normalize
// -l  = preload all sounds
/////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SynthDefs.h"
#include "SynthList.h"
#include "WaveTable.h"
#include "WaveFile.h"
#include "EnvGenSeg.h"
#include "GenWaveWT.h"
#include "Filter.h"
#include "Mixer.h"
#include "SFFile.h"
#include "DLSFile.h"
#include "SFGen.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

const char *opNames[] =
{
"0 startAddrsOffset",
"1 endAddrsOffset",
"2 startloopAddrsOffset",
"3 endloopAddrsOffset",
"4 startAddrsCoarseOffset",
"5 modLfoToPitch",
"6 vibLfoToPitch",
"7 modEnvToPitch",
"8 initialFilterFc",
"9 initialFilterQ",
"10 modLfoToFilterFc",
"11 modEnvToFilterFc",
"12 endAddrsCoarseOffset",
"13 modLfoToVolume",
"14 unused1",
"15 chorusEffectsSend ",
"16 reverbEffectsSend",
"17 pan",
"18 unused2",
"19 unused3",
"20 unused4",
"21 delayModLFO",
"22 freqModLFO",
"23 delayVibLFO",
"24 freqVibLFO",
"25 delayModEnv",
"26 attackModEnv",
"27 holdModEnv",
"28 decayModEnv",
"29 sustainModEnv",
"30 releaseModEnv",
"31 keynumToModEnvHold",
"32 keynumToModEnvDecay",
"33 delayVolEnv",
"34 attackVolEnv",
"35 holdVolEnv",
"36 decayVolEnv",
"37 sustainVolEnv",
"38 releaseVolEnv",
"39 keynumToVolEnvHold",
"40 keynumToVolEnvDecay",
"41 instrument",
"42 reserved1",
"43 keyRange",
"44 velRange",
"45 startloopAddrsCoarseOffset",
"46 keynum ",
"47 velocity",
"48 initialAttenuation",
"49 reserved2",
"50 endloopAddrsCoarseOffset",
"51 coarseTune",
"52 fineTune",
"53 sampleID",
"54 sampleModes",
"55 reserved3",
"56 scaleTuning",
"57 exclusiveClass",
"58 overridingRootKey",
"59 unused5",
"60 endOper"
};

const char *dstNames[] =
{
	"None",
	"Pitch",
	"Volume",
	"Filter",
	"Filt. Q",
	"Pan",
	"Chorus",
	"Reverb",
	"VibLFO-Pitch",
	"ModLFO-Pitch",
	"ModLFO-Volume",
	"ModLFO-Filter",
	"ModEnv-Pitch",
	"ModEnv-Filter",
	"VibLFO Rate",
	"VibLFO Delay",
	"ModLFO Rate",
	"ModLFO Delay",
	"EG1 Dly.",
	"EG1 Atk.",
	"EG1 Hold",
	"EG1 Dec.",
	"EG1 Sus.",
	"EG1 Rel.",
	"EG1 Key-Hold",
	"EG1 Key-Dec.",
	"EG1 Vel-Atk.",
	"EG1 Shutdown",
	"EG2 Dly.",
	"EG2 Atk.",
	"EG2 Hold",
	"EG2 Dec.",
	"EG2 Sus.",
	"EG2 Rel.",
	"EG2 Key-Hold",
	"EG2 Key-Dec.",
	"EG2 Vel-Atk.",
	"Key Number",
	"Velocity",
	"Pitch Wheel",
	"Channel Pressure",
	"Key Pressure",
	"Pitch Wheel Range",
	"VibLFO",
	"ModLFO",
	"VolEG",
	"Modeg"
};

static const char *SBDstName(short ndx)
{
	if (ndx >= 0 && ndx < SBGEN_SIZE)
		return dstNames[ndx];
	return "(dst)";
}

static char srcCC[40];
static const char *SBSrcName(short ndx)
{
	if (ndx & SBGEN_CHCTL)
	{
		snprintf(srcCC, 40, "CC #%d", ndx & 0x7f);
		return srcCC;
	}

	switch (ndx)
	{
	// internal unit generators
	case SBGEN_NONE:
		return "None";
	case SBGEN_LFO1:
		return "VibLFO";
	case SBGEN_LFO2:
		return "ModLFO";
	case SBGEN_EG1:
		return "VolEG";
	case SBGEN_EG2:
		return "ModEG";

	// MIDI Channel voice message source
	case SBGEN_KEYN:
		return "Note-on Key";
	case SBGEN_VELN:
		return "Note-on Velocity";
	case SBGEN_CHNLAT:
		return "Channel pressure";
	case SBGEN_PKEYAT:
		return "Key pressure";
	case SBGEN_PITCHWHL:
		return "Pitch Wheel";

	case SBGEN_RPN0:
		return "Pit. Whl. Sens.";
	}
	return "Other";
}

static char opUndef[80];
static const char *OpName(short ndx)
{
	if (ndx & 0x80)
	{
		snprintf(opUndef, 80, "CC #%d", ndx & 0x7f);
		return opUndef;
	}
	if (ndx < 0 || ndx > 60)
	{
		snprintf(opUndef, 80, "%d undefined", ndx);
		return opUndef;
	}
	return opNames[ndx];
}

static char modUndef[80];
static const char *ModName(short ndx)
{
	if (ndx & 0x80)
	{
		snprintf(modUndef, 80, "CC# %d", ndx & 0x7f);
		return modUndef;
	}
	else
	{
		switch (ndx & 0x7f)
		{
		case 0:
			return "None";
		case 2:
			return "Note-on velocity";
		case 3:
			return "Note-on key";
		case 10:
			return "Poly Pressure";
		case 13:
			return "Channel Pressure";
		case 14:
			return "Pitch Wheel";
		case 16:
			return "Pitch Wheel Sens.";
		case 127:
			return "Link";
		}
	}
	snprintf(modUndef, 80, "Mod %04x", ndx);
	return modUndef;
}

static char ctlUndef[80];
static const char *CtlName(short ndx)
{
	if (ndx == 0)
		return "None";

	if (ndx & 0x80)
	{
		snprintf(ctlUndef, 80, "CC# %d", ndx & 0x7f);
		return ctlUndef;
	}
	return ModName(ndx);
}

char opAmntBuf[40];
const char *OpAmount(sfGenList& gen)
{
	switch (gen.sfGenOper)
	{
	case sfgKeyRange:
	case sfgVelRange:
		snprintf(opAmntBuf, 40, "{%d,%d}", (int)gen.genAmount.ranges.byLo, (int)gen.genAmount.ranges.byHi);
		break;
	default:
		snprintf(opAmntBuf, 40, "%d", (int)gen.genAmount.shAmount);
		break;
	}
	return opAmntBuf;
}

FrqValue FrqCents(FrqValue pc)
{
	return pow(2.0, pc / 1200.0);
}

void Dump(SBZone *zp)
{
	printf("  Zone[%d] %s\n", zp->zoneNdx, (const char *)zp->name);
	printf("   Range: key={%d,%d} vel={%d,%d} ex:%d\n",
		zp->lowKey, zp->highKey, zp->lowVel, zp->highVel, zp->exclNote);
	printf("   Tune: coarse=%d fine=%d key=%d vel=%d scale=%d\n",
		zp->coarseTune, zp->fineTune, zp->fixedKey, zp->fixedVel, zp->scaleTune);
	printf("   Vol EG: Dly=%6.3f Atk=%6.3f Hld=%6.3f Dec=%6.3f Sus=%6.3f Rel=%6.3f Vol=%6.3f\n",
		zp->volEg.delay, zp->volEg.attack,
		zp->volEg.hold, zp->volEg.decay,
		zp->volEg.sustain, zp->volEg.release,
		zp->initAtten);
	float susval = zp->volEg.sustain;
	if (zp->genFlags & SBGEN_SF2)
	{
		if (susval < 960)
			susval = (960.0 - susval) / 960;
		else
			susval = 0;
	}
	else
		susval *= 0.001;
	printf("         : Dly=%6.3f Atk=%6.3f Hld=%6.3f Dec=%6.3f Sus=%6.3f Rel=%6.3f Vol=%6.3f\n",
		SoundBank::EnvRate(zp->volEg.delay),
		SoundBank::EnvRate(zp->volEg.attack),
		SoundBank::EnvRate(zp->volEg.hold),
		SoundBank::EnvRate(zp->volEg.decay),
		susval,
		SoundBank::EnvRate(zp->volEg.release),
		SoundBank::Attenuation(zp->initAtten));
	printf("   Mod EG: Dly=%6.3f Atk=%6.3f Hld=%6.3f Dec=%6.3f Sus=%6.3f Rel=%6.3f\n",
		zp->modEg.delay, zp->modEg.attack,
		zp->modEg.hold, zp->modEg.decay,
		zp->modEg.sustain, zp->modEg.release);
	printf("         : Dly=%6.3f Atk=%6.3f Hld=%6.3f Dec=%6.3f Sus=%6.3f Rel=%6.3f\n",
		SoundBank::EnvRate(zp->modEg.delay),
		SoundBank::EnvRate(zp->modEg.attack),
		SoundBank::EnvRate(zp->modEg.hold),
		SoundBank::EnvRate(zp->modEg.decay),
		zp->modEg.sustain * 0.001,
		SoundBank::EnvRate(zp->modEg.release));
	printf("         : ToPitch=%6.3f ToFilter=%6.3f\n", zp->modEnvFrq, zp->modEnvFlt);
	printf("   Vib LFO: Frq=%6.3f (%6.3fHz) Dly=%6.3f (%6.3fsec) ToPitch=%6.3f\n",
		zp->vibLfo.rate, SoundBank::Frequency(zp->vibLfo.rate),
		zp->vibLfo.delay, SoundBank::EnvRate(zp->vibLfo.delay),
		zp->vibLfoFrq);
	printf("   Mod LFO: Frq=%6.3f (%6.3fHz) Dly=%6.3f (%6.3fsec) ToPitch=%6.3f ToFilter=%6.3f ToVol=%6.3f\n",
		zp->modLfo.rate, SoundBank::Frequency(zp->modLfo.rate),
		zp->modLfo.delay, SoundBank::EnvRate(zp->modLfo.delay),
		zp->modLfoFrq, zp->modLfoFlt, zp->modLfoVol);
	printf("   Pan: %6.3f\n", zp->pan);
	printf("   Filter: Fc=%6.3f (%6.3f Hz) Q=%6.3f\n",
		zp->filtFreq, SoundBank::Frequency(zp->filtFreq), zp->filtQ);
	printf("   Sample: sr=%5.1f rf=%5.3f chan=%d key=%d cents=%d table{%d,%d} loop{%d,%d} mode=%d\n",
		zp->rate, zp->recFreq, zp->chan, zp->keyNum, zp->cents,
		zp->tableStart, zp->tableEnd, zp->loopStart, zp->loopEnd, zp->mode);
	SBModInfo *mi = 0;
/*	while ((mi = zp->EnumIniInfo(mi)) != 0)
	{
		printf("   Ini: src=%s dst=%s ctl=%s val=%f trn=%d \n",
			SBSrcName(mi->srcOp), SBDstName(mi->dstOp),
			SBSrcName(mi->ctlOp), mi->scale, mi->trnOp);
	}
*/	while ((mi = zp->EnumModInfo(mi)) != 0)
	{
		printf("   Mod: src=%s dst=%s ctl=%s val=%f trn=%d \n",
			SBSrcName(mi->srcOp), SBDstName(mi->dstOp),
			SBSrcName(mi->ctlOp), mi->scale, mi->trnOp);
	}
	printf("   data addr=0x%08x length=%d\n", zp->sample, zp->sample->sampleLen);
}

void Dump(SBZoneGroup *grp)
{
	printf("  Group %d: key={%d,%d} vel={%d,%d}\n", grp->index,
		grp->lowKey, grp->highKey, grp->lowVel, grp->highVel);
//	int key = grp->lowKey;
//	while (key < grp->highKey)
//	{
//		SBZoneRef *ref = grp->map[key];
//		while (ref)
//		{
//			printf("  key %d: Zone[%d] %s\n", key, ref->zone->zoneNdx, (const char *)ref->zone->name);
//			ref = ref->next;
//		}
//		key++;
//	}
}

void Dump(SBInstr *ip)
{
	printf(" Instrument '%s' bank=%d prog=%d\n", (const char*)ip->instrName, ip->bank, ip->prog);
	SBModInfo *mi = 0;
	while ((mi = ip->EnumModInfo(mi)) != 0)
	{
		printf("   Mod: src=%s dst=%s ctl=%s val=%6.3f trn=%d \n",
			SBSrcName(mi->srcOp), SBDstName(mi->dstOp),
			SBSrcName(mi->ctlOp), mi->scale, mi->trnOp);
	}
	SBZoneGroup *grp = 0;
	while ((grp = ip->EnumGroups(grp)) != 0)
		Dump(grp);
	SBZone *z = 0;
	while ((z = ip->EnumZones(z)) != 0)
		Dump(z);
}


void Dump(SoundBank *sb)
{
	printf("Name '%s'\n", (const char *)sb->info.szName);
	printf("Copy '%s'\n", (const char *)sb->info.szCopyright);
	printf("Cmnt '%s'\n", (const char *)sb->info.szComment);
	printf("Date '%s'\n", (const char *)sb->info.szDate);
	printf("Vers %d.%d %d.%d\n",
		sb->info.wMajorFile, sb->info.wMinorFile,
		sb->info.wMajorVer, sb->info.wMinorVer);

	for (int bnkNo = 0; bnkNo < 129; bnkNo++)
	{
		if (sb->instrBank[bnkNo] != 0)
		{
			for (int preNo = 0; preNo < 128; preNo++)
			{
				SBInstr *p = sb->instrBank[bnkNo][preNo];
				if (p)
					Dump(p);
			}
		}
	}
}

void Dump(SFFile *sf)
{
	char nameTemp[24];
	short bagNdx1, bagNdx2;
	short genNdx1, genNdx2;
	short modNdx1, modNdx2;
	short ibagNdx1, ibagNdx2;
	short igenNdx1, igenNdx2;
	short imodNdx1, imodNdx2;
	int n;

	if (sf->sfbnk)
	{
		printf("File version: %d.%d\n", sf->sfbnk->info.wMajorFile, sf->sfbnk->info.wMinorFile);
		printf("Version     : %d.%d\n", sf->sfbnk->info.wMajorVer, sf->sfbnk->info.wMinorVer);
		printf("Sound Engine: %s\n", (const char *)sf->sfbnk->info.szSoundEngine);
		printf("Name        : %s\n", (const char *)sf->sfbnk->info.szName);
		printf("Date        : %s\n", (const char *)sf->sfbnk->info.szDate);
		printf("Engineer    : %s\n", (const char *)sf->sfbnk->info.szEng);
		printf("Product     : %s\n", (const char *)sf->sfbnk->info.szProduct);
		printf("Copyright   : %s\n", (const char *)sf->sfbnk->info.szCopyright);
		printf("Comment     : %s\n", (const char *)sf->sfbnk->info.szComment);
		printf("Tools       : %s\n", (const char *)sf->sfbnk->info.szTools);
	}

	printf("Samples     : total=%d offs1=%u offs2=%u\n", sf->sampleSize, sf->sampleFileOffs1, sf->sampleFileOffs2);

	int m = sf->npresets-1;
	for (n = 0; n < m; n++)
	{
		memcpy(nameTemp, sf->phdr[n].achPresetName, 20);
		nameTemp[20] = 0;
		bagNdx1 = sf->phdr[n].wPresetBagNdx;
		bagNdx2 = sf->phdr[n+1].wPresetBagNdx;
		printf("PHDR %d: '%-20s' Bank %d, Preset %d, bags: %d -> %d\n", n, nameTemp, sf->phdr[n].wBank, sf->phdr[n].wPreset, bagNdx1, bagNdx2);
		while (bagNdx1 < bagNdx2)
		{
			genNdx1 = sf->pbag[bagNdx1].wGenNdx;
			modNdx1 = sf->pbag[bagNdx1].wModNdx;
			genNdx2 = sf->pbag[bagNdx1+1].wGenNdx;
			modNdx2 = sf->pbag[bagNdx1+1].wModNdx;
			printf(" PBAG %d: GEN = %d:%d MOD = %d:%d\n", bagNdx1, genNdx1, genNdx2, modNdx1, modNdx2);
			while (modNdx1 < modNdx2)
			{
				printf("  PMOD %d: srcOp[%02x]='%s' ctlOp[%02x]='%s' dstOp[%02x]='%s' amnt = %d\n", modNdx1,
					sf->pmod[modNdx1].sfModSrcOper, ModName(sf->pmod[modNdx1].sfModSrcOper),
					sf->pmod[modNdx1].sfModAmtSrcOper, CtlName(sf->pmod[modNdx1].sfModAmtSrcOper),
					sf->pmod[modNdx1].sfModDestOper, OpName(sf->pmod[modNdx1].sfModDestOper),
					(int)sf->pmod[modNdx1].modAmount);
				modNdx1++;
			}
			while (genNdx1 < genNdx2)
			{
				printf("  PGEN %d: genOp[%d] = %s amnt = %s\n", genNdx1,
					sf->pgen[genNdx1].sfGenOper, OpName(sf->pgen[genNdx1].sfGenOper),
					OpAmount(sf->pgen[genNdx1]));
				if (sf->pgen[genNdx1].sfGenOper == sfgInstrument)
				{
					int nn = sf->pgen[genNdx1].genAmount.wAmount;
					memcpy(nameTemp, sf->inst[nn].achInstName, 20);
					nameTemp[20] = 0;
					ibagNdx1 = sf->inst[nn].wInstBagNdx;
					ibagNdx2 = sf->inst[nn+1].wInstBagNdx;
					printf("  INST %d: '%-20s' bags: %d -> %d\n", nn, nameTemp, ibagNdx1, ibagNdx2);
					while (ibagNdx1 < ibagNdx2)
					{
						igenNdx1 = sf->ibag[ibagNdx1].wGenNdx;
						igenNdx2 = sf->ibag[ibagNdx1+1].wGenNdx;
						imodNdx1 = sf->ibag[ibagNdx1].wModNdx;
						imodNdx2 = sf->ibag[ibagNdx1+1].wModNdx;
						printf("   IBAG %d: GEN=%d:%d MOD=%d:%d\n", ibagNdx1, igenNdx1, igenNdx2, imodNdx1, imodNdx2);
						while (imodNdx1 < imodNdx2)
						{
							printf("    IMOD %d: srcOp[%02x]='%s' ctlOp[%02x]='%s' dstOp[%02x]='%s' amnt=%d\n", imodNdx1,
								sf->imod[imodNdx1].sfModSrcOper, ModName(sf->imod[imodNdx1].sfModSrcOper),
								sf->imod[imodNdx1].sfModAmtSrcOper, CtlName(sf->imod[imodNdx1].sfModAmtSrcOper),
								sf->imod[imodNdx1].sfModDestOper, OpName(sf->imod[imodNdx1].sfModDestOper),
								sf->imod[imodNdx1].modAmount);
							imodNdx1++;
						}
						while (igenNdx1 < igenNdx2)
						{
							printf("    IGEN %d: genOp[%d]='%s' amnt=%s\n", igenNdx1,
								sf->igen[igenNdx1].sfGenOper, OpName(sf->igen[igenNdx1].sfGenOper),
								OpAmount(sf->igen[igenNdx1]));
							if (sf->igen[igenNdx1].sfGenOper == 53) // sampleID)
							{
								int id = sf->igen[igenNdx1].genAmount.wAmount;
								memcpy(nameTemp, sf->shdr[id].achSampleName, 20);
								printf("     SAMP: %d %s st=%d end=%d ls=%d le=%d sr=%d k=%d ch=%d ty=%d lnk=%d\n",
									id, nameTemp,
									sf->shdr[id].dwStart, sf->shdr[id].dwEnd,
									sf->shdr[id].dwStartloop, sf->shdr[id].dwEndloop,
									sf->shdr[id].dwSampleRate, sf->shdr[id].byOriginalKey,
									sf->shdr[id].chCorrection,
									sf->shdr[id].sfSampleType, sf->shdr[id].wSampleLink);
							}
							igenNdx1++;
						}
						ibagNdx1++;
					}
				}
				genNdx1++;
			}
			bagNdx1++;
		}
	}
}

double DLSScale(bsInt32 val)
{
	return (double)val / 65536.0;
}

FrqValue DLSTimeCents(bsInt32 tc)
{
	return pow(2.0, ((double)tc / (1200.0*65536.0)));
}

AmpValue DLSRelGain(bsInt32 cb)
{
	double x = (double) cb / 65536.0;
	if (x >= 960.0)
		return 0.0;
	return pow(10.0, x / -200.0);
}

FrqValue DLSPitchCents(bsInt32 pc)
{
	return 440.0 * pow(2.0, (((double)pc / 65536.0) - 6900.0) / 1200.0);
}

double DLSPercent(bsInt32 val)
{
	 return (double)val / (1000.0*65536.0);
}

void DLSSource(int source)
{
	switch (source)
	{
	case CONN_SRC_NONE:
		printf("SRC_NONE ");
		break;
	case CONN_SRC_LFO:
		printf("SRC_LFO ");
		break;
	case CONN_SRC_KEYONVELOCITY:
		printf("SRC_KEYONVELOCITY ");
		break;
	case CONN_SRC_KEYNUMBER:
		printf("SRC_KEYNUMBER ");
		break;
	case CONN_SRC_EG1:
		printf("SRC_EG1 ");
		break;
	case CONN_SRC_EG2:
		printf("SRC_EG2 ");
		break;
	case CONN_SRC_PITCHWHEEL:
		printf("SRC_PITCHWHEEL ");
		break;
	case CONN_SRC_POLYPRESSURE:
		printf("CONN_SRC_POLYPRESSURE ");
		break;
	case CONN_SRC_CHANNELPRESSURE:
		printf("CONN_SRC_CHANNELPRESSURE ");
		break;
	case CONN_SRC_VIBRATO:
		printf("CONN_SRC_VIBRATO ");
		break;
	case CONN_SRC_MONOPRESSURE:
		printf("CONN_SRC_MONOPRESSURE ");
		break;
	// Midi Controllers 0-127
	case CONN_SRC_CC1:
		printf("SRC_CC1 ");
		break;
	case CONN_SRC_CC7:
		printf("SRC_CC7 ");
		break;
	case CONN_SRC_CC10:
		printf("SRC_CC10 ");
		break;
	case CONN_SRC_CC11:
		printf("SRC_CC11 ");
		break;
	default:
		if (source & 0x80)
			printf("SRC_CC%d ", source & 0x7f);
		else
			printf("CONN_SRC UNKNOWN (%04x) ", source);
		break;
	}
}

void DLSDestination(dlsConnection *blk)
{
	switch (blk->destination)
	{
	// Generic Destinations
	case CONN_DST_NONE:
		printf("DST_NONE");
		break;
	case CONN_DST_ATTENUATION:
		printf("DST_ATTENUATION %f (%f)", DLSScale(blk->scale), DLSRelGain(blk->scale));
		break;
	case CONN_DST_RESERVED:
		printf("DST_RESERVED %d", DLSScale(blk->scale));
		break;
	case CONN_DST_PITCH:
		printf("DST_PITCH %d", DLSScale(blk->scale));
		break;
	case CONN_DST_PAN:
		printf("DST_PAN %f", ((double)blk->scale / (10.0 * 65536.0)) / 100.0);
		break;
	// LFO Destinations
	case CONN_DST_LFO_FREQUENCY:
		printf("DST_LFO_FREQUENCY %f", DLSPitchCents(blk->scale));
		break;
	case CONN_DST_LFO_STARTDELAY:
		printf("DST_LFO_STARTDELAY %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_VIB_FREQUENCY:
		printf("DST_VIB_FREQUENCY %f", DLSPitchCents(blk->scale));
		break;
	case CONN_DST_VIB_STARTDELAY:
		printf("DST_VIB_STARTDELAY %f", DLSTimeCents(blk->scale));
		break;
	// EG1 Destinations
	case CONN_DST_EG1_DELAYTIME:
		printf("CONN_DST_EG1_DELAYTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_ATTACKTIME:
		printf("DST_EG1_ATTACKTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_HOLDTIME:
		printf("CONN_DST_EG1_HOLDTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_DECAYTIME:
		printf("DST_EG1_DECAYTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_RESERVED:
		printf("DST_EG1_RESERVED %f ", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_RELEASETIME:
		printf("DST_EG1_RELEASETIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG1_SUSTAINLEVEL:
		printf("DST_EG1_SUSTAINLEVEL %f", DLSPercent(blk->scale));
		break;
	case CONN_DST_EG1_SHUTDOWNTIME:
		printf("CONN_DST_EG1_SHUTDOWNTIME %f", DLSPercent(blk->scale));
		break;
	// EG2 Destinations
	case CONN_DST_EG2_DELAYTIME:
		printf("CONN_DST_EG2_DELAYTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_ATTACKTIME:
		printf("DST_EG2_ATTACKTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_HOLDTIME:
		printf("CONN_DST_EG2_HOLDTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_DECAYTIME:
		printf("DST_EG2_DECAYTIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_RESERVED:
		printf("DST_EG2_RESERVED %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_RELEASETIME:
		printf("DST_EG2_RELEASETIME %f", DLSTimeCents(blk->scale));
		break;
	case CONN_DST_EG2_SUSTAINLEVEL:
		printf("DST_EG2_SUSTAINLEVEL %f", DLSPercent(blk->scale));
		break;
	case CONN_DST_FLT_CUTOFF:
		printf("CONN_DST_FLT_CUTOFF %f", DLSScale(blk->scale));
		break;
	case CONN_DST_FLT_Q:
		printf("CONN_DST_FLT_Q %f", DLSScale(blk->scale));
		break;
	default:
		if (blk->destination & 0x80)
			printf("DST_CC%d %f", blk->destination & 0x7f, DLSScale(blk->scale));
		else
			printf("CONN_DST UNKNOWN (%04x) %f", blk->destination, DLSScale(blk->scale));
		break;
	}
}

void Dump(DLSArtInfo *art)
{
	printf("    Artic %c:\n", art->art2 ? '2' : '1');
	for (bsUint32 n = 0; n < art->connections; n++)
	{
		dlsConnection *blk = &art->info[n];
		printf("      blk(%d) ctl=%d src=%x dst=%x xf=%x scl=%d (%7.4f)\n              ", n,
			blk->control, blk->source,
			blk->destination, blk->transform, blk->scale, (float) blk->scale / 65536.0);
		DLSSource(blk->source);
		if (blk->control != 0)
		{
			printf(" CONTROL ");
			DLSSource(blk->control);
		}
		DLSDestination(blk);
		printf("\n");
	}
}

void Dump(DLSRgnInfo *rgn)
{
	printf("    Region %c grp=%d opt=%d key={%d,%d} vel={%d,%d} ex=%d\n",
		rgn->rgn2 ? '2' : '1',
		rgn->rgnh.keyGroup, rgn->rgnh.options,
		rgn->rgnh.rangeKey.low, rgn->rgnh.rangeKey.high,
		rgn->rgnh.rangeVel.low, rgn->rgnh.rangeVel.high,
		rgn->rgnh.exclusive);

	printf("           atn=%d (%f) ft=%d (%f) opt=%d loop=%d key=%d\n",
		rgn->wsmp.attenuation, pow(10.0, (double)rgn->wsmp.attenuation / (200.0 * 65536.0)),
		rgn->wsmp.fineTune, pow(2.0, (double)rgn->wsmp.fineTune / 1200.0),
		rgn->wsmp.options,	rgn->wsmp.sampleLoops, rgn->wsmp.unityNote);
	printf("           loop len=%d sz=%d st=%d ty=%d\n",
		rgn->loop.length, rgn->loop.size, rgn->loop.start, rgn->loop.type);
	printf("           ch=%d opt=%d pg=%d ti=%d\n",
		rgn->wlnk.channel, rgn->wlnk.options, rgn->wlnk.phaseGroup, rgn->wlnk.tableIndex);

	DLSArtInfo *art = 0;
	while ((art = rgn->lart.EnumArt(art)) != 0)
		Dump(art);
}

void Dump(DLSInsInfo *in)
{
	printf("  Instr: bnk=%d prg=%d m/d=%d\n",
		in->GetBank(), in->GetProg(), in->IsDrum());
	printf("     ID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n",
		in->id.data1, in->id.data2, in->id.data3,
		in->id.data4[0], in->id.data4[1], in->id.data4[2], in->id.data4[3],
		in->id.data4[4], in->id.data4[5], in->id.data4[6], in->id.data4[7]);

	DLSInfoStr *is = 0;
	while ((is = in->info.EnumInfo(is)) != 0)
		printf("      '%s'\n", (const char *)is->str);

	DLSRgnInfo *rgn = 0;
	while ((rgn = in->rgnlist.EnumRgn(rgn)) != 0)
		Dump(rgn);

	DLSArtInfo *art = 0;
	while ((art = in->lart.EnumArt(art)) != 0)
		Dump(art);
}

void Dump(DLSWaveInfo *wvi)
{
	printf("  Wave %d size=%d\n", wvi->index, wvi->sampsize);

	DLSInfoStr *is = 0;
	while ((is = wvi->info.EnumInfo(is)) != 0)
		printf("      '%s'\n", (const char *)is->str);

	printf("    ID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n",
		wvi->id.data1, wvi->id.data2, wvi->id.data3,
		wvi->id.data4[0], wvi->id.data4[1], wvi->id.data4[2], wvi->id.data4[3],
		wvi->id.data4[4], wvi->id.data4[5], wvi->id.data4[6], wvi->id.data4[7]);
	printf("    FMT t=%d ch=%d sr=%d bps=%d bits=%d algn=%d\n",
		wvi->wvfmt.fmtTag,
		wvi->wvfmt.channels,
		wvi->wvfmt.sampleRate,
		wvi->wvfmt.avgBytesPerSec,
		wvi->wvfmt.bitsPerSamp,
		wvi->wvfmt.blockAlign);

	printf("    SAMP atn=%d (%f) ft=%d (%f) opt=%d loop=%d key=%d\n",
		wvi->wvsmp.attenuation, pow(10.0, (double)wvi->wvsmp.attenuation / (200.0 * 65536.0)),
		wvi->wvsmp.fineTune, pow(2.0, (double)wvi->wvsmp.fineTune / 1200.0),
		wvi->wvsmp.options,	wvi->wvsmp.sampleLoops, wvi->wvsmp.unityNote);
	printf("    LOOP len=%d sz=%d st=%d ty=%d\n",
		wvi->loop.length, wvi->loop.size, wvi->loop.start, wvi->loop.type);
}

void Dump(DLSFileInfo *dls)
{
	printf("DLS FILE: \n");
	printf(" VER %d.%d.%d.%d\n",
		dls->vers.dwVersionMS >> 16,
		dls->vers.dwVersionMS & 0xffff,
		dls->vers.dwVersionLS >> 16,
		dls->vers.dwVersionLS & 0xffff);
	printf(" DLID %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n",
		dls->id.data1, dls->id.data2, dls->id.data3,
		dls->id.data4[0], dls->id.data4[1], dls->id.data4[2], dls->id.data4[3],
		dls->id.data4[4], dls->id.data4[5], dls->id.data4[6], dls->id.data4[7]);
	printf(" instruments = %d\n", dls->colh.instruments);
	printf(" pool cues: %d\n", dls->ptbl.cues);
	/*
	for (int c = 0; c < dls->ptbl.cues; c++)
		printf("    cue %d = %d\n", c, dls->cues[c]);
	*/
	DLSInfoStr *inf = 0;
	while ((inf = dls->inflist.EnumInfo(inf)) != 0)
		printf("%s\n", (const char *)inf->str);

	DLSInsInfo *in = 0;
	while ((in = dls->ins.EnumIns(in)) != 0)
		Dump(in);

	DLSWaveInfo *wv = 0;
	while ((wv = dls->wvpl.EnumWave(wv)) != 0)
		Dump(wv);
}


class SFEnvelope
{
private:
	AmpValue curLevel;
	AmpValue susLevel;
	AmpValue atkIncr;
	AmpValue decIncr;
	AmpValue relIncr;
	bsInt32  delayCount;
	bsInt32  holdCount;
	int      segNdx;
public:
	SFEnvelope()
	{
		curLevel = 0;
		susLevel = 0;
		atkIncr = 1.0;
		decIncr = 1.0;
		relIncr = 1.0;
		delayCount = 0;
		holdCount = 0;
		segNdx = 0;
	}

	// rate is in timecents = 1200 * log2(secs)
	void SetDelay(FrqValue tc)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * tc);
	}

	void SetAttack(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			atkIncr = 1.0 / count;
		else
			atkIncr = 1.0;
	}

	void SetHold(FrqValue tc)
	{
		holdCount = (bsInt32) (synthParams.sampleRate * tc);
	}

	void SetDecay(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			decIncr = 1.0 / count;
		else
			decIncr = 1.0;
	}

	void SetRelease(FrqValue tc)
	{
		FrqValue count = (synthParams.sampleRate * tc);
		if (count > 0)
			relIncr = 1.0 / count;
		else
			relIncr = 1.0;
	}

	// sustain: 0 <= lvl <= 1.0
	void SetSustain(AmpValue lvl)
	{
		susLevel = lvl;
	}

	void Start()
	{
		segNdx = 0;
	}

	int IsFinished()
	{
		return segNdx > 5;
	}

	void Release()
	{
		if (segNdx < 5)
			segNdx = 5;
	}

	AmpValue Gen()
	{
		switch (segNdx)
		{
		case 0: // delay
			if (delayCount > 1)
			{
				delayCount--;
				return 0.0;
			}
			segNdx++;
		case 1:
			curLevel += atkIncr;
			if (curLevel < 1.0)
				return curLevel * curLevel;
			segNdx++;
			curLevel = 1.0;
		case 2:
			if (holdCount > 1)
			{
				holdCount--;
				return 1.0;
			}
			segNdx++;
		case 3:
			curLevel -= decIncr;
			if (curLevel > susLevel)
				return curLevel;
			segNdx++;
			curLevel = susLevel;
		case 4:
			return susLevel;
		case 5:
			curLevel -= relIncr;
			if (curLevel > 0.0001)
				return curLevel;
			segNdx++;
			curLevel = 0;
		case 6:
			return 0.0;
		}
		return 0.0;
	}
};

float attenScale = 1.0;

// simplified form of GMPlayer to dump samples
class ZonePlayer
{
private:
	class ZonePlay : public SynthList<ZonePlay>
	{
	public:
		SBZone *zone;
		// Unit generators
		GenWaveSB  osc;
		GenWave32  vibLFO;
		GenWave32  modLFO;
		EnvGenSB   volEnv;
		EnvGenSB   modEnv;
		Panner     pan;
		FilterIIR2p filt;
		// summing nodes
		FrqValue phsPC;
		FrqValue fltPC;
		AmpValue volCB;
		bsInt16 filtOn;
		bsInt32 vibDelay;
		bsInt32 modDelay;
	};
	SynthEnumList<ZonePlay> zoneList;
	SBInstr *instr;
public:

	ZonePlayer()
	{
		instr = 0;
	}

	~ZonePlayer()
	{
		zoneList.Clear();
	}

	void Reset()
	{
		zoneList.Clear();
	}

	void Init(int pit, int vel, SBZone *zone)
	{
		ZonePlay *zp = zoneList.AddItem();
		zp->zone = zone;

		float smpl = 0;
		if (zone->rate != synthParams.isampleRate)
		{
			double wsrCents = 1200.0 * SoundBank::log2((double)zone->rate/440.0);
			double srCents = 1200.0 * SoundBank::log2((double)synthParams.sampleRate/440.0);
			smpl = (float)(wsrCents - srCents);
		}
		FrqValue adjKey = (float)pit + zone->coarseTune - zone->keyNum;
		FrqValue adjCents = zone->fineTune * 0.01;
		zp->phsPC = (zone->scaleTune * (adjKey + adjCents)) - zone->cents + smpl;
		zp->osc.InitSB(zone, SoundBank::GetPow2n1200(zp->phsPC));

		zp->volCB = (zone->initAtten * attenScale) + SoundBank::posConcave[127-vel] * zone->velScale;

		FrqValue km;
		if (zone->genFlags & SBGEN_SF2)
			km = FrqValue(60.0) - pit;
		else //if (zone->genFlags & SBGEN_DLS)
			km = (float)pit/128.0;

		zp->volEnv.SetDelay(SoundBank::EnvRate(zone->volEg.delay));
		zp->volEnv.SetAttack(SoundBank::EnvRate(zone->volEg.attack + ((float)vel/127.0) * zone->volEg.velAttack));
		zp->volEnv.SetHold(SoundBank::EnvRate(zone->volEg.hold + (km * zone->volEg.keyHold)));
		zp->volEnv.SetDecay(SoundBank::EnvRate(zone->volEg.decay + (km * zone->volEg.keyDecay)));
		if (zone->genFlags & SBGEN_SF2)
		{
			if (zone->volEg.sustain < 960.0f)
				zp->volEnv.SetSustain((960.0f - zone->volEg.sustain) / 960.0f);
			else
				zp->volEnv.SetSustain(0);
		}
		else
			zp->volEnv.SetSustain(zone->volEg.sustain * 0.001);
		zp->volEnv.SetRelease(SoundBank::EnvRate(zone->volEg.release));
		zp->volEnv.SetSegment(0);

		zp->modEnv.SetDelay(SoundBank::EnvRate(zone->modEg.delay));
		zp->modEnv.SetAttack(SoundBank::EnvRate(zone->modEg.attack + (((float)vel/127.0) * zone->modEg.velAttack)));
		zp->modEnv.SetHold(SoundBank::EnvRate(zone->modEg.hold + (km * zone->modEg.keyHold)));
		zp->modEnv.SetDecay(SoundBank::EnvRate(zone->modEg.decay + (km * zone->modEg.keyDecay)));
		if (zone->genFlags & SBGEN_SF2)
			zp->modEnv.SetSustain(1.0 - (zone->modEg.sustain * 0.001f));
		else
			zp->modEnv.SetSustain(zone->modEg.sustain * 0.001f);
		zp->modEnv.SetRelease(SoundBank::EnvRate(zone->modEg.release));
		zp->modEnv.SetSegment(0);

		zp->vibLFO.InitWT(SoundBank::Frequency(zone->vibLfo.rate), WT_SIN);
		zp->vibDelay = (bsInt32) (zone->vibLfo.delay * synthParams.sampleRate);
		zp->modLFO.InitWT(SoundBank::Frequency(zone->modLfo.rate), WT_SIN);
		zp->modDelay = (bsInt32) (zone->modLfo.delay * synthParams.sampleRate);

		zp->pan.Set(panSqr, zone->pan / 500.0);

		zp->fltPC = zone->filtFreq;
		if (zp->fltPC < 13500)
		{
			zp->filtOn = 1;
			zp->filt.CalcCoef(SoundBank::Frequency(zp->fltPC), SoundBank::Gain(zone->filtQ)/2.0);
			zp->volCB += zone->filtQ / 2.0;
		}
	}

	void Init(int pit, SBInstr *instr)
	{
		int vel = 100;
		SBZoneGroup *grp = 0;
		while ((grp = instr->EnumGroups(grp)) != 0)
		{
			if (grp->Match(pit, vel))
			{
				SBZoneRef *ref = grp->map[pit];
				while (ref)
				{
					SBZone *zone = ref->zone;
					if (zone->Match(pit, vel))
						Init(pit, vel, zone);
					ref = ref->next;
				}
			}
		}
	}

	void Stop()
	{
		ZonePlay *zp = 0;
		while ((zp = zoneList.EnumItem(zp)) != 0)
		{
			zp->volEnv.Release();
			zp->modEnv.Release();
			zp->osc.Release();
		}
	}

	int IsFinished()
	{
		ZonePlay *zp = 0;
		while ((zp = zoneList.EnumItem(zp)) != 0)
		{
			if (!(zp->volEnv.IsFinished() || zp->osc.IsFinished()))
				return 0;
		}
		return 1;
	}

	void Tick(WaveOutBuf *wf)
	{
		AmpValue outLft = 0;
		AmpValue outRgt = 0;
		ZonePlay *zp = 0;
		while ((zp = zoneList.EnumItem(zp)) != 0)
		{
			SBZone *zone = zp->zone;
			AmpValue volEG = zp->volEnv.Gen();
			AmpValue modEG = zp->modEnv.Gen();
			AmpValue vibLFO = 0;
			AmpValue modLFO = 0;

			if (zp->vibDelay == 0 || --zp->vibDelay == 0)
				vibLFO = zp->vibLFO.Gen();

			if (zp->modDelay == 0 || --zp->modDelay == 0)
				modLFO = zp->modLFO.Gen();

			FrqValue phsPC = zp->phsPC
			       + (vibLFO * zone->vibLfoFrq)
				   + (modLFO * zone->modLfoFrq)
				   + (modEG * zone->modEnvFrq);
			zp->osc.UpdatePhaseIncr(SoundBank::GetPow2n1200(phsPC));
			AmpValue out = zp->osc.Gen();

			if (zp->filtOn )
			{
				FrqValue fltPC = zp->fltPC + (modLFO * zone->modLfoFlt) + (modEG * zone->modEnvFlt);
				if (fltPC != zp->fltPC)
				{
					zp->fltPC = fltPC;
					zp->filt.CalcCoef(SoundBank::Frequency(fltPC),
						SoundBank::Gain(zone->filtQ)/2.0);
				}
				out = zp->filt.Sample(out);
			}

			if (zp->volEnv.GetSegment() > 2)
				volEG = SoundBank::Attenuation((1.0 - volEG) * 960.0);
			out *= SoundBank::Attenuation(zp->volCB) * volEG;
			outLft += out * zp->pan.panlft;
			outRgt += out * zp->pan.panrgt;
		}
		wf->Output2(outLft, outRgt);
	}
};

const char *wavePrefix = "";

void PlayPreset(SoundBank *bnk, int bnkNum, int preNum, int mono)
{
	SBInstr *instr = bnk->GetInstr(bnkNum, preNum);
	if (instr == 0)
	{
		fprintf(stderr, "Bank %d and preset %d does not exist\n", bnkNum, preNum);
		return;
	}

	fprintf(stderr, "Playing bank %d, preset %d : %s\n", bnkNum, preNum, (const char *)instr->instrName);

	WaveFile wf;
	char waveFile[1024];
	strcpy(waveFile, wavePrefix);
	strcat(waveFile, instr->instrName);
	strcat(waveFile, ".wav");
	char *sep = &waveFile[strlen(wavePrefix)];
	while (*sep)
	{
		if (*sep == '/' || *sep == '\\')
			*sep = '_';
		sep++;
	}

	wf.SetBufSize(30);
	if (wf.OpenWaveFile(waveFile, 2))
	{
		fprintf(stderr, "Cannot open file '%s'\n", (const char *)waveFile);
		return;
	}

	long silence = (long) (synthParams.sampleRate * 0.1);
	long dur = synthParams.isampleRate;
	long t;
	ZonePlayer play;

	// loop over all zones
	int pit;
	int vel;
	SBZone *zone = 0;
	while ((zone = instr->EnumZones(zone)) != 0)
	{
		fprintf(stderr, "  zone %s, %d [%d,%d] [%d,%d] ch=%d len=%d\n",
			(const char *)zone->name, zone->keyNum,
			zone->lowKey, zone->highKey, zone->lowVel, zone->highVel,
			zone->chan, zone->sample->sampleLen);
		pit = zone->keyNum + zone->coarseTune;
		if (pit < zone->lowKey || pit > zone->highKey)
			pit = (zone->highKey + zone->lowKey) / 2;
		vel = (zone->lowVel + zone->highVel) / 2;
		dur = zone->sample->sampleLen;
		play.Reset();
		play.Init(pit, vel, zone);
		for (t = 0; t < dur; t++)
			play.Tick(&wf);
		play.Stop();
		while (!play.IsFinished())
			play.Tick(&wf);
		for (t = 0; t < silence; t++)
			wf.Output1(0.0);
		if (wf.GetOOR() > 0)
		{
			printf("Out of range=%d\n", wf.GetOOR());
			wf.ClrOOR();
		}
	}
	if (bnkNum != 128)
	{
		dur = synthParams.isampleRate / 4;
		int pitches[] = { 36, 48, 60, 72, 84, -1 };
		for (pit = 0; pitches[pit] != -1; pit++)
		{
			play.Reset();
			play.Init(pitches[pit], instr);
			for (t = 0; t < dur; t++)
				play.Tick(&wf);
			play.Stop();
			while (!play.IsFinished())
				play.Tick(&wf);
		}
	}
	wf.CloseWaveFile();
}

void usage()
{
	fprintf(stderr, "use: example4a soundfont -bbank -ppreset -m -d(1|2|3) -wprefix -l[0|1] -n[0|1]\n");
	fprintf(stderr, "     -d1 = print internal representation\n");
	fprintf(stderr, "     -d2 = print raw file contents\n");
	fprintf(stderr, "     -d3 = print both\n");
	fprintf(stderr, "     -m  = force monophonic\n");
	fprintf(stderr, "     -w  = prefix to output file name\n");
	fprintf(stderr, "     -l0 = don't preload samples\n");
	fprintf(stderr, "     -l1 = preload all samples (default)\n");
	fprintf(stderr, "     -sn = scale attenuation by 'n'\n");
	fprintf(stderr, "     -bn = only output bank #n, else all banks\n");
	fprintf(stderr, "     -pn = only output patch #n, else all patches\n");
	exit(1);
}

int main(int argc, char *argv[])
{

	if (argc < 2)
		usage();

	int preload = 1;
	int doDump = 0;
	char *fileName = argv[1];
	char *waveFile = 0;
	int bnkNum = -1;
	int preNum = -1;
	int mono = 0;
	int isDLS = 0;
	int isSF2 = 0;

	int argn;
	for (argn = 2; argn < argc; argn++)
	{
		char *ap = argv[argn];
		if (*ap++ != '-')
			usage();
		switch (*ap++)
		{
		case 'b':
			bnkNum = atoi(ap);
			break;
		case 'p':
			preNum = atoi(ap);
			break;
		case 'd':
			if (*ap)
				doDump = atoi(ap);
			else
				doDump = 1;
			break;
		case 'w':
			wavePrefix = ap;
			break;
		case 'm':
			mono = 1;
			break;
		case 'l':
			if (*ap)
				preload = atoi(ap);
			else
				preload = 1;
			break;
		case 's':
			attenScale = atof(ap);
			break;
		default:
			fprintf(stderr, "Invalid argument: %s\n", argv[argn]);
			usage();
			break;
		}
	}
	InitSynthesizer();

	SoundBank *bnk = 0;
	isSF2 = SFFile::IsSF2File(fileName);
	if (!isSF2)
	{
		isDLS = DLSFile::IsDLSFile(fileName);
		if (!isDLS)
		{
			fprintf(stderr, "File format is not SF2 or DLS\n");
			exit(1);
		}
		DLSFile df;
		bnk = df.LoadSoundBank(fileName, preload);
		if (doDump & 2)
			Dump(df.GetInfo());
	}
	else
	{
		SFFile sounds;
		bnk = sounds.LoadSoundBank(fileName, preload);
		if (doDump & 2)
			Dump(&sounds);
	}
	if (!bnk)
	{
		fprintf(stderr, "No bank\n");
		exit(1);
	}
	bnk->Optimize();
	if (doDump)
	{
		if (doDump & 1)
			Dump(bnk);
		exit(0);
	}

	bnk->Lock();
	if (bnkNum == -1)
	{
		for (bnkNum = 0; bnkNum <= 128; bnkNum++)
		{
			if (!bnk->instrBank[bnkNum])
				continue;
			if (preNum == -1)
			{
				for (int pn = 0; pn < 128; pn++)
					PlayPreset(bnk, bnkNum, pn, mono);
			}
			else
				PlayPreset(bnk, bnkNum, preNum, mono);
		}
	}
	else
	{
		if (preNum == -1)
		{
			for (int pn = 0; pn < 128; pn++)
				PlayPreset(bnk, bnkNum, pn, mono);
		}
		else
			PlayPreset(bnk, bnkNum, preNum, mono);
	}
	bnk->Unlock();

	return 0;
}
