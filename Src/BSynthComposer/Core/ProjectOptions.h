#ifndef PROJECTOPTIONS_H
#define PROJECTOPTIONS_H

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

class RecentDoc
{
public:
	bsString path; ///< actual file path
	bsString name; ///< Display name

	void SetPath(const char *str)
	{
		path = str;
		char *sl = strrchr((char*)str, '/');
		if (sl == NULL)
			sl = strrchr((char*)str, '\\');
		if (sl)
			name = sl+1;
		else
			name = str;
	}
};

class SoundDevInfo : public SynthList<SoundDevInfo>
{
public:
	bsString name;   ///< display name
	void *info;      ///< device specifc information
	long id;         ///< device id
	long sub;        ///< sub-device id
	int type;        ///< type of device: 0=PCM, 1=MIDI
};


class ProjectOptions
{
public:
	char programName[MAX_PATH];
	char installDir[MAX_PATH];
	char formsDir[MAX_PATH];
	char helpFile[MAX_PATH];
	char colorsFile[MAX_PATH];
	char formsFont[MAX_PATH];
	char defAuthor[MAX_PATH];
	char defCopyright[MAX_PATH];
	char defPrjDir[MAX_PATH];
	char defLibDir[MAX_PATH];
	char defWaveIn[MAX_PATH];
	char defWaveOut[MAX_PATH];
	int inclNotelist;
	int inclSequence;
	int inclScripts;
	int inclTextFiles;
	int inclLibraries;
	int inclSoundFonts;
	int inclInstr;
	int inclMIDI;
	int midiDevice;
	char midiDeviceName[MAX_PATH];
	char waveDevice[MAX_PATH];
	float playBuf;
	float tickRes;
	int frmTop;
	int frmLeft;
	int frmWidth;
	int frmHeight;
	int frmMax;
	int editTextColor;
	int editFontSize;
	int editTabSize;
	char editFontFace[100];
	int toolBarSize;
#if defined(_WIN32) && _WIN32
	HWND dsoundHWND;
#endif
	RecentDoc *docList;
	int docMax;
	int docCount;
	SynthEnumList<SoundDevInfo> waveList;
	SynthEnumList<SoundDevInfo> midiList;

	ProjectOptions();
	~ProjectOptions();

	void InitDevInfo();
	void Load();
	void Save();
	int xtoi(const char *p);

};

extern ProjectOptions prjOptions;

#endif
