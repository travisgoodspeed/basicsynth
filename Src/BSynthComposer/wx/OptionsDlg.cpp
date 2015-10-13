//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "OptionsDlg.h"

BEGIN_EVENT_TABLE(OptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, OptionsDlg::OnOK)
	EVT_BUTTON(XRCID("IDC_BROWSE_PROJECTS"), OptionsDlg::OnBrowseProjects)
	EVT_BUTTON(XRCID("IDC_BROWSE_WAVEIN"), OptionsDlg::OnBrowseWavein)
	EVT_BUTTON(XRCID("IDC_BROWSE_FORMS"), OptionsDlg::OnBrowseForms)
	EVT_BUTTON(XRCID("IDC_BROWSE_COLORS"), OptionsDlg::OnBrowseColors)
	EVT_BUTTON(XRCID("IDC_BROWSE_LIBRARIES"), OptionsDlg::OnBrowseLibraries)
END_EVENT_TABLE()

OptionsDlg::OptionsDlg(wxWindow *parent)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_OPTIONS");
	CenterOnParent();

	CheckButton("IDC_INCL_NOTELIST", prjOptions.inclNotelist);
	CheckButton("IDC_INCL_SCRIPTS", prjOptions.inclScripts);
	CheckButton("IDC_INCL_SEQUENCE", prjOptions.inclSequence);
	CheckButton("IDC_INCL_TEXTFILES", prjOptions.inclTextFiles);
	CheckButton("IDC_INCL_LIBRARIES", prjOptions.inclLibraries);
	CheckButton("IDC_INCL_SOUNDFONTS", prjOptions.inclSoundFonts);
	CheckButton("IDC_INCL_MIDI", prjOptions.inclMIDI);
	SetItemText("IDC_DEF_PROJECTS", prjOptions.defPrjDir);
	SetItemText("IDC_DEF_WAVEIN", prjOptions.defWaveIn);
	SetItemText("IDC_DEF_FORMS", prjOptions.formsDir);
	SetItemText("IDC_DEF_COLORS", prjOptions.colorsFile);
	SetItemText("IDC_DEF_LIBRARIES", prjOptions.defLibDir);
	SetItemText("IDC_DEF_AUTHOR", prjOptions.defAuthor);
	SetItemText("IDC_DEF_COPYRIGHT", prjOptions.defCopyright);

	char buf[40];
	snprintf(buf, 40, "%f", prjOptions.playBuf);
	SetItemText("IDC_LATENCY", buf);

	wxComboBox *waveDev = (wxComboBox*)FindWindow("IDC_WAVE_OUT");
	SoundDevInfo *sdi = NULL;
	while ((sdi = prjOptions.waveList.EnumItem(sdi)) != NULL)
		waveDev->Append((const char *)sdi->name, reinterpret_cast<void*>(sdi));
	if (prjOptions.waveDevice[0])
		waveDev->SetStringSelection(prjOptions.waveDevice);
	else if (waveDev->GetCount() > 0)
		waveDev->SetSelection(0);

	wxComboBox *midiDev = (wxComboBox*)FindWindow("IDC_MIDI_IN");
	sdi = NULL;
	while ((sdi = prjOptions.midiList.EnumItem(sdi)) != NULL)
		midiDev->Append((const char *)sdi->name, reinterpret_cast<void*>(sdi));
	if (prjOptions.midiDeviceName[0])
		midiDev->SetStringSelection(prjOptions.midiDeviceName);
	else if (midiDev->GetCount() > 0)
		midiDev->SetSelection(0);

}

OptionsDlg::~OptionsDlg(void)
{
}

void OptionsDlg::OnOK(wxCommandEvent& evt)
{
	prjOptions.inclNotelist = IsButtonChecked("IDC_INCL_NOTELIST");
	prjOptions.inclScripts = IsButtonChecked("IDC_INCL_SCRIPTS");
	prjOptions.inclSequence = IsButtonChecked("IDC_INCL_SEQUENCE");
	prjOptions.inclTextFiles = IsButtonChecked("IDC_INCL_TEXTFILES");
	prjOptions.inclLibraries = IsButtonChecked("IDC_INCL_LIBRARIES");
	prjOptions.inclSoundFonts = IsButtonChecked("IDC_INCL_SOUNDFONTS");
	prjOptions.inclMIDI = IsButtonChecked("IDC_INCL_MIDI");
	GetItemText("IDC_DEF_PROJECTS", prjOptions.defPrjDir, MAX_PATH);
	GetItemText("IDC_DEF_WAVEIN", prjOptions.defWaveIn, MAX_PATH);
	GetItemText("IDC_DEF_FORMS", prjOptions.formsDir, MAX_PATH);
	GetItemText("IDC_DEF_COLORS", prjOptions.colorsFile, MAX_PATH);
	GetItemText("IDC_DEF_LIBRARIES", prjOptions.defLibDir, MAX_PATH);
	GetItemText("IDC_DEF_AUTHOR", prjOptions.defAuthor, MAX_PATH);
	GetItemText("IDC_DEF_COPYRIGHT", prjOptions.defCopyright, MAX_PATH);

	wxString lat;
	GetItemText("IDC_LATENCY", lat);
	double d = 0;
	if (lat.ToDouble(&d))
		prjOptions.playBuf = (float) d;

	SoundDevInfo *sdi = NULL;
	int ndx;

	wxComboBox *waveDev = (wxComboBox*)FindWindow("IDC_WAVE_OUT");
	ndx = waveDev->GetSelection();
	if (ndx == wxNOT_FOUND)
	{
		prjOptions.waveDevice[0] = '\0';
	}
	else
	{
		sdi = reinterpret_cast<SoundDevInfo*>(waveDev->GetClientData(ndx));
		strncpy(prjOptions.waveDevice, sdi->name, MAX_PATH);
	}

	wxComboBox *midiDev = (wxComboBox*)FindWindow("IDC_MIDI_IN");
	ndx = midiDev->GetSelection();
	if (ndx == wxNOT_FOUND)
	{
		prjOptions.midiDeviceName[0] = '\0';
		prjOptions.midiDevice = 0;
	}
	else
	{
		sdi = reinterpret_cast<SoundDevInfo*>(midiDev->GetClientData(ndx));
		strncpy(prjOptions.midiDeviceName, sdi->name, MAX_PATH);
		prjOptions.midiDevice = sdi->id;
	}

	EndModal(1);
}

void OptionsDlg::BrowseDir(const char *id, const char *prompt)
{
	wxString cur;
	GetItemText(id, cur);
	wxString dir = wxDirSelector(prompt, cur, 0, wxDefaultPosition, this);
	if (!dir.IsEmpty())
		SetItemText(id, dir);
}

void OptionsDlg::OnBrowseProjects(wxCommandEvent& evt)
{
	BrowseDir("IDC_DEF_PROJECTS", "Default Project Path");
}

void OptionsDlg::OnBrowseWavein(wxCommandEvent& evt)
{
	BrowseDir("IDC_DEF_WAVEIN", "Wave Files Folder");
}

void OptionsDlg::OnBrowseForms(wxCommandEvent& evt)
{
	BrowseDir("IDC_DEF_FORMS", "Forms Folder");
}

void OptionsDlg::OnBrowseColors(wxCommandEvent& evt)
{
	wxString frm;
	GetItemText("IDC_DEF_FORMS", frm);
	wxString clr;
	GetItemText("IDC_DEF_COLORS", clr);
	wxString file = wxFileSelector("Colors Files", frm, clr, "xml", "XML|*.xml|All Files|*.*", 0, this);
	if (!file.IsEmpty())
		SetItemText("IDC_DEF_COLORS", file);
}

void OptionsDlg::OnBrowseLibraries(wxCommandEvent& evt)
{
	BrowseDir("IDC_DEF_LIBRARIES", "Library Folder");
}

void OptionsDlg::CheckButton(const char *id, int onoff)
{
	wxCheckBox *btn = (wxCheckBox*)FindWindow(id);
	if (btn)
		btn->SetValue(onoff);
}

int OptionsDlg::IsButtonChecked(const char *id)
{
	wxCheckBox *btn = (wxCheckBox*)FindWindow(id);
	if (btn)
		return btn->GetValue();
	return 0;
}

void OptionsDlg::SetItemText(const char *id, wxString& txt)
{
	wxWindow *tw = FindWindow(id);
	if (tw)
	{
		if (tw->IsKindOf(CLASSINFO(wxTextCtrl)))
			((wxTextCtrl*)tw)->ChangeValue(txt);
		else
			((wxControl*)tw)->SetLabel(txt);
	}
}

void OptionsDlg::SetItemText(const char *id, const char *txt)
{
	wxMBConvUTF8 conv;
	wxString t(txt, conv);
	SetItemText(id, t);
}

void OptionsDlg::GetItemText(const char *id, wxString& txt)
{
	wxWindow *tw = FindWindow(id);
	if (tw)
	{
		if (tw->IsKindOf(CLASSINFO(wxTextCtrl)))
			txt = ((wxTextCtrl*)tw)->GetValue();
		else
			txt = ((wxControl*)tw)->GetLabel();
	}
}

void OptionsDlg::GetItemText(const char *id, char *txt, int len)
{
	wxString wval;
	GetItemText(id, wval);
	bsString::utf8(wval.wc_str(), txt, len);
}
