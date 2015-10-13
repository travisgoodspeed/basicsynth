//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef OPTIONS_DLG_H
#define OPTIONS_DLG_H

class OptionsDlg : public CDialogImpl<OptionsDlg>
{
private:
	void Browse(int id, char *caption);
	CComboBox waveDev;
	CComboBox midiDev;

	void GetItemUTF8(int id, char *cbuf, int blen);
	void SetItemUTF8(int id, const char *str);

public:
	OptionsDlg();

	enum { IDD = IDD_OPTIONS };

	BEGIN_MSG_MAP(OptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE_WAVEIN, OnBrowseWaveIn)
		COMMAND_ID_HANDLER(IDC_BROWSE_PROJECTS, OnBrowseProjects)
		COMMAND_ID_HANDLER(IDC_BROWSE_FORMS, OnBrowseForms)
		COMMAND_ID_HANDLER(IDC_BROWSE_COLORS, OnBrowseColors)
		COMMAND_ID_HANDLER(IDC_BROWSE_LIBRARIES, OnBrowseLibraries)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBrowseProjects(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseWaveIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseForms(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseColors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseLibraries(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
