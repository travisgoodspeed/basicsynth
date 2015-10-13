//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef _MIXERSETUPDLG_H_
#define _MIXERSETUPDLG_H_

#pragma once

/// @brief Dialog for mixer effects.
/// @details The effects dialog handles properties for
/// mixer effects (reverb, flanger and echo).
class EffectsSetupDlg : 
	public wxDialog,
	public ProjectItemDlg<EffectsSetupDlg>
{
private:
    DECLARE_EVENT_TABLE()
public:
	EffectsSetupDlg(wxWindow *parent, ProjectItem *pi);
	virtual ~EffectsSetupDlg();

	virtual int GetFieldID(int id, wxWindow **child);
	virtual int GetLabelID(int id, wxWindow **child);

	void OnInitDialog(wxInitDialogEvent&);
	void OnOK(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnReset(wxCommandEvent&);
};

/// @brief Mixer setup dialog.
/// @details This dialog configures the mixer.
class MixerSetupDlg : 
	public wxDialog,
	public ProjectItemDlg<MixerSetupDlg>
{
private:
    DECLARE_EVENT_TABLE()
	wxListBox  *fxList;
	wxComboBox *fxType;
	wxTextCtrl *edName;
	
	void EnableUpDn();

public:
	MixerSetupDlg(wxWindow *parent, ProjectItem *pi);

	virtual ~MixerSetupDlg();

	virtual int GetFieldID(int id, wxWindow **child);

	void OnInitDialog(wxInitDialogEvent&);
	void OnOK(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnFxMvup(wxCommandEvent&);
	void OnFxMvdn(wxCommandEvent&);
	void OnFxAdd(wxCommandEvent&);
	void OnFxRem(wxCommandEvent&);
	void OnFxEdit(wxCommandEvent&);
	void OnFxSelect(wxCommandEvent&);
};

#endif
