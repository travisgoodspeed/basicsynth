//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Main menu implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "MainMenu.h"
#include "MainFrm.h"

static void prjcbNew(Fl_Widget*, void* v)
{
	prjFrame->NewProject();
}

static void prjcbOpen(Fl_Widget*, void* v)
{
	prjFrame->OpenProject(0);
}

static void prjcbSave(Fl_Widget*, void* v)
{
	prjFrame->SaveProject();
}

static void prjcbSaveAs(Fl_Widget*, void* v)
{
	prjFrame->SaveProjectAs();
}

static void prjcbSaveFile(Fl_Widget*, void* v)
{
	prjFrame->SaveFile();
}

static void prjcbSaveAll(Fl_Widget*, void* v)
{
	prjFrame->SaveAllEditors(0);
}

static void prjcbCloseFile(Fl_Widget*, void* v)
{
	prjFrame->CloseFile();
}

static void prjcbCloseAll(Fl_Widget*, void* v)
{
	prjFrame->CloseAllEditors();
}

static void prjcbRecent(Fl_Widget*, void* v)
{
	mainWnd->OpenRecent(reinterpret_cast<long>(v));
}

static void prjcbExit(Fl_Widget*, void* v)
{
	mainWnd->Exit();
}

static void prjcbUndo(Fl_Widget*, void* v) 
{
	prjFrame->EditUndo();
}

static void prjcbRedo(Fl_Widget*, void* v)
{
	prjFrame->EditRedo();
}

static void prjcbCut(Fl_Widget*, void* v)
{
	prjFrame->EditCut();
}

static void prjcbCopy(Fl_Widget*, void* v)
{
	prjFrame->EditCopy();
}

static void prjcbPaste(Fl_Widget*, void* v)
{
	prjFrame->EditPaste();
}

static void prjcbFind(Fl_Widget*, void* v)
{
	prjFrame->EditFind();
}

static void prjcbSelectAll(Fl_Widget*, void* v)
{
	prjFrame->EditSelectAll();
}

static void prjcbFindNext(Fl_Widget*, void* v)
{
	prjFrame->EditFindNext();
}

static void prjcbGotoLine(Fl_Widget*, void* v)
{
	prjFrame->EditGoto();
}

static void prjcbItemAdd(Fl_Widget*, void* v) 
{ 
	prjFrame->AddItem();
}

static void prjcbItemNew(Fl_Widget*, void* v)
{ 
	prjFrame->NewItem();
}

static void prjcbItemRem(Fl_Widget*, void* v)
{ 
	prjFrame->RemoveItem();
}

static void prjcbItemCopy(Fl_Widget*, void* v)
{
	prjFrame->CopyItem();
}

static void prjcbItemProps(Fl_Widget*, void* v)
{
	prjFrame->ItemProperties();
}

static void prjcbItemEdit(Fl_Widget*, void* v)
{ 
	prjFrame->EditItem();
}

static void prjcbItemSave(Fl_Widget*, void* v)
{ 
	prjFrame->SaveItem();
}

static void prjcbItemClose(Fl_Widget*, void* v)
{ 
	prjFrame->CloseItem();
}

static void prjcbGenerate(Fl_Widget*, void* v)
{
	prjFrame->Generate(0, -1);
}

static void prjcbPlay(Fl_Widget*, void* v)
{
	prjFrame->StartPlayer();
}

static void prjcbOptions(Fl_Widget*, void* v)
{
	mainWnd->ProjectOptions();
}

static void prjcbVwPrj(Fl_Widget*, void* v)
{
	mainWnd->ViewProject();
}

static void prjcbVwKbd(Fl_Widget*, void* v)
{
	mainWnd->ViewKeyboard();
}

static void prjcbAbout(Fl_Widget*, void* v)
{
	mainWnd->ShowAbout();
}

static void prjcbHelp(Fl_Widget*, void* v)
{
	mainWnd->ShowHelp();
}

static Fl_Menu_Item *mruMenuData;
static char *mruLabels;

// text shortcut callback user_data flags labeltype labelfont labelsize labelcolor
Fl_Menu_Item mainMenuData[] = 
{
/* 0 */
#define IDM_FILE 0
#define IDM_PRJ_NEW    (IDM_FILE+1)
#define IDM_PRJ_OPEN   (IDM_FILE+2)
#define IDM_PRJ_SAVE   (IDM_FILE+3)
#define IDM_PRJ_SAVEAS (IDM_FILE+4)
#define IDM_FILE_SAVE  (IDM_FILE+5)
#define IDM_FILE_SAVEALL  (IDM_FILE+6)
#define IDM_FILE_CLOSE (IDM_FILE+7)
#define IDM_FILE_CLOSEALL (IDM_FILE+8)
#define IDM_FILE_RECENT (IDM_FILE+9)
#define IDM_EXIT (IDM_FILE+10)
  {"&File", FL_ALT+'f', 0, 0, FL_SUBMENU},
    {"&New Project",    FL_SHIFT+FL_CTRL+'n', prjcbNew,  0, 0},
    {"&Open Project",   FL_SHIFT+FL_CTRL+'o', prjcbOpen, 0, 0},
    {"&Save Project",   FL_SHIFT+FL_CTRL+'s', prjcbSave, 0, FL_MENU_INACTIVE},
    {"Save Project As", 0, prjcbSaveAs, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
    {"&Save File",      FL_CTRL+'s', prjcbSaveFile, 0, FL_MENU_INACTIVE},
    {"Save All Files",  0, prjcbSaveAll, 0, FL_MENU_INACTIVE},
    {"Close File",      0, prjcbCloseFile, 0, FL_MENU_INACTIVE},
    {"Close All Files", 0, prjcbCloseAll, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
    {"Recent Projects", 0, 0, 0, FL_SUBMENU_POINTER|FL_MENU_DIVIDER},
    {"E&xit",           0, prjcbExit, 0, 0},
	{0},
/* 10 */
#define IDM_EDIT (IDM_EXIT+2)
#define IDM_EDIT_UNDO (IDM_EDIT+1)
#define IDM_EDIT_CUT  (IDM_EDIT+2)
#define IDM_EDIT_COPY (IDM_EDIT+3)
#define IDM_EDIT_PASTE (IDM_EDIT+4)
#define IDM_EDIT_SELALL (IDM_EDIT+5)
#define IDM_EDIT_FIND (IDM_EDIT+6)
#define IDM_EDIT_FINDNEXT (IDM_EDIT+7)
#define IDM_EDIT_GOTO (IDM_EDIT+8)
  {"&Edit", FL_ALT+'e', 0, 0, FL_SUBMENU},
    {"&Undo",   FL_CTRL+'z', prjcbUndo, 0, FL_MENU_INACTIVE},
    {"Cu&t",    FL_CTRL+'x', prjcbCut, 0, FL_MENU_INACTIVE},
    {"&Copy",   FL_CTRL+'c', prjcbCopy, 0, FL_MENU_INACTIVE},
    {"&Paste",  FL_CTRL+'v', prjcbPaste, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
    {"Select &All", FL_CTRL+'a', prjcbSelectAll, 0, FL_MENU_INACTIVE},
    {"&Find/Replace", FL_CTRL+'f', prjcbFind, 0, FL_MENU_INACTIVE},
    {"Find &Next", FL_F+3, prjcbFindNext, 0, FL_MENU_INACTIVE},
    {"&Goto line", FL_CTRL+'g', prjcbGotoLine, 0, FL_MENU_INACTIVE},
	{0},
/*21*/
#define IDM_ITEM (IDM_EDIT_GOTO+2)
#define IDM_ITEM_EDIT  (IDM_ITEM+1)
#define IDM_ITEM_PROP  (IDM_ITEM+2)
#define IDM_ITEM_ADD   (IDM_ITEM+3)
#define IDM_ITEM_NEW   (IDM_ITEM+4)
#define IDM_ITEM_COPY  (IDM_ITEM+5)
#define IDM_ITEM_REM   (IDM_ITEM+6)
#define IDM_ITEM_SAVE  (IDM_ITEM+7)
#define IDM_ITEM_CLOSE (IDM_ITEM+8)
  {"&Item", FL_ALT+'i', 0, 0, FL_SUBMENU},
    {"&Edit",   0, prjcbItemEdit, 0, FL_MENU_INACTIVE},
    {"&Properties",   0, prjcbItemProps, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
    {"&Add",    0, prjcbItemAdd, 0, FL_MENU_INACTIVE},
    {"&New",    0, prjcbItemNew, 0, FL_MENU_INACTIVE},
    {"&Copy",   0, prjcbItemCopy, 0, FL_MENU_INACTIVE},
    {"&Remove", 0, prjcbItemRem, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
    {"&Save",   0, prjcbItemSave, 0, FL_MENU_INACTIVE},
    {"C&lose",  0, prjcbItemClose, 0, FL_MENU_INACTIVE},
	{0},
/*31*/
#define IDM_PROJECT (IDM_ITEM_CLOSE+2)
#define IDM_PRJ_GEN  (IDM_PROJECT+1)
#define IDM_PRJ_PLAY (IDM_PROJECT+2)
#define IDM_PRJ_OPTS (IDM_PROJECT+3)
  {"&Project", FL_ALT+'p', 0, 0, FL_SUBMENU},
    {"&Generate",  0, prjcbGenerate, 0, FL_MENU_INACTIVE},
    {"&Play",      0, prjcbPlay, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
    {"&Options...", 0, prjcbOptions, 0, 0},
	{0},
/*36*/
#define IDM_VIEW (IDM_PRJ_OPTS+2)
#define IDM_VIEW_PRJ (IDM_VIEW+1)
#define IDM_VIEW_KBD (IDM_VIEW+2)
  {"&View",0,0,0,FL_SUBMENU},
    {"&Project", 0, prjcbVwPrj, 0, FL_MENU_TOGGLE},
    {"&Keyboard", 0, prjcbVwKbd, 0, FL_MENU_TOGGLE},
	{0},
/*40*/
#define IDM_HELP (IDM_VIEW_KBD+2)
#define IDM_HELP_CONTENTS (IDM_HELP+1)
#define IDM_HELP_ABOUT (IDM_HELP+2)
  {"&Help", FL_ALT+'h',0,0,FL_SUBMENU},
    {"&Help Contents...", 0, prjcbHelp, 0, 0},
    {"&About...", 0, prjcbAbout, 0, 0},
	{0},
  {0}
};

MainMenu::MainMenu(int w) : Fl_Menu_Bar(0, 0, w, 30, "mainMenu")
{
	mruMenuData = new Fl_Menu_Item[prjOptions.docMax+1];
	memset(mruMenuData, 0, sizeof(Fl_Menu_Item) * (prjOptions.docMax+1));
	mruLabels = new char[prjOptions.docMax * 80];
	memset(mruLabels, 0, sizeof(char) * prjOptions.docMax * 80);
	menu(mainMenuData);
}

MainMenu::~MainMenu()
{
	delete[] mruMenuData;
	delete mruLabels;
}

void MainMenu::UpdateRecentList()
{
	if (prjOptions.docCount == 0)
	{
		mainMenuData[IDM_FILE_RECENT].user_data((void*)0);
		return;
	}
	mainMenuData[IDM_FILE_RECENT].user_data((void*)mruMenuData);
	char *name = mruLabels;
	int n = 0;
	while (n < prjOptions.docCount)
	{
		snprintf(name, 79, "&%d - %s", n+1, (const char*)prjOptions.docList[n].name);
		mruMenuData[n].label(name);
		mruMenuData[n].callback(prjcbRecent, (void*)n);
		n++;
		name += 80;
	}
	while (n < prjOptions.docMax)
		mruMenuData[n++].label((const char *)0);
}

void MainMenu::ItemSelected(ProjectItem *pi)
{
	int file = 0;
	long flags = 0;
	if (pi)
	{
		flags = pi->ItemActions();
		file = pi->GetEditor() != 0;
	}
	EnableItem(IDM_ITEM_ADD, flags & ITM_ENABLE_ADD);
	EnableItem(IDM_ITEM_NEW, flags & ITM_ENABLE_NEW);
	EnableItem(IDM_ITEM_REM, flags & ITM_ENABLE_REM);
	EnableItem(IDM_ITEM_COPY, flags & ITM_ENABLE_COPY);
	EnableItem(IDM_ITEM_EDIT, flags & ITM_ENABLE_EDIT);
	EnableItem(IDM_ITEM_PROP, flags & ITM_ENABLE_PROPS);
	EnableItem(IDM_ITEM_SAVE, file && (flags & ITM_ENABLE_SAVE));
	EnableItem(IDM_ITEM_CLOSE, file && (flags & ITM_ENABLE_CLOSE));
}

void MainMenu::EditorSelected(EditorView *vw)
{
	long flags = 0;
	if (vw)
		flags = vw->EditState();
	UpdateEditState(flags);
}

void MainMenu::UpdateEditState(long flags)
{
	EnableItem(IDM_EDIT_COPY, flags & VW_ENABLE_COPY);
	EnableItem(IDM_EDIT_CUT, flags & VW_ENABLE_CUT);
	EnableItem(IDM_EDIT_PASTE, flags & VW_ENABLE_PASTE);
	EnableItem(IDM_EDIT_UNDO, flags & VW_ENABLE_UNDO);
//	EnableItem(IDM_EDIT_REDO, flags & VW_ENABLE_REDO);
	EnableItem(IDM_EDIT_GOTO, flags & VW_ENABLE_GOTO);
	EnableItem(IDM_EDIT_SELALL, flags & VW_ENABLE_SELALL);
	EnableItem(IDM_EDIT_FIND, flags & VW_ENABLE_FIND);
	EnableItem(IDM_EDIT_FINDNEXT, flags & VW_ENABLE_FIND);
	EnableFile(flags & VW_ENABLE_FILE);
}

void MainMenu::EnableFile(int e)
{
	EnableItem(IDM_FILE_SAVE, e);
	EnableItem(IDM_FILE_CLOSE, e);
}

void MainMenu::EnableProject(int e)
{
	EnableItem(IDM_PRJ_SAVE, e);
	EnableItem(IDM_PRJ_SAVEAS, e);
	EnableItem(IDM_FILE_SAVEALL, e);
	EnableItem(IDM_FILE_CLOSEALL, e);
	EnableItem(IDM_PRJ_GEN, e);
	EnableItem(IDM_PRJ_PLAY, e);
}

void MainMenu::EnableItem(int ndx, int enb)
{
	if (enb)
		mainMenuData[ndx].activate();
	else
		mainMenuData[ndx].deactivate();
}

void MainMenu::CheckProject(int ck)
{
	if (ck)
		mainMenuData[IDM_VIEW_PRJ].set();
	else
		mainMenuData[IDM_VIEW_PRJ].clear();
}

void MainMenu::CheckKeyboard(int ck)
{
	if (ck)
		mainMenuData[IDM_VIEW_KBD].set();
	else
		mainMenuData[IDM_VIEW_KBD].clear();
}
