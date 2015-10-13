//////////////////////////////////////////////////////////////////////
// BasicSynth - wx specific code for project options
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"

void ProjectOptions::Load()
{
	long lval;
	wxString str;

#if UNIX
	wxString locname;
	locname = wxStandardPaths::Get().GetUserConfigDir();
	locname += "/.";
	locname += wxTheApp->GetAppName();
	wxString globname;
	globname = wxStandardPaths::Get().GetDataDir();
	globname += "/.";
	globname += wxTheApp->GetAppName();
	// if the user does not have a config file,
	// copy the global configuration
	if (!wxFile::Exists(locname) && wxFile::Exists(globname))
	{
		wxFile in(globname, wxFile::read);
		wxFile out(locname, wxFile::write);
		char buffer[1024];
		size_t count;
		while ((count = in.Read(buffer, 1024)) > 0)
			out.Write(buffer, count);
	}
#endif
	wxConfigBase *config = wxConfig::Get();
	if (config->Read("Install", &str))
		bsString::utf8(str.wc_str(), installDir, MAX_PATH);
	if (config->Read("HelpFile", &str))
		bsString::utf8(str.wc_str(), helpFile, MAX_PATH);
	if (config->Read("Forms", &str))
		bsString::utf8(str.wc_str(), formsDir, MAX_PATH);
	if (config->Read("Colors", &str))
		bsString::utf8(str.wc_str(), colorsFile, MAX_PATH);
	if (config->Read("InstrLib", &str))
		bsString::utf8(str.wc_str(), defLibDir, MAX_PATH);
	if (config->Read("Projects", &str))
		bsString::utf8(str.wc_str(), defPrjDir, MAX_PATH);
	if (config->Read("WaveIn", &str))
		bsString::utf8(str.wc_str(), defWaveIn, MAX_PATH);
	if (config->Read("InclNotelist", &lval))
		inclNotelist = (int)lval;
	if (config->Read("InclSequence", &lval))
		inclSequence = lval;
	if (config->Read("InclScripts", &lval))
		inclScripts = lval;
	if (config->Read("InclTextFiles", &lval))
		inclTextFiles = lval;
	if (config->Read("InclLibraries", &lval))
		inclLibraries = lval;
	if (config->Read("InclSoundFonts", &lval))
		inclSoundFonts = lval;
	if (config->Read("Latency", &str))
		playBuf = bsString::StrToFlp(str.c_str());
	if (config->Read("TickRes", &str))
		tickRes = bsString::StrToFlp(str.c_str());
	if (config->Read("InclInstruments", &str))
		inclInstr = bsString::StrToNum(str.c_str(), 16);
	if (config->Read("Author", &str))
		bsString::utf8(str.wc_str(), defAuthor, MAX_PATH);
	if (config->Read("Copyright", &str))
		bsString::utf8(str.wc_str(), defCopyright, MAX_PATH);
	if (config->Read("MIDIDeviceName", &str))
		bsString::utf8(str.wc_str(), midiDeviceName, MAX_PATH);
	if (config->Read("MIDIDevice", &lval))
		midiDevice = lval;
	if (config->Read("WaveDevice", &str))
		bsString::utf8(str.wc_str(), waveDevice, MAX_PATH);
	if (config->Read("Top", &lval))
		frmTop = lval;
	if (config->Read("Left", &lval))
		frmLeft = lval;
	if (config->Read("Width", &lval))
		frmWidth = lval;
	if (config->Read("Height", &lval))
		frmHeight = lval;
	if (config->Read("Maximize", &lval))
		frmMax = lval;
	if (config->Read("EditFontFace", &str))
		bsString::utf8(str.wc_str(), editFontFace, sizeof(editFontFace));
	if (config->Read("EditFontSize", &lval))
		editFontSize = (int)lval;
	if (config->Read("EditTabSize", &lval))
		editTabSize = (int)lval;
	if (config->Read("ToolBarSize", &lval))
		toolBarSize = (int)lval;

	if (installDir[0] == '\0')
	{
		str = ::wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
		strncpy(installDir, str, sizeof(installDir));
	}
	str = wxStandardPaths::Get().GetDataDir();
	if (formsDir[0] == '\0')
	{
		strncpy(formsDir, str, sizeof(formsDir)-8);
		strcat(formsDir, "/Forms");
	}

//	str = wxStandardPaths::Get().GetUserDataDir();
//	if (defLibDir[0] == '\0')
//		strncpy(defLibDir, str, MAX_PATH);
//	if (defPrjDir[0] == '\0')
//		strncpy(defPrjDir, str, MAX_PATH);
	if (colorsFile[0] == '\0')
		strncpy(colorsFile, "Colors.xml", MAX_PATH);
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
	SynthProject::NormalizePath(installDir);
	SynthProject::NormalizePath(defPrjDir);
	SynthProject::NormalizePath(defLibDir);
	SynthProject::NormalizePath(defWaveIn);
	SynthProject::NormalizePath(formsDir);
	SynthProject::NormalizePath(colorsFile);
	//SynthProject::NormalizePath(helpFile);

	wxConfigBase *config = wxConfig::Get();

	// explicitly convert from utf8
	wxMBConvUTF8 conv;

	config->Write("Projects", wxString(defPrjDir, conv));
	config->Write("Forms", wxString(formsDir, conv));
	config->Write("HelpFile", wxString(helpFile, conv));
	config->Write("Colors", wxString(colorsFile, conv));
	config->Write("WaveIn", wxString(defWaveIn, conv));
	config->Write("Author", wxString(defAuthor, conv));
	config->Write("Copyright", wxString(defCopyright, conv));
	config->Write("InclNotelist", inclNotelist);
	config->Write("InclSequence", inclSequence);
	config->Write("InclScripts", inclScripts);
	config->Write("InclTextFiles", inclTextFiles);
	config->Write("InclLibraries", inclLibraries);
	config->Write("InclSoundFonts", inclSoundFonts);
	char buf[40];
	bsString::NumToStr(inclInstr, buf, 40, 16);
	config->Write("InclInstruments", wxString(buf));
	config->Write("Latency", playBuf);
	config->Write("TickRes", tickRes);
	config->Write("MIDIDevice", midiDevice);
	config->Write("MIDIDeviceName", wxString(midiDeviceName, conv));
	config->Write("WaveDevice", wxString(waveDevice, conv));
	config->Write("Top", frmTop);
	config->Write("Left", frmLeft);
	config->Write("Width", frmWidth);
	config->Write("Height", frmHeight);
	config->Write("Maximize", frmMax);
	config->Write("EditFontFace", wxString(editFontFace, conv));
	config->Write("EditFontSize", editFontSize);
	config->Write("EditTabSize", editTabSize);
	config->Write("ToolBarSize", toolBarSize);
}

