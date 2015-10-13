//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Query for a value.
class QueryValueDlg :
	public wxDialog
{
private:
    DECLARE_EVENT_TABLE()

	char *value;
	int   vlen;
	const char *caption;
	wxTextCtrl *txtwnd;

public:
	QueryValueDlg(wxWindow *parent, const char *p, char *v, int l);
	virtual ~QueryValueDlg(void);

	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
};
