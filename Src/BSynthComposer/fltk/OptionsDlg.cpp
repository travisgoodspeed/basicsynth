//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Global options dialog implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "OptionsDlg.h"
#include "MainFrm.h"

static void OkCB(Fl_Widget *wdg, void *arg)
{
	((ProjectOptionsDlg*)arg)->OnOK();
}

static void CanCB(Fl_Widget *wdg, void *arg)
{
	((ProjectOptionsDlg*)arg)->OnCancel();
}

static void BrowsePrj(Fl_Widget *wdg, void *arg)
{
	ProjectOptionsDlg* dlg = (ProjectOptionsDlg*)arg;
	dlg->BrowsePath("Project Files Directory", dlg->GetProject());
}

static void BrowseWvin(Fl_Widget *wdg, void *arg)
{
	ProjectOptionsDlg* dlg = (ProjectOptionsDlg*)arg;
	dlg->BrowsePath("Wave Files Directory", dlg->GetWaveIn());
}

static void BrowseForms(Fl_Widget *wdg, void *arg)
{
	ProjectOptionsDlg* dlg = (ProjectOptionsDlg*)arg;
	dlg->BrowsePath("Forms Directory", dlg->GetForms());
}

static void BrowseLibs(Fl_Widget *wdg, void *arg)
{
	ProjectOptionsDlg* dlg = (ProjectOptionsDlg*)arg;
	dlg->BrowsePath("Libraries Directory", dlg->GetLibs());
}

static void BrowseColors(Fl_Widget *wdg, void *arg)
{
	ProjectOptionsDlg* dlg = (ProjectOptionsDlg*)arg;
	Fl_Input *inp = dlg->GetColors();
	const char *dir = fl_file_chooser("Colors File", NULL, inp->value());
	if (dir)
		inp->value(dir);
}

void ProjectOptionsDlg::BrowsePath(const char *title, Fl_Input *inp)
{
	const char *dir = fl_dir_chooser(title, inp->value());
	if (dir)
		inp->value(dir);
}

ProjectOptionsDlg::ProjectOptionsDlg()
	: Fl_Window(100, 100, 540, 500, "Project Options")
{
	int txtHeight = 25;
	int txtSpace = 30;
	int ypos = 5;

	nameInp = new Fl_Input(90, ypos, 440, txtHeight, "Author: ");
	nameInp->value(prjOptions.defAuthor);
	ypos += txtSpace;

	cpyrInp = new Fl_Input(90, ypos, 440, txtHeight, "Copyright: ");
	cpyrInp->value(prjOptions.defCopyright);
	ypos += txtSpace;

	Fl_Box *lbl = new Fl_Box(5, ypos, 85, txtHeight, "Include:");
	incSco = new Fl_Check_Button(90, ypos, 90, txtHeight, "Notelists");
	incSco->value(prjOptions.inclNotelist);

	incSeq = new Fl_Check_Button(190, ypos, 90, txtHeight, "Sequences");
	incSeq->value(prjOptions.inclSequence);

	incTxt = new Fl_Check_Button(290, ypos, 90, txtHeight, "Text Files");
	incTxt->value(prjOptions.inclTextFiles);

	incScr = new Fl_Check_Button(390, ypos, 90, txtHeight, "Scripts");
	incScr->value(prjOptions.inclScripts);
	ypos += txtSpace;

	incSF = new Fl_Check_Button(90, ypos, 90, txtHeight, "Sound Fonts");
	incSF->value(prjOptions.inclSoundFonts);

	incLib = new Fl_Check_Button(190, ypos, 90, txtHeight, "Libraries");
	incLib->value(prjOptions.inclLibraries);

	incMIDI = new Fl_Check_Button(290, ypos, 90, txtHeight, "MIDI");
	incMIDI->value(prjOptions.inclMIDI);
	ypos += txtSpace;

	prjfInp = new Fl_Input(90, ypos, 420, txtHeight, "Project Files: ");
	prjfInp->value(prjOptions.defPrjDir);
	prjfBrowse = new Fl_Button(510, ypos, 20, txtHeight, "...");
	prjfBrowse->callback(BrowsePrj, (void*)this);
	ypos += txtSpace;

	wvinInp = new Fl_Input(90, ypos, 420, txtHeight, "Wave Files: ");
	wvinInp->value(prjOptions.defWaveIn);
	wvinBrowse = new Fl_Button(510, ypos, 20, txtHeight, "...");
	wvinBrowse->callback(BrowseWvin, (void*)this);
	ypos += txtSpace;

	formInp = new Fl_Input(90, ypos, 420, txtHeight, "Form Files: ");
	formInp->value(prjOptions.formsDir);
	formBrowse = new Fl_Button(510, ypos, 20, txtHeight, "...");
	formBrowse->callback(BrowseForms, (void*)this);
	ypos += txtSpace;

	colorsInp = new Fl_Input(90, ypos, 420, txtHeight, "Colors: ");
	colorsInp->value(prjOptions.colorsFile);
	colorsBrowse = new Fl_Button(510, ypos, 20, txtHeight, "...");
	colorsBrowse->callback(BrowseColors, (void*)this);
	ypos += txtSpace;

	libsInp = new Fl_Input(90, ypos, 420, txtHeight, "Library Files: ");
	libsInp->value(prjOptions.defLibDir);
	libsBrowse = new Fl_Button(510, ypos, 20, txtHeight, "...");
	libsBrowse->callback(BrowseLibs, (void*)this);
	ypos += txtSpace;

	midiDev = new Fl_Choice(90, ypos, 175, txtHeight, "MIDI In");
	SoundDevInfo *sdi = NULL;
	while ((sdi = prjOptions.midiList.EnumItem(sdi)) != NULL)
		midiDev->add(sdi->name);
	if (prjOptions.midiDeviceName[0])
	{
		const Fl_Menu_Item *itm = midiDev->find_item(prjOptions.midiDeviceName);
		if (itm)
			midiDev->value(itm);
	}

	waveDev = new Fl_Choice(350, ypos, 175, txtHeight, "Wave Out");
	sdi = NULL;
	while ((sdi = prjOptions.waveList.EnumItem(sdi)) != NULL)
		waveDev->add(sdi->name);
	if (prjOptions.waveDevice[0])
	{
		const Fl_Menu_Item *itm = waveDev->find_item(prjOptions.waveDevice);
		if (itm)
			waveDev->value(itm);
	}
	ypos += txtSpace;

	latency = new Fl_Input(120, ypos, 100, txtHeight, "Playback Latency");
	char buf[80];
	snprintf(buf, 80, "%f", prjOptions.playBuf);
	latency->value(buf);


	okBtn = new Fl_Button(420, ypos, 50, txtHeight, "OK");
	okBtn->callback(OkCB, (void*)this);

	canBtn = new Fl_Button(480, ypos, 50, txtHeight, "Cancel");
	canBtn->callback(CanCB, (void*)this);
	ypos += txtSpace;

	end();
	resize(mainWnd->x()+100, mainWnd->y()+100, 540, ypos+5);
}

void ProjectOptionsDlg::OnOK()
{
	strncpy(prjOptions.defAuthor, nameInp->value(), MAX_PATH);
	strncpy(prjOptions.defCopyright, cpyrInp->value(), MAX_PATH);
	prjOptions.inclNotelist = incSco->value();
	prjOptions.inclSequence = incSeq->value();
	prjOptions.inclTextFiles = incTxt->value();
	prjOptions.inclSoundFonts = incSF->value();
	prjOptions.inclScripts = incScr->value();
	prjOptions.inclLibraries = incLib->value();
	prjOptions.inclMIDI = incMIDI->value();
	strncpy(prjOptions.defPrjDir, prjfInp->value(), MAX_PATH);
	strncpy(prjOptions.defWaveIn, wvinInp->value(), MAX_PATH);
	strncpy(prjOptions.formsDir, formInp->value(), MAX_PATH);
	strncpy(prjOptions.colorsFile, colorsInp->value(), MAX_PATH);
	strncpy(prjOptions.defLibDir, libsInp->value(), MAX_PATH);
	prjOptions.playBuf = atof(latency->value());
	const char *itm;
	if ((itm = waveDev->text()) != NULL)
		strncpy(prjOptions.waveDevice, itm, MAX_PATH);
	if ((itm = midiDev->text()) != NULL)
		strncpy(prjOptions.midiDeviceName, itm, MAX_PATH);
	SoundDevInfo *sdi = 0;
	while ((sdi = prjOptions.midiList.EnumItem(sdi)) != 0)
	{
		if (sdi->name.CompareNC(prjOptions.midiDeviceName) == 0)
		{
			prjOptions.midiDevice = sdi->id;
			break;
		}
	}
	doneInput = 1;
	prjOptions.Save();
}

void ProjectOptionsDlg::OnCancel()
{
	doneInput = 2;
}

int ProjectOptionsDlg::DoModal()
{
	doneInput = 0;
	set_modal();
	show();
	while (!doneInput)
		Fl::wait();

	return doneInput == 1 ? 1 : 0;
}

