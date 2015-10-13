//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Find and Replace Dialog
/// @details This dialog is used by the text editor
/// to get find and replace text. The dialog is a non-modal
/// popup window, created once and reused. When the editor
/// is activated, it sets a reference to itself so that the 
/// dialog dispatches commands to the active editor.
class FindReplDlg :
	public wxDialog
{
private:
    DECLARE_EVENT_TABLE()

	wxComboBox *findText;
	wxComboBox *replText;
	wxButton *findBtn;
	wxButton *replBtn;
	wxButton *replAllBtn;
	wxButton *replSelBtn;
	wxCheckBox *matchBtn;
	wxCheckBox *fullWordBtn;
	wxCheckBox *startWordBtn;
	wxCheckBox *regexpBtn;
	wxWindow *statText;

	TextEditor *ed;

	int GetFlags();
	void SetEnable();
	int FindNext();
	void DoReplace(int inSel);
	int GetFindText(wxComboBox* wnd, bsString& text);

public:
	FindReplDlg(wxWindow *parent);
	~FindReplDlg();

	TextEditor *GetEditor()
	{
		return ed;
	}

	void SetEditor(TextEditor *p)
	{
		if (p != ed)
		{
			ed = p;
			SetEnable();
		}
	}

	void OnActivate(wxActivateEvent& evt);
	void OnFindNext(wxCommandEvent& evt);
	void OnReplace(wxCommandEvent& evt);
	void OnReplaceAll(wxCommandEvent& evt);
	void OnReplaceSel(wxCommandEvent& evt);
	void OnCloseCmd(wxCommandEvent& evt);
	void OnTextChanged(wxCommandEvent& evt);
};
