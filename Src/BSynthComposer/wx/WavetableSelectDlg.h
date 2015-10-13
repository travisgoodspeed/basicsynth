//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WVTABLE_SELECT_DLG_H
#define WVTABLE_SELECT_DLG_H

/// @brief Widget to draw a waveform
class WavetableDraw  : public wxWindow
{
private:
    DECLARE_EVENT_TABLE()
public:
	int index;

	WavetableDraw(wxWindow *parent, wxWindowID id, wxPoint& pos, wxSize& sz) 
		: wxWindow(parent, id, pos, sz, wxBORDER_SIMPLE)
	{
		index = 0;
	}

	void OnPaint(wxPaintEvent& evt);
};

/// @brief Select from the available waveforms.
class WavetableSelectDlg : public wxDialog
{
private:
    DECLARE_EVENT_TABLE()
	wxListBox *lst;
	WavetableDraw *plotter;
	int initSelect;

public:
	WavetableSelectDlg(wxWindow *parent, int wt);

	void SetIndex(int n) { initSelect = n; }
	int GetIndex() { return initSelect; }

	void OnIndexChange(wxCommandEvent& evt);
	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
	void OnInitDialog(wxInitDialogEvent& evt);
};

#endif
