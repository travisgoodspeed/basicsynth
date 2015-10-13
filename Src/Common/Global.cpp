//////////////////////////////////////////////////////////////////
/// @file Global.cpp Global functions and objects.
//
// BasicSynth -- Global objects and functions for BasicSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveTable.h>
#include <SynthFile.h>

SynthConfig synthParams;
WaveTableSet wtSet;

int InitSynthesizer(bsInt32 sr, bsInt32 wtlen, bsInt32 wtusr)
{
	synthParams.Init(sr, wtlen);
	wtSet.Init(wtusr);
	for (int wtNdx = WT_USR(0); wtNdx < WT_USR(wtusr); wtNdx++)
		wtSet.wavSet[wtNdx].wavID = wtNdx;
	return 0;
}

int SynthConfig::FindOnPath(bsString& fullPath, const char *fname)
{
	if (fname == 0 || *fname == '\0')
		return 0;

	// see if OS can resolve this as-is
	if (SynthFileExists(fname))
	{
		fullPath = fname;
		return 1;
	}

	// skip windows drive letter
	if (fname[1] == ':')
		fname += 2;

	// if full path - we didn't find it
	if (*fname == '/' || *fname == '\\')
		return 0;

	// relative . path
	if (*fname == '.')
		return 0;

	int start = 0;
	int srchlen = (int) wvPath.Length();
	while (start < srchlen)
	{
		int semi = wvPath.Find(start, ';');
		if (semi <= 0)
			semi = srchlen;
		wvPath.SubString(fullPath, start, (size_t) semi - start);
		fullPath += "/";
		fullPath += fname;
		if (SynthFileExists(fullPath))
			return 1;
		start = semi+1;
	}
	fullPath = fname;
	return 0;
}

int SynthCreateFile(const char *fname, void *data, size_t datalen)
{
	FileWriteUnBuf fh;
	if (fh.FileOpen(fname) == 0)
	{
		if (data && datalen > 0)
			fh.FileWrite(data, datalen);
		fh.FileClose();
		return 0;
	}
	return -1;
}
