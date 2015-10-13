//////////////////////////////////////////////////////////////////////
// BasicSynth - Windows specific code for configuration options.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <Mmsystem.h>
#include <Mmreg.h>
#include <ShlObj.h>

class RegKey
{
public:
	HKEY hk;
	wchar_t ws[MAX_PATH];
	wchar_t ks[MAX_PATH];
	RegKey()
	{
		hk = 0;
	}

	operator HKEY(){ return hk; }

	int Open(HKEY parent, const char *key)
	{
		return ::RegOpenKeyEx(parent, key, 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS;
	}

	int Create(HKEY parent, const char *key)
	{
		DWORD dw = 0;
		return ::RegCreateKeyExA(parent, key, 0, 0, 0, KEY_ALL_ACCESS, 0, &hk, &dw) == ERROR_SUCCESS;
	}

	int QueryValue(const char *key, char *value, DWORD len = MAX_PATH)
	{
		DWORD blen = MAX_PATH*sizeof(wchar_t);
		DWORD type = 0;
		bsString::utf16(key, ks, MAX_PATH);
		if (::RegQueryValueExW(hk, ks, NULL, &type, (LPBYTE)ws, &blen) == ERROR_SUCCESS)
		{
			bsString::utf8(ws, value, len);
			return 1;
		}
		return 0;
	}

	int QueryValue(const char *key, int& value)
	{
		DWORD dwval = 0;
		DWORD type;
		DWORD len = 4;
		if (::RegQueryValueEx(hk, key, NULL, &type, (LPBYTE)&dwval, &len) == ERROR_SUCCESS
		 && type == REG_DWORD)
		{
			value = (int) dwval;
			return 1;
		}
		return 0;
	}

	int QueryValue(const char *key, float& value)
	{
		char buf[40];
		DWORD type;
		DWORD len = 40;
		if (::RegQueryValueEx(hk, key, NULL, &type, (LPBYTE)buf, &len) == ERROR_SUCCESS)
		{
			value = (float) bsString::StrToFlp(buf);
			return 1;
		}
		return 0;
	}

	int SetValue(const char *key, const char *value)
	{
		bsString::utf16(key, ks, MAX_PATH);
		size_t wlen = bsString::utf16(value, ws, MAX_PATH);
		return ::RegSetValueExW(hk, ks, NULL, REG_SZ, (LPBYTE)ws, (DWORD)wlen*sizeof(wchar_t)) == ERROR_SUCCESS;
	}

	int SetValue(const char *key, int value)
	{
		DWORD dval = (DWORD)value;
		return ::RegSetValueEx(hk, key, NULL, REG_DWORD, (LPBYTE)&dval, 4) == ERROR_SUCCESS;
	}

	int SetValue(const char *key, float value)
	{
		bsString fval((double)value);
		return ::RegSetValueEx(hk, key, NULL, REG_SZ, (LPBYTE)(const char*)fval, (DWORD)fval.Length()) == ERROR_SUCCESS;
	}

	void Close()
	{
		::RegCloseKey(hk);
		hk = 0;
	}

	int EnumValue(int n, char *value, DWORD len = MAX_PATH)
	{
		char valueName[MAX_PATH];
		DWORD type;
		DWORD vlen;
		return ::RegEnumValue(hk, (DWORD)n, valueName, &vlen, 0, &type, (LPBYTE)value, &len) == ERROR_SUCCESS;
	}
};

void ProjectOptions::Load()
{
	bsString bsKey;
	bsKey = "Software\\";
	bsKey += programName;

	RegKey rk;
	ULONG len = MAX_PATH;
	char buf[MAX_PATH];

	char curdir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, curdir);

	char module[MAX_PATH];
	GetModuleFileName(NULL, module, MAX_PATH);
	SynthProject::NormalizePath(module);
	char *base = strrchr(module, '/');
	if (base)
		*base = 0;

	len = MAX_PATH;
	GetUserName(defAuthor, &len);
	SYSTEMTIME systm;
	GetSystemTime(&systm);
	snprintf(defCopyright, MAX_PATH, "Copyright %d", systm.wYear);

	if (rk.Open(HKEY_LOCAL_MACHINE, bsKey))
	{
		rk.QueryValue("Install", installDir);
		rk.QueryValue("HelpFile", helpFile);
		rk.QueryValue("Forms", formsDir);
		rk.QueryValue("Colors", colorsFile);
		rk.QueryValue("Instrlib", defLibDir);
		rk.Close();
	}

	if (rk.Open(HKEY_CURRENT_USER, bsKey))
	{
		rk.QueryValue("Projects", defPrjDir);
		rk.QueryValue("Forms", formsDir);
		rk.QueryValue("Colors", colorsFile);
		rk.QueryValue("WaveIn", defWaveIn);
		rk.QueryValue("InclNotelist", inclNotelist);
		rk.QueryValue("InclSequence", inclSequence);
		rk.QueryValue("InclScripts", inclScripts);
		rk.QueryValue("InclTextFiles", inclTextFiles);
		rk.QueryValue("InclLibraries", inclLibraries);
		rk.QueryValue("InclSoundFonts", inclSoundFonts);
		rk.QueryValue("Latency", playBuf);
		rk.QueryValue("TickRes", tickRes);
		if (rk.QueryValue("InclInstruments", buf))
			inclInstr = bsString::StrToNum(buf, 16);
		rk.QueryValue("Author", defAuthor);
		rk.QueryValue("Copyright", defCopyright);
		rk.QueryValue("MIDIDeviceName", midiDeviceName);
		rk.QueryValue("MIDIDevice", midiDevice);
		rk.QueryValue("WaveDevice", waveDevice);
		rk.QueryValue("Top", frmTop);
		rk.QueryValue("Left", frmLeft);
		rk.QueryValue("Width", frmWidth);
		rk.QueryValue("Height", frmHeight);
		rk.QueryValue("Maximize", frmMax);
		if (!rk.QueryValue("EditFontFace", editFontFace))
			strncpy(editFontFace, "Courier New", sizeof(editFontFace));
		rk.QueryValue("EditFontSize", editFontSize);
		rk.QueryValue("EditTabSize", editTabSize);
		rk.QueryValue("ToolBarSize", toolBarSize);
#ifndef __ATLMISC_H__
		RegKey rdlKey;
		if (rdlKey.Open(rk, "Recent Document List"))
		{
			char docName[MAX_PATH+1];
			while (rdlKey.EnumValue(docCount++, docName))
			{
				docName[MAX_PATH] = '\0';
				if (strcmp(docName, "DocumentCount") != 0)
				{
					RecentDoc *doc = docList.AddItem();
					doc->SetPath(docName);
				}
				if (docCount >= docMax)
					break;
			}
			rdlKey.Close();
		}
#endif
		rk.Close();
	}
#ifdef SHGFP_TYPE_CURRENT
	else
	{
		HRESULT hr;
		hr = SHGetFolderPathAndSubDir(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, programName, defPrjDir);
		if (hr != S_OK)
			hr = SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, programName, defPrjDir);
	}
#endif
	if (defPrjDir[0] == 0)
	{
		strncpy(defPrjDir, curdir, MAX_PATH);
	}

	if (installDir[0] == 0)
	{
		strncpy(installDir, module, MAX_PATH);
	}
	if (formsDir[0] == 0)
	{
		strncpy(formsDir, installDir, MAX_PATH-7);
		strcat(formsDir, "\\Forms");
	}
	if (colorsFile[0] == 0)
	{
		strncpy(colorsFile, "Colors.xml", MAX_PATH);
	}
	if (helpFile[0] == 0)
	{
		strncpy(helpFile, installDir, MAX_PATH-16);
		strcat(helpFile, "\\BSynthHelp.chm");
	}
	if (defLibDir[0] == 0)
	{
		strncpy(defLibDir, installDir, MAX_PATH-10);
		strcat(defLibDir, "\\Instrlib");
	}
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
	SynthProject::NormalizePath(colorsFile);
	SynthProject::NormalizePath(helpFile);
}

void ProjectOptions::Save()
{
	bsString bsKey;
	bsKey = "Software\\";
	bsKey += programName;
	RegKey rk;
	if (!rk.Open(HKEY_CURRENT_USER, bsKey))
		rk.Create(HKEY_CURRENT_USER, bsKey);
	rk.SetValue("Projects", defPrjDir);
	rk.SetValue("Forms", formsDir);
	rk.SetValue("Colors", colorsFile);
	rk.SetValue("WaveIn", defWaveIn);
	rk.SetValue("Author", defAuthor);
	rk.SetValue("Copyright", defCopyright);
	rk.SetValue("InclNotelist", inclNotelist);
	rk.SetValue("InclSequence", inclSequence);
	rk.SetValue("InclScripts", inclScripts);
	rk.SetValue("InclTextFiles", inclTextFiles);
	rk.SetValue("InclLibraries", inclLibraries);
	rk.SetValue("InclSoundFonts", inclSoundFonts);
	char buf[40];
	snprintf(buf, 40, "%x", inclInstr);
	rk.SetValue("InclInstruments", buf);
	rk.SetValue("Latency", playBuf);
	rk.SetValue("TickRes", tickRes);
	rk.SetValue("MIDIDevice", midiDevice);
	rk.SetValue("MIDIDeviceName", midiDeviceName);
	rk.SetValue("WaveDevice", waveDevice);
	rk.SetValue("Top", frmTop);
	rk.SetValue("Left", frmLeft);
	rk.SetValue("Width", frmWidth);
	rk.SetValue("Height", frmHeight);
	rk.SetValue("Maximize", frmMax);
	rk.SetValue("EditFontFace", editFontFace);
	rk.SetValue("EditFontSize", editFontSize);
	rk.SetValue("EditTabSize", editTabSize);
	rk.SetValue("ToolBarSize", toolBarSize);
#ifndef __ATLMISC_H__
	RegKey rdlKey;
	if (!rdlKey.Open(rk, "Recent Document List"))
		rdlKey.Create(rk,"Recent Document List");
	RecentDoc *doc = 0;
	docCount = 0;
	while ((doc = docList.EnumItem(doc)) != 0)
	{
		char docName[30];
		snprintf(docName, 30, "Document%d", ++docCount);
		rdlKey.SetValue(docName, doc->path);
	}
	rdlKey.SetValue("DocumentCount", docCount);
	rdlKey.Close();
#endif
	rk.Close();
}

