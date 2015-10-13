//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef WVTABLE_SELECT_DLG_H
#define WVTABLE_SELECT_DLG_H

class WavetableDraw  : public CWindowImpl<WavetableDraw>
{
public:
	int index;

	WavetableDraw()
	{
		index = 0;
	}

	DECLARE_WND_CLASS(_T("WavetableDraw"))

	BEGIN_MSG_MAP(WavetableDraw)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class WvtableSelectDlg : public CDialogImpl<WvtableSelectDlg>
{
private:
	//CListBox lst;
	utf8ListBox lst;
	WavetableDraw plotter;

public:
	WvtableSelectDlg();

	void SetIndex(int n) { plotter.index = n; }
	int GetIndex() { return plotter.index; }

	enum { IDD = IDD_WT_SELECT };

	BEGIN_MSG_MAP(WvtableSelectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_WT_LIST, LBN_SELCHANGE, OnIndexChange)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnIndexChange(WORD code, WORD id, HWND ctl, BOOL& bHandled);
	LRESULT OnOK(WORD code, WORD id, HWND ctl, BOOL& bHandled);
	LRESULT OnCancel(WORD code, WORD id, HWND ctl, BOOL& bHandled);
};

#endif
