//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "ProjectItemDlg.h"
#include "ItemPropertiesDlg.h"

BEGIN_EVENT_TABLE(ItemPropertiesBase,wxDialog)
	EVT_BUTTON(wxID_OK, ItemPropertiesBase::OnOK)
	EVT_BUTTON(wxID_CANCEL, ItemPropertiesBase::OnCancel)
	EVT_TEXT(XRCID("IDC_ITEM_NAME"), ItemPropertiesBase::OnNameChange)
	EVT_TEXT(XRCID("IDC_FILE_NAME"), ItemPropertiesBase::OnFileNameChange)
	EVT_TEXT(XRCID("IDC_INST_NUM"), ItemPropertiesBase::OnNumChange)
	EVT_BUTTON(XRCID("IDC_FILE_BROWSE"), ItemPropertiesBase::OnBrowse)
	EVT_INIT_DIALOG(ItemPropertiesBase::OnInitDialog)
END_EVENT_TABLE()

static const char *itmPropIds[] =
{
	"NONE",
	"IDC_ITEM_NAME",          // PROP_NAME
	"IDC_ITEM_DESCR",         // PROP_DESC
	"IDC_FILE_NAME",          // PROP_FILE
	"IDC_FILE_INCLUDE",       // PROP_INCL
	"IDC_INST_NUM",           // PROP_INUM
	"IDC_INST_TYPE",          // PROP_ITYP
	"IDC_INST_LIST",          // PROP_ILST
	"IDC_WVF_ID",             // PROP_WVID
	"IDC_RENUMBER",           // PROP_REN
	"IDC_SB_PRELOAD",         // PROP_PRELOAD
	"IDC_SB_SCALE"            // PROP_SCALE
};

int ItemPropertiesBase::GetFieldID(int id, wxWindow **child)
{
	if (id < (int)(sizeof(itmPropIds)/sizeof(char*)))
		*child = FindWindow(itmPropIds[id]);
	else
		*child = 0;
	if (*child)
		return 1;
	return 0;
}

void ItemPropertiesBase::OnInitDialog(wxInitDialogEvent&)
{
	CenterOnParent();

	InitSpecific();

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	EnableOK();
}

void ItemPropertiesBase::OnOK(wxCommandEvent&)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndModal(1);
}

void ItemPropertiesBase::OnCancel(wxCommandEvent&)
{
	EndModal(0);
}

void ItemPropertiesBase::OnNameChange(wxCommandEvent&)
{
	if (needName)
		EnableOK();
}

void ItemPropertiesBase::OnFileNameChange(wxCommandEvent&)
{
	if (needFile)
		EnableOK();
}

void ItemPropertiesBase::OnNumChange(wxCommandEvent&)
{
	if (needInum)
		EnableOK();
}

void ItemPropertiesBase::OnBrowse(wxCommandEvent&)
{
	const char *spc = ProjectItem::GetFileSpec(pi->GetType());
	const char *ext = ProjectItem::GetFileExt(pi->GetType());

	char path[MAX_PATH];
	path[0] = '\0';
	GetValue(PROP_FILE, path, MAX_PATH);

	if (prjFrame->BrowseFile(0, path, spc, ext))
	{
		SetValue(PROP_FILE, path, NULL);
		if (needFile)
			EnableOK();
	}
}

void ItemPropertiesBase::EnableOK()
{
	wxWindow *w;
	int enable = 1;
	if (needName)
	{
		if (GetValueLength(PROP_NAME) <= 0)
			enable = 0;
	}
	if (needFile)
	{
		if (GetValueLength(PROP_FILE) <= 0)
			enable = 0;
	}
	if (needInum)
	{
		if (GetValueLength(PROP_INUM) <= 0)
			enable = 0;
	}
	w = FindWindow(wxID_OK);
	w->Enable(enable);
}

void InstrPropertiesDlg::InitSpecific()
{
	wxListBox *typeList = (wxListBox*)FindWindow("IDC_INST_TYPE");

	InstrMapEntry *im = 0;
	while ((im = theProject->mgr.EnumType(im)) != 0)
		typeList->Append(im->GetType(), im);
}

void LibPropertiesDlg::InitSpecific()
{
}

////////////////////////////////////////////////////////////////////////////

int InstrSelectDlg::GetFieldID(int id, wxWindow **child)
{
	if (id == PROP_ILST)
	{
		*child = FindWindow("IDC_INST_LIST");
		return 1;
	}
	if (id == PROP_INUM)
	{
		*child = FindWindow("IDC_INST_NUM");
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(FilelistOrder,wxDialog)
	EVT_BUTTON(wxID_OK, FilelistOrder::OnOK)
	EVT_BUTTON(wxID_CANCEL, FilelistOrder::OnCancel)
	EVT_BUTTON(XRCID("IDC_FILE_UP"), FilelistOrder::OnFileMvup)
	EVT_BUTTON(XRCID("IDC_FILE_DN"), FilelistOrder::OnFileMvdn)
	EVT_LISTBOX(XRCID("IDC_FILE_LIST"), FilelistOrder::OnSelect)
	EVT_INIT_DIALOG(FilelistOrder::OnInitDialog)
END_EVENT_TABLE()

void FilelistOrder::OnInitDialog(wxInitDialogEvent& evt)
{
	CenterOnParent();

	if (pi)
	{
		ProjectItem *ch = prjTree->FirstChild(pi);
		while (ch)
		{
			fileList->Append(ch->GetName(), reinterpret_cast<void*>(ch));
			ch = prjTree->NextSibling(ch);
		}
	}

	if (fileList->GetCount() > 0)
		fileList->SetSelection(0);

	EnableUpDn();
}

void FilelistOrder::OnOK(wxCommandEvent& evt)
{
	ProjectItem *itm;
	int count = (int)fileList->GetCount();
	int index = 0;
	while (index < count)
	{
		itm = (ProjectItem *) fileList->GetClientData(index);
		itm->AddRef();
		prjTree->RemoveNode(itm);
		prjTree->AddNode(itm);
		itm->Release();
		index++;
	}
	theProject->SetChange(1);
	EndModal(1);
}

void FilelistOrder::OnCancel(wxCommandEvent& evt)
{
	EndModal(0);
}

void FilelistOrder::OnFileMvup(wxCommandEvent& evt)
{
	int sel = fileList->GetSelection();
	if (sel == wxNOT_FOUND || sel == 0)
		return;

	ProjectItem *p = (ProjectItem *)fileList->GetClientData(sel);
	fileList->Delete(sel);
	sel--;
	fileList->Insert(p->GetName(), (unsigned int)sel, reinterpret_cast<void*>(p));
	fileList->SetSelection(sel);

	EnableUpDn();
}

void FilelistOrder::OnFileMvdn(wxCommandEvent& evt)
{
	int sel = fileList->GetSelection();
	if (sel == wxNOT_FOUND)
		return;

	ProjectItem *p = (ProjectItem *)fileList->GetClientData(sel);
	fileList->Delete(sel);
	sel++;
	fileList->Insert(p->GetName(), (unsigned int)sel, reinterpret_cast<void*>(p));
	fileList->SetSelection(sel);

	EnableUpDn();
}

void FilelistOrder::OnSelect(wxCommandEvent& evt)
{
	EnableUpDn();
}

void FilelistOrder::EnableUpDn()
{
	int count = fileList->GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = fileList->GetSelection();
	if (sel == wxNOT_FOUND)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	FindWindow("IDC_FILE_UP")->Enable(mvUp);
	FindWindow("IDC_FILE_DN")->Enable(mvDn);
}
