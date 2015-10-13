/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file SoundBank.cpp Sound bank (SF2 or DLS) implementation.
//
// Copyright 2009,2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthList.h>
#include <SoundBank.h>

SoundBank SoundBank::SoundBankList;

// N.B. - this unconditionally clears the list without checking for locks.
// The sound bank is still valid until the last lock is removed, but it
// will no longer be found with FindBank().
void SoundBank::DeleteBankList()
{
	SoundBank *bnk;
	while ((bnk = SoundBankList.next) != 0)
	{
		bnk->Remove();
		bnk->Unlock();
	}
}

/// Locate a bank by name.
SoundBank *SoundBank::FindBank(const char *name)
{
	SoundBank *bnk;
	for (bnk = SoundBankList.next; bnk; bnk = bnk->next)
	{
		if (bnk->name.Compare(name) == 0)
			break;
	}
	return bnk;
}

/// Locate a bank by file path.
SoundBank *SoundBank::FindBankFile(const char *file)
{
	SoundBank *bnk;
	for (bnk = SoundBankList.next; bnk; bnk = bnk->next)
	{
		if (bnk->file.Compare(file) == 0)
			break;
	}
	return bnk;
}

SoundBank *SoundBank::DefaultBank()
{
	// TODO: maybe have a static member for default bank?
	return SoundBankList.next;
}

FrqValue SoundBank::maxFilter;
FrqValue SoundBank::minFilter;

FrqValue *SoundBank::pow2Table;
AmpValue *SoundBank::pow10Table;

float SoundBank::posLinear[128] = { -1 };
float SoundBank::posConcave[128];
float SoundBank::posConvex[128];
float SoundBank::posSwitch[128];
float *SoundBank::normalize[4] =
{
	SoundBank::posLinear,
	SoundBank::posConcave,
	SoundBank::posConvex,
	SoundBank::posSwitch,
};

// calculate logarithm - base 2
#define lnTwo 0.693147180559945

double SoundBank::log2(double f)
{
	return log(f) / lnTwo; // log(f)/log(2)
}

// duration as tc = 1200 * log2(sec)
// tc ranges from -12000 (~.9ms) to the maximum rate.
// max rate varies, but is usually no more than 8000 (~100s).
// We use the general purpose 2^(n/1200) table for this,
// but limit the value to the output range [1ms,100s].
// 0x80000 (-32768) is magic and indicates a duration of 0.
FrqValue SoundBank::EnvRate(FrqValue tc)
{
	int ic = (int)tc;
	if (ic == -32768)
		return 0;
	if (ic <= -12000)
		return 0.00097;
	// This is stupid... 100 sec is too long
	if (ic >= 8000)
		return 101.59;
	return pow2Table[ic + 12000];
}

// Get frequency in Hz from pitch cents.
FrqValue SoundBank::Frequency(FrqValue pc)
{
	return 8.175 * GetPow2n1200(pc);
}

// Attenuation in centibels
// cb ranges from 0 (no attenuation) to 1440 (silence)
AmpValue SoundBank::Attenuation(AmpValue cb)
{
	int ic = (int)cb;
	if (ic <= 0)
		return 1.0;
	if (ic >= 1440)
		return 0.0;
	return pow10Table[ic];
}

// Return linear gain from centibels.
// Gain is pow(10, cb/200).
// The pow10Table is constructed as
//  pow(10, cb/-200)
// Gain can be represented as:
//    pow(10, 0-cb/-200)
// Which we can refactor as:
//    pow(10, 0/-200) / pow(10, cb/-200)
AmpValue SoundBank::Gain(AmpValue cb)
{
	int ic = (int)cb;
	if (ic <= 0)
		return 1.0;
	if (ic > 1440)
		ic = 1440;
	return 1.0 / pow10Table[ic];
}

// Lookup 2^(n/1200)
FrqValue SoundBank::GetPow2n1200(FrqValue fc)
{
	int ic = (int)fc;
	if (ic >= -12000 && ic < 25501 && pow2Table)
		return pow2Table[ic + 12000];
	return (FrqValue)pow(2.0, (double)fc/1200.0);
}

// Create the powers of 2 table.
// This function is a static class function
// called automatically when the first SoundBank is
// instantiated. Call this directly if you
// want to use the pow2Table without loading
// a SoundBank. Yes, "It's big" - but fast.
// This can be done with several smaller tables
// but what else ya gonna do with 4-8GB of ram?
void SoundBank::InitPow2n1200()
{
	if (pow2Table == 0)
	{
		pow2Table = new FrqValue[25501];
		double x = -12000.0;
		for (int index = 0; index <= 25500; index++)
		{
			pow2Table[index] = (FrqValue) pow(2.0, x / 1200.0);
			x += 1.0;
		}
	}
}


// Lookup 10^(n/-200)
AmpValue SoundBank::GetPow10n200(AmpValue cb)
{
	int ic = (int)cb;
	if (ic >= 0 && ic < 1440 && pow10Table)
		return pow10Table[ic];
	return pow(10.0, (double)cb/-200.0);
}

// Create the powers of 10 table.
// This is used to convert centibels
// to linear, normalized amplitude.
// cb = -200 * log10(amp)
// amp = pow(10, cb/-200)
void SoundBank::InitPow10n200()
{
	if (pow10Table == 0)
	{
		pow10Table = new AmpValue[1441];
		double x = 0.0;
		for (int index = 0; index <= 1440; index++)
		{
			pow10Table[index] = (AmpValue) pow(10.0, (double)x / -200.0);
			x += 1.0;
		}
	}
}

// The four transform tables are used to normalize MIDI inputs.
// Negatvie transforms are simply the appropriate positive
// transform reversed, i.e.:
//   negLinear[n] = posLinear[127-n];
//   negConvex[n] = posConcave[127-n];
//   negConcave[n] = posConvex[127-n];
//   negSwitch[n] = posSwitch[127-n];
//
// For Reference:
// r = (double)n / 127.0f;
// rn = 1.0 - r;
// posLinear[n] = r;
// negLinear[n] = rn;
// posConcave[n] = -40.0f * log10(rn) / 96.0;
// negConcave[n] = -40.0f * log10(r) / 96.0;
// posConvex[n] = 1.0 + (40.0f * log10(r) / 96.0);
// negConvex[n] = 1.0 + (40.0f * log10(rn) / 96.0);
///////////////////////////////////////////////////////
void SoundBank::InitTransform()
{
	if (posLinear[0] != -1)
		return;

	posLinear[0] = 0.0f;
	posLinear[127] = 1.0f;
	posConvex[0] = 0.0f;
	posConvex[127] = 1.0f;
	posConcave[0] = 0.0f;
	posConcave[127] = 1.0f;
	posSwitch[0] = 0.0f;
	posSwitch[127] = 1.0;

	for (int n = 1; n < 127; n++)
	{
		float r = (float) n / 127.0;
		float v = -40.0f/96.0f * log10(r);
		posLinear[n] = r;
		posConcave[127-n] = v;
		posConvex[n] = 1.0 - v;
		posSwitch[n] = r >= 0.5;
	}
}

float SoundBank::Transform(short val, short tt)
{
	if (tt & SBGEN_NEGFLG)
		val = 127 - val;
	if (tt & SBGEN_POLFLG)
	{
		tt &= 3;
		if (tt == 3) // switch
			return val >= 64 ? 1.0f : -1.0f;
		if (tt == 0) // linear
			return (posLinear[val] * 2.0f) - 1.0f;
		if (val < 64)
		{
			tt ^= 3; // 2 -> 1, 1 -> 2
			val *= 2;
			return normalize[tt][val] - 1.0f;
		}
		val = (val - 64) * 2;
		return normalize[tt][val];
	}
	return normalize[tt&3][val];
}

int SoundBank::OpenSampleFile()
{
	if (sampleFileOpen)
		return 0;
	// big enough to hold a typical sample,
	// not too big to avoid reading things we don't need.
	sampleFile.SetBufSize(8192);
	if (sampleFile.FileOpen(file) != 0)
		return -1;
	sampleFileOpen = 1;
	return 0;
}

/// Load samples. These will always create a valid
/// pointer to sample values, but the values will
/// be zero on failure.
int SoundBank::LoadInstr(SBInstr *in)
{
	if (in->loaded)
		return 0;

	if (OpenSampleFile())
		return -1;
	return LoadInstr(in, sampleFile);
}

int SoundBank::LoadInstr(SBInstr *in, FileReadBuf& f)
{
	if (in->loaded)
		return 0;

	int err = 0;

	SBZone *zone = 0;
	while ((zone = in->EnumZones(zone)) != 0)
	{
		SBSample *samp = zone->sample;
		if (samp == 0)
		{
			samp = samples;
			while (samp)
			{
				if (samp->index == zone->sampleNdx)
				{
					zone->sample = samp;
					zone->tableEnd = samp->sampleLen;
					break;
				}
				samp = samp->next;
			}
		}
		if (samp)
		{
			if (samp->sample == 0)
			{
				err |= LoadSample(samp, f);
			}
			/*if (zone->mode && zone->peak == 0.0 && samp->sample)
			{
				bsInt32 sampnum = zone->loopStart;
				AmpValue *sp = samp->sample + zone->loopStart;
				while (++sampnum <= zone->loopEnd)
				{
					AmpValue v = fabs(*sp++);
					if (v > zone->peak)
						zone->peak = v;
				}
			}*/
		}
	}
	in->loaded = 1;
	return err;
}

static void ZeroSample(AmpValue *sp, bsInt32 len)
{
	while (--len >= 0)
		*sp++ = 0.0;
}

int SoundBank::LoadSample(SBSample *samp)
{
	if (samp->sample)
		return 0;

	if (OpenSampleFile())
	{
		bsUint32 samplen = samp->sampleLen+2;
		samp->sample = new AmpValue[samplen];
		ZeroSample(samp->sample, samplen);
		return -1;
	}
	return LoadSample(samp, sampleFile);
}

// We can have one block of either 1 or 2 channel,
// or one block for each channel.
// Two zeros are added at the end as guard points.
int SoundBank::LoadSample(SBSample *samp, FileReadBuf& f)
{
	if (samp->sample)
		return 0;

	samp->sample = new AmpValue[samp->sampleLen + 2];

	int err = 0;
	if (samp->channels == 1)
		err = ReadSamples1(samp, f);
	else
	{
		samp->linked = new AmpValue[samp->sampleLen + 2];
		err = ReadSamples2(samp, f);
	}

	return err;
}

#if SYNTH_BIG_ENDIAN
static float SwapFloat(float n)
{
	bsUint32 *bytes = (bsUint32 *) &n;
	bsUint32 value = *bytes;
	*bytes = ((value << 24)& 0xff000000)
		   | ((value << 8) & 0x00ff0000)
		   | ((value >> 8) & 0x0000ff00)
		   | ((value >> 24)& 0x000000ff);
	return n;
}
#else
#define SwapFloat(n) n
#endif

int SoundBank::ReadSamples1(SBSample *samp, FileReadBuf& f)
{
	bsInt32 cnt;
	bsInt32 samplen = samp->sampleLen;
	AmpValue *sp = samp->sample;

	if (samp->filepos == 0)
	{
		ZeroSample(sp, samplen+2);
		return -1;
	}
	f.FileRewind(samp->filepos);
	if (samp->format == 0) // 8-bit
	{
		for (cnt = 0; cnt < samplen; cnt++)
			*sp++ = (AmpValue) (f.ReadCh() - 128) / 128.0;
	}
	else if (samp->format == 1 || samp->format == 3) // 16-bit
	{
		for (cnt = 0; cnt < samplen; cnt++)
		{
			short val = f.ReadCh() | (f.ReadCh() << 8);
			*sp++ = (AmpValue) val / 32768.0;
		}
		if (samp->format == 3 && samp->filepos2 != 0) // 24-bit SF2
		{
			f.FileRewind(samp->filepos2);
			sp = samp->sample;
			for (cnt = 0; cnt < samplen; cnt++)
				*sp++ += ((AmpValue) f.ReadCh() / 8388608.0);
		}
	}
	else if (samp->format == 2) // IEEE float (rare)
	{
		float val;
		for (cnt = 0; cnt < samplen; cnt++)
		{
			f.FileRead(&val, 4);
			*sp++ = SwapFloat(val);
		}
	}
	*sp++ = 0;
	*sp = 0;
	return 0;
}

// Two-channel samples. Allowed by DLS2, but not
// really useful since we need a mono sample for the
// oscillator phase to work correctly. We load these
// but only the left channel is currently used. It is
// possible to construct an oscillator to use both left
// and right channels.
int SoundBank::ReadSamples2(SBSample *samp, FileReadBuf& f)
{
	int skip = samp->channels > 2;
	bsInt32 chnl;
	bsInt32 cnt;
	bsInt32 samplen = samp->sampleLen;
	AmpValue *sp1 = samp->sample;
	AmpValue *sp2 = samp->linked;

	if (samp->filepos == 0)
	{
		ZeroSample(sp1, samplen+2);
		ZeroSample(sp2, samplen+2);
		return -1;
	}

	f.FileRewind(samp->filepos);
	if (samp->format == 0)
	{
		for (cnt = 0; cnt < samplen; cnt++)
		{
			*sp1++ = (AmpValue) (f.ReadCh() - 128) / 128.0;
			*sp2++ = (AmpValue) (f.ReadCh() - 128) / 128.0;
			if (skip)
			{
				for (chnl = samp->channels - 2; chnl > 0; chnl--)
					f.ReadCh();
			}
		}
	}
	else if (samp->format == 1)
	{
		short val;
		for (cnt = 0; cnt < samplen; cnt++)
		{
			val = (short) (f.ReadCh() | (f.ReadCh() << 8));
			*sp1++ = (AmpValue) val / 32768.0;
			val = (short) (f.ReadCh() | (f.ReadCh() << 8));
			*sp2++ = (AmpValue) val / 32768.0;
			if (skip)
			{
				for (chnl = samp->channels - 2; chnl > 0; chnl--)
					f.FileRead(&val, 2);
			}
		}
	}
	else if (samp->format == 2)
	{
		float val;
		for (cnt = 0; cnt < samplen; cnt++)
		{
			f.FileRead(&val, 4);
			*sp1++ = SwapFloat(val);
			f.FileRead(&val, 4);
			*sp2++ = SwapFloat(val);
			if (skip)
			{
				for (chnl = samp->channels - 2; chnl > 0; chnl--)
					f.FileRead(&val, 4);
			}
		}
	}
	*sp1++ = 0;
	*sp1 = 0;
	*sp2++ = 0;
	*sp2 = 0;
	return 0;
}

