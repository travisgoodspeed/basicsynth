//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _ITEMPROPERTIES_DLG_H_
#define _ITEMPROPERTIES_DLG_H_

#pragma once

class ItemPropertiesBase : 
	public CDialogImpl<ItemPropertiesBase>,
	public ProjectItemDlg<ItemPropertiesBase>
{
protected:
	int needName;
	int needFile;
	int needInum;

	void EnableOK();

public:
	ItemPropertiesBase() 
	{
		needName = 1;
		needFile = 0;
		needInum = 0;
		IDD = 0;
	}

	virtual ~ItemPropertiesBase()
	{}

	virtual void InitSpecific() { }
	virtual int GetFieldID(int id, int& idval);

	// enum { IDD = 0 }; //<--- brain dead...
	int IDD;

	BEGIN_MSG_MAP(ItemPropertiesBase)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_ITEM_NAME, EN_CHANGE, OnNameChange)
		COMMAND_HANDLER(IDC_FILE_NAME, EN_CHANGE, OnFileNameChange)
		COMMAND_ID_HANDLER(IDC_FILE_BROWSE, OnBrowse)
		COMMAND_HANDLER(IDC_INST_NUM, EN_CHANGE, OnNumChange)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNumChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

class NamePropertiesDlg : public ItemPropertiesBase
{
public:
	NamePropertiesDlg()
	{
		IDD = IDD_NAME_PROPERTIES;
	}
};

class FilePropertiesDlg : public ItemPropertiesBase
{
public:
	FilePropertiesDlg()
	{
		IDD = IDD_FILE_PROPERTIES;
		needFile = 1;
	}
};

class InstrPropertiesDlg : public ItemPropertiesBase
{
public:

	InstrPropertiesDlg()
	{
		IDD = IDD_INSTR_PROPERTIES;
		needInum = 1;
	}
	virtual void InitSpecific();

};

class LibPropertiesDlg : public ItemPropertiesBase
{
public:
	LibPropertiesDlg()
	{
		IDD = IDD_LIB_PROPERTIES;
		needFile = 1;
	}
	virtual void InitSpecific();
};

class WavefilePropertiesDlg : public ItemPropertiesBase
{
public:
	WavefilePropertiesDlg()
	{
		IDD = IDD_WVF_PROPERTIES;
		needFile = 1;
	}

};

class SoundBankPropertiesDlg : public ItemPropertiesBase
{
public:
	SoundBankPropertiesDlg()
	{
		IDD = IDD_SOUNDBANK_PROPERTIES;
		needFile = 1;
	}
};

class InstrSelectDlg : public ItemPropertiesBase
{
private:

public:
	InstrSelectDlg()
	{
		IDD = IDD_SELECT_INSTR;
		needName = 0;
	}

	virtual int GetFieldID(int id, int& idval);
};

class FilelistOrder : 
	public CDialogImpl<FilelistOrder>,
	public ProjectItemDlg<FilelistOrder>
{
protected:
	CListBox fileList;

	void EnableUpDn();

public:
	int IDD;

	FilelistOrder() 
	{
		IDD = IDD_FILE_ORDER;
	}

	//virtual void InitSpecific() { }
	//virtual int GetFieldID(int id, int& idval);

	BEGIN_MSG_MAP(FilelistOrder)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_FILE_UP, OnFileMvup)
		COMMAND_ID_HANDLER(IDC_FILE_DN, OnFileMvdn)
		COMMAND_HANDLER(IDC_FILE_LIST, LBN_SELCHANGE, OnSelect)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
#endif
