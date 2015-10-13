//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "ProjectItemDlg.h"
#include "ProjectPropertiesDlg.h"

BEGIN_EVENT_TABLE(ProjectPropertiesDlg, wxDialog)
	EVT_BUTTON(wxID_OK, ProjectPropertiesDlg::OnOK)
	EVT_BUTTON(XRCID("IDC_PROJECT_IN_BROWSE"), ProjectPropertiesDlg::OnBrowseIn)
	EVT_BUTTON(XRCID("IDC_PROJECT_OUT_BROWSE"), ProjectPropertiesDlg::OnBrowseOut)
	EVT_BUTTON(XRCID("IDC_PROJECT_WV_BROWSE"), ProjectPropertiesDlg::OnBrowseWv)
	EVT_TEXT(XRCID("IDC_PROJECT_NAME"), ProjectPropertiesDlg::OnNameChange)
	EVT_TEXT(XRCID("IDC_PROJECT_FILE"), ProjectPropertiesDlg::OnNameChange)
	EVT_BUTTON(XRCID("IDC_PROJECT_PATH_ADD"), ProjectPropertiesDlg::OnPathAdd)
	EVT_BUTTON(XRCID("IDC_PROJECT_PATH_REM"), ProjectPropertiesDlg::OnPathRem)
	EVT_BUTTON(XRCID("IDC_PROJECT_PATH_UP"), ProjectPropertiesDlg::OnPathMvup)
	EVT_BUTTON(XRCID("IDC_PROJECT_PATH_DN"), ProjectPropertiesDlg::OnPathMvdn)
	EVT_LISTBOX(XRCID("IDC_PROJECT_PATH"), ProjectPropertiesDlg::OnPathSelect)
	EVT_COMMAND_KILL_FOCUS(XRCID("IDC_PROJECT_FILE"), ProjectPropertiesDlg::OnFileNameBlur)
END_EVENT_TABLE()

ProjectPropertiesDlg::ProjectPropertiesDlg(wxWindow *parent, ProjectItem *p)
	: ProjectItemDlg<ProjectPropertiesDlg>(parent, p, "DLG_PROJECT_PROPERTIES")
{

	prjNameWnd = (wxTextCtrl*)FindWindow("IDC_PROJECT_NAME");
	prjFileWnd = (wxTextCtrl*)FindWindow("IDC_PROJECT_FILE");
	wavFileWnd = (wxTextCtrl*)FindWindow("IDC_PROJECT_OUTPUT");
	wavPathWnd = (wxTextCtrl*)FindWindow("IDC_PROJECT_WVPATH");

	wxComboBox *cbRate = (wxComboBox*)FindWindow("IDC_PROJECT_SAMPLERATE");
	cbRate->Append("22050");
	cbRate->Append("44100");
	cbRate->Append("48000");
	cbRate->Append("96000");

	wxComboBox *cbFmt = (wxComboBox*)FindWindow("IDC_PROJECT_SAMPLEFMT");
	cbFmt->Append("16-bit PCM");
	cbFmt->Append("32-bit Float");

	pathList = (wxListBox*)FindWindow("IDC_PROJECT_PATH");
	// Fill in path list...
	PathListItem *pl = theProject->libPath->EnumList(0);
	while (pl)
	{
		pathList->Append(wxString((const char *)pl->path));
		pl = theProject->libPath->EnumList(pl);
	}

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	AutoWavefile();
	EnableUpDn();
	EnableOK();
}

ProjectPropertiesDlg::~ProjectPropertiesDlg(void)
{
}


static const char *prjPropIds[] = { 0,
	"IDC_PROJECT_NAME", "IDC_PROJECT_COMPOSER", "IDC_PROJECT_COPYRIGHT", "IDC_PROJECT_DESCR",
	"IDC_PROJECT_FILE", "IDC_PROJECT_OUTPUT", "IDC_PROJECT_LEAD", "IDC_PROJECT_TAIL",
	"IDC_PROJECT_SAMPLERATE", "IDC_PROJECT_SAMPLEFMT", "IDC_PROJECT_WTSIZE", "IDC_PROJECT_WTUSER",
	"IDC_PROJECT_WVPATH" };

int ProjectPropertiesDlg::GetFieldID(int id, wxWindow **child)
{
	if (id < (int)(sizeof(prjPropIds)/sizeof(prjPropIds[0])))
	{
		if ((*child = FindWindow(prjPropIds[id])) != 0)
			return 1;
	}
	return 0;
}

void ProjectPropertiesDlg::OnOK(wxCommandEvent& evt)
{
	if (!pi->SaveProperties(static_cast<PropertyBox*>(this)))
		return;

	theProject->libPath->RemoveAll();

	unsigned int lbCount = pathList->GetCount();
	unsigned int lbIndex;
	for (lbIndex = 0; lbIndex < lbCount; lbIndex++)
	{
		theProject->libPath->AddItem(pathList->GetString(lbIndex));
	}

	EndModal(1);
}

void ProjectPropertiesDlg::OnNameChange(wxCommandEvent& evt)
{
	EnableOK();
}

void ProjectPropertiesDlg::OnFileNameBlur(wxCommandEvent& evt)
{
	AutoWavefile();
	EnableOK();
}

void ProjectPropertiesDlg::OnPathSelect(wxCommandEvent& evt)
{
	EnableUpDn();
}

void ProjectPropertiesDlg::OnPathMvup(wxCommandEvent& evt)
{
	int sel = pathList->GetSelection();
	if (sel == wxNOT_FOUND || sel == 0)
		return;
	wxString file(pathList->GetString(sel));
	pathList->Delete(sel);
	sel--;
	pathList->Insert(file, (unsigned int)sel);
	pathList->SetSelection(sel);

	EnableUpDn();
}

void ProjectPropertiesDlg::OnPathMvdn(wxCommandEvent& evt)
{
	int sel = pathList->GetSelection();
	if (sel == wxNOT_FOUND)
		return;
	wxString file(pathList->GetString(sel));
	pathList->Delete(sel);
	sel++;
	pathList->Insert(file, (unsigned int)sel);
	pathList->SetSelection(sel);
	EnableUpDn();
}

void ProjectPropertiesDlg::OnPathAdd(wxCommandEvent& evt)
{
	wxString path = wxDirSelector("Select the folder for project files", "", 0, wxDefaultPosition, this);
	if (!path.IsEmpty())
		pathList->Append(path);
	EnableUpDn();
}

void ProjectPropertiesDlg::OnPathRem(wxCommandEvent& evt)
{
	int sel = pathList->GetSelection();
	if (sel != wxNOT_FOUND)
		pathList->Delete(sel);

	EnableUpDn();
}

void ProjectPropertiesDlg::OnBrowseOut(wxCommandEvent& evt)
{
	const char *spc = ProjectItem::GetFileSpec(PRJNODE_WVFILE);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_WVFILE);
	char path[MAX_PATH];
	strncpy(path, wavFileWnd->GetValue().mb_str(*wxConvUI), MAX_PATH);
	if (prjFrame->BrowseFile(0, path, spc, ext))
		wavFileWnd->SetValue(SynthProject::SkipProjectDir(path));
}

void ProjectPropertiesDlg::OnBrowseIn(wxCommandEvent& evt)
{
	const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
	char path[MAX_PATH];
	strncpy(path, prjFileWnd->GetValue().mb_str(*wxConvUI), MAX_PATH);
	if (prjFrame->BrowseFile(0, path, spc, ext))
		prjFileWnd->SetValue(path);
}


void ProjectPropertiesDlg::OnBrowseWv(wxCommandEvent& evt)
{
	wxString path = wxDirSelector("Select the folder for WAV files",
		                          wavPathWnd->GetValue(), 0, wxDefaultPosition,
								  this);
	if (path.size() > 0)
		wavPathWnd->SetValue(path);
}


void ProjectPropertiesDlg::EnableOK()
{
	int len1 = prjNameWnd->GetValue().Length();
	int len2 = prjFileWnd->GetValue().Length();
	FindWindow(wxID_OK)->Enable(len1 > 0 && len2 > 0);
}


void ProjectPropertiesDlg::EnableUpDn()
{
	int count = pathList->GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = pathList->GetSelection();
	if (sel == wxNOT_FOUND)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	FindWindow("IDC_PROJECT_PATH_UP")->Enable(mvUp);
	FindWindow("IDC_PROJECT_PATH_DN")->Enable(mvDn);
}

void ProjectPropertiesDlg::AutoWavefile()
{
	wxString txt = wavFileWnd->GetValue();
	if (txt.size() > 0)
		return;

	txt = prjFileWnd->GetValue();
	if (txt.size() > 0)
	{
		wxString name = txt.AfterLast('/').BeforeLast('.');
		name += ".wav";
		wavFileWnd->SetLabel(name);
	}
}

