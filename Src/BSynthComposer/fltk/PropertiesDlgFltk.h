//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Item Properties dialog class declarations.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef PROPERTIES_DLG_H
#define PROPERTIES_DLG_H

class ItemPropertiesBase;

struct BrowseArg
{
	Fl_Input *wdg;
	ItemPropertiesBase *dlg;
	const char *typ;
};

class ItemPropertiesBase :
	public Fl_Window,
	public PropertyBox
{
protected:
	int doneSet;
	ProjectItem *pi;
	Fl_Button *okBtn;
	Fl_Button *canBtn;
	Fl_Input *nameInp;
	Fl_Input *descInp;
	int txtHeight;
	int txtSpace;

	int AddNameDesc(int ypos);
	int AddOkCancel(int ypos);

public:
	ItemPropertiesBase(ProjectItem *p, int width, int height);
	~ItemPropertiesBase();

	ProjectItem *GetItem()
	{
		return pi;
	}

	void SetItem(ProjectItem *p)
	{
		pi = p;
	}

	void OnOK();
	void OnCancel();
	void OnBrowse(Fl_Input *wdg, const char *typ);
	
	virtual void EnableValue(int id, int enable);
	virtual void EnableValue(Fl_Widget *wdg, int enable);
	virtual void SetCaption(const char *text);

	virtual void SetState(int id, short onoff);
	virtual void SetState(Fl_Button *wdg, short onoff);

	virtual int GetState(int id, short& onoff);
	virtual int GetState(Fl_Button *wdg, short& onoff);
	
	virtual void SetValue(int id, float val, const char *lbl);
	virtual void SetValue(int id, long val, const char *lbl);
	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual void SetValue(Fl_Input *wdg, const char *val, const char *lbl);

	virtual int GetValue(int id, float& val);
	virtual int GetValue(int id, long& val);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);
	virtual int GetValue(Fl_Input*, char *val, int len);
	virtual int GetValue(Fl_Input*, bsString& val);
//	virtual int GetValue(Fl_Choice*, char *val, int len);
//	virtual int GetValue(Fl_Choice*, bsString& val);

	virtual int GetSelection(int id, short& sel);
	virtual int GetSelection(Fl_Hold_Browser *wdg, short& sel);
	virtual int GetSelection(Fl_Choice *wdg, short& sel);

	virtual void SetSelection(int id, short sel);
	virtual void SetSelection(Fl_Hold_Browser *wdg, short sel);
	virtual void SetSelection(Fl_Choice *wdg, short sel);

	virtual int GetSelection(int id, void **sel);
	virtual int GetSelection(Fl_Hold_Browser *wdg, void **sel);
	virtual int GetSelection(Fl_Choice *wdg, void **sel);

	virtual void SetSelection(int id, void *sel);
	virtual void SetSelection(Fl_Hold_Browser *wdg, void *sel);
	virtual void SetSelection(Fl_Choice *wdg, void *sel);

	virtual int ListChildren(int id, ProjectItem *parent);
	virtual int ListChildren(Fl_Hold_Browser *wdg, ProjectItem *parent);
	virtual int ListChildren(Fl_Choice *wdg, ProjectItem *parent);
	virtual int GetListCount(int id, int& count);
	virtual int GetListCount(Fl_Hold_Browser *wdg, int& count);
	virtual int GetListCount(Fl_Choice *wdg, int& count);
	virtual ProjectItem *GetListItem(int id, int ndx);
	virtual ProjectItem *GetListItem(Fl_Hold_Browser *wdg, int ndx);

	virtual int Activate(int modal);
};

class NamePropertiesDlg : public ItemPropertiesBase
{
public:
	NamePropertiesDlg(ProjectItem *p);
};

class FilePropertiesDlg : public ItemPropertiesBase
{
private:
	Fl_Input *fileInp;
	Fl_Button *fileBrowse;
	Fl_Check_Button *fileIncl;
	BrowseArg fileArg;
public:
	FilePropertiesDlg(ProjectItem *pi);

	virtual void EnableValue(int id, int enable);
	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);

	virtual void SetState(int id, short onoff);
	virtual int GetState(int id, short& onoff);
};

class InstrPropertiesDlg : public ItemPropertiesBase
{
private:
	Fl_Input  *instrNum;
	Fl_Hold_Browser *instrType;
public:
	InstrPropertiesDlg(ProjectItem *p);

	virtual void EnableValue(int id, int enable);
	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);
	virtual int GetSelection(int id, void **sel);
	virtual void SetSelection(int id, void *sel);
};

class WavefilePropertiesDlg : public ItemPropertiesBase
{
private:
	Fl_Input *fileInp;
	Fl_Button *fileBrowse;
	Fl_Input *wfId;
	Fl_Input *lpStart;
	Fl_Input *lpEnd;
	BrowseArg fileArg;
public:
	WavefilePropertiesDlg(ProjectItem *p);

	virtual void EnableValue(int id, int enable);
	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);
};

class ProjectPropertiesDlg : public ItemPropertiesBase
{
private:
	Fl_Input *compInp;
	Fl_Input *cpyrInp;
	
	Fl_Input *prjfInp;
	Fl_Button *prjfSel;
	
	Fl_Input *outfInp;
	Fl_Button *outfSel;

	Fl_Choice *srtSel;
	Fl_Input *leadInp;
	Fl_Input *wtszInp;

	Fl_Choice *fmtSel;
	Fl_Input *tailInp;
	Fl_Input *wtusrInp;

	Fl_Input *wavfInp;
	Fl_Button *wavfSel;

	BrowseArg prjfArg;
	BrowseArg outfArg;
	BrowseArg wavfArg;

	Fl_Input *GetInput(int id);

public:
	ProjectPropertiesDlg();

	virtual void EnableValue(int id, int enable);
	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);
	virtual int GetSelection(int id, short& sel);
	virtual void SetSelection(int id, short sel);
};

class MixerSetupDlg : public ItemPropertiesBase
{
private:
	Fl_Input *chnlInp;
	Fl_Light_Button *linBtn;
	Fl_Light_Button *trgBtn;
	Fl_Light_Button *sqrBtn;
	Fl_Hold_Browser *fxList;
	Fl_Choice *fxTypes;
	Fl_Input *fxName;
	Fl_Button *fxAdd;
	Fl_Button *fxRem;

	Fl_Input *GetInput(int id);

public:
	MixerSetupDlg(ProjectItem *p);
	void OnAdd();
	void OnRemove();

	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual void SetState(int id, short onoff);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetState(int id, short& onoff);
	virtual int ListChildren(int id, ProjectItem *parent);
	virtual int GetListCount(int id, int& count);
	virtual ProjectItem *GetListItem(int id, int ndx);
};

class EffectsSetupDlg : public ItemPropertiesBase
{
private:
	Fl_Input *volInp;
	Fl_Input *panInp;
	Fl_Input *val1Inp;
	Fl_Input *val2Inp;
	Fl_Input *val3Inp;
	Fl_Input *val4Inp;
	Fl_Input *val5Inp;
	
	Fl_Input *GetInput(int id);

public:
	EffectsSetupDlg(ProjectItem *p);

	virtual void SetValue(int id, const char *val, const char *lbl);
	virtual int GetValue(int id, char *val, int len);
	virtual int GetValue(int id, bsString& val);
	virtual void EnableValue(int id, int enable);
};

class QueryValueDlg : public Fl_Window
{
private:
	Fl_Input *inp;
	Fl_Box   *lbl;
	Fl_Button *ok;
	Fl_Button *can;
	char *value;
	int vallen;
	int done;
public:
	QueryValueDlg();
	
	void OnOK();
	void OnCancel();
	int Activate(const char *prompt, char *val, int len);
};

#endif
