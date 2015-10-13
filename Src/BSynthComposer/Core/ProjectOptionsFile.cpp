//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file ProjectOptionsFile.cpp Project options loader, file storage.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#if _WIN32
#include <windows.h>
#endif
#include "ComposerGlobal.h"
#include "ComposerCore.h"

static FILE *GetConfigFile(const char *mode, int local)
{
	bsString cfgFile;
#if _WIN32
	if (local)
	{
		char *hm = getenv("HOMEPATH"); // Windows name for HOME
		if (hm == 0)
            cfgFile = ".";
        else
            cfgFile = hm;
	}
	else
		cfgFile = "c:/Program Files/BasicSynth";
	cfgFile += "/basicsynth.ini";
#endif
#if UNIX
	if (local)
	{
		char *hm = getenv("HOME");
		if (hm == 0)
			cfgFile = ".";
        else
            cfgFile = hm;
	}
	else
	{
		cfgFile = "/usr/local/share/basicsynth";
	}
	cfgFile += "/.basicsynth";
#endif
	return fopen(cfgFile, mode);
}

void ProjectOptions::Load()
{
	strcpy(waveDevice, "default");
	docCount = 0;
	docList = new RecentDoc[docMax];
	FILE *fp = GetConfigFile("r", 1);
	if (fp == NULL)
		fp = GetConfigFile("r", 0);
	if (fp)
	{
		char lnbuf[1024];
		while (fgets(lnbuf, 1024, fp))
		{
			if (lnbuf[0] == '#')
				continue;
			char *eq = strchr(lnbuf, '\n');
			if (eq)
				*eq = 0;
			if (lnbuf[0] == 0)
				continue;
			if ((eq = strchr(lnbuf, '=')) != NULL)
			{
				*eq++ = 0;
				if (strcmp(lnbuf, "Install") == 0)
					strcpy(installDir, eq);
				else if (strcmp(lnbuf, "Forms") == 0)
					strcpy(formsDir, eq);
				else if (strcmp(lnbuf, "Colors") == 0)
					strcpy(colorsFile, eq);
				else if (strcmp(lnbuf, "HelpFile") == 0)
					strcpy(helpFile, eq);
				else if (strcmp(lnbuf, "Instrlib") == 0)
					strcpy(defLibDir, eq);
				else if (strcmp(lnbuf, "Projects") == 0)
					strcpy(defPrjDir, eq);
				else if (strcmp(lnbuf, "WaveIn") == 0)
					strcpy(defWaveIn, eq);
				else if (strcmp(lnbuf, "Author") == 0)
					strcpy(defAuthor, eq);
				else if (strcmp(lnbuf, "Copyright") == 0)
					strcpy(defCopyright, eq);
				else if (strcmp(lnbuf, "InclNotelist") == 0)
					inclNotelist = atoi(eq);
				else if (strcmp(lnbuf, "InclSequence") == 0)
					inclSequence = atoi(eq);
				else if (strcmp(lnbuf, "InclScripts") == 0)
					inclScripts = atoi(eq);
				else if (strcmp(lnbuf, "InclTextFiles") == 0)
					inclTextFiles = atoi(eq);
				else if (strcmp(lnbuf, "InclLibraries") == 0)
					inclLibraries = atoi(eq);
				else if (strcmp(lnbuf, "InclInstruments") == 0)
					inclInstr = xtoi(eq);
				else if (strcmp(lnbuf, "InclSoundFonts") == 0)
					inclSoundFonts = atoi(eq);
				else if (strcmp(lnbuf, "Latency") == 0)
					playBuf = atof(eq);
				else if (strcmp(lnbuf, "TickRes") == 0)
					tickRes = atof(eq);
				else if (strcmp(lnbuf, "MIDIDeviceName") == 0)
					strcpy(midiDeviceName, eq);
				else if (strcmp(lnbuf, "MIDIDevice") == 0)
					midiDevice = atoi(eq);
				else if (strcmp(lnbuf, "WaveDevice") == 0)
					strcpy(waveDevice, eq);
				else if (strcmp(lnbuf, "Top") == 0)
					frmTop = atoi(eq);
				else if (strcmp(lnbuf, "Left") == 0)
					frmLeft = atoi(eq);
				else if (strcmp(lnbuf, "Width") == 0)
					frmWidth = atoi(eq);
				else if (strcmp(lnbuf, "Height") == 0)
					frmHeight = atoi(eq);
				else if (strcmp(lnbuf, "Maximize") == 0)
					frmMax = atoi(eq);
				else if (strncmp(lnbuf, "file", 4) == 0 && isdigit(lnbuf[4]))
				{
					if (*eq && docCount < docMax)
						docList[docCount++].SetPath(eq);
				}
			}
		}
		fclose(fp);
	}
	if (installDir[0] == '\0')
	{
#if _WIN32
		strcpy(installDir, "c:/Program Files/BasicSynth");
#else
		strcpy(installDir, "/usr/local/share/basicsynth");
#endif
	}
	if (defLibDir[0] == '\0')
	{
		strcpy(defLibDir, installDir);
		strcat(defLibDir, "/libs");
	}
	if (defWaveIn[0] == '\0')
	{
		strcpy(defWaveIn, installDir);
		strcat(defWaveIn, "/wavefiles");
	}
	if (formsDir[0] == '\0')
	{
		strcpy(formsDir, installDir);
		strcat(formsDir, "/Forms");
	}
	if (helpFile[0] == '\0')
	{
		strcpy(helpFile, installDir);
		strcat(helpFile, "/Help/index.html");
	}
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
}

void ProjectOptions::Save()
{
	FILE *fp = GetConfigFile("w", 1);
	if (fp)
	{
		fprintf(fp, "Install=%s\n", installDir);
		fprintf(fp, "Forms=%s\n", formsDir);
		fprintf(fp, "Colors=%s\n", colorsFile);
		fprintf(fp, "HelpFile=%s\n", helpFile);
		fprintf(fp, "Instrlib=%s\n", defLibDir);
		fprintf(fp, "Projects=%s\n", defPrjDir);
		fprintf(fp, "WaveIn=%s\n", defWaveIn);
		fprintf(fp, "Author=%s\n", defAuthor);
		fprintf(fp, "Copyright=%s\n", defCopyright);
		fprintf(fp, "InclNotelist=%d\n", inclNotelist);
		fprintf(fp, "InclSequence=%d\n", inclSequence);
		fprintf(fp, "InclScripts=%d\n", inclScripts);
		fprintf(fp, "InclTextFiles=%d\n", inclTextFiles);
		fprintf(fp, "InclLibraries=%d\n", inclLibraries);
		fprintf(fp, "InclInstruments=%x\n", inclInstr);
		fprintf(fp, "InclSoundFonts=%d\n", inclSoundFonts);
		fprintf(fp, "Latency=%f\n", playBuf);
		fprintf(fp, "TickRes=%f\n", tickRes);
		fprintf(fp, "MIDIDevice=%d\n", midiDevice);
		fprintf(fp, "MIDIDeviceName=%s\n", midiDeviceName);
		fprintf(fp, "WaveDevice=%s\n", waveDevice);
		fprintf(fp, "Top=%d\n", frmTop);
		fprintf(fp, "Left=%d\n", frmLeft);
		fprintf(fp, "Width=%d\n", frmWidth);
		fprintf(fp, "Height=%d\n", frmHeight);
		fprintf(fp, "Maximize=%d\n", frmMax);
		for (int n = 0; n < docCount; n++)
			fprintf(fp, "file%d=%s\n", n+1, (const char *)docList[n].path);
		fclose(fp);
	}
}
