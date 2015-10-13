///////////////////////////////////////////////////////////
// BasicSynth - MIDI DLS File
//
/// @file DLSFile.h DLS file loader.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSoundbank
//@{
#ifndef _DLSFILE_H_
#define _DLSFILE_H_

#include <DLSDefs.h>

/// INFO string value.
class DLSInfoStr : public SynthList<DLSInfoStr>
{
public:
	bsUint32 id;
	bsString str;

	DLSInfoStr()
	{
		id = 0;
	}

	int Read(FileReadBuf& file, bsInt32 cksz);
};

/// List of INFO strings.
class DLSInfoList : public SynthEnumList<DLSInfoStr>
{
public:
	DLSInfoStr *EnumInfo(DLSInfoStr *is)
	{
		return EnumItem(is);
	}

	const char *GetInfo(bsUint32 id)
	{
		DLSInfoStr *str = 0;
		while ((str = EnumItem(str)) != 0)
		{
			if (str->id == id)
				return str->str;
		}
		return "";
	}

	int Read(FileReadBuf& file, bsUint32 cksz);
};

/// Articulation information data.
/// Articulation is specifed by source, destination, and scaling values.
class DLSArtInfo : public SynthList<DLSArtInfo>
{
public:
	bsUint32 connections;
	dlsConnection *info;
	int art2;

	DLSArtInfo()
	{
		art2 = 0;
		connections = 0;
		info = 0;
	}

	~DLSArtInfo()
	{
		delete info;
	}

	/// Get the KEY modifier value for the given destination.
	bsInt32 GetKeyMod(bsInt16 dst)
	{
		return GetConnection(CONN_SRC_KEYNUMBER, CONN_SRC_NONE, dst, 0);
	}

	/// Get the VELOCITY modifier for the given destination.
	bsInt32 GetVelMod(bsInt16 dst)
	{
		return GetConnection(CONN_SRC_KEYONVELOCITY, CONN_SRC_NONE, dst, 0);
	}

	/// Get the default (SOURCE NONE) for a destination.
	bsInt32 GetDefault(bsInt16 dst, bsInt32 defval)
	{
		return GetConnection(CONN_SRC_NONE, CONN_SRC_NONE, dst, defval);
	}

	/// Find a connection if it exists.
	/// If the connection does not exist, return the default value.
	bsInt32 GetConnection(bsInt16 src, bsInt16 ctl, bsInt16 dst, bsInt32 defval)
	{
		dlsConnection *ci = info;
		for (bsUint32 n = 0; n < connections; n++)
		{
			if (ci->source == src
			 && ci->control == ctl
			 && ci->destination == dst)
			{
				return ci->scale;
			}
			ci++;
		}
		return defval;
	}

	/// Read the articulation info into this object.
	int Read(FileReadBuf& file, bsUint32 cksz);
};

/// List of DLS articulation information.
class DLSArtList : public SynthEnumList<DLSArtInfo>
{
public:
	DLSArtInfo *EnumArt(DLSArtInfo *ai)
	{
		return EnumItem(ai);
	}

	int Read(FileReadBuf& file, bsUint32 cksz);
};

/// DLS region information.
/// A region defines a sample for a specific range of
/// key numbers and velocity values.
class DLSRgnInfo : public SynthList<DLSRgnInfo>
{
public:
	dlsRgnHeader rgnh;
	dlsSample    wsmp;
	dlsLoop      loop;
	dlsWaveLink  wlnk;
	DLSArtList   lart;
	int wsmpValid;
	int rgn2;

	DLSRgnInfo()
	{
		rgn2 = 0;
		wsmpValid = 0;
		memset(&rgnh, 0, sizeof(rgnh));
		memset(&wsmp, 0, sizeof(wsmp));
		memset(&wlnk, 0, sizeof(wlnk));
		memset(&loop, 0, sizeof(loop));
	}

	~DLSRgnInfo()
	{
	}

	/// Read the region information from the file.
	int Read(FileReadBuf& file, bsUint32 cksz);
};

/// List of DLS region information.
class DLSRgnList : public SynthEnumList<DLSRgnInfo>
{
public:
	DLSRgnInfo *EnumRgn(DLSRgnInfo *ri)
	{
		return EnumItem(ri);
	}

	/// Read the region list from the file.
	int Read(FileReadBuf& file, bsUint32 chksz);
};

/// DLS instrument information.
/// An instrument contains region and articulation info.
class DLSInsInfo : public SynthList<DLSInsInfo>
{
public:
	bsInt32 index;
	dlsID id;
	dlsInstHeader hdr;
	DLSRgnList rgnlist;
	DLSArtList lart;
	DLSInfoList info;

	DLSInsInfo()
	{
		index = -1;
		memset(&id, 0, sizeof(id));
		memset(&hdr, 0, sizeof(hdr));
	}

	~DLSInsInfo()
	{
	}

	bsInt16 GetBank()
	{
		// percussion bank?
		if (hdr.locale.bank & F_INSTRUMENT_DRUMS)
			return 128;
		short msb = hdr.locale.bank & 0x7f00;
		if (msb == 0x7900)
			return 128;
		if (msb != 0x7800)  // hmmm...
			return (msb >> 8) & 0x7F;
		return hdr.locale.bank & 0x7f;
	}

	bsInt16 GetProg()
	{
		return (bsInt16) (hdr.locale.instrument & 0x7f);
	}

	bsInt16 IsDrum()
	{
		if (hdr.locale.bank & 0x80000000)
			return 1;
		return 0;
	}

	/// Read the region information from the file.
	int Read(FileReadBuf& file, bsUint32 chksz);
};

/// List of DLS instruments.
class DLSInsList : public SynthEnumList<DLSInsInfo>
{
public:
	DLSInsInfo *EnumIns(DLSInsInfo *ii)
	{
		return EnumItem(ii);
	}

	/// Read the list of instruments from the file.
	int Read(FileReadBuf& file, bsUint32 chksz);
};

/// Wavetable from a DLS file.
/// Typically, each region uses a separate sample,
/// but samples can be shared.
class DLSWaveInfo : public SynthList<DLSWaveInfo>
{
public:
	bsInt32 index;
	dlsID id;
	dlsWaveFmt wvfmt;
	dlsSample  wvsmp;
	dlsLoop    loop;
	AmpValue  *samples;
	bsUint32  sampsize;
	bsUint32  filepos;
	DLSInfoList info;

	DLSWaveInfo()
	{
		memset(&id, 0, sizeof(id));
		memset(&wvfmt, 0, sizeof(wvfmt));
		samples = 0;
		sampsize = 0;
		filepos = 0;
	}

	~DLSWaveInfo()
	{
		// the sample data is owned by the sound bank object.
		//if (samples)
		//	delete samples;
	}

	int Read(FileReadBuf& file, bsUint32 chksz);
	int ReadSamples(FileReadBuf& file, bsUint32 chksz);
};

/// DLS wave pool information.
/// This is added for completeness, but is not
/// currently used. The wave pool data allows quick
/// locating of wavetables in cases where the samples
/// are not retained in memory. We store the offset
/// into the file in the DLSWaveInfo as the file is
/// scanned, thus making the wave pool redundant.
class DLSWvplInfo : public SynthEnumList<DLSWaveInfo>
{
public:
	bsUint32 filepos;
	DLSWvplInfo()
	{
		filepos = 0;
	}

	DLSWaveInfo *EnumWave(DLSWaveInfo *wi)
	{
		return EnumItem(wi);
	}

	DLSWaveInfo *FindWave(bsInt32 ndx)
	{
		DLSWaveInfo *wi = 0;
		while ((wi = EnumItem(wi)) != 0)
		{
			if (wi->index == ndx)
				return wi;
		}
		return 0;
	}

	int Read(FileReadBuf& file, bsUint32 chksz);
};

/// DLS file information.
/// This includes global INFO strings along
/// with lists of instruments.
class DLSFileInfo
{
public:
	dlsVersion vers;
	dlsID      id;
	dlsHeader  colh;
	dlsPoolTable ptbl;
	bsInt32    *cues;
	DLSInsList  ins;
	DLSWvplInfo wvpl;
	DLSInfoList inflist;

	DLSFileInfo()
	{
		memset(&vers, 0, sizeof(vers));
		memset(&id, 0, sizeof(id));
		memset(&colh, 0, sizeof(colh));
		memset(&ptbl, 0, sizeof(ptbl));
		cues = 0;
	}

	int Read(FileReadBuf& file, bsUint32 riffsize);

};

/// Downloadable Sounds (DLS) file.
/// This class loads a DLS1 or DLS2 file and builds a SoundBank
/// object from it.
class DLSFile
{
protected:
	FileReadBuf file;
	DLSFileInfo info;
	int preload;

	float DLSScale(bsInt32 scl);
	float MapScale(short destination, bsInt32 scale);

	short MapDestination(short destination);
	short MapSource(short source);
	void ApplyArt(SBZone *zone, DLSArtInfo *art);
	void AddModulator(SBModList *list, dlsConnection *ci);
	void InitModulator(SBModInfo *mi, dlsConnection *ci);
	SoundBank *BuildSoundBank(const char *fname);

public:
	DLSFile();
	~DLSFile();

	/// Determine if the file is a DLS format file.
	static int IsDLSFile(const char *fname);

	/// Load the file and return as a SoundBank object.
	/// The caller is responsible for deleteing the returned object.
	/// @param fname path to the file
	/// @param pre preload all samples
	/// @returns pointer to SoundBank object.
	SoundBank *LoadSoundBank(const char *fname, int pre = 1);

	/// Get the info records.
	/// @returns pointer to file info.
	DLSFileInfo *GetInfo() { return &info; }
};


#endif
//@}
