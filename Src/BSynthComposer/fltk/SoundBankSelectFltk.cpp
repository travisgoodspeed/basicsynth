//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file SoundBankSelectFltk.cpp Sound bank selection dialog implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "SoundBankSelectFltk.h"
#include "MainFrm.h"

int SelectSoundBankPreset(SFPlayerInstr *instr)
{
	SoundBankSelect dlg;
	float value;
	instr->GetParam(16, &value);
	dlg.bnkNum = (bsInt16) value;
	instr->GetParam(17, &value);
	dlg.preNum = (bsInt16) value;

	dlg.fileID = instr->GetSoundFile();
	dlg.preName = instr->GetInstrName();

	if (dlg.DoModal())
	{
		instr->SetSoundFile(dlg.fileID);
		instr->SetInstrName(dlg.preName);
		instr->SetParam(16, (float) dlg.bnkNum);
		instr->SetParam(17, (float) dlg.preNum);
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
	dlg.preNum = (bsInt16) val;

	if (dlg.DoModal())
	{
		gm->SetSoundFile(dlg.fileID);
		gm->SetParam(GMPLAYER_BANK, (float) dlg.bnkNum);
		gm->SetParam(GMPLAYER_PROG, (float) dlg.preNum);
		SoundBank *sbnk = gm->GetSoundBank();
		if (sbnk)
			sbnk->GetInstr(dlg.bnkNum, dlg.preNum, 1);
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
		dlg.preNum = instr->GetPatch(chnl);
	}

	if (dlg.DoModal())
	{
		file = dlg.fileID;
		if (instr && chnl >= 0)
		{
			instr->SetBank(chnl, dlg.bnkNum);
			instr->SetPatch(chnl, dlg.preNum);
		}
		return 1;
	}
	return 0;
}

static void OkCB(Fl_Widget *wdg, void *arg)
{
	((SoundBankSelect*)arg)->OnOK();
}

static void CanCB(Fl_Widget *wdg, void *arg)
{
	((SoundBankSelect*)arg)->OnCancel();
}

static void FileCB(Fl_Widget *wdg, void *arg)
{
	((SoundBankSelect*)arg)->OnFileChange();
}

static void BankCB(Fl_Widget *wdg, void *arg)
{
	((SoundBankSelect*)arg)->OnBankChange();
}

SoundBankSelect::SoundBankSelect()
	: Fl_Window(100, 100, 400, 500, "SoundBank")
{
	doneSet = 0;
	int txtHeight = 25;
	int txtSpace  = 30;
	int ypos = 5;

	fileList = new Fl_Choice(50, ypos, 340, txtHeight, "File ");
	fileList->callback(FileCB, (void*)this);

	ypos += txtSpace*2;
	bankList = new Fl_Hold_Browser(10, ypos, 100, 300, "Bank");
	bankList->align(FL_ALIGN_TOP);
	bankList->callback(BankCB, (void*)this);

	presetList = new Fl_Hold_Browser(120, ypos, 270, 300, "Preset");
	presetList->align(FL_ALIGN_TOP);
	ypos += presetList->h() + 5;

	int xpos = (w() - 110) / 2;
	okBtn = new Fl_Button(xpos, ypos, 50, txtHeight, "OK");
	okBtn->callback(OkCB, (void*)this);
	canBtn = new Fl_Button(xpos+60, ypos, 50, txtHeight, "Cancel");
	canBtn->callback(CanCB, (void*)this);
	ypos += 60;
	size(w(), ypos);
	end();
}

SoundBankSelect::~SoundBankSelect()
{
}

int SoundBankSelect::DoModal()
{
	position(mainWnd->x()+100, mainWnd->y()+100);

	int bnkSel = 0;
	int index = 0;
	SoundBank *sb = SoundBank::SoundBankList.next;
	while (sb)
	{
		fileList->add(sb->name, 0, 0, (void*)sb, 0);
		if (sb->name.Compare(fileID) == 0)
			bnkSel = index;
		sb = sb->next;
		index++;
	}

	fileList->value(bnkSel);
	SwitchFile();

	doneSet = 0;
	set_modal();
	show();
	while (!doneSet)
		Fl::wait();
	return doneSet == 1 ? 1 : 0;
}

void SoundBankSelect::OnOK()
{
	SoundBank *bnk = GetFile();
	if (bnk)
	{
		fileID = bnk->name;
		bnkNum = GetBankNum();
		SBInstr *pre = GetPreset();
		if (pre)
		{
			preNum = pre->prog;
			preName = pre->instrName;
		}
	}
	doneSet = 1;
}

void SoundBankSelect::OnCancel()
{
	doneSet = -1;
}

void SoundBankSelect::OnFileChange()
{
	bnkNum = 0;
	preNum = 0;
	SwitchFile();
}

void SoundBankSelect::OnBankChange()
{
	preNum = 0;
	SwitchBank();
}

void SoundBankSelect::SwitchFile()
{
	bankList->clear();

	int bnkSel = 0;
	SoundBank *sb = GetFile();
	if (sb)
	{
		int index = 1;
		for (int n = 0; n < 129; n++)
		{
			if (sb->instrBank[n])
			{
				char num[40];
				snprintf(num, 40, "%d", n);
				bankList->add(num, (void*)n);
				if (n == bnkNum)
					bnkSel = index;
				index++;
			}
		}
		bankList->value(bnkSel);
		SwitchBank();
	}
}

void SoundBankSelect::SwitchBank()
{
	presetList->clear();

	int preSel = 0;
	SoundBank *sb = GetFile();
	if (sb)
	{
		int bank = GetBankNum();
		if (bank >= 0)
		{
			int index = 1;
			SBInstr *pre;
			for (int n = 0; n < 128; n++)
			{
				if ((pre = sb->GetInstr(bank, n, 0)) != 0)
				{
					presetList->add(pre->instrName, (void*)pre);
					if (pre->prog == preNum)
						preSel = index;
					index++;
				}
			}
			presetList->value(preSel);
		}
	}
}

SoundBank *SoundBankSelect::GetFile()
{
	int sel = fileList->value();
	const Fl_Menu_Item *menu = fileList->menu();
	if (sel >= 0)
		return (SoundBank *) menu[sel].user_data();
	return 0;
}

int SoundBankSelect::GetBankNum()
{
	int sel = bankList->value();
	if (sel > 0)
		return (int) (long) bankList->data(sel);
	return -1;
}

SBInstr *SoundBankSelect::GetPreset()
{
	int sel = presetList->value();
	if (sel > 0)
		return (SBInstr*) presetList->data(sel);
	return 0;
}

int SoundBankSelect::GetPresetNum()
{
	SBInstr *pre = GetPreset();
	if (pre)
		return pre->prog;
	return -1;
}
