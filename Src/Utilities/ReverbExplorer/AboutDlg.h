/////////////////////////////////////////////////////////////////////////////
// Reverb explorer About... and Help dialogs
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};

#include "MetafileDisplay.h"

class CHelpDlg : public CDialogImpl<CHelpDlg>
{
private:
	CMetafileDisplay mfView;

public:
	enum { IDD = IDD_HELP};

	BEGIN_MSG_MAP(CHelpDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		CWindow txtWnd;
		txtWnd = GetDlgItem(IDC_TEXT);

		HRSRC resInfo = FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_HELPTEXT), "TEXT");
		HGLOBAL txt = LoadResource(_Module.GetResourceInstance(), resInfo);
		if (txt != NULL)
		{
			char *ptxt = (char *)::LockResource(txt);
			if (ptxt == NULL)
				ptxt = "Could not load text resource...";
			txtWnd.SetWindowText(ptxt);
		}

		CWindow frm = GetDlgItem(IDC_REVERB);
		RECT rc;
		frm.GetClientRect(&rc);
		frm.MapWindowPoints(m_hWnd, &rc);
		frm.ShowWindow(SW_HIDE);
		mfView.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, WS_EX_CLIENTEDGE);
		mfView.CreateFromResourceID(IDR_RVRB);

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};