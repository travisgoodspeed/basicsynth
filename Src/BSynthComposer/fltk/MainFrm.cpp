//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Main window implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "MainMenu.h"
#include "ProjectTreeFltk.h"
#include "KbdGenDlg2.h"
#include "TabsView.h"
#include "FindReplDlgFltk.h"
#include "FormEditorFltk.h"
#include "TextEditorFltk.h"
#include "PropertiesDlgFltk.h"
#include "OptionsDlg.h"
#include "MainFrm.h"
#if _WIN32
#include "resource.h"
#endif

MainFrame *mainWnd;

#define MENU_H 30
#define TABS_H 40
#define TREE_W 200
#define KBD_H  200

static char aboutText[] =
"BasicSynth Composer\n"
"For latest information and updates visit http://basicsynth.com\n\n"
"© 2010 Daniel R. Mitchell\n"
"This work is licensed under a Creative Commons\n"
"Attribution-Share Alike 3.0 United States License\n"
"(http://creativecommons.org/licenses/by-sa/3.0/us/)\n\n"
"For FLTK information see: http://www.fltk.org";

static void MainFrameCB(Fl_Widget *wdg, void *arg)
{
	int evt = Fl::event();
	//LogDebug("MainFrame callback: %d\r\n", evt);
	if (evt == FL_CLOSE)
		mainWnd->Exit();
}

MainFrame::MainFrame(int X, int Y, int W, int H, const char* t)
		: Fl_Double_Window(X, Y, W, H, t)
{
    tabs = 0;
    tree = 0;
    kbd = 0;
#if _WIN32
	icon(::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN)));
#endif
	end();
	shutdown = 0;
	prjFileTypes = "Project files (*.bsprj)\tXML files (*.xml)\tAll files (*)";
	mainWnd = this;
	callback(MainFrameCB, (void*)0);
	size_range(400, 200);
	mnu = new MainMenu(W);
	add(mnu);
	mnu->UpdateRecentList();
	tree = new ProjectTreeFltk(0, MENU_H, TREE_W, H-(KBD_H+MENU_H));
	add(tree);
	tree->hide();
	kbd = new KbdGenDlg(0, H-KBD_H, W, KBD_H);
	add(kbd);
	kbd->hide();
	tabs = new TabsView(TREE_W, MENU_H, W-TREE_W, H-(MENU_H+KBD_H));
	add(tabs);
	tabs->hide();
	prjFrame = static_cast<ProjectFrame*>(this);
	curItem = 0;
	curEditor = 0;
	helpDlg = 0;
	findDlg = 0;
	Layout();
}

MainFrame::~MainFrame()
{
	prjFrame = 0;
}

void MainFrame::resize(int X, int Y, int W, int H)
{
	prjOptions.frmLeft = X;
	prjOptions.frmTop = Y;
	prjOptions.frmWidth = W;
	prjOptions.frmHeight = H;
	Fl_Double_Window::resize(X, Y, W, H);
	if (mnu)
	{
		mnu->size(W, MENU_H);
		mnu->redraw();
	}
	Layout();
}

void MainFrame::Layout()
{
	int wx;
	int wy;
	int ww;
	int wh;
	int kh;

	wx = 0;
	wy = MENU_H;
	ww = w();
	wh = h() - MENU_H;
	kh = 0;

	if (kbd && kbd->visible())
	{
		kh = kbd->h();
		wh -= kh;
		kbd->resize(0, h()-kh, w(), kh);
		kbd->redraw();
	}

	if (tree && ((Fl_Widget*)tree)->visible())
	{
		tree->resize(wx, wy, TREE_W, wh);
		tree->redraw();
		wx += TREE_W;
		ww -= TREE_W;
	}

	if (tabs && tabs->visible())
	{
		tabs->resize(wx, wy, ww, TABS_H);
		tabs->redraw();
		wh -= TABS_H;
		wy += TABS_H;
	}

	int count = children();
	int index;
	for (index = 0; index < count; index++)
	{
		Fl_Widget *ch = child(index);
		if (ch && ch != mnu && ch != tree && ch != kbd && ch != tabs)
			ch->resize(wx, wy, ww, wh);
	}
}

void MainFrame::ViewProject()
{
	if (!tree)
		return;

	int v = !((Fl_Widget*)tree)->visible();
	if (v)
		tree->show();
	else
		tree->hide();
	mnu->CheckProject(v);
	Layout();
	redraw();
}

void MainFrame::ViewKeyboard()
{
	if (!kbd)
		return;

	int v = !kbd->visible();
	if (v)
		kbd->show();
	else
		kbd->hide();
	mnu->CheckKeyboard(v);
	Layout();
	redraw();
}

void MainFrame::Generate(int autostart, int todisk)
{
	int wasPlaying = StopPlayer();
	GenerateDlg *dlg = new GenerateDlg(todisk);
	dlg->Run(autostart);
	if (wasPlaying)
		StartPlayer();
}


void MainFrame::ProjectOptions()
{
	ProjectOptionsDlg *dlg = new ProjectOptionsDlg;
	dlg->DoModal();
	delete dlg;
}

void MainFrame::ShowFind(TextEditorFltk *ed)
{
	if (findDlg == NULL)
	{
		findDlg = new FindReplDlgFltk();
		//add(findDlg);
		findDlg->set_non_modal();
	}
	findDlg->SetEditor(ed);
	findDlg->show();
}

void MainFrame::ShowAbout()
{
	fl_message(aboutText);
}

void MainFrame::ShowHelp()
{
	if (helpDlg == NULL)
	{
		if (prjOptions.helpFile[0] == 0 || !SynthFileExists(prjOptions.helpFile))
		{
			prjOptions.helpFile[0] = 0;
			char *file = fl_file_chooser("Select The Help File", "HTML (*.html)", prjOptions.helpFile);
			if (!file)
				return;
			strncpy(prjOptions.helpFile, file, MAX_PATH);
		}
		helpDlg = new Fl_Help_Dialog();
		helpDlg->load(prjOptions.helpFile);
	}
	helpDlg->show();
}

void MainFrame::ItemSelected(ProjectItem *pi)
{
	mnu->ItemSelected(pi);
}

void MainFrame::ItemDoubleClick(ProjectItem *pi)
{
	if (!OpenEditor(pi))
		pi->ItemProperties();
}

// The popup menu(s) could be created once and
// items enabled based on flags.
void MainFrame::ItemContextMenu(ProjectItem *pi)
{
	if (pi == NULL)
		pi = theProject;

	tree->SelectNode(pi);

	long flags = pi->ItemActions();
	long file = pi->GetEditor() != 0;

	int last1 = -1;
	int last2 = -1;
	Fl_Menu_Item *mi;

	Fl_Menu_Button *pop = new Fl_Menu_Button(0, 0, 200, 200);
	pop->type(Fl_Menu_Button::POPUP3);
	add(pop);
	switch (pi->GetType())
	{
	case PRJNODE_PROJECT:
		pop->add("Generate", 0, 0, (void*)9, 0);
		pop->add("Start player", 0, 0, (void*)10, 0);
		pop->add("Save Project", 0, 0, (void*)11, FL_MENU_DIVIDER);
		break;
	case PRJNODE_WAVEOUT:
		pop->add("Generate", 0, 0, (void*)9, FL_MENU_DIVIDER);
		break;
	case PRJNODE_SYNTH:
		pop->add("New Wavetable", 0, 0, (void*)3, FL_MENU_DIVIDER);
		break;
	case PRJNODE_NOTELIST:
	case PRJNODE_SEQLIST:
	case PRJNODE_TEXTLIST:
	case PRJNODE_LIBLIST:
	case PJRNODE_SCRIPTLIST:
	case PRJNODE_WVFLIST:
	case PRJNODE_SBLIST:
		pop->add("New File", 0, 0, (void*)3, 0);
		pop->add("Add Existing File", 0, 0, (void*)4, FL_MENU_DIVIDER);
		break;
	case PRJNODE_LIB:
		pop->add("Save Library", 0, 0, (void*)7, 0);
		pop->add("Add to Library", 0, 0, (void*)4, 0);
		pop->add("Copy to Project", 0, 0, (void*)6, 0);
		pop->add("Remove library", 0, 0, (void*)5, FL_MENU_DIVIDER);
		break;
	default:
		if (flags & ITM_ENABLE_EDIT)
			last1 = pop->add("Edit", 0, 0, (void*)1, 0);
		if (flags & ITM_ENABLE_SAVE)
			last1 = pop->add("Save", 0, 0, (void*)7, file ? 0 : FL_MENU_INACTIVE);
		if (flags & ITM_ENABLE_CLOSE)
			last1 = pop->add("Close", 0, 0, (void*)8, file ? 0 : FL_MENU_INACTIVE);
		if (flags & ITM_ENABLE_NEW)
			last2 = pop->add("New", 0, 0, (void*)3, 0);
		if (flags & ITM_ENABLE_ADD)
			last2 = pop->add("Add", 0, 0, (void*)4, 0);
		if (flags & ITM_ENABLE_COPY)
			last2 = pop->add("Copy", 0, 0, (void*)6, 0);
		if (flags & ITM_ENABLE_REM)
			last2 = pop->add("Remove", 0, 0, (void*)5, 0);
		if (last1 >= 0)
		{
			mi = (Fl_Menu_Item *)pop->menu();
			mi[last1].flags |= FL_MENU_DIVIDER;
		}
		if (last2 >= 0)
		{
			mi = (Fl_Menu_Item *)pop->menu();
			mi[last2].flags |= FL_MENU_DIVIDER;
		}
		break;
	}
	if (flags & ITM_ENABLE_PROPS)
		pop->add("Properties...", 0, 0, (void*)2, 0);
	const Fl_Menu_Item *itm = pop->popup();
	if (itm && itm->label())
	{
		switch ((long)itm->user_data())
		{
		case 1:
			pi->EditItem();
			break;
		case 2:
			pi->ItemProperties();
			break;
		case 3:
			pi->NewItem();
			break;
		case 4:
			pi->AddItem();
			break;
		case 5:
			pi->CopyItem();
			break;
		case 6:
			pi->RemoveItem();
			break;
		case 7:
			pi->SaveItem();
			break;
		case 8:
			pi->CloseItem();
			break;
		case 9:
			Generate(0, 1);
			break;
		case 10:
			StartPlayer();
			break;
		case 11:
			SaveProject();
			break;
		}
	}
	Fl::delete_widget(pop);
}

EditorView *MainFrame::GetActiveEditor()
{
	if (tabs)
		return tabs->GetActiveItem();
	return 0;
}

void MainFrame::EditorSelected(EditorView *vw)
{
	mnu->EditorSelected(vw);
	if (vw)
	{
		Fl_Widget *wdg = (Fl_Widget*)vw->GetPSData();
		int count = children();
		int index;
		for (index = 0; index < count; index++)
		{
			Fl_Widget *ch = child(index);
			if (ch && ch != mnu && ch != tree && ch != kbd && ch != tabs)
			{
				if (wdg != ch)
					ch->hide();
			}
		}
		wdg->show();
		damage(FL_DAMAGE_CHILD, wdg->x(), wdg->y(), wdg->w(), wdg->h());
		ProjectItem *pi = vw->GetItem();
		if (pi && pi == tree->GetSelectedNode())
			mnu->ItemSelected(pi);
	}
	else
		redraw();
}

int MainFrame::Exit()
{
	if (CloseProject(1))
	{
		prjOptions.Save();
		if (findDlg)
		{
			Fl::delete_widget(findDlg);
			findDlg = 0;
		}
		Fl::delete_widget(this);
		shutdown = 1;
		return 1;
	}
	return 0;
}

void FixFileSpec(char *out, const char *in)
{
	// types are in the MSVC resource form: Descr|pattern|Descr|pattern|...
	// fltk form is Desc (pattern)\tDescr (pattern)\t...
	int cls = 0;
	const char *p1 = in;
	char *p2 = out;
	char *pe = out+510;
	while (*p1 && p2 < pe)
	{
		if (*p1 == '|')
		{
			if (cls)
			{
				*p2++ = ')';
				*p2++ = '\t';
				cls = 0;
			}
			else
			{
				*p2++ = ' ';
				*p2++ = '(';
				cls = 1;
			}
			p1++;
		}
		else
			*p2++ = *p1++;
	}
	if (cls)
		*p2++ = ')';
	p1 = "\tAll Files (*)";
	while (*p1 && p2 < pe)
		*p2++ = *p1++;
	*p2 = 0;
}

int MainFrame::BrowseFile(int open, char *file, const char *spec, const char *ext)
{
	char spcbuf[512];
	FixFileSpec(spcbuf, spec);
	char *ret = fl_file_chooser(open ? "Open" : "Save", spcbuf, file);
	if (ret)
	{
		strncpy(file, ret, 512);
		return 1;
	}
	return 0;
}

int MainFrame::BrowseFile(int open, bsString& file, const char *spec, const char *ext)
{
	char fnbuf[512];
	strcpy(fnbuf, file);
	int ret = BrowseFile(open, fnbuf, spec, ext);
	if (ret)
	{
		file = fnbuf;
		return 1;
	}
	return 0;
}

void MainFrame::AfterOpenProject()
{
	SynthWidget::colorMap.Load(prjOptions.formsDir, prjOptions.colorsFile, 1);

	// update recent file list
	int count = prjOptions.docCount;
	bsString path;
	theProject->GetProjectPath(path);
	int ndx1 = 0;
	while (ndx1 < count)
	{
		if (prjOptions.docList[ndx1].path.CompareNC(path) == 0)
		{
			count--;
			break;
		}
		ndx1++;
	}
	if (ndx1 >= prjOptions.docMax)
		ndx1--;

	while (ndx1 > 0)
	{
		prjOptions.docList[ndx1].path = prjOptions.docList[ndx1-1].path;
		prjOptions.docList[ndx1].name = prjOptions.docList[ndx1-1].name;
		ndx1--;
	}
	prjOptions.docList[0].SetPath(path);
	if (++count > prjOptions.docMax)
		count = prjOptions.docMax;
	prjOptions.docCount = count;

	mnu->EnableProject(1);
	mnu->CheckProject(1);
	mnu->CheckKeyboard(1);
	mnu->UpdateRecentList();
	tree->UpdateNode(theProject);
	tree->show();
	kbd->Load();
	kbd->show();
	tabs->show();

	bsString titleStr;
	titleStr = prjOptions.programName;
	titleStr += " - ";
	titleStr += theProject->GetName();
	label(titleStr);
	Layout();
}

int MainFrame::NewProject()
{
	if (ProjectFrame::NewProject())
	{
		AfterOpenProject();
		return 1;
	}
	return 0;
}

void MainFrame::OpenRecent(long n)
{
	if (n < prjOptions.docCount)
	{
		bsString tmp(prjOptions.docList[n].path);
		OpenProject(tmp);
	}
}

int MainFrame::OpenProject(const char *fname)
{
	if (ProjectFrame::OpenProject(fname))
	{
		AfterOpenProject();
		return 1;
	}
	return 0;
}

int MainFrame::CloseProject(int q)
{
	if (ProjectFrame::CloseProject(q))
	{
		mnu->EnableProject(0);
		mnu->ItemSelected(0);
		mnu->EditorSelected(0);
		return 1;
	}
	return 0;
}

int MainFrame::Verify(const char *msg, const char *title)
{
	switch (fl_choice(msg, "Yes", "No", "Cancel"))
	{
	case 0:
		return 1;
	case 1:
		return 0;
	}
	return -1;
}

PropertyBox *MainFrame::CreatePropertyBox(ProjectItem *pi, int type)
{
	PropertyBox *pb = 0;
	if (type == 0)
		type = pi->GetType();
	switch (type)
	{
	case PRJNODE_PROJECT:
	case PRJNODE_SYNTH:
	case PRJNODE_WAVEOUT:
		pb = (PropertyBox *) new ProjectPropertiesDlg();
		break;
	case PRJNODE_MIXER:
		StopPlayer();
		pb = (PropertyBox *) new MixerSetupDlg(pi);
		break;
	case PRJNODE_REVERB:
	case PRJNODE_FLANGER:
	case PRJNODE_ECHO:
		pb = (PropertyBox *) new EffectsSetupDlg(pi);
		break;
	case PRJNODE_NOTEFILE:
	case PRJNODE_SEQFILE:
	case PRJNODE_TEXTFILE:
	case PRJNODE_SCRIPT:
		pb = (PropertyBox *) new FilePropertiesDlg(pi);
		break;
	case PRJNODE_INSTR:
		pb = (PropertyBox *) new InstrPropertiesDlg(pi);
		break;
	case PRJNODE_WVTABLE:
		pb = (PropertyBox *) new NamePropertiesDlg(pi);
		break;
	case PRJNODE_WVFILE:
		pb = (PropertyBox *) new WavefilePropertiesDlg(pi);
		break;
	case PRJNODE_LIBLIST:
		break;
	case PRJNODE_LIB:
		break;
	case PRJNODE_LIBINSTR:
		break;
	}
	return pb;
}

FormEditor *MainFrame::CreateFormEditor(ProjectItem *pi)
{
	//printf("Create form editor %s\n", pi->GetName());
	int ww = w();
	int wx = 0;
	int wy = MENU_H+TABS_H;
	int wh = h() - wy;
	if (tree && ((Fl_Widget*)tree)->visible())
	{
		wx += tree->w();
		ww -= tree->w();
	}
	if (kbd && kbd->visible())
		wh -= kbd->h();
	FormEditorFltk *formEd = new FormEditorFltk(wx, wy, ww, wh);
	Fl_Widget *wdg = static_cast<Fl_Widget*>(formEd);
	add(wdg);
	formEd->SetPSData(reinterpret_cast<void*>(wdg));
	formEd->SetItem(pi);
	tabs->AddItem(formEd);
	EditorSelected(formEd);

	return static_cast<FormEditor*>(formEd);
}

TextEditor *MainFrame::CreateTextEditor(ProjectItem *pi)
{
	int ww = w();
	int wx = 0;
	int wy = MENU_H+TABS_H;
	int wh = h() - wy;
	if (tree && ((Fl_Widget*)tree)->visible())
	{
		wx += tree->w();
		ww -= tree->w();
	}
	if (kbd && kbd->visible())
		wh -= kbd->h();
	TextEditorFltk *ed = new TextEditorFltk(wx, wy, ww, wh);
	Fl_Widget *wdg = static_cast<Fl_Widget*>(ed);
	add(wdg);
	ed->SetPSData(reinterpret_cast<void*>(wdg));
	ed->SetItem(pi);
//	pi->SetEditor(static_cast<EditorView*>(ed));
	tabs->AddItem(ed);
	EditorSelected(ed);

	return static_cast<TextEditor*>(ed);
}

void MainFrame::EditStateChanged()
{
	mnu->EditorSelected(tabs->GetActiveItem());
}

int MainFrame::OpenEditor(ProjectItem *pi)
{
	EditorView *ed = pi->GetEditor();
	if (ed)
	{
		EditorSelected(ed);
		return 1;
	}
	return pi->EditItem();
}

int MainFrame::CloseEditor(ProjectItem *pi)
{
	EditorView *ed = pi->GetEditor();
	int issel = ed == tabs->GetActiveItem();
	if (ed)
		tabs->RemoveItem(ed);
	if (issel)
		EditorSelected(tabs->GetActiveItem());
	if (pi == tree->GetSelectedNode())
		mnu->ItemSelected(pi);
	return 1;
}

int MainFrame::CloseAllEditors()
{
	return tabs->CloseAll(1);
}

int MainFrame::SaveAllEditors(int query)
{
	return tabs->SaveAll(query);
}


int MainFrame::QueryValue(const char *prompt, char *value, int len)
{
	QueryValueDlg dlg;
	return dlg.Activate(prompt, value, len);
}

int MainFrame::Alert(const char *msg, const char *title)
{
	fl_message(msg);
	return 1;
}

void MainFrame::InitPlayer()
{
	kbd->InitInstrList();
	kbd->UpdateChannels();
}

void MainFrame::ClearPlayer()
{
	kbd->Clear();
}

int MainFrame::StopPlayer()
{
	return kbd->Stop();
}

void MainFrame::StartPlayer()
{
	kbd->Start();
}

// these functions are used when there is a "live" player
// that needs notifications of instrument changes...
void MainFrame::InstrAdded(InstrConfig *inst)
{
	kbd->AddInstrument(inst);
}

void MainFrame::InstrRemoved(InstrConfig *inst)
{
	kbd->RemoveInstrument(inst);
}

void MainFrame::InstrChanged(InstrConfig *inst)
{
	kbd->UpdateInstrument(inst);
}

void MainFrame::MixerChanged()
{
	kbd->UpdateChannels();
}
