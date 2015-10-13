//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef SOUNDBANKSELECT_H
#define SOUNDBANKSELECT_H

/// @brief Select a patch from a sound bank (SF2/DLS).
class SoundBankSelect : public wxDialog
{
protected:
    DECLARE_EVENT_TABLE()

	wxComboBox *fileList;
	wxListBox  *bankList;
	wxListBox  *instrList;

	void SwitchFile();
	void SwitchBank();
	SoundBank *GetFile();
	SBInstr *GetInstr();
	int GetBankNum();
	int GetInstrNum();

public:
	bsString fileID;
	bsString insName;
	bsInt16 bnkNum;
	bsInt16 insNum;

	SoundBankSelect(bsInt16 bn, bsInt16 in, const char *fs, const char *is) 
	{
		bnkNum = bn;
		insNum = in;
		fileID = fs;
		insName = is;
		wxXmlResource::Get()->LoadDialog(this, ::wxGetActiveWindow(), "DLG_SOUNDBANK_SEL");
	}

	void OnInitDialog(wxInitDialogEvent&);
	void OnOK(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnFileChange(wxCommandEvent&);
	void OnBankChange(wxCommandEvent&);
};

#endif
