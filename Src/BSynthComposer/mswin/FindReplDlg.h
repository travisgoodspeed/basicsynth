//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

#include "resource.h"

class TextEditor;

class FindReplDlg : 
	public CDialogImpl<FindReplDlg>,
	public CMessageFilter
{
private:
	CComboBox findText;
	CComboBox replText;
	CButton findBtn;
	CButton replBtn;
	CButton replAllBtn;
	CButton replSelBtn;
	CButton matchBtn;
	CButton fullWordBtn;
	CButton startWordBtn;
	CButton regexpBtn;
	CWindow statText;

	TextEditor *ed;

	int GetFlags();
	void SetEnable();
	int FindNext();
	void DoReplace(int inSel);
	int GetFindText(CComboBox& wnd, bsString& text);

public:
	FindReplDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (IsDialogMessage(pMsg))
			return TRUE;
		return FALSE;
	}

	TextEditor *GetEditor()
	{
		return ed;
	}

	void SetEditor(TextEditor *p)
	{
		if (p != ed)
		{
			ed = p;
			if (IsWindow())
				SetEnable();
		}
	}

	enum { IDD = IDD_FINDREPL };

	BEGIN_MSG_MAP(FindReplDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDC_FIND_TEXT, CBN_EDITCHANGE, OnTextChanged)
		COMMAND_HANDLER(IDC_REPL_TEXT, CBN_EDITCHANGE, OnTextChanged)
		COMMAND_ID_HANDLER(IDC_FIND_NEXT, OnFindNext)
		COMMAND_ID_HANDLER(IDC_REPLACE, OnReplace)
		COMMAND_ID_HANDLER(IDC_REPLACE_ALL, OnReplaceAll)
		COMMAND_ID_HANDLER(IDC_REPLACE_SELALL, OnReplaceSel)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReplace(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReplaceAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReplaceSel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
