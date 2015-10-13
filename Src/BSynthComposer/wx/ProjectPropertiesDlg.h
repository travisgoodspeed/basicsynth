//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Project properties configuration.
/// @details This dialog sets properties for the project.
class ProjectPropertiesDlg :
	public wxDialog,
	public ProjectItemDlg<ProjectPropertiesDlg>
{
private:
	wxListBox *pathList;
	wxTextCtrl *prjNameWnd;
	wxTextCtrl *prjFileWnd;
	wxTextCtrl *wavFileWnd;
	wxTextCtrl *wavPathWnd;

	void OnNameChange(wxCommandEvent& evt);
	void OnBrowseIn(wxCommandEvent& evt);
	void OnBrowseOut(wxCommandEvent& evt);
	void OnBrowseWv(wxCommandEvent& evt);
	void OnOK(wxCommandEvent& evt);
	void OnFileNameBlur(wxCommandEvent& evt);
	void OnPathMvdn(wxCommandEvent& evt);
	void OnPathMvup(wxCommandEvent& evt);
	void OnPathSelect(wxCommandEvent& evt);
	void OnPathAdd(wxCommandEvent& evt);
	void OnPathRem(wxCommandEvent& evt);

	void EnableOK();
	void EnableUpDn();
	void AutoWavefile();

public:
	ProjectPropertiesDlg(wxWindow *parent, ProjectItem *p);
	~ProjectPropertiesDlg(void);

	int GetFieldID(int id, wxWindow **child);


private:
    DECLARE_EVENT_TABLE()
};
