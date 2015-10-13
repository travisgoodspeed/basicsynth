//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"

/////////////////////////////////////////////////////////////////////////////
OptionsDlg::OptionsDlg()
{
}

LRESULT OptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	CheckDlgButton(IDC_INCL_NOTELIST, prjOptions.inclNotelist);
	CheckDlgButton(IDC_INCL_SCRIPTS, prjOptions.inclScripts);
	CheckDlgButton(IDC_INCL_SEQUENCE, prjOptions.inclSequence);
	CheckDlgButton(IDC_INCL_TEXTFILES, prjOptions.inclTextFiles);
	CheckDlgButton(IDC_INCL_LIBRARIES, prjOptions.inclLibraries);
	CheckDlgButton(IDC_INCL_SOUNDFONTS, prjOptions.inclSoundFonts);
	CheckDlgButton(IDC_INCL_MIDI, prjOptions.inclMIDI);
	SetItemUTF8(IDC_DEF_PROJECTS, prjOptions.defPrjDir);
	SetItemUTF8(IDC_DEF_WAVEIN, prjOptions.defWaveIn);
	SetItemUTF8(IDC_DEF_FORMS, prjOptions.formsDir);
	SetItemUTF8(IDC_DEF_COLORS, prjOptions.colorsFile);
	SetItemUTF8(IDC_DEF_LIBRARIES, prjOptions.defLibDir);
	SetItemUTF8(IDC_DEF_AUTHOR, prjOptions.defAuthor);
	SetItemUTF8(IDC_DEF_COPYRIGHT, prjOptions.defCopyright);

	bsString lat((double)prjOptions.playBuf);
	SetDlgItemText(IDC_LATENCY, lat);

	SoundDevInfo *sdi = NULL;
	waveDev = GetDlgItem(IDC_WAVE_OUT);
	while ((sdi = prjOptions.waveList.EnumItem(sdi)) != NULL)
		waveDev.AddString(sdi->name);
	if (prjOptions.waveDevice[0])
		waveDev.SelectString(-1, prjOptions.waveDevice);
	else
		waveDev.SetCurSel(0);

	midiDev = GetDlgItem(IDC_MIDI_IN);
	sdi = NULL;
	while ((sdi = prjOptions.midiList.EnumItem(sdi)) != NULL)
		midiDev.AddString(sdi->name);
	if (prjOptions.midiDeviceName[0])
		midiDev.SelectString(-1, prjOptions.midiDeviceName);
	else
		midiDev.SetCurSel(0);

	return 1;
}

void OptionsDlg::Browse(int id, char *caption)
{
	wchar_t wcap[MAX_PATH];
	bsString::utf16(caption, wcap, MAX_PATH);

	wchar_t name[MAX_PATH];
	name[0] = 0;
	::GetDlgItemTextW(m_hWnd, id, name, MAX_PATH);
	//GetItemUTF8(id, name, MAX_PATH);

	BROWSEINFOW bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = wcap;
    bi.ulFlags = BIF_USENEWUI;

//	PIDLIST_ABSOLUTE pidl;
	LPCITEMIDLIST pidl;
	if ((pidl = SHBrowseForFolderW(&bi)) != NULL)
	{
		SHGetPathFromIDListW(pidl, name);
		::SetDlgItemTextW(m_hWnd, id, name);
		IMalloc *mp;
		SHGetMalloc(&mp);
		mp->Free((void*)pidl);
		mp->Release();
	}
}

LRESULT OptionsDlg::OnBrowseProjects(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_PROJECTS, "Default Project Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseForms(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_FORMS, "Forms Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseColors(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char name[MAX_PATH];
	GetItemUTF8(IDC_DEF_COLORS, name, MAX_PATH);
	if (prjFrame->BrowseFile(1, name, "XML Files|*.xml|", "xml"))
		SetItemUTF8(IDC_DEF_COLORS, name);
	return 0;
}

LRESULT OptionsDlg::OnBrowseLibraries(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_LIBRARIES, "Library Folder");
	return 0;
}

LRESULT OptionsDlg::OnBrowseWaveIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Browse(IDC_DEF_WAVEIN, "Wave Files Folder");
	return 0;
}

LRESULT OptionsDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	prjOptions.inclNotelist = IsDlgButtonChecked(IDC_INCL_NOTELIST);
	prjOptions.inclScripts = IsDlgButtonChecked(IDC_INCL_SCRIPTS);
	prjOptions.inclSequence = IsDlgButtonChecked(IDC_INCL_SEQUENCE);
	prjOptions.inclTextFiles = IsDlgButtonChecked(IDC_INCL_TEXTFILES);
	prjOptions.inclLibraries = IsDlgButtonChecked(IDC_INCL_LIBRARIES);
	prjOptions.inclSoundFonts = IsDlgButtonChecked(IDC_INCL_SOUNDFONTS);
	prjOptions.inclMIDI = IsDlgButtonChecked(IDC_INCL_MIDI);
	GetItemUTF8(IDC_DEF_PROJECTS, prjOptions.defPrjDir, MAX_PATH);
	GetItemUTF8(IDC_DEF_WAVEIN, prjOptions.defWaveIn, MAX_PATH);
	GetItemUTF8(IDC_DEF_FORMS, prjOptions.formsDir, MAX_PATH);
	GetItemUTF8(IDC_DEF_COLORS, prjOptions.colorsFile, MAX_PATH);
	GetItemUTF8(IDC_DEF_LIBRARIES, prjOptions.defLibDir, MAX_PATH);
	GetItemUTF8(IDC_DEF_AUTHOR, prjOptions.defAuthor, MAX_PATH);
	GetItemUTF8(IDC_DEF_COPYRIGHT, prjOptions.defCopyright, MAX_PATH);

	char buf[40];
	buf[0] = '\0';
	GetDlgItemText(IDC_LATENCY, buf, 40);
	prjOptions.playBuf = (float) bsString::StrToFlp(buf);

	prjOptions.midiDevice = SendDlgItemMessage(IDC_MIDI_IN, CB_GETCURSEL);
	if (prjOptions.midiDevice == CB_ERR)
		memset(prjOptions.midiDeviceName, 0, MAX_PATH);
	else
		GetItemUTF8(IDC_MIDI_IN, prjOptions.midiDeviceName, MAX_PATH);
	if (theProject)
		theProject->prjMidiIn.SetDevice(prjOptions.midiDevice, prjOptions.midiDeviceName);

	waveDev.GetWindowText(prjOptions.waveDevice, MAX_PATH);

	prjOptions.Save();

	EndDialog(1);
	return 0;
}

LRESULT OptionsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

void OptionsDlg::GetItemUTF8(int id, char *cbuf, int clen)
{
	utf8Window w = GetDlgItem(id);
	w.GetTextUTF8(cbuf, clen);
//	memset(cbuf, 0, clen);
//	wchar_t wbuf[MAX_PATH];
//	memset(wbuf, 0, sizeof(wbuf));
//	::GetWindowTextW(GetDlgItem(id), wbuf, MAX_PATH);
//	::WideCharToMultiByte(CP_UTF8, 0, wbuf, wcslen(wbuf)+1, cbuf, clen, NULL, NULL);
}

void OptionsDlg::SetItemUTF8(int id, const char *str)
{
	utf8Window w = GetDlgItem(id);
	w.SetTextUTF8(str);
//	int slen = (int) strlen(str);
//	wchar_t wstr[MAX_PATH];
//	memset(wstr, 0, sizeof(wstr));
//	::MultiByteToWideChar(CP_UTF8, 0, str, slen+1, wstr, MAX_PATH);
//	::SetWindowTextW(GetDlgItem(id), wstr);
}
