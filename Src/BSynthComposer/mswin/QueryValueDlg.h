//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _QUERYVALUEDLG_H
#define _QUERYVALUEDLG_H

class QueryValueDlg : public CDialogImpl<QueryValueDlg>
{
private:
	char *value;
	int   vlen;
	const char *caption;

public:
	QueryValueDlg(const char *p, char *v, int l)
	{
		value = v;
		vlen = l;
		caption = p;
	}

	enum { IDD = IDD_GET_VALUE };

	BEGIN_MSG_MAP(QueryValueDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt;
		GetCursorPos(&pt);
		SetWindowPos(0, pt.x, pt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		//CenterWindow();
		SetWindowText(caption);
		SetDlgItemText(IDC_VALUE, value);
		return 1;
	}
	LRESULT OnOK(WORD code, WORD id, HWND ctl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_VALUE, value, vlen);
		EndDialog(IDOK);
		return 0;
	}
	LRESULT OnCancel(WORD code, WORD id, HWND ctl, BOOL& bHandled)
	{
		EndDialog(IDCANCEL);
		return 0;
	}
};

#endif
