//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _ITEMPROPERTIES_DLG_H_
#define _ITEMPROPERTIES_DLG_H_

#pragma once

/// @brief Base class for item property pages.
/// @details This class implements generic property page
/// functions. Derived classes specify the actual dialog
/// resource information and any dialog specific overrides.
class ItemPropertiesBase : 
	public wxDialog,
	public ProjectItemDlg<ItemPropertiesBase>
{
protected:
    DECLARE_EVENT_TABLE()

	int needName;
	int needFile;
	int needInum;

	void EnableOK();

	void Load(wxWindow *parent, const char *id)
	{
		wxXmlResource::Get()->LoadDialog(this, parent, id);
	}

public:
	ItemPropertiesBase() 
	{
		needName = 1;
		needFile = 0;
		needInum = 0;
	}

	virtual ~ItemPropertiesBase() {}

	virtual void InitSpecific() { }
	virtual int GetFieldID(int id, wxWindow **child);

	void OnInitDialog(wxInitDialogEvent&);
	void OnOK(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnBrowse(wxCommandEvent&);
	void OnNameChange(wxCommandEvent&);
	void OnFileNameChange(wxCommandEvent&);
	void OnNumChange(wxCommandEvent&);
};

class NamePropertiesDlg : public ItemPropertiesBase
{
public:
	NamePropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_NAME_PROPERTIES");
	}
};

class FilePropertiesDlg : public ItemPropertiesBase
{
public:
	FilePropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		needFile = 1;
		Load(parent, "DLG_FILE_PROPERTIES");
	}
};

class InstrPropertiesDlg : public ItemPropertiesBase
{
public:
	InstrPropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_INSTR_PROPERTIES");
		needInum = 1;
	}
	virtual void InitSpecific();

};

class LibPropertiesDlg : public ItemPropertiesBase
{
public:
	LibPropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_LIB_PROPERTIES");
		needFile = 1;
	}
	virtual void InitSpecific();
};

class WavefilePropertiesDlg : public ItemPropertiesBase
{
public:
	WavefilePropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_WVF_PROPERTIES");
		needFile = 1;
	}

};

class SoundBankPropertiesDlg : public ItemPropertiesBase
{
public:
	SoundBankPropertiesDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_SOUNDBANK_PROPERTIES");
		needFile = 1;
	}
};

class InstrSelectDlg : public ItemPropertiesBase
{
private:

public:
	InstrSelectDlg(wxWindow *parent, ProjectItem *pi)
	{
		SetItem(pi);
		Load(parent, "DLG_SELECT_INSTR");
		needName = 0;
	}

	virtual int GetFieldID(int id, wxWindow **child);
};


class FilelistOrder : 
	public wxDialog,
	public ProjectItemDlg<FilelistOrder>
{
protected:
    DECLARE_EVENT_TABLE()
	wxListBox *fileList;

	void EnableUpDn();

public:
	FilelistOrder(wxWindow *parent, ProjectItem *pi) 
	{
		SetItem(pi);
		wxXmlResource::Get()->LoadDialog(this, parent, "DLG_FILE_ORDER");
		fileList = (wxListBox*)FindWindow("IDC_FILE_LIST");
	}

	void OnInitDialog(wxInitDialogEvent& evt);
	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
	void OnFileMvup(wxCommandEvent& evt);
	void OnFileMvdn(wxCommandEvent& evt);
	void OnSelect(wxCommandEvent& evt);
};

#endif
