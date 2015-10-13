//////////////////////////////////////////////////////////////////////
// BasicSynth - Project options initialization (generic)
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

ProjectOptions::ProjectOptions()
{
	strcpy(programName, "BasicSynth");
	memset(installDir, 0, MAX_PATH);
	memset(formsDir, 0, MAX_PATH);
	strcpy(colorsFile, "Colors.xml");
	strcpy(formsFont, "Helvetica");
	memset(helpFile, 0, MAX_PATH);
	memset(defAuthor, 0, MAX_PATH);
	memset(defCopyright, 0, MAX_PATH);
	memset(defPrjDir, 0, MAX_PATH);
	memset(defLibDir, 0, MAX_PATH);
	memset(defWaveIn, 0, MAX_PATH);
	memset(defWaveOut, 0, MAX_PATH);
	inclNotelist = 1;
	inclSequence = 0;
	inclScripts = 0;
	inclTextFiles = 0;
	inclLibraries = 0;
	inclSoundFonts = 1;
	inclMIDI = 1;
	inclInstr = -1;
	midiDevice = -1;
	memset(midiDeviceName, 0, MAX_PATH);
	memset(waveDevice, 0, MAX_PATH);
	playBuf = 0.1;
	tickRes = 0.0005;
	frmLeft = 0;
	frmTop = 0;
	frmWidth = 1024;
	frmHeight = 768;
	frmMax = 0;
	editTextColor = 0; //RGB(0,0,0);
	editFontSize = 12;
	editTabSize = 4;
	strncpy(editFontFace, "Courier New", sizeof(editFontFace));
	toolBarSize = 32;
	docList = 0;
	docCount = 0;
	docMax = 6;
#if defined(_WIN32) && _WIN32
	dsoundHWND = 0;
#endif
	InitDevInfo();
}

ProjectOptions::~ProjectOptions()
{
	if (docList)
		delete[] docList;
}

int ProjectOptions::xtoi(const char *p)
{
	int h;
	int x = 0;
	while (*p)
	{
		if (*p >= 0 && *p <= '9')
			h = *p - '0';
		else if (*p >= 'A' && *p <= 'F')
			h = (*p - 'A') + 10;
		else if (*p >= 'a' && *p <= 'f')
			h = (*p - 'a') + 10;
		else
			break;
		x = (x << 4) | h;
		p++;
	}
	return x;
}


#if defined(_WIN32) && _WIN32
#include <Mmsystem.h>
#include <Mmreg.h>

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT (WINAPI *tDirectSoundEnumerate)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

static BOOL CALLBACK EnumWaveDevice(LPGUID lpGUID,LPCTSTR lpszDesc,LPCTSTR lpszDrvName, LPVOID lpContext)
{
	SoundDevInfo *inf = prjOptions.waveList.AddItem();
	inf->name = lpszDesc;
	if (lpGUID)
	{
		inf->info = new GUID;
		memcpy(inf->info, lpGUID, sizeof(GUID));
	}
	else
		inf->info = NULL;
	long *cnt = (long*)lpContext;
	inf->id = *cnt;
	inf->sub = 0;
	inf->type = 0;
	*cnt = inf->id+1;

	return TRUE;
}

void ProjectOptions::InitDevInfo()
{
	long id = 0;
	HMODULE h = LoadLibrary("dsound.dll");
	if (h)
	{
		tDirectSoundEnumerate pDirectSoundEnumerate = (tDirectSoundEnumerate)GetProcAddress(h, "DirectSoundEnumerateA");
		if (pDirectSoundEnumerate)
			pDirectSoundEnumerate(EnumWaveDevice, &id);
	}

	UINT ndev = midiInGetNumDevs();
	for (UINT n = 0; n < ndev; n++)
	{
		MIDIINCAPS caps;
		memset(&caps, 0, sizeof(caps));
		midiInGetDevCaps(n, &caps, sizeof(caps));
		SoundDevInfo *inf = midiList.AddItem();
		inf->name = caps.szPname;
		inf->info = NULL;
		inf->id = (long)n;
		inf->sub = 0;
		inf->type = 1;
	}
}
#endif

#ifdef UNIX
static void alsa_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...)
{
	if (prjFrame)
	{
// TODO: we may be on a background thread...
//		prjFrame->Alert(...);
	}
}

void ProjectOptions::InitDevInfo()
{
	snd_config_update();

	int ctlNum = -1;
	SoundDevInfo *inf = waveList.AddItem();
	inf->name = "default";
	inf->id = -1;
	inf->sub = -1;
	inf->type = 0;
	inf->info = NULL;

	while (snd_card_next(&ctlNum) == 0 && ctlNum >= 0 )
	{
#if SND_LIB_VERSION >= 0x1000E
		void **hints = 0;
		void **hh;
		if (snd_device_name_hint(ctlNum, "pcm", &hints) == 0)
		{
			hh = hints;
			while (*hh)
			{
				char *name = snd_device_name_get_hint(*hh, "NAME");
				char *desc = snd_device_name_get_hint(*hh, "DESC");
				for (char *cp = desc; *cp; cp++)
				{
					if (*cp < 0x20)
						*cp = ' ';
				}
				SoundDevInfo *inf = waveList.AddItem();
				inf->id = ctlNum;
				inf->sub = 0;
				inf->type = 0;
				inf->name = desc;
				inf->info = name;
				if (desc)
					free(desc);
				hh++;
			}
			snd_device_name_free_hint(hints);
		}
		if (snd_device_name_hint(ctlNum, "rawmidi", &hints) == 0)
		{
			hh = hints;
			while (*hh)
			{
				char *name = snd_device_name_get_hint(*hh, "NAME");
				char *desc = snd_device_name_get_hint(*hh, "DESC");
				for (char *cp = desc; *cp; cp++)
				{
					if (*cp < 0x20)
						*cp = ' ';
				}
				SoundDevInfo *inf = midiList.AddItem();
				inf->id = ctlNum;
				inf->sub = 0;
				inf->type = 0;
				inf->name = desc;
				inf->info = name;
				if (desc)
					free(desc);
				hh++;
			}
			snd_device_name_free_hint(hints);
		}
#endif
		/*
        snd_ctl_card_info_t *devInfo;
        snd_pcm_t *pcmHandle;
        snd_pcm_info_t *pcmInfo;
        snd_rawmidi_t *midiHandle;
        snd_rawmidi_info_t *midiInfo;

        char hw[128];

        snprintf(hw, sizeof(hw), "hw:%d", ctlNum);

        int devNum;
        snd_ctl_t *ctl;
        if (snd_ctl_open(&ctl, hw, 0) >= 0)
        {
        	devNum = -1;
			while (snd_ctl_pcm_next_device(ctl, &devNum) == 0 && devNum >= 0)
			{

				snprintf(hw, sizeof(hw), "hw:%d,%d", ctlNum, devNum);
				if (snd_pcm_open(&pcmHandle, hw, SND_PCM_STREAM_PLAYBACK, 0) >= 0)
				{
					snd_pcm_info_malloc(&pcmInfo);
					snd_pcm_info(pcmHandle, pcmInfo);
					SoundDevInfo *inf = waveList.AddItem();
					inf->id = ctlNum;
					inf->sub = devNum;
					inf->type = 0;
					inf->info = (void*)pcmInfo;
					inf->name = snd_pcm_info_get_name(pcmInfo);
					//inf->name += " - ";
					//inf->name += snd_pcm_info_get_subdevice_name(pcmInfo);
					snd_pcm_close(pcmHandle);
					pcmHandle = 0;
				}
			}
			devNum = -1;
			while (snd_ctl_rawmidi_next_device(ctl, &devNum) == 0 && devNum >= 0)
			{
				snprintf(hw, sizeof(hw), "hw:%d,%d", ctlNum, devNum);
				if (snd_rawmidi_open(&midiHandle, NULL, hw, 0) >= 0)
				{
					snd_rawmidi_info_malloc(&midiInfo);
					snd_rawmidi_info(midiHandle, midiInfo);
					SoundDevInfo *inf = midiList.AddItem();
					inf->id = ctlNum;
					inf->sub = devNum;
					inf->type = 1;
					inf->info = (void*)midiInfo;
					inf->name = snd_rawmidi_info_get_name(midiInfo);
					//inf->name += " - ";
					//inf->name += snd_rawmidi_info_get_subdevice_name(midiInfo);
					snd_rawmidi_close(midiHandle);
				}
			}
//			SoundDevInfo *inf = waveList.AddItem();
//			snd_ctl_card_info_alloca(&devInfo);
//	        snd_ctl_card_info(ctl, devInfo);
//			inf->name = snd_ctl_card_info_get_name(devInfo);
//			inf->info = reinterpret_cast<void*>(devInfo);
//			inf->id = (long)devNum;
			snd_ctl_close(ctl);
		}*/
	}
	// TODO:
	//snd_lib_error_set_handler(alsa_error_handler);
}
#endif
