//////////////////////////////////////////////////////////////////////
// Save settings for the waveform
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_SAVEARGS_H__F8D2CD42_19EA_4CF0_8536_47C17C34A5AA__INCLUDED_)
#define AFX_SAVEARGS_H__F8D2CD42_19EA_4CF0_8536_47C17C34A5AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CSaveArgs : public CDialogImpl<CSaveArgs>
{
public:
	double width;
	double height;
	int inchOrMM;
	char fileName[MAX_PATH];

	CSaveArgs();
	~CSaveArgs();

	enum { IDD = IDD_SAVEPIC };

	BEGIN_MSG_MAP(CSaveArgs)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_HANDLER(IDC_FILENAME, EN_CHANGE, OnNameChanged)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNameChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif // !defined(AFX_SAVEARGS_H__F8D2CD42_19EA_4CF0_8536_47C17C34A5AA__INCLUDED_)
