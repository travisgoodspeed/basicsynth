//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file PropertiesDlgFltk.cpp Properties dialogs implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "PropertiesDlgFltk.h"
#include "MainFrm.h"

static void OkCB(Fl_Widget *wdg, void *arg)
{
	((ItemPropertiesBase*)arg)->OnOK();
}

static void CanCB(Fl_Widget *wdg, void *arg)
{
	((ItemPropertiesBase*)arg)->OnCancel();
}

static void BrowseCB(Fl_Widget *wdg, void *arg)
{
	BrowseArg *ba = (BrowseArg*)arg;
	ba->dlg->OnBrowse(ba->wdg, ba->typ);
}

ItemPropertiesBase::ItemPropertiesBase(ProjectItem *p, int width, int height) 
	: Fl_Window(100, 100, width, height, "Properites")
{
	nameInp = 0;
	descInp = 0;
	doneSet = 0;
	pi = p;
	txtHeight = 25;
	txtSpace  = 30;
}

ItemPropertiesBase::~ItemPropertiesBase()
{
}

int ItemPropertiesBase::AddNameDesc(int ypos)
{
	nameInp = new Fl_Input(90, ypos, 240, txtHeight, "Name");
	ypos += txtSpace;
	descInp = new Fl_Input(90, ypos, 240, txtHeight, "Description");
	return ypos + txtSpace;
}

int ItemPropertiesBase::AddOkCancel(int ypos)
{
	int xpos = (w() - 110) / 2;
	okBtn = new Fl_Button(xpos, ypos, 50, txtHeight, "OK");
	okBtn->callback(OkCB, (void*)this);
	canBtn = new Fl_Button(xpos+60, ypos, 50, txtHeight, "Cancel");
	canBtn->callback(CanCB, (void*)this);
	end();
	ypos += txtSpace;
	size(w(), ypos);
	return ypos;
}

void ItemPropertiesBase::OnOK()
{
	if (pi->SaveProperties((PropertyBox*)this))
	{
		if (theProject)
			theProject->SetChange(1);
		doneSet = 1;
	}
}

void ItemPropertiesBase::OnCancel()
{
	doneSet = 2;
}

void ItemPropertiesBase::OnBrowse(Fl_Input *wdg, const char *typ)
{
	bsString file;
	file = wdg->value();
	if (prjFrame->BrowseFile(0, file, typ, 0))
		wdg->value(file);
}

void ItemPropertiesBase::EnableValue(int id, int enable)
{
	if (id == PROP_NAME)
		EnableValue(nameInp, enable);
	else if (id == PROP_DESC)
		EnableValue(descInp, enable);
}

void ItemPropertiesBase::EnableValue(Fl_Widget *wdg, int enable)
{
	if (wdg)
	{
		if (enable)
			wdg->activate();
		else
			wdg->deactivate();
	}
}

void ItemPropertiesBase::SetCaption(const char *text)
{
	label(text);
}

void ItemPropertiesBase::SetState(int id, short onoff)
{
}

int ItemPropertiesBase::GetState(int id, short& onoff)
{
	onoff = 0;
	return 0;
}

void ItemPropertiesBase::SetState(Fl_Button *wdg, short onoff)
{
	if (wdg)
		wdg->value(onoff);
}

int ItemPropertiesBase::GetState(Fl_Button *wdg, short& onoff)
{
	if (wdg)
		onoff = wdg->value();
	else
		onoff = 0;
	return 1;
}

void ItemPropertiesBase::SetValue(int id, float val, const char *lbl)
{
	char txt[40];
	snprintf(txt, 40, "%f", val);
	SetValue(id, txt, lbl);
}

void ItemPropertiesBase::SetValue(int id, long val, const char *lbl)
{
	char txt[40];
	snprintf(txt, 40, "%d", val);
	SetValue(id, txt, lbl);
}

void ItemPropertiesBase::SetValue(int id, const char *val, const char *lbl)
{
	if (id == PROP_NAME)
		SetValue(nameInp, val, lbl);
	else if (id == PROP_DESC)
		SetValue(descInp, val, lbl);
}

void ItemPropertiesBase::SetValue(Fl_Input *wdg, const char *val, const char *lbl)
{
	if (wdg)
	{
		wdg->value(val);
		if (lbl)
			wdg->label(lbl);
	}
}

int ItemPropertiesBase::GetValue(int id, float& val)
{
	char txt[40];
	if (GetValue(id, txt, 40))
	{
		val = atof(txt);
		return 1;
	}
	return 0;
}

int ItemPropertiesBase::GetValue(int id, long& val)
{
	char txt[40];
	if (GetValue(id, txt, 40))
	{
		val = atol(txt);
		return 1;
	}
	return 0;
}

int ItemPropertiesBase::GetValue(int id, char *val, int len)
{
	if (id == PROP_NAME)
		return GetValue(nameInp, val, len);
	if (id == PROP_DESC)
		return GetValue(descInp, val, len);
	return 0;
}

int ItemPropertiesBase::GetValue(Fl_Input *wdg, char *val, int len)
{
	*val = 0;
	if (wdg)
	{
		const char *cval = wdg->value();
		if (cval)
			strncpy(val, cval, len);
		return 1;
	}
	return 0;
}

int ItemPropertiesBase::GetValue(int id, bsString& val)
{
	if (id == PROP_NAME)
		return GetValue(nameInp, val);
	if (id == PROP_DESC)
		return GetValue(descInp, val);
	val = "";
	return 0;
}

int ItemPropertiesBase::GetValue(Fl_Input *wdg, bsString& val)
{
	if (wdg)
	{
		val = wdg->value();
		return 1;
	}
	val = "";
	return 0;
}

int ItemPropertiesBase::GetSelection(int id, short& sel)
{
	return 0;
}

int ItemPropertiesBase::GetSelection(Fl_Hold_Browser *wdg, short& sel)
{
	if (wdg)
		sel = wdg->value() - 1;
	else
		sel = -1;
	return (sel >= 0);
}

int ItemPropertiesBase::GetSelection(Fl_Choice *wdg, short& sel)
{
	if (wdg)
		sel = wdg->value();
	else
		sel = -1;
	return (sel >= 0);
}

void ItemPropertiesBase::SetSelection(int id, short sel)
{
}

void ItemPropertiesBase::SetSelection(Fl_Hold_Browser *wdg, short sel)
{
	if (wdg)
		wdg->value(sel+1);
}

void ItemPropertiesBase::SetSelection(Fl_Choice *wdg, short sel)
{
	if (wdg)
		wdg->value(sel);
}

int ItemPropertiesBase::GetSelection(int id, void **sel)
{
	return 0;
}

int ItemPropertiesBase::GetSelection(Fl_Hold_Browser *wdg, void **sel)
{
	if (wdg)
		*sel = wdg->data(wdg->value());
	else
		*sel = 0;
	return *sel != 0;
}

int ItemPropertiesBase::GetSelection(Fl_Choice *wdg, void **sel)
{
	if (wdg)
	{
		const Fl_Menu_Item *mi = wdg->mvalue();
		if (mi)
		{
			*sel = mi->user_data();
			return 1;
		}
	}
	*sel = 0;
	return 0;
}

void ItemPropertiesBase::SetSelection(int id, void *sel)
{
}

void ItemPropertiesBase::SetSelection(Fl_Hold_Browser *wdg, void *sel)
{
	for (int index = 1; index <= wdg->size(); index++)
	{
		if (wdg->data(index) == sel)
		{
			wdg->select(index);
			break;
		}
	}
}

void ItemPropertiesBase::SetSelection(Fl_Choice *wdg, void *sel)
{
	for (int index = 0; index <= wdg->size(); index++)
	{
		const Fl_Menu_Item *mi = wdg->mvalue();
		if (mi && mi->user_data() == sel)
		{
			wdg->value(mi);
			break;
		}
	}
}

int ItemPropertiesBase::ListChildren(int id, ProjectItem *parent)
{
	return 0;
}

int ItemPropertiesBase::ListChildren(Fl_Hold_Browser *wdg, ProjectItem *parent)
{
	int count = 0;
	ProjectItem *itm = prjTree->FirstChild(parent);
	while (itm)
	{
		wdg->insert(wdg->size()+1, itm->GetName(), (void*)itm);
		itm = prjTree->NextSibling(itm);
		count++;
	}
	return count;
}

int ItemPropertiesBase::ListChildren(Fl_Choice *wdg, ProjectItem *parent)
{
	int count = 0;
	ProjectItem *itm = prjTree->FirstChild(parent);
	while (itm)
	{
		wdg->add(itm->GetName(), 0, 0, (void*)itm);
		itm = prjTree->NextSibling(itm);
		count++;
	}
	return count;
}

int ItemPropertiesBase::GetListCount(int id, int& count)
{
	count = 0;
	return 0;
}

int ItemPropertiesBase::GetListCount(Fl_Hold_Browser *wdg, int& count)
{
	if (wdg)
		count = wdg->size();
	else
		count = 0;
	return 1;
}

int ItemPropertiesBase::GetListCount(Fl_Choice *wdg, int& count)
{
	if (wdg)
		count = wdg->size();
	else
		count = 0;
	return 1;
}

ProjectItem *ItemPropertiesBase::GetListItem(int id, int ndx)
{
	return 0;
}

ProjectItem *ItemPropertiesBase::GetListItem(Fl_Hold_Browser *wdg, int ndx)
{
	if (wdg)
		return (ProjectItem*)wdg->data(ndx+1);
	return 0;
}

int ItemPropertiesBase::Activate(int mdl)
{
	pi->LoadProperties((PropertyBox*)this);
	position(mainWnd->x()+100, mainWnd->y()+100);
	doneSet = 0;
	if (mdl)
		set_modal();
	show();
	if (mdl)
	{
		while (!doneSet)
			Fl::wait();
	}
	return doneSet == 1 ? 1 : 0;
}

NamePropertiesDlg::NamePropertiesDlg(ProjectItem *p) 
	: ItemPropertiesBase(p, 350, 90)
{
	int ypos = AddNameDesc(5);
	AddOkCancel(ypos);
}

FilePropertiesDlg::FilePropertiesDlg(ProjectItem *p)
	:  ItemPropertiesBase(p, 350, 135)
{
	int ypos = AddNameDesc(5);
	fileInp = new Fl_Input(90, ypos, 220, txtHeight, "File");
	fileBrowse = new Fl_Button(310, ypos, 20, txtHeight, "...");
	fileArg.dlg = this;
	fileArg.wdg = fileInp;
	fileArg.typ = ProjectItem::GetFileSpec(p->GetType());
	fileBrowse->callback(BrowseCB, (void*)&fileArg);
	ypos += txtSpace;
	fileIncl = new Fl_Check_Button(90, ypos, 220, txtHeight, "Include this file");
	AddOkCancel(ypos+txtSpace);
}

void FilePropertiesDlg::EnableValue(int id, int enable)
{
	if (id == PROP_FILE)
		ItemPropertiesBase::EnableValue(fileInp, enable);
	else
		ItemPropertiesBase::EnableValue(id, enable);
}

void FilePropertiesDlg::SetValue(int id, const char *val, const char *lbl)
{
	if (id == PROP_FILE)
		ItemPropertiesBase::SetValue(fileInp, val, lbl);
	else
		ItemPropertiesBase::SetValue(id, val, lbl);
}

int FilePropertiesDlg::GetValue(int id, char *val, int len)
{
	if (id == PROP_FILE)
		return ItemPropertiesBase::GetValue(fileInp, val, len);
	return ItemPropertiesBase::GetValue(id, val, len);
}

int FilePropertiesDlg::GetValue(int id, bsString& val)
{
	if (id == PROP_FILE)
		return ItemPropertiesBase::GetValue(fileInp, val);
	return ItemPropertiesBase::GetValue(id, val);
}

void FilePropertiesDlg::SetState(int id, short onoff)
{
	if (id == PROP_INCL)
		ItemPropertiesBase::SetState(fileIncl, onoff);
}

int FilePropertiesDlg::GetState(int id, short& onoff)
{
	if (id == PROP_INCL)
		return ItemPropertiesBase::GetState(fileIncl, onoff);
	return 0;
}

InstrPropertiesDlg::InstrPropertiesDlg(ProjectItem *p)
	:  ItemPropertiesBase(p, 350, 320)
{
	int ypos = AddNameDesc(5);
	instrType = new Fl_Hold_Browser(90, ypos, 240, 200, "Type");
	instrType->align(FL_ALIGN_LEFT);
	ypos += 205;
	instrNum = new Fl_Input(90, ypos, 50, txtHeight, "ID");
	ypos = AddOkCancel(ypos+txtSpace);
	InstrMapEntry *im = 0;
	while ((im = theProject->mgr.EnumType(im)) != 0)
		instrType->insert(instrType->size(), im->GetType(), (void*)im);
}

void InstrPropertiesDlg::EnableValue(int id, int enable)
{
	if (id == PROP_ITYP)
		ItemPropertiesBase::EnableValue(instrType, enable);
	else if (id == PROP_INUM)
		ItemPropertiesBase::EnableValue(instrNum, enable);
	else
		ItemPropertiesBase::EnableValue(id, enable);
}

void InstrPropertiesDlg::SetValue(int id, const char *val, const char *lbl)
{
	if (id == PROP_INUM)
		ItemPropertiesBase::SetValue(instrNum, val, lbl);
	else
		ItemPropertiesBase::SetValue(id, val, lbl);
}

int InstrPropertiesDlg::GetValue(int id, char *val, int len)
{
	if (id == PROP_INUM)
		return ItemPropertiesBase::GetValue(instrNum, val, len);
	return ItemPropertiesBase::GetValue(id, val, len);
}

int InstrPropertiesDlg::GetValue(int id, bsString& val)
{
	if (id == PROP_INUM)
		return ItemPropertiesBase::GetValue(instrNum, val);
	return ItemPropertiesBase::GetValue(id, val);
}

int InstrPropertiesDlg::GetSelection(int id, void **sel)
{
	if (id == PROP_ITYP)
		return ItemPropertiesBase::GetSelection(instrType, sel);
	return 0;
}

void InstrPropertiesDlg::SetSelection(int id, void *sel)
{
	if (id == PROP_ITYP)
		ItemPropertiesBase::SetSelection(instrType, sel);
}

WavefilePropertiesDlg::WavefilePropertiesDlg(ProjectItem *p)
	:  ItemPropertiesBase(p, 350, 150)
{
	int ypos = AddNameDesc(5);
	fileInp = new Fl_Input(90, ypos, 220, txtHeight, "File");
	fileBrowse = new Fl_Button(310, ypos, 20, txtHeight, "...");
	fileArg.dlg = this;
	fileArg.wdg = fileInp;
	fileArg.typ = ProjectItem::GetFileSpec(p->GetType());
	fileBrowse->callback(BrowseCB, (void*)&fileArg);
	ypos += txtSpace;
	wfId = new Fl_Input(90, ypos, 80, txtHeight, "Wavefile ID");
	ypos = AddOkCancel(ypos+txtSpace);
}

void WavefilePropertiesDlg::EnableValue(int id, int enable)
{
	if (id == PROP_FILE)
		ItemPropertiesBase::EnableValue(fileInp, enable);
	else if (id == PROP_WVID)
		ItemPropertiesBase::EnableValue(wfId, enable);
	else
		ItemPropertiesBase::EnableValue(id, enable);
}

void WavefilePropertiesDlg::SetValue(int id, const char *val, const char *lbl)
{
	if (id == PROP_FILE)
		ItemPropertiesBase::SetValue(fileInp, val, lbl);
	else if (id == PROP_WVID)
		ItemPropertiesBase::SetValue(wfId, val, lbl);
	else
		ItemPropertiesBase::SetValue(id, val, lbl);
}

int WavefilePropertiesDlg::GetValue(int id, char *val, int len)
{
	if (id == PROP_FILE)
		return ItemPropertiesBase::GetValue(fileInp, val, len);
	if (id == PROP_WVID)
		return ItemPropertiesBase::GetValue(wfId, val, len);
	return ItemPropertiesBase::GetValue(id, val, len);
}

int WavefilePropertiesDlg::GetValue(int id, bsString& val)
{
	if (id == PROP_FILE)
		return ItemPropertiesBase::GetValue(fileInp, val);
	if (id == PROP_WVID)
		return ItemPropertiesBase::GetValue(wfId, val);
	return ItemPropertiesBase::GetValue(id, val);
}

ProjectPropertiesDlg::ProjectPropertiesDlg()
	: ItemPropertiesBase(theProject, 500, 500)
{

	int ypos = 5;
	nameInp = new Fl_Input(90, ypos, 400, txtHeight, "Name");
	ypos += txtSpace;
	
	compInp = new Fl_Input(90, ypos, 150, txtHeight, "Composer");
	cpyrInp = new Fl_Input(340, ypos, 150, txtHeight, "Copyright");
	ypos += txtSpace;

	descInp = new Fl_Multiline_Input(90, ypos, 400, txtHeight*3, "Description");
	ypos += txtSpace+(txtHeight*2);
	
	prjfInp = new Fl_Input(90, ypos, 380, txtHeight, "Project File");
	prjfSel = new Fl_Button(470, ypos, 20, txtHeight, "...");
	prjfArg.dlg = this;
	prjfArg.wdg = prjfInp;
	prjfArg.typ = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	prjfSel->callback(BrowseCB, (void*)&prjfArg);
	ypos += txtSpace;
	
	outfInp = new Fl_Input(90, ypos, 380, txtHeight, "Output File");
	outfSel = new Fl_Button(470, ypos, 20, txtHeight, "...");
	outfArg.dlg = this;
	outfArg.wdg = outfInp;
	outfArg.typ = ProjectItem::GetFileSpec(PRJNODE_WVFILE);
	outfSel->callback(BrowseCB, (void*)&outfArg);
	ypos += txtSpace;

	srtSel = new Fl_Choice(90, ypos, 100, txtHeight, "Sample Rt.");
	leadInp = new Fl_Input(260, ypos, 80, txtHeight, "Leader");
	wtszInp = new Fl_Input(410, ypos, 80, txtHeight, "WT Siz.");
	ypos += txtSpace;

	fmtSel = new Fl_Choice(90, ypos, 100, txtHeight, "Sample Rt.");
	tailInp = new Fl_Input(260, ypos, 80, txtHeight, "Tail");
	wtusrInp = new Fl_Input(410, ypos, 80, txtHeight, "Usr. WT");
	ypos += txtSpace;

	wavfInp = new Fl_Input(90, ypos, 380, txtHeight, "Wave Files");
	wavfSel = new Fl_Button(470, ypos, 20, txtHeight, "...");
	wavfArg.dlg = this;
	wavfArg.wdg = wavfInp;
	wavfArg.typ = ProjectItem::GetFileSpec(PRJNODE_WVFILE);
	wavfSel->callback(BrowseCB, (void*)&wavfArg);
	ypos += txtSpace;

	AddOkCancel(ypos);

	srtSel->add("22050");
	srtSel->add("44100");
	srtSel->add("48000");
	srtSel->add("96000");
	srtSel->add("192000");

	fmtSel->add("16-bit PCM");
	fmtSel->add("32-bit Float");
}

Fl_Input *ProjectPropertiesDlg::GetInput(int id)
{
	Fl_Input *wdg = 0;
	switch (id)
	{
	case PROP_PRJ_NAME:
		wdg = nameInp;
		break;
	case PROP_PRJ_AUTH:
		wdg = compInp;
		break;
	case PROP_PRJ_CPYR:
		wdg = cpyrInp;
		break;
	case PROP_PRJ_DESC:
		wdg = descInp;
		break;
	case PROP_PRJ_PATH:
		wdg = prjfInp;
		break;
	case PROP_PRJ_OUTF:
		wdg = outfInp;
		break;
	case PROP_PRJ_LEAD:
		wdg = leadInp;
		break;
	case PROP_PRJ_TAIL:
		wdg = tailInp;
		break;
	case PROP_PRJ_WTSZ:
		wdg = wtszInp;
		break;
	case PROP_PRJ_WTU:
		wdg = wtusrInp;
		break;
	case PROP_PRJ_WVIN:
		wdg = wavfInp;
		break;
	default:
		wdg = 0;
	}
	return wdg;
}

void ProjectPropertiesDlg::EnableValue(int id, int enable)
{
	Fl_Widget *wdg = GetInput(id);
	if (wdg == 0)
	{
		if (id == PROP_PRJ_SRT)
			wdg = srtSel;
		else if (id == PROP_PRJ_SFMT)
			wdg = fmtSel;
		else
			return;
	}
	ItemPropertiesBase::EnableValue(wdg, enable);
}

void ProjectPropertiesDlg::SetValue(int id, const char *val, const char *lbl)
{
	Fl_Input *inpwdg = GetInput(id);
	if (inpwdg)
		ItemPropertiesBase::SetValue(inpwdg, val, lbl);
	const Fl_Menu_Item *mnuItm;
	if (id == PROP_PRJ_SRT)
	{
		mnuItm = srtSel->find_item(val);
		if (mnuItm)
			srtSel->value(mnuItm);
		else
			srtSel->add(val, 0, 0, 0);
	}
	if (id == PROP_PRJ_SFMT)
	{
		mnuItm = fmtSel->find_item(val);
		if (mnuItm)
			fmtSel->value(mnuItm);
		else
			fmtSel->add(val, 0, 0, 0);
	}
}


int ProjectPropertiesDlg::GetValue(int id, char *val, int len)
{
	Fl_Input *inp = GetInput(id);
	if (inp)
		return ItemPropertiesBase::GetValue(inp, val, len);
	if (id == PROP_PRJ_SRT)
	{
		strncpy(val, srtSel->text(), len);
		return 1;
	}
	if (id == PROP_PRJ_SFMT)
	{
		strncpy(val, fmtSel->text(), len);
		return 1;
	}
	return 0;
}

int ProjectPropertiesDlg::GetValue(int id, bsString& val)
{
	Fl_Input *inp = GetInput(id);
	if (inp)
		return ItemPropertiesBase::GetValue(inp, val);
	if (id == PROP_PRJ_SRT)
		val = srtSel->text();
	if (id == PROP_PRJ_SFMT)
		val = fmtSel->text();
	return 0;
}

int ProjectPropertiesDlg::GetSelection(int id, short& sel)
{
	sel = -1;
	if (id == PROP_PRJ_SRT)
		sel = srtSel->value();
	else if (id == PROP_PRJ_SFMT)
		sel = fmtSel->value();
	return sel >= 0;
}

void ProjectPropertiesDlg::SetSelection(int id, short sel)
{
	if (id == PROP_PRJ_SRT)
		sel = srtSel->value(sel);
	else if (id == PROP_PRJ_SFMT)
		sel = fmtSel->value(sel);
}

///////////////////////////////////////////////////////////////////////

static void FxAddCB(Fl_Widget *w, void *arg)
{
	((MixerSetupDlg*)arg)->OnAdd();
}

static void FxRemCB(Fl_Widget *w, void *arg)
{
	((MixerSetupDlg*)arg)->OnRemove();
}

MixerSetupDlg::MixerSetupDlg(ProjectItem *p) : ItemPropertiesBase(p, 430, 400)
{
	Fl_Group *grp;

	int ypos = 5;
	chnlInp = new Fl_Input(80, ypos, 80, txtHeight, "Inputs");
	chnlInp->align(FL_ALIGN_LEFT);
	grp = new Fl_Group(165, ypos, 270, txtHeight, 0);
	linBtn = new Fl_Light_Button(165, ypos, 80, txtHeight, "Pan Lin.");
	linBtn->type(FL_RADIO_BUTTON);
	trgBtn = new Fl_Light_Button(255, ypos, 80, txtHeight, "Pan Trig.");
	trgBtn->type(FL_RADIO_BUTTON);
	sqrBtn = new Fl_Light_Button(345, ypos, 80, txtHeight, "Pan Sqrt.");
	sqrBtn->type(FL_RADIO_BUTTON);
	grp->end();
	ypos += txtSpace;

	fxList = new Fl_Hold_Browser(80, ypos, 345, 200, "Effects");
	fxList->align(FL_ALIGN_LEFT);
	ypos += fxList->h() + 5;
	fxName = new Fl_Input(80, ypos, 80, txtHeight, "Fx Name");
	fxTypes = new Fl_Choice(165, ypos, 80, txtHeight, 0);
	fxTypes->add("reverb");
	fxTypes->add("flanger");
	fxTypes->add("echo");
	fxAdd = new Fl_Button(255, ypos, 80, txtHeight, "Add");
	fxAdd->callback(FxAddCB, (void*)this);
	fxRem = new Fl_Button(345, ypos, 80, txtHeight, "Remove");
	fxRem->callback(FxRemCB, (void*)this);
	AddOkCancel(ypos+txtSpace+5);
}

void MixerSetupDlg::OnAdd()
{
	char name[80];
	char type[80];
	memset(name, 0, sizeof(name));
	memset(type, 0, sizeof(type));
	int ndx = fxTypes->value();
	if (ndx >= 0)
	{
		strncpy(type, fxTypes->text(), 80);
		const char *cval = fxName->value();
		if (cval)
			strncpy(name, cval, 80);
		if (name[0] == 0)
			strcpy(name, type);
		FxItem *fx = ((MixerItem*)pi)->AddEffect(type);
		if (fx)
		{
			fx->SetName(name);
			fx->SetUnit(-1);
			fxList->add(name, fx);
		}
	}
}

void MixerSetupDlg::OnRemove()
{
	int sel = fxList->value();
	if (sel > 0)
	{
		FxItem *fx = (FxItem *) fxList->data(sel);
		fxList->remove(sel);
	}
}

int MixerSetupDlg::ListChildren(int id, ProjectItem *parent)
{
	if (id == PROP_MIX_FX)
		ItemPropertiesBase::ListChildren(fxList, parent);
	return 0;
}

void MixerSetupDlg::SetValue(int id, const char *val, const char *lbl)
{
	if (id == PROP_MIX_CHNL)
		ItemPropertiesBase::SetValue(chnlInp, val, lbl);
}

void MixerSetupDlg::SetState(int id, short onoff)
{
	if (id == PROP_MIX_PLIN)
		ItemPropertiesBase::SetState(linBtn, onoff);
	else if (id == PROP_MIX_PTRG)
		ItemPropertiesBase::SetState(trgBtn, onoff);
	else if (id == PROP_MIX_PSQR)
		ItemPropertiesBase::SetState(sqrBtn, onoff);
}

int MixerSetupDlg::GetValue(int id, char *val, int len)
{
	if (id == PROP_MIX_CHNL)
		return ItemPropertiesBase::GetValue(chnlInp, val, len);
	*val = 0;
	return 0;
}

int MixerSetupDlg::GetState(int id, short& onoff)
{
	if (id == PROP_MIX_PLIN)
		ItemPropertiesBase::GetState(linBtn, onoff);
	else if (id == PROP_MIX_PTRG)
		ItemPropertiesBase::GetState(trgBtn, onoff);
	else if (id == PROP_MIX_PSQR)
		ItemPropertiesBase::GetState(sqrBtn, onoff);
	return 0;
}

int MixerSetupDlg::GetListCount(int id, int& count)
{
	if (id == PROP_MIX_FX)
	{
		count = fxList->size();
		return 1;
	}
	count = 0;
	return 0;
}

ProjectItem *MixerSetupDlg::GetListItem(int id, int ndx)
{
	if (id == PROP_MIX_FX)
		return ItemPropertiesBase::GetListItem(fxList, ndx);
	return 0;
}


EffectsSetupDlg::EffectsSetupDlg(ProjectItem *pi) 
 : ItemPropertiesBase(pi, 200, 400)
{
	int lblw = 100;
	int valw = 80;
	int ypos = 5;
	nameInp = new Fl_Input(lblw, ypos, valw, txtHeight, "Name");
	ypos += txtSpace;
//	descInp = new Fl_Input(lblw, ypos, valw, txtHeight, "Desc.");
//	ypos += txtSpace;
	volInp = new Fl_Input(lblw, ypos, valw, txtHeight, "Volume");
	ypos += txtSpace;
	panInp = new Fl_Input(lblw, ypos, valw, txtHeight, "Pan");
	ypos += txtSpace;
	val1Inp = new Fl_Input(lblw, ypos, valw, txtHeight, "");
	ypos += txtSpace;
	val2Inp = new Fl_Input(lblw, ypos, valw, txtHeight, "");
	if (pi->GetType() != PRJNODE_ECHO)
	{
		ypos += txtSpace;
		val3Inp = new Fl_Input(lblw, ypos, valw, txtHeight, "");
		ypos += txtSpace;
		val4Inp = new Fl_Input(lblw, ypos, valw, txtHeight, "");
		ypos += txtSpace;
		val5Inp = new Fl_Input(lblw, ypos, valw, txtHeight, "");
	}
	else
	{
		val3Inp = 0;
		val4Inp = 0;
		val5Inp = 0;
	}
	ypos += txtSpace;
	AddOkCancel(ypos);
}


Fl_Input *EffectsSetupDlg::GetInput(int id)
{
	switch (id)
	{
	case PROP_NAME:  return nameInp;
	case PROP_DESC:  return descInp;
	case PROP_FX_VOL: return volInp;
	case PROP_FX_PAN: return panInp;
	case PROP_FX_V1: return val1Inp;
	case PROP_FX_V2: return val2Inp;
	case PROP_FX_V3: return val3Inp;
	case PROP_FX_V4: return val4Inp;
	case PROP_FX_V5: return val5Inp;
	}
	return 0;
}

void EffectsSetupDlg::SetValue(int id, const char *val, const char *lbl)
{
	ItemPropertiesBase::SetValue(GetInput(id), val, lbl);
}

int EffectsSetupDlg::GetValue(int id, char *val, int len)
{
	return ItemPropertiesBase::GetValue(GetInput(id), val, len);
}

int EffectsSetupDlg::GetValue(int id, bsString& val)
{
	return ItemPropertiesBase::GetValue(GetInput(id), val);
}

void EffectsSetupDlg::EnableValue(int id, int enable)
{
	ItemPropertiesBase::EnableValue(GetInput(id), enable);
}

///////////////////////////////////////////////////////////////////////

static void QueryOkCB(Fl_Widget *wdg, void *arg)
{
	((QueryValueDlg*)arg)->OnOK();
}

static void QueryCanCB(Fl_Widget *wdg, void *arg)
{
	((QueryValueDlg*)arg)->OnCancel();
}

QueryValueDlg::QueryValueDlg() : Fl_Window(100, 100, 300, 100, "Enter Value")
{
	done = 0;
	vallen = 0;
	value = 0;
	lbl = new Fl_Box(5, 5, 290, 25);
	inp = new Fl_Input(5, 35, 290, 25);
	ok = new Fl_Button(5, 65, 60, 25, "Ok");
	ok->callback(QueryOkCB, (void*)this);
	can = new Fl_Button(70, 65, 60, 25, "Cancel");
	can->callback(QueryOkCB, (void*)this);
	end();
	resize((Fl::w() - w()) / 2, (Fl::h() - h()) / 2, w(), h());
}

void QueryValueDlg::OnOK()
{
	strncpy(value, inp->value(), vallen);
	done = 1;
}

void QueryValueDlg::OnCancel()
{
	done = 2;
}

int QueryValueDlg::Activate(const char *prompt, char *val, int len)
{
	done = 0;
	value = val;
	vallen = len;
	inp->value(val);
	lbl->label(prompt);
	set_modal();
	show();
	while (!done)
		Fl::wait();
	return done == 1;
}

