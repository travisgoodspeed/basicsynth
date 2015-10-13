//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file FindReplDlgFltk.cpp Find/Replace dialog implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "FindReplDlgFltk.h"
#include "MainFrm.h"

static void findTextCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->SetEnable();
}

static void selFindTextCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->SelFindText();
}

static void selReplTextCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->SelReplText();
}

static void findCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->OnFindNext();
}

static void replCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->OnReplace();
}

static void replAllCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->OnReplaceAll();
}

static void replSelCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->OnReplaceSel();
}

static void openFindList(Fl_Widget *wdg, void *arg)
{
	Fl_Hold_Browser *lst = (Fl_Hold_Browser*)arg;
	// TODO: if visible hide() else show()
	if (lst->Fl_Widget::visible())
		lst->hide();
	else
		lst->show();
}

static void closeCB(Fl_Widget *wdg, void *arg)
{
	((FindReplDlgFltk*)arg)->OnCloseCmd();
}

FindReplDlgFltk::FindReplDlgFltk()
	: Fl_Window(100, 100, 440, 500, "Find-Replace")
{
	int txtHeight = 25;
	int txtSpace = txtHeight + 5;
	int ypos = 5;

	findText  = new Fl_Input(90, ypos, 300, txtHeight, "Find: ");
	findText->callback(findTextCB, (void*)this);
	int list1 = ypos+txtHeight;
	findListOpen = new Fl_Button(390, ypos, txtHeight, txtHeight, "@2>");

	ypos += txtSpace;
	replText  = new Fl_Input(90, ypos, 300, txtHeight, "Replace: ");
	replText->callback(findTextCB, (void*)this);
	replListOpen = new Fl_Button(390, ypos, txtHeight, txtHeight, "@2>");
	int list2 = ypos+txtHeight;

	ypos += txtSpace;
	int savy = ypos;
	matchBtn = new Fl_Check_Button(5, ypos, 200, txtHeight, "Match Case");
	ypos += txtSpace;
	fullWordBtn = new Fl_Check_Button(5, ypos, 200, txtHeight, "Whole word only");
	ypos += txtSpace;
	startWordBtn = new Fl_Check_Button(5, ypos, 200, txtHeight, "Start of word");
	ypos += txtSpace;
	regexpBtn = new Fl_Check_Button(5, ypos, 200, txtHeight, "Regular expression");
	ypos = savy;

	findBtn = new Fl_Button(220, ypos, 200, txtHeight, "Find Next");
	findBtn->callback(findCB, (void*)this);
	ypos += txtSpace;
	replBtn = new Fl_Button(220, ypos, 200, txtHeight, "Replace");
	replBtn->callback(replCB, (void*)this);
	ypos += txtSpace;
	replAllBtn = new Fl_Button(220, ypos, 200, txtHeight, "Replace All");
	replAllBtn->callback(replAllCB, (void*)this);
	ypos += txtSpace;
	replSelBtn = new Fl_Button(220, ypos, 200, txtHeight, "Replace Selection");
	replSelBtn->callback(replSelCB, (void*)this);
	ypos += txtSpace;

	statText = new Fl_Output(5, ypos, 300, txtHeight, "");
	//statText->box(FL_DOWN_FRAME);
	statText->box(FL_NO_BOX);
	statText->align(FL_ALIGN_LEFT);
	closeBtn = new Fl_Button(330, ypos, 90, txtHeight, "Close");
	closeBtn->callback(closeCB, (void*)this);
	ypos += txtSpace;

	findList = new Fl_Hold_Browser(90, list1, 300+txtHeight, ypos-list1-5, "");
	findList->callback(selFindTextCB, (void*)this);
	findList->hide();
	findListOpen->callback(openFindList, (void*)findList);

	replList = new Fl_Hold_Browser(90, list2, 300+txtHeight, ypos-list2-5, "");
	replList->callback(selReplTextCB, (void*)this);
	replList->hide();
	replListOpen->callback(openFindList, (void*)replList);

	end();
	int xx = mainWnd->x() + (mainWnd->w() / 2) - 220;
	int yy = mainWnd->y() + (mainWnd->h() / 2) - ypos/2;
	resize(xx, yy, 440, ypos);
}

FindReplDlgFltk::~FindReplDlgFltk()
{
}

int FindReplDlgFltk::handle(int evt)
{
	return Fl_Window::handle(evt);
}

void FindReplDlgFltk::OnFindNext()
{
	if (!ed)
		return;

	bsString ftext;
	GetFindText(findText, findList, ftext);
	int res = ed->Find(GetFlags(), ftext);
	const char *msg = "";
	if (res == -1)
		msg = "Search text was not found";
	else if (res == 1)
		msg = "Passed end of file.";
	statText->value(msg);
}

void FindReplDlgFltk::OnReplace()
{
	if (!ed)
		return;

	bsString ftext;
	bsString rtext;
	GetFindText(findText, findList, ftext);
	GetFindText(replText, replList, rtext);
	int flags = GetFlags();
	const char *msg = "Search text was not found";
	if (ed->MatchSel(flags, ftext))
		ed->Replace(rtext);
	if (ed->Find(flags, 0) != -1)
		msg = "";
	statText->value(msg);
}

void FindReplDlgFltk::OnReplaceAll()
{
	DoReplace(0);
}

void FindReplDlgFltk::OnReplaceSel()
{
	DoReplace(1);
}

void FindReplDlgFltk::DoReplace(int inSel)
{
	if (!ed)
		return;

	bsString ftext;
	bsString rtext;
	GetFindText(findText, findList, ftext);
	GetFindText(replText, replList, rtext);

	int flags = GetFlags();
	SelectInfo si;
	if (inSel)
		ed->GetSelection(si);
	else
		si.startPos = si.endPos = 0;
	int count = ed->ReplaceAll(flags, ftext, rtext, si);
	if (inSel)
		ed->SetSelection(si);
	char msg[256];
	snprintf(msg, 256, "%d replacement%s", count, count == 1 ? "." : "s.");
	statText->value(msg);
}

void FindReplDlgFltk::OnCloseCmd()
{
	if (ed)
		ed->Focus();
	hide();
}

int FindReplDlgFltk::GetFlags()
{
	int flags = 0;
	if (matchBtn->value())
		flags |= TXTFIND_MATCHCASE;
	if (fullWordBtn->value())
		flags |= TXTFIND_WHOLEWORD;
	if (startWordBtn->value())
		flags |= TXTFIND_WORDSTART;
	if (regexpBtn->value())
		flags |= TXTFIND_REGEXP;
	return flags;
}

void FindReplDlgFltk::SetEnable()
{
/*
	const char *txt = findText->value();
	int enableFind = (ed != NULL) && txt && strlen(txt) > 0;
	if (enableFind)
	{
		findBtn->activate();
		replAllBtn->activate();
		replSelBtn->activate();
	}
	else
	{
		findBtn->deactivate();
		replAllBtn->deactivate();
		replSelBtn->deactivate();
	}
	if (ed != NULL)
		replBtn->activate();
	else
		replBtn->deactivate();
*/
}

int FindReplDlgFltk::GetFindText(Fl_Input *txt, Fl_Hold_Browser *lst, bsString& text)
{
	int sel = 0;
	const char *tp = txt->value();
	if (tp)
	{
		int lines = lst->size();
		int index = 1;
		while (index <= lines)
		{
			const char *lp = lst->text(index);
			if (lp && strcmp(lp, tp) == 0)
			{
				sel = index;
				break;
			}
			index++;
		}
		if (index > lines)
			lst->insert(0, tp);
		while (lines > 20)
			lst->remove(lines--);
	}
	text = tp;
	return sel;
}

void FindReplDlgFltk::SelFindText()
{
	int sel = findList->value();
	if (sel > 0)
	{
		const char *txt = findList->text(sel);
		if (txt)
			findText->value(txt);
	}
	findList->hide();
}

void FindReplDlgFltk::SelReplText()
{
	int sel = replList->value();
	if (sel > 0)
	{
		const char *txt = replList->text(sel);
		if (txt)
			replText->value(txt);
	}
	replList->hide();
}
