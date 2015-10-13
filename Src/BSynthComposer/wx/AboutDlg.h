//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief About Dialog
/// @details Display the "About..." dialog.
class AboutDlg :
	public wxDialog
{
public:
	AboutDlg(wxWindow *parent)
	{
		wxXmlResource::Get()->LoadDialog(this, parent, "DLG_ABOUTBOX");
		CenterOnParent();
	}

	~AboutDlg(void) { }
};
