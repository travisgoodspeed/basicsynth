//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef SCORE_ERRORS_DLG
#define SCORE_ERRORS_DLG

#include "resource.h"

class ScoreErrorsDlg : 
	public CDialogImpl<ScoreErrorsDlg>
{
private:
	CComboBox itmSel;
	CListBox  errLst;
	CButton   nextBtn;
	CButton   prevBtn;
	CButton   gotoBtn;

public:
	ScoreErrorsDlg();
	~ScoreErrorsDlg();

	void RemoveItem(ProjectItem *itm);
	void Clear();
	void Refresh();
	void ShowErrors();
	void MarkErrors();
	void GotoLine();
	void CheckSyntax();
	void ErrSelect(int index, int count);

	enum { IDD = IDD_SCORE_ERRORS };

	BEGIN_MSG_MAP(ScoreErrorsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_ERROR_NEXT, BN_CLICKED, OnNext)
		COMMAND_HANDLER(IDC_ERROR_PREV, BN_CLICKED, OnPrev)
		COMMAND_HANDLER(IDC_ERROR_GOTO, BN_CLICKED, OnGoto)
		COMMAND_HANDLER(IDC_ERROR_CHECK, BN_CLICKED, OnCheck)
		COMMAND_HANDLER(IDC_ERROR_MARK, BN_CLICKED, OnMark)
		COMMAND_HANDLER(IDC_ERROR_REFRESH, BN_CLICKED, OnRefresh)
		COMMAND_HANDLER(IDC_ERROR_ITEMS, CBN_SELCHANGE, OnItemSel)
		COMMAND_HANDLER(IDC_ERROR_LIST, LBN_DBLCLK, OnErrGoto)
		COMMAND_HANDLER(IDC_ERROR_LIST, LBN_SELCHANGE, OnErrSel)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGoto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPrev(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMark(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnItemSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnErrGoto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnErrSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
