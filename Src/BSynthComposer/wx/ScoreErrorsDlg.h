//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Display score errors.
class ScoreErrorsDlg :
	public wxDialog
{
private:
    DECLARE_EVENT_TABLE()

	wxComboBox *itmSel;
	wxListBox  *errLst;
	wxButton   *nextBtn;
	wxButton   *prevBtn;
	wxButton   *gotoBtn;

public:
	ScoreErrorsDlg(wxWindow *parent);
	~ScoreErrorsDlg();

	void RemoveItem(ProjectItem *itm);
	void Clear();
	void Refresh();
	void ShowErrors();
	void MarkErrors();
	void GotoLine();
	void CheckSyntax();
	void ErrSelect(int index, int count);


	void OnGoto(wxCommandEvent& evt);
	void OnPrev(wxCommandEvent& evt);
	void OnNext(wxCommandEvent& evt);
	void OnMark(wxCommandEvent& evt);
	void OnCheck(wxCommandEvent& evt);
	void OnRefresh(wxCommandEvent& evt);
	void OnItemSel(wxCommandEvent& evt);
	void OnErrGoto(wxCommandEvent& evt);
	void OnErrSel(wxCommandEvent& evt);
	void OnCloseCmd(wxCommandEvent& evt);
};
