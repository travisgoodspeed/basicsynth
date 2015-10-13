//////////////////////////////////////////////////////////////////////
/// @file SoundBankSelect.cpp Dialog to select an instrument in a sound bank.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "SoundBankSelect.h"

int SelectSoundBankPreset(SFPlayerInstr *instr)
{
	SoundBankSelect dlg;
	float value;
	instr->GetParam(SFPLAYER_BANK, &value);
	dlg.bnkNum = (bsInt16) value;
	instr->GetParam(SFPLAYER_PROG, &value);
	dlg.insNum = (bsInt16) value;

	dlg.fileID = instr->GetSoundFile();
	dlg.insName = instr->GetInstrName();

	if (dlg.DoModal() == IDOK)
	{
		instr->SetSoundFile(dlg.fileID);
		instr->SetInstrName(dlg.insName);
		instr->SetParam(SFPLAYER_BANK, (float) dlg.bnkNum);
		instr->SetParam(SFPLAYER_PROG, (float) dlg.insNum);
		return 1;
	}
	return 0;
}

int SelectSoundBankPreset(GMPlayer *gm)
{
	SoundBankSelect dlg;

	float val;
	dlg.fileID = gm->GetSoundFile();
	gm->GetParam(GMPLAYER_BANK, &val);
	dlg.bnkNum = (bsInt16) val;
	gm->GetParam(GMPLAYER_PROG, &val);
	dlg.insNum = (bsInt16) val;

	if (dlg.DoModal() == IDOK)
	{
		gm->SetSoundFile(dlg.fileID);
		gm->SetParam(GMPLAYER_BANK, (float) dlg.bnkNum);
		gm->SetParam(GMPLAYER_PROG, (float) dlg.insNum);
		SoundBank *sbnk = gm->GetSoundBank();
		if (sbnk)
			sbnk->GetInstr(dlg.bnkNum, dlg.insNum, 1);
		return 1;
	}
	return 0;
}

int SelectSoundBankPreset(bsString& file, MIDIControl *instr, int chnl)
{
	SoundBankSelect dlg;

	dlg.fileID = file;
	if (instr && chnl >= 0)
	{
		dlg.bnkNum = instr->GetBank(chnl);
		dlg.insNum = instr->GetPatch(chnl);
	}

	if (dlg.DoModal() == IDOK)
	{
		file = dlg.fileID;
		if (instr && chnl >= 0)
		{
			instr->SetBank(chnl, dlg.bnkNum);
			instr->SetPatch(chnl, dlg.insNum);
		}
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////

LRESULT SoundBankSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	fileList = GetDlgItem(IDC_SB_FILE);
	bankList = GetDlgItem(IDC_SB_BANK);
	instrList = GetDlgItem(IDC_SB_PRESET);

	int bnkSel = 0;
	SoundBank *sb = SoundBank::SoundBankList.next;
	while (sb)
	{
		int index = fileList.AddString(sb->name);
		fileList.SetItemDataPtr(index, (void*)sb);
		//if (sb->name.Compare(fileID) == 0)
		//	bnkSel = index;
		sb = sb->next;
	}
	bnkSel = fileList.FindStringExact(0, fileID);
	fileList.SetCurSel(bnkSel);
	SwitchFile();

	return 1;
}

LRESULT SoundBankSelect::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SoundBank *bnk = GetFile();
	if (bnk)
	{
		fileID = bnk->name;
		bnkNum = GetBankNum();
		SBInstr *pre = GetInstr();
		if (pre)
		{
			insNum = pre->prog;
			insName = pre->instrName;
		}
	}

	EndDialog(IDOK);
	return 0;
}

LRESULT SoundBankSelect::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT SoundBankSelect::OnFileChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bnkNum = 0;
	insNum = 0;
	SwitchFile();
	return 0;
}

LRESULT SoundBankSelect::OnBankChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	insNum = 0;
	SwitchBank();
	return 0;
}


void SoundBankSelect::SwitchFile()
{
	bankList.ResetContent();
	int sel = fileList.GetCurSel();
	if (sel != CB_ERR)
	{
		int bnkSel = 0;
		SoundBank *sb = (SoundBank *) fileList.GetItemDataPtr(sel);
		for (int n = 0; n < 129; n++)
		{
			if (sb->instrBank[n])
			{
				char num[40];
				sprintf_s(num, 40, "%d", n);
				int index = bankList.AddString(num);
				bankList.SetItemData(index, n);
				if (n == bnkNum)
					bnkSel = index;
			}
		}
		bankList.SetCurSel(bnkSel);
		SwitchBank();
	}
	else
		instrList.ResetContent();
}

void SoundBankSelect::SwitchBank()
{
	instrList.ResetContent();

	int insSel = 0;
	int index;
	SoundBank *sb = GetFile();
	if (sb)
	{
		int bank = GetBankNum();
		if (bank >= 0)
		{
			SBInstr *ins;
			for (int n = 0; n < 128; n++)
			{
				if ((ins = sb->GetInstr(bank, n, 0)) != 0)
				{
					index = instrList.AddString(ins->instrName);
					instrList.SetItemDataPtr(index, ins);
				}
			}
			// because the list is sorted, we have to search...
			int count = instrList.GetCount();
			for (index = 0; index < count; index++)
			{
				ins = (SBInstr *)instrList.GetItemDataPtr(index);
				if (ins->prog == insNum)
				{
					instrList.SetCurSel(index);
					break;
				}
			}
		}
	}
}

SoundBank *SoundBankSelect::GetFile()
{
	int sel = fileList.GetCurSel();
	if (sel != CB_ERR)
		return (SoundBank *) fileList.GetItemDataPtr(sel);
	return 0;
}

int SoundBankSelect::GetBankNum()
{
	int sel = bankList.GetCurSel();
	if (sel != LB_ERR)
		return (int) bankList.GetItemData(sel);
	return -1;
}

SBInstr *SoundBankSelect::GetInstr()
{
	int sel = instrList.GetCurSel();
	if (sel != LB_ERR)
		return (SBInstr *) instrList.GetItemDataPtr(sel);
	return 0;
}

int SoundBankSelect::GetInstrNum()
{
	SBInstr *ins = GetInstr();
	if (ins)
		return ins->prog;
	return -1;
}

