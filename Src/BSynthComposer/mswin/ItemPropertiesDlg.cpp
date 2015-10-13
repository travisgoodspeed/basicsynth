//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "resource.h"
#include "ProjectItemDlg.h"
#include "ItemPropertiesDlg.h"

static int itmPropIds[] = {
	-1,
	IDC_ITEM_NAME,          // PROP_NAME
	IDC_ITEM_DESCR,         // PROP_DESC
	IDC_FILE_NAME,          // PROP_FILE
	IDC_FILE_INCLUDE,       // PROP_INCL
	IDC_INST_NUM,           // PROP_INUM
	IDC_INST_TYPE,          // PROP_ITYP
	IDC_INST_LIST,          // PROP_ILST
	IDC_WVF_ID,             // PROP_WVID
	IDC_RENUMBER,           // PROP_REN
	IDC_SB_PRELOAD,         // PROP_PRELOAD
	IDC_SB_SCALE            // PROP_SCALE
};

int ItemPropertiesBase::GetFieldID(int id, int& idval)
{
	if (id < (sizeof(itmPropIds)/sizeof(int)))
	{
		idval = itmPropIds[id];
		return 1;
	}
	return 0;
}

LRESULT ItemPropertiesBase::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	InitSpecific();

	pi->LoadProperties(static_cast<PropertyBox*>(this));

	EnableOK();

	return 1;
}

LRESULT ItemPropertiesBase::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (pi->SaveProperties(static_cast<PropertyBox*>(this)))
		EndDialog(1);
	return 0;
}

LRESULT ItemPropertiesBase::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

LRESULT ItemPropertiesBase::OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needName)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnFileNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needFile)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnNumChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (needInum)
		EnableOK();
	return 0;
}

LRESULT ItemPropertiesBase::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	const char *spc = ProjectItem::GetFileSpec(pi->GetType());
	const char *ext = ProjectItem::GetFileExt(pi->GetType());
	char path[MAX_PATH];
	path[0] = '\0';
//	GetDlgItemText(IDC_FILE_NAME, path, MAX_PATH);
	GetTextUTF8(IDC_FILE_NAME, path, MAX_PATH);

	if (prjFrame->BrowseFile(1, path, spc, ext))
	{
		//SetDlgItemText(IDC_FILE_NAME, SynthProject::SkipProjectDir(path));
		SetTextUTF8(IDC_FILE_NAME, SynthProject::SkipProjectDir(path));
		if (needFile)
			EnableOK();
	}

	return 0;
}

void ItemPropertiesBase::EnableOK()
{
	HWND w;
	int enable = 1;
	if (needName)
	{
		if ((w = GetDlgItem(IDC_ITEM_NAME)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	if (needFile)
	{
		if ((w = GetDlgItem(IDC_FILE_NAME)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	if (needInum)
	{
		if ((w = GetDlgItem(IDC_INST_NUM)) == 0 || ::GetWindowTextLength(w) < 1)
			enable = 0;
	}
	::EnableWindow(GetDlgItem(IDOK), enable);
}

void InstrPropertiesDlg::InitSpecific()
{
	CListBox typeList = GetDlgItem(IDC_INST_TYPE);

	InstrMapEntry *im = 0;
	while ((im = theProject->mgr.EnumType(im)) != 0)
	{
		int ndx = typeList.AddString(im->GetType());
		typeList.SetItemDataPtr(ndx, im);
	}
}

void LibPropertiesDlg::InitSpecific()
{
}

////////////////////////////////////////////////////////////////////////////

int InstrSelectDlg::GetFieldID(int id, int& idval)
{
	if (id == PROP_ILST || id == IDC_INST_LIST)
	{
		idval = IDC_INST_LIST;
		return 1;
	}
	if (id == PROP_INUM || id == IDC_INST_NUM)
	{
		idval = IDC_INST_NUM;
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////

LRESULT FilelistOrder::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();

	CButton btn;
	btn = GetDlgItem(IDC_FILE_UP);
	RECT rc;
	btn.GetClientRect(&rc);
//	int cx = 16;
//	if (rc.right >= 32)
//		cx = 32;
	int cx = 32;

	HICON mvUp = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVUP), IMAGE_ICON, cx, cx, LR_SHARED);
	HICON mvDn = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MVDN), IMAGE_ICON, cx, cx, LR_SHARED);

	btn.SetIcon(mvUp);
	btn = GetDlgItem(IDC_FILE_DN);
	btn.SetIcon(mvDn);

	fileList = GetDlgItem(IDC_FILE_LIST);

	ProjectItem *ch = prjTree->FirstChild(pi);
	while (ch)
	{
		//int ndx = fileList.AddString(ch->GetName());
		int ndx = AddLBTextUTF8(fileList, ch->GetName());
		fileList.SetItemDataPtr(ndx, (void*)ch);
		ch = prjTree->NextSibling(ch);
	}

	fileList.SetCurSel(0);
	EnableUpDn();

	return 1;
}

LRESULT FilelistOrder::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ProjectItem *itm;
	int count = fileList.GetCount();
	int index = 0;
	while (index < count)
	{
		itm = (ProjectItem *) fileList.GetItemDataPtr(index);
		itm->AddRef();
		prjTree->RemoveNode(itm);
		prjTree->AddNode(itm);
		itm->Release();
		index++;
	}
	theProject->SetChange(1);
	EndDialog(1);
	return 0;
}

LRESULT FilelistOrder::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

LRESULT FilelistOrder::OnFileMvup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fileList.GetCurSel();
	if (sel == LB_ERR || sel == 0)
		return 0;
	
	ProjectItem *p = (ProjectItem *)fileList.GetItemDataPtr(sel);
	fileList.DeleteString(sel);
	sel--;
	fileList.InsertString(sel, p->GetName());
	fileList.SetItemDataPtr(sel, (void*)p);
	fileList.SetCurSel(sel);

	EnableUpDn();

	return 0;
}

LRESULT FilelistOrder::OnFileMvdn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int sel = fileList.GetCurSel();
	if (sel == LB_ERR && sel < fileList.GetCount())
		return 0;

	ProjectItem *p = (ProjectItem *) fileList.GetItemDataPtr(sel);
	fileList.DeleteString(sel);
	sel++;
	fileList.InsertString(sel, p->GetName());
	fileList.SetItemDataPtr(sel, (void*)p);
	fileList.SetCurSel(sel);

	EnableUpDn();

	return 0;
}


void FilelistOrder::EnableUpDn()
{
	int count = fileList.GetCount();
	int mvUp = count > 1;
	int mvDn = count > 1;
	int sel = fileList.GetCurSel();
	if (sel == LB_ERR)
	{
		mvUp = FALSE;
		mvDn = FALSE;
	}
	else if (sel == 0)
		mvUp = FALSE;
	else if (sel == count-1)
		mvDn = FALSE;
	::EnableWindow(GetDlgItem(IDC_FILE_UP), mvUp);
	::EnableWindow(GetDlgItem(IDC_FILE_DN), mvDn);
}

LRESULT FilelistOrder::OnSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EnableUpDn();
	return 0;
}

