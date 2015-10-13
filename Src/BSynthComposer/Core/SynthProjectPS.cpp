//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

/////////////////////////////////////////////////////////////////////////////
// Platform-specific project functions
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <WaveOutDirect.h>
#endif
#ifdef UNIX
#include <WaveOutALSA.h>
#endif

int SynthProject::SetupSoundDevice(float latency)
{
	if (wop != NULL)
	{
		wop->Shutdown();
		delete wop;
		wop = NULL;
	}

	SoundDevInfo *sdi = 0;
	while ((sdi = prjOptions.waveList.EnumItem(sdi)) != NULL)
	{
		if (sdi->name.CompareNC(prjOptions.waveDevice) == 0)
			break;
	}

#ifdef _WIN32
	WaveOutDirectI *wvd = new WaveOutDirectI;
	GUID *waveID = NULL;
	if (sdi != NULL)
		waveID = reinterpret_cast<GUID*>(sdi->info);
	if (wvd->Setup(prjOptions.dsoundHWND, latency, 4, waveID))
	{
		if (prjGenerate)
			prjGenerate->AddMessage("DirectSound Device failed to open");
		delete wvd;
		return -1;
	}
#endif
#ifdef UNIX
	WaveOutALSA *wvd = new WaveOutALSA;
	char *pcm;
	if (sdi && sdi->id >= 0)
		pcm = (char *)sdi->info;
	else
		pcm = (char*) "default";
	int err;
	if ((err = wvd->Setup(pcm, latency, 3)) < 0)
	{
		perror("ALSA Setup");
		if (prjGenerate)
		{
			char msg[80];
			snprintf(msg, 80, "ALSA Device '%s' failed to open (%d): %s", pcm, err, snd_strerror(err));
			prjGenerate->AddMessage(msg);
		}
		delete wvd;
		return -1;
	}
#endif
	wop = wvd;
	return 0;
}
