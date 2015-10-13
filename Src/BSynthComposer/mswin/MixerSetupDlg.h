//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef _MIXERSETUPDLG_H_
#define _MIXERSETUPDLG_H_

#pragma once

class EffectsSetupDlg : 
	public CDialogImpl<EffectsSetupDlg>,
	public ProjectItemDlg<EffectsSetupDlg>
{
public:
	EffectsSetupDlg();
	virtual ~EffectsSetupDlg();

	virtual int GetFieldID(int id, int& idval);
	virtual int GetLabelID(int id, int& idval);

	enum { IDD = IDD_EFFECTS};

	BEGIN_MSG_MAP(EffectsSetupDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_RESET, OnReset)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

class MixerSetupDlg : 
	public CDialogImpl<MixerSetupDlg>,
	public ProjectItemDlg<MixerSetupDlg>
{
private:
	CListBox fxList;
	CComboBox fxType;
	CEdit edName;
	
	void EnableUpDn();
	void SetButtonImage(int ctrl, int imgid);
	//void EnableOK();

public:
	MixerSetupDlg();

	virtual ~MixerSetupDlg();

	virtual int GetFieldID(int id, int& idval);

	enum { IDD = IDD_MIXER_SETUP};

	BEGIN_MSG_MAP(MixerSetupDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_FX_LIST, LBN_SELCHANGE, OnFxSelect)
		COMMAND_ID_HANDLER(IDC_FX_UP, OnFxMvup)
		COMMAND_ID_HANDLER(IDC_FX_DN, OnFxMvdn)
		COMMAND_ID_HANDLER(IDC_FX_ADD, OnFxAdd)
		COMMAND_ID_HANDLER(IDC_FX_REM, OnFxRem)
		COMMAND_ID_HANDLER(IDC_FX_EDIT, OnFxEdit)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxRem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFxSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
