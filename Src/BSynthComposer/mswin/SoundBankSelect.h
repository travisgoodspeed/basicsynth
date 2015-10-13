#ifndef SOUNDBANKSELECT_H
#define SOUNDBANKSELECT_H

class SoundBankSelect : public CDialogImpl<SoundBankSelect>
{
protected:
	CComboBox fileList;
	CListBox bankList;
	CListBox instrList;

	void SwitchFile();
	void SwitchBank();
	SoundBank *GetFile();
	SBInstr *GetInstr();
	int GetBankNum();
	int GetInstrNum();

public:
	int IDD;
	bsString fileID;
	bsString insName;
	bsInt16 bnkNum;
	bsInt16 insNum;

	SoundBankSelect() 
	{
		bnkNum = -1;
		insNum = -1;
		IDD = IDD_SOUNDBANK_SEL;
	}


	BEGIN_MSG_MAP(SoundBankSelect)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_SB_FILE, CBN_SELCHANGE, OnFileChange)
		COMMAND_HANDLER(IDC_SB_BANK, LBN_SELCHANGE, OnBankChange)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBankChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
