//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Options configuration.
/// @details The options dialog configures application options.
/// The options apply to all projects and generally select the
/// default directories for various files. This dialog also selects
/// the sound card for audio output and the MIDI input for keyboard.
class OptionsDlg :
	public wxDialog
{
private:
    DECLARE_EVENT_TABLE()

	void CheckButton(const char *id, int onoff);
	int IsButtonChecked(const char *id);
	void SetItemText(const char *id, wxString& txt);
	void SetItemText(const char *id, const char *txt);
	void GetItemText(const char *id, wxString& txt);
	void GetItemText(const char *id, char *txt, int len);
	void BrowseDir(const char *id, const char *prompt);

public:
	OptionsDlg(wxWindow *parent);
	~OptionsDlg(void);

	void OnOK(wxCommandEvent& evt);
	void OnBrowseProjects(wxCommandEvent& evt);
	void OnBrowseWavein(wxCommandEvent& evt);
	void OnBrowseForms(wxCommandEvent& evt);
	void OnBrowseColors(wxCommandEvent& evt);
	void OnBrowseLibraries(wxCommandEvent& evt);

};
