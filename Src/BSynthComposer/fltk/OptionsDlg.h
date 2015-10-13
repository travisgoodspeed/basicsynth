//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Project options dialog class declaration.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef PROJECT_OPTIONS_DLG
#define PROJECT_OPTIONS_DLG

class ProjectOptionsDlg : public Fl_Window
{
private:
	Fl_Input *nameInp;
	Fl_Input *cpyrInp;
	Fl_Check_Button *incSco;
	Fl_Check_Button *incSeq;
	Fl_Check_Button *incTxt;
	Fl_Check_Button *incScr;
	Fl_Check_Button *incSF;
	Fl_Check_Button *incLib;
	Fl_Check_Button *incMIDI;
	Fl_Input *prjfInp;
	Fl_Button *prjfBrowse;
	Fl_Input *wvinInp;
	Fl_Button *wvinBrowse;
	Fl_Input *formInp;
	Fl_Button *formBrowse;
	Fl_Input *colorsInp;
	Fl_Button *colorsBrowse;
	Fl_Input *libsInp;
	Fl_Button *libsBrowse;
	Fl_Input *latency;
	Fl_Choice *midiDev;
	Fl_Choice *waveDev;
	Fl_Button *okBtn;
	Fl_Button *canBtn;

	int doneInput;

public:
	ProjectOptionsDlg();

	Fl_Input *GetProject() { return prjfInp; }
	Fl_Input *GetWaveIn()  { return wvinInp; }
	Fl_Input *GetForms()   { return formInp; }
	Fl_Input *GetLibs()    { return libsInp; }
	Fl_Input *GetColors()  { return colorsInp; }
	void BrowsePath(const char *title, Fl_Input *wdg);
	int DoModal();
	void OnOK();
	void OnCancel();
};

#endif
