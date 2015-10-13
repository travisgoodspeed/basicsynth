//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MODSYNTH_DLG_H
#define MODSYNTH_DLG_H

/// @brief Modular instrument configuration.
/// @details The modular synthesis instrument can be configured
/// with any number of unit generators. This dialog allows selection
/// of the unit generators for a specific patch.
class ModSynthConfig : public wxDialog
{
private:
    DECLARE_EVENT_TABLE()
	ModSynth *ms;
	int changed;
	wxBitmapButton *btnUp;
	wxBitmapButton *btnDn;
	wxBitmapButton *btnAdd;
	wxBitmapButton *btnRem;
	wxListBox *lbType;
	wxListBox *lbUgens;
	wxTextCtrl *edName;

	void EnableButtons();

public:
	ModSynthConfig(ModSynth *in)
	{
		changed = 0;
		ms = in;
		wxXmlResource::Get()->LoadDialog(this, ::wxGetActiveWindow(), "DLG_MODSYNTH_CONFIG");
	}

	void OnInitDialog(wxInitDialogEvent& evt);
	void OnAdd(wxCommandEvent& evt);
	void OnRemove(wxCommandEvent& evt);
	void OnMoveUp(wxCommandEvent& evt);
	void OnMoveDown(wxCommandEvent& evt);
	void OnChange(wxCommandEvent& evt);
	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
};

/// @brief Modular instrument connections.
/// @details This dialog configures connections between the unit generators
/// for a modular instrument setup. It displays lists of inputs and outputs
/// so that the user may connect the values together.
class ModSynthConnect : public wxDialog
{
private:
    DECLARE_EVENT_TABLE()
	ModSynth *ms;
	int changed;
	wxListBox *lbConn;
	wxListBox *lbSrc;
	wxListBox *lbDst;
	wxListBox *lbParam;
	wxBitmapButton *btnAdd;
	wxBitmapButton *btnRem;

	struct ConnInfo
	{
		ModSynthUG *src;
		ModSynthUG *dst;
		int index;
	};

	void ClearConnInfo();
	void EnableButtons();

public:
	ModSynthConnect(ModSynth *in)
	{
		ms = in;
		changed = 0;
		wxXmlResource::Get()->LoadDialog(this, ::wxGetActiveWindow(), "DLG_MODSYNTH_CONNECT");
	}

	void OnInitDialog(wxInitDialogEvent& evt);
	void OnAdd(wxCommandEvent& evt);
	void OnRemove(wxCommandEvent& evt);
	void OnSelSrc(wxCommandEvent& evt);
	void OnSelDst(wxCommandEvent& evt);
	void OnSelChange(wxCommandEvent& evt);
	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
};
#endif
