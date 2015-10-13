//////////////////////////////////////////////////////////////////////
// BasicSynth - Specialized widget form for the virtual keyboard.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

int KeyboardForm::Load(const char *fileName, int xo, int yo)
{
	SynthWidget *wdg;

	float defvol = 1.0f;
	int defchnl = 0;
	int defrhy = 4;
	int defacc = 1;

	kbd = 0;
	startStop = 0;
	mainGroup->Clear();
	if (WidgetForm::Load(fileName, xo, yo) != 0)
		return -1;
	// Because the form is loaded from a file and the user
	// may have "customized" it, we have to check each widget
	// to make sure it exists. 
	kbd = (KeyboardWidget*)mainGroup->FindID(2);
	if (kbd)
	{
		kbd->SetVolume(defvol);
		kbd->SetChannel(defchnl);
		kbd->SetDuration(defrhy);
		kbd->SetRecSharps(defacc);
	}
	startStop = mainGroup->FindID(30);

	wdg = mainGroup->FindID(22); // volume
	if (wdg)
		wdg->SetValue(defvol);

	wdg = mainGroup->FindID(25); // channel
	if (wdg)
		wdg->SetValue((float)defchnl);

	wdg = mainGroup->FindID(31); // rhythm
	if (wdg)
		wdg->SetValue(defrhy);

	wdg = mainGroup->FindID(39); // sharps/flats
	if (wdg)
		wdg->SetValue(defacc);

//	wdg = mainGroup->FindID(46); // MIDI on/off
//	if (prjOptions.midiDevice < 0 || prjOptions.midiDeviceName[0] == 0)
//		wdg->SetEnable(0);

	return 0;
}

int KeyboardForm::Start()
{
	if (theProject)
	{
		if (startStop)
		{
			startStop->SetState(1);
			Redraw(startStop);
		}
		theProject->PlayMode(2);
		return theProject->Start();
	}
	return 0;
}

int KeyboardForm::Stop()
{
	if (theProject)
	{
		if (startStop)
		{
			startStop->SetState(0);
			Redraw(startStop);
		}
		return theProject->Stop();
	}
	return 0;
}

void KeyboardForm::ValueChanged(SynthWidget *wdg)
{
	int n, m;
	switch (wdg->GetID())
	{
	case 22: // volume
		if (kbd)
		{
			float vol = wdg->GetValue();
			kbd->SetVolume(vol);
			theProject->mgr.SetVolume(kbd->GetChannel(), (short)(vol*127.0));
		}
		break;
	case 25:
		if (kbd)
			kbd->SetChannel((int)wdg->GetValue());
		break;
	case 26:
		wdg = mainGroup->FindID(25);
		n = (int) wdg->GetValue();
		m = theProject->mixInfo->GetMixerInputs();
		if (++n >= m)
			n = 0;
		if (kbd)
			kbd->SetChannel(n);
		wdg->SetValue((float)n);
		Redraw(wdg);
		break;
	case 31: // rhythm
		if (kbd)
			kbd->SetDuration(wdg->GetValue());
		break;
	case 30: // player on/off
		if (theProject)
		{
			if (wdg->GetState())
			{
				theProject->PlayMode(2);
				theProject->Start();
			}
			else
				theProject->Stop();
		}
		else
			wdg->SetState(0);
		break;
	case 37:
		if (kbd)
			kbd->SetRecord(wdg->GetState());
		break;
	case 38:
		if (kbd)
			kbd->SetRecGroup(wdg->GetState());
		break;
	case 39:
		if (kbd)
			kbd->SetRecSharps((int)wdg->GetValue());
		break;
	case 42:
		if (kbd)
			kbd->CopyNotes();
		break;
	case 44:
		// pop-up generate and route to speaker
		prjFrame->Generate(1, 1);
		break;
	case 45:
		// pop-up generate and route to disk
		prjFrame->Generate(1, 0);
		break;
	case 46:
		// MIDI On/Off
		if (wdg->GetState())
		{
			if (theProject->prjMidiIn.Start())
			{
				wdg->SetState(0);
				Redraw(wdg);
			}
		}
		else
			theProject->prjMidiIn.Stop();
		break;
	}
}
