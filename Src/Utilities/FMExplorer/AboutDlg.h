// aboutdlg.h : interface of the CAboutDlg class
//
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

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());
		CWindow edWnd = GetDlgItem(IDC_TEXT);
		HRSRC resInfo = FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_HELPTEXT), "TEXT");
		HGLOBAL txt = LoadResource(_Module.GetResourceInstance(), resInfo);
		edWnd.SetWindowText((char *)LockResource(txt));

		CWindow frm = GetDlgItem(IDC_DIAGRAM);
		RECT rc;
		frm.GetClientRect(&rc);
		frm.MapWindowPoints(m_hWnd, &rc);
		frm.ShowWindow(SW_HIDE);
		mfView.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, WS_EX_CLIENTEDGE);
		mfView.CreateFromResourceID(IDR_DIAGRAM);

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};
