//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef PROJECT_PROPERTIES_DLG_H
#define PROJECT_PROPERTIES_DLG_H

class ProjectPropertiesDlg : 
	public CDialogImpl<ProjectPropertiesDlg>,
	public ProjectItemDlg<ProjectPropertiesDlg>
{
private:
	CEdit prjNameWnd;
	CEdit prjFileWnd;
	CEdit wavFileWnd;
	CEdit wavPathWnd;
	CListBox lbPath;

	void AutoWavefile();
	void EnableUpDn();
	void EnableOK();

public:
	ProjectPropertiesDlg()
	{
	}

	virtual int GetFieldID(int id, int& idval);
	virtual int GetSelection(int id, short& sel);
	virtual void SetSelection(int id, short sel);

	enum { IDD = IDD_PROJECT_PROPERTIES };

	BEGIN_MSG_MAP(ProjectPropertiesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_PROJECT_NAME, EN_CHANGE, OnNameChange)
		COMMAND_HANDLER(IDC_PROJECT_FILE, EN_CHANGE, OnNameChange)
		COMMAND_HANDLER(IDC_PROJECT_FILE, EN_KILLFOCUS, OnFileNameBlur)
		COMMAND_HANDLER(IDC_PROJECT_PATH, LBN_SELCHANGE, OnPathSelect)
		COMMAND_ID_HANDLER(IDC_PROJECT_PATH_UP, OnPathMvup)
		COMMAND_ID_HANDLER(IDC_PROJECT_PATH_DN, OnPathMvdn)
		COMMAND_ID_HANDLER(IDC_PROJECT_PATH_ADD, OnPathAdd)
		COMMAND_ID_HANDLER(IDC_PROJECT_PATH_REM, OnPathRem)
		COMMAND_ID_HANDLER(IDC_PROJECT_OUT_BROWSE, OnBrowseOut)
		COMMAND_ID_HANDLER(IDC_PROJECT_IN_BROWSE, OnBrowseIn)
		COMMAND_ID_HANDLER(IDC_PROJECT_WV_BROWSE, OnBrowseWv)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPathSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPathMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPathMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPathAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPathRem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseOut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseWv(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileNameBlur(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
