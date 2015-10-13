//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Wavetable selection dialog class declaration.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WVTABLE_SELECT_DLG
#define WVTABLE_SELECT_DLG

class WavetableDraw : public Fl_Widget
{
public:
	int index;

	WavetableDraw(int X, int Y, int W, int H);

	void draw();
};

class WvtableSelectDlg :
	public Fl_Window
{
private:
	WavetableDraw *plotter;
	Fl_Hold_Browser *lst;
	Fl_Button *okbtn;
	Fl_Button *canbtn;
	int done;
public:
	WvtableSelectDlg();
	~WvtableSelectDlg();

	void OnSelect();
	void OnOK();
	void OnCancel();

	int SelectWaveform(int index);
};

#endif
