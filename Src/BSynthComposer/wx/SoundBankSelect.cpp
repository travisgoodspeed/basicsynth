//////////////////////////////////////////////////////////////////////
/// @file SoundBankSelect.cpp Dialog to select an instrument in a sound bank.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "SoundBankSelect.h"

int SelectSoundBankPreset(SFPlayerInstr *instr)
{
	float bn;
	float in;
	instr->GetParam(SFPLAYER_BANK, &bn);
	instr->GetParam(SFPLAYER_PROG, &in);
	SoundBankSelect dlg((bsInt16)bn, (bsInt16)in, instr->GetSoundFile(), instr->GetInstrName());

	if (dlg.ShowModal())
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
	float bn;
	float in;
	gm->GetParam(GMPLAYER_BANK, &bn);
	gm->GetParam(GMPLAYER_PROG, &in);
	SoundBankSelect dlg((bsInt16)bn, (bsInt16)in, gm->GetSoundFile(), "");

	if (dlg.ShowModal())
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
	int bnk = 0;
	int prg = 0;
	if (instr && chnl >= 0)
	{
		bnk = instr->GetBank(chnl);
		prg = instr->GetPatch(chnl);
	}
	SoundBankSelect dlg(bnk, prg, file, "");

	if (dlg.ShowModal())
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

BEGIN_EVENT_TABLE(SoundBankSelect,wxDialog)
	EVT_BUTTON(wxID_OK, SoundBankSelect::OnOK)
	EVT_BUTTON(wxID_CANCEL, SoundBankSelect::OnCancel)
	EVT_COMBOBOX(XRCID("IDC_SB_FILE"), SoundBankSelect::OnFileChange)
	EVT_LISTBOX(XRCID("IDC_SB_BANK"), SoundBankSelect::OnBankChange)
	EVT_INIT_DIALOG(SoundBankSelect::OnInitDialog)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////

void SoundBankSelect::OnInitDialog(wxInitDialogEvent&)
{
	CenterOnParent();

	fileList = (wxComboBox*) FindWindow("IDC_SB_FILE");
	bankList = (wxListBox*)  FindWindow("IDC_SB_BANK");
	instrList = (wxListBox*)  FindWindow("IDC_SB_PRESET");

	SoundBank *sb = SoundBank::SoundBankList.next;
	while (sb)
	{
		fileList->Append((const char*)sb->name, reinterpret_cast<void*>(sb));
		sb = sb->next;
	}
	fileList->SetStringSelection((const char *)fileID);
	SwitchFile();
}

void SoundBankSelect::OnOK(wxCommandEvent& evt)
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
	EndModal(1);
}

void SoundBankSelect::OnCancel(wxCommandEvent& evt)
{
	EndModal(0);
}

void SoundBankSelect::OnFileChange(wxCommandEvent& evt)
{
	bnkNum = 0;
	insNum = 0;
	SwitchFile();
}

void SoundBankSelect::OnBankChange(wxCommandEvent& evt)
{
	insNum = 0;
	SwitchBank();
}

void SoundBankSelect::SwitchFile()
{
	bankList->Clear();
	int sel = fileList->GetSelection();
	if (sel != wxNOT_FOUND)
	{
		int bnkSel = 0;
		SoundBank *sb = (SoundBank *) fileList->GetClientData(sel);
		for (int n = 0; n < 129; n++)
		{
			if (sb->instrBank[n])
			{
				char num[40];
				snprintf(num, 40, "%d", n);
				int index = bankList->Append(num, (void*)n);
				if (n == bnkNum)
					bnkSel = index;
			}
		}
		bankList->SetSelection(bnkSel);
		SwitchBank();
	}
	else
		instrList->Clear();
}

void SoundBankSelect::SwitchBank()
{
	instrList->Clear();

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
					index = instrList->Append((const char *)ins->instrName, (void*)ins);
			}
			// because the list is sorted, we have to search...
			int count = instrList->GetCount();
			for (index = 0; index < count; index++)
			{
				ins = (SBInstr *)instrList->GetClientData(index);
				if (ins->prog == insNum)
				{
					instrList->SetSelection(index);
					break;
				}
			}
		}
	}
}

SoundBank *SoundBankSelect::GetFile()
{
	int sel = fileList->GetSelection();
	if (sel != wxNOT_FOUND)
		return (SoundBank *) fileList->GetClientData(sel);
	return 0;
}

int SoundBankSelect::GetBankNum()
{
	int sel = bankList->GetSelection();
	if (sel != wxNOT_FOUND)
		return (int) (long) bankList->GetClientData(sel);
	return -1;
}

SBInstr *SoundBankSelect::GetInstr()
{
	int sel = instrList->GetSelection();
	if (sel != wxNOT_FOUND)
		return (SBInstr *) instrList->GetClientData(sel);
	return 0;
}

int SoundBankSelect::GetInstrNum()
{
	SBInstr *ins = GetInstr();
	if (ins)
		return ins->prog;
	return -1;
}

