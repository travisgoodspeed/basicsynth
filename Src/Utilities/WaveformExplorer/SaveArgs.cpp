//////////////////////////////////////////////////////////////////////
// Save settings for the waveform
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SaveArgs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSaveArgs::CSaveArgs()
{
	width = 4.0;
	height = 3.0;
	inchOrMM = 0; // inches
	memset(fileName, 0, sizeof(fileName));
}

CSaveArgs::~CSaveArgs()
{

}


LRESULT CSaveArgs::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow();

	char valstr[20];
	sprintf(valstr, "%.2f", width);
	SetDlgItemText(IDC_WIDTH, valstr);
	sprintf(valstr, "%.2f", height);
	SetDlgItemText(IDC_HEIGHT, valstr);
	CheckRadioButton(IDC_MM, IDC_INCH, inchOrMM ? IDC_MM : IDC_INCH);
	SetDlgItemText(IDC_FILENAME, fileName);

	if (fileName[0] == 0)
		::EnableWindow(GetDlgItem(IDOK), FALSE);

	return TRUE;
}

LRESULT CSaveArgs::OnBrowse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = "Meta Files(*.emf)\0*.emf\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = fileName;
	ofn.lpstrDefExt = ".emf";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	
	if (GetSaveFileName (&ofn))
		SetDlgItemText(IDC_FILENAME, fileName);
	return 0;
}

LRESULT CSaveArgs::OnNameChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CEdit name = GetDlgItem(IDC_FILENAME);
	CButton ok = GetDlgItem(IDOK);
	ok.EnableWindow(name.GetWindowTextLength() > 0);
	return 0;
}

LRESULT CSaveArgs::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		char valstr[40];
		GetDlgItemText(IDC_WIDTH, valstr, 40);
		width = atof(valstr);
		GetDlgItemText(IDC_HEIGHT, valstr, 40);
		height = atof(valstr);
		if (IsDlgButtonChecked(IDC_INCH) == BST_CHECKED)
			inchOrMM = 0;
		else
			inchOrMM = 1;
		GetDlgItemText(IDC_FILENAME, fileName, MAX_PATH-1);
	}
	EndDialog(wID);
	return 0;
}